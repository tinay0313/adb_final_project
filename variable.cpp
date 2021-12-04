/** @file variable.cpp
    @author Tina Yuan thy258
    The file provides the constructor for the variable class.
 */


#include "lockDetails.h"
#include "variable.h"

using namespace std;

/* variable Constructor */
variable::variable(int i, int value)
{
    this->idx = idx;
    this->name = "x" + to_string(idx);
    this->isReplicated = false;
    variable::setValue(value);
    variable::setReadableTrue();
}