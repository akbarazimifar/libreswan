/* impair message send/recv, for libreswan
 *
 * Copyright (C) 2020,2023  Andrew Cagney
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
 *
 */

#include <errno.h>

#include "impair.h"
#include "lswalloc.h"
#include "passert.h"
#include "lswlog.h"

#include "chunk.h"

#include "defs.h"
#include "log.h"
#include "demux.h"
#include "impair_message.h"
#include "state.h"
#include "iface.h"

struct message;
struct direction_impairment;

static void drip_message(const struct direction_impairment *direction,
			 const struct message *m, const char *reason,
			 struct logger *logger);

static void drip_inbound(const struct message *m, struct logger *logger);
static void drip_outbound(const struct message *m, struct logger *logger);

/*
 * Track messages to impair.
 */

static bool impair_messages;

struct message {
	unsigned nr;
	chunk_t body;
	struct {
		struct msg_digest *md;
	} inbound;
	struct {
		/*
		 * Danger: assumes message is deleted before
		 * interface.  Deleting an interface probably crashes.
		 */
		const struct iface_endpoint *interface;
		ip_endpoint endpoint;
	} outbound;
	struct list_entry entry;
};

static size_t jam_message(struct jambuf *buf, const struct message *r)
{
	return jam(buf, "replay packet %lu", r == NULL ? 0L : r->nr);
}

LIST_INFO(message, entry, message_info, jam_message);

struct message_impairment {
	unsigned message_nr;
	struct message_impairment *next;
};

struct direction_impairment {
	const char *name;
	struct message_impairment *impairments;
	struct list_head messages;
	unsigned nr_messages;
	bool replay_duplicates;
	bool replay_forward;
	bool replay_backward;
	bool block;
	void (*drip)(const struct message *m, struct logger *logger);
};

static struct direction_impairment inbound_impairments = {
	.name = "inbound",
	.messages = INIT_LIST_HEAD(&inbound_impairments.messages, &message_info),
	.drip = drip_inbound,
};

static struct direction_impairment outbound_impairments = {
	.name = "outbound",
	.messages = INIT_LIST_HEAD(&outbound_impairments.messages, &message_info),
	.drip = drip_outbound,
};

struct direction_impairment *const message_impairments[] = {
	[IMPAIR_INBOUND_MESSAGE] = &inbound_impairments,
	[IMPAIR_OUTBOUND_MESSAGE] = &outbound_impairments,
};

/*
 * Track each unique message.
 */

static const struct message *save_message(struct direction_impairment *direction,
					  shunk_t message,
					  struct msg_digest *inbound_md,
					  const struct iface_endpoint *outbound_interface,
					  const ip_endpoint outbound_endpoint)
{
	unsigned nr = 0;
	struct message *old;
	FOR_EACH_LIST_ENTRY_OLD2NEW(old, &direction->messages) {
		nr++;
		if (hunk_eq(old->body, message)) {
			if (DBGP(DBG_BASE)) {
				DBG_log("matching %u", nr);
				DBG_dump_hunk(NULL, old->body);
			}
			return old;
		}
	}
	/* save new */
	struct message *new = alloc_thing(struct message, "message");
	new->body = clone_hunk(message, "message-body");
	new->inbound.md = md_addref(inbound_md);
	new->outbound.interface = outbound_interface;
	new->outbound.endpoint = outbound_endpoint;
	direction->nr_messages++;
	new->nr = direction->nr_messages;
	init_list_entry(&message_info, new, &new->entry); /* back-link */
	insert_list_entry(&direction->messages, &new->entry);
	return new;
}

static const struct message *save_inbound(struct msg_digest *md)
{
	return save_message(&inbound_impairments,
			    pbs_in_all(&md->packet_pbs),
			    /*inbound.md*/md,
			    /*outbound.interface*/NULL,
			    /*outbound.endpoint*/unset_endpoint);
}

static const struct message *save_outbound(shunk_t message,
					   const struct iface_endpoint *interface,
					   const ip_endpoint endpoint)
{
	return save_message(&outbound_impairments, message,
			    /*inbound.md*/NULL,
			    interface, endpoint);
}

/*
 * Find a message then drip feed it (to pluto or the peer).
 */

static void impair_message_drip(struct direction_impairment *direction,
				unsigned nr, struct logger *logger)
{
	struct message *m = NULL;
	FOR_EACH_LIST_ENTRY_OLD2NEW(m, &direction->messages) {
		if (m->nr == nr) {
			break;
		}
	}
	if (m == NULL) {
		llog(RC_LOG, logger, "IMPAIR: %s message %u not found",
		     direction->name, nr);
		return;
	}
	drip_message(direction, m, "drip", logger);
}

void add_message_impairment(enum impair_action impair_action,
			    enum impair_message_direction impair_direction,
			    unsigned biased_value, struct logger *logger)
{
	impair_messages = true; /* sticky */
	PASSERT(logger, impair_direction < elemsof(message_impairments));
	struct direction_impairment *direction = message_impairments[impair_direction];
	unsigned message_nr = biased_value - 1; /*unbias 0*/
	switch (impair_action) {
	case CALL_IMPAIR_MESSAGE_DROP:
		break;
	case CALL_IMPAIR_MESSAGE_BLOCK:
		direction->block = biased_value; /*0-or-1*/
		llog(RC_LOG, logger, "IMPAIR: block all %s messages: %s",
		     direction->name, bool_str(direction->block));
		return;
	case CALL_IMPAIR_MESSAGE_DRIP:
		impair_message_drip(direction, message_nr, logger);
		return;
	case CALL_IMPAIR_MESSAGE_REPLAY_DUPLICATES:
		direction->replay_duplicates = biased_value;
		return;
	case CALL_IMPAIR_MESSAGE_REPLAY_FORWARD:
		direction->replay_forward = biased_value;
		return;
	case CALL_IMPAIR_MESSAGE_REPLAY_BACKWARD:
		direction->replay_backward = biased_value;
		return;
	default:
		bad_case(impair_action);
	}
	PASSERT(logger, direction != NULL);
	llog(RC_LOG, logger, "IMPAIR: will drop %s message %u",
	     direction->name, message_nr);
	struct message_impairment *m = alloc_thing(struct message_impairment, "impair message");
	m->message_nr = message_nr;
	m->next = direction->impairments;
	direction->impairments = m;
}

