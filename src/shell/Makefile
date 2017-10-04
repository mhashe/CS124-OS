CFLAGS = -Wall -g

OBJS = mysh.o token.o

all: mysh

mysh: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Temporary, for testing
token: token.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean: 
	rm -f *.o *~ mysh token

.PHONY: all clean

