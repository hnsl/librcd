/*
 * This file contains source code copied from Linux headers.
 *
 * Linux is licenced under the GPLv2:
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 * For more information see: https://www.kernel.org/
 */

#ifndef LINUX_H
#define	LINUX_H

#include "musl.h"

/// Syscall eio class.
typedef struct {
    int errno_v;
} syscall_eio_t;
define_eio_complex(syscall, errno_v);

/// Throws a syscall exception.
#define RCD_SYSCALL_EXCEPTION(name, rcd_exception_type) _rcd_syscall_exception(#name "() failed with errno ", rcd_exception_type)

#define USER_HZ (100)

/*
 * If no_new_privs is set, then operations that grant new privileges (i.e.
 * execve) will either fail or not grant them. This affects suid/sgid,
 * file capabilities, and LSMs.
 *
 * Operations that merely manipulate or drop existing privileges (setresuid,
 * capset, etc.) will still work. Drop those privileges if you want them gone.
 *
 * Changing LSM security domain is considered a new privilege. So, for example,
 * asking selinux for a specific new context (e.g. with runcon) will result
 * in execve returning -EPERM.
 */
#define PR_SET_NO_NEW_PRIVS 38
#define PR_GET_NO_NEW_PRIVS 39

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004

struct mmsghdr {
    struct msghdr msg_hdr; /* Message header */
    unsigned int msg_len; /* Number of received bytes for header */
};

typedef int32_t daddr_t;

struct ustat {
    daddr_t f_tfree; /* Total free blocks */
    ino_t f_tinode; /* Number of free inodes */
    char f_fname[6]; /* Filsys name */
    char f_fpack[6]; /* Filsys pack name */
};

typedef unsigned long int __cpu_mask;

/* Data structure to describe CPU mask.  */
typedef struct {
    __cpu_mask __bits[1024 / (8 * sizeof (__cpu_mask))];
} cpu_set_t;

/************************** CAPABILITIES *************************************/

#define _LINUX_CAPABILITY_VERSION_1  0x19980330
#define _LINUX_CAPABILITY_U32S_1     1

#define _LINUX_CAPABILITY_VERSION_2  0x20071026  /* deprecated - use v3 */
#define _LINUX_CAPABILITY_U32S_2     2

#define _LINUX_CAPABILITY_VERSION_3  0x20080522
#define _LINUX_CAPABILITY_U32S_3     2

typedef struct __user_cap_header_struct {
    uint32_t version;
    int pid;
}* cap_user_header_t;

typedef struct __user_cap_data_struct {
    uint32_t effective;
    uint32_t permitted;
    uint32_t inheritable;
}* cap_user_data_t;


#define VFS_CAP_REVISION_MASK	0xFF000000
#define VFS_CAP_REVISION_SHIFT	24
#define VFS_CAP_FLAGS_MASK	~VFS_CAP_REVISION_MASK
#define VFS_CAP_FLAGS_EFFECTIVE	0x000001

#define VFS_CAP_REVISION_1	0x01000000
#define VFS_CAP_U32_1           1
#define XATTR_CAPS_SZ_1         (sizeof(__le32)*(1 + 2*VFS_CAP_U32_1))

#define VFS_CAP_REVISION_2	0x02000000
#define VFS_CAP_U32_2           2
#define XATTR_CAPS_SZ_2         (sizeof(__le32)*(1 + 2*VFS_CAP_U32_2))

#define XATTR_CAPS_SZ           XATTR_CAPS_SZ_2
#define VFS_CAP_U32             VFS_CAP_U32_2
#define VFS_CAP_REVISION	VFS_CAP_REVISION_2

struct vfs_cap_data {
    int32_t magic_etc; /* Little endian */
    struct {
        int32_t permitted; /* Little endian */
        int32_t inheritable; /* Little endian */
    } data[VFS_CAP_U32];
};

/**
 ** POSIX-draft defined capabilities.
 **/

/* In a system with the [_POSIX_CHOWN_RESTRICTED] option defined, this
   overrides the restriction of changing file ownership and group
   ownership. */

#define CAP_CHOWN            0

/* Override all DAC access, including ACL execute access if
   [_POSIX_ACL] is defined. Excluding DAC access covered by
   CAP_LINUX_IMMUTABLE. */

#define CAP_DAC_OVERRIDE     1

/* Overrides all DAC restrictions regarding read and search on files
   and directories, including ACL restrictions if [_POSIX_ACL] is
   defined. Excluding DAC access covered by CAP_LINUX_IMMUTABLE. */

#define CAP_DAC_READ_SEARCH  2

/* Overrides all restrictions about allowed operations on files, where
   file owner ID must be equal to the user ID, except where CAP_FSETID
   is applicable. It doesn't override MAC and DAC restrictions. */

#define CAP_FOWNER           3

/* Overrides the following restrictions that the effective user ID
   shall match the file owner ID when setting the S_ISUID and S_ISGID
   bits on that file; that the effective group ID (or one of the
   supplementary group IDs) shall match the file owner ID when setting
   the S_ISGID bit on that file; that the S_ISUID and S_ISGID bits are
   cleared on successful return from chown(2) (not implemented). */

#define CAP_FSETID           4

/* Overrides the restriction that the real or effective user ID of a
   process sending a signal must match the real or effective user ID
   of the process receiving the signal. */

#define CAP_KILL             5

/* Allows setgid(2) manipulation */
/* Allows setgroups(2) */
/* Allows forged gids on socket credentials passing. */

#define CAP_SETGID           6

/* Allows set*uid(2) manipulation (including fsuid). */
/* Allows forged pids on socket credentials passing. */

#define CAP_SETUID           7


/**
 ** Linux-specific capabilities
 **/

/* Without VFS support for capabilities:
 *   Transfer any capability in your permitted set to any pid,
 *   remove any capability in your permitted set from any pid
 * With VFS support for capabilities (neither of above, but)
 *   Add any capability from current's capability bounding set
 *       to the current process' inheritable set
 *   Allow taking bits out of capability bounding set
 *   Allow modification of the securebits for a process
 */

#define CAP_SETPCAP          8

/* Allow modification of S_IMMUTABLE and S_APPEND file attributes */

#define CAP_LINUX_IMMUTABLE  9

/* Allows binding to TCP/UDP sockets below 1024 */
/* Allows binding to ATM VCIs below 32 */

#define CAP_NET_BIND_SERVICE 10

/* Allow broadcasting, listen to multicast */

#define CAP_NET_BROADCAST    11

/* Allow interface configuration */
/* Allow administration of IP firewall, masquerading and accounting */
/* Allow setting debug option on sockets */
/* Allow modification of routing tables */
/* Allow setting arbitrary process / process group ownership on
   sockets */
/* Allow binding to any address for transparent proxying (also via NET_RAW) */
/* Allow setting TOS (type of service) */
/* Allow setting promiscuous mode */
/* Allow clearing driver statistics */
/* Allow multicasting */
/* Allow read/write of device-specific registers */
/* Allow activation of ATM control sockets */

#define CAP_NET_ADMIN        12

/* Allow use of RAW sockets */
/* Allow use of PACKET sockets */
/* Allow binding to any address for transparent proxying (also via NET_ADMIN) */

#define CAP_NET_RAW          13

/* Allow locking of shared memory segments */
/* Allow mlock and mlockall (which doesn't really have anything to do
   with IPC) */

#define CAP_IPC_LOCK         14

/* Override IPC ownership checks */

#define CAP_IPC_OWNER        15

/* Insert and remove kernel modules - modify kernel without limit */
#define CAP_SYS_MODULE       16

/* Allow ioperm/iopl access */
/* Allow sending USB messages to any device via /proc/bus/usb */

#define CAP_SYS_RAWIO        17

/* Allow use of chroot() */

#define CAP_SYS_CHROOT       18

/* Allow ptrace() of any process */

#define CAP_SYS_PTRACE       19

/* Allow configuration of process accounting */

#define CAP_SYS_PACCT        20

/* Allow configuration of the secure attention key */
/* Allow administration of the random device */
/* Allow examination and configuration of disk quotas */
/* Allow setting the domainname */
/* Allow setting the hostname */
/* Allow calling bdflush() */
/* Allow mount() and umount(), setting up new smb connection */
/* Allow some autofs root ioctls */
/* Allow nfsservctl */
/* Allow VM86_REQUEST_IRQ */
/* Allow to read/write pci config on alpha */
/* Allow irix_prctl on mips (setstacksize) */
/* Allow flushing all cache on m68k (sys_cacheflush) */
/* Allow removing semaphores */
/* Used instead of CAP_CHOWN to "chown" IPC message queues, semaphores
   and shared memory */
/* Allow locking/unlocking of shared memory segment */
/* Allow turning swap on/off */
/* Allow forged pids on socket credentials passing */
/* Allow setting readahead and flushing buffers on block devices */
/* Allow setting geometry in floppy driver */
/* Allow turning DMA on/off in xd driver */
/* Allow administration of md devices (mostly the above, but some
   extra ioctls) */
/* Allow tuning the ide driver */
/* Allow access to the nvram device */
/* Allow administration of apm_bios, serial and bttv (TV) device */
/* Allow manufacturer commands in isdn CAPI support driver */
/* Allow reading non-standardized portions of pci configuration space */
/* Allow DDI debug ioctl on sbpcd driver */
/* Allow setting up serial ports */
/* Allow sending raw qic-117 commands */
/* Allow enabling/disabling tagged queuing on SCSI controllers and sending
   arbitrary SCSI commands */
/* Allow setting encryption key on loopback filesystem */
/* Allow setting zone reclaim policy */

#define CAP_SYS_ADMIN        21

/* Allow use of reboot() */

#define CAP_SYS_BOOT         22

/* Allow raising priority and setting priority on other (different
   UID) processes */
/* Allow use of FIFO and round-robin (realtime) scheduling on own
   processes and setting the scheduling algorithm used by another
   process. */
/* Allow setting cpu affinity on other processes */

#define CAP_SYS_NICE         23

/* Override resource limits. Set resource limits. */
/* Override quota limits. */
/* Override reserved space on ext2 filesystem */
/* Modify data journaling mode on ext3 filesystem (uses journaling
   resources) */
