/** @file dataManager.h
    @author Tina Yuan thy258
    The file defines the dataManager structure and provides function declarations and definitions
    for simple getters and setters and function declarations for related functions.
 */

#ifndef _DATAMANAGER_H
#define _DATAMANAGER_H

#include "lockDetails.h"
#include "variable.h"
#include "site.h"
#include "Transaction.h"
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>

using namespace std;

class dataManager {
    /* list of all 10 sites */
    vector<site*> sites;

    /* key is the variable id, value is all the sites the variable is saved at */
    unordered_map<int, vector<site* >> varSiteList;

public:
    /* constructor */
    dataManager();
    
    /* returns all sites managed by dataManager */
    vector<site* > getSites() { return sites; }

    /* returns map of key being variable id and values being the sites the variable
        is saved at */
    unordered_map<int, vector<site* >> getVarSiteList() { return varSiteList; }
    
    /* prints out the commited values of all copies of all variables at all sites,
        sorted per site with all values per site in ascending order by variable name. */
    void dump();
    
    /* fail a running site and record its failed time as timestamp */
    void fail(int site_id, int timestamp);
    
    /* recover a failed site */
    void recover(int site_id);

    void generateVarValCache(Transaction* t);

    /* puts read lock on variable if it can be read locked
       (aka lock was previously free or read)
       return DOWN if all sites that store the variable are down
       return CONFLICT if one site has a write lock on the variable
       return the variable's value as a string if can read lock all instances of the variable */
    string read(Transaction* t, int var_id);

    /* determines whether a transaction t can get the requested lock
       when attempting to write to the database.
       If can get write locks then return list of site numbers,
       otherwise return blockers */
    string write(Transaction* t, int var_id);

    /* commit a transaction
        2nd param is MAP_OF_VARIABLE_IDX_AND_VALUES_THAT_SHOULD_BE_UPDATED_TO_EACH_VARIABLE */
    pair<bool, unordered_set<int>> commit(Transaction* t);

    bool checkValidReadWrite(Transaction* t, int var_id, unordered_set<int>& sites_to_verify);
    
    /* writes the value to the variable that are stored in running sites.
       Function called upon commit */
    void writeValueToSite(Transaction* t, int var_id, int value);

    /* prints list of sites affected by transaction t */
    void printAffectedSites(vector<int>& affected_sites);

    /* release locks after transaction commits or aborts.
       return list of variable id's of variables that become free after locks are released */
    unordered_set<int> releaseLocks(Transaction* t);
};

#endif