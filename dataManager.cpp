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
    for(int i = 0; i < sites.size(); ++i) {
        site* s = this->sites[i];
        printf("site %d - ", s->getSiteId());
        map<int, variable*> variables = s->getAllVariables();
        for(auto it = variables.begin(); it != variables.end(); ++it) {
            variable* v = it->second;
            if(it == variables.begin()) {
                printf("%s: %d", v->getName().c_str(), v->getValue());
            } else {
                printf(", %s: %d", v->getName().c_str(), v->getValue());
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
                if(t->varValCache.count(var_id)) {
                    //already cached variable value using variable
                    //stored at some other valid site
                    continue;
                }
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
    //t is not RO transaction
    vector<site*> sites = this->varSiteList[var_id];
    unordered_set<site*> read_from;
    unordered_set<site*> wait_from;
    bool conflict = false;
    string conflict_transaction;
    int num_sites = sites.size();
    int down_sites = 0;
    int invalid_read_sites = 0;
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
        if(conflict) {
            //another transaction holds a write lock on variable at site s so conflict
            s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            continue;
        }
        if(s->getLockType(var_id) != 2) {
            //no write lock on variable so can read variable at site s
            read_from.insert(s);
        } else {
            //variable is write-locked
            if(s->getLockOwners(var_id).count(t->name)) {
                //write lock held by transaction t itself so can still read the value
                return to_string(s->getVariable(var_id)->getValue());
            }
            //write lock held by transaction that is not t so lock conflict
            //and t has to wait
            conflict_transaction = *(s->getLockOwners(var_id).begin());
            s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            conflict = true;
        }
    }
    if(conflict) return conflict_transaction;
    
    if(down_sites == num_sites) {
        return "DOWN";
    } else if (down_sites + invalid_read_sites == num_sites) {
        for(auto s : wait_from) {
            if(!s->getIsRunning()) continue;
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
        //add lock to transaction's successfully obtained lock list
        //list will have var_id and site_num
        t->ownedLocks[var_id].insert(s->getSiteId());
        s->lockVariable(var_id, t->name, 1);
    }
    return to_string(s->getVariable(var_id)->getValue());
}

/* determines whether a transaction t can get the requested lock
   when attempting to write to the database
   return DOWN if all sites that store the variable are down
   return CONFLICT if one site has a read or write lock on the variable
   return the variable's id as a string if can write lock all available copies
   of the variable */
string dataManager::write(Transaction* t, int var_id, int value)
{
    vector<site*> sites = this->varSiteList[var_id];
    unordered_set<site*> write_to;
    unordered_set<site*> wait_from;
    bool conflict = false;
    string conflict_transaction;
    int num_sites = sites.size();
    int down_sites = 0;
    for(auto s : sites) {
        //increment down_sites if site s is down
        if(!s->getIsRunning()) {
            wait_from.insert(s);
            down_sites++;
            continue;
        }
        //if already determined that there is lock conflict then just add t
        //to waiting queue
        if(conflict) {
            //cout << "adding " << t->name << " to site " << s->getSiteId() << " waitQ for var " << var_id << endl;
            s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            continue;
        }
        //if lock wasn't previously free then can't write to any of the site
        //and t has to wait
        if(!s->isVariableFree(var_id)) {
            /*
            cout << "at site " << s->getSiteId() << ", var " << var_id << " has lock type " << s->getLockType(var_id);
            for(auto lockOwner : s->getLockOwners(var_id)) {
                cout << " by lockOwner: " << lockOwner << endl;
            }
            */
            if(s->getLockOwners(var_id).size() == 1 && s->getLockOwners(var_id).count(t->name)) {
                //if previous read or write lock held by transaction t itself and no transaction waiting
                //in waiting queue then can still write to the variable
                if(s->getLockTable()[var_id]->getWaitingQueue().empty() ||
                (s->getLockTable()[var_id]->getTransactionFromWaitingQueue()->name == t->name)) {
                    write_to.insert(s);
                    continue;
                } else {
                    conflict_transaction = s->getLockTable()[var_id]->getTransactionFromWaitingQueue()->name;
                    conflict = true;
                    continue;
                }
            }
            conflict_transaction = *(s->getLockOwners(var_id).begin());
            //cout << "adding " << t->name << " to site " << s->getSiteId() << " waitQ for var " << var_id << endl;
            s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
            conflict = true;
            continue;
        }
        //can obtain write lock at site s
        write_to.insert(s);
    }
    if(conflict) return conflict_transaction;
    //if all sites that store the variable are down then didn't write to any site
    if(num_sites == down_sites) {
        for(auto s : wait_from) {
            s->getLockTable()[var_id]->addTransactionToWaitingQueue(t);
        }
        return "DOWN";
    }
    //can obtain write locks to all sites that are up
    for(auto s : write_to) {
        //add lock to transaction's successfully obtained lock list
        //list will have var_id and site_num
        //cout << "owned" << s->getSiteId() << endl;
        t->ownedLocks[var_id].insert(s->getSiteId());
        s->lockVariable(var_id, t->name, 2);
    }
    /*
    for(auto it = t->ownedLocks.begin(); it != t->ownedLocks.end(); ++it) {
        cout << it->first << ": ";
        for(auto se : it->second) {
            cout << se << ", ";
        }
        cout << endl;
    }
    */
    t->varValueList[var_id] = value;
    return to_string(value);
}

/* commit a transaction. Returns true if successfully commit and false otherwise */
pair<bool, unordered_set<int>> dataManager::commit(Transaction* t)
{
    bool commit_success = true;
    //check that transaction can still read and write to all variables it obtained
    //locks for
    for(auto it = t->ownedLocks.begin(); it != t->ownedLocks.end(); ++it) {
        int var_id = it->first;
        unordered_set<int> sites_to_verify = it->second;
        if(!this->checkValidReadWrite(t, var_id, sites_to_verify)) commit_success = false;
    }
    //if variables are still valid write, commit all the values in the map
    if(commit_success) {
        for(auto it = t->varValueList.begin(); it != t->varValueList.end(); ++it) {
            int var_id = it->first;
            int var_value = it->second;
            this->writeValueToSite(t, var_id, var_value);
            printf("Transaction %s changed variable %d's value to %d\n", t->name.c_str(), var_id, var_value);
        }
    }
    //release all locks held by t since t is ending
    unordered_set<int> freeVars = releaseLocks(t);
    return {commit_success, freeVars};
}

bool dataManager::checkValidReadWrite(Transaction* t, int var_id, unordered_set<int>& sites_to_verify) {
    //sites_to_verify are sites that transaction t obtained locks for when that particular
    //read/write operation was processed
    for(auto site_id : sites_to_verify) {
        //cout << "site: " << site_id << endl;
        site* s = this->sites[site_id - 1];
        if(s->getIsRunning()) {
            if(!s->getLockOwners(var_id).count(t->name)) {
                //if transaction t doesn't own a lock for var_id at site s
                //and should've owned that lock then transaction fails
                //this'll happen if t obtained lock at site s and then
                //site s failed and recovered before t commits
                //t would then lose the lock it initially obtained on s
                return false;
            }
        } else {
            //transaction t wants to acces a site that failed so transaction fails
            return false;
        }
    }
    return true;
}

/* writes the value to the variable that are stored in running
   sites. Function called upon commit
   Returns true if successfully wrote to sites and false otherwise */
void dataManager::writeValueToSite(Transaction* t, int var_id, int value)
{
    vector<site*> sites = this->varSiteList[var_id];
    unordered_set<int> affected_sites = t->ownedLocks[var_id];
    //cout << "about to go thru affected_sites of " << var_id << endl;
    //if(t->ownedLocks[var_id].size() == 0) cout << "affected_sites=0" << endl;
    for(auto it = affected_sites.begin(); it != affected_sites.end(); ++it) {
        int site_id = *it;
        site* s = this->sites[site_id - 1];
        if(s->getLockType(var_id) == 2) {
            s->setVariableValue(var_id, value);
            s->setCanReadVar(var_id, true);
        }
    }
    vector<int> to_print_sites(affected_sites.begin(), affected_sites.end());
    /*
    vector<int> affected_sites;
    for(auto s : sites) {
        if(s->getIsRunning()) {
            if(s->getLockOwners(var_id).count(t) && s->getLockType(var_id) == 2) {
                //if transaction holds write lock on var_id at site s
                s->setVariableValue(var_id, value);
                s->setCanReadVar(var_id, true);
                affected_sites.push_back(s->getSiteId());
            }
        }
    }
    */
    printAffectedSites(to_print_sites);
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
    for(auto it = t->ownedLocks.begin(); it != t->ownedLocks.end(); ++it) {
        int var_id = it->first;
        vector<site*> sites = this->varSiteList[var_id];
        bool isFree = true;
        for(auto s : sites) {
            if(s->getIsRunning()) {
                s->unlockVariable(var_id, t->name);
                //cout << t->name << " unlocked " << var_id << " at site " << s->getSiteId() << " so lock type is now " << s->getLockType(var_id) << endl;
                if(!s->isVariableFree(var_id)) {
                    //variable should now be free at all sites it is stored at
                    isFree = false;
                }
                if((s->getLockOwners(var_id).size() == 1) && !s->getLockTable()[var_id]->getWaitingQueue().empty() && 
                    (s->getLockTable()[var_id]->getTransactionFromWaitingQueue()->name == *(s->getLockOwners(var_id).begin()))) {
                        isFree = true;
                    }
            }
        }
        if(isFree) {
            freeVariables.insert(var_id);
            t->freeVars.insert(var_id);
        }
    }
    return freeVariables;
}