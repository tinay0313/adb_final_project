/** @file Transaction.h
    @author Ann Tsai mt4050
 */

#ifndef TRANSACTION
#define TRANSACTION

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "dataManager.h"

class Transaction {

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

        /* get varValueList*/
        std::vector<int> getSiteAccessedList();
        /* get varValueList*/
        std::unordered_map<int, int> getVarValueList();
        /* prints message every time transaction waits because of a lock conflict */
        void printLockConflict(std::string tran);
        /* prints message every time transaction waits because of a down site */
        void printDownSite(std::string tran);
};


#endif
