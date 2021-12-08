/** @file TransactionManager.h
    @author Ann Tsai mt4050
 */

#ifndef TRANSACTION_MANAGER
#define TRANSACTION_MANAGER

#include <unordered_map>
#include <unordered_set>


#include "Transaction.h"
#include "dataManager.h"


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
        /* ID  of variable, 20 refers to x20 */
        int varID;
        /* key is the transaction name and the value are the transactions that are waiting For key’s data lock */
        std::unordered_map<std::string, std::vector<std::string> > blockingGraph;
        /* keep information of active transactions */
        std::unordered_map<std::string, Transaction> transactionList;
        /* record read/write instructions to be executed */
        std::vector<Instruction> instructionQueue;


        /* detects deadlock by checking if there is a cycle in the blockingGraph Returns transaction that has to be aborted or nullptr otherwise */
        /* True : A deadlock is detected, the youngest transaction will be aborted.*/
        bool detectDeadlock();
        /* detect deadlock by dfs */
        bool helper(std::string start, std::string target, std::unordered_set<std::string>& visited,
                    std::unordered_map<std::string, std::vector<std::string>> blockingGraph);
        /* execute transaction from the waiting queue */
        void executeNextTransaction(std::string tran);
        
        /* add an edge to the blockingGraph*/
        void addEdge(std::string trans1, std::string trans2);
        /* delete an edge in the blockingGraph*/
        void deleteEdge(std::string trans1);

        /* Begin a transactione*/
        void begin(std::string tran, bool isReadOnly, int startTime);
        bool readRO(std::string tran, std::string var);
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
        void commit(Transaction* t, std::unordered_map<int, int> varValueList);
        void fail(int siteID, int timeStamp);
        void recover(int siteID);
        /* enqueue read/write enqueueReadInstruction to instructionQueue */
        void enqueueReadInstruction(std::string tran, std::string var);
        void enqueueWriteInstruction(std::string tran, std::string var, int val);
        /* update varAccessedList*/
        void updateVarAccessedList(std::string tran, std::string var);
};


#endif
