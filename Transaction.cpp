/** @file Transaction.cpp
    @author Ann Tsai mt4050
 */

#include "Transaction.h"

Transaction::Transaction(std::string name, bool isReadOnly, int startTime)
{
    this->name = name;
    this->isReadOnly = isReadOnly;
    this->startTime = startTime;
    this->toBeAborted = false;
    /*
    varValCache(),
    varAccessedList(),
    siteAccessedList(),
    varValueList()
    */
}

void Transaction::printLockConflict(std::string tran) {
    std::cout << "Transaction: " << tran << " waits because of a lock conflict." << std::endl;
}

void Transaction::printDownSite(std::string tran) {
    std::cout << "Transaction: " << tran << " waits because of a down site." << std::endl;
}

void Transaction::printInvalid(std::string tran) {
    std::cout << "Transaction: " << tran << " waits because of a invalid variable." << std::endl;
}

void Transaction::printDownInvalid(std::string tran) {
    std::cout << "Transaction: " << tran << " waits because of a down site and invalid variable." << std::endl;
}