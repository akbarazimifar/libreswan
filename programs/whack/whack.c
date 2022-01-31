/*
 * command interface to Pluto
 *
 * Copyright (C) 1997 Angelos D. Keromytis.
 * Copyright (C) 1998-2003  D. Hugh Redelmeier.
 * Copyright (C) 2004-2008 Michael Richardson <mcr@sandelman.ottawa.on.ca>
 * Copyright (C) 2007-2008 Paul Wouters <paul@xelerance.com>
 * Copyright (C) 2008 Shingo Yamawaki
 * Copyright (C) 2008-2009 David McCullough <david_mccullough@securecomputing.com>
 * Copyright (C) 2010-2019 D. Hugh Redelmeier <hugh@mimosa.com>
 * Copyright (C) 2011 Mika Ilmaranta <ilmis@foobar.fi>
 * Copyright (C) 2012-2020 Paul Wouters <pwouters@redhat.com>
 * Copyright (C) 2012 Philippe Vouters <philippe.vouters@laposte.net>
 * Copyright (C) 2013 David McCullough <ucdevel@gmail.com>
 * Copyright (C) 2013 Matt Rogers <mrogers@redhat.com>
 * Copyright (C) 2013-2018 Antony Antony <antony@phenome.org>
 * Copyright (C) 2017 Sahana Prasad <sahana.prasad07@gmail.com>
 * Copyright (C) 2019 Andrew Cagney <cagney@gnu.org>
 * Copyright (C) 2019 Tuomo Soini <tis@foobar.fi>
 * Copyright (C) 2020 Yulia Kuzovkova <ukuzovkova@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <https://www.gnu.org/licenses/gpl2.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <assert.h>
#include <limits.h>	/* for INT_MAX */

#include <libreswan.h>

#include "lswtool.h"
#include "sysdep.h"
#include "socketwrapper.h"
#include "constants.h"
#include "lswlog.h"
#include "whack.h"
#include "ip_address.h"
#include "ip_info.h"

#include "ipsecconf/confread.h" /* for DEFAULT_UPDOWN */
#include <net/if.h> /* for IFNAMSIZ */
/*
 * Print the 'ipsec --whack help' message
 */
static void help(void)
{
	fprintf(stderr,
		"Usage:\n"
		"\n"
		"all forms: [--rundir <path>] [--ctlsocket <file>] [--label <string>]\n"
		"\n"
		"help: whack [--help] [--version]\n"
		"\n"
		"connection: whack --name <connection_name> \\\n"
		"	--connalias <alias_names> \\\n"
		"	[--ipv4 | --ipv6] [--tunnelipv4 | --tunnelipv6] \\\n"
		"	(--host <ip-address> | --id <identity>) \\\n"
		"	[--ca <distinguished name>] \\\n"
		"	[--ikeport <port-number>] [--srcip <ip-address>] \\\n"
		"	[--vtiip <ip-address>/mask] \\\n"
		"	[--updown <updown>] \\\n"
		"	[--authby <psk | rsasig | ecdsa | null>] \\\n"
		"	[--autheap <none | tls>] \\\n"
		"	[--groups <access control groups>] \\\n"
		"	[--cert <friendly_name> | --ckaid <ckaid>] \\\n"
		"	[--ca <distinguished name>] \\\n"
		"	[--sendca no|issuer|all] [--sendcert yes|always|no|never|ifasked] \\\n"
		"	[--nexthop <ip-address>] \\\n"
		"	[--client <subnet> \\\n"
		"	[--clientprotoport <protocol>/<port>] \\\n"
		"	[--dnskeyondemand] [--updown <updown>] \\\n"
		"	[--psk] | [--rsasig] | [--rsa-sha1] | [--rsa-sha2] | \\\n"
		"		[--rsa-sha2_256] | [--rsa-sha2_384 ] | [--rsa-sha2_512 ] | \\\n"
		"		[ --auth-null] | [--auth-never] \\\n"
		"	[--encrypt] [--authenticate] [--compress] [--sha2-truncbug] \\\n"
		"	[--ms-dh-downgrade] \\\n"
		"	[--overlapip] [--tunnel] [--pfs] \\\n"
		"	[--allow-cert-without-san-id] [--dns-match-id] \\\n"
		"	[--pfsgroup <modp1024 | modp1536 | modp2048 | \\\n"
		"		modp3072 | modp4096 | modp6144 | modp8192 \\\n"
		"		dh22 | dh23 | dh24>] \\\n"
		"	[--ikelifetime <seconds>] [--ipseclifetime <seconds>] \\\n"
		"	[--rekeymargin <seconds>] [--rekeyfuzz <percentage>] \\\n"
		"	[--retransmit-timeout <seconds>] \\\n"
		"	[--retransmit-interval <msecs>] \\\n"
		"	[--send-redirect] [--redirect-to] \\\n"
		"	[--accept-redirect] [--accept-redirect-to] \\\n"
		"	[--keyingtries <count>] \\\n"
		"	[--replay-window <num>] \\\n"
		"	[--esp <esp-algos>] \\\n"
		"	[--remote-peer-type <cisco>] \\\n"
		"	[--mtu <mtu>] \\\n"
		"	[--priority <prio>] [--reqid <reqid>] \\\n"
		"	[--tfc <size>] [--send-no-esp-tfc] \\\n"
		"	[--ikev1 | --ikev2] \\\n"
		"	[--allow-narrowing] \\\n"
		"	[--ikefrag-allow | --ikefrag-force] [--no-ikepad] \\\n"
		"	[--esn ] [--no-esn] [--decap-dscp] [--nopmtudisc] [--mobike] \\\n"
		"	[--tcp <no|yes|fallback>] --tcp-remote-port <port>\\\n"
#ifdef HAVE_NM
		"	[--nm-configured] \\\n"
#endif
#ifdef HAVE_LABELED_IPSEC
		"	[--policylabel <label>] \\\n"
#endif
		"	[--xauthby file|pam|alwaysok] [--xauthfail hard|soft] \\\n"
		"	[--dontrekey] [--aggressive] \\\n"
		"	[--initialcontact] [--cisco-unity] [--fake-strongswan] \\\n"
		"	[--encaps <auto|yes|no>] [--no-nat-keepalive] \\\n"
		"	[--ikev1-natt <both|rfc|drafts>] [--no-nat_keepalive] \\\n"
		"	[--dpddelay <seconds> --dpdtimeout <seconds>] \\\n"
		"	[--dpdaction (clear|hold|restart)] \\\n"
		"	[--xauthserver | --xauthclient] \\\n"
		"	[--modecfgserver | --modecfgclient] [--modecfgpull] \\\n"
		"	[--addresspool <network range>] \\\n"
		"	[--modecfgdns <ip-address, ip-address>  \\\n"
		"	[--modecfgdomains <dns-domain, dns-domain, ..>] \\\n"
		"	[--modecfgbanner <login banner>] \\\n"
		"	[--metric <metric>] \\\n"
		"	[--nflog-group <groupnum>] \\\n"
		"	[--conn-mark <mark/mask>] [--conn-mark-in <mark/mask>] \\\n"
		"	[--conn-mark-out <mark/mask>] \\\n"
		"	[--ipsec-interface <num>] \\\n"
		"	[--vti-iface <iface> ] [--vti-routing] [--vti-shared] \\\n"
		"	[--initiateontraffic | --pass | --drop | --reject] \\\n"
		"	[--failnone | --failpass | --faildrop | --failreject] \\\n"
		"	[--negopass ] \\\n"
		"	[--donotrekey ] [--reauth ] \\\n"
		"	[--nic-offload ] \\\n"
		"	--to\n"
		"\n"
		"routing: whack (--route | --unroute) --name <connection_name>\n"
		"\n"
		"initiation: whack (--initiate [--remote-host <ip or hostname>] | --terminate) \\\n"
		"	--name <connection_name> [--asynchronous] \\\n"
		"	[--username <name>] [--xauthpass <pass>]\n"
		"\n"
		"rekey: whack (--rekey-ike | --rekey-ipsec) \\\n"
		"	--name <connection_name> [--asynchronous] \\\n"
		"\n"
		"active redirect: whack [--name <connection_name>] \\\n"
		"	--redirect-to <ip-address(es)> \n"
		"\n"
		"global redirect: whack --global-redirect yes|no|auto\n"
		"	--global-redirect-to <ip-address, dns-domain, ..> \n"
		"\n"
		"opportunistic initiation: whack [--tunnelipv4 | --tunnelipv6] \\\n"
		"	--oppohere <ip-address> --oppothere <ip-address> \\\n"
		"	--opposport <port> --oppodport <port> \\\n"
		"	[--oppoproto <protocol>]\n"
		"\n"
		"delete: whack --delete --name <connection_name>\n"
		"\n"
		"delete: whack --deleteid --name <id>\n"
		"\n"
		"deletestate: whack --deletestate <state_object_number>\n"
		"\n"
		"delete user: whack --deleteuser --name <user_name> \\\n"
		"	[--crash <ip-address>]\n"
		"\n"
		"pubkey: whack --keyid <id> [--addkey] [--pubkeyrsa <key>]\n"
		"\n"
		"debug: whack [--name <connection_name>] \\\n"
		"	[--debug help|none|<class>] \\\n"
		"	[--no-debug <class>] \\\n"
		"	[--impair help|list|none|<behaviour>]  \\\n"
		"	[--no-impair <behaviour>]\n"
		"\n"
		"listen: whack (--listen | --unlisten)\n"
		"\n"
		"socket buffers: whack --ike-socket-bufsize <bufsize>\n"
		"socket errqueue: whack --ike-socket-errqueue-toggle\n"
		"\n"
		"ddos-protection: whack (--ddos-busy | --ddos-unlimited | \\\n"
		"	--ddos-auto)\n"
		"\n"
		"list: whack [--utc] [--checkpubkeys] [--listpubkeys] [--listcerts] \\\n"
		"	[--listcacerts] [--listcrls] [--listpsks] [--listevents] [--listall]\n"
		"\n"
		"purge: whack --purgeocsp\n"
		"\n"
		"reread: whack [--fetchcrls] [--rereadcerts] [--rereadsecrets] [--rereadall]\n"
		"\n"
		"status: whack [--status] | [--trafficstatus] | [--globalstatus] | \\\n"
		"	[--clearstats] | [--shuntstatus] | [--fipsstatus] | [--briefstatus] \n"
		"	[--showstates] | [--addresspoolstatus] [--processstatus]\n"
		"\n"
		"refresh dns: whack --ddns\n"
		"\n"
#ifdef HAVE_SECCOMP
		"testing: whack --seccomp-crashtest (CAREFUL!)\n"
		"\n"
#endif
		"shutdown: whack --shutdown [--leave-state]\n"
		"\n"
		"Libreswan %s\n",
		ipsec_version_code());
}

/* --label operand, saved for diagnostics */
static const char *label = NULL;

/* --name operand, saved for diagnostics */
static const char *name = NULL;

static const char *remote_host = NULL;

static bool auth_specified = false;

/*
 * Print a string as a diagnostic, then exit whack unhappily
 *
 * @param mess The error message to print when exiting
 * @return NEVER
 */
static void diagw(const char *mess) NEVER_RETURNS;

static void diagw(const char *mess)
{
	if (mess != NULL) {
		fprintf(stderr, "whack error: ");
		if (label != NULL)
			fprintf(stderr, "%s ", label);
		if (name != NULL)
			fprintf(stderr, "\"%s\" ", name);
		fprintf(stderr, "%s\n", mess);
	}

	exit(RC_WHACK_PROBLEM);
}

/*
 * Conditially calls diag if ugh is set.
 * Prints second arg, if non-NULL, as quoted string
 *
 * @param ugh Error message
 * @param this Optional 2nd part of error message
 * @return void
 */
static void diagq(err_t ugh, const char *this)
{
	if (ugh != NULL) {
		if (this == NULL) {
			diagw(ugh);
		} else {
			char buf[120];	/* arbitrary limit */

			snprintf(buf, sizeof(buf), "%s \"%s\"", ugh, this);
			diagw(buf);
		}
	}
}

/*
 * complex combined operands return one of these enumerated values
 * Note: these become flags in an lset_t.  Since there could be more
 * than lset_t could hold (currently 64), we partition them into:
 * - OPT_* options (most random options)
 * - LST_* options (list various internal data)
 * - DBGOPT_* option (DEBUG options)
 * - END_* options (End description options)
 * - CD_* options (Connection Description options)
 * - CDP_* options (Connection Description Policy bit options)
 */
enum option_enums {
#   define OPT_FIRST1    OPT_RUNDIR	/* first normal option, range 1 */
	OPT_RUNDIR,
	OPT_CTLSOCKET,
	OPT_NAME,
	OPT_REMOTE_HOST,
	OPT_CONNALIAS,

	OPT_CD,

	OPT_KEYID,
	OPT_ADDKEY,
	OPT_PUBKEYRSA,
	OPT_PUBKEYECDSA,

	OPT_ROUTE,
	OPT_UNROUTE,

	OPT_INITIATE,
	OPT_TERMINATE,
	OPT_DELETE,
	OPT_DELETEID,
	OPT_DELETESTATE,
	OPT_DELETEUSER,
	OPT_LISTEN,
	OPT_UNLISTEN,
	OPT_IKEBUF,
	OPT_IKE_MSGERR,
	OPT_REKEY_IKE,
	OPT_REKEY_IPSEC,

	OPT_ACTIVE_REDIRECT,
	OPT_GLOBAL_REDIRECT,
	OPT_GLOBAL_REDIRECT_TO,

	OPT_DDOS_BUSY,
	OPT_DDOS_UNLIMITED,
	OPT_DDOS_AUTO,

	OPT_DDNS,

	OPT_REREADSECRETS,
	OPT_REREADCRLS,
	OPT_FETCHCRLS,
	OPT_REREADCERTS,
	OPT_REREADALL,

	OPT_PURGEOCSP,

	OPT_STATUS,
	OPT_GLOBAL_STATUS,
	OPT_CLEAR_STATS,
	OPT_SHUTDOWN,
	OPT_SHUTDOWN_DIRTY,
	OPT_TRAFFIC_STATUS,
	OPT_SHUNT_STATUS,
	OPT_SHOW_STATES,
	OPT_ADDRESSPOOL_STATUS,
	OPT_FIPS_STATUS,
	OPT_BRIEF_STATUS,
	OPT_PROCESS_STATUS,

#ifdef HAVE_SECCOMP
	OPT_SECCOMP_CRASHTEST,
#endif

