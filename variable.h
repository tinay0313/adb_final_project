/** @file variable.h
    @author Tina Yuan thy258
    The file defines the variable structure and provides function declarations and definitions
    for simple getters and setters.
 */

#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "lockDetails.h"
#include <string>

using namespace std;

class variable
{
    string name;
    int idx;
    int value;
    bool isReplicated;
    bool readable;
    lockDetails* lock;

public:
    /* constructor */
    variable(int i, int value);

    /* return variable name */ 
    string getName() { return name; }

    /* return variable name's index value */ 
    int getIdx() { return idx; }

    /* return variable value */
    int getValue() { return value; }

    /* return isReplicated */
    bool getIsReplicated() { return isReplicated; }

    /* return readable */
    bool getReadable() { return readable; }

    /* return lock */
    lockDetails* getLock() { return lock; }

    /* sets isReplicated to true, default value upon variable construction is false */
    void setIsReplicatedTrue() { isReplicated = true; }

    /* sets readable to true, default value upon variable construction is true */
    void setReadableTrue() { readable = true; }

    /* sets readable to false, default value upon variable construction is true */
    void setReadableFalse() { readable = false; }

    /* sets variable value */
    void setValue(int value) { this->value = value; }
};

#endif