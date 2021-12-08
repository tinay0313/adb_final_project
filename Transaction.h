/** @file Transaction.h
    @author Ann Tsai mt4050
 */

#ifndef TRANSACTION
#define TRANSACTION

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <iostream>

class Transaction {
    private:

        /* for readOnly transaction */
        std::unordered_map<int, int> varValCache; // varID : val
        /* variable has accessed by the transaction before  */
        std::vector<int> varAccessedList;
        /* list of variables that become free after locks hold by the transaction are released */
        std::unordered_set<int> freeVars;

    public:
        Transaction(std::string name, bool readOnly, int startTime);


        /* transaction name */
        std::string name;
        /* true if transaction is read-only, false if transaction is read-write */
        bool isReadOnly;
        /* transaction start time */
        int startTime;
        /* transaction should be aborted or not */
        bool toBeAborted;

        /* saves all the the sites the transaction had accessed */
        std::vector<int> siteAccessedList;
        /* save the value the transaction want to write to the variable */
        std::unordered_map<int, int> varValueList;
        /* key is var_id, value is all site(s) transaction successfully obtained locks for var_id */
        std::unordered_map<int, std::unordered_set<int>> ownedLocks;


        /* get varValCache*/
        std::unordered_map<int, int> getVarValCache() { return varValCache; }

        /* get varAccessedList*/
        std::vector<int> getVarAccessedList() { return varAccessedList; }
        /* get siteAccessed */
        std::vector<int> getSiteAccessedList() { return siteAccessedList; }
        /* get varValueList*/
        std::unordered_map<int, int> getVarValueList() { return varValueList; }
        /* get ownedLocks*/
        std::unordered_map<int, std::unordered_set<int>> getOwnedLocks() { return ownedLocks; }
        /* get freeVars*/
        std::unordered_set<int> getFreeVars() { return freeVars; }
        

        /* prints message every time transaction waits because of a lock conflict */
        void printLockConflict(std::string tran);
        /* prints message every time transaction waits because of a down site */
        void printDownSite(std::string tran);
        /* prints message every time transaction waits because of a invalid var */
        void printInvalid(std::string tran);
        /* prints message every time transaction waits because of a down site & invalid var */
        void printDownInvalid(std::string tran);
};


#endif
