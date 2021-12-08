/** @file lockDetails.cpp
    @author Tina Yuan thy258
    The file provides the constructor and function definitions for member functions 
    declared in lockDetails.h.
 */

#include "lockDetails.h"
 
using namespace std;

/* lockDetails Constructor */
lockDetails::lockDetails()
{
    lockDetails::freeLock();
}

//lockDetails member functions
/* adds a lock indicated by lockType to the variable and indicate that transaction
   trans is the owner */
void lockDetails::addLock(string t, int lockType)
{
    this->owners.insert(t);
    this->type = lockType;
    //cout << "added lock: " << t << " of type " << lockType << endl;
}

/* removes a lock to the variable held by transaction t */
void lockDetails::removeLock(string t)
{
    auto it = this->owners.find(t);
    if(it != this->owners.end()) {
        //transaction is one of the lock owners
        this->owners.erase(it);
    }
    if(owners.size() == 0) {
        //free the lock if no one else holds a lock on the variable
        this->type = 0;
    }
}

/* add a transaction to waiting queue */
void lockDetails::addTransactionToWaitingQueue(Transaction* t)
{
    this->waitingQueue.push_back(t);
}

/* gets the first transaction queued at the waiting queue */
Transaction* lockDetails::getTransactionFromWaitingQueue()
{
    if(this->waitingQueue.empty()) {
        return nullptr;
    } else {
        return this->waitingQueue.front();
    }
}

/* remove a transaction from waiting queue */
void lockDetails::removeTransactionFromWaitingQueue(Transaction* t)
{
    int i = 0;
    for(; i < this->waitingQueue.size(); ++i) {
        if(this->waitingQueue[i]->name == t->name) break;
    }
    this->waitingQueue.erase(this->waitingQueue.begin() + i);
}