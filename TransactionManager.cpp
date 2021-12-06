/** @file TransactionManager.cpp
    @author Ann Tsai mt4050
 */

#include "TransactionManager.h"
#include "Transaction.h"

#include <cstddef>



TransactionManager::TransactionManager():
    DM(),
    timeStamp(0),
    siteSize(11),
    siteVariableList(),
    failureHistory(),
    blockingGraph(),
    transactionList(),
    instructionQueue()
{};

bool TransactionManager::detectDeadlock() {

    std::string youngest;
    int younestStartTime = INT_MAX;

    for (auto const& blocking : blockingGraph) {
        std::unordered_set<std::string> visited;
        std::string start = blocking.first;
        // dfs
        if (TransactionManager::helper(start, start, visited, blockingGraph)) {
            Transaction transaction = transactionList[start];
            // find the youngest
            if (transaction.startTime < younestStartTime) {
                youngest = start;
                younestStartTime = transaction.startTime;
            }
        }
    }
    // abort the youngest
    if (!youngest.empty()) {
        TransactionManager::abort(youngest);
    }
}

// todo why no match declaration?
bool TransactionManager::helper(std::string start, std::string target, std::unordered_set<std::string> visited,
                                std::unordered_map<std::string, std::vector<std::string>> blockingGraph) {
    if (visited.size() != 0 & start == target) {
        return true;
    }
    for (auto const& transactionBlocked : blockingGraph.at(start)) {
        if (visited.find(transactionBlocked) == visited.end()) {
            visited.insert(transactionBlocked);
            if (TransactionManager::helper(transactionBlocked, target, visited, blockingGraph)) {
                return true;
            }
        }
    return false;
    }
}

void TransactionManager::executeInstructionQueue() {
    bool isSuccessful = false;
    for (auto const& ins : instructionQueue) {
        Transaction transaction = transactionList.at(ins.tran);
        Transaction* tran_ptr = &transaction;
        if (tran_ptr == nullptr) {
            instructionQueue.erase(ins);
            continue;
        }
        if (ins.type == "R") {
            if (transaction.isReadOnly) {
                isSuccessful = TransactionManager::read(ins.tran, ins.var);
            }
        } else if (ins.type == "W") {
            isSuccessful = TransactionManager::write(ins.tran, ins.var, ins.val);
        }
        // erase instruction when completed successfully
        if (isSuccessful) {
            instructionQueue.erase(ins);
        }
    }
}

/* modify blockingGraph */
void TransactionManager::addEdge(std::string trans1, std::string trans2) {
    blockingGraph.at(trans1).push_back(trans2);
}

void TransactionManager::deleteEdge(std::string trans1, std::string trans2)  {
    std::vector<std::string> blocked_trans = blockingGraph.at(trans1);
    blocked_trans.erase(std::remove(blocked_trans.begin(), blocked_trans.end(), trans2), blocked_trans.end());
    // update value
    auto it = blockingGraph.find(trans1);
    if (it != blockingGraph.end()) {
        it->second = blocked_trans;
    }
}

/* execution */
void TransactionManager::begin(std::string tran, bool isReadOnly, int startTime) {
    // add new transaction
    transactionList.insert({ tran, Transaction(tran, isReadOnly, startTime) });
}

void TransactionManager::enqueueReadInstruction(std::string tran, std::string var) {
    // instructionQueue.push_back({"R", tran, var, 0}); // todo pointer being freed was not allocated
}

void TransactionManager::enqueueWriteInstruction(std::string tran, std::string var, int val) {
    // instructionQueue.push_back({"W", tran, var, val}); // todo pointer being freed was not allocated
}

bool TransactionManager::read(std::string tran, std::string var) {
    
    Transaction transaction = transactionList.at(tran);

    for (auto & site : DM.getSites()) {
        int varID; // TODO ???
        if (site->getIsRunning() & site->getAllVariables().count(varID)!= 0
            & site->isVariableValidForRead(varID) & site->getLockType(varID) == 1
            || site->getLockType(varID) == 2) {
            
            DM.read(&transaction, varID); // TODO ??????
            return true;
        }
    }
    return false;
}

bool TransactionManager::write(std::string tran, std::string var, int val) { // TODO
    std::cout << "info: "<< tran << " " <<  " " << var << " " << val << std::endl;
    
    Transaction transaction = transactionList.at(tran);

    bool canGetAllLocks = true;
    bool isAtLeastOneSitesUp = false;

    int varID; // TODO ???

    // check can write or not
    for (auto & site : DM.getSites()) {
        // check site status
        if (site->getIsRunning() & site->getAllVariables().count(varID)!= 0) {
            isAtLeastOneSitesUp = true;
            if (site->getLockType(varID) != 2) {
                // check lock type
                canGetAllLocks = false;
            }      
        }
    }
    // write
    if (isAtLeastOneSitesUp & canGetAllLocks) {
        for (auto & site : DM.getSites()) {
            if (site->getIsRunning() & site->getAllVariables().count(varID)!= 0) {
                DM.write(&transaction, varID, val); // TODO ??????
                return true;
            }
        }    
    }
    return false;
    
}

void TransactionManager::dump() {    
    DM.dump();
}

void TransactionManager::end(std::string tran) {
    Transaction transaction = transactionList.at(tran);
    if (transaction.toBeAborted) {
        TransactionManager::abort(tran);
    } else {
        unordered_map<int, int> variableValueMap;  // todo
        TransactionManager::commit(&transaction, variableValueMap); // todo variableValueMap
    }
    transactionList.erase(tran);
}

// todo move to site??? wait for DM
void TransactionManager::abort(std::string tran) {
    std::cout << "Abort transaction: " << tran << std::endl;
}

// todo site? wait for DM
void TransactionManager::commit(Transaction* t, unordered_map<int, int> variableValueMap) {
     DM.commit(t, variableValueMap); // todo variableValueMap???????
}

void TransactionManager::fail(int siteID, int timeStamp) {    
    DM.fail(siteID);
    /* When a site fails, all transactions which ever accessed this site should be aborted in the end */
    for (auto const& tran : transactionList) {
        Transaction transaction = tran.second;
        if (transaction.isReadOnly || transaction.toBeAborted) {
            continue;
        }
        
        if (std::find(transaction.siteAccessedList.begin(), transaction.siteAccessedList.end(),
            siteID)!=transaction.siteAccessedList.end()) {
                transaction.toBeAborted = true;
        }
    }
}

void TransactionManager::recover(int siteID) {
    DM.recover(siteID);
}

