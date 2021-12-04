/** @file variable.cpp
    @author Tina Yuan thy258
    The file provides the constructor for the variable class.
 */


#include "lockDetails.h"
#include "variable.h"

using namespace std;

/* variable Constructor */
variable::variable(string name, int value)
{
    variable::setName(name);
    variable::setValue(value);
    variable::setIsReplicatedFalse();
    variable::setReadableTrue();
}