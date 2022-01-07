/** @file Parser.cpp
    @author Ann Tsai mt4050
 */

#include "Parser.h"
#include "TransactionManager.h"

#include <regex>
#include <iostream>
#include <string>


Parser::Parser(std::string *infile):
    TM(),
    //delimiters("[(,)]")
    delimiters("[\\s+(,)]")
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
        std::vector<std::string> tokens = parse(curLine);
        
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
            cout << "trying to recover" << endl;
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

std::vector<std::string> Parser::parse(std::string curLine) {
    std::vector<std::string> raw;
    std::vector<std::string> tokens;

    std::size_t prev = 0, pos;
    while ((pos = curLine.find_first_of(" (,)", prev)) != std::string::npos)
    {
        if (pos > prev)
            raw.push_back(curLine.substr(prev, pos-prev));
        prev = pos+1;
    }
    if (prev < curLine.length())
        raw.push_back(curLine.substr(prev, std::string::npos));

    for (auto const& token: raw) {
        if (token == "") {
            continue;
        } else {
            tokens.push_back(token);
        }
    }
    
    /*
    std::stringstream stringStream(instruction);
    std::string line;
    std::vector<std::string> tokens;
    while(std::getline(stringStream, line)) 
    {
        std::size_t prev = 0, pos;
        while ((pos = line.find_first_of(delimiters, prev)) != std::string::npos)
        {
            if (pos > prev)
                tokens.push_back(line.substr(prev, pos-prev));
            prev = pos+1;
        }
        if (prev < line.length())
            tokens.push_back(line.substr(prev, std::string::npos));
    }
    
    std::regex re(delimiters);
    std::sregex_token_iterator first{instruction.begin(), instruction.end(), re, -1}, last;
    std::vector<std::string> tokens{first, last};
    */
    return tokens;
}
