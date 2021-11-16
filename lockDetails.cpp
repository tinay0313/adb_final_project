#include <vector>
#include <queue>
#include "lockDetails.h"
 
using namespace std;

class lockDetails
{
    /* 0 is free, 1 is read lock, 2 is write lock */
    int type;
    vector<Transaction> owners;
    queue<Transaction> waitingQueue;
}