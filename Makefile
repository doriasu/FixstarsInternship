all:Resource Pulse Setfile
Resource:ResourceManager.c
	ntoarmv7-gcc -o Resource -Wall -Wextra -pedantic-errors ResourceManager.c -lspi-master
Pulse:send_pulse.c
	ntoarmv7-gcc -o Pulse -Wall -Wextra -pedantic-errors send_pulse.c -lspi-master
Setfile:mynull_setfile.c
	ntoarmv7-gcc -o Setfile -Wall -Wextra -pedantic-errors mynull_setfile.c -lspi-master
clean:
	rm -f Resource
	rm -f Pulse
	rm -f Setfile
