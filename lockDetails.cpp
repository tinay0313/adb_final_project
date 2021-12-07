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
void lockDetails::addLock(Transaction* t, int lockType)
{
    this->owners.insert(t);
    this->type = lockType;
}

/* removes a lock to the variable held by transaction t */
void lockDetails::removeLock(Transaction* t)
{
    auto it = this->owners.find(t);
    if(it != this->owners.end()) {
        //transaction is in owners set
        this->owners.erase(it);
    }
    if(owners.size() == 0) {
        //free the lock
        this->type = 0;
    }
}

/* add a transaction as an owner to the owners vector */
void lockDetails::addOwner(Transaction* t)
{
    this->owners.insert(t);
}

/* remove a transaction as an owner from the owners vector */
void lockDetails::removeOwner(Transaction* t)
{
    this->owners.erase(t);
}

/* add a transaction to waiting queue */
void lockDetails::addTransactionToWaitingQueue(Transaction* t)
{
    waitingQueue.push_back(t);
}

/* remove a transaction from waiting queue */
void lockDetails::removeTransactionFromWaitingQueue(Transaction* t)
{
    int i = 0;
    for(; i < this->waitingQueue.size(); ++i) {
        if(this->waitingQueue[i]->name == t->name) break;
    }
    waitingQueue.erase(this->waitingQueue.begin() + i);
}

/* gets the first transaction queued at the waiting queue 
    and pops it from the queue */
Transaction* lockDetails::getTransactionFromWaitingQueue()
{
    Transaction* t = waitingQueue.front();
    return t;
}

void lockDetails::removeFirstTransactionFromWaitingQueue()
{
    waitingQueue.erase(waitingQueue.begin());
}