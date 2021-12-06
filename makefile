## ----------------------------------------------------------------------
## Author : Ann Tsai mt4050
## ----------------------------------------------------------------------

# declare variables
CC = g++ -std=c++11
CPPFLAGS = -c -Wall
OFILES = main.o Parser.o TransactionManager.o Transaction.o DataManager.o

# commands
main : $(OFILES)
	$(CC) $(OFILES) -o main

main.o: main.cpp Parser.h
	$(CC) $(CPPFLAGS) main.cpp

Parser.o: Parser.cpp Parser.h TransactionManager.h
	$(CC) $(CPPFLAGS) Parser.cpp

TransactionManager.o: TransactionManager.cpp TransactionManager.h Transaction.h
	$(CC) $(CPPFLAGS) TransactionManager.cpp

Transaction.o: Transaction.cpp Transaction.h DataManager.h
	$(CC) $(CPPFLAGS) Transaction.cpp

DataManager.o: DataManager.cpp DataManager.h Variable.h Site.h 
	$(CC) $(CPPFLAGS) DataManager.cpp

clean:
	rm -f main *.o *~
