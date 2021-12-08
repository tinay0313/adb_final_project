## ----------------------------------------------------------------------
## Author : Ann Tsai mt4050
## ----------------------------------------------------------------------

# declare variables
CC = g++ -std=c++11
CPPFLAGS = -c -Wall
OFILES = main.o Parser.o TransactionManager.o Transaction.o dataManager.o site.o variable.o lockDetails.o

# commands
main : $(OFILES)
	$(CC) $(OFILES) -o main

main.o: main.cpp Parser.h
	$(CC) $(CPPFLAGS) main.cpp

Parser.o: Parser.cpp Parser.h TransactionManager.h
	$(CC) $(CPPFLAGS) Parser.cpp

TransactionManager.o: TransactionManager.cpp TransactionManager.h Transaction.h
	$(CC) $(CPPFLAGS) TransactionManager.cpp

Transaction.o: Transaction.cpp Transaction.h dataManager.h
	$(CC) $(CPPFLAGS) Transaction.cpp

dataManager.o: dataManager.cpp dataManager.h variable.h site.h Transaction.h lockDetails.h
	$(CC) $(CPPFLAGS) dataManager.cpp

site.o: site.cpp site.h variable.h lockDetails.h Transaction.h
	$(CC) $(CPPFLAGS) site.cpp

variable.o: variable.cpp variable.h lockDetails.h
	$(CC) $(CPPFLAGS) variable.cpp

lockDetails.o: lockDetails.cpp lockDetails.h Transaction.h
	$(CC) $(CPPFLAGS) lockDetails.cpp

clean:
	rm -f main *.o *~
