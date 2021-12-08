/** @file TransactionManager.cpp
    @author Ann Tsai mt4050
 */

#include "TransactionManager.h"
#include "Transaction.h"

#include <cstddef>
#include <climits>
#include <algorithm>


TransactionManager::TransactionManager():
    DM(),
    timeStamp(0),
    varID(0),
    blockingGraph(),
    transactionList(),
    instructionQueue()
{};


bool TransactionManager::detectDeadlock() {
    std::string youngest;
    int youngestStartTime = INT_MIN;
    for (auto const& blocking : blockingGraph) {
        std::unordered_set<std::string> visited;
        std::string start = blocking.first;
        // dfs
        if (TransactionManager::helper(start, start, visited, blockingGraph)) {
            Transaction* transaction = transactionList.at(start);
            // get the youngest
            if (transaction->startTime > youngestStartTime) {
                youngest = start;
                youngestStartTime = transaction->startTime;
            }
        }
    }
    // abort the youngest
    if (!youngest.empty()) {
        TransactionManager::abort(youngest);
        return true;
    }
    return false;
}

bool TransactionManager::helper(std::string start, std::string target, std::unordered_set<std::string>& visited,
                                std::unordered_map<std::string, std::vector<std::string>> blockingGraph) {
    
    if (visited.size() != 0 && start == target) {
        return true;
    }
    if(!blockingGraph.count(start)) {
        return false;
    }
    for (auto const& transactionBlocked : blockingGraph.at(start)) {
        if (visited.find(transactionBlocked) == visited.end()) {
            //haven't seen this transaction in topological sort yet
            visited.insert(transactionBlocked);
            if (TransactionManager::helper(transactionBlocked, target, visited, blockingGraph)) {
                return true;
            }
        }
    }
    return false;
}

void TransactionManager::executeNextTransaction(std::string tran) {
    Transaction* curTran = transactionList.at(tran);
    bool isSuccessful = false;

    // get first transaction from wating queue of the variable which the transaction has accessed before
    for (auto const& var : curTran->getFreeVars()) {
        vector<site*> sites = DM.getVarSiteList()[var];
        //if there is a transaction in the waiting queue of the now free variables
        //the transaction name would be stored in nextTranName
        std::string nextTranName;
        for(auto s : sites) {
            //don't check s's waiting queue if s is down
            if(!s->getIsRunning()) continue;

            //no transaction at this site's waiting queue for variable var
            if(s->getLockTable()[var]->getWaitingQueue().empty()) {
                continue;
            }
            nextTranName = s->getLockTable()[var]->getTransactionFromWaitingQueue()->name;
            //found the next waiting transaction
            if(nextTranName.length() > 0) break;
        }
        Transaction* nextTran = transactionList[nextTranName];
        if (nextTran == nullptr) {
            continue;
        }

        auto it = std::find_if(instructionQueue.begin(), instructionQueue.end(),
                                [&nextTranName](const Instruction& ins) { 
            return ins.tran == nextTranName;
        });
        Instruction nextIns = *it;

        if (nextIns.type == "R") {
            isSuccessful = TransactionManager::read(nextIns.tran, nextIns.var);
        } else if (nextIns.type == "W") {
            isSuccessful = TransactionManager::write(nextIns.tran, nextIns.var, nextIns.val);
        }
        // erase instruction when completed successfully
        if (isSuccessful) {
            for(auto s : sites) {
                if(!s->getIsRunning()) continue;
                if(s->getLockTable()[var]->getTransactionFromWaitingQueue()->name == nextTran->name)
                    s->getLockTable()[var]->removeTransactionFromWaitingQueue(nextTran);
            }
        }
        
    }
}


/* modify blockingGraph */
void TransactionManager::addEdge(std::string trans1, std::string trans2) {
    blockingGraph[trans1].push_back(trans2);
}


void TransactionManager::deleteEdge(std::string trans1)  {
    blockingGraph.erase(trans1);
}


/* execution */
void TransactionManager::begin(std::string tran, bool isReadOnly, int startTime) {
    Transaction* transaction = new Transaction(tran, isReadOnly, startTime);
    if (isReadOnly) {
        DM.generateVarValCache(transaction);
    }
    // add new transaction
    transactionList.insert({ tran, transaction });
}


bool TransactionManager::readRO(std::string tran, std::string var) {
    // check if var is in the varValCache
    Transaction* transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));

    if(transaction->getVarValCache().empty()) {
        DM.generateVarValCache(transaction);
    }

    int hasCache = false;
    int res;
    if(transaction->getVarValCache().count(varID)) {
        hasCache = true;
        res = transaction->getVarValCache().at(varID);
    }
    if (hasCache) {
        std::cout << var << ": " << res << std::endl;
        return true;
    }
    return false;
}


