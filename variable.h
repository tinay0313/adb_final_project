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
}

#endif