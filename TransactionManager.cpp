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
    int youngestStartTime = INT_MAX;
    if(blockingGraph.empty()) std::cout << "empty block graph" << std::endl;
    for (auto const& blocking : blockingGraph) {
        std::unordered_set<std::string> visited;
        std::string start = blocking.first;
        
        std::cout << "start edge: " << start << std::endl;
        
        // dfs
        if (TransactionManager::helper(start, start, visited, blockingGraph)) {
            Transaction transaction = transactionList.at(start);
            std::cout << transaction.name << std::endl;
            // get the youngest
            if (transaction.startTime < youngestStartTime) {
                youngest = start;
                youngestStartTime = transaction.startTime;
            }
        }
    }
    // abort the youngest
    if (!youngest.empty()) {
        TransactionManager::abort(youngest);
        std::cout << "abort" << std::endl;
        return true;
    }
    return false;
}


bool TransactionManager::helper(std::string start, std::string target, std::unordered_set<std::string>& visited,
                                std::unordered_map<std::string, std::vector<std::string>> blockingGraph) {
    std::cout << "visited.size()= " << visited.size() << std::endl;
    if (visited.size() != 0 && start == target) {
        std::cout << "returning true from helper" << std::endl;
        return true;
    }
    for (auto const& transactionBlocked : blockingGraph.at(start)) {
        std::cout << "blocked transaction: " << transactionBlocked << std::endl;
        if (visited.find(transactionBlocked) == visited.end()) {
            std::cout << "didn't find blocked transaction in visited: " << transactionBlocked << std::endl;
            visited.insert(transactionBlocked);
            if (TransactionManager::helper(transactionBlocked, target, visited, blockingGraph)) {
                std::cout << "returning true from helper" << std::endl;
                return true;
            }
        }
    }
    return false;
}

void TransactionManager::executeNextTransaction(std::string tran) {
    Transaction curTran = transactionList.at(tran);
    
    bool isSuccessful = false;

    // get first transaction from wating queue of the variable which the transaction has accessed before
    for (auto const& var : curTran.getFreeVars()) {
        vector<site*> sites = DM.getVarSiteList()[var];
        variable* v;
        for(auto s : sites) {
            v = s->getVariable(var);
            break;
        }
        Transaction* nextTran = v->getLock()->getTransactionFromWaitingQueue();
        std::string nextTranName = nextTran->name;
        auto it = std::find_if(instructionQueue.begin(), instructionQueue.end(),
                                [&nextTranName](const Instruction& ins) { 
            return ins.tran == nextTranName;
        });
        Instruction nextIns = *it;

        if (nextTran == nullptr) {
            // auto it = std::find(instructionQueue.begin(), instructionQueue.end(), nextIns);
            instructionQueue.erase(it);
            continue;
        }
        if (nextIns.type == "R") {
            isSuccessful = TransactionManager::read(nextIns.tran, nextIns.var);
        } else if (nextIns.type == "W") {
            isSuccessful = TransactionManager::write(nextIns.tran, nextIns.var, nextIns.val);
        }
        // erase instruction when completed successfully
        if (isSuccessful) {
            v->getLock()->removeTransactionFromWaitingQueue(nextTran);
            // auto it = std::find(instructionQueue.begin(), instructionQueue.end(), nextIns);
            instructionQueue.erase(it);
        }
    }
}


/* modify blockingGraph */
void TransactionManager::addEdge(std::string trans1, std::string trans2) {
    blockingGraph[trans1].push_back(trans2);
    std::cout << trans1 << " is blocking " << trans2 << endl;
}


void TransactionManager::deleteEdge(std::string trans1)  {
    blockingGraph.erase(trans1);
}


/* execution */
void TransactionManager::begin(std::string tran, bool isReadOnly, int startTime) {
    Transaction transaction = Transaction(tran, isReadOnly, startTime);
    if (isReadOnly) {
        DM.generateVarValCache(&transaction);
    }
    // add new transaction
    transactionList.insert({ tran, transaction });
}


