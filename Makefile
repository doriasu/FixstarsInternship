reset:Camera_reset.c
	ntoarmv7-gcc -o reset -Wall -Wextra -pedantic-errors Camera_reset.c -lspi-master
ID:ProductID.c
	ntoarmv7-gcc -o ID -Wall -Wextra -pedantic-errors ProductID.c -lspi-master
clean:
	rm -f reset
	rm -f ID
