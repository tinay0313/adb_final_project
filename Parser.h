/** @file Parser.h
    @author Ann Tsai mt4050
 */

#ifndef PARSER
#define PARSER


#include <fstream>

#include "TransactionManager.h"


class Parser {

    public:

        Parser(std::string *infile);

        /* initialize a transaction manager */ 
        TransactionManager TM;

        std::string delimiters;
        /* Parse the input file into the required format for the transaction manager. */ 
        std::vector<std::string> parse(std::string instruction, std::string delimiters);
};


#endif
