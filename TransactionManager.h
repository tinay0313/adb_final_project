/** @file TransactionManager.h
    @author Ann Tsai mt4050
 */

#ifndef TRANSACTION_MANAGER
#define TRANSACTION_MANAGER

#include <unordered_map>
#include <unordered_set>


#include "Transaction.h"


class TransactionManager {
    
    public:

        TransactionManager();

        struct Instruction
        {
            std::string type; // R/W
            std::string tran;
            std::string var;
            int val;          // 0 for Read
        };

        dataManager DM;  // todo why pointer being freed was not allocated?????

        /* cur time*/
        int timeStamp;
        /* size  of siteSize */
        int siteSize;
        /* SiteVariableList[i] has a vector of all variables saved at site i */ 
        std::vector<std::vector<variable> > siteVariableList; //todo why pointer being freed was not allocated?????
        /* record the failure history of every site. */ 
        std::vector<std::vector<int> > failureHistory;
        /* key is the transaction name and the value are the transactions that are waiting For key’s data lock */
        std::unordered_map<std::string, std::vector<std::string> > blockingGraph;
        // turn into transaction list, no need to record age  
        std::unordered_map<std::string, Transaction> transactionList;

        /* record read/write instructions to be executed */
        std::vector<Instruction> instructionQueue;

        /* if SiteStatus[i] == true then site i is up, Else site i is down */
        // turn into site list, no need to record status --> site.isRunning
        // no need site list --> DM.sites
        // std::vector<bool> siteList;  // initial size = 11, true 


        /* detects deadlock by checking if there is a cycle in the blockingGraph Returns transaction that has to be aborted or nullptr otherwise */
        /* True : A deadlock is detected, the youngest transaction will be aborted.*/
        bool detectDeadlock();
        /* detect deadlock by dfs */
        bool helper(std::string start, std::string target, std::unordered_set<std::string> visited,
                    std::unordered_map<std::string, std::vector<std::string>> blockingGraph);
        /* execute instruction queue one more time if there is a deadlock detected */
        void executeInstructionQueue();
        
        /* add an edge to the blockingGraph*/
        void addEdge(std::string trans1, std::string trans2);
        /* delete an edge in the blockingGraph*/
        void deleteEdge(std::string trans1, std::string trans2);

        /* Begin a transactione*/
        void begin(std::string tran, bool isReadOnly, int startTime);
        /* enqueue read/write enqueueReadInstruction to instructionQueue */
        void enqueueReadInstruction(std::string tran, std::string var);
        void enqueueWriteInstruction(std::string tran, std::string var, int val);
        bool read(std::string tran, std::string var);
        /*
        1. write to all up sites if write lock can be acquired
        2. if cannot get all write locks, cannot write
        */
        bool write(std::string tran, std::string var, int val);
        void dump();
        /* delete transaction instance */
        void end(std::string tran);
                /* 
        1. release all current locks for this transaction
        2. update owners
        3. prints transaction name upon abort
        */
        void abort(std::string tran);
        /* 
        1. release all current locks for this transaction
        2. update commit value
        3. update owners
        4. prints transaction name upon commit
        */
        void commit(Transaction* t, unordered_map<int, int> variableValueMap);
        void fail(int siteID, int timeStamp);
        void recover(int siteID);
        

};


#endif


