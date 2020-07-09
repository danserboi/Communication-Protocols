
CFLAGS = -Wall -g

all: server client

# Compileaza server.c
server: server.c glist.c

# Compileaza client.c
client: client.c glist.c

.PHONY: clean run_server run_client

# Ruleaza serverul cu: make run_server ARGS="<PORT_SERVER>"
# Alternativ, se poate da make si apoi ./server <PORT_SERVER>
run_server: ./server
	./server $(ARGS)

# Ruleaza subscriber-ul cu: make run_subscriber ARGS="<IP_SERVER> <PORT_SERVER>"
# Alternativ, se poate da make si apoi ./subscriber <IP_SERVER> <PORT_SERVER>
run_client: ./client
	./client $(ARGS)

clean:
	rm -f server client