/* NOTE: ext2 honors fsuid when checking for resource overrides, so
   you can override using fsuid too */
/* Override size restrictions on IPC message queues */
/* Allow more than 64hz interrupts from the real-time clock */
/* Override max number of consoles on console allocation */
/* Override max number of keymaps */

#define CAP_SYS_RESOURCE     24

/* Allow manipulation of system clock */
/* Allow irix_stime on mips */
/* Allow setting the real-time clock */

#define CAP_SYS_TIME         25

/* Allow configuration of tty devices */
/* Allow vhangup() of tty */

#define CAP_SYS_TTY_CONFIG   26

/* Allow the privileged aspects of mknod() */

#define CAP_MKNOD            27

/* Allow taking of leases on files */

#define CAP_LEASE            28

#define CAP_AUDIT_WRITE      29

#define CAP_AUDIT_CONTROL    30

#define CAP_SETFCAP	     31

/* Override MAC access.
   The base kernel enforces no MAC policy.
   An LSM may enforce a MAC policy, and if it does and it chooses
   to implement capability based overrides of that policy, this is
   the capability it should use to do so. */

#define CAP_MAC_OVERRIDE     32

/* Allow MAC configuration or state changes.
   The base kernel requires no MAC configuration.
   An LSM may enforce a MAC policy, and if it does and it chooses
   to implement capability based checks on modifications to that
   policy or the data required to maintain it, this is the
   capability it should use to do so. */

#define CAP_MAC_ADMIN        33

/* Allow configuring the kernel's syslog (printk behaviour) */

#define CAP_SYSLOG           34

/* Allow triggering something that will wake the system */

#define CAP_WAKE_ALARM            35

/* Allow preventing system suspends */

#define CAP_BLOCK_SUSPEND    36

#define CAP_LAST_CAP         CAP_BLOCK_SUSPEND

#define cap_valid(x) ((x) >= 0 && (x) <= CAP_LAST_CAP)

/*
 * Bit location of each capability (used by user-space library and kernel)
 */

#define CAP_TO_INDEX(x)     ((x) >> 5)        /* 1 << 5 == bits in __u32 */
#define CAP_TO_MASK(x)      (1 << ((x) & 31)) /* mask for indexed __u32 */

/**********************^**** NETLINK ETC *************************************/

struct sockaddr_nl {
    sa_family_t nl_family; /* AF_NETLINK	*/
    unsigned short nl_pad; /* zero		*/
    uint32_t nl_pid; /* process pid	*/
    uint32_t nl_groups; /* multicast groups mask */
};

struct nlmsghdr {
    uint32_t nlmsg_len; /* Length of message including header */
    uint16_t nlmsg_type; /* Message content */
    uint16_t nlmsg_flags; /* Additional flags */
    uint32_t nlmsg_seq; /* Sequence number */
    uint32_t nlmsg_pid; /* Sending process PID */
};

struct nlmsgerr {
    int error;
    struct nlmsghdr msg;
};

/* RFC 2863 operational status */
enum {
    IF_OPER_UNKNOWN,
    IF_OPER_NOTPRESENT,
    IF_OPER_DOWN,
    IF_OPER_LOWERLAYERDOWN,
    IF_OPER_TESTING,
    IF_OPER_DORMANT,
    IF_OPER_UP,
};

/* link modes */
enum {
    IF_LINK_MODE_DEFAULT,
    IF_LINK_MODE_DORMANT, /* limit upward transition to dormant */
};

/*
 *	Device mapping structure. I'd just gone off and designed a
 *	beautiful scheme using only loadable modules with arguments
 *	for driver options and along come the PCMCIA people 8)
 *
 *	Ah well. The get() side of this is good for WDSETUP, and it'll
 *	be handy for debugging things. The set side is fine for now and
 *	being very small might be worth keeping for clean configuration.
 */

struct if_settings {
    unsigned int type; /* Type of physical device or protocol */
    unsigned int size; /* Size of the data allocated by the caller */
    void* ifs_ifsu;
};

#define NETLINK_ROUTE		0	/* Routing/device hook				*/
#define NETLINK_UNUSED		1	/* Unused number				*/
#define NETLINK_USERSOCK	2	/* Reserved for user mode socket protocols 	*/
#define NETLINK_FIREWALL	3	/* Firewalling hook				*/
#define NETLINK_INET_DIAG	4	/* INET socket monitoring			*/
#define NETLINK_NFLOG		5	/* netfilter/iptables ULOG */
#define NETLINK_XFRM		6	/* ipsec */
#define NETLINK_SELINUX		7	/* SELinux event notifications */
#define NETLINK_ISCSI		8	/* Open-iSCSI */
#define NETLINK_AUDIT		9	/* auditing */
#define NETLINK_FIB_LOOKUP	10
#define NETLINK_CONNECTOR	11
#define NETLINK_NETFILTER	12	/* netfilter subsystem */
#define NETLINK_IP6_FW		13
#define NETLINK_DNRTMSG		14	/* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT	15	/* Kernel messages to userspace */
#define NETLINK_GENERIC		16
/* leave room for NETLINK_DM (DM Events) */
#define NETLINK_SCSITRANSPORT	18	/* SCSI Transports */
#define NETLINK_ECRYPTFS	19

#define MAX_LINKS 32

/* Flags values */

#define NLM_F_REQUEST		1	/* It is request message. 	*/
#define NLM_F_MULTI		2	/* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK		4	/* Reply with ack, with zero or error code */
#define NLM_F_ECHO		8	/* Echo this request 		*/

/* Modifiers to GET request */
#define NLM_F_ROOT	0x100	/* specify tree	root	*/
#define NLM_F_MATCH	0x200	/* return all matching	*/
#define NLM_F_ATOMIC	0x400	/* atomic GET		*/
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

/* Modifiers to NEW request */
#define NLM_F_REPLACE	0x100	/* Override existing		*/
#define NLM_F_EXCL	0x200	/* Do not touch, if it exists	*/
#define NLM_F_CREATE	0x400	/* Create, if it does not exist	*/
#define NLM_F_APPEND	0x800	/* Add to end of list		*/

#define NLMSG_ALIGNTO	4
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_HDRLEN	 ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(len) ((len)+NLMSG_ALIGN(NLMSG_HDRLEN))
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len)	 ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
				  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP		0x1	/* Nothing.		*/
#define NLMSG_ERROR		0x2	/* Error		*/
#define NLMSG_DONE		0x3	/* End of a dump	*/
#define NLMSG_OVERRUN		0x4	/* Data lost		*/

#define NLMSG_MIN_TYPE		0x10	/* < 0x10: reserved control messages */

#define NETLINK_ADD_MEMBERSHIP	1
#define NETLINK_DROP_MEMBERSHIP	2
#define NETLINK_PKTINFO		3

struct nl_pktinfo {
    uint32_t group;
};

#define NET_MAJOR 36		/* Major 36 is reserved for networking 						*/

enum {
    NETLINK_UNCONNECTED = 0,
    NETLINK_CONNECTED,
};

/*
 *  <------- NLA_HDRLEN ------> <-- NLA_ALIGN(payload)-->
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 * |        Header       | Pad |     Payload       | Pad |
 * |   (struct nlattr)   | ing |                   | ing |
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 *  <-------------- nlattr->nla_len -------------->
 */

struct nlattr {
    uint16_t nla_len;
    uint16_t nla_type;
};

#define NLA_ALIGNTO		4
#define NLA_ALIGN(len)		(((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN		((int) NLA_ALIGN(sizeof(struct nlattr)))

/* rtnetlink families. Values up to 127 are reserved for real address
 * families, values above 128 may be used arbitrarily.
 */
#define RTNL_FAMILY_IPMR		128
#define RTNL_FAMILY_IP6MR		129
#define RTNL_FAMILY_MAX			129

/****
 *		Routing/neighbour discovery messages.
 ****/

/* Types of messages */

enum {
    RTM_BASE = 16,
#define RTM_BASE	RTM_BASE

    RTM_NEWLINK = 16,
#define RTM_NEWLINK	RTM_NEWLINK
    RTM_DELLINK,
#define RTM_DELLINK	RTM_DELLINK
    RTM_GETLINK,
#define RTM_GETLINK	RTM_GETLINK
    RTM_SETLINK,
#define RTM_SETLINK	RTM_SETLINK

    RTM_NEWADDR = 20,
#define RTM_NEWADDR	RTM_NEWADDR
    RTM_DELADDR,
#define RTM_DELADDR	RTM_DELADDR
    RTM_GETADDR,
#define RTM_GETADDR	RTM_GETADDR

    RTM_NEWROUTE = 24,
#define RTM_NEWROUTE	RTM_NEWROUTE
    RTM_DELROUTE,
#define RTM_DELROUTE	RTM_DELROUTE
    RTM_GETROUTE,
#define RTM_GETROUTE	RTM_GETROUTE

    RTM_NEWNEIGH = 28,
#define RTM_NEWNEIGH	RTM_NEWNEIGH
    RTM_DELNEIGH,
#define RTM_DELNEIGH	RTM_DELNEIGH
    RTM_GETNEIGH,
#define RTM_GETNEIGH	RTM_GETNEIGH

    RTM_NEWRULE = 32,
#define RTM_NEWRULE	RTM_NEWRULE
    RTM_DELRULE,
#define RTM_DELRULE	RTM_DELRULE
    RTM_GETRULE,
#define RTM_GETRULE	RTM_GETRULE

    RTM_NEWQDISC = 36,
#define RTM_NEWQDISC	RTM_NEWQDISC
    RTM_DELQDISC,
#define RTM_DELQDISC	RTM_DELQDISC
    RTM_GETQDISC,
#define RTM_GETQDISC	RTM_GETQDISC

    RTM_NEWTCLASS = 40,
#define RTM_NEWTCLASS	RTM_NEWTCLASS
    RTM_DELTCLASS,
#define RTM_DELTCLASS	RTM_DELTCLASS
    RTM_GETTCLASS,
#define RTM_GETTCLASS	RTM_GETTCLASS

