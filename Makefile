all:Resource Pulse Setfile Camera Client
Resource:ResourceManager.c
	ntoarmv7-gcc -o Resource -Wall -Wextra -pedantic-errors ResourceManager.c -lspi-master
Pulse:send_pulse.c
	ntoarmv7-gcc -o Pulse -Wall -Wextra -pedantic-errors send_pulse.c -lspi-master
Setfile:mynull_setfile.c
	ntoarmv7-gcc -o Setfile -Wall -Wextra -pedantic-errors mynull_setfile.c -lspi-master
Camera:Camera_resource.c
	ntoarmv7-gcc -o Camera -Wall -Wextra -pedantic-errors Camera_resource.c -lspi-master
Client:Camera_client.c
	ntoarmv7-gcc -o Client -Wall -Wextra -pedantic-errors Camera_client.c -lspi-master
try:camera_devctl.c
	ntoarmv7-gcc -o try -Wall -Wextra -pedantic-errors camera_devctl.c -lspi-master

clean:
	rm -f Resource
	rm -f Pulse
	rm -f Setfile
	rm -f Camera
	rm -f Client
