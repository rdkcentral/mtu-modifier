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

extern int init_mtu_mod_proc(void);
extern void deinit_mtu_mod_proc(void);
extern void mtu_mod_node_init(void);
extern void mtu_mod_node_deinit(void);

/************************************************************/

int mtu_mod_init(void)
{
	mtu_mod_node_init();
	if(init_mtu_mod_proc()){
		return(-1);
	}

	printk(KERN_INFO "MTU Modifier loaded\n");
	return(0);
}

void mtu_mod_clean(void)
{
	deinit_mtu_mod_proc();
	mtu_mod_node_deinit();
	printk(KERN_INFO "MTU Modifier unloaded\n");
}

module_init(mtu_mod_init);
module_exit(mtu_mod_clean);

MODULE_LICENSE("GPL") ; 