    RTM_NEWTFILTER = 44,
#define RTM_NEWTFILTER	RTM_NEWTFILTER
    RTM_DELTFILTER,
#define RTM_DELTFILTER	RTM_DELTFILTER
    RTM_GETTFILTER,
#define RTM_GETTFILTER	RTM_GETTFILTER

    RTM_NEWACTION = 48,
#define RTM_NEWACTION   RTM_NEWACTION
    RTM_DELACTION,
#define RTM_DELACTION   RTM_DELACTION
    RTM_GETACTION,
#define RTM_GETACTION   RTM_GETACTION

    RTM_NEWPREFIX = 52,
#define RTM_NEWPREFIX	RTM_NEWPREFIX

    RTM_GETMULTICAST = 58,
#define RTM_GETMULTICAST RTM_GETMULTICAST

    RTM_GETANYCAST = 62,
#define RTM_GETANYCAST	RTM_GETANYCAST

    RTM_NEWNEIGHTBL = 64,
#define RTM_NEWNEIGHTBL	RTM_NEWNEIGHTBL
    RTM_GETNEIGHTBL = 66,
#define RTM_GETNEIGHTBL	RTM_GETNEIGHTBL
    RTM_SETNEIGHTBL,
#define RTM_SETNEIGHTBL	RTM_SETNEIGHTBL

    RTM_NEWNDUSEROPT = 68,
#define RTM_NEWNDUSEROPT RTM_NEWNDUSEROPT

    RTM_NEWADDRLABEL = 72,
#define RTM_NEWADDRLABEL RTM_NEWADDRLABEL
    RTM_DELADDRLABEL,
#define RTM_DELADDRLABEL RTM_DELADDRLABEL
    RTM_GETADDRLABEL,
#define RTM_GETADDRLABEL RTM_GETADDRLABEL

    RTM_GETDCB = 78,
#define RTM_GETDCB RTM_GETDCB
    RTM_SETDCB,
#define RTM_SETDCB RTM_SETDCB

    __RTM_MAX,
#define RTM_MAX		(((__RTM_MAX + 3) & ~3) - 1)
};

#define RTM_NR_MSGTYPES	(RTM_MAX + 1 - RTM_BASE)
#define RTM_NR_FAMILIES	(RTM_NR_MSGTYPES >> 2)
#define RTM_FAM(cmd)	(((cmd) - RTM_BASE) >> 2)

/*
   Generic structure for encapsulation of optional route information.
   It is reminiscent of sockaddr, but with sa_family replaced
   with attribute type.
 */

struct rtattr {
    unsigned short rta_len;
    unsigned short rta_type;
};

/* Macros to handle rtattributes */

#define RTA_ALIGNTO	4
#define RTA_ALIGN(len) ( ((len)+RTA_ALIGNTO-1) & ~(RTA_ALIGNTO-1) )
#define RTA_OK(rta,len) ((len) >= (int)sizeof(struct rtattr) && \
			 (rta)->rta_len >= sizeof(struct rtattr) && \
			 (rta)->rta_len <= (len))
#define RTA_NEXT(rta,attrlen)	((attrlen) -= RTA_ALIGN((rta)->rta_len), \
				 (struct rtattr*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len)))
#define RTA_LENGTH(len)	(RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_SPACE(len)	RTA_ALIGN(RTA_LENGTH(len))
#define RTA_DATA(rta)   ((void*)(((char*)(rta)) + RTA_LENGTH(0)))
#define RTA_PAYLOAD(rta) ((int)((rta)->rta_len) - RTA_LENGTH(0))

/******************************************************************************
 *		Definitions used in routing table administration.
 ****/

struct rtmsg {
    unsigned char rtm_family;
    unsigned char rtm_dst_len;
    unsigned char rtm_src_len;
    unsigned char rtm_tos;

    unsigned char rtm_table; /* Routing table id */
    unsigned char rtm_protocol; /* Routing protocol; see below	*/
    unsigned char rtm_scope; /* See below */
    unsigned char rtm_type; /* See below	*/

    unsigned rtm_flags;
};

/* rtm_type */

enum {
    RTN_UNSPEC,
    RTN_UNICAST, /* Gateway or direct route	*/
    RTN_LOCAL, /* Accept locally		*/
    RTN_BROADCAST, /* Accept locally as broadcast,
				   send as broadcast */
    RTN_ANYCAST, /* Accept locally as broadcast,
				   but send as unicast */
    RTN_MULTICAST, /* Multicast route		*/
    RTN_BLACKHOLE, /* Drop				*/
    RTN_UNREACHABLE, /* Destination is unreachable   */
    RTN_PROHIBIT, /* Administratively prohibited	*/
    RTN_THROW, /* Not in this table		*/
    RTN_NAT, /* Translate this address	*/
    RTN_XRESOLVE, /* Use external resolver	*/
    __RTN_MAX
};

#define RTN_MAX (__RTN_MAX - 1)


/* rtm_protocol */

#define RTPROT_UNSPEC	0
#define RTPROT_REDIRECT	1	/* Route installed by ICMP redirects;
				   not used by current IPv4 */
#define RTPROT_KERNEL	2	/* Route installed by kernel		*/
#define RTPROT_BOOT	3	/* Route installed during boot		*/
#define RTPROT_STATIC	4	/* Route installed by administrator	*/

/* Values of protocol >= RTPROT_STATIC are not interpreted by kernel;
   they are just passed from user and back as is.
   It will be used by hypothetical multiple routing daemons.
   Note that protocol values should be standardized in order to
   avoid conflicts.
 */

#define RTPROT_GATED	8	/* Apparently, GateD */
#define RTPROT_RA	9	/* RDISC/ND router advertisements */
#define RTPROT_MRT	10	/* Merit MRT */
#define RTPROT_ZEBRA	11	/* Zebra */
#define RTPROT_BIRD	12	/* BIRD */
#define RTPROT_DNROUTED	13	/* DECnet routing daemon */
#define RTPROT_XORP	14	/* XORP */
#define RTPROT_NTK	15	/* Netsukuku */
#define RTPROT_DHCP	16      /* DHCP client */

/* rtm_scope

   Really it is not scope, but sort of distance to the destination.
   NOWHERE are reserved for not existing destinations, HOST is our
   local addresses, LINK are destinations, located on directly attached
   link and UNIVERSE is everywhere in the Universe.

   Intermediate values are also possible f.e. interior routes
   could be assigned a value between UNIVERSE and LINK.
 */

enum rt_scope_t {
    RT_SCOPE_UNIVERSE = 0,
    /* User defined values  */
    RT_SCOPE_SITE = 200,
    RT_SCOPE_LINK = 253,
    RT_SCOPE_HOST = 254,
    RT_SCOPE_NOWHERE = 255
};

/* rtm_flags */

#define RTM_F_NOTIFY		0x100	/* Notify user of route change	*/
#define RTM_F_CLONED		0x200	/* This route is cloned		*/
#define RTM_F_EQUALIZE		0x400	/* Multipath equalizer: NI	*/
#define RTM_F_PREFIX		0x800	/* Prefix addresses		*/

/* Reserved table identifiers */

enum rt_class_t {
    RT_TABLE_UNSPEC = 0,
    /* User defined values */
    RT_TABLE_COMPAT = 252,
    RT_TABLE_DEFAULT = 253,
    RT_TABLE_MAIN = 254,
    RT_TABLE_LOCAL = 255,
    RT_TABLE_MAX = 0xFFFFFFFF
};

/* Routing message attributes */

enum rtattr_type_t {
    RTA_UNSPEC,
    RTA_DST,
    RTA_SRC,
    RTA_IIF,
    RTA_OIF,
    RTA_GATEWAY,
    RTA_PRIORITY,
    RTA_PREFSRC,
    RTA_METRICS,
    RTA_MULTIPATH,
    RTA_PROTOINFO, /* no longer used */
    RTA_FLOW,
    RTA_CACHEINFO,
    RTA_SESSION, /* no longer used */
    RTA_MP_ALGO, /* no longer used */
    RTA_TABLE,
    RTA_MARK,
    __RTA_MAX
};

#define RTA_MAX (__RTA_MAX - 1)

#define RTM_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct rtmsg))))
#define RTM_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct rtmsg))

/* RTM_MULTIPATH --- array of struct rtnexthop.
 *
 * "struct rtnexthop" describes all necessary nexthop information,
 * i.e. parameters of path to a destination via this nexthop.
 *
 * At the moment it is impossible to set different prefsrc, mtu, window
 * and rtt for different paths from multipath.
 */

struct rtnexthop {
    unsigned short rtnh_len;
    unsigned char rtnh_flags;
    unsigned char rtnh_hops;
    int rtnh_ifindex;
};

/* rtnh_flags */

#define RTNH_F_DEAD		1	/* Nexthop is dead (used by multipath)	*/
#define RTNH_F_PERVASIVE	2	/* Do recursive gateway lookup	*/
#define RTNH_F_ONLINK		4	/* Gateway is forced on link	*/

/* Macros to handle hexthops */

#define RTNH_ALIGNTO	4
#define RTNH_ALIGN(len) ( ((len)+RTNH_ALIGNTO-1) & ~(RTNH_ALIGNTO-1) )
#define RTNH_OK(rtnh,len) ((rtnh)->rtnh_len >= sizeof(struct rtnexthop) && \
			   ((int)(rtnh)->rtnh_len) <= (len))
#define RTNH_NEXT(rtnh)	((struct rtnexthop*)(((char*)(rtnh)) + RTNH_ALIGN((rtnh)->rtnh_len)))
#define RTNH_LENGTH(len) (RTNH_ALIGN(sizeof(struct rtnexthop)) + (len))
#define RTNH_SPACE(len)	RTNH_ALIGN(RTNH_LENGTH(len))
#define RTNH_DATA(rtnh)   ((struct rtattr*)(((char*)(rtnh)) + RTNH_LENGTH(0)))

/* RTM_CACHEINFO */

