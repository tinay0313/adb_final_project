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
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>

using namespace std;

class dataManager {
    /* list of all 10 sites */
    vector<site* > sites;

    /* key is the variable id, value is all the sites the variable is saved at */
    unordered_map<int, vector<site* >> varSiteList;

public:
    /* returns all sites managed by dataManager */
    vector<site* > getSites() { return sites; }

    /* returns map of key being variable id and values being the sites the variable
        is saved at */
    unordered_map<int, vector<site* >> getVarSiteList() { return varSiteList; }
    
    /* prints out the commited values of all copies of all variables at all sites,
        sorted per site with all values per site in ascending order by variable name. */
    void dump();
    
    /* fail a running site */
    void fail(int site_id);
    
    /* recover a failed site */
    void recover(int site_id);

    /* determines whether a transaction t can get the requested lock
       when attempting to read from the database.
       If can get read locks then return list of site numbers,
       otherwise return blockers */
    string read(transaction* t, int var_id);

    /* determines whether a transaction t can get the requested lock
       when attempting to write to the database.
       If can get write locks then return list of site numbers,
       otherwise return blockers */
    string write(transaction* t, int var_id);

    /* commit a transaction
        2nd param is MAP_OF_VARIABLE_IDX_AND_VALUES_THAT_SHOULD_BE_UPDATED_TO_EACH_VARIABLE */
    void commit(transaction* t);

    /* writes the value to the variable that are stored in running sites.
       Function called upon commit */
    void writeValueToSite(int var_id, int value);

    void printAffectedSites();

    /* release locks after transaction commits or aborts.
       return list of variable id's of variables that become free after locks are released */
    unordered_set<int> releaseLocks(transaction* t);
};

#endif