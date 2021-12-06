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
        /* saves all the the variables the transaction wants to access */
        std::vector<Variable> variableList;
        /* saves the sites that are affected by the transaction’s write */
        std::vector<Site>  siteList;
        /* saves the values of the variables the transaction successfully read upon transaction
        Begin Key is the variable, value is the value of the variable transaction read upon begin */ 
        // std::map<Variable, int> varValCache;

        /* print the sites that are affected by the transaction’s write */ 
        void printAffectedSites();
        /* prints message every time transaction waits because of a lock conflict */
        void printLockConflict();
        /* prints message every time transaction waits because of a down site */
        void printDownSite();
};


#endif
