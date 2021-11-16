/** @file lockDetails.h
    @author Tina Yuan thy258
    The file defines the lockDetails structure and provides function declarations for
    related functions.
 */

#ifndef _LOCKDETAILS_H_
#define _LOCKDETAILS_H_

#include <vector>
#include <queue>
#include "transaction.h"

class lockDetails
{
    /* 0 is free, 1 is read lock, 2 is write lock */
    int type;
    vector<transaction> owners;
    queue<transaction> waitingQueue;

public:
    /* returns lock type */
    int getType();

    /* returns owners */
    vector<transaction> getOwners();

    /* returns waiting queue */
    queue<transaction> getWaitingQueue();

    /* set lock type to free (0) */
    void freeLock();

    /* set lock type to read (1) */
    void readLock();

    /* set lock type to write (2) */
    void writeLock();

    /* add a transaction as an owner to the owners vector */
    void addOwner(transaction t);

    /* remove a transaction as an owner from the owners vector */
    void removeOwner(transaction t);

    /* add a transaction to waiting queue */
    void addTransactionToWaitingQueue(transaction t);

    /* remove a transaction from waiting queue */
    void removeTransactionFromWaitingQueue(transaction t);
}

#endif