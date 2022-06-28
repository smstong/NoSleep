/**********************************************************************
 *                 NoSleep For Linux
 * Description:
 * A simple app to keep Linux not to sleep by moving mouse
 * back and forth frequently.
 *
 * Author: jinqiangzhao2017@gmail.com
 *
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/uinput.h>

static int g_fd_uinput = -1;		/* fileno of /dev/uinput */
static int g_dx = 1;				/* step to move mouse */
static int g_sec_interval = 10;		/* seconds between two moves */

/* Create a virtual mouse via Linux u(ser)input system */
static int MouseInit()
{
	struct uinput_setup us;
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(fd == -1) {
		printf("open(/dev/uinput) failed.\n");
		return -1;
	}

	/* enable mouste buttons */
	ioctl(fd, UI_SET_EVBIT, EV_KEY);	/* enable mouste buttons */
	ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);	/* enable left button */
	ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);	/* enable right button */
	ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);	/* enable middle button */

	/* enable mouse move/scroll */
	ioctl(fd, UI_SET_EVBIT, EV_REL);	/* enable mouse move and scroll */
	ioctl(fd, UI_SET_RELBIT, REL_X);	/* enable x direction move */
	ioctl(fd, UI_SET_RELBIT, REL_Y);	/* enable y direction move */
	ioctl(fd, UI_SET_RELBIT, REL_WHEEL);	/* enable scroll */

	memset(&us, 0, sizeof(us));
	us.id.bustype = BUS_USB;
	us.id.vendor = 0x1234;
	us.id.product = 0x5678;
	strcpy(us.name, "Virtual Mouse");

	ioctl(fd, UI_DEV_SETUP, &us);
	ioctl(fd, UI_DEV_CREATE);

	g_fd_uinput = fd;
	return 0;
}

/* delete the virtual mouse device */
static void MouseExit()
{
	if (-1 != g_fd_uinput) {
		ioctl(g_fd_uinput, UI_DEV_DESTROY);
		close(g_fd_uinput);
		g_fd_uinput = -1;
	}
}

/* Move the mouse by $g_dx back and forth herizontally */
static void MouseMove()
{
	struct input_event e1,e2;

	e1.type = EV_REL;
	e1.code = REL_X;
	e1.value = g_dx;
	e1.time.tv_sec = 0;
	e1.time.tv_usec = 0;

	e2.type = EV_SYN;
	e2.code = SYN_REPORT;
	e2.value = 0;
	e2.time.tv_sec = 0;
	e2.time.tv_usec = 0;

	write(g_fd_uinput, &e1, sizeof(struct input_event));
	write(g_fd_uinput, &e2, sizeof(struct input_event));

	g_dx = -g_dx;
}

/* move mouse every $secInterval second to keep the computer not sleeping */
static int NoSleep()
{
	if(0 != MouseInit()){
		printf("MouseInit() failed.\n");
		return -1;
	}

	while(1){
		MouseMove();
		sleep(g_sec_interval);
	}

	MouseExit();
	return 0;
}

int main(int argc, char** argv)
{
	if(argc >= 2){
		if(0==strcmp("help", argv[1]) ||
		   0==strcmp("-h", argv[1]) || 
		   0==strcmp("--help", argv[1])){
			printf("Usage: NoSleep [-h|--help|help|seconds=10] [g_dx=1]\n");
			printf("Example: NoSleep 60 100\n");
			return 0;
		}else{
			g_sec_interval = atoi(argv[1]);
		}
	}
	if(argc >= 3){
		g_dx = atoi(argv[2]);
	}

	NoSleep();
}
