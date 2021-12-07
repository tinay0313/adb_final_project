/** @file TransactionManager.cpp
    @author Ann Tsai mt4050
 */

#include "TransactionManager.h"
#include "Transaction.h"

#include <cstddef>


TransactionManager::TransactionManager():
    DM(),
    timeStamp(0),
    varID(0),
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

void TransactionManager::deleteEdge(std::string trans1)  {
    blockingGraph.erase(trans1);
}

/* execution */
void TransactionManager::begin(std::string tran, bool isReadOnly, int startTime) {
    // add new transaction
    transactionList.insert({ tran, Transaction(tran, isReadOnly, startTime) });
}

void TransactionManager::enqueueReadInstruction(std::string tran, std::string var) {
    instructionQueue.push_back({"R", tran, var, 0});
}

void TransactionManager::enqueueWriteInstruction(std::string tran, std::string var, int val) {
    instructionQueue.push_back({"W", tran, var, val});
}

void TransactionManager::read(std::string tran, std::string var) {

    Transaction transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));

    std::string res = DM.read(&tran, varID);

    if (res == "conflict") {
        transaction.printLockConflict(tran);
        // update blockingGraph
        for (auto & site : DM.getSites()) {
            if (site->getIsRunning() & site->getAllVariables().count(varID)!= 0
                & site->isVariableValidForRead(varID) & site->getLockType(varID) == 1
                || site->getLockType(varID) == 2) {
                    std::string blocker = site->getVariable(varID).getLock().getOwners().name;
                    // add edage
                    TransactionManager::addEdge(blocker, tran);
            }
        }
    } else if (res == "down") {
        transaction.printDownSite(tran);
    } else {
        std::cout << "Read sucessfully. " << var << " = " << res << std::endl;
    }
}

bool TransactionManager::write(std::string tran, std::string var, int val) {
    
    Transaction transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));

    std::string res = DM.write(&tran, varID, val);

    if (res == "conflict") {
        transaction.printLockConflict(tran);
        // update blockingGraph
        for (auto & site : DM.getSites()) {
            if (site->getIsRunning() & site->getAllVariables().count(varID)!= 0
                & site->isVariableValidForRead(varID) & site->getLockType(varID) == 2) {
                    std::string blocker = site->getVariable(varID).getLock().getOwners().name;
                    // add edage
                    TransactionManager::addEdge(blocker, tran);
            }
        }
    } else if (res == "down") {
        transaction.printDownSite(tran);
    } else {
        std::cout << var << " can be updated to " << val << " after commit." << std::endl;
    }
    // update varValList
    transaction.getVarValueList().at(varID).push_back(val);
}

void TransactionManager::dump() {    
    DM.dump();
}

void TransactionManager::end(std::string tran) {
    Transaction transaction = transactionList.at(tran);
    if (transaction.toBeAborted) {
        TransactionManager::abort(tran);
    } else {
        TransactionManager::commit(&transaction, transaction.getVarValueList());
    }
}

void TransactionManager::abort(std::string tran) {
    std::cout << "Abort transaction: " << tran << std::endl;
    DM.releaseLocks(tran);
    // update transaction list
    transactionList.erase(tran);
    // update blocking graph
    TransactionManager::deleteEdge(tran);
}

void TransactionManager::commit(Transaction* t, unordered_map<int, int> varValueList) {
    DM.commit(t, varValueList);
    // update transaction list
    std::string tran = t->name;
    transactionList.erase(tran);
    // update blocking graph
    TransactionManager::deleteEdge(tran);
}

void TransactionManager::fail(int siteID, int timeStamp) {    
    DM.fail(siteID, timeStamp);
    /* When a site fails, all transactions which ever accessed this site should be aborted in the end */
    for (auto const& tran : transactionList) {
        Transaction transaction = tran.second;
        if (transaction.isReadOnly || transaction.toBeAborted) {
            continue;
        }
        if (std::find(transaction.getSiteAccessedList().begin(), transaction.siteAccessedList.end(),
            siteID)!=transaction.getSiteAccessedList().end()) {
                transaction.toBeAborted = true;
        }
    }
}

void TransactionManager::recover(int siteID) {
    DM.recover(siteID);
}
