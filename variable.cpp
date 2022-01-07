/** @file variable.cpp
    @author Tina Yuan thy258
    The file provides the constructor for the variable class.
 */

#include "variable.h"

using namespace std;

/* variable Constructor */
variable::variable(int i, int value)
{
    this->idx = i;
    this->name = "x" + to_string(i);
    this->isReplicated = false;
    variable::setValue(value);
    variable::setReadableTrue();
}