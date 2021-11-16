/** @file variable.h
    @author Tina Yuan thy258
    The file defines the variable structure and provides function declarations for
    related functions.
 */

#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "lockDetails.h"
#include <cstring>

class variable
{
    string name;
    int value;
    bool isReplicated;
    bool readable;
    lockDetails lock;

public:
    /* return variable name */ 
    string getName();

    /* return variable value */
    int getValue();

    /* return isReplicated */
    bool getIsReplicated();

    /* return readable */
    bool getReadable();

    /* return lock */
    lockDetails getLock();

    /* sets isReplicated to true, default value is false */
    void setIsReplicated();

    /* sets readable to true, default value is false */
    void setReadable();
    
    /* sets variable name */
    void setName(string name);

    /* sets variable value */
    void setValue(int value);
};

#endif