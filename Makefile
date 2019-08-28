all:Resource Pulse
Resource:ResourceManager.c
	ntoarmv7-gcc -o Resource -Wall -Wextra -pedantic-errors ResourceManager.c -lspi-master
Pulse:send_pulse.c
	ntoarmv7-gcc -o Pulse -Wall -Wextra -pedantic-errors send_pulse.c -lspi-master
clean:
	rm -f Resource
	rm -f Pulse
