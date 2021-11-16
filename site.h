/** @file site.h
    @author Tina Yuan thy258
    The file defines the site structure and provides function declarations for
    related functions.
 */

#ifndef _SITE_H_
#define _SITE_H_

#include "lockDetails.h"
#include <unordered_map>
#include <vector>
#include "variable.h"

class site {
	/* site id */
	int id;
	bool isRunning;
	bool isRecovered;
	/* key is variable, value is commit time of this version of the variable */
	unordered_map<variable, int> variableList;
	/* will record the timestamp upon site failure */
	vector<int> failedTimes;

public:
	/* fail the site and clear the lock table. Will add timestamp to failedTimes vector */
	void failSite(int timestamp);
	/*recover the site and initialize lock table */
	void recoverSite();
};

#endif