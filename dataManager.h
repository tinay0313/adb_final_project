/** @file dataManager.h
    @author Tina Yuan thy258
    The file defines the site structure and provides function declarations for
    related functions.
 */

#ifndef _DATAMANAGER_H
#define _DATAMANAGER_H

#include "lockDetails.h"
#include <unordered_map>
#include <vector>
#include "variable.h"
#include "site.h"

class dataManager {
    /* list of all 10 sites */
    vector<site> sites;

    /* key is the variable, value is all the sites the variable is saved at */
    unordered_map<variable, vector<site>> varSiteList;

public:
    /** prints out the commited values of all copies of all variables at all sites,
      * sorted per site with all values per site in ascending order by variable name.
      */
    void dump();
    /* fail a running site */
    void fail(site site, int timestamp);
    /* recover a failed site */
    void recover(site site);
};

#endif