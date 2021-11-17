/** @file lockDetails.cpp
    @author Tina Yuan thy258
    The file provides the function definitions for functions declared in lockDetails.h.
 */

#include <vector>
#include "lockDetails.h"
#include "transaction.h"
 
using namespace std;

class lockDetails
{
    /* 0 is free, 1 is read lock, 2 is write lock */
    int type;
    vector<transaction* > owners;
    vector<transaction* > waitingQueue;

public:
    /* returns lock type */
    int getType() {
        return this->type;
    }

    /* returns owners */
    vector<transaction* > getOwners() {
        return this->owners;
    }

    /* returns waiting queue */
    vector<transaction* > getWaitingQueue() {
        return this->waitingQueue;
    }

    /* set lock type to free (0) */
    void freeLock() {
        this->type = 0;
    }

    /* set lock type to read (1) */
    void readLock() {
        this->type = 1;
    }

    /* set lock type to write (2) */
    void writeLock() {
        this->type = 2;
    }

    /* add a transaction as an owner to the owners vector */
    void addOwner(transaction* t) {
        this->owners.push_back(t);
    }

    /* remove a transaction as an owner from the owners vector */
    void removeOwner(transaction* t) {
        this->owners.erase(t);
    }

    /* add a transaction to waiting queue */
    void addTransactionToWaitingQueue(transaction* t) {
        this->waitingQueue.push_back(t);
    }

    /* remove a transaction from waiting queue */
    void removeTransactionFromWaitingQueue(transaction* t) {
        this->waitingQueue.erase(t);
    }

    transaction* getTransactionFromWaitingQueue() {
        transaaction* t = this->waitingQueue.front();
        this->waitingQueue.erase(this->waitingQueue.erase(this->waitingQueue.begin()));
        return t;
    }
}