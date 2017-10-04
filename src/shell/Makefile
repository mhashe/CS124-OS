CFLAGS = -Wall -g

OBJS = mysh.o 
# Include token.o once tokenizer complete

all: mysh

mysh: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Temporary, for testing
token: token.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean: 
	rm -f *.o *~ mysh token

.PHONY: all clean

