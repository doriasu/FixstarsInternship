all:Resource Pulse Setfile Camera shot tcp
Resource:ResourceManager.c
	ntoarmv7-gcc -o Resource -Wall -Wextra -pedantic-errors ResourceManager.c -lspi-master
Pulse:send_pulse.c
	ntoarmv7-gcc -o Pulse -Wall -Wextra -pedantic-errors send_pulse.c -lspi-master
Setfile:mynull_setfile.c
	ntoarmv7-gcc -o Setfile -Wall -Wextra -pedantic-errors mynull_setfile.c -lspi-master
Camera:Camera_resource.c
	ntoarmv7-gcc -o Camera -Wall -Wextra -pedantic-errors Camera_resource.c -lspi-master
shot:Camera_client.c
	ntoarmv7-gcc -o shot -Wall -Wextra -pedantic-errors Camera_client.c -lspi-master
try:camera_devctl.c
	ntoarmv7-gcc -o try -Wall -Wextra -pedantic-errors camera_devctl.c -lspi-master
tcp:Camera_client_tcp.c
	ntoarmv7-gcc -o tcp -Wall -Wextra -pedantic-errors Camera_client_tcp.c -lspi-master -lsocket 


clean:
	rm -f Resource
	rm -f Pulse
	rm -f Setfile
	rm -f Camera
	rm -f shot
	rm -f try
	rm -f tcp