struct rta_cacheinfo {
    uint32_t rta_clntref;
    uint32_t rta_lastuse;
    int32_t rta_expires;
    uint32_t rta_error;
    uint32_t rta_used;

#define RTNETLINK_HAVE_PEERINFO 1
    uint32_t rta_id;
    uint32_t rta_ts;
    uint32_t rta_tsage;
};

/* RTM_METRICS --- array of struct rtattr with types of RTAX_* */

enum {
    RTAX_UNSPEC,
#define RTAX_UNSPEC RTAX_UNSPEC
    RTAX_LOCK,
#define RTAX_LOCK RTAX_LOCK
    RTAX_MTU,
#define RTAX_MTU RTAX_MTU
    RTAX_WINDOW,
#define RTAX_WINDOW RTAX_WINDOW
    RTAX_RTT,
#define RTAX_RTT RTAX_RTT
    RTAX_RTTVAR,
#define RTAX_RTTVAR RTAX_RTTVAR
    RTAX_SSTHRESH,
#define RTAX_SSTHRESH RTAX_SSTHRESH
    RTAX_CWND,
#define RTAX_CWND RTAX_CWND
    RTAX_ADVMSS,
#define RTAX_ADVMSS RTAX_ADVMSS
    RTAX_REORDERING,
#define RTAX_REORDERING RTAX_REORDERING
    RTAX_HOPLIMIT,
#define RTAX_HOPLIMIT RTAX_HOPLIMIT
    RTAX_INITCWND,
#define RTAX_INITCWND RTAX_INITCWND
    RTAX_FEATURES,
#define RTAX_FEATURES RTAX_FEATURES
    RTAX_RTO_MIN,
#define RTAX_RTO_MIN RTAX_RTO_MIN
    RTAX_INITRWND,
#define RTAX_INITRWND RTAX_INITRWND
    __RTAX_MAX
};

#define RTAX_MAX (__RTAX_MAX - 1)

#define RTAX_FEATURE_ECN	0x00000001
#define RTAX_FEATURE_SACK	0x00000002
#define RTAX_FEATURE_TIMESTAMP	0x00000004
#define RTAX_FEATURE_ALLFRAG	0x00000008

struct rta_session {
    uint8_t proto;
    uint8_t pad1;
    uint16_t pad2;

    union {

        struct {
            uint16_t sport;
            uint16_t dport;
        } ports;

        struct {
            uint8_t type;
            uint8_t code;
            uint16_t ident;
        } icmpt;

        uint32_t spi;
    } u;
};

/****
 *		General form of address family dependent message.
 ****/

struct rtgenmsg {
    unsigned char rtgen_family;
};

/*****************************************************************
 *		Link layer specific messages.
 ****/

/* struct ifinfomsg
 * passes link level specific information, not dependent
 * on network protocol.
 */

struct ifinfomsg {
    unsigned char ifi_family;
    unsigned char __ifi_pad;
    unsigned short ifi_type; /* ARPHRD_* */
    int ifi_index; /* Link index	*/
    unsigned ifi_flags; /* IFF_* flags	*/
    unsigned ifi_change; /* IFF_* change mask */
};

/********************************************************************
 *		prefix information
 ****/

struct prefixmsg {
    unsigned char prefix_family;
    unsigned char prefix_pad1;
    unsigned short prefix_pad2;
    int prefix_ifindex;
    unsigned char prefix_type;
    unsigned char prefix_len;
    unsigned char prefix_flags;
    unsigned char prefix_pad3;
};

enum {
    PREFIX_UNSPEC,
    PREFIX_ADDRESS,
    PREFIX_CACHEINFO,
    __PREFIX_MAX
};

#define PREFIX_MAX	(__PREFIX_MAX - 1)

struct prefix_cacheinfo {
    uint32_t preferred_time;
    uint32_t valid_time;
};

/*****************************************************************
 *		Traffic control messages.
 ****/

struct tcmsg {
    unsigned char tcm_family;
    unsigned char tcm__pad1;
    unsigned short tcm__pad2;
    int tcm_ifindex;
    uint32_t tcm_handle;
    uint32_t tcm_parent;
    uint32_t tcm_info;
};

enum {
    TCA_UNSPEC,
    TCA_KIND,
    TCA_OPTIONS,
    TCA_STATS,
    TCA_XSTATS,
    TCA_RATE,
    TCA_FCNT,
    TCA_STATS2,
    TCA_STAB,
    __TCA_MAX
};

#define TCA_MAX (__TCA_MAX - 1)

#define TCA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct tcmsg))))
#define TCA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct tcmsg))

/********************************************************************
 *		Neighbor Discovery userland options
 ****/

struct nduseroptmsg {
    unsigned char nduseropt_family;
    unsigned char nduseropt_pad1;
    unsigned short nduseropt_opts_len; /* Total length of options */
    int nduseropt_ifindex;
    uint8_t nduseropt_icmp_type;
    uint8_t nduseropt_icmp_code;
    unsigned short nduseropt_pad2;
    unsigned int nduseropt_pad3;
    /* Followed by one or more ND options */
};

enum {
    NDUSEROPT_UNSPEC,
    NDUSEROPT_SRCADDR,
    __NDUSEROPT_MAX
};

#define NDUSEROPT_MAX	(__NDUSEROPT_MAX - 1)

#ifndef __KERNEL__
/* RTnetlink multicast groups - backwards compatibility for userspace */
#define RTMGRP_LINK		1
#define RTMGRP_NOTIFY		2
#define RTMGRP_NEIGH		4
#define RTMGRP_TC		8

#define RTMGRP_IPV4_IFADDR	0x10
#define RTMGRP_IPV4_MROUTE	0x20
#define RTMGRP_IPV4_ROUTE	0x40
#define RTMGRP_IPV4_RULE	0x80

#define RTMGRP_IPV6_IFADDR	0x100
#define RTMGRP_IPV6_MROUTE	0x200
#define RTMGRP_IPV6_ROUTE	0x400
#define RTMGRP_IPV6_IFINFO	0x800

#define RTMGRP_DECnet_IFADDR    0x1000
#define RTMGRP_DECnet_ROUTE     0x4000

#define RTMGRP_IPV6_PREFIX	0x20000
#endif

/* RTnetlink multicast groups */
enum rtnetlink_groups {
    RTNLGRP_NONE,
#define RTNLGRP_NONE		RTNLGRP_NONE
    RTNLGRP_LINK,
#define RTNLGRP_LINK		RTNLGRP_LINK
    RTNLGRP_NOTIFY,
#define RTNLGRP_NOTIFY		RTNLGRP_NOTIFY
    RTNLGRP_NEIGH,
#define RTNLGRP_NEIGH		RTNLGRP_NEIGH
    RTNLGRP_TC,
#define RTNLGRP_TC		RTNLGRP_TC
    RTNLGRP_IPV4_IFADDR,
#define RTNLGRP_IPV4_IFADDR	RTNLGRP_IPV4_IFADDR
    RTNLGRP_IPV4_MROUTE,
#define	RTNLGRP_IPV4_MROUTE	RTNLGRP_IPV4_MROUTE
    RTNLGRP_IPV4_ROUTE,
#define RTNLGRP_IPV4_ROUTE	RTNLGRP_IPV4_ROUTE
    RTNLGRP_IPV4_RULE,
#define RTNLGRP_IPV4_RULE	RTNLGRP_IPV4_RULE
    RTNLGRP_IPV6_IFADDR,
#define RTNLGRP_IPV6_IFADDR	RTNLGRP_IPV6_IFADDR
    RTNLGRP_IPV6_MROUTE,
#define RTNLGRP_IPV6_MROUTE	RTNLGRP_IPV6_MROUTE
    RTNLGRP_IPV6_ROUTE,
#define RTNLGRP_IPV6_ROUTE	RTNLGRP_IPV6_ROUTE
    RTNLGRP_IPV6_IFINFO,
#define RTNLGRP_IPV6_IFINFO	RTNLGRP_IPV6_IFINFO
    RTNLGRP_DECnet_IFADDR,
#define RTNLGRP_DECnet_IFADDR	RTNLGRP_DECnet_IFADDR
    RTNLGRP_NOP2,
    RTNLGRP_DECnet_ROUTE,
#define RTNLGRP_DECnet_ROUTE	RTNLGRP_DECnet_ROUTE
    RTNLGRP_DECnet_RULE,
#define RTNLGRP_DECnet_RULE	RTNLGRP_DECnet_RULE
    RTNLGRP_NOP4,
    RTNLGRP_IPV6_PREFIX,
#define RTNLGRP_IPV6_PREFIX	RTNLGRP_IPV6_PREFIX
    RTNLGRP_IPV6_RULE,
#define RTNLGRP_IPV6_RULE	RTNLGRP_IPV6_RULE
    RTNLGRP_ND_USEROPT,
#define RTNLGRP_ND_USEROPT	RTNLGRP_ND_USEROPT
    RTNLGRP_PHONET_IFADDR,
#define RTNLGRP_PHONET_IFADDR	RTNLGRP_PHONET_IFADDR
    RTNLGRP_PHONET_ROUTE,
#define RTNLGRP_PHONET_ROUTE	RTNLGRP_PHONET_ROUTE
    RTNLGRP_DCB,
#define RTNLGRP_DCB		RTNLGRP_DCB
    __RTNLGRP_MAX
};
#define RTNLGRP_MAX	(__RTNLGRP_MAX - 1)

/* TC action piece */
struct tcamsg {
    unsigned char tca_family;
    unsigned char tca__pad1;
    unsigned short tca__pad2;
};
#define TA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct tcamsg))))
#define TA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct tcamsg))
#define TCA_ACT_TAB 1 /* attr type must be >=1 */
#define TCAA_MAX 1

/* New extended info filters for IFLA_EXT_MASK */
#define RTEXT_FILTER_VF		(1 << 0)

struct ifaddrmsg {
    uint8_t ifa_family;
    uint8_t ifa_prefixlen; /* The prefix length		*/
    uint8_t ifa_flags; /* Flags			*/
    uint8_t ifa_scope; /* Address scope		*/
    uint32_t ifa_index; /* Link index			*/
};

