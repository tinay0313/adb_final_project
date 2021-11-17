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
}

/* fails the site and clear the lock table. Adds timestamp to failedTimes vector */
void site::failSite(int timestamp)
{
    failedTimes.push_back(timestamp);
    setIsRunningFalse;
    setIsRecoveredFalse();
}

/*recovers the site and initializes lock table */
void site::recoverSite()
{
    setIsRecoveredTrue();
    setIsRunningTrue();
}