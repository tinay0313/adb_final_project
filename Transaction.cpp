/** @file Transaction.cpp
    @author Ann Tsai mt4050
 */

#include "Transaction.h"
#include "DataManager.h"

Transaction::Transaction(std::string name, bool isReadOnly, int startTime):
    name(name),
    isReadOnly(isReadOnly),
    startTime(startTime),
    toBeAborted(false),
    siteAccessedList(),
    variableList(),
    siteList()
    // varValCache()
{};

void Transaction::printAffectedSites() {
    for(auto site: siteList) {
        std::cout << "Affected Sites: " << site.id << std::endl;
    }
}



void Transaction::printLockConflict() { // todo lock
    // std::cout << "Transaction: " << name << " waits because of a lock conflict." << name << std::endl;
}

void Transaction::printDownSite() {  // todo dm
    // std::cout << "Transaction: " << name << " waits because of a down site." << name << std::endl;
}