/*
 * Important comment:
 * IFA_ADDRESS is prefix address, rather than local interface address.
 * It makes no difference for normally configured broadcast interfaces,
 * but for point-to-point IFA_ADDRESS is DESTINATION address,
 * local address is supplied in IFA_LOCAL attribute.
 */
enum {
    IFA_UNSPEC,
    IFA_ADDRESS,
    IFA_LOCAL,
    IFA_LABEL,
    IFA_BROADCAST,
    IFA_ANYCAST,
    IFA_CACHEINFO,
    IFA_MULTICAST,
    __IFA_MAX,
};

#define IFA_MAX (__IFA_MAX - 1)

/* ifa_flags */
#define IFA_F_SECONDARY		0x01
#define IFA_F_TEMPORARY		IFA_F_SECONDARY

#define	IFA_F_NODAD		0x02
#define IFA_F_OPTIMISTIC	0x04
#define IFA_F_DADFAILED		0x08
#define	IFA_F_HOMEADDRESS	0x10
#define IFA_F_DEPRECATED	0x20
#define IFA_F_TENTATIVE		0x40
#define IFA_F_PERMANENT		0x80

struct ifa_cacheinfo {
    uint32_t ifa_prefered;
    uint32_t ifa_valid;
    uint32_t cstamp; /* created timestamp, hundredths of seconds */
    uint32_t tstamp; /* updated timestamp, hundredths of seconds */
};

/* backwards compatibility for userspace */
#ifndef __KERNEL__
#define IFA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define IFA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifaddrmsg))
#endif

/* This struct should be in sync with struct rtnl_link_stats64 */
struct rtnl_link_stats {
    uint32_t rx_packets; /* total packets received	*/
    uint32_t tx_packets; /* total packets transmitted	*/
    uint32_t rx_bytes; /* total bytes received 	*/
    uint32_t tx_bytes; /* total bytes transmitted	*/
    uint32_t rx_errors; /* bad packets received		*/
    uint32_t tx_errors; /* packet transmit problems	*/
    uint32_t rx_dropped; /* no space in linux buffers	*/
    uint32_t tx_dropped; /* no space available in linux	*/
    uint32_t multicast; /* multicast packets received	*/
    uint32_t collisions;

    /* detailed rx_errors: */
    uint32_t rx_length_errors;
    uint32_t rx_over_errors; /* receiver ring buff overflow	*/
    uint32_t rx_crc_errors; /* recved pkt with crc error	*/
    uint32_t rx_frame_errors; /* recv'd frame alignment error */
    uint32_t rx_fifo_errors; /* recv'r fifo overrun		*/
    uint32_t rx_missed_errors; /* receiver missed packet	*/

    /* detailed tx_errors */
    uint32_t tx_aborted_errors;
    uint32_t tx_carrier_errors;
    uint32_t tx_fifo_errors;
    uint32_t tx_heartbeat_errors;
    uint32_t tx_window_errors;

    /* for cslip etc */
    uint32_t rx_compressed;
    uint32_t tx_compressed;
};

/* The main device statistics structure */
struct rtnl_link_stats64 {
    uint64_t rx_packets; /* total packets received	*/
    uint64_t tx_packets; /* total packets transmitted	*/
    uint64_t rx_bytes; /* total bytes received 	*/
    uint64_t tx_bytes; /* total bytes transmitted	*/
    uint64_t rx_errors; /* bad packets received		*/
    uint64_t tx_errors; /* packet transmit problems	*/
    uint64_t rx_dropped; /* no space in linux buffers	*/
    uint64_t tx_dropped; /* no space available in linux	*/
    uint64_t multicast; /* multicast packets received	*/
    uint64_t collisions;

    /* detailed rx_errors: */
    uint64_t rx_length_errors;
    uint64_t rx_over_errors; /* receiver ring buff overflow	*/
    uint64_t rx_crc_errors; /* recved pkt with crc error	*/
    uint64_t rx_frame_errors; /* recv'd frame alignment error */
    uint64_t rx_fifo_errors; /* recv'r fifo overrun		*/
    uint64_t rx_missed_errors; /* receiver missed packet	*/

    /* detailed tx_errors */
    uint64_t tx_aborted_errors;
    uint64_t tx_carrier_errors;
    uint64_t tx_fifo_errors;
    uint64_t tx_heartbeat_errors;
    uint64_t tx_window_errors;

    /* for cslip etc */
    uint64_t rx_compressed;
    uint64_t tx_compressed;
};

/* The struct should be in sync with struct ifmap */
struct rtnl_link_ifmap {
    uint64_t mem_start;
    uint64_t mem_end;
    uint64_t base_addr;
    uint16_t irq;
    uint8_t dma;
    uint8_t port;
};

/*
 * IFLA_AF_SPEC
 *   Contains nested attributes for address family specific attributes.
 *   Each address family may create a attribute with the address family
 *   number as type and create its own attribute structure in it.
 *
 *   Example:
 *   [IFLA_AF_SPEC] = {
 *       [AF_INET] = {
 *           [IFLA_INET_CONF] = ...,
 *       },
 *       [AF_INET6] = {
 *           [IFLA_INET6_FLAGS] = ...,
 *           [IFLA_INET6_CONF] = ...,
 *       }
 *   }
 */

enum {
    IFLA_UNSPEC,
    IFLA_ADDRESS,
    IFLA_BROADCAST,
    IFLA_IFNAME,
    IFLA_MTU,
    IFLA_LINK,
    IFLA_QDISC,
    IFLA_STATS,
    IFLA_COST,
#define IFLA_COST IFLA_COST
    IFLA_PRIORITY,
#define IFLA_PRIORITY IFLA_PRIORITY
    IFLA_MASTER,
#define IFLA_MASTER IFLA_MASTER
    IFLA_WIRELESS, /* Wireless Extension event - see wireless.h */
#define IFLA_WIRELESS IFLA_WIRELESS
    IFLA_PROTINFO, /* Protocol specific information for a link */
#define IFLA_PROTINFO IFLA_PROTINFO
    IFLA_TXQLEN,
#define IFLA_TXQLEN IFLA_TXQLEN
    IFLA_MAP,
#define IFLA_MAP IFLA_MAP
    IFLA_WEIGHT,
#define IFLA_WEIGHT IFLA_WEIGHT
    IFLA_OPERSTATE,
    IFLA_LINKMODE,
    IFLA_LINKINFO,
#define IFLA_LINKINFO IFLA_LINKINFO
    IFLA_NET_NS_PID,
    IFLA_IFALIAS,
    IFLA_NUM_VF, /* Number of VFs if device is SR-IOV PF */
    IFLA_VFINFO_LIST,
    IFLA_STATS64,
    IFLA_VF_PORTS,
    IFLA_PORT_SELF,
    IFLA_AF_SPEC,
    IFLA_GROUP, /* Group the device belongs to */
    IFLA_NET_NS_FD,
    IFLA_EXT_MASK, /* Extended info mask, VFs, etc */
    IFLA_PROMISCUITY, /* Promiscuity count: > 0 means acts PROMISC */
#define IFLA_PROMISCUITY IFLA_PROMISCUITY
    IFLA_NUM_TX_QUEUES,
    IFLA_NUM_RX_QUEUES,
    __IFLA_MAX
};


#define IFLA_MAX (__IFLA_MAX - 1)

/* backwards compatibility for userspace */
#ifndef __KERNEL__
#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFLA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))
#endif

enum {
    IFLA_INET_UNSPEC,
    IFLA_INET_CONF,
    __IFLA_INET_MAX,
};

#define IFLA_INET_MAX (__IFLA_INET_MAX - 1)

/* ifi_flags.

   IFF_* flags.

   The only change is:
   IFF_LOOPBACK, IFF_BROADCAST and IFF_POINTOPOINT are
   more not changeable by user. They describe link media
   characteristics and set by device driver.

   Comments:
   - Combination IFF_BROADCAST|IFF_POINTOPOINT is invalid
   - If neither of these three flags are set;
     the interface is NBMA.

   - IFF_MULTICAST does not mean anything special:
   multicasts can be used on all not-NBMA links.
   IFF_MULTICAST means that this media uses special encapsulation
   for multicast frames. Apparently, all IFF_POINTOPOINT and
   IFF_BROADCAST devices are able to use multicasts too.
 */

/* IFLA_LINK.
   For usual devices it is equal ifi_index.
   If it is a "virtual interface" (f.e. tunnel), ifi_link
   can point to real physical interface (f.e. for bandwidth calculations),
   or maybe 0, what means, that real media is unknown (usual
   for IPIP tunnels, when route to endpoint is allowed to change)
 */

/* Subtype attributes for IFLA_PROTINFO */
enum {
    IFLA_INET6_UNSPEC,
    IFLA_INET6_FLAGS, /* link flags			*/
    IFLA_INET6_CONF, /* sysctl parameters		*/
    IFLA_INET6_STATS, /* statistics			*/
    IFLA_INET6_MCAST, /* MC things. What of them?	*/
    IFLA_INET6_CACHEINFO, /* time values and max reasm size */
    IFLA_INET6_ICMP6STATS, /* statistics (icmpv6)		*/
    __IFLA_INET6_MAX
};

#define IFLA_INET6_MAX	(__IFLA_INET6_MAX - 1)

struct ifla_cacheinfo {
    uint32_t max_reasm_len;
    uint32_t tstamp; /* ipv6InterfaceTable updated timestamp */
    uint32_t reachable_time;
    uint32_t retrans_time;
};

enum {
    IFLA_INFO_UNSPEC,
    IFLA_INFO_KIND,
    IFLA_INFO_DATA,
    IFLA_INFO_XSTATS,
    __IFLA_INFO_MAX,
};

#define IFLA_INFO_MAX	(__IFLA_INFO_MAX - 1)

/* VLAN section */

enum {
    IFLA_VLAN_UNSPEC,
    IFLA_VLAN_ID,
    IFLA_VLAN_FLAGS,
    IFLA_VLAN_EGRESS_QOS,
    IFLA_VLAN_INGRESS_QOS,
    __IFLA_VLAN_MAX,
};

#define IFLA_VLAN_MAX	(__IFLA_VLAN_MAX - 1)

