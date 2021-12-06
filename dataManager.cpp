/** @file dataManager.cpp
    @author Tina Yuan thy258
    The file provides the constructor and function definitions for member functions 
    declared in dataManager.h.
 */

#include "dataManager.h"
#include <iostream>

using namespace std;

/* dataManager Constructor */
dataManager::dataManager()
{
    // initiate sites 1 - 10 and insert them into data member: sites
    for(int i = 1; i <= 10; ++i) {
        site* s = new site(i);
        this->sites.push_back(s);
    }

    for(int i = 1; i <= 20; ++i) {
        if(i % 2 == 0) {
            // even variables saved at all sites
            for(auto it = sites.begin(); it != sites.end(); ++it) {
                this->varSiteList[i].push_back(*it);
            }
        } else {
            this->varSiteList[i].push_back(sites[(i + 1) % 10 - 1]);
        }
    }
}

// dataManager member functions

/* prints out the commited values of all copies of all variables at all sites, 
   sorted per site with all values per site in ascending order by variable name. */
void dataManager::dump()
{
    cout << "Current DataManager Phase:" << endl;
    for(int i = 0; i < sites.size(); ++i) {
        site* s = sites[i];
        if(s->getIsRunning()) {
            //site i is up
            printf("Site %d: \n", s->getSiteId());
            map<int, variable> variables = s->getAllVariables();
            for(auto it = variables.begin(); it != variables.end(); ++it) {
                variable v = it->second;
                printf("Variable: %d | Value: %d\n", it->first, v.getValue());
            }
        }
    }
}

/* fail a running site */
void dataManager::fail(int site_id, int timestamp)
{
    site* s = sites[site_id - 1];
    if(s->getIsRunning()) {
        s->failSite(timestamp);
        printf("Site %d is down now\n", site_id);
    } else {
        printf("Site %d is already down\n", site_id);
    }
}

/* recover a failed site */
void dataManager::recover(int site_id)
{
    site* s = sites[site_id - 1];
    if(s->getIsRunning()) {
        printf("Site %d is already running\n", site_id);
    } else {
        s->recoverSite();
        printf("Site %d is running now\n", site_id);
    }
}

/* determines whether a transaction t can get the requested lock
   when attempting to read from the database
   if can get read locks then return list of site numbers,
   otherwise return blockers */
string dataManager::read(transaction* t, int var_id)
{
    //read lock variables that can be read locked
    //(lock was previously free or read)
    //and lock them
    //use canReadVar to verify if can read or not
    //update site lock table
    //return string DOWN, CONFLICT, value as a string
}

/* determines whether a transaction t can get the requested lock
   when attempting to write to the database
   if can get write locks then return list of site numbers,
   otherwise return blockers */
string dataManager::write(transaction* t, int var_id)
{
    //write lock variables that can be write locked
    //(lock was previously free)
    //and lock them
    //update site lock table
    //site s -> lockTable displays that var_id is free then i will write lock
    //return string DOWN, CONFLICT, variable_id as a string
    //return conflict if can only get locks to some sites but not all
}

/* commit a transaction */
void dataManager::commit(transaction* t)
{
    if(variableValueMap.size() == 0) {
        printf("Transaction %d has nothing to commit.\n", t.id);
    } else {
        // commit all the values in the map
        for(auto it = variableValueMap.begin(); it != variableValueMap.end(); ++it) {
            int var_id = it->first;
            int var_value = it->second;
            this->writeValueToSite(var_id, var_value);
            printf("Transaction %d changed variable %d's value to %d\n", t.id, var_id, var_value);
        }
    }
    releaseLocks(t);
}

/* writes the value to the variable that are stored in running
   sites. Function called upon commit */
void dataManager::writeValueToSite(transaction* t, int var_id, int value)
{
    vector<site*> sites = this->varSiteList[var_id];
    for(auto s : sites) {
        //check that transaction t has write lock on variable in site s's lockTable
        if(s->getIsRunning() ) {
            s->setVariableValue(var_id, value);
        }
    }
}

void dataManager::printAffectedSites();

/* release locks after transaction commits or aborts.
   return list of variables that become free after locks are released */
unordered_set<int> dataManager::releaseLocks(transaction* t)
{
    unordered_set<int> freeVariables;
    for(auto var_id : t->LIST_OF_VARIABLES_LOCKED_BY_T) {
        vector<site*> sites = this->varSiteList[var_id];
        bool isFree = true;
        for(auto s : sites) {
            if(s->getIsRunning()) {
                s->unlockVariable(var_id, t);
                if(!s->isVariableFree(var_id)) {
                    //variable should now be free at all sites it is stored at
                    isFree = false;
                }
            }
        }
        if(isFree) freeVariables.insert(var_id);
    }
    return freeVariables;
}