/** @file site.h
    @author Tina Yuan thy258
    The file defines the site structure and provides function declarations and definitions
    for simple getters and setters and function declarations for related functions.
 */

#ifndef _SITE_H_
#define _SITE_H_

#include "lockDetails.h"
#include "variable.h"
#include "Transaction.h"
#include <unordered_map>
#include <map>
#include <vector>

using namespace std;

class site {
	/* site id */
	int id;
	bool isRunning;
	bool isRecovered;
	int timestamp;
	/* key is variable id, value is variable */
	map<int, variable*> variableList;
	/* key is variable id, value is lock(s), where the locks keep track of the owners (of type transaction)
	   of the lock, the waiting queue, and the current lock type */
	unordered_map<int, lockDetails*> lockTable;
	/* key is variable id, value is whether or not variable can be read (boolean) */
	unordered_map<int, bool> canReadVar;
	/* will record the timestamp upon site failure */
	vector<int> failedTimes;

public:
	/* constructor */
	site(int id);

	/* returns site id */
	int getSiteId() { return id; }

	/* returns isRunning status */
	bool getIsRunning() { return isRunning; }

	/* returns isRecovered status */
	bool getIsRecovered() { return isRecovered; }

	/* returns timestamp */
	int getTimestamp() { return timestamp; }

	/* returns lockTable */
	unordered_map<int, lockDetails*> getLockTable() { return lockTable; }

	/* sets isRunning to true, default value is true */
	void setIsRunningTrue() { isRunning = true; }

	/* sets isRecovered to true, default value is false */
	void setIsRecoveredTrue() { isRecovered = true; }

	/* sets isRunning to false, default value is true */
	void setIsRunningFalse() { isRunning = false; }

	/* sets isRecovered to false, default value is false */
	void setIsRecoveredFalse() { isRecovered = false; }

	/* sets whether or not a variable can be read at this site */
	void setCanReadVar(int var_id, bool flag) { canReadVar[var_id] = flag; }

	/* returns all variables */
	map<int, variable*> getAllVariables() { return variableList; }

	/* return variable as identified by input parameter id */
	variable* getVariable(int id) { return variableList[id]; }

	/* return lock type of lock that is currently enforced on variable as identified by input parameter id */
	int getLockType(int id) { return lockTable[id]->getType(); }

	/* return owners of lock that is currently enforced on variable as identified by input parameter id */
	unordered_set<Transaction*> getLockOwners(int id) { return lockTable[id]->getOwners(); }

	/* returns true if variable identified by input parameter id is free and false otherwise */
	bool isVariableFree(int id) { return getLockType(id) == 0; }

	/* returns true if variable as identified by input parameter id is valid for read and false otherwise */
	bool isVariableValidForRead(int id) { return canReadVar[id]; }

	/* add a lock to the variable as identified by input parameter id */
	void lockVariable(int id, Transaction* t, int lockType) { lockTable[id]->addLock(t, lockType); }

	/* remove lock imposed on the variable as identified by input parameter id by transaction t */
	void unlockVariable(int id, Transaction* t) { lockTable[id]->removeLock(t); }

	/* set variable value to val */
	void setVariableValue (int id, int val) { variableList[id]->setValue(val); }

	/* fails the site and clear the lock table. Adds timestamp to failedTimes vector */
	void failSite(int timestamp);

	/*recovers the site and initializes lock table */
	void recoverSite();

	/* return the most recent time the site failed */
	int getMostRecentFailTime() { return failedTimes.back(); }
};

#endif