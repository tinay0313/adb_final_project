#include "lockDetails.h"
#include "variable.h"

using namespace std;

class variable
{
    string name;
    int value;
    bool isReplicated;
    bool readable;
    lockDetails lock;
}