bool TransactionManager::readRO(std::string tran, std::string var) {
    // check if var is in the varValCache
    Transaction transaction = transactionList.at(tran);
    int res = transaction.getVarValCache().at(varID);
    if (res) {
        std::cout << var << ": " << res << std::endl;
        return true;
    }
    return false;
}


bool TransactionManager::read(std::string tran, std::string var) {

    Transaction transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));

    std::string res;
    bool isSuccessful = false;
    // read-only
    if (transaction.isReadOnly) {
        isSuccessful = TransactionManager::readRO(tran, var);
    }
    // non read-only
    res = DM.read(&transaction, varID);

    if (res.rfind("T")) {
        transaction.printLockConflict(tran);
        // update blockingGraph
        TransactionManager::addEdge(res, tran);
    } else if (res == "ABORT") {
        TransactionManager::abort(tran);
    } else if (res == "DOWN") {
        transaction.printDownSite(tran);
    } else if (res == "INVALID") {
        transaction.printInvalid(tran);
    } else if (res == "DOWN_INVALID") {
        transaction.printDownInvalid(tran);
    } else {
        std::cout << var << ": " << res << std::endl;
        // get all accessed sites
        std::unordered_set<int> accessedSites = transaction.getOwnedLocks().at(varID);
        // update SiteAccessedList
        for (auto const& site: accessedSites){
            transaction.getSiteAccessedList().push_back(site);
        }
        isSuccessful = true;
    }
    return isSuccessful;
}


bool TransactionManager::write(std::string tran, std::string var, int val) {
    std::cout << "enetered tm write" << std::endl;
    Transaction transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));
    std::cout << "calling dm write" << std::endl;
    std::string res = DM.write(&transaction, varID, val);

    std::cout << "tm got result= " << res << std::endl;

    bool isSuccessful = false;

    if (res.rfind("T",0) == 0) {
        std::cout << "found T: " << res << std::endl;
        transaction.printLockConflict(tran);
        // update blockingGraph
        TransactionManager::addEdge(res, tran);
        std::cout << "added edge" << std::endl;
    } else if (res == "DOWN") {
        std::cout << "founud down: " << res << std::endl;
        transaction.printDownSite(tran);
    } else {
        std::cout << var << " can be updated to " << val << " after commit." << std::endl;
        // get all accessed sites
        std::unordered_set<int> accessedSites = transaction.getOwnedLocks().at(varID);
        // update SiteAccessedList
        for (auto const& site: accessedSites){
            transaction.getSiteAccessedList().push_back(site);
        }
        isSuccessful = true;
    }
    // update varValList
    transaction.varValueList[varID] = val;
    //transaction.getVarValueList()[varID] = val;
    std::cout << "almost done with tm write" << std::endl;
    return isSuccessful;
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
    std::cout << tran << " aborts" << std::endl;
    
    Transaction transaction = transactionList.at(tran);
    // release locks hold by the transaction
    transaction.getFreeVars() = DM.releaseLocks(&transaction);
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

        Transaction transaction = transactionList.at(t->name);
        
        std::string tran = t->name;
        // update blocking graph
        TransactionManager::deleteEdge(tran);
        // execute next transaction from waiting queue
        TransactionManager::executeNextTransaction(tran);
        // update transaction list
        transactionList.erase(tran);
    } else {
        std::cout << t->name << " commits failed" << std::endl;
    }
}


void TransactionManager::fail(int siteID, int timeStamp) {    
    DM.fail(siteID, timeStamp);
    /* When a site fails, all transactions which ever accessed this site should be aborted in the end */
    for (auto const& tran : transactionList) {
        Transaction transaction = tran.second;
        if (transaction.isReadOnly || transaction.toBeAborted) {
            continue;
        }
        if ( std::find(transaction.getSiteAccessedList().begin(), transaction.getSiteAccessedList().end(),
        siteID) != transaction.getSiteAccessedList().end()) {
                transaction.toBeAborted = true;
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
    Transaction transaction = transactionList.at(tran);
    int varID = std::stoi(var.substr(1));
    transaction.getVarAccessedList().push_back(varID);
}