	OPT_OPPO_HERE,
	OPT_OPPO_THERE,
	OPT_OPPO_PROTO,
	OPT_OPPO_SPORT,
	OPT_OPPO_DPORT,

#   define OPT_LAST1 OPT_OPPO_DPORT	/* last "normal" option, range 1 */

#define OPT_FIRST2  OPT_ASYNC	/* first normal option, range 2 */

	OPT_ASYNC,

	OPT_DELETECRASH,
	OPT_USERNAME,
	OPT_XAUTHPASS,

#define OPT_LAST2 OPT_XAUTHPASS	/* last "normal" option, range 2 */

/* List options */

#   define LST_FIRST LST_UTC	/* first list option */
	LST_UTC,
	LST_CHECKPUBKEYS,
	LST_PUBKEYS,
	LST_CERTS,
	LST_CACERTS,
	LST_CRLS,
	LST_PSKS,
	LST_EVENTS,
	LST_ACERTS,
	LST_AACERTS,
	LST_GROUPS,
	LST_CARDS,
	LST_ALL,

#   define LST_LAST LST_ALL	/* last list option */

/* Connection End Description options */

#   define END_FIRST END_HOST	/* first end description */
	END_HOST,
	END_ID,
	END_CERT,
	END_CKAID,
	END_CA,
	END_GROUPS,
	END_IKEPORT,
	END_NEXTHOP,
	END_CLIENT,
	END_CLIENTPROTOPORT,
	END_DNSKEYONDEMAND,
	END_XAUTHNAME,
	END_XAUTHSERVER,
	END_XAUTHCLIENT,
	END_MODECFGCLIENT,
	END_MODECFGSERVER,
	END_ADDRESSPOOL,
	END_SENDCERT,
	END_SRCIP,
	END_VTIIP,
	END_AUTHBY,
	END_AUTHEAP,
	END_UPDOWN,

#define END_LAST  END_UPDOWN	/* last end description*/

/* Connection Description options -- segregated */

#   define CD_FIRST CD_TO	/* first connection description */
	CD_TO,

	CD_IKEv1,
	CD_IKEv2,

	CD_MODECFGDNS,
	CD_MODECFGDOMAINS,
	CD_MODECFGBANNER,
	CD_METRIC,
	CD_CONNMTU,
	CD_PRIORITY,
	CD_TFCPAD,
	CD_SEND_TFCPAD,
	CD_REQID,
	CD_NFLOG_GROUP,
	CD_CONN_MARK_BOTH,
	CD_CONN_MARK_IN,
	CD_CONN_MARK_OUT,
	CD_VTI_IFACE,
	CD_VTI_ROUTING,
	CD_VTI_SHARED,
	CD_IPSEC_IFACE,
	CD_TUNNELIPV4,
	CD_TUNNELIPV6,
	CD_CONNIPV4,
	CD_CONNIPV6,

	CD_RETRANSMIT_T,
	CD_RETRANSMIT_I,
	CD_IKELIFETIME,
	CD_IPSECLIFETIME,
	CD_RKMARGIN,
	CD_RKFUZZ,
	CD_KTRIES,
	CD_REPLAY_W,
	CD_DPDDELAY,
	CD_DPDTIMEOUT,
	CD_DPDACTION,
	CD_SEND_REDIRECT,
	CD_REDIRECT_TO,
	CD_ACCEPT_REDIRECT,
	CD_ACCEPT_REDIRECT_TO,
	CD_FORCEENCAPS,
	CD_ENCAPS,
	CD_NO_NAT_KEEPALIVE,
	CD_IKEV1_NATT,
	CD_INITIAL_CONTACT,
	CD_CISCO_UNITY,
	CD_FAKE_STRONGSWAN,
	CD_MOBIKE,
	CD_IKE,
	CD_IKE_TCP,
	CD_IKE_TCP_REMOTE_PORT,
	CD_SEND_CA,
	CD_PFSGROUP,
	CD_REMOTEPEERTYPE,
	CD_SHA2_TRUNCBUG,
	CD_NMCONFIGURED,
	CD_LABELED_IPSEC,
	CD_SEC_LABEL,
	CD_XAUTHBY,
	CD_XAUTHFAIL,
	CD_NIC_OFFLOAD,
	CD_ESP,
	CD_INTERMEDIATE,
#   define CD_LAST CD_INTERMEDIATE	/* last connection description */

/*
 * Algorithm options (just because CD_ was full)
 */
#define ALGO_FIRST	ALGO_RSASIG
	ALGO_RSASIG, /* SHA1 and (for IKEv2) SHA2 */
	ALGO_RSA_SHA1,
	ALGO_RSA_SHA2,
	ALGO_RSA_SHA2_256,
	ALGO_RSA_SHA2_384,
	ALGO_RSA_SHA2_512,
	ALGO_ECDSA, /* no SHA1 support */
	ALGO_ECDSA_SHA2_256,
	ALGO_ECDSA_SHA2_384,
	ALGO_ECDSA_SHA2_512,

#define ALGO_LAST	ALGO_ECDSA_SHA2_512

/*
 * Shunt policies
 */

	CDS_PROSPECTIVE,
	CDS_NEGOTIATION = CDS_PROSPECTIVE + SHUNT_POLICY_ROOF,
	CDS_FAILURE = CDS_NEGOTIATION + SHUNT_POLICY_ROOF,
	CDS_LAST = CDS_FAILURE + SHUNT_POLICY_ROOF - 1,

/*
 * Policy options
 *
 * Really part of Connection Description but too many bits
 * for cd_seen.
 */

#define CDP_FIRST	CDP_SINGLETON

	/*
	 * The next range is for single-element policy options.
	 * It covers enum sa_policy_bits values.
	 */
	CDP_SINGLETON,
	/* large gap of unnamed values... */
	CDP_SINGLETON_LAST = CDP_SINGLETON + POLICY_IX_LAST,

#define CDP_LAST	CDP_SINGLETON_LAST

/* === end of correspondence with POLICY_* === */

#   define DBGOPT_FIRST DBGOPT_NONE
	DBGOPT_NONE,
	DBGOPT_ALL,

	DBGOPT_DEBUG,
	DBGOPT_NO_DEBUG,
	DBGOPT_IMPAIR,
	DBGOPT_NO_IMPAIR,

	DBGOPT_LAST = DBGOPT_NO_IMPAIR,

#define	OPTION_ENUMS_LAST	DBGOPT_LAST
};

/*
 * Carve up space for result from getop_long.
 * Stupidly, the only result is an int.
 * Numeric arg is bit immediately left of basic value.
 *
 */
#define OPTION_OFFSET   256	/* to get out of the way of letter options */
#define NUMERIC_ARG (1 << 11)	/* expect a numeric argument */
#define AUX_SHIFT   12	/* amount to shift for aux information */

int long_index;

static const struct option long_opts[] = {
#   define OO   OPTION_OFFSET
	/* name, has_arg, flag, val */

	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'v' },
	{ "label", required_argument, NULL, 'l' },

	{ "rundir", required_argument, NULL, OPT_RUNDIR + OO },
	{ "ctlbase", required_argument, NULL, OPT_RUNDIR + OO }, /* backwards compat */
	{ "ctlsocket", required_argument, NULL, OPT_CTLSOCKET + OO },
	{ "name", required_argument, NULL, OPT_NAME + OO },
	{ "remote-host", required_argument, NULL, OPT_REMOTE_HOST + OO },
	{ "connalias", required_argument, NULL, OPT_CONNALIAS + OO },

	{ "keyid", required_argument, NULL, OPT_KEYID + OO },
	{ "addkey", no_argument, NULL, OPT_ADDKEY + OO },
	{ "pubkeyrsa", required_argument, NULL, OPT_PUBKEYRSA + OO },

	{ "route", no_argument, NULL, OPT_ROUTE + OO },
	{ "ondemand", no_argument, NULL, OPT_ROUTE + OO },	/* alias */
	{ "unroute", no_argument, NULL, OPT_UNROUTE + OO },

	{ "initiate", no_argument, NULL, OPT_INITIATE + OO },
	{ "terminate", no_argument, NULL, OPT_TERMINATE + OO },
	{ "delete", no_argument, NULL, OPT_DELETE + OO },
	{ "deleteid", no_argument, NULL, OPT_DELETEID + OO },
	{ "deletestate", required_argument, NULL, OPT_DELETESTATE + OO + NUMERIC_ARG },
	{ "deleteuser", no_argument, NULL, OPT_DELETEUSER + OO },
	{ "crash", required_argument, NULL, OPT_DELETECRASH + OO },
	{ "listen", no_argument, NULL, OPT_LISTEN + OO },
	{ "unlisten", no_argument, NULL, OPT_UNLISTEN + OO },
	{ "ike-socket-bufsize", required_argument, NULL, OPT_IKEBUF + OO + NUMERIC_ARG},
	{ "ike-socket-errqueue-toggle", no_argument, NULL, OPT_IKE_MSGERR + OO },

	{ "redirect-to", required_argument, NULL, OPT_ACTIVE_REDIRECT + OO },
	{ "global-redirect", required_argument, NULL, OPT_GLOBAL_REDIRECT + OO },
	{ "global-redirect-to", required_argument, NULL, OPT_GLOBAL_REDIRECT_TO + OO },

	{ "ddos-busy", no_argument, NULL, OPT_DDOS_BUSY + OO },
	{ "ddos-unlimited", no_argument, NULL, OPT_DDOS_UNLIMITED + OO },
	{ "ddos-auto", no_argument, NULL, OPT_DDOS_AUTO + OO },

	{ "ddns", no_argument, NULL, OPT_DDNS + OO },

	{ "rereadsecrets", no_argument, NULL, OPT_REREADSECRETS + OO },
	{ "rereadcrls", no_argument, NULL, OPT_REREADCRLS + OO }, /* obsolete */
	{ "rereadcerts", no_argument, NULL, OPT_REREADCERTS + OO },
	{ "fetchcrls", no_argument, NULL, OPT_FETCHCRLS + OO },
	{ "rereadall", no_argument, NULL, OPT_REREADALL + OO },

	{ "purgeocsp", no_argument, NULL, OPT_PURGEOCSP + OO },

	{ "status", no_argument, NULL, OPT_STATUS + OO },
	{ "globalstatus", no_argument, NULL, OPT_GLOBAL_STATUS + OO },
	{ "clearstats", no_argument, NULL, OPT_CLEAR_STATS + OO },
	{ "trafficstatus", no_argument, NULL, OPT_TRAFFIC_STATUS + OO },
	{ "shuntstatus", no_argument, NULL, OPT_SHUNT_STATUS + OO },
	{ "addresspoolstatus", no_argument, NULL, OPT_ADDRESSPOOL_STATUS + OO },
	{ "fipsstatus", no_argument, NULL, OPT_FIPS_STATUS + OO },
	{ "briefstatus", no_argument, NULL, OPT_BRIEF_STATUS + OO },
	{ "processstatus", no_argument, NULL, OPT_PROCESS_STATUS + OO },
	{ "showstates", no_argument, NULL, OPT_SHOW_STATES + OO },
#ifdef HAVE_SECCOMP
	{ "seccomp-crashtest", no_argument, NULL, OPT_SECCOMP_CRASHTEST + OO },
#endif
	{ "shutdown", no_argument, NULL, OPT_SHUTDOWN + OO },
	{ "leave-state", no_argument, NULL, OPT_SHUTDOWN_DIRTY + OO },
	{ "username", required_argument, NULL, OPT_USERNAME + OO },
	{ "xauthuser", required_argument, NULL, OPT_USERNAME + OO }, /* old name */
	{ "xauthname", required_argument, NULL, OPT_USERNAME + OO }, /* old name */
	{ "xauthpass", required_argument, NULL, OPT_XAUTHPASS + OO },

	{ "oppohere", required_argument, NULL, OPT_OPPO_HERE + OO },
	{ "oppothere", required_argument, NULL, OPT_OPPO_THERE + OO },
	{ "oppoproto", required_argument, NULL, OPT_OPPO_PROTO + OO },
	{ "opposport", required_argument, NULL, OPT_OPPO_SPORT + OO },
	{ "oppodport", required_argument, NULL, OPT_OPPO_DPORT + OO },

	{ "asynchronous", no_argument, NULL, OPT_ASYNC + OO },

	{ "rekey-ike", no_argument, NULL, OPT_REKEY_IKE + OO },
	{ "rekey-ipsec", no_argument, NULL, OPT_REKEY_IPSEC + OO },
	/* list options */

	{ "utc", no_argument, NULL, LST_UTC + OO },
	{ "checkpubkeys", no_argument, NULL, LST_CHECKPUBKEYS + OO },
	{ "listpubkeys", no_argument, NULL, LST_PUBKEYS + OO },
	{ "listcerts", no_argument, NULL, LST_CERTS + OO },
	{ "listcacerts", no_argument, NULL, LST_CACERTS + OO },
	{ "listcrls", no_argument, NULL, LST_CRLS + OO },
	{ "listpsks", no_argument, NULL, LST_PSKS + OO },
	{ "listevents", no_argument, NULL, LST_EVENTS + OO },
	{ "listall", no_argument, NULL, LST_ALL + OO },

	/* options for an end description */

	{ "host", required_argument, NULL, END_HOST + OO },
	{ "id", required_argument, NULL, END_ID + OO },
	{ "cert", required_argument, NULL, END_CERT + OO },
	{ "ckaid", required_argument, NULL, END_CKAID + OO },
	{ "ca", required_argument, NULL, END_CA + OO },
	{ "groups", required_argument, NULL, END_GROUPS + OO },
	{ "ikeport", required_argument, NULL, END_IKEPORT + OO + NUMERIC_ARG },
	{ "nexthop", required_argument, NULL, END_NEXTHOP + OO },
	{ "client", required_argument, NULL, END_CLIENT + OO },
	{ "clientprotoport", required_argument, NULL, END_CLIENTPROTOPORT + OO },
#ifdef USE_DNSSEC
	{ "dnskeyondemand", no_argument, NULL, END_DNSKEYONDEMAND + OO },
