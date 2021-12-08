/** @file lockDetails.h
    @author Tina Yuan thy258
    The file defines the lockDetails structure and provides function declarations and definitions
    for simple getters and setters and function declarations for related functions.
 */

#ifndef _LOCKDETAILS_H_
#define _LOCKDETAILS_H_

#include <vector>
#include <unordered_set>
#include "Transaction.h"

using namespace std;

class lockDetails
{
    /* 0 is free, 1 is read lock, 2 is write lock */
    int type;
    unordered_set<string> owners;
    vector<Transaction *> waitingQueue;

public:
    /* constructor */
    lockDetails();
    
    /* returns lock type */
    int getType() { return type; }

    /* returns owners */
    unordered_set<string> getOwners() { return owners; }

    /* returns waiting queue */
    vector<Transaction*> getWaitingQueue() { return waitingQueue; }

    /* returns true if lock is free (type == 0) and false otherwise */
    bool isFree() { return type == 0; }

    /* adds a lock indicated by lockType to the variable and indicate that transaction
       t is the owner */
    void addLock(string t, int lockType);

    /* removes a lock to the variable held by transaction t */
    void removeLock(string t);

    /* set lock type to free (0) */
    void freeLock() { type = 0; }

    /* set lock type to read (1) */
    void readLock() { type = 1; }

    /* set lock type to write (2) */
    void writeLock() { type = 2; }

    /* add a transaction to waiting queue */
    void addTransactionToWaitingQueue(Transaction* t);

    /* remove a transaction from waiting queue */
    void removeTransactionFromWaitingQueue(Transaction* t);

    /* gets the first transaction queued at the waiting queue */
    Transaction* getTransactionFromWaitingQueue() { return waitingQueue.front(); }

    /* removes the first transaction queued at the waiting queue */
    void removeFirstTransactionFromWaitingQueue() { waitingQueue.erase(waitingQueue.begin()); }
};

#endif