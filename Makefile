reset:Camera_reset.c
	ntoarmv7-gcc -o reset -Wall -Wextra -pedantic-errors Camera_reset.c -lspi-master
clean:
	rm -f reset
