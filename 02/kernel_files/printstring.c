#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>


asmlinkage long sys_printstring(int len, char __user * src)
{
	char buf[256];
	unsigned long lenleft = len;
	unsigned long chunklen = sizeof(buf);//256

	while( lenleft > 0 ) {
		if ( lenleft < chunklen )
			chunklen = lenleft;
		if ( copy_from_user(buf, src, chunklen) )//copy form user to kernel, return 0 mean success
			return -EFAULT;

		lenleft -= chunklen;
		printk("%s", buf);
	}
	return 0;
}