struct ifla_vlan_flags {
    uint32_t flags;
    uint32_t mask;
};

enum {
    IFLA_VLAN_QOS_UNSPEC,
    IFLA_VLAN_QOS_MAPPING,
    __IFLA_VLAN_QOS_MAX
};

#define IFLA_VLAN_QOS_MAX	(__IFLA_VLAN_QOS_MAX - 1)

struct ifla_vlan_qos_mapping {
    uint32_t from;
    uint32_t to;
};

/* MACVLAN section */
enum {
    IFLA_MACVLAN_UNSPEC,
    IFLA_MACVLAN_MODE,
    IFLA_MACVLAN_FLAGS,
    __IFLA_MACVLAN_MAX,
};

#define IFLA_MACVLAN_MAX (__IFLA_MACVLAN_MAX - 1)

enum macvlan_mode {
    MACVLAN_MODE_PRIVATE = 1, /* don't talk to other macvlans */
    MACVLAN_MODE_VEPA = 2, /* talk to other ports through ext bridge */
    MACVLAN_MODE_BRIDGE = 4, /* talk to bridge ports directly */
    MACVLAN_MODE_PASSTHRU = 8, /* take over the underlying device */
};

#define MACVLAN_FLAG_NOPROMISC	1

/* VXLAN section */
enum {
    IFLA_VXLAN_UNSPEC,
    IFLA_VXLAN_ID,
    IFLA_VXLAN_GROUP,
    IFLA_VXLAN_LINK,
    IFLA_VXLAN_LOCAL,
    IFLA_VXLAN_TTL,
    IFLA_VXLAN_TOS,
    IFLA_VXLAN_LEARNING,
    IFLA_VXLAN_AGEING,
    IFLA_VXLAN_LIMIT,
    IFLA_VXLAN_PORT_RANGE,
    __IFLA_VXLAN_MAX
};
#define IFLA_VXLAN_MAX	(__IFLA_VXLAN_MAX - 1)

struct ifla_vxlan_port_range {
    uint16_t low;
    uint16_t high;
};

/* SR-IOV virtual function management section */

enum {
    IFLA_VF_INFO_UNSPEC,
    IFLA_VF_INFO,
    __IFLA_VF_INFO_MAX,
};

#define IFLA_VF_INFO_MAX (__IFLA_VF_INFO_MAX - 1)

enum {
    IFLA_VF_UNSPEC,
    IFLA_VF_MAC, /* Hardware queue specific attributes */
    IFLA_VF_VLAN,
    IFLA_VF_TX_RATE, /* TX Bandwidth Allocation */
    IFLA_VF_SPOOFCHK, /* Spoof Checking on/off switch */
    __IFLA_VF_MAX,
};

#define IFLA_VF_MAX (__IFLA_VF_MAX - 1)

struct ifla_vf_mac {
    uint32_t vf;
    uint8_t mac[32]; /* MAX_ADDR_LEN */
};

struct ifla_vf_vlan {
    uint32_t vf;
    uint32_t vlan; /* 0 - 4095, 0 disables VLAN filter */
    uint32_t qos;
};

struct ifla_vf_tx_rate {
    uint32_t vf;
    uint32_t rate; /* Max TX bandwidth in Mbps, 0 disables throttling */
};

struct ifla_vf_spoofchk {
    uint32_t vf;
    uint32_t setting;
};

/* VF ports management section
 *
 *	Nested layout of set/get msg is:
 *
 *		[IFLA_NUM_VF]
 *		[IFLA_VF_PORTS]
 *			[IFLA_VF_PORT]
 *				[IFLA_PORT_*], ...
 *			[IFLA_VF_PORT]
 *				[IFLA_PORT_*], ...
 *			...
 *		[IFLA_PORT_SELF]
 *			[IFLA_PORT_*], ...
 */

enum {
    IFLA_VF_PORT_UNSPEC,
    IFLA_VF_PORT, /* nest */
    __IFLA_VF_PORT_MAX,
};

#define IFLA_VF_PORT_MAX (__IFLA_VF_PORT_MAX - 1)

enum {
    IFLA_PORT_UNSPEC,
    IFLA_PORT_VF, /* uint32_t */
    IFLA_PORT_PROFILE, /* string */
    IFLA_PORT_VSI_TYPE, /* 802.1Qbg (pre-)standard VDP */
    IFLA_PORT_INSTANCE_UUID, /* binary UUID */
    IFLA_PORT_HOST_UUID, /* binary UUID */
    IFLA_PORT_REQUEST, /* uint8_t */
    IFLA_PORT_RESPONSE, /* __u16, output only */
    __IFLA_PORT_MAX,
};

#define IFLA_PORT_MAX (__IFLA_PORT_MAX - 1)

#define PORT_PROFILE_MAX	40
#define PORT_UUID_MAX		16
#define PORT_SELF_VF		-1

enum {
    PORT_REQUEST_PREASSOCIATE = 0,
    PORT_REQUEST_PREASSOCIATE_RR,
    PORT_REQUEST_ASSOCIATE,
    PORT_REQUEST_DISASSOCIATE,
};

enum {
    PORT_VDP_RESPONSE_SUCCESS = 0,
    PORT_VDP_RESPONSE_INVALID_FORMAT,
    PORT_VDP_RESPONSE_INSUFFICIENT_RESOURCES,
    PORT_VDP_RESPONSE_UNUSED_VTID,
    PORT_VDP_RESPONSE_VTID_VIOLATION,
    PORT_VDP_RESPONSE_VTID_VERSION_VIOALTION,
    PORT_VDP_RESPONSE_OUT_OF_SYNC,
    /* 0x08-0xFF reserved for future VDP use */
    PORT_PROFILE_RESPONSE_SUCCESS = 0x100,
    PORT_PROFILE_RESPONSE_INPROGRESS,
    PORT_PROFILE_RESPONSE_INVALID,
    PORT_PROFILE_RESPONSE_BADSTATE,
    PORT_PROFILE_RESPONSE_INSUFFICIENT_RESOURCES,
    PORT_PROFILE_RESPONSE_ERROR,
};

struct ifla_port_vsi {
    uint8_t vsi_mgr_id;
    uint8_t vsi_type_id[3];
    uint8_t vsi_type_version;
    uint8_t pad[3];
};

/* IPoIB section */

enum {
    IFLA_IPOIB_UNSPEC,
    IFLA_IPOIB_PKEY,
    IFLA_IPOIB_MODE,
    IFLA_IPOIB_UMCAST,
    __IFLA_IPOIB_MAX
};

enum {
    IPOIB_MODE_DATAGRAM = 0, /* using unreliable datagram QPs */
    IPOIB_MODE_CONNECTED = 1, /* using connected QPs */
};

#define IFLA_IPOIB_MAX (__IFLA_IPOIB_MAX - 1)

#define	IFALIASZ	256

/* Standard interface flags (netdevice->flags). */
#define	IFF_UP		0x1		/* interface is up		*/
#define	IFF_BROADCAST	0x2		/* broadcast address valid	*/
#define	IFF_DEBUG	0x4		/* turn on debugging		*/
#define	IFF_LOOPBACK	0x8		/* is a loopback net		*/
#define	IFF_POINTOPOINT	0x10		/* interface is has p-p link	*/
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers	*/
#define	IFF_RUNNING	0x40		/* interface RFC2863 OPER_UP	*/
#define	IFF_NOARP	0x80		/* no ARP protocol		*/
#define	IFF_PROMISC	0x100		/* receive all packets		*/
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets*/

#define IFF_MASTER	0x400		/* master of a load balancer 	*/
#define IFF_SLAVE	0x800		/* slave of a load balancer	*/

#define IFF_MULTICAST	0x1000		/* Supports multicast		*/

#define IFF_PORTSEL	0x2000          /* can set media type		*/
#define IFF_AUTOMEDIA	0x4000		/* auto media select active	*/
#define IFF_DYNAMIC	0x8000		/* dialup device with changing addresses*/

#define IFF_LOWER_UP	0x10000		/* driver signals L1 up		*/
#define IFF_DORMANT	0x20000		/* driver signals dormant	*/

#define IFF_ECHO	0x40000		/* echo sent packets		*/

/* Private (from user) interface flags (netdevice->priv_flags). */
#define IFF_802_1Q_VLAN 0x1             /* 802.1Q VLAN device.          */
#define IFF_EBRIDGE	0x2		/* Ethernet bridging device.	*/
#define IFF_SLAVE_INACTIVE	0x4	/* bonding slave not the curr. active */
#define IFF_MASTER_8023AD	0x8	/* bonding master, 802.3ad. 	*/
#define IFF_MASTER_ALB	0x10		/* bonding master, balance-alb.	*/
#define IFF_BONDING	0x20		/* bonding master or slave	*/
#define IFF_SLAVE_NEEDARP 0x40		/* need ARPs for validation	*/
#define IFF_ISATAP	0x80		/* ISATAP interface (RFC4214)	*/
#define IFF_MASTER_ARPMON 0x100		/* bonding master, ARP mon in use */
#define IFF_WAN_HDLC	0x200		/* WAN HDLC device		*/
#define IFF_XMIT_DST_RELEASE 0x400	/* dev_hard_start_xmit() is allowed to
					 * release skb->dst
					 */
#define IFF_DONT_BRIDGE 0x800		/* disallow bridging this ether dev */
#define IFF_DISABLE_NETPOLL	0x1000	/* disable netpoll at run-time */
#define IFF_MACVLAN_PORT	0x2000	/* device used as macvlan port */
#define IFF_BRIDGE_PORT	0x4000		/* device used as bridge port */
#define IFF_OVS_DATAPATH	0x8000	/* device used as Open vSwitch
					 * datapath port */
#define IFF_TX_SKB_SHARING	0x10000	/* The interface supports sharing
					 * skbs on transmit */
#define IFF_UNICAST_FLT	0x20000		/* Supports unicast filtering	*/
#define IFF_TEAM_PORT	0x40000		/* device used as team port */
#define IFF_SUPP_NOFCS	0x80000		/* device supports sending custom FCS */
#define IFF_LIVE_ADDR_CHANGE 0x100000	/* device supports hardware address
					 * change when it's running */


