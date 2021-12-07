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
    cout << "DataManager dump:" << endl;
    for(int i = 0; i < sites.size(); ++i) {
        site* s = sites[i];
        printf("site %d - ", s->getSiteId());
        map<int, variable*> variables = s->getAllVariables();
        for(auto it = variables.begin(); it != variables.end(); ++it) {
            variable* v = it->second;
            if(it == variables.begin()) {
                printf("%s: %d", v->getName(), v->getValue());
            } else {
                printf(", %s: %d", v->getName(), v->getValue());
            }
        }
        cout << endl;
    }
}

/* fail a running site and record its failed time as timestamp */
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

void dataManager::generateVarValCache(Transaction* t)
{
    for(auto s : this->sites) {
        if(s->getIsRunning()) {
            map<int, variable*> variables = s->getAllVariables();
            for(auto it = variables.begin(); it != variables.end(); ++it) {
                int var_id = it->first;
                if(s->isVariableValidForRead(var_id)) {
                    int value = variables[var_id]->getValue();
                    t->varValCache[var_id] = value;
                }
            }
        }
    }
}

/* puts read lock on variable if it can be read locked
   (aka lock was previously free or read)
   return DOWN if all sites that store the variable are down
   return CONFLICT if one site has a write lock on the variable
   return the variable's value as a string if can read an available copy
   of the variable */
string dataManager::read(Transaction* t, int var_id)
{
    if(t->isReadOnly()) {
        if(t->varValCache.empty()) {
            //cache all variable values at start of RO transaction t
            this->generateVarValCache(t);
            if(t->varValCache.count(var_id)) {
                return to_string(t->varValCache[var_id]);
            } else {
                //transaction should abort
                return "ABORT";
            }
        } else {
            if(t->varValCache.count(var_id)) {
                return to_string(t->varValCache[var_id]);
            } else {
                //transaction should abort
                return "ABORT";
            }
        }
    } else {
        //t is not RO transaction
        vector<site*> sites = this->varSiteList[var_id];
        unordered_set<site*> read_from;
        unordered_set<site*> wait_from;
        int num_sites = sites.size();
        int down_sites = 0;
        int invalid_read_sites = 0;
        int valid_read_sites = 0;
        int trans_start_time = t->startTime;
        for(auto s : sites) {
            //increment down_sites if site s is down
            if(!s->getIsRunning()) {
                wait_from.insert(s);
                down_sites++;
                continue;
            }
            //increment invalid_read_sites if variable not valid for read
            //at site s
            //a variable v at site s would be invalid read if site just recovered and
            //v is a replicated variable and no new writes have been commited to v yet
            //at site s
            if(!s->isVariableValidForRead(var_id)) {
                wait_from.insert(s);
                invalid_read_sites++;
                continue;
            }
            //at this point, a variable may or may not be replicated
            //but even if the variable is replicated, it is valid for read
            if(s->getLockType(var_id) != 2) {
                //if can read from a site, do we still need to read lock all sites that have this variable??
                //i think the answer is yes
                read_from.insert(s);
            } else {
                //variable is write-locked
                if(s->getLockOwners(var_id).count(t)) {
                    //write lock held by transaction t itself so can still read the value
                    return to_string(s->getVariable(var_id)->getValue());
                }
                //write lock held by transaction that is not t so lock conflict
                //and t has to wait
                s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
                return "CONFLICT";
            }
        }
        if(down_sites == num_sites) {
            for(auto s : wait_from) {
                s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            }
            return "DOWN";
        } else if (down_sites + invalid_read_sites == num_sites) {
            for(auto s : wait_from) {
                s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            }
            return "DOWN_INVALID";
        } else if (invalid_read_sites == num_sites) {
            for(auto s : wait_from) {
                s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            }
            return "INVALID";
        }
        //can obtain read locks at sites that are up and have valid, readable values
        site* s = *(read_from.begin());
        for(auto s : read_from) {
            s->lockVariable(var_id, t, 1);
        }
        printf("%s: %d", s->getVariable(var_id)->getName(), s->getVariable(var_id)->getValue());
        return to_string(s->getVariable(var_id)->getValue());
    }
}

