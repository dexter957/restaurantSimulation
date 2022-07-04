CC = gcc
CFLAG = -c 
LIBS = -lpthread
#Variables for the executables
RES = restaurant
CUS = customer
WAI = waiter
DOO = doorman
#Variables for the object files, on which each executable depends 
RESOBJS = restaurant.o
CUSOBJS = customer.o
WAIOBJS = waiter.o
DOOOBJS = doorman.o wQueue.o
#Variable for the source files
SRCS = restaurant.c waiter.c customer.c doorman.c wQueue.c 
OBJS = $(SRCS: .c=.o)

#Rules

all: $(RES) $(CUS) $(WAI) $(DOO)

$(RES) : $(RESOBJS)
	$(CC) -o $@ $(RESOBJS) $(LIBS)

$(CUS) : $(CUSOBJS)
	$(CC) -o $@ $(CUSOBJS) $(LIBS)

$(WAI) : $(WAIOBJS)
	$(CC) -o $@ $(WAIOBJS) $(LIBS)

$(DOO) : $(DOOOBJS)
	$(CC) -o $@ $(DOOOBJS) $(LIBS)