#define IF_GET_IFACE	0x0001		/* for querying only */
#define IF_GET_PROTO	0x0002

/* For definitions see hdlc.h */
#define IF_IFACE_V35	0x1000		/* V.35 serial interface	*/
#define IF_IFACE_V24	0x1001		/* V.24 serial interface	*/
#define IF_IFACE_X21	0x1002		/* X.21 serial interface	*/
#define IF_IFACE_T1	0x1003		/* T1 telco serial interface	*/
#define IF_IFACE_E1	0x1004		/* E1 telco serial interface	*/
#define IF_IFACE_SYNC_SERIAL 0x1005	/* can't be set by software	*/
#define IF_IFACE_X21D   0x1006          /* X.21 Dual Clocking (FarSite) */

/* For definitions see hdlc.h */
#define IF_PROTO_HDLC	0x2000		/* raw HDLC protocol		*/
#define IF_PROTO_PPP	0x2001		/* PPP protocol			*/
#define IF_PROTO_CISCO	0x2002		/* Cisco HDLC protocol		*/
#define IF_PROTO_FR	0x2003		/* Frame Relay protocol		*/
#define IF_PROTO_FR_ADD_PVC 0x2004	/*    Create FR PVC		*/
#define IF_PROTO_FR_DEL_PVC 0x2005	/*    Delete FR PVC		*/
#define IF_PROTO_X25	0x2006		/* X.25				*/
#define IF_PROTO_HDLC_ETH 0x2007	/* raw HDLC, Ethernet emulation	*/
#define IF_PROTO_FR_ADD_ETH_PVC 0x2008	/*  Create FR Ethernet-bridged PVC */
#define IF_PROTO_FR_DEL_ETH_PVC 0x2009	/*  Delete FR Ethernet-bridged PVC */
#define IF_PROTO_FR_PVC	0x200A		/* for reading PVC status	*/
#define IF_PROTO_FR_ETH_PVC 0x200B
#define IF_PROTO_RAW    0x200C          /* RAW Socket                   */


#define ifr_name	ifr_ifrn.ifrn_name	/* interface name 	*/
#define ifr_hwaddr	ifr_ifru.ifru_hwaddr	/* MAC address 		*/
#define	ifr_addr	ifr_ifru.ifru_addr	/* address		*/
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-p lnk	*/
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address	*/
#define	ifr_netmask	ifr_ifru.ifru_netmask	/* interface net mask	*/
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags		*/
#define	ifr_metric	ifr_ifru.ifru_ivalue	/* metric		*/
#define	ifr_mtu		ifr_ifru.ifru_mtu	/* mtu			*/
#define ifr_map		ifr_ifru.ifru_map	/* device map		*/
#define ifr_slave	ifr_ifru.ifru_slave	/* slave device		*/
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface	*/
#define ifr_ifindex	ifr_ifru.ifru_ivalue	/* interface index	*/
#define ifr_bandwidth	ifr_ifru.ifru_ivalue    /* link bandwidth	*/
#define ifr_qlen	ifr_ifru.ifru_ivalue	/* Queue length 	*/
#define ifr_newname	ifr_ifru.ifru_newname	/* New name		*/
#define ifr_settings	ifr_ifru.ifru_settings	/* Device/proto settings*/

#define	ifc_buf	ifc_ifcu.ifcu_buf		/* buffer address	*/
#define	ifc_req	ifc_ifcu.ifcu_req		/* array of structures	*/

#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_BATCH 3
#define SCHED_IDLE 5
#define SCHED_RESET_ON_FORK 0x40000000

#define CSIGNAL		0x000000ff
#define CLONE_VM	0x00000100
#define CLONE_FS	0x00000200
#define CLONE_FILES	0x00000400
#define CLONE_SIGHAND	0x00000800
#define CLONE_PTRACE	0x00002000
#define CLONE_VFORK	0x00004000
#define CLONE_PARENT	0x00008000
#define CLONE_THREAD	0x00010000
#define CLONE_NEWNS	0x00020000
#define CLONE_SYSVSEM	0x00040000
#define CLONE_SETTLS	0x00080000
#define CLONE_PARENT_SETTID	0x00100000
#define CLONE_CHILD_CLEARTID	0x00200000
#define CLONE_DETACHED	0x00400000
#define CLONE_UNTRACED	0x00800000
#define CLONE_CHILD_SETTID	0x01000000
#define CLONE_NEWUTS	0x04000000
#define CLONE_NEWIPC	0x08000000
#define CLONE_NEWUSER	0x10000000
#define CLONE_NEWPID	0x20000000
#define CLONE_NEWNET	0x40000000
#define CLONE_IO	0x80000000