/* determines whether a transaction t can get the requested lock
   when attempting to write to the database
   return DOWN if all sites that store the variable are down
   return CONFLICT if one site has a read or write lock on the variable
   return the variable's id as a string if can write lock all available copies
   of the variable */
string dataManager::write(Transaction* t, int var_id)
{
    vector<site*> sites = this->varSiteList[var_id];
    unordered_set<site*> write_to;
    unordered_set<site*> wait_from;
    int num_sites = sites.size();
    int down_sites = 0;
    int valid_write_sites = 0;
    for(auto s : sites) {
        //increment down_sites if site s is down
        if(!s->getIsRunning()) {
            wait_from.insert(s);
            down_sites++;
            continue;
        }
        //if lock wasn't previously free then can't write to any of the site
        //and t has to wait
        if(!s->isVariableFree(var_id)) {
            if(s->getLockOwners(var_id).size() == 1 && s->getLockOwners(var_id).count(t)) {
                //if previous read or write lock held by transaction t itself
                //then can still write to the variable
                write_to.insert(s);
                continue;
            }
            s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            return "CONFLICT";
        }
        //can obtain write lock at site s
        write_to.insert(s);
    }
    //if all sites that store the variable are down then didn't write to any site
    if(num_sites == down_sites) {
        for(auto s : wait_from) {
            s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
        }
        return "DOWN";
    }
    //can obtain write locks to all sites that are up
    for(auto s : write_to) {
        s->lockVariable(var_id, t, 2);
    }
    return to_string(var_id);
}

/* commit a transaction. Returns true if successfully commit and false otherwise */
bool dataManager::commit(Transaction* t)
{
    bool commit_success = true;
    //check that transaction can still write to all variables
    for(auto it = t->variableValueMap.begin(); it != t->variableValueMap.end(); ++it) {
        int var_id = it->first;
        if(!this->checkValidWrite(t, var_id)) commit_success = false;
    }
    //if all variables are still valid write, commit all the values in the map
    if(commit_success) {
        for(auto it = t->variableValueMap.begin(); it != t->variableValueMap.end(); ++it) {
            int var_id = it->first;
            int var_value = it->second;
            this->writeValueToSite(t, var_id, var_value);
            printf("Transaction %s changed variable %d's value to %d\n", t->name, var_id, var_value);
        }
    }
    //release all locks held by t since t is ending
    releaseLocks(t);
    return commit_success;
}

bool dataManager::checkValidWrite(Transaction* t, int var_id) {
    vector<site*> sites = this->varSiteList[var_id];
    int num_sites = sites.size();
    int down_sites = 0;
    for(auto s : sites) {
        if(s->getIsRunning()) {
            //if transaction t doesn't own a write lock for var_id, transaction failed
            if(!s->getLockOwners(var_id).count(t) || s->getLockType(var_id) != 2)
                return false;
        } else {
            //track if site is down
            down_sites++;
        }
    }
    //if all sites are down, transaction failed because it didn't write to any site
    if(down_sites == num_sites) return false;
    return true;
}

/* writes the value to the variable that are stored in running
   sites. Function called upon commit
   Returns true if successfully wrote to sites and false otherwise */
void dataManager::writeValueToSite(Transaction* t, int var_id, int value)
{
    vector<site*> sites = this->varSiteList[var_id];
    vector<int> affected_sites;
    for(auto s : sites) {
        if(s->getIsRunning()) {
            s->setVariableValue(var_id, value);
            s->setCanReadVar(var_id, true);
            affected_sites.push_back(s->getSiteId());
        }
    }
    printAffectedSites(affected_sites);
}

void dataManager::printAffectedSites(vector<int>& affected_sites)
{
    cout << "Sites ";
    for(int i = 0; i < affected_sites.size() - 1; ++i) {
        cout << affected_sites[i] << ", ";
    }
    cout << affected_sites.back() << " were affected by this transaction" << endl;
}

/* release locks after transaction commits or aborts.
   return list of variables that become free after locks are released */
unordered_set<int> dataManager::releaseLocks(Transaction* t)
{
    unordered_set<int> freeVariables;
    for(auto var_id : t->variableList) {
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