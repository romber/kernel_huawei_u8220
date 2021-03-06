/* arch/arm/mach-msm/rpc_server_time_remote.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 * Author: Iliyan Malchev <ibm@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/msm_rpcrouter.h>
#include "rpc_server_time_remote.h"

/* time_remote_mtoa server definitions. */

#define TIME_REMOTE_MTOA_PROG 0x3000005d
#define TIME_REMOTE_MTOA_VERS_OLD 0
#define TIME_REMOTE_MTOA_VERS 0x9202a8e4
#define TIME_REMOTE_MTOA_VERS_COMP 0x00010001
#define RPC_TIME_REMOTE_MTOA_NULL   0
#define RPC_TIME_TOD_SET_APPS_BASES 2

struct rpc_time_tod_set_apps_bases_args {
	uint32_t tick;
	uint64_t stamp;
};

static int handle_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len)
{
	switch (req->procedure) {
	case RPC_TIME_REMOTE_MTOA_NULL:
		return 0;

	case RPC_TIME_TOD_SET_APPS_BASES: {
		struct rpc_time_tod_set_apps_bases_args *args;
		args = (struct rpc_time_tod_set_apps_bases_args *)(req + 1);
		args->tick = be32_to_cpu(args->tick);
		args->stamp = be64_to_cpu(args->stamp);
		printk(KERN_INFO "RPC_TIME_TOD_SET_APPS_BASES:\n"
		       "\ttick = %d\n"
		       "\tstamp = %lld\n",
		       args->tick, args->stamp);
		rtc_hctosys();
		return 0;
	}
	default:
		return -ENODEV;
	}
}

static struct msm_rpc_server rpc_server[] = {
	{
		.prog = TIME_REMOTE_MTOA_PROG,
		.vers = TIME_REMOTE_MTOA_VERS_OLD,
		.rpc_call = handle_rpc_call,
	},
	{
		.prog = TIME_REMOTE_MTOA_PROG,
		.vers = TIME_REMOTE_MTOA_VERS,
		.rpc_call = handle_rpc_call,
	},
	{
		.prog = TIME_REMOTE_MTOA_PROG,
		.vers = TIME_REMOTE_MTOA_VERS_COMP,
		.rpc_call = handle_rpc_call,
	},
};

static int __init rpc_server_init(void)
{
	/* Dual server registration to support backwards compatibility vers */
	int ret;
	ret = msm_rpc_create_server(&rpc_server[2]);
	if (ret < 0)
		return ret;
	ret = msm_rpc_create_server(&rpc_server[1]);
	if (ret < 0)
		return ret;
	printk(KERN_ERR "Using very old AMSS modem firmware.\n");
	return msm_rpc_create_server(&rpc_server[0]);
}


module_init(rpc_server_init);
