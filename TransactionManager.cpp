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
            // get the youngest
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


void TransactionManager::executeNextTransaction(std::string tran) {

    Transaction curTran = transactionList.at(tran);
    bool isSuccessful = false;

    // get first transaction from wating queue of the variable which the transaction has accessed before
    for (auto const& var : curTran.getFreeVars()) {
        Transaction* nextTran = var.getLock().getTransactionFromWaitingQueue();
        Instruction nextIns = instructionQueue.at(nextTran->name);

        if (nextTran == nullptr) {
            instructionQueue.erase(nextIns);
            continue;
        }
        if (nextIns.type == "R") {
            isSuccessful = TransactionManager::read(nextIns.tran, nextIns.var);
        } else if (nextIns.type == "W") {
            isSuccessful = TransactionManager::write(nextIns.tran, nextIns.var, nextIns.val);
        }
        // erase instruction when completed successfully
        if (isSuccessful) {
            var->removeTransactionFromWaitingQueue(nextTran);
            instructionQueue.erase(nextIns);
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
    res = DM.read(&tran, varID);

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
            transaction.getSiteAccessedList().insert(site);
        }
        isSuccessful = true;
    }
    return isSuccessful;
}


bool TransactionManager::write(std::string tran, std::string var, int val) {
    
    Transaction transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));

    std::string res = DM.write(&tran, varID, val);
    bool isSuccessful = false;

    if (res.rfind("T")) {
        transaction.printLockConflict(tran);
        // update blockingGraph
        TransactionManager::addEdge(res, tran);
    } else if (res == "DOWN") {
        transaction.printDownSite(tran);
    } else {
        std::cout << var << " can be updated to " << val << " after commit." << std::endl;
        // get all accessed sites
        std::unordered_set<int> accessedSites = transaction.getOwnedLocks().at(varID);
        // update SiteAccessedList
        for (auto const& site: accessedSites){
            transaction.getSiteAccessedList().insert(site);
        }
        isSuccessful = true;
    }
    // update varValList
    transaction.getVarValueList().at(varID).push_back(val);

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
    transaction.getFreeVars() = DM.releaseLocks(tran);
    // update transaction list
    transactionList.erase(tran);
    // update blocking graph
    TransactionManager::deleteEdge(tran);
    // execute next transaction from waiting queue
    TransactionManager::executeNextTransaction(tran);
    
}

// todo why no match declaration?
void TransactionManager::commit(Transaction* t, std::unordered_map<int, int> varValueList) {
    bool success = DM.commit(t, varValueList);
    if (success) {
        std::cout << t->name << " commits" << std::endl;

        Transaction transaction = transactionList.at(t->name);
        // release locks hold by the transaction
        transaction.getFreeVars() = DM.releaseLocks(t->name);
        // update transaction list
        std::string tran = t->name;
        transactionList.erase(tran);
        // update blocking graph
        TransactionManager::deleteEdge(tran);
        // execute next transaction from waiting queue
        TransactionManager::executeNextTransaction(tran);
        // release locks hold by transaction
        std::unordered_set<int> freeVars = DM.releaseLocks(tran);
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
        if (std::find(transaction.getSiteAccessedList().begin(), transaction.getSiteAccessedList().end(),
            siteID)!=transaction.getSiteAccessedList().end()) {
                transaction.toBeAborted = true;
        }
    }
}


void TransactionManager::recover(int siteID) {
    DM.recover(siteID);
}


void TransactionManager::enqueueROInstruction(std::string tran, std::string var) {
    instructionQueue.push_back({"RO", tran, var, 0});
}


void TransactionManager::enqueueReadInstruction(std::string tran, std::string var) {
    instructionQueue.push_back({"R", tran, var, 0});
}


void TransactionManager::enqueueWriteInstruction(std::string tran, std::string var, int val) {
    instructionQueue.push_back({"W", tran, var, val});
}


void TransactionManager::updateVarAccessedList(std::string tran, std::string var) {
    Transaction transaction = transactionList.at(tran);
    transaction.getVarAccessedList().push_back(var);
}
