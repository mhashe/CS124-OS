CFLAGS = -Wall -g

OBJS = mysh.o token.o com_parser.o

all: mysh

mysh: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

com_exec: com_exec.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

com_parser: com_parser.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)	

clean: 
	rm -f *.o *~ mysh com_exec com_parser

.PHONY: all clean