#endif
	{ "srcip",  required_argument, NULL, END_SRCIP + OO },
	{ "vtiip",  required_argument, NULL, END_VTIIP + OO },
	{ "authby",  required_argument, NULL, END_AUTHBY + OO },
	{ "autheap",  required_argument, NULL, END_AUTHEAP + OO },
	{ "updown", required_argument, NULL, END_UPDOWN + OO },

	/* options for a connection description */

	{ "to", no_argument, NULL, CD_TO + OO },

	/* option for cert rotation */

#define PS(o, p)	{ o, no_argument, NULL, CDP_SINGLETON + POLICY_##p##_IX + OO }
	PS("psk", PSK),
	PS("intermediate", INTERMEDIATE),
	/* These require more complicated settings now, done below
	 * PS("rsasig", RSASIG),
	 * PS("ecdsa", ECDSA),
	 */
	PS("auth-never", AUTH_NEVER),
	PS("auth-null", AUTH_NULL),

	PS("encrypt", ENCRYPT),
	PS("authenticate", AUTHENTICATE),
	PS("compress", COMPRESS),
	PS("overlapip", OVERLAPIP),
	PS("tunnel", TUNNEL),
	{ "tunnelipv4", no_argument, NULL, CD_TUNNELIPV4 + OO },
	{ "tunnelipv6", no_argument, NULL, CD_TUNNELIPV6 + OO },
	PS("pfs", PFS),
	PS("ms-dh-downgrade", MSDH_DOWNGRADE),
	PS("dns-match-id", DNS_MATCH_ID),
	PS("allow-cert-without-san-id", ALLOW_NO_SAN),
	PS("sha2-truncbug", SHA2_TRUNCBUG),
	PS("sha2_truncbug", SHA2_TRUNCBUG), /* backwards compatibility */
	PS("aggressive", AGGRESSIVE),
	PS("aggrmode", AGGRESSIVE), /*  backwards compatibility */

	{ "initiateontraffic", no_argument, NULL, CDS_PROSPECTIVE + SHUNT_TRAP + OO },
	{ "pass", no_argument, NULL, CDS_PROSPECTIVE + SHUNT_PASS + OO },
	{ "drop", no_argument, NULL, CDS_PROSPECTIVE + SHUNT_DROP + OO },
	{ "reject", no_argument, NULL, CDS_PROSPECTIVE + SHUNT_REJECT + OO },

	{ "negopass", no_argument, NULL, CDS_NEGOTIATION + SHUNT_PASS + OO },

	{ "failnone", no_argument, NULL, CDS_FAILURE + SHUNT_NONE + OO },
	{ "failpass", no_argument, NULL, CDS_FAILURE + SHUNT_PASS + OO },
	{ "faildrop", no_argument, NULL, CDS_FAILURE + SHUNT_DROP + OO },
	{ "failreject", no_argument, NULL, CDS_FAILURE + SHUNT_REJECT + OO },

	PS("dontrekey", DONT_REKEY),
	PS("reauth", REAUTH),
	{ "forceencaps", no_argument, NULL, CD_FORCEENCAPS + OO }, /* backwards compatibility */
	{ "encaps", required_argument, NULL, CD_ENCAPS + OO },
	{ "no-nat_keepalive", no_argument, NULL,  CD_NO_NAT_KEEPALIVE + OO },
	{ "ikev1_natt", required_argument, NULL, CD_IKEV1_NATT + OO },	/* obsolete _ */
	{ "ikev1-natt", required_argument, NULL, CD_IKEV1_NATT + OO },
	{ "initialcontact", no_argument, NULL,  CD_INITIAL_CONTACT + OO },
	{ "cisco_unity", no_argument, NULL, CD_CISCO_UNITY + OO },	/* obsolete _ */
	{ "cisco-unity", no_argument, NULL, CD_CISCO_UNITY + OO },
	{ "fake-strongswan", no_argument, NULL, CD_FAKE_STRONGSWAN + OO },
	PS("mobike", MOBIKE),

	{ "dpddelay", required_argument, NULL, CD_DPDDELAY + OO + NUMERIC_ARG },
	{ "dpdtimeout", required_argument, NULL, CD_DPDTIMEOUT + OO + NUMERIC_ARG },
	{ "dpdaction", required_argument, NULL, CD_DPDACTION + OO },
	{ "send-redirect", required_argument, NULL, CD_SEND_REDIRECT + OO },
	{ "redirect-to", required_argument, NULL, CD_REDIRECT_TO + OO },
	{ "accept-redirect", required_argument, NULL, CD_ACCEPT_REDIRECT + OO },
	{ "accept-redirect-to", required_argument, NULL, CD_ACCEPT_REDIRECT_TO + OO },

	{ "xauth", no_argument, NULL, END_XAUTHSERVER + OO },
	{ "xauthserver", no_argument, NULL, END_XAUTHSERVER + OO },
	{ "xauthclient", no_argument, NULL, END_XAUTHCLIENT + OO },
	{ "xauthby", required_argument, NULL, CD_XAUTHBY + OO },
	{ "xauthfail", required_argument, NULL, CD_XAUTHFAIL + OO },
	PS("modecfgpull", MODECFG_PULL),
	{ "modecfgserver", no_argument, NULL, END_MODECFGSERVER + OO },
	{ "modecfgclient", no_argument, NULL, END_MODECFGCLIENT + OO },
	{ "addresspool", required_argument, NULL, END_ADDRESSPOOL + OO },
	{ "modecfgdns", required_argument, NULL, CD_MODECFGDNS + OO },
	{ "modecfgdomains", required_argument, NULL, CD_MODECFGDOMAINS + OO },
	{ "modecfgbanner", required_argument, NULL, CD_MODECFGBANNER + OO },
	{ "modeconfigserver", no_argument, NULL, END_MODECFGSERVER + OO },
	{ "modeconfigclient", no_argument, NULL, END_MODECFGCLIENT + OO },

	{ "metric", required_argument, NULL, CD_METRIC + OO + NUMERIC_ARG },
	{ "mtu", required_argument, NULL, CD_CONNMTU + OO + NUMERIC_ARG },
	{ "priority", required_argument, NULL, CD_PRIORITY + OO + NUMERIC_ARG },
	{ "tfc", required_argument, NULL, CD_TFCPAD + OO + NUMERIC_ARG },
	{ "send-no-esp-tfc", no_argument, NULL, CD_SEND_TFCPAD + OO },
	{ "reqid", required_argument, NULL, CD_REQID + OO + NUMERIC_ARG },
	{ "nflog-group", required_argument, NULL, CD_NFLOG_GROUP + OO + NUMERIC_ARG },
	{ "conn-mark", required_argument, NULL, CD_CONN_MARK_BOTH + OO },
	{ "conn-mark-in", required_argument, NULL, CD_CONN_MARK_IN + OO },
	{ "conn-mark-out", required_argument, NULL, CD_CONN_MARK_OUT + OO },
	{ "vti-iface", required_argument, NULL, CD_VTI_IFACE + OO },
	{ "vti-routing", no_argument, NULL, CD_VTI_ROUTING + OO },
	{ "vti-shared", no_argument, NULL, CD_VTI_SHARED + OO },
	{ "ipsec-interface", required_argument, NULL, CD_IPSEC_IFACE + OO + NUMERIC_ARG },
	{ "sendcert", required_argument, NULL, END_SENDCERT + OO },
	{ "sendca", required_argument, NULL, CD_SEND_CA + OO },
	{ "ipv4", no_argument, NULL, CD_CONNIPV4 + OO },
	{ "ipv6", no_argument, NULL, CD_CONNIPV6 + OO },
	{ "ikelifetime", required_argument, NULL, CD_IKELIFETIME + OO + NUMERIC_ARG },
	{ "ipseclifetime", required_argument, NULL, CD_IPSECLIFETIME + OO + NUMERIC_ARG },
	{ "retransmit-timeout", required_argument, NULL, CD_RETRANSMIT_T + OO + NUMERIC_ARG },
	{ "retransmit-interval", required_argument, NULL, CD_RETRANSMIT_I + OO + NUMERIC_ARG },
	{ "rekeymargin", required_argument, NULL, CD_RKMARGIN + OO + NUMERIC_ARG },
	/* OBSOLETE */
	{ "rekeywindow", required_argument, NULL, CD_RKMARGIN + OO + NUMERIC_ARG },
	{ "rekeyfuzz", required_argument, NULL, CD_RKFUZZ + OO + NUMERIC_ARG },
	{ "keyingtries", required_argument, NULL, CD_KTRIES + OO + NUMERIC_ARG },
	{ "replay-window", required_argument, NULL, CD_REPLAY_W + OO + NUMERIC_ARG },
	{ "ike",    required_argument, NULL, CD_IKE + OO },
	{ "ikealg", required_argument, NULL, CD_IKE + OO },
	{ "pfsgroup", required_argument, NULL, CD_PFSGROUP + OO },
	{ "esp", required_argument, NULL, CD_ESP + OO },
	{ "remote-peer-type", required_argument, NULL, CD_REMOTEPEERTYPE + OO },
	{ "nic-offload", required_argument, NULL, CD_NIC_OFFLOAD + OO},

	{ "rsasig", no_argument, NULL, ALGO_RSASIG + OO },
	{ "ecdsa", no_argument, NULL, ALGO_ECDSA + OO },
	{ "ecdsa-sha2", no_argument, NULL, ALGO_ECDSA + OO },
	{ "ecdsa-sha2_256", no_argument, NULL, ALGO_ECDSA_SHA2_256 + OO },
	{ "ecdsa-sha2_384", no_argument, NULL, ALGO_ECDSA_SHA2_384 + OO },
	{ "ecdsa-sha2_512", no_argument, NULL, ALGO_ECDSA_SHA2_512 + OO },
	{ "rsa-sha1", no_argument, NULL, ALGO_RSA_SHA1 + OO },
	{ "rsa-sha2", no_argument, NULL, ALGO_RSA_SHA2 + OO },
	{ "rsa-sha2_256", no_argument, NULL, ALGO_RSA_SHA2_256 + OO },
	{ "rsa-sha2_384", no_argument, NULL, ALGO_RSA_SHA2_384 + OO },
	{ "rsa-sha2_512", no_argument, NULL, ALGO_RSA_SHA2_512 + OO },


	{ "ikev1", no_argument, NULL, CD_IKEv1 + OO },
	{ "ikev1-allow", no_argument, NULL, CD_IKEv1 + OO }, /* obsolete name */
	{ "ikev2", no_argument, NULL, CD_IKEv2 +OO },
	{ "ikev2-allow", no_argument, NULL, CD_IKEv2 +OO }, /* obsolete name */
	{ "ikev2-propose", no_argument, NULL, CD_IKEv2 +OO }, /* obsolete, map onto allow */

	PS("allow-narrowing", IKEV2_ALLOW_NARROWING),
#ifdef AUTH_HAVE_PAM
	PS("ikev2-pam-authorize", IKEV2_PAM_AUTHORIZE),
#endif
	PS("ikefrag-allow", IKE_FRAG_ALLOW),
	PS("ikefrag-force", IKE_FRAG_FORCE),
	PS("no-ikepad", NO_IKEPAD),

	PS("no-esn", ESN_NO),
	PS("esn", ESN_YES),
	PS("decap-dscp", DECAP_DSCP),
	PS("nopmtudisc", NOPMTUDISC),
	PS("ms-dh-downgrade", MSDH_DOWNGRADE),
	PS("dns-match-id", DNS_MATCH_ID),
	PS("ignore-peer-dns", IGNORE_PEER_DNS),
#undef PS

	{ "tcp", required_argument, NULL, CD_IKE_TCP + OO },
	{ "tcp-remote-port", required_argument, NULL, CD_IKE_TCP_REMOTE_PORT + OO + NUMERIC_ARG},

#ifdef HAVE_NM
	{ "nm_configured", no_argument, NULL, CD_NMCONFIGURED + OO }, /* backwards compat */
	{ "nm-configured", no_argument, NULL, CD_NMCONFIGURED + OO },
#endif

	{ "policylabel", required_argument, NULL, CD_SEC_LABEL + OO },

	{ "debug-none", no_argument, NULL, DBGOPT_NONE + OO },
	{ "debug-all", no_argument, NULL, DBGOPT_ALL + OO },
	{ "debug", required_argument, NULL, DBGOPT_DEBUG + OO, },
	{ "no-debug", required_argument, NULL, DBGOPT_NO_DEBUG + OO, },
	{ "impair", required_argument, NULL, DBGOPT_IMPAIR + OO, },
	{ "no-impair", required_argument, NULL, DBGOPT_NO_IMPAIR + OO, },

#    undef DO
#   undef OO
	{ 0, 0, 0, 0 }
};

/*
 * figure out an address family.
 */
struct family {
	const char *used_by;
	const struct ip_info *type;
};

static err_t opt_ttoaddress_num(struct family *family, ip_address *address)
{
	err_t err = ttoaddress_num(shunk1(optarg), family->type, address);
	if (err == NULL && family->type == NULL) {
		family->type = address_type(address);
		family->used_by = long_opts[long_index].name;
	}
	return err;
}

static err_t opt_ttoaddress_dns(struct family *family, ip_address *address)
{
	err_t err = ttoaddress_dns(shunk1(optarg), family->type, address);
	if (err == NULL && family->type == NULL) {
		family->type = address_type(address);
		family->used_by = long_opts[long_index].name;
	}
	return err;
}

static void opt_to_address(struct family *family, ip_address *address)
{
	diagq(opt_ttoaddress_dns(family, address), optarg);
}

static void opt_to_cidr(struct family *family, ip_cidr *cidr)
{
	diagq(numeric_to_cidr(shunk1(optarg), family->type, cidr), optarg);
	if (family->type == NULL) {
		family->type = cidr_type(cidr);
		family->used_by = long_opts[long_index].name;
	}
}

static void opt_to_subnet(struct family *family, ip_subnet *subnet, struct logger *logger)
{
	diagq(ttosubnet(shunk1(optarg), family->type, '6', subnet, logger), optarg);
	if (family->type == NULL) {
		family->type = subnet_type(subnet);
		family->used_by = long_opts[long_index].name;
	}
}

static const struct ip_info *get_address_family(struct family *family)
{
	if (family->type == NULL) {
		family->type = &ipv4_info;
		family->used_by = long_opts[long_index].name;
	}
	return family->type;
}

static ip_address get_address_any(struct family *family)
{
	return get_address_family(family)->address.any;
}

