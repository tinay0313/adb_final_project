/** @file site.cpp
    @author Tina Yuan thy258
    The file provides the constructor and function definitions for member functions 
    declared in site.h.
 */

#include "site.h"

using namespace std;

/* site Constructor */
site::site(int id)
{
    this->id = id;
    setIsRunningTrue();
    setIsRecoveredFalse();
    this->failedTimes.push_back(-1);
    this->timestamp = 0;
    // create variables stored in site
    for(int i = 1; i <= 20; ++i) {
        variable* v = new variable(i, 10 * i);
        if(i % 2 == 0) {
            // even variable stored at all sites
            v->setIsReplicatedTrue();
            this->variableList[i] = v;
            lockDetails* lock = new lockDetails();
            this->lockTable[i] = lock;
            this->canReadVar[i] = true;
        } else if ((i + 1) % 10 == id) {
            this->variableList[i] = v;
            lockDetails* lock = new lockDetails();
            this->lockTable[i] = lock;
            this->canReadVar[i] = true;
        }
    }
}

// site member functions
/* fails the site and clear the lock table. Adds timestamp to failedTimes vector */
void site::failSite(int timestamp)
{
    failedTimes.push_back(timestamp);
    setIsRunningFalse();
    setIsRecoveredFalse();
    this->lockTable.clear();
    for(auto it = this->variableList.begin(); it != this->variableList.end(); ++it) {
        this->setCanReadVar(it->first, false);
    }
}

/*recovers the site and initializes lock table */
void site::recoverSite()
{
    setIsRecoveredTrue();
    setIsRunningTrue();
    for(auto it = this->variableList.begin(); it != this->variableList.end(); ++it) {
        int var_id = it->first;
        lockDetails* lock = new lockDetails();
        this->lockTable[var_id] = lock;
    }
    for(auto it = this->canReadVar.begin(); it != this->canReadVar.end(); ++it) {
        int var_id = it->first;
        bool var_valid = it->second;
        if(this->variableList[var_id]->getIsReplicated()) {
            this->setCanReadVar(var_id, false);
        } else {
            this->setCanReadVar(var_id, true);
        }
    }
}