all:Resource
Resource:ResourceManager.c
	ntoarmv7-gcc -o Resource -Wall -Wextra -pedantic-errors ResourceManager.c -lspi-master
clean:
	rm -f Resource