bool TransactionManager::read(std::string tran, std::string var) {

    Transaction* transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));

    std::string res;
    bool isSuccessful = false;
    // read-only
    if (transaction->isReadOnly) {
        isSuccessful = TransactionManager::readRO(tran, var);
        if(!isSuccessful) {
            //couldn't read value so abort
            TransactionManager::abort(tran);
        } else {
            auto it = std::find_if(instructionQueue.begin(), instructionQueue.end(),
                                    [&tran](const Instruction& ins) { 
                return ins.tran == tran;
            });
            Instruction nextIns = *it;
            instructionQueue.erase(it);
        }
    } else {
        // non read-only
        res = DM.read(transaction, varID);
        if (res.rfind("T",0) == 0) {
            transaction->printLockConflict(tran);
            // update blockingGraph
            TransactionManager::addEdge(res, tran);
        } else if (res == "DOWN") {
            transaction->printDownSite(tran);
        } else if (res == "INVALID") {
            transaction->printInvalid(tran);
        } else if (res == "DOWN_INVALID") {
            transaction->printDownInvalid(tran);
        } else {
            std::cout << var << ": " << res << std::endl;
            // get all accessed sites
            std::unordered_set<int> accessedSites = transaction->getOwnedLocks().at(varID);
            // update SiteAccessedList
            for (auto const& site: accessedSites){
                transaction->getSiteAccessedList().push_back(site);
            }
            isSuccessful = true;
        }
        if(isSuccessful) {
            auto it = std::find_if(instructionQueue.begin(), instructionQueue.end(),
                                    [&tran](const Instruction& ins) { 
                return ins.tran == tran;
            });
            Instruction nextIns = *it;
            instructionQueue.erase(it);
        }
    }
    while (detectDeadlock()) {
        std::cout << "Deadlock detected." << std::endl;
    }
    return isSuccessful;
}


bool TransactionManager::write(std::string tran, std::string var, int val) {
    Transaction* transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));
    std::string res = DM.write(transaction, varID, val);

    bool isSuccessful = false;

    if (res.rfind("T",0) == 0) {
        transaction->printLockConflict(tran);
        // update blockingGraph
        TransactionManager::addEdge(res, tran);
    } else if (res == "DOWN") {
        transaction->printDownSite(tran);
    } else {
        std::cout << var << " can be updated to " << val << " after commit." << std::endl;
        // get all accessed sites
        std::unordered_set<int> accessedSites = transaction->getOwnedLocks().at(varID);
        // update SiteAccessedList
        for (auto const& site: accessedSites){
            transaction->getSiteAccessedList().push_back(site);
        }
        // update varValList
        transaction->varValueList[varID] = val;
        isSuccessful = true;
    }
    // update varValList
    transaction->varValueList[varID] = val;
    if(isSuccessful) {
        auto it = std::find_if(instructionQueue.begin(), instructionQueue.end(),
                                [&tran](const Instruction& ins) { 
            return ins.tran == tran;
        });
        Instruction nextIns = *it;
        instructionQueue.erase(it);
    }
    while (detectDeadlock()) {
        std::cout << "Deadlock detected." << std::endl;
    }
    return isSuccessful;
}


void TransactionManager::dump() {
    DM.dump();
}


void TransactionManager::end(std::string tran) {
    Transaction* transaction = transactionList.at(tran);
    
    if (transaction->toBeAborted) {
        TransactionManager::abort(tran);
    } else {
        TransactionManager::commit(transaction, transaction->getVarValueList());
    }
}


void TransactionManager::abort(std::string tran) {
    std::cout << tran << " aborts" << std::endl;
    
    Transaction* transaction = transactionList.at(tran);
    // release locks hold by the transaction
    DM.releaseLocks(transaction);
    // update blocking graph
    TransactionManager::deleteEdge(tran);
    // execute next transaction from waiting queue
    TransactionManager::executeNextTransaction(tran);
    // update transaction list
    transactionList.erase(tran);
}

// todo why no match declaration?
void TransactionManager::commit(Transaction* t, std::unordered_map<int, int> varValueList) {
    pair<bool, unordered_set<int>> res = DM.commit(t); // already released locks

    bool success = res.first;
    t->getFreeVars() = res.second;

    if (success) {
        std::cout << t->name << " commits" << std::endl;
    } else {
        std::cout << t->name << " commits failed" << std::endl;
    }
    std::string tran = t->name;
    // update blocking graph
    TransactionManager::deleteEdge(tran);
    // execute next transaction from waiting queue
    TransactionManager::executeNextTransaction(tran);
    // update transaction list
    transactionList.erase(tran);
}


void TransactionManager::fail(int siteID, int timeStamp) {    
    DM.fail(siteID, timeStamp);

    /* When a site fails, all transactions which ever accessed this site should be aborted in the end */
    for (auto const& tran : transactionList) {
        if(tran.first == "") {
            continue;
        }
        Transaction* transaction = tran.second;
        if (transaction->isReadOnly || transaction->toBeAborted) {
            continue;
        }
        if ( std::find(transaction->getSiteAccessedList().begin(), transaction->getSiteAccessedList().end(),
        siteID) != transaction->getSiteAccessedList().end()) {
                transaction->toBeAborted = true;
        }
    }
}


void TransactionManager::recover(int siteID) {
    DM.recover(siteID);
}


void TransactionManager::enqueueReadInstruction(std::string tran, std::string var) {
    Instruction ins = {
        "R",
        tran,
        var,
        0
    };
    instructionQueue.push_back(ins);
    read(tran, var);
}


void TransactionManager::enqueueWriteInstruction(std::string tran, std::string var, int val) {
    Instruction ins = {
        "W",
        tran,
        var,
        val
    };
    instructionQueue.push_back(ins);
    write(tran, var, val);
}


void TransactionManager::updateVarAccessedList(std::string tran, std::string var) {
    if(!transactionList.count(tran)) return;
    Transaction* transaction = transactionList.at(tran);
    int varID = std::stoi(var.substr(1));
    transaction->getVarAccessedList().push_back(varID);
}
