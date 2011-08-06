#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
	int on=1;
	int off=0;
	int led_no=0;
	int fd;

	/*if (argc != 3 || sscanf(argv[1], "%d", &led_no) != 1 || sscanf(argv[2],"%d", &on) != 1 ||
			on < 0 || on > 1 || led_no < 0 || led_no > 3)
	{
		fprintf(stderr, "Usage: leds led_no 0|1\n");
		exit(1);
	}
	*/
	fd = open("/dev/leds0", 0);
	if (fd < 0) 
	{
		fd = open("/dev/leds", 0);
	}
	if (fd < 0)
	{
		perror("open device leds");
		exit(1);
	}
	while(1)
	{
		ioctl(fd, on, led_no%4);
		sleep(1);
		ioctl(fd, off, led_no%4);
		sleep(1);
		led_no++;
			
	}
	//close(fd);

	return 0;
}

