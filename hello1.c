// SPDX-License-Identifier: GPL-2.0
/*
 * hello1: exports print_hello() and keeps the list of print events
 * with the time spent on every print (AK-2, lab 5).
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/slab.h>

#include "hello1.h"

MODULE_AUTHOR("Rentsevych D., IO-45");
MODULE_DESCRIPTION("Exports print_hello() and measures the time of every print");
MODULE_LICENSE("GPL");

static LIST_HEAD(hello_events);

int print_hello(void)
{
	struct hello_event *ev;

	ev = kmalloc_obj(*ev, GFP_KERNEL);
	if (!ev) {
		pr_err("cannot allocate a list entry\n");
		return -ENOMEM;
	}

	ev->start = ktime_get();
	pr_info("Hello, world!\n");
	ev->end = ktime_get();

	list_add_tail(&ev->list, &hello_events);

	return 0;
}
EXPORT_SYMBOL(print_hello);

static int __init hello1_init(void)
{
	pr_info("loaded\n");
	return 0;
}

static void __exit hello1_exit(void)
{
	struct hello_event *ev, *tmp;
	unsigned int i = 0;

	list_for_each_entry_safe(ev, tmp, &hello_events, list) {
		pr_info("print %u: start %lld ns, took %lld ns\n", ++i,
			ktime_to_ns(ev->start),
			ktime_to_ns(ktime_sub(ev->end, ev->start)));
		list_del(&ev->list);
		kfree(ev);
	}

	pr_info("unloaded, %u print(s) recorded\n", i);
}

module_init(hello1_init);
module_exit(hello1_exit);
