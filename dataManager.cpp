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
void dump()
{
    cout << "Current DataManager Phase:" << endl;
    for(int i = 0; i < sites.size(); ++i) {
        site* s = sites[i];
        if(s->getIsRunning()) {
            //site i is up
            cout << "Site " << s->getSiteId() << ": " << endl;
            map<int, variable> variables = s->getAllVariables();
            for(auto it = variables.begin(); it != variables.end(); ++it) {
                variable v = it->second;
                cout << "Variable: " << it->first << " Value: " << v.getValue() << endl;
            }
        }
    }
}

/* fail a running site */
void fail(int site_id)
{
    site* s = sites[site_id - 1];
    if(s->getIsRunning()) {
        s->failSite();
        cout << "Site " << site_id << " is down now" << endl;
    } else {
        cout << "Site " << site_id << " is already down" << endl;
    }
}

/* recover a failed site */
void recover(int site_id)
{
    site* s = sites[site_id - 1];
    if(s->getIsRunning()) {
        cout << "Site " << site_id << " is already running" << endl;
    } else {
        s->recoverSite();
        cout << "Site " << site_id << " is running now" << endl;
    }
}