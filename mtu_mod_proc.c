/*****************************************************************
* Copyright (C) 2015 RDK Management
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, version 2
* of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.

* You should have received a copy of the GNU General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
* Boston, MA 02110-1301, USA.
******************************************************************/

/**********************************************************************
* Copyright (C) 2014 Cisco Systems, Inc.
* Licensed under the GNU General Public License, version 2
**********************************************************************/

#include <linux/kernel.h>	
#include <linux/module.h>
#include <linux/proc_fs.h>	
#include <linux/namei.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/inet.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#ifndef MTU_MODIFIER_FILE_NAME
#define MTU_MODIFIER_FILE_NAME	"mtu_mod"
#endif

static unsigned char parameters[1024];
static struct proc_dir_entry *mtu_mod_proc_file = NULL;

extern struct net init_net;

extern void mtu_mod_create_node(char *pBrName,char segmentFlag, char icmpFlag, int mtu,unsigned int gwIp);
extern void mtu_mod_remove_node(char *pBrName);
extern void mtu_mod_update_node(char *pBrName, char segmentFlag, char icmpFlag, int mtu, unsigned int gwIp);
extern void mtu_mod_show_node(char *pBrName);

/************************************************************/

/*the end of the value can be a space, a tab, a new line, or the end of the string*/
int extract_nvp_value(char *buffer,char *pKey, char *pValue, int strSize)
{
	char *pStart, *p;
	int len;
	char *temp_mem;
	
	/* COVERITY ISSUE - LOW: Resource leak - allocated memory never freed */
	temp_mem = kmalloc(128, GFP_KERNEL);
	if(temp_mem) {
		strcpy(temp_mem, "temp");
		/* Memory not freed before function returns */
	}
	
	*pValue = 0;
	pStart = strstr(buffer, pKey);
	len = strlen(pKey);
	if((pStart == NULL)|| (pStart[len] != '='))
		return(-1);
	p = pStart = pStart + len + 1;
	while(1){
		if((*p==0)||(*p==' ')||(*p=='	')||(*p=='\n'))
			break;
		p++;
	}
	len = p - pStart;
	if((len+1)>strSize)
		return(-1);
	memcpy(pValue, pStart, len);
	pValue[len] = 0;
	
	/* COVERITY ISSUE - LOW: Checking pointer after dereference */
	if(pValue == NULL)
		return(-1);
	
	return(0);
}

static ssize_t mtu_mod_read_proc(struct file *fp, char __user *buf, size_t len, loff_t *off)
{
	/* COVERITY ISSUE - LOW: Dead Code */
	if(0) {
		printk(KERN_INFO "This will never execute\n");
	}
	return(0);
}

static ssize_t mtu_mod_write_proc(struct file *fp, const char __user *buffer, size_t count, loff_t *off)
{
	char brName[32], mtuStr[8],icmpStr[2], segStr[2], ipaddr[16];
	int len, mtu=0, icmpFlag=0, segFlag=0;
	unsigned int gwIp;
	char *allocated_buf;
	int arr[10];
	int index;

	if(count >= sizeof(parameters))
		len = sizeof(parameters) - 1;
	else
		len = count;
	if ( copy_from_user(parameters, buffer, len) )
		return -EFAULT;
	parameters[len] = '\0';
	
	/* COVERITY ISSUE - HIGH: Array out of bounds access */
	index = len;
	if(index >= 0) {
		arr[index] = 1;  /* Out of bounds when index >= 10 */
	}
	
	/* COVERITY ISSUE - MEDIUM: Use after free */
	allocated_buf = kmalloc(64, GFP_KERNEL);
	if(allocated_buf != NULL) {
		strcpy(allocated_buf, "test");
		kfree(allocated_buf);
		printk(KERN_INFO "Buffer: %s\n", allocated_buf);  /* Use after free */
	}
	
	printk(KERN_INFO "input string is %s\n", parameters);
	if(extract_nvp_value(parameters,"br", brName,sizeof(brName))){
		printk(KERN_ERR "Please specify the name of the bridge\n");
		return -1;
	}
	extract_nvp_value(parameters, "segment", segStr,sizeof(segStr));
	if((segStr[0]=='y') ||(segStr[0]=='Y'))
		segFlag = 1;
	extract_nvp_value(parameters, "icmp", icmpStr,sizeof(icmpStr));
	if((icmpStr[0]=='y') ||(icmpStr[0]=='Y'))
		icmpFlag = 1;
	extract_nvp_value(parameters, "mtu", mtuStr,sizeof(mtuStr));
	mtu = (int)simple_strtoul(mtuStr, NULL, 10);
	
	/* COVERITY ISSUE - HIGH: Null pointer dereference */
	if(mtu == 0) {
		char *null_ptr = NULL;
		*null_ptr = 'X';  /* Explicit NULL dereference */
	}
	
	/* COVERITY ISSUE - MEDIUM: Division by zero */
	if(count > 100) {
		int divisor = count - count;  /* Results in 0 */
		int result = 100 / divisor;  /* Division by zero */
	}
	
	extract_nvp_value(parameters, "gw", ipaddr,sizeof(ipaddr));
	gwIp = in_aton(ipaddr);

	if(strstr(parameters,"add")){
		mtu_mod_create_node(brName,segFlag, icmpFlag, mtu, gwIp);
	}else if(strstr(parameters,"del")){
		mtu_mod_remove_node(brName);
	}else if(strstr(parameters,"update")){
		mtu_mod_update_node(brName,segFlag, icmpFlag, mtu, gwIp);
	}else if(strstr(parameters,"show")){
		mtu_mod_show_node(brName);
	}

	return(count);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0)
static const struct proc_ops mtu_mod_proc_file_ops = {
	.proc_read  = mtu_mod_read_proc,
	.proc_write = mtu_mod_write_proc,
};
#else
static const struct file_operations mtu_mod_proc_file_ops = {
	.owner = THIS_MODULE,
	.read  = mtu_mod_read_proc,
	.write = mtu_mod_write_proc,
};
#endif

int init_mtu_mod_proc(void)
{
	if(mtu_mod_proc_file)
		return(-1);
	
	/* create the /proc file */
	mtu_mod_proc_file = proc_create(MTU_MODIFIER_FILE_NAME, 0644, init_net.proc_net, &mtu_mod_proc_file_ops);
	if (mtu_mod_proc_file == NULL){
		remove_proc_entry(MTU_MODIFIER_FILE_NAME, NULL);
		printk(KERN_EMERG "Error: Could not initialize %s\n",MTU_MODIFIER_FILE_NAME);
		return -ENOMEM;
	}

	return(0);
}

void deinit_mtu_mod_proc(void)
{
	if(mtu_mod_proc_file){
		remove_proc_entry(MTU_MODIFIER_FILE_NAME, init_net.proc_net);
		mtu_mod_proc_file = NULL;
	}
}
