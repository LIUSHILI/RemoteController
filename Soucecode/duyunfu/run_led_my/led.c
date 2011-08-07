#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

enum{ ON=1, OFF=0 };

int main(int argc, char **argv)
{
	int no = 1;
	int i;
	int led_no = 0;
	int fd;

	/*
	if (argc != 3 || sscanf(argv[1], "%d", &led_no) != 1 || sscanf(argv[2],"%d", &on) != 1 ||
			on < 0 || on > 1 || led_no < 0 || led_no > 3) 
	{
		fprintf(stderr, "Usage: leds led_no 0|1\n");
		exit(1);
	}
	*/

	fd = open("/dev/led_test", 0);

	if (fd < 0) 
	{
		perror("open device leds");
		exit(1);
	}

	while(1)
	{
		for(led_no=0; led_no < 4; led_no++)
		{
			for(i=0; i < 4; i++)
			{
				if(i != led_no)
					ioctl(fd, OFF, i);
			}
			ioctl(fd, ON, led_no);
			for(i=0; i < 0x666666; i++);
		}		
	}
	//ioctl(fd, on, led_no);
	close(fd);

	return 0;
}

