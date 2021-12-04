/** @file site.h
    @author Tina Yuan thy258
    The file defines the site structure and provides function declarations and definitions
    for simple getters and setters and function declarations for related functions.
 */

#ifndef _SITE_H_
#define _SITE_H_

#include "lockDetails.h"
#include "variable.h"
#include <unordered_map>
#include <vector>

using namespace std;

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
	/* returns site id */
	int getId() { return id; }

	/* returns isRunning status */
	bool getIsRunning() { return isRunning; }

	/* returns isRecovered status */
	bool getIsRecovered() { return isRecovered; }

	/* sets isRunning to true, default value is true */
	void setIsRunningTrue() { isRunning = true; }

	/* sets isRecovered to true, default value is false */
	void setIsRecoveredTrue() { isRecovered = true; }

	/* sets isRunning to false, default value is true */
	void setIsRunningFalse() { isRunning = false; }

	/* sets isRecovered to false, default value is false */
	void setIsRecoveredFalse() { isRecovered = false; }

	/* fails the site and clear the lock table. Adds timestamp to failedTimes vector */
	void failSite(int timestamp);

	/*recovers the site and initializes lock table */
	void recoverSite();
};

#endif