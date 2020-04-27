#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

asmlinkage long sys_printdmesg(int len, char __user * src)
{
	char buf[256];
	unsigned long lenleft = len;
	//copy form user to kernel, return 0 mean success
	if ( copy_from_user(buf, src, lenleft) )return -EFAULT;
	printk("%s", buf);
	return 0;
}