struct sockaddr_un ctl_addr = {
	.sun_family = AF_UNIX,
	.sun_path  = DEFAULT_CTL_SOCKET,
#if defined(HAS_SUN_LEN)
	.sun_len = sizeof(struct sockaddr_un),
#endif
};

/* ??? there seems to be no consequence for invalid life_time. */
static void check_life_time(deltatime_t life, time_t raw_limit,
			    const char *which,
			    const struct whack_message *msg)
{
	deltatime_t limit = deltatime(raw_limit);
	deltatime_t mint = deltatimescale(100 + msg->sa_rekey_fuzz, 100, msg->sa_rekey_margin);

	if (deltatime_cmp(limit, <, life)) {
		char buf[200];	/* arbitrary limit */

		snprintf(buf, sizeof(buf),
			 "%s [%ld seconds] must be less than %ld seconds",
			 which,
			 (long)deltasecs(life),
			 (long)deltasecs(limit));
		diagw(buf);
	}
	if ((msg->policy & POLICY_DONT_REKEY) == LEMPTY && !deltatime_cmp(mint, <, life)) {
		char buf[200];	/* arbitrary limit */

		snprintf(buf, sizeof(buf),
			 "%s [%ld] must be greater than rekeymargin*(100+rekeyfuzz)/100 [%ld*(100+%lu)/100 = %ld]",
			 which,
			 (long)deltasecs(life),
			 (long)deltasecs(msg->sa_rekey_margin),
			 msg->sa_rekey_fuzz,
			 (long)deltasecs(mint));
		diagw(buf);
	}
}

static void check_end(struct whack_end *this, struct whack_end *that,
		      const struct ip_info *haf, const struct ip_info *caf)
{
	if (haf != NULL && haf != address_type(&this->host_addr))
		diagw("address family of host inconsistent");

	if (this->client.is_set) {
		if (caf != subnet_type(&this->client))
			diagw("address family of client subnet inconsistent");
	} else {
		/* fill in anyaddr-anyaddr aka ::/128 as (missing) client subnet */
		this->client = unset_subnet;
	}

	/* check protocol */
	if (this->protoport.ipproto != that->protoport.ipproto) {
		diagq("the protocol for leftprotoport and rightprotoport must be the same",
			NULL);
	}
}

static void send_reply(int sock, char *buf, ssize_t len)
{
	/* send the secret to pluto */
	if (write(sock, buf, len) != len) {
		int e = errno;

		fprintf(stderr, "whack: write() failed (%d %s)\n", e,
			strerror(e));
		exit(RC_WHACK_PROBLEM);
	}
}

/* This is a hack for initiating ISAKMP exchanges. */