// Declare all syscalls in Linux as direct C wrappers.
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
/* int open(const char* pathname, int flags, mode_t mode); */
int close(int fd);
int stat(const char* path, struct stat* buf);
int fstat(int fd, struct stat* buf);
int lstat(const char* path, struct stat* buf);
int poll(struct pollfd* fds, nfds_t nfds, int timeout);
off_t lseek(int fd, off_t offset, int whence);
void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int mprotect(void* addr, size_t length, int prot);
int munmap(void* addr, size_t length);
int brk(void* addr);
int rt_sigaction(int signum, const struct k_sigaction *act, struct k_sigaction *oldact);
int rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int rt_sigreturn();
/*int ioctl(int d, int request, void* argp);*/
ssize_t pread(int fd, void* buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset);
ssize_t readv(int fd, const struct iovec* iov, int iovcnt);
ssize_t writev(int fd, const struct iovec* iov, int iovcnt);
int access(const char* pathname, int mode);
int pipe(int pipefd[2]);
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
int sched_yield();
void* mremap(void* old_address, size_t old_size, size_t new_size, int flags, void* new_address);
int msync(void* addr, size_t length, int flags);
int mincore(void* addr, size_t length, unsigned char* vec);
int madvise(void* addr, size_t length, int advice);
int shmget(key_t key, size_t size, int shmflg);
void* shmat(int shmid, const void* shmaddr, int shmflg);
int shmctl(int shmid, int cmd, struct shmid_ds* buf);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int nanosleep(const struct timespec* req, struct timespec* rem);
/* SYS_getitimer 36 */
/* SYS_alarm 37 */
/* SYS_setitimer 38 */
pid_t getpid();
ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count);
int socket(int domain, int type, int protocol);
int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
/* SYS_accept 43 */
ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t sendmsg(int sockfd, const struct msghdr* msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags);
int shutdown(int sockfd, int how);
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
int socketpair(int domain, int type, int protocol, int sv[2]);
int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen);
/// clone() is deprecated. librcd implements it's own thread starting wrapper
/// _start_new_thread() which should not be used in normal librcd programs.
__attribute__((deprecated)) int clone(int (*)(void *), void *, int, void *, ...);
/// fork() is deprecated, no librcd implementation of it exists that would be safe. this is intentional.
/// its definition makes deprecated assumptions about the process that are not true in librcd, so it would not be possible to write an implementation.
__attribute__((deprecated)) pid_t fork();
/// vfork() is deprecated, no librcd implementation of it exists that would be safe. this is intentional.
/// its definition makes deprecated assumptions about the process that are not true in librcd, so it would not be possible to write an implementation.
__attribute__((deprecated)) pid_t vfork();
int execve(const char* filename, char* const argv[], char* const envp[]);
void _exit(int status);
int wait4(pid_t pid, int* status, int options, struct rusage* rusage);
int kill(pid_t pid, int sig);
int uname(struct utsname* buf);
/* SYS_semget 64 */
/* SYS_semop 65 */
/* SYS_semctl 66 */
int shmdt(const void *shmaddr);
/* SYS_msgget 68 */
/* SYS_msgsnd 69 */
/* SYS_msgrcv 70 */
/* SYS_msgctl 71 */
/* int fcntl(int fd, int cmd, long arg); */
int flock(int fd, int operation);
int fsync(int fd);
int fdatasync(int fd);
int truncate(const char* path, off_t length);
int ftruncate(int fd, off_t length);
int getdents(unsigned int fd, struct dirent* dirp, unsigned int count);
char* getcwd(char* buf, size_t size);
int chdir(const char* path);
int fchdir(int fd);
int rename(const char* oldpath, const char* newpath);
int mkdir(const char* pathname, mode_t mode);
int rmdir(const char* pathname);
/* int creat(const char* pathname, mode_t mode); */
int link(const char* oldpath, const char* newpath);
int unlink(const char* pathname);
int symlink(const char* oldpath, const char* newpath);
ssize_t readlink(const char* path, char* buf, size_t bufsiz);
int chmod(const char* path, mode_t mode);
int fchmod(int fd, mode_t mode);
int chown(const char* path, uid_t owner, gid_t group);
int fchown(int fd, uid_t owner, gid_t group);
int lchown(const char* path, uid_t owner, gid_t group);
mode_t umask(mode_t mask);
int gettimeofday(struct timeval* tv, struct timezone* tz);
int getrlimit(int resource, struct rlimit* rlim);
int getrusage(int who, struct rusage* usage);
int sysinfo(struct sysinfo* info);
clock_t times(struct tms* buf);
uid_t getuid();
gid_t getgid();
int setuid(uid_t uid);
int setgid(gid_t gid);
uid_t geteuid();
gid_t getegid();
int setpgid(pid_t pid, pid_t pgid);
pid_t getppid();
pid_t getpgrp();
pid_t setsid();
int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);
int getgroups(int size, gid_t list[]);
int setgroups(size_t size, const gid_t* list);
int setresuid(uid_t ruid, uid_t euid, uid_t suid);
int getresuid(uid_t* ruid, uid_t* euid, uid_t* suid);
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
int getresgid(gid_t* rgid, gid_t* egid, gid_t* sgid);
pid_t getpgid(pid_t pid);
int setfsuid(uid_t fsuid);
int setfsgid(uid_t fsgid);
pid_t getsid(pid_t pid);
/* int capget(cap_user_header_t hdrp, cap_user_data_t datap); */
/* int capset(cap_user_header_t hdrp, const cap_user_data_t datap); */
int rt_sigpending(sigset_t* set);
int rt_sigtimedwait(const sigset_t* set, siginfo_t* info, const struct timespec* timeout);
int rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t* uinfo);
int rt_sigsuspend(const sigset_t* mask);
int sigaltstack(const stack_t* ss, stack_t* oss);
int utime(const char* filename, const struct utimbuf* times);
int mknod(const char* pathname, mode_t mode, dev_t dev);
int uselib(const char* library);
int personality(unsigned long persona);
int ustat(dev_t dev, struct ustat* ubuf);
int statfs(const char* path, struct statfs* buf);
int fstatfs(int fd, struct statfs* buf);
/* int sysfs(int option); */
int getpriority(int which, int who);
int setpriority(int which, int who, int prio);
int sched_setparam(pid_t pid, const struct sched_param* param);
int sched_getparam(pid_t pid, struct sched_param* param);
int sched_setscheduler(pid_t pid, int policy, const struct sched_param* param);
int sched_getscheduler(pid_t pid);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_rr_get_interval(pid_t pid, struct timespec * tp);
int mlock(const void* addr, size_t len);
int munlock(const void* addr, size_t len);
int mlockall(int flags);
int munlockall();
int vhangup();
/* int modify_ldt(int func, void* ptr, unsigned long bytecount); */
int pivot_root(const char* new_root, const char* put_old);
/*int prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);*/
int arch_prctl(int code, unsigned long* addr);
/* int adjtimex(struct timex* buf); */
int setrlimit(int resource, const struct rlimit* rlim);
int chroot(const char* path);
void sync();
int acct(const char* filename);
int settimeofday(const struct timeval* tv, const struct timezone* tz);
int mount(const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data);
int umount2(__const char* __special_file, int __flags);
int swapon(const char* path, int swapflags);
int swapoff(const char* path);
int reboot(int magic, int magic2, int cmd, void* arg);
int sethostname(const char* name, size_t len);
int setdomainname(const char* name, size_t len);
int iopl(int level);
/* int ioperm(unsigned long from, unsigned long num, int turn_on); */
/* caddr_t create_module(const char* name, size_t size); */
/* int init_module(const char* name, struct module* image); */
/* int delete_module(const char* name); */
/* int get_kernel_syms(struct kernel_sym* table); */
/* int query_module(const char* name, int which, void* buf, size_t bufsize, size_t* ret); */
/* int quotactl(int cmd, const char* special, int id, caddr_t addr); */
/* long nfsservctl(int cmd, struct nfsctl_arg* argp, union nfsctl_res* resp); */
pid_t gettid();
/* ssize_t readahead(int fd, off64_t offset, size_t count); */
int setxattr(const char* path, const char* name, const void* value, size_t size, int flags);
int lsetxattr(const char* path, const char* name, const void* value, size_t size, int flags);
int fsetxattr(int fd, const char* name, const void* value, size_t size, int flags);
ssize_t getxattr(const char* path, const char* name, void* value, size_t size);
ssize_t lgetxattr(const char* path, const char* name, void* value, size_t size);
ssize_t fgetxattr(int fd, const char* name, void* value, size_t size);
ssize_t listxattr(const char* path, char* list, size_t size);
ssize_t llistxattr(const char* path, char* list, size_t size);
ssize_t flistxattr(int fd, char* list, size_t size);
int removexattr(const char* path, const char* name);
int lremovexattr(const char* path, const char* name);
int fremovexattr(int fd, const char* name);
int tkill(int tid, int sig);
time_t time(time_t* t);
int futex(int* uaddr, int op, int val, const struct timespec* timeout, int* uaddr2, int val3);
int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* mask);
int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* mask);
/* int set_thread_area(struct user_desc* u_info); */
/* int io_setup(unsigned nr_events, aio_context_t* ctxp); */
/* int io_destroy(aio_context_t ctx); */
/* int io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event* eventsstruct timespec* " timeout ); */
/* int io_submit(aio_context_t ctx_id, long nrstruct iocb* *" iocbpp ); */
/* int io_cancel(aio_context_t ctx_id, struct iocb* iocb, struct io_event* result); */
/* int get_thread_area(struct user_desc* u_info); */
/* int lookup_dcookie(u64 cookie, char* buffer, size_t len); */
/* SYS_epoll_create 213 */
/* int remap_file_pages(void* addr, size_t size, int prot, ssize_t pgoff, int flags); */
/* int getdents64(unsigned int fd, struct dirent* dirp, unsigned int count); */
long set_tid_address(int* tidptr);
long restart_syscall();
/* int semtimedop(int semid, struct sembuf* sops, unsigned nsops, struct timespec* timeout); */
int fadvise64(int fd, off_t offset, off_t len, int advice);
int timer_create(clockid_t clockid, struct sigevent* sevp, timer_t* timerid);
int timer_settime(timer_t timerid, int flags, const struct itimerspec* new_value, struct itimerspec * old_value);
int timer_gettime(timer_t timerid, struct itimerspec* curr_value);
int timer_getoverrun(timer_t timerid);
int timer_delete(timer_t timerid);
int clock_settime(clockid_t clk_id, const struct timespec* tp);
int clock_gettime(clockid_t clk_id, struct timespec* tp);
int clock_getres(clockid_t clk_id, struct timespec* res);
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec* request, struct timespec* remain);
void exit_group(int status);
int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
int tgkill(int tgid, int tid, int sig);
int utimes(const char* filename, const struct timeval times[2]);
/* SYS_vserver 236 */
/* SYS_mbind 237 */
/* SYS_set_mempolicy 238 */
/* SYS_get_mempolicy 239 */
/* SYS_mq_open 240 */
/* SYS_mq_unlink 241 */
/* SYS_mq_timedsend 242 */
/* SYS_mq_timedreceive 243 */
/* SYS_mq_notify 244 */
/* SYS_mq_getsetattr 245 */
/* SYS_kexec_load 246 */
int waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options, struct rusage* rusage);
/* SYS_add_key 248 */
/* SYS_request_key 249 */
/* SYS_keyctl 250 */
int ioprio_set(int which, int who, int ioprio);
int ioprio_get(int which, int who);
/* SYS_inotify_init 253 */
int inotify_add_watch(int fd, const char* pathname, uint32_t mask);
int inotify_rm_watch(int fd, int wd);
/* SYS_migrate_pages 256 */
/* int openat(int dirfd, const char *pathname, int flags, mode_t mode); */
int mkdirat(int dirfd, const char* pathname, mode_t mode);
int mknodat(int dirfd, const char* pathname, mode_t mode, dev_t dev);
int fchownat(int dirfd, const char* pathname, uid_t owner, gid_t group, int flags);
int futimesat(int dirfd, const char* pathname, const struct timeval times[2]);
int fstatat(int dirfd, const char* pathname, struct stat* buf, int flags);
int unlinkat(int dirfd, const char* pathname, int flags);
int renameat(int olddirfd, const char* oldpath, int newdirfd, const char* newpath);
int linkat(int olddirfd, const char* oldpath, int newdirfd, const char* newpath, int flags);
int symlinkat(const char* oldpath, int newdirfd, const char* newpath);
ssize_t readlinkat(int dirfd, const char* pathname, char* buf, size_t bufsiz);
int fchmodat(int dirfd, const char* pathname, mode_t mode, int flags);
int faccessat(int dirfd, const char* pathname, int mode, int flags);
int pselect6(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timespec* timeout, const sigset_t* sigmask);
int ppoll(struct pollfd* fds, nfds_t nfds, const struct timespec* timeout_ts, const sigset_t* sigmask);
int unshare(int flags);
/* SYS_set_robust_list 273 */
/* SYS_get_robust_list 274 */
ssize_t splice(int fd_in, loff_t* off_in, int fd_out, loff_t* off_out, size_t len, unsigned int flags);
ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags);
int sync_file_range(int fd, off64_t offset, off64_t nbytes, unsigned int flags);
ssize_t vmsplice(int fd, const struct iovec* iov, unsigned long nr_segs, unsigned int flags);
/* SYS_move_pages 279 */
int utimensat(int dirfd, const char* pathname, const struct timespec times[2], int flags);
int epoll_pwait(int epfd, struct epoll_event* events, int maxevents, int timeout, const sigset_t* sigmask);
/* SYS_signalfd 282 */
int timerfd_create(int clockid, int flags);
/* SYS_eventfd 284 */
int fallocate(int fd, int mode, off_t offset, off_t len);
int timerfd_settime(int fd, int flags, const struct itimerspec* new_value, struct itimerspec* old_value);
int timerfd_gettime(int fd, struct itimerspec* curr_value);
int accept4(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags);
int signalfd4(int ufd, const sigset_t* user_mask, int flags);
int eventfd2(unsigned int initval, int flags);
int epoll_create1(int flags);
int dup3(int oldfd, int newfd, int flags);
int pipe2(int pipefd[2], int flags);
int inotify_init1(int flags);
ssize_t preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset);
ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset);
int rt_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t* uinfo);
int recvmmsg(int sockfd, struct mmsghdr* msgvec, unsigned int vlen, unsigned int flags, struct timespec* timeout);
/* SYS_fanotify_init 300 */
/* SYS_fanotify_mark 301 */
/* SYS_prlimit64 302 */
/* SYS_name_to_handle_at 303 */
/* SYS_open_by_handle_at 304 */
/* SYS_clock_adjtime 305 */
int syncfs(int fd);
/* SYS_sendmmsg 307 */
/* SYS_setns 308 */
/* SYS_getcpu 309 */
/* SYS_process_vm_readv 310 */
/* SYS_process_vm_writev 311 */

/// Librcd wrapper for the mount syscall that uses fixed strings instead of
/// c strings. Throws an io exception if the mount fails.
void linux_mount(fstr_t source, fstr_t mnt_path, fstr_t fs_type, uint64_t mountflags, fstr_t data);

// Internal helper for syscall exceptions.
noret void _rcd_syscall_exception(fstr_t msg_start, rcd_exception_type_t type);

#endif	/* LINUX_H */
