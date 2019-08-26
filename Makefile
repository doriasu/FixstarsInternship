Client:Client.c
	ntoarmv7-gcc -o Client -Wall -Wextra -pedantic-errors Client.c -lspi-master
Server:Server.c
	ntoarmv7-gcc -o Server -Wall -Wextra -pedantic-errors Server.c -lspi-master
clean:
	rm -f Server
	rm -f Client
