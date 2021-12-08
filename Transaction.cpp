/** @file Transaction.cpp
    @author Ann Tsai mt4050
 */

#include "Transaction.h"

Transaction::Transaction(std::string name, bool isReadOnly, int startTime):
    name(name),
    isReadOnly(isReadOnly),
    startTime(startTime),
    toBeAborted(false),
    varValCache(),
    varAccessedList(),
    siteAccessedList(),
    varValueList()
{};




void Transaction::printLockConflict(std::string tran) {
    std::cout << "Transaction: " << tran << " waits because of a lock conflict." << std::endl;
}

void Transaction::printDownSite(std::string tran) {
    std::cout << "Transaction: " << tran << " waits because of a down site." << std::endl;
}