static bool impair_message(const struct message *message,
			   struct direction_impairment *direction,
			   struct logger *logger)
{
	ldbg(logger, "%s message nr is %u", direction->name, message->nr);

	bool impaired = false;

	if (direction->block) {
		llog(RC_LOG, logger, "IMPAIR: blocking %s message %u",
		     direction->name, message->nr);
		return true;
	}

	for (struct message_impairment **mp = &direction->impairments; (*mp) != NULL; mp = &(*mp)->next) {
		struct message_impairment *m = (*mp);
		if (m->message_nr == message->nr) {
			/*
			 * Hack to find a whack to log to.
			 */
			bool whacked = false;
			struct state_filter sf = { .where = HERE, };
			while (next_state_new2old(&sf)) {
				struct state *st = sf.st;
				if (st->st_logger->object_whackfd != NULL ||
				    st->st_logger->global_whackfd != NULL) {
					llog(RC_LOG, st->st_logger,
					     "IMPAIR: drop %s message %u",
					     direction->name,
					     m->message_nr);
					whacked = true;
				}
			}
			if (!whacked) {
				llog(RC_LOG, logger, "IMPAIR: dropping %s message %u",
				     direction->name, message->nr);
			}
			/*
			 * Delete each impairment as it is consumed.
			 * This way, at the end of a successful test
			 * no impairments are left.
			 */
			*mp = m->next;
			pfree(m);
			impaired = true;
			break;
		}
	}

	if (direction->replay_duplicates) {
		/* MD is the most recent entry */
		drip_message(direction, message, "original", logger);
		drip_message(direction, message, "duplicate", logger);
		impaired = true;
	}

	if (direction->replay_forward) {
		struct message *m = NULL;
		FOR_EACH_LIST_ENTRY_OLD2NEW(m, &direction->messages) {
			drip_message(direction, m, "replay forward", logger);
		}
		impaired = true;
	}

	if (direction->replay_backward) {
		struct message *m = NULL;
		FOR_EACH_LIST_ENTRY_NEW2OLD(m, &direction->messages) {
			drip_message(direction, m, "replay backward", logger);
		}
		impaired = true;
	}

	return impaired;
}

static void free_direction(struct direction_impairment *direction, struct logger *logger)
{
	/*
	 * XXX: free the messages but NOT the impairments.  At the end
	 * of a test run all the impairments should have been consumed
	 * so leaking them acts as an additional red flag.
	 */

	struct message *m;
	FOR_EACH_LIST_ENTRY_OLD2NEW(m, &direction->messages) {
		free_chunk_content(&m->body);
		md_delref(&m->inbound.md);
		pfree(m);
	}

	if (direction->impairments != NULL) {
		llog(RC_LOG, logger, "IMPAIR: outstanding %s impairment",
			direction->name);
	}
}

static void drip_message(const struct direction_impairment *direction,
			 const struct message *m, const char *reason,
			 struct logger *logger)
{
	llog(RC_LOG, logger,
	     "IMPAIR: start processing %s %s packet %u",
	     direction->name, reason, m->nr);
	if (DBGP(DBG_BASE)) {
		llog_dump_hunk(DEBUG_STREAM, logger, m->body);
	}

	direction->drip(m, logger);

	llog(RC_LOG, logger,
	     "IMPAIR: stop processing %s %s packet %u",
	     direction->name, reason, m->nr);
}

static void drip_inbound(const struct message *m, struct logger *logger)
{
	struct msg_digest *md = clone_raw_md(m->inbound.md, HERE);
	md_attach(md, logger);
	process_md(md);
	md_delref(&md);
	pexpect(md == NULL);
}

static void drip_outbound(const struct message *m, struct logger *logger)
{
	const struct iface_endpoint *interface = m->outbound.interface;
	ssize_t wlen = interface->io->write_packet(interface,
						   HUNK_AS_SHUNK(m->body),
						   &m->outbound.endpoint,
						   logger);
	if (wlen != (ssize_t)m->body.len) {
		endpoint_buf lb;
		endpoint_buf rb;
		llog_error(logger, errno,
			   "send on %s from %s to %s using %s failed",
			   interface->ip_dev->id_rname,
			   str_endpoint(&interface->local_endpoint, &lb),
			   str_endpoint_sensitive(&m->outbound.endpoint, &rb),
			   interface->io->protocol->name);
	}
}

bool impair_inbound(struct msg_digest *md)
{
	if (!impair_messages) {
		return false;
	}

	const struct message *saved_message = save_inbound(md);
	return impair_message(saved_message, &inbound_impairments, md->md_logger);
}

bool impair_outbound(const struct iface_endpoint *interface, shunk_t message,
		     const ip_endpoint *endpoint, struct logger *logger)
{
	if (!impair_messages) {
		return false;
	}

	const struct message *saved_message = save_outbound(message, interface, *endpoint);
	return impair_message(saved_message, &outbound_impairments, logger);
}

void shutdown_impair_message(struct logger *logger)
{
	free_direction(&inbound_impairments, logger);
	free_direction(&outbound_impairments, logger);
}
