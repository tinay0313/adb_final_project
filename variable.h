/** @file variable.h
    @author Tina Yuan thy258
    The file defines the variable structure and provides function declarations and definitions
    for simple getters and setters.
 */

#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "lockDetails.h"
#include <string>

class variable
{
    string name;
    int value;
    bool isReplicated;
    bool readable;
    lockDetails lock;

public:
    /* return variable name */ 
    string getName() { return name; }

    /* return variable value */
    int getValue() { return value; }

    /* return isReplicated */
    bool getIsReplicated() { return isReplicated; }

    /* return readable */
    bool getReadable() { return readable; }

    /* return lock */
    lockDetails getLock() { return lock; }

    /* sets isReplicated to true, default value is false */
    void setIsReplicatedTrue() { isReplicated = true; }

    /* sets readable to true, default value is true */
    void setReadableTrue() { readable = true; }

    /* sets isReplicated to false, default value is false */
    void setIsReplicatedFalse() { isReplicated = false; }

    /* sets readable to false, default value is true */
    void setReadableFalse() { readable = false; }
    
    /* sets variable name */
    void setName(string name) { this->name = name; }

    /* sets variable value */
    void setValue(int value) { this->value = value; }
};

#endif