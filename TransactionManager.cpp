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

//todo RETURN VALUE?
bool TransactionManager::detectDeadlock() {

    std::string youngest;
    int youngestStartTime = INT_MAX;

    for (auto const& blocking : blockingGraph) {
        std::unordered_set<std::string> visited;
        std::string start = blocking.first;
        // dfs
        if (TransactionManager::helper(start, start, visited, blockingGraph)) {
            Transaction transaction = transactionList.at(start);
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
    }
}


// todo return value?
bool TransactionManager::helper(std::string start, std::string target, std::unordered_set<std::string> visited,
                                std::unordered_map<std::string, std::vector<std::string>> blockingGraph) {
    if (visited.size() != 0 && start == target) {
        return true;
    }
    for (auto const& transactionBlocked : blockingGraph.at(start)) {
        if (visited.find(transactionBlocked) == visited.end()) {
            visited.insert(transactionBlocked);
            if (TransactionManager::helper(transactionBlocked, target, visited, blockingGraph)) {
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
        auto it = std::find(instructionQueue.begin(), instructionQueue.end(), nextTran->name);
        Instruction nextIns = *it;
        //Instruction nextIns = instructionQueue.at(nextTran->name);

        if (nextTran == nullptr) {
            auto it = std::find(instructionQueue.begin(), instructionQueue.end(), nextIns);
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
            auto it = std::find(instructionQueue.begin(), instructionQueue.end(), nextIns);
            instructionQueue.erase(it);
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
    
    Transaction transaction = transactionList.at(tran);
    varID = std::stoi(var.substr(1));

    std::string res = DM.write(&transaction, varID);
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
            transaction.getSiteAccessedList().push_back(site);
        }
        isSuccessful = true;
    }
    // update varValList
    transaction.varValueList[varID] = val;
    //transaction.getVarValueList()[varID] = val;

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
    // update transaction list
    transactionList.erase(tran);
    // update blocking graph
    TransactionManager::deleteEdge(tran);
    // execute next transaction from waiting queue
    TransactionManager::executeNextTransaction(tran);
    
}

// todo why no match declaration?
void TransactionManager::commit(Transaction* t, std::unordered_map<int, int> varValueList) {
    pair<bool, unordered_set<int>> res = DM.commit(t);
    bool success = res.first;
    //save free vars?
    if (success) {
        std::cout << t->name << " commits" << std::endl;

        Transaction transaction = transactionList.at(t->name);
        //WHY RELEASE LOCKS TWICE???
        // release locks hold by the transaction
        //transaction.getFreeVars() = res.second;
        // update transaction list
        std::string tran = t->name;
        transactionList.erase(tran);
        // update blocking graph
        TransactionManager::deleteEdge(tran);
        // execute next transaction from waiting queue
        TransactionManager::executeNextTransaction(tran);
        // release locks hold by transaction
        //std::unordered_set<int> freeVars = DM.releaseLocks(tran);
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
    int varID = std::stoi(var.substr(1));
    transaction.getVarAccessedList().push_back(varID);
}
