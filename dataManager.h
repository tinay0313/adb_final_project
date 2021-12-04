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
};

#endif