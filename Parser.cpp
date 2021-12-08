/** @file Parser.cpp
    @author Ann Tsai mt4050
 */

#include "Parser.h"
#include "TransactionManager.h"

#include <regex>


Parser::Parser(std::string *infile):
    TM(),
    delimiters("[(,)]")

{   
    // instruction infos
    std::string tran;
    bool isReadOnly;
    std::string var;
    int val;
    int site;

    // read input data
    std::ifstream inputFile;
    std::string curLine;

    inputFile.open(*infile);
      
    while (getline(inputFile, curLine) && !inputFile.eof()){
        // get instruction infos
        std::vector<std::string> tokens = Parser::parse(curLine, delimiters);
        std::cout << curLine << std::endl;
        // abort the youngest transaction if a deadlock is founded
        while (TM.detectDeadlock()) {
            std::cout << "Deadlock detected." << std::endl;
        }
        //  pass intructions to transaction manager
        if (curLine.rfind("beginRO", 0) == 0) {  // won't affetc waiting queue
            tran = tokens.at(1);
            TM.begin(tran, true, TM.timeStamp);
        } else if (curLine.rfind("begin", 0) == 0) {
            tran = tokens.at(1);
            TM.begin(tran, false, TM.timeStamp);
        } else if (curLine.rfind("R", 0) == 0) {
            tran = tokens.at(1);
            var = tokens.at(2);
            TM.enqueueReadInstruction(tran, var);
            TM.updateVarAccessedList(tran, var);
        } else if (curLine.rfind("W", 0) == 0) {
            tran = tokens.at(1);
            var = tokens.at(2);
            val = std::stoi(tokens.at(3));  //convert to int
            TM.enqueueWriteInstruction(tran, var, val);
            TM.updateVarAccessedList(tran, var);
        } else if (curLine.rfind("dump", 0) == 0) {
            TM.dump();
        } else if (curLine.rfind("end", 0) == 0) {
            tran = tokens.at(1);
            TM.end(tran);
        } else if (curLine.rfind("fail", 0) == 0) {
            site = std::stoi(tokens.at(1));
            TM.fail(site, TM.timeStamp);
        } else if (curLine.rfind("recover", 0) == 0) {
            site = std::stoi(tokens.at(1));
            TM.recover(site);
        } else {
            std::cout << "invalid" << std::endl;
            continue;
        }
        // advances timestamp by one upon reading newline from input.
        TM.timeStamp++;
    }
    inputFile.close();
}

std::vector<std::string> Parser::parse(std::string instruction, std::string delimiters) {
    std::regex re(delimiters);
    std::sregex_token_iterator first{instruction.begin(), instruction.end(), re, -1}, last;
    std::vector<std::string> tokens{first, last};
    return tokens;
}