int main(int argc, char **argv)
{
	struct logger *logger = tool_init_log(argv[0]);

	struct whack_message msg;
	struct whackpacker wp;
	char esp_buf[256];	/* uses snprintf */
	lset_t
		opts1_seen = LEMPTY,
		opts2_seen = LEMPTY,
		lst_seen = LEMPTY,
		cd_seen = LEMPTY,
		cdp_seen = LEMPTY,
		end_seen = LEMPTY,
		algo_seen = LEMPTY;

	/* space for at most one RSA key */
	char keyspace[RSA_MAX_ENCODING_BYTES];

	char xauthusername[MAX_XAUTH_USERNAME_LEN];
	char xauthpass[XAUTH_MAX_PASS_LENGTH];
	int usernamelen = 0;
	int xauthpasslen = 0;
	bool gotusername = false, gotxauthpass = false;
	const char *ugh;
	bool ignore_errors = false;

	/* check division of numbering space */
	assert(OPTION_OFFSET + OPTION_ENUMS_LAST < NUMERIC_ARG);
	assert(OPT_LAST1 - OPT_FIRST1 < LELEM_ROOF);
	assert(OPT_LAST2 - OPT_FIRST2 < LELEM_ROOF);
	assert(LST_LAST - LST_FIRST < LELEM_ROOF);
	assert(END_LAST - END_FIRST < LELEM_ROOF);
	assert(CD_LAST - CD_FIRST < LELEM_ROOF);
	assert(ALGO_LAST - ALGO_FIRST < LELEM_ROOF);

	zero(&msg);	/* ??? pointer fields might not be NULLed */

	clear_end(&msg.right);	/* left set from this after --to */

	struct family host_family = { 0, };
	struct family client_family = { 0, };

	msg.name = NULL;
	msg.remote_host = NULL;
	msg.dnshostname = NULL;

	msg.keyid = NULL;
	msg.keyval.ptr = NULL;
	msg.esp = NULL;
	msg.ike = NULL;
	msg.pfsgroup = NULL;

	msg.remotepeertype = NON_CISCO;
	msg.nat_keepalive = true;

	/* Network Manager support */
#ifdef HAVE_NM
	msg.nmconfigured = false;
#endif

	msg.xauthby = XAUTHBY_FILE;
	msg.xauthfail = XAUTHFAIL_HARD;
	msg.modecfg_domains = NULL;
	msg.modecfg_dns = NULL;
	msg.modecfg_banner = NULL;

	msg.nic_offload = yna_auto;
	msg.sa_ike_life_seconds = deltatime(IKE_SA_LIFETIME_DEFAULT);
	msg.sa_ipsec_life_seconds = deltatime(IPSEC_SA_LIFETIME_DEFAULT);
	msg.sa_rekey_margin = deltatime(SA_REPLACEMENT_MARGIN_DEFAULT);
	msg.sa_rekey_fuzz = SA_REPLACEMENT_FUZZ_DEFAULT;
	msg.sa_keying_tries = SA_REPLACEMENT_RETRIES_DEFAULT;
	/* whack cannot access kernel_ops->replay_window */
	msg.sa_replay_window = IPSEC_SA_DEFAULT_REPLAY_WINDOW;
	msg.retransmit_timeout = deltatime(RETRANSMIT_TIMEOUT_DEFAULT);
	msg.retransmit_interval = deltatime_ms(RETRANSMIT_INTERVAL_DEFAULT_MS);

	msg.active_redirect_dests = NULL;

	msg.tunnel_addr_family = AF_UNSPEC;

	msg.right.updown = DEFAULT_UPDOWN;
	msg.left.updown = DEFAULT_UPDOWN;

	msg.iketcp = IKE_TCP_NO;
	msg.remote_tcpport = NAT_IKE_UDP_PORT;

	msg.xfrm_if_id = UINT32_MAX;

	/* set defaults to ICMP PING request */
	msg.oppo.ipproto = IPPROTO_ICMP;
	msg.oppo.local.port = ip_hport(8);
	msg.oppo.remote.port = ip_hport(0);

	for (;;) {
		/* numeric argument for some flags */
		unsigned long opt_whole = 0;

		/*
		 * Note: we don't like the way short options get parsed
		 * by getopt_long, so we simply pass an empty string as
		 * the list.  It could be "hp:d:c:o:eatfs" "NARXPECK".
		 */
		int c = getopt_long(argc, argv, "", long_opts, &long_index)
			- OPTION_OFFSET;
		int aux = 0;

		/* decode a numeric argument, if expected */
		if (0 <= c) {
			if (c & NUMERIC_ARG) {
				c -= NUMERIC_ARG;
				if (ttoul(optarg, 0, 0, &opt_whole) != NULL)
					diagq("badly formed numeric argument",
					      optarg);
			}
			if (c >= (1 << AUX_SHIFT)) {
				aux = c >> AUX_SHIFT;
				c -= aux << AUX_SHIFT;
			}
		}

		/*
		 * per-class option processing
		 *
		 * Mostly detection of repeated flags.
		 */
		if (OPT_FIRST1 <= c && c <= OPT_LAST1) {
			/*
			 * OPT_* options get added opts1_seen.
			 * Reject repeated options (unless later code
			 * intervenes).
			 */
			lset_t f = LELEM(c);

			if (opts1_seen & f)
				diagq("duplicated flag",
				      long_opts[long_index].name);
			opts1_seen |= f;
		} else if (OPT_FIRST2 <= c && c <= OPT_LAST2) {
			/*
			 * OPT_* options get added opts_seen2.
			 * Reject repeated options (unless later code
			 * intervenes).
			 */
			lset_t f = LELEM(c);

			if (opts2_seen & f)
				diagq("duplicated flag",
				      long_opts[long_index].name);
			opts2_seen |= f;
		} else if (LST_FIRST <= c && c <= LST_LAST) {
			/*
			 * LST_* options get added lst_seen.
			 * Reject repeated options (unless later code
			 * intervenes).
			 */
			lset_t f = LELEM(c - LST_FIRST);

			if (lst_seen & f)
				diagq("duplicated flag",
				      long_opts[long_index].name);
			lst_seen |= f;
		} else if (DBGOPT_FIRST <= c && c <= DBGOPT_LAST) {
			/*
			 * DBGOPT_* options are treated separately to reduce
			 * potential members of opts1_seen.
			 */
			msg.whack_options = true;
		} else if (END_FIRST <= c && c <= END_LAST) {
			/*
			 * END_* options are added to end_seen.
			 * Reject repeated options (unless later code
			 * intervenes).
			 */
			lset_t f = LELEM(c - END_FIRST);

			if (end_seen & f)
				diagq("duplicated flag",
				      long_opts[long_index].name);
			end_seen |= f;
			opts1_seen |= LELEM(OPT_CD);
		} else if (CD_FIRST <= c && c <= CD_LAST) {
			/*
			 * CD_* options are added to cd_seen.
			 * Reject repeated options (unless later code
			 * intervenes).
			 */
			lset_t f = LELEM(c - CD_FIRST);

			if (cd_seen & f)
				diagq("duplicated flag",
				      long_opts[long_index].name);
			cd_seen |= f;
			opts1_seen |= LELEM(OPT_CD);
		} else if (CDP_FIRST <= c && c <= CDP_LAST) {
			/*
			 * CDP_* options are added to cdp_seen.
			 * Reject repeated options (unless later code
			 * intervenes).
			 */
			lset_t f = LELEM(c - CDP_FIRST);

			if (cdp_seen & f)
				diagq("duplicated flag",
				      long_opts[long_index].name);
			cdp_seen |= f;
			opts1_seen |= LELEM(OPT_CD);
		} else if (ALGO_FIRST <= c && c <= ALGO_LAST) {
			/*
 			 * ALGO options all translate into two lset's
 			 * msg.policy and msg.sighash
 			 */
			lset_t f = LELEM(c - ALGO_FIRST);

			if (algo_seen & f)
				diagq("duplicated flag",
				      long_opts[long_index].name);
			algo_seen |= f;
			opts1_seen |= LELEM(OPT_CD);
		}

		/*
		 * Note: "break"ing from switch terminates loop.
		 * most cases should end with "continue".
		 */
		switch (c) {
		case EOF - OPTION_OFFSET:	/* end of flags */
			break;

		case 0 - OPTION_OFFSET:	/* long option already handled */
			continue;

		/* diagnostic already printed by getopt_long */
		case ':' - OPTION_OFFSET:
		/* diagnostic already printed by getopt_long */
		case '?' - OPTION_OFFSET:
			/* print no additional diagnostic, but exit sadly */
			diagw(NULL);
			/* not actually reached */
			break;

		case 'h' - OPTION_OFFSET:	/* --help */
			help();
			/* GNU coding standards say to stop here */
			return 0;

		case 'v' - OPTION_OFFSET:	/* --version */
		{
			printf("%s\n", ipsec_version_string());
		}
			/* GNU coding standards say to stop here */
			return 0;

		case 'l' - OPTION_OFFSET:	/* --label <string> */
			label = optarg;	/* remember for diagnostics */
			continue;

		/* the rest of the options combine in complex ways */

		case OPT_RUNDIR:	/* --rundir <dir> */
			if (snprintf(ctl_addr.sun_path,
				     sizeof(ctl_addr.sun_path),
				     "%s/pluto.ctl", optarg) == -1)
				diagw("Invalid rundir for sun_addr");

			continue;

		case OPT_CTLSOCKET:	/* --ctlsocket <file> */
			if (snprintf(ctl_addr.sun_path,
				     sizeof(ctl_addr.sun_path),
				     "%s", optarg) == -1)
				diagw("Invalid ctlsocket for sun_addr");

			continue;

		case OPT_NAME:	/* --name <connection-name> */
			name = optarg;
			msg.name = optarg;
			continue;

		case OPT_REMOTE_HOST:	/* --remote-host <ip or hostname> */
			remote_host = optarg;
			msg.remote_host = optarg;
			continue;

		case OPT_KEYID:	/* --keyid <identity> */
			msg.whack_key = true;
			msg.keyid = optarg;	/* decoded by Pluto */
			continue;

		case OPT_IKEBUF:	/* --ike-socket-bufsize <bufsize> */
			if (opt_whole < 1500) {
				diagw("Ignoring extremely unwise IKE buffer size choice");
			} else {
				msg.ike_buf_size = opt_whole;
				msg.whack_listen = true;
			}
			continue;

		case OPT_IKE_MSGERR:	/* --ike-socket-errqueue-toggle */
			msg.ike_sock_err_toggle = true;
			msg.whack_listen = true;
			continue;

		case OPT_ADDKEY:	/* --addkey */
			msg.whack_addkey = true;
			continue;

		case OPT_PUBKEYRSA:	/* --pubkeyrsa <key> */
		{
			char mydiag_space[TTODATAV_BUF];

			if (msg.keyval.ptr != NULL)
				diagq("only one RSA public-key allowed", optarg);

			ugh = ttodatav(optarg, 0, 0,
				       keyspace, sizeof(keyspace),
				       &msg.keyval.len, mydiag_space,
				       sizeof(mydiag_space),
				       TTODATAV_SPACECOUNTS);

			if (ugh != NULL) {
				/* perhaps enough space */
				char ugh_space[80];

				snprintf(ugh_space, sizeof(ugh_space),
					 "RSA public-key data malformed (%s)",
					 ugh);
				diagq(ugh_space, optarg);
			}
			msg.pubkey_alg = PUBKEY_ALG_RSA;
			msg.keyval.ptr = (unsigned char *)keyspace;
		}
			continue;
		case OPT_PUBKEYECDSA:	/* --pubkeyecdsa <key> */
		{
			char mydiag_space[TTODATAV_BUF];

			if (msg.keyval.ptr != NULL)
				diagq("only one ECDSA public-key allowed", optarg);

			ugh = ttodatav(optarg, 0, 0,
				       keyspace, sizeof(keyspace),
				       &msg.keyval.len, mydiag_space,
				       sizeof(mydiag_space),
				       TTODATAV_SPACECOUNTS);

			if (ugh != NULL) {
				/* perhaps enough space */
				char ugh_space[80];

				snprintf(ugh_space, sizeof(ugh_space),
					 "ECDSA public-key data malformed (%s)",
					 ugh);
				diagq(ugh_space, optarg);
			}
			msg.pubkey_alg = PUBKEY_ALG_ECDSA;
			msg.keyval.ptr = (unsigned char *)keyspace;
		}
			continue;

		case OPT_ROUTE:	/* --route */
			msg.whack_route = true;
			continue;

		case OPT_UNROUTE:	/* --unroute */
			msg.whack_unroute = true;
			continue;

		case OPT_INITIATE:	/* --initiate */
			msg.whack_initiate = true;
			continue;

		case OPT_TERMINATE:	/* --terminate */
			msg.whack_terminate = true;
			continue;

		case OPT_REKEY_IKE: /* --rekey-ike */
			msg.whack_rekey_ike = true;
			continue;

		case OPT_REKEY_IPSEC: /* --rekey-ipsec */
			msg.whack_rekey_ipsec = true;
			continue;

		case OPT_DELETE:	/* --delete */
			msg.whack_delete = true;
			continue;

		case OPT_DELETEID: /* --deleteid --name <id> */
			msg.whack_deleteid = true;
			continue;

		case OPT_DELETESTATE: /* --deletestate <state_object_number> */
			msg.whack_deletestate = true;
			msg.whack_deletestateno = opt_whole;
			continue;

		case OPT_DELETECRASH:	/* --crash <ip-address> */
			msg.whack_crash = true;
			opt_to_address(&host_family, &msg.whack_crash_peer);
			if (address_is_any(msg.whack_crash_peer)) {
				diagq("0.0.0.0 or 0::0 isn't a valid client address",
				      optarg);
			}
			continue;

		/* --deleteuser --name <xauthusername> */
		case OPT_DELETEUSER:
			msg.whack_deleteuser = true;
			continue;

		case OPT_ACTIVE_REDIRECT:	/* --redirect-to */
			msg.active_redirect_dests = strdup(optarg);
			continue;

		case OPT_GLOBAL_REDIRECT:	/* --global-redirect */
			if (streq(optarg, "yes")) {
				msg.global_redirect = GLOBAL_REDIRECT_YES;
			} else if (streq(optarg, "no")) {
				msg.global_redirect = GLOBAL_REDIRECT_NO;
			} else if (streq(optarg, "auto")) {
				msg.global_redirect = GLOBAL_REDIRECT_AUTO;
			} else {
				diagw("invalid option argument for --global-redirect (allowed arguments: yes, no, auto)");
			}
			continue;

		case OPT_GLOBAL_REDIRECT_TO:	/* --global-redirect-to */
			if (!strlen(optarg)) {
				msg.global_redirect_to = strdup("<none>");
			} else {
				msg.global_redirect_to = strdup(optarg);
			}
			continue;

		case OPT_DDOS_BUSY:	/* --ddos-busy */
			msg.whack_ddos = DDOS_FORCE_BUSY;
			continue;

		case OPT_DDOS_UNLIMITED:	/* --ddos-unlimited */
			msg.whack_ddos = DDOS_FORCE_UNLIMITED;
			continue;

		case OPT_DDOS_AUTO:	/* --ddos-auto */
			msg.whack_ddos = DDOS_AUTO;
			continue;

		case OPT_DDNS:	/* --ddns */
			msg.whack_ddns = true;
			continue;

		case OPT_LISTEN:	/* --listen */
			msg.whack_listen = true;
			continue;

		case OPT_UNLISTEN:	/* --unlisten */
			msg.whack_unlisten = true;
			continue;

		case OPT_REREADSECRETS:	/* --rereadsecrets */
		case OPT_REREADCRLS:	/* --rereadcrls */
		case OPT_REREADCERTS:	/* --rereadcerts */
		case OPT_FETCHCRLS:	/* --fetchcrls */
			msg.whack_reread |= LELEM(c - OPT_REREADSECRETS);
			continue;

		case OPT_REREADALL:	/* --rereadall */
			msg.whack_reread = REREAD_ALL;
			continue;

		case OPT_PURGEOCSP:	/* --purgeocsp */
			msg.whack_purgeocsp = true;
			continue;

		case OPT_STATUS:	/* --status */
			msg.whack_status = true;
			ignore_errors = true;
			continue;

		case OPT_GLOBAL_STATUS:	/* --global-status */
			msg.whack_global_status = true;
			ignore_errors = true;
			continue;

		case OPT_CLEAR_STATS:	/* --clearstats */
			msg.whack_clear_stats = true;
			continue;

		case OPT_TRAFFIC_STATUS:	/* --trafficstatus */
			msg.whack_traffic_status = true;
			ignore_errors = true;
			continue;

		case OPT_SHUNT_STATUS:	/* --shuntstatus */
			msg.whack_shunt_status = true;
			ignore_errors = true;
			continue;

		case OPT_ADDRESSPOOL_STATUS:	/* --addresspoolstatus */
			msg.whack_addresspool_status = true;
			ignore_errors = true;
			continue;

		case OPT_FIPS_STATUS:	/* --fipsstatus */
			msg.whack_fips_status = true;
			ignore_errors = true;
			continue;

		case OPT_BRIEF_STATUS:	/* --briefstatus */
			msg.whack_brief_status = true;
			ignore_errors = true;
			continue;

		case OPT_PROCESS_STATUS:	/* --processstatus */
			msg.whack_process_status = true;
			ignore_errors = true;
			continue;

		case OPT_SHOW_STATES:	/* --showstates */
			msg.whack_show_states = true;
			ignore_errors = true;
			continue;
#ifdef HAVE_SECCOMP
		case OPT_SECCOMP_CRASHTEST:	/* --seccomp-crashtest */
			msg.whack_seccomp_crashtest = true;
			continue;
#endif

		case OPT_SHUTDOWN:	/* --shutdown */
			msg.whack_shutdown = true;
			continue;

		case OPT_SHUTDOWN_DIRTY:	/* --leave-state */
			msg.whack_leave_state = true;
			continue;

		case OPT_OPPO_HERE:	/* --oppohere <ip-address> */
			opt_to_address(&client_family, &msg.oppo.local.address);
			if (address_is_any(msg.oppo.local.address)) {
				diagq("0.0.0.0 or 0::0 isn't a valid client address",
				      optarg);
			}
			continue;

		case OPT_OPPO_THERE:	/* --oppothere <ip-address> */
			opt_to_address(&client_family, &msg.oppo.remote.address);
			if (address_is_any(msg.oppo.remote.address)) {
				diagq("0.0.0.0 or 0::0 isn't a valid client address",
				      optarg);
			}
			continue;

		case OPT_OPPO_PROTO:	/* --oppoproto <protocol> */
			msg.oppo.ipproto = strtol(optarg, NULL, 0);
			continue;

		case OPT_OPPO_SPORT:	/* --opposport <port> */
			msg.oppo.local.port = ip_hport(strtol(optarg, NULL, 0));
			continue;

		case OPT_OPPO_DPORT:	/* --oppodport <port> */
			msg.oppo.remote.port = ip_hport(strtol(optarg, NULL, 0));
			continue;

		case OPT_ASYNC:	/* --asynchronous */
			msg.whack_async = true;
			continue;

		/* List options */

		case LST_UTC:	/* --utc */
			msg.whack_utc = true;
			continue;

		case LST_CERTS:	/* --listcerts */
		case LST_CACERTS:	/* --listcacerts */
		case LST_CRLS:	/* --listcrls */
		case LST_PSKS:	/* --listpsks */
		case LST_EVENTS:	/* --listevents */
			msg.whack_list |= LELEM(c - LST_PUBKEYS);
			ignore_errors = true;
			continue;

		case LST_PUBKEYS:	/* --listpubkeys */
			msg.whack_listpubkeys = true;
			ignore_errors = true;
			continue;

		case LST_CHECKPUBKEYS:	/* --checkpubkeys */
			msg.whack_checkpubkeys = true;
			ignore_errors = true;
			continue;

		case LST_ALL:	/* --listall */
			msg.whack_list = LIST_ALL;
			msg.whack_listpubkeys = true;
			ignore_errors = true;
			continue;

		/* Connection Description options */

		case END_HOST:	/* --host <ip-address> */
		{
			lset_t new_policy;
			if (streq(optarg, "%any")) {
				new_policy = LEMPTY;
				msg.right.host_addr = get_address_any(&host_family);
			} else if (streq(optarg, "%opportunistic")) {
				/* always use tunnel mode; mark as opportunistic */
				new_policy = POLICY_TUNNEL | POLICY_OPPORTUNISTIC;
				msg.right.host_addr = get_address_any(&host_family);
			} else if (streq(optarg, "%group")) {
				/* always use tunnel mode; mark as group */
				new_policy = POLICY_TUNNEL | POLICY_GROUP;
				msg.right.host_addr = get_address_any(&host_family);
			} else if (streq(optarg, "%opportunisticgroup")) {
				/* always use tunnel mode; mark as opportunistic */
				new_policy = POLICY_TUNNEL | POLICY_OPPORTUNISTIC | POLICY_GROUP;
				msg.right.host_addr = get_address_any(&host_family);
			} else if (msg.left.id != NULL && !streq(optarg, "%null")) {
				new_policy = LEMPTY;
				if (opt_ttoaddress_num(&host_family, &msg.right.host_addr) == NULL) {
					/*
					 * we have a proper numeric IP
					 * address.
					 */
				} else {
					/*
					 * We assume that we have a DNS name.
					 * This logic matches confread.c.
					 * ??? it would be kind to check
					 * the syntax.
					 */
					msg.dnshostname = optarg;
					opt_ttoaddress_dns(&host_family, &msg.right.host_addr);
					/*
					 * we don't fail here.  pluto
					 * will re-check the DNS later
					 */
				}
			} else {
				new_policy = LEMPTY;
				opt_to_address(&host_family, &msg.right.host_addr);
			}

			msg.policy |= new_policy;

			if (new_policy & (POLICY_OPPORTUNISTIC | POLICY_GROUP)) {
				if (!LHAS(end_seen, END_CLIENT - END_FIRST)) {
					/*
					 * set host to 0.0.0.0 and
					 * --client to 0.0.0.0/0
					 * or IPV6 equivalent
					 */
					msg.right.client = get_address_family(&client_family)->subnet.all;
				}
				pexpect(msg.right.client.is_set);
			}
			if (new_policy & POLICY_GROUP) {
				/*
				 * client subnet must not be specified by
				 * user: it will come from the group's file.
				 */
				if (LHAS(end_seen, END_CLIENT - END_FIRST))
					diagw("--host %group clashes with --client");

				end_seen |= LELEM(END_CLIENT - END_FIRST);
			}
			if (new_policy & POLICY_OPPORTUNISTIC)
				msg.right.key_from_DNS_on_demand = true;
			continue;
		}

		case END_ID:	/* --id <identity> */
			msg.right.id = optarg;	/* decoded by Pluto */
			continue;

		case END_SENDCERT:	/* --sendcert */
			if (streq(optarg, "yes") || streq(optarg, "always")) {
				msg.right.sendcert = CERT_ALWAYSSEND;
			} else if (streq(optarg,
					 "no") || streq(optarg, "never")) {
				msg.right.sendcert = CERT_NEVERSEND;
			} else if (streq(optarg, "ifasked")) {
				msg.right.sendcert = CERT_SENDIFASKED;
			} else {
				diagq("whack sendcert value is not legal",
				      optarg);
				continue;
			}
			continue;

		case END_CERT:	/* --cert <path> */
			if (msg.right.ckaid != NULL)
				diagw("only one --cert <nickname> or --ckaid <ckaid> allowed");
			msg.right.cert = optarg;	/* decoded by Pluto */
			continue;

		case END_CKAID:	/* --ckaid <ckaid> */
			if (msg.right.cert != NULL)
				diagw("only one --cert <nickname> or --ckaid <ckaid> allowed");
			/* try parsing it; the error isn't the most specific */
			const char *ugh = ttodata(optarg, 0, 16, NULL, 0, NULL);
			diagq(ugh, optarg);
			msg.right.ckaid = optarg;	/* decoded by Pluto */
			continue;

		case END_CA:	/* --ca <distinguished name> */
			msg.right.ca = optarg;	/* decoded by Pluto */
			continue;

		case END_GROUPS:	/* --groups <access control groups> */
			msg.right.groups = optarg;	/* decoded by Pluto */
			continue;

		case END_IKEPORT:	/* --ikeport <port-number> */
			if (opt_whole <= 0 || opt_whole >= 0x10000) {
				diagq("<port-number> must be a number between 1 and 65535",
					optarg);
			}
			msg.right.host_ikeport = opt_whole;
			continue;

		case END_NEXTHOP:	/* --nexthop <ip-address> */
			if (streq(optarg, "%direct")) {
				msg.right.host_nexthop = get_address_any(&host_family);
			} else {
				opt_to_address(&host_family, &msg.right.host_nexthop);
			}
			continue;

		case END_SRCIP:	/* --srcip <ip-address> */
			opt_to_address(&host_family, &msg.right.host_srcip);
			continue;

		case END_VTIIP:	/* --vtiip <ip-address/mask> */
			opt_to_cidr(&client_family, &msg.right.host_vtiip);
			continue;

		/*
		 * --authby secret | rsasig | null
		 *  Note: auth-never cannot be asymmetrical
		 */
		case END_AUTHBY:
			auth_specified = true;
			if (streq(optarg, "psk"))
				msg.right.authby = AUTHBY_PSK;
			else if (streq(optarg, "null"))
				msg.right.authby = AUTHBY_NULL;
			else if (streq(optarg, "rsasig"))
				msg.right.authby = AUTHBY_RSASIG;
			else if (streq(optarg, "ecdsa"))
				msg.right.authby = AUTHBY_ECDSA;
			else diagw("authby option is not one of psk, ecdsa, rsasig or null");
			continue;

		case END_AUTHEAP:
			if (streq(optarg, "tls"))
				msg.right.eap = IKE_EAP_TLS;
			else if (streq(optarg, "none"))
				msg.right.eap = IKE_EAP_NONE;
			else diagw("--autheap option is not one of none, tls");
			continue;

		case END_CLIENT:	/* --client <subnet> */
			if (startswith(optarg, "vhost:") ||
			    startswith(optarg, "vnet:")) {
				msg.right.virt = optarg;
			} else {
				opt_to_subnet(&client_family, &msg.right.client, logger);
				pexpect(msg.right.client.is_set);
			}
			msg.policy |= POLICY_TUNNEL;	/* client => tunnel */
			continue;

		/* --clientprotoport <protocol>/<port> */
		case END_CLIENTPROTOPORT:
			diagq(ttoprotoport(optarg, &msg.right.protoport),
				optarg);
			continue;

		case END_DNSKEYONDEMAND:	/* --dnskeyondemand */
			msg.right.key_from_DNS_on_demand = true;
			continue;

		case END_UPDOWN:	/* --updown <updown> */
			msg.right.updown = optarg;
			continue;

		case CD_TO:	/* --to */
			/* process right end, move it to left, reset it */
			if (!LHAS(end_seen, END_HOST - END_FIRST))
				diagw("connection missing --host before --to");

			msg.left = msg.right;
			clear_end(&msg.right);
			end_seen = LEMPTY;
			continue;

		/* --ikev1 --ikev2 --ikev2-propose */
		case CD_IKEv1:
		case CD_IKEv2:
		{
			const enum ike_version ike_version = IKEv1 + c - CD_IKEv1;
			if (msg.ike_version != 0 && msg.ike_version != ike_version) {
				diagw("connection can no longer have --ikev1 and --ikev2");
			}
			msg.ike_version = ike_version;
			continue;
		}

		/* RSASIG/ECDSA need more than a single policy bit */
		case CDP_SINGLETON + POLICY_PSK_IX:	/* --psk */
		case CDP_SINGLETON + POLICY_AUTH_NEVER_IX:	/* --auth-never */
		case CDP_SINGLETON + POLICY_AUTH_NULL_IX:	/* --auth-null */
			auth_specified = true;
			/* FALL THROUGH */

		case CDP_SINGLETON + POLICY_ENCRYPT_IX:	/* --encrypt */
		/* --authenticate */
		case CDP_SINGLETON + POLICY_AUTHENTICATE_IX:
		/* --compress */
		case CDP_SINGLETON + POLICY_COMPRESS_IX:
		case CDP_SINGLETON + POLICY_TUNNEL_IX:	/* --tunnel */
		case CDP_SINGLETON + POLICY_PFS_IX:	/* --pfs */

		/* --donotrekey */
		case CDP_SINGLETON + POLICY_DONT_REKEY_IX:

		/* --reauth */
		case CDP_SINGLETON + POLICY_REAUTH_IX:

		/* --modecfgpull */
		case CDP_SINGLETON + POLICY_MODECFG_PULL_IX:
		/* --aggrmode */
		case CDP_SINGLETON + POLICY_AGGRESSIVE_IX:
		/* --overlapip */
		case CDP_SINGLETON + POLICY_OVERLAPIP_IX:

		/* --allow-narrowing */
		case CDP_SINGLETON + POLICY_IKEV2_ALLOW_NARROWING_IX:

		/* --mobike */
		case CDP_SINGLETON + POLICY_MOBIKE_IX:

		/* --intermediate */
		case CDP_SINGLETON + POLICY_INTERMEDIATE_IX:

		/* --ikefrag-allow */
		case CDP_SINGLETON + POLICY_IKE_FRAG_ALLOW_IX:
		/* --ikefrag-force */
		case CDP_SINGLETON + POLICY_IKE_FRAG_FORCE_IX:
		/* --no-ikepad */
		case CDP_SINGLETON + POLICY_NO_IKEPAD_IX:
		/* --no-esn */
		case CDP_SINGLETON + POLICY_ESN_NO_IX:
		/* --esn */
		case CDP_SINGLETON + POLICY_ESN_YES_IX:
		/* --decap-dscp */
		case CDP_SINGLETON + POLICY_DECAP_DSCP_IX:
		/* --nopmtudisc */
		case CDP_SINGLETON + POLICY_NOPMTUDISC_IX:
		/* --ms-dh-downgrade */
		case CDP_SINGLETON + POLICY_MSDH_DOWNGRADE_IX:
		/* --dns-match-id */
		case CDP_SINGLETON + POLICY_DNS_MATCH_ID_IX:
		/* --allow-cert-without-san-id */
		case CDP_SINGLETON + POLICY_ALLOW_NO_SAN_IX:
		/* --sha2-truncbug or --sha2_truncbug */
		case CDP_SINGLETON + POLICY_SHA2_TRUNCBUG_IX:

			msg.policy |= LELEM(c - CDP_SINGLETON);
			continue;

		case CDS_PROSPECTIVE + SHUNT_TRAP:	/* --initiateontraffic */
		case CDS_PROSPECTIVE + SHUNT_PASS:	/* --pass */
		case CDS_PROSPECTIVE + SHUNT_DROP:	/* --drop */
		case CDS_PROSPECTIVE + SHUNT_REJECT:	/* --reject */
			msg.prospective_shunt = c - CDS_PROSPECTIVE;
			continue;

		case CDS_NEGOTIATION + SHUNT_PASS:	/* --negopass */
			msg.negotiation_shunt = c - CDS_NEGOTIATION;
			continue;

		case CDS_FAILURE + SHUNT_NONE:		/* --failnone */
		case CDS_FAILURE + SHUNT_PASS:		/* --failpass */
		case CDS_FAILURE + SHUNT_DROP:		/* --faildrop */
		case CDS_FAILURE + SHUNT_REJECT:	/* --failreject */
			msg.failure_shunt = c - CDS_FAILURE;
			continue;

		case CD_RETRANSMIT_T:	/* --retransmit-timeout <seconds> */
			msg.retransmit_timeout = deltatime(opt_whole);
			continue;

		case CD_RETRANSMIT_I:	/* --retransmit-interval <msecs> */
			msg.retransmit_interval = deltatime_ms(opt_whole);
			continue;

		case CD_IKELIFETIME:	/* --ikelifetime <seconds> */
			msg.sa_ike_life_seconds = deltatime(opt_whole);
			continue;

		case CD_IPSECLIFETIME:	/* --ipseclifetime <seconds> */
			msg.sa_ipsec_life_seconds = deltatime(opt_whole);
			continue;

		case CD_RKMARGIN:	/* --rekeymargin <seconds> */
			msg.sa_rekey_margin = deltatime(opt_whole);
			continue;

		case CD_RKFUZZ:	/* --rekeyfuzz <percentage> */
			msg.sa_rekey_fuzz = opt_whole;
			continue;

		case CD_KTRIES:	/* --keyingtries <count> */
			msg.sa_keying_tries = opt_whole;
			continue;

		case CD_REPLAY_W: /* --replay-window <num> */
			/*
			 * ??? what values are legitimate?
			 * 32 and often 64, but what else?
			 * Not so large that the
			 * number of bits overflows uint32_t.
			 */
			msg.sa_replay_window = opt_whole;
			continue;

		case CD_SEND_CA:	/* --sendca */
			if (streq(optarg, "issuer"))
				msg.send_ca = CA_SEND_ISSUER;
			else if (streq(optarg, "all"))
				msg.send_ca = CA_SEND_ALL;
			else
				msg.send_ca = CA_SEND_NONE;
			continue;

		/* backwards compatibility */
		case CD_FORCEENCAPS:	/* --forceencaps */
			msg.encaps = yna_yes;
			continue;

		case CD_ENCAPS:	/* --encaps */
			if (streq(optarg, "auto"))
				msg.encaps = yna_auto;
			else if (streq(optarg, "yes"))
				msg.encaps = yna_yes;
			else if (streq(optarg, "no"))
				msg.encaps = yna_no;
			else
				diagw("--encaps options are 'auto', 'yes' or 'no'");
			continue;

		case CD_NIC_OFFLOAD:  /* --nic-offload */
			if (streq(optarg, "no"))
				msg.nic_offload = yna_no;
			else if (streq(optarg, "yes"))
				msg.nic_offload = yna_yes;
			else if (streq(optarg, "auto"))
				msg.nic_offload = yna_auto;
			else
				diagw("--nic-offload options are 'no', 'yes' or 'auto'");
			continue;

		case CD_NO_NAT_KEEPALIVE:	/* --no-nat_keepalive */
			msg.nat_keepalive = false;
			continue;

		case CD_IKEV1_NATT:	/* --ikev1-natt */
			if (streq(optarg, "both"))
				msg.ikev1_natt = NATT_BOTH;
			else if (streq(optarg, "rfc"))
				msg.ikev1_natt = NATT_RFC;
			else if (streq(optarg, "drafts"))
				msg.ikev1_natt = NATT_DRAFTS;
			else if (streq(optarg, "none"))
				msg.ikev1_natt = NATT_NONE;
			else
				diagw("--ikev1-natt options are 'both', 'rfc' or 'drafts'");
			continue;

		case CD_INITIAL_CONTACT:	/* --initialcontact */
			msg.initial_contact = true;
			continue;

		case CD_CISCO_UNITY:	/* --cisco-unity */
			msg.cisco_unity = true;
			continue;

		case CD_FAKE_STRONGSWAN:	/* --fake-strongswan */
			msg.fake_strongswan = true;
			continue;

		case CD_DPDDELAY:	/* --dpddelay */
			msg.dpd_delay = deltatime(opt_whole);
			continue;

		case CD_DPDTIMEOUT:	/* --dpdtimeout */
			msg.dpd_timeout = deltatime(opt_whole);
			continue;

		case CD_DPDACTION:	/* --dpdaction */
			msg.dpd_action = 255;
			if (streq(optarg, "clear"))
				msg.dpd_action = DPD_ACTION_CLEAR;
			else if (streq(optarg, "hold"))
				msg.dpd_action = DPD_ACTION_HOLD;
			else if (streq(optarg, "restart"))
				msg.dpd_action = DPD_ACTION_RESTART;
			else if (streq(optarg, "restart_by_peer"))
				/*
				 * obsolete (not advertised) option for
				 * compatibility
				 */
				msg.dpd_action = DPD_ACTION_RESTART;
			continue;

		case CD_SEND_REDIRECT:	/* --send-redirect */
		{
			lset_t new_policy = LEMPTY;

			if (streq(optarg, "yes"))
				new_policy |= POLICY_SEND_REDIRECT_ALWAYS;
			else if (streq(optarg, "no"))
				new_policy |= POLICY_SEND_REDIRECT_NEVER;
			else if (streq(optarg, "auto"))
				new_policy = LEMPTY;	/* avoid compiler error for no expression */
			else
				diagw("--send-redirect options are 'yes', 'no' or 'auto'");

			msg.policy = msg.policy & ~(POLICY_SEND_REDIRECT_MASK);
			msg.policy |= new_policy;
		}
			continue;

		case CD_REDIRECT_TO:	/* --redirect-to */
			msg.redirect_to = strdup(optarg);
			continue;

		case CD_ACCEPT_REDIRECT:	/* --accept-redirect */
		{
			lset_t new_policy = LEMPTY;

			if (streq(optarg, "yes"))
				new_policy |= POLICY_ACCEPT_REDIRECT_YES;
			else if (streq(optarg, "no"))
				new_policy |= LEMPTY;
			else
				diagw("--accept-redirect options are 'yes' and 'no'");

			if (new_policy != LEMPTY)
				msg.policy |= new_policy;
			else
				msg.policy = msg.policy & ~POLICY_ACCEPT_REDIRECT_YES;
		}
			continue;

		case CD_ACCEPT_REDIRECT_TO:	/* --accept-redirect-to */
			msg.accept_redirect_to = strdup(optarg);
			continue;

		case CD_IKE:	/* --ike <ike_alg1,ike_alg2,...> */
			msg.ike = optarg;
			continue;

		case CD_PFSGROUP:	/* --pfsgroup modpXXXX */
			msg.pfsgroup = optarg;
			continue;

		case CD_ESP:	/* --esp <esp_alg1,esp_alg2,...> */
			msg.esp = optarg;
			continue;

		case CD_REMOTEPEERTYPE:	/* --remote-peer-type <cisco> */
			if (streq(optarg, "cisco"))
				msg.remotepeertype = CISCO;
			else
				msg.remotepeertype = NON_CISCO;
			continue;

#ifdef HAVE_NM
		case CD_NMCONFIGURED:	/* --nm-configured */
			msg.nmconfigured = true;
			continue;
#endif

		case CD_IKE_TCP: /* --tcp */
			if (streq(optarg, "yes"))
				msg.iketcp = IKE_TCP_ONLY;
			else if (streq(optarg, "no"))
				msg.iketcp = IKE_TCP_NO;
			else if (streq(optarg, "fallback"))
				msg.iketcp = IKE_TCP_FALLBACK;
			else
				diagw("--tcp-options are 'yes', 'no' or 'fallback'");
			continue;

		case CD_LABELED_IPSEC:	/* obsolete --labeledipsec */
			/* ignore */
			continue;

		case CD_SEC_LABEL:	/* --sec-label */
			/* we only support symmetric labels but put it in struct end */
			msg.sec_label = optarg;
			continue;

		case ALGO_RSASIG: /* --rsasig */
			auth_specified = true;
			msg.policy |= POLICY_RSASIG;
			msg.policy |= POLICY_RSASIG_v1_5;
			msg.sighash_policy |= POL_SIGHASH_SHA2_256;
			msg.sighash_policy |= POL_SIGHASH_SHA2_384;
			msg.sighash_policy |= POL_SIGHASH_SHA2_512;
			continue;
		case ALGO_RSA_SHA1: /* --rsa-sha1 */
			auth_specified = true;
			msg.policy |= POLICY_RSASIG_v1_5;
			continue;
		case ALGO_RSA_SHA2: /* --rsa-sha2 */
			auth_specified = true;
			msg.policy |= POLICY_RSASIG;
			msg.sighash_policy |= POL_SIGHASH_SHA2_256;
			msg.sighash_policy |= POL_SIGHASH_SHA2_384;
			msg.sighash_policy |= POL_SIGHASH_SHA2_512;
			continue;
		case ALGO_RSA_SHA2_256:	/* --rsa-sha2_256 */
			auth_specified = true;
			msg.sighash_policy |= POL_SIGHASH_SHA2_256;
			msg.policy |= POLICY_RSASIG;
			continue;
		case ALGO_RSA_SHA2_384:	/* --rsa-sha2_384 */
			auth_specified = true;
			msg.sighash_policy |= POL_SIGHASH_SHA2_384;
			msg.policy |= POLICY_RSASIG;
			continue;
		case ALGO_RSA_SHA2_512:	/* --rsa-sha2_512 */
			auth_specified = true;
			msg.sighash_policy |= POL_SIGHASH_SHA2_512;
			msg.policy |= POLICY_RSASIG;
			continue;

		case ALGO_ECDSA: /* --ecdsa and --ecdsa-sha2 */
			auth_specified = true;
			msg.policy |= POLICY_ECDSA;
			msg.sighash_policy |= POL_SIGHASH_SHA2_256;
			msg.sighash_policy |= POL_SIGHASH_SHA2_384;
			msg.sighash_policy |= POL_SIGHASH_SHA2_512;
			continue;
		case ALGO_ECDSA_SHA2_256:	/* --ecdsa-sha2_256 */
			auth_specified = true;
			msg.sighash_policy |= POL_SIGHASH_SHA2_256;
			msg.policy |= POLICY_ECDSA;
			continue;
		case ALGO_ECDSA_SHA2_384:	/* --ecdsa-sha2_384 */
			auth_specified = true;
			msg.sighash_policy |= POL_SIGHASH_SHA2_384;
			msg.policy |= POLICY_ECDSA;
			continue;
		case ALGO_ECDSA_SHA2_512:	/* --ecdsa-sha2_512 */
			auth_specified = true;
			msg.sighash_policy |= POL_SIGHASH_SHA2_512;
			msg.policy |= POLICY_ECDSA;
			continue;

		case CD_CONNIPV4:	/* --ipv4; mimic --ipv6 */
			if (host_family.type == &ipv4_info) {
				/* ignore redundant options */
				continue;
			}

			if (LHAS(cd_seen, CD_CONNIPV6 - CD_FIRST)) {
				/* i.e., --ipv6 ... --ipv4 */
				diagw("--ipv4 conflicts with --ipv6");
			}

			if (host_family.used_by != NULL) {
				/* i.e., --host ::1 --ipv4; useful? wrong message? */
				diagq("--ipv4 must precede", host_family.used_by);
			}
			host_family.used_by = long_opts[long_index].name;
			host_family.type = &ipv4_info;

			/*
			 * Consider defaulting client_family to
			 * AF_INET6.  Do so only if it hasn't yet been
			 * specified or used.
			 */
			if (LDISJOINT(cd_seen,
				      LELEM(CD_TUNNELIPV4 - CD_FIRST) |
				      LELEM(CD_TUNNELIPV6 - CD_FIRST)) &&
			    client_family.used_by == NULL) {
				client_family.used_by = long_opts[long_index].name;
				client_family.type = &ipv4_info;
			}
			continue;

		case CD_CONNIPV6:	/* --ipv6; mimic ipv4 */
			if (host_family.type == &ipv6_info) {
				/* ignore redundant options */
				continue;
			}

			if (LHAS(cd_seen, CD_CONNIPV4 - CD_FIRST)) {
				/* i.e., --ipv4 ... --ipv6 */
				diagw("--ipv6 conflicts with --ipv4");
			}

			if (host_family.used_by != NULL) {
				/* i.e., --host 0.0.0.1 --ipv6; useful? wrong message? */
				diagq("--ipv6 must precede", host_family.used_by);
			}
			host_family.used_by = long_opts[long_index].name;
			host_family.type = &ipv6_info;

			/*
			 * Consider defaulting client_family to
			 * AF_INET6.  Do so only if it hasn't yet been
			 * specified or used.
			 */
			if (LDISJOINT(cd_seen,
				      LELEM(CD_TUNNELIPV4 - CD_FIRST) |
				      LELEM(CD_TUNNELIPV6 - CD_FIRST)) &&
			    client_family.used_by == NULL) {
				client_family.used_by = long_opts[long_index].name;
				client_family.type = &ipv6_info;
			}
			continue;

		case CD_TUNNELIPV4:	/* --tunnelipv4 */
			if (LHAS(cd_seen, CD_TUNNELIPV6 - CD_FIRST))
				diagw("--tunnelipv4 conflicts with --tunnelipv6");
			if (client_family.used_by != NULL)
				diagq("--tunnelipv4 must precede", client_family.used_by);
			client_family.used_by = long_opts[long_index].name;
			client_family.type = &ipv4_info;
			continue;

		case CD_TUNNELIPV6:	/* --tunnelipv6 */
			if (LHAS(cd_seen, CD_TUNNELIPV4 - CD_FIRST))
				diagw("--tunnelipv6 conflicts with --tunnelipv4");
			if (client_family.used_by != NULL)
				diagq("--tunnelipv6 must precede", client_family.used_by);
			client_family.used_by = long_opts[long_index].name;
			client_family.type = &ipv6_info;
			continue;

		case END_XAUTHSERVER:	/* --xauthserver */
			msg.right.xauth_server = true;
			continue;

		case END_XAUTHCLIENT:	/* --xauthclient */
			msg.right.xauth_client = true;
			continue;

		case OPT_USERNAME:	/* --username, was --xauthname */
			/*
			 * we can't tell if this is going to be --initiate, or
			 * if this is going to be an conn definition, so do
			 * both actions
			 */
			msg.right.xauth_username = optarg;
			gotusername = true;
			/* ??? why does this length include NUL? */
			usernamelen = jam_str(xauthusername, sizeof(xauthusername),
					optarg) -
				xauthusername + 1;
			continue;

		case OPT_XAUTHPASS:	/* --xauthpass */
			gotxauthpass = true;
			/* ??? why does this length include NUL? */
			xauthpasslen = jam_str(xauthpass, sizeof(xauthpass),
					optarg) -
				xauthpass + 1;
			continue;

		case END_MODECFGCLIENT:	/* --modeconfigclient */
			msg.right.modecfg_client = true;
			continue;

		case END_MODECFGSERVER:	/* --modeconfigserver */
			msg.right.modecfg_server = true;
			continue;

		case END_ADDRESSPOOL:	/* --addresspool */
			diagq(ttorange(optarg, NULL, &msg.right.pool_range), optarg);
			continue;

		case CD_MODECFGDNS:	/* --modecfgdns */
			msg.modecfg_dns = strdup(optarg);
			continue;
		case CD_MODECFGDOMAINS:	/* --modecfgdomains */
			msg.modecfg_domains = strdup(optarg);
			continue;
		case CD_MODECFGBANNER:	/* --modecfgbanner */
			msg.modecfg_banner = strdup(optarg);
			continue;

		case CD_CONN_MARK_BOTH:      /* --conn-mark */
			msg.conn_mark_both = strdup(optarg);
			continue;
		case CD_CONN_MARK_IN:      /* --conn-mark-in */
			msg.conn_mark_in = strdup(optarg);
			continue;
		case CD_CONN_MARK_OUT:      /* --conn-mark-out */
			msg.conn_mark_out = strdup(optarg);
			continue;

		case CD_VTI_IFACE:      /* --vti-iface */
			if (optarg != NULL && strlen(optarg) < IFNAMSIZ)
				msg.vti_iface = strdup(optarg);
			else
				fprintf(stderr, "whack: invalid interface name '%s' ignored\n",
					optarg);
			continue;
		case CD_VTI_ROUTING:	/* --vti-routing */
			msg.vti_routing = true;
			continue;
		case CD_VTI_SHARED:	/* --vti-shared */
			msg.vti_shared = true;
			continue;

		case CD_IPSEC_IFACE:      /* --ipsec-interface */
			msg.xfrm_if_id = opt_whole;
			continue;

		case CD_XAUTHBY:	/* --xauthby */
			if (streq(optarg, "file")) {
				msg.xauthby = XAUTHBY_FILE;
				continue;
#ifdef AUTH_HAVE_PAM
			} else if (streq(optarg, "pam")) {
				msg.xauthby = XAUTHBY_PAM;
				continue;
#endif
			} else if (streq(optarg, "alwaysok")) {
				msg.xauthby = XAUTHBY_ALWAYSOK;
				continue;
			} else {
				diagq("whack: unknown xauthby method", optarg);
			}
			continue;

		case CD_XAUTHFAIL:	/* --xauthfail */
			if (streq(optarg, "hard")) {
				msg.xauthfail = XAUTHFAIL_HARD;
				continue;
			} else if (streq(optarg, "soft")) {
				msg.xauthfail = XAUTHFAIL_SOFT;
				continue;
			} else {
				fprintf(stderr,
					"whack: unknown xauthfail method '%s' ignored\n",
					optarg);
			}
			continue;

		case CD_METRIC:	/* --metric */
			msg.metric = opt_whole;
			continue;

		case CD_CONNMTU:	/* --mtu */
			msg.connmtu = opt_whole;
			continue;

		case CD_PRIORITY:	/* --priority */
			msg.sa_priority = opt_whole;
			continue;

		case CD_TFCPAD:	/* --tfc */
			msg.sa_tfcpad = opt_whole;
			continue;

		case CD_SEND_TFCPAD:	/* --send-no-esp-tfc */
			msg.send_no_esp_tfc = true;
			continue;

		case CD_NFLOG_GROUP:	/* --nflog-group */
			if (opt_whole <= 0  ||
			    opt_whole > 65535) {
				char buf[120];

				snprintf(buf, sizeof(buf),
					"invalid nflog-group value - range must be 1-65535 \"%s\"",
					optarg);
				diagw(buf);
			}
			msg.nflog_group = opt_whole;
			continue;

		case CD_REQID:	/* --reqid */
			if (opt_whole <= 0  ||
			    opt_whole > IPSEC_MANUAL_REQID_MAX) {
				char buf[120];

				snprintf(buf, sizeof(buf),
					"invalid reqid value - range must be 1-%u \"%s\"",
					IPSEC_MANUAL_REQID_MAX,
					optarg);
				diagw(buf);
			}

			msg.sa_reqid = opt_whole;
			continue;

		case DBGOPT_NONE:	/* --debug-none (obsolete) */
			/*
			 * Clear all debug and impair options.
			 *
			 * This preserves existing behaviour where
			 * sequences like:
			 *
			 *     --debug-none
			 *     --debug-none --debug something
			 *
			 * force all debug/impair options to values
			 * defined by whack.
			 */
			msg.debugging = lmod_clr(msg.debugging, DBG_MASK);
			continue;

		case DBGOPT_ALL:	/* --debug-all (obsolete) */
			/*
			 * Set most debug options ('all' does not
			 * include PRIVATE which is cleared) and clear
			 * all impair options.
			 *
			 * This preserves existing behaviour where
			 * sequences like:
			 *
			 *     --debug-all
			 *     --debug-all --impair something
			 *
			 * force all debug/impair options to values
			 * defined by whack.
			 */
			msg.debugging = lmod_clr(msg.debugging, DBG_MASK);
			msg.debugging = lmod_set(msg.debugging, DBG_ALL);
			continue;

		case DBGOPT_DEBUG:	/* --debug */
		case DBGOPT_NO_DEBUG:	/* --no-debug */
		{
			bool enable = (c == DBGOPT_DEBUG);
			if (streq(optarg, "list") || streq(optarg, "help") || streq(optarg, "?")) {
				fprintf(stderr, "aliases:\n");
				for (struct lmod_alias *a = debug_lmod_info.aliases;
				     a->name != NULL; a++) {
					JAMBUF(buf) {
						jam(buf, "  %s: ", a->name);
						jam_lset_short(buf, debug_lmod_info.names, "+", a->bits);
						fprintf(stderr, PRI_SHUNK"\n",
							pri_shunk(jambuf_as_shunk(buf)));
					}
				}
				fprintf(stderr, "bits:\n");
				for (long e = next_enum(&debug_names, -1);
				     e != -1; e = next_enum(&debug_names, e)) {
					JAMBUF(buf) {
						jam(buf, "  ");
						jam_enum_short(buf, &debug_names, e);
						const char *help = enum_name(&debug_help, e);
						if (help != NULL) {
							jam(buf, ": ");
							jam_string(buf, help);
						}
						fprintf(stderr, PRI_SHUNK"\n",
							pri_shunk(jambuf_as_shunk(buf)));
					}
				}
				exit(1);
			} else if (!lmod_arg(&msg.debugging, &debug_lmod_info, optarg, enable)) {
				fprintf(stderr, "whack: unrecognized -%s-debug '%s' option ignored\n",
					enable ? "" : "-no", optarg);
			}
			continue;
		}

		case DBGOPT_IMPAIR:	/* --impair */
		case DBGOPT_NO_IMPAIR:	/* --no-impair */
		{
			bool enable = (c == DBGOPT_IMPAIR);
			realloc_things(msg.impairments, msg.nr_impairments,
				       msg.nr_impairments+1, "impairments");
			switch (parse_impair(optarg, &msg.impairments[msg.nr_impairments],
					     enable, logger)) {
			case IMPAIR_OK:
				break;
			case IMPAIR_HELP:
				/* parse_impair() printed help */
				exit(0);
			case IMPAIR_ERROR:
				/* parse_impair() printed the error */
				exit(1);
			}
			msg.nr_impairments++;
			continue;
		}

		default:
			bad_case(c);
		}
		break;
	}

	if (msg.ike_version == 0) {
		/* no ike version specified, default to IKEv2 */
		msg.ike_version = IKEv2;
	}

	switch (msg.ike_version) {
	case IKEv1:
		if (msg.policy & POLICY_ECDSA)
			diagw("connection cannot specify --ecdsa and --ikev1");
		/* delete any inherited sighash_poliyc from --rsasig including sha2 */
		msg.sighash_policy = LEMPTY;
		break;
	case IKEv2:
		if (msg.policy & POLICY_AGGRESSIVE)
			diagw("connection cannot specify --ikev2 and --aggressive");
		break;
	}

	if (client_family.type != NULL) {
		msg.tunnel_addr_family = client_family.type->af;
	}

	if (!auth_specified) {
		/* match the parser loading defaults to whack defaults */
		msg.policy |= POLICY_RSASIG;
		if (msg.ike_version == IKEv2) {
			msg.policy |= POLICY_RSASIG_v1_5;
			msg.policy |= POLICY_ECDSA;
			msg.sighash_policy |= POL_SIGHASH_SHA2_256;
			msg.sighash_policy |= POL_SIGHASH_SHA2_384;
			msg.sighash_policy |= POL_SIGHASH_SHA2_512;
		}
	}

	if (optind != argc) {
		/*
		 * If you see this message unexpectedly, perhaps the
		 * case for the previous option ended with "break"
		 * instead of "continue"
		 */
		diagq("unexpected argument", argv[optind]);
	}

	/*
	 * For each possible form of the command, figure out if an argument
	 * suggests whether that form was intended, and if so, whether all
	 * required information was supplied.
	 */

	/* check opportunistic initiation simulation request */
	switch (opts1_seen & (LELEM(OPT_OPPO_HERE) | LELEM(OPT_OPPO_THERE))) {
	case LELEM(OPT_OPPO_HERE):
	case LELEM(OPT_OPPO_THERE):
		diagw("--oppohere and --oppothere must be used together");
		/*NOTREACHED*/
	case LELEM(OPT_OPPO_HERE) | LELEM(OPT_OPPO_THERE):
		msg.whack_oppo_initiate = true;
		if (LIN(cd_seen,
			LELEM(CD_TUNNELIPV4 -
			      CD_FIRST) | LELEM(CD_TUNNELIPV6 - CD_FIRST)))
			opts1_seen &= ~LELEM(OPT_CD);
		break;
	}

	/* check connection description */
	if (LHAS(opts1_seen, OPT_CD)) {
		if (!LHAS(cd_seen, CD_TO - CD_FIRST))
			diagw("connection description option, but no --to");

		if (!LHAS(end_seen, END_HOST - END_FIRST))
			diagw("connection missing --host after --to");

		if (msg.policy & POLICY_OPPORTUNISTIC) {
			if ((msg.policy & (POLICY_PSK | POLICY_RSASIG)) !=
			    POLICY_RSASIG)
				diagw("only RSASIG is supported for opportunism");

			if ((msg.policy & POLICY_PFS) == 0)
				diagw("PFS required for opportunism");
			if ((msg.policy & POLICY_ENCRYPT) == 0)
				diagw("encryption required for opportunism");
		}

		check_end(&msg.left, &msg.right, host_family.type, client_family.type);
		check_end(&msg.right, &msg.left, host_family.type, client_family.type);

		if (subnet_type(&msg.left.client) !=
		    subnet_type(&msg.right.client))
			diagw("endpoints clash: one is IPv4 and the other is IPv6");

		if (msg.policy & POLICY_AUTH_NEVER) {
			if (msg.prospective_shunt == SHUNT_TRAP ||
			    msg.prospective_shunt == SHUNT_UNSET) {
				diagw("shunt connection must have shunt policy (eg --pass, --drop or --reject). Is this a non-shunt connection missing an authentication method such as --psk or --rsasig or --auth-null ?");
			}
		} else {
			/* not just a shunt: a real ipsec connection */
			if ((msg.policy & POLICY_ID_AUTH_MASK) == LEMPTY &&
				msg.left.authby == AUTHBY_NEVER && msg.right.authby == AUTHBY_NEVER)
				diagw("must specify connection authentication, eg --rsasig, --psk or --auth-null for non-shunt connection");
			/*
			 * ??? this test can never fail:
			 *	!NEVER_NEGOTIATE=>HAS_IPSEC_POLICY
			 * These interlocking tests should be redone.
			 */
			if (!HAS_IPSEC_POLICY(msg.policy) &&
			    (msg.left.client.is_set || msg.right.client.is_set))
				diagw("must not specify clients for ISAKMP-only connection");
		}

		msg.whack_connection = true;
	}

	/* decide whether --name is mandatory or forbidden */
	if (!LDISJOINT(opts1_seen,
		       LELEM(OPT_ROUTE) | LELEM(OPT_UNROUTE) |
		       LELEM(OPT_INITIATE) | LELEM(OPT_TERMINATE) |
		       LELEM(OPT_DELETE) |  LELEM(OPT_DELETEID) |
		       LELEM(OPT_DELETEUSER) | LELEM(OPT_CD) |
		       LELEM(OPT_REKEY_IKE) |
		       LELEM(OPT_REKEY_IPSEC))) {
		if (!LHAS(opts1_seen, OPT_NAME))
			diagw("missing --name <connection_name>");
	} else if (msg.whack_options == LEMPTY) {
		if (LHAS(opts1_seen, OPT_NAME) && !LELEM(OPT_TRAFFIC_STATUS))
			diagw("no reason for --name");
	}

	if (!LDISJOINT(opts1_seen, LELEM(OPT_REMOTE_HOST))) {
		if (!LHAS(opts1_seen, OPT_INITIATE))
			diagw("--remote-host can only be used with --initiate");
	}

	if (!LDISJOINT(opts1_seen, LELEM(OPT_PUBKEYRSA) | LELEM(OPT_ADDKEY))) {
		if (!LHAS(opts1_seen, OPT_KEYID))
			diagw("--addkey and --pubkeyrsa require --keyid");
	}

	if (!(msg.whack_connection || msg.whack_key ||
	      msg.whack_delete ||msg.whack_deleteid || msg.whack_deletestate ||
	      msg.whack_deleteuser || msg.active_redirect_dests != NULL ||
	      msg.global_redirect || msg.global_redirect_to ||
	      msg.whack_initiate || msg.whack_oppo_initiate ||
	      msg.whack_terminate ||
	      msg.whack_route || msg.whack_unroute || msg.whack_listen ||
	      msg.whack_unlisten || msg.whack_list || msg.ike_buf_size ||
	      msg.whack_ddos != DDOS_undefined || msg.whack_ddns ||
	      msg.whack_reread || msg.whack_crash || msg.whack_shunt_status ||
	      msg.whack_status || msg.whack_global_status || msg.whack_traffic_status ||
	      msg.whack_addresspool_status ||
	      msg.whack_process_status ||
	      msg.whack_fips_status || msg.whack_brief_status || msg.whack_clear_stats || msg.whack_options ||
	      msg.whack_shutdown || msg.whack_purgeocsp || msg.whack_seccomp_crashtest || msg.whack_show_states ||
	      msg.whack_rekey_ike || msg.whack_rekey_ipsec ||
	      msg.whack_listpubkeys || msg.whack_checkpubkeys))
		diagw("no action specified; try --help for hints");

	if (msg.policy & POLICY_AGGRESSIVE) {
		if (msg.ike == NULL)
			diagw("cannot specify aggressive mode without ike= to set algorithm");
	}

	/*
	 * Check for wild values
	 * Must never overflow: rekeymargin*(100+rekeyfuzz)/100
	 * We don't know the maximum value for a time_t, so we use INT_MAX
	 * ??? this should be checked wherever any of these is set in Pluto
	 * too.
	 */
	if (msg.sa_rekey_fuzz > INT_MAX - 100 ||
	    deltasecs(msg.sa_rekey_margin) > (time_t)(INT_MAX / (100 + msg.sa_rekey_fuzz)))
		diagw("rekeymargin or rekeyfuzz values are so large that they cause overflow");

	check_life_time(msg.sa_ike_life_seconds, IKE_SA_LIFETIME_MAXIMUM,
			"ikelifetime", &msg);

	check_life_time(msg.sa_ipsec_life_seconds, IPSEC_SA_LIFETIME_MAXIMUM,
			"ipseclifetime", &msg);

	if (deltasecs(msg.dpd_delay) != 0 &&
	    deltasecs(msg.dpd_timeout) == 0)
		diagw("dpddelay specified, but dpdtimeout is zero, both should be specified");

	if (deltasecs(msg.dpd_delay) == 0 &&
	    deltasecs(msg.dpd_timeout) != 0)
		diagw("dpdtimeout specified, but dpddelay is zero, both should be specified");

	switch (msg.dpd_action) {
	case DPD_ACTION_DISABLED:
	case DPD_ACTION_CLEAR:
	case DPD_ACTION_HOLD:
	case DPD_ACTION_RESTART:
		break;
	default:
		diagw("dpdaction can only be \"clear\", \"hold\" or \"restart\"");
	}

	if (msg.remotepeertype != CISCO &&
	    msg.remotepeertype != NON_CISCO) {
		diagw("remote-peer-type can only be \"CISCO\" or \"NON_CISCO\" - defaulting to non-cisco mode");
		msg.remotepeertype = NON_CISCO;	/*NON_CISCO=0*/
	}

	/* pack strings for inclusion in message */
	wp.msg = &msg;

	/* build esp message as esp="<esp>;<pfsgroup>" */
	if (msg.pfsgroup != NULL) {
		snprintf(esp_buf, sizeof(esp_buf), "%s;%s",
			 msg.esp != NULL ? msg.esp : "",
			 msg.pfsgroup != NULL ? msg.pfsgroup : "");
		msg.esp = esp_buf;
	}
	ugh = pack_whack_msg(&wp);
	if (ugh != NULL)
		diagw(ugh);

	msg.magic = ((opts1_seen & ~(LELEM(OPT_SHUTDOWN) | LELEM(OPT_STATUS))) |
		     opts2_seen | lst_seen | cd_seen) != LEMPTY ||
		    msg.whack_options ?
		    WHACK_MAGIC : WHACK_BASIC_MAGIC;

	/* send message to Pluto */
	if (access(ctl_addr.sun_path, R_OK | W_OK) < 0) {
		int e = errno;

		switch (e) {
		case EACCES:
			fprintf(stderr,
				"whack: no right to communicate with pluto (access(\"%s\"))\n",
				ctl_addr.sun_path);
			break;
		case ENOENT:
			fprintf(stderr,
				"whack: Pluto is not running (no \"%s\")\n",
				ctl_addr.sun_path);
			break;
		default:
			fprintf(stderr,
				"whack: access(\"%s\") failed with %d %s\n",
				ctl_addr.sun_path, errno, strerror(e));
			break;
		}
		exit(RC_WHACK_PROBLEM);
	}

	int sock = safe_socket(AF_UNIX, SOCK_STREAM, 0);
	int exit_status = 0;
	ssize_t len = wp.str_next - (unsigned char *)&msg;

	if (sock == -1) {
		int e = errno;

		fprintf(stderr, "whack: socket() failed (%d %s)\n", e, strerror(
				e));
		exit(RC_WHACK_PROBLEM);
	}

	if (connect(sock, (struct sockaddr *)&ctl_addr,
		    offsetof(struct sockaddr_un,
			     sun_path) + strlen(ctl_addr.sun_path)) < 0)
	{
		int e = errno;

		fprintf(stderr,
			"whack:%s connect() for \"%s\" failed (%d %s)\n",
			e == ECONNREFUSED ? " is Pluto running? " : "",
			ctl_addr.sun_path, e, strerror(e));
		exit(RC_WHACK_PROBLEM);
	}

	if (write(sock, &msg, len) != len) {
		int e = errno;

		fprintf(stderr, "whack: write() failed (%d %s)\n",
			e, strerror(e));
		exit(RC_WHACK_PROBLEM);
	}

	/* for now, just copy reply back to stdout */

	char buf[4097];	/* arbitrary limit on log line length */
	char *be = buf;

	for (;;) {
		char *ls = buf;
		ssize_t rl = read(sock, be, (buf + sizeof(buf) - 1) - be);

		if (rl < 0) {
			int e = errno;

			fprintf(stderr,
				"whack: read() failed (%d %s)\n",
				e, strerror(e));
			exit(RC_WHACK_PROBLEM);
		}
		if (rl == 0) {
			if (be != buf)
				fprintf(stderr,
					"whack: last line from pluto too long or unterminated\n");
			break;
		}

		be += rl;
		*be = '\0';

		for (;;) {
			char *le = strchr(ls, '\n');

			if (le == NULL) {
				/*
				 * move last, partial line
				 * to start of buffer
				 */
				memmove(buf, ls, be - ls);
				be -= ls - buf;
				break;
			}
			le++;	/* include NL in line */

			/*
			 * figure out prefix number and how it should
			 * affect our exit status and printing
			 */
			char *lpe = NULL; /* line-prefix-end */
			unsigned long s = strtoul(ls, &lpe, 10);
			if (lpe == ls || *lpe != ' ') {
				/* includes embedded NL, see above */
				fprintf(stderr, "whack: log line missing NNN prefix: %*s",
					(int)(le - ls), ls);
#if 0
				ls = le;
				continue;
#else
				exit(RC_WHACK_PROBLEM);
#endif
			}
			if (s == RC_RAW) {
				ls = lpe + 1; /* skip NNN_ */
			}

			if (write(STDOUT_FILENO, ls, le - ls) != (le - ls)) {
				int e = errno;

				fprintf(stderr,
					"whack: write() failed to stdout(%d %s)\n",
					e, strerror(e));
			}

			switch (s) {
			/* these logs are informational only */
			case RC_COMMENT:
			case RC_RAW:
			case RC_INFORMATIONAL:
			case RC_INFORMATIONAL_TRAFFIC:
			case RC_LOG:
			/* RC_LOG_SERIOUS is supposed to be here according to lswlog.h, but seems oudated? */
				/* ignore */
				break;
			case RC_SUCCESS:
				/* be happy */
				exit_status = 0;
				break;

			case RC_ENTERSECRET:
				if (!gotxauthpass) {
					xauthpasslen = whack_get_secret(
						xauthpass,
						sizeof(xauthpass));
				}
				send_reply(sock,
					   xauthpass,
					   xauthpasslen);
				break;

			case RC_USERPROMPT:
				if (!gotusername) {
					usernamelen = whack_get_value(
						xauthusername,
						sizeof(xauthusername));
				}
				send_reply(sock,
					   xauthusername,
					   usernamelen);
				break;

			default:
				/*
				 * Only RC_ codes between
				 * RC_EXIT_FLOOR (RC_DUPNAME) and
				 * RC_EXIT_ROOF are errors
				 */
				if (s > 0 && (s < RC_EXIT_FLOOR || s >= RC_EXIT_ROOF))
					s = 0;
				exit_status = msg.whack_async ?
					0 : s;
				break;
			}

			ls = le;
		}
	}

	if (ignore_errors)
		return 0;

	return exit_status;
}
