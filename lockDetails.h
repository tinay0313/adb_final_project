/** @file lockDetails.h
    @author Tina Yuan thy258
    The file defines the lockDetails structure and provides function declarations for
    related functions.
 */

#ifndef _LOCKDETAILS_H_
#define _LOCKDETAILS_H_

#include <vector>
#include <queue>

class lockDetails
{
    /* 0 is free, 1 is read lock, 2 is write lock */
    int type;
    vector<Transaction> owners;
    queue<Transaction> waitingQueue;
}

#endif