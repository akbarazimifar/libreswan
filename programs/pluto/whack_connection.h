/* whack receive routines, for libreswan
 *
 * Copyright (C) 2023  Andrew Cagney
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

#ifndef WHACK_CONNECTION_H
#define WHACK_CONNECTION_H

#include <stdbool.h>

#include "where.h"

struct child_sa;
struct ike_sa;
struct connection;
struct whack_message;
struct show;

struct each {
	const char *future_tense;
	const char *past_tense;
	bool log_unknown_name;
	bool skip_instances;
};

void whack_all_connections(const struct whack_message *m, struct show *s,
			   bool (*whack_connection)
			   (struct show *s,
			    struct connection **cp,
			    const struct whack_message *m));

void whack_each_connection(const struct whack_message *m, struct show *s,
			   bool (*whack_connection)
			   (struct show *s,
			    struct connection **cp,
			    const struct whack_message *m),
			   struct each each);

/*
 * Visit all the connections matching M, bottom up.
 *
 * This means that an instance is visited before it's template; and
 * group templates are visited before the matching group.
 *
 * Caller of whack_connection() takes a reference so never needs to
 * worry about connection disappearing.
 */
void whack_connections_bottom_up(const struct whack_message *m, struct show *s,
				 bool (*whack_connection)
				 (struct show *s,
				  struct connection *c,
				  const struct whack_message *m),
				 struct each each);

/*
 * Whack each of a connection's states in turn.
 */

enum whack_state {
	/*
	 * When connection has an established IKE SA.
	 */
	WHACK_START_IKE,
	/*
	 * Random stuff that gets in the way; typically just blown
	 * away.  These are returned in no particular order.
	 *
	 * The lurking IKE includes the larval IKE SA from a failed
	 * establish.
	 */
	WHACK_LURKING_IKE,
	WHACK_LURKING_CHILD,
	/*
	 * The connection's Child SA.
	 *
	 * A child still has its IKE SA.  An orphan lost its IKE SA
	 * (IKEv1) and a cuckoo is the child of someother IKE SA
	 * (which may or may not exist).
	 *
	 * This is returned before any siblings so that it gets
	 * priority for things like revival.
	 */
	WHACK_CHILD,
	WHACK_ORPHAN,
	WHACK_CUCKOO,
	/*
	 * Any IKE SA children that are not for the current
	 * connection.
	 *
	 * This is the reverse of a CUCKOO.
	 */
	WHACK_SIBLING,
	/*
	 * When the connection has no Child SA, or the connection's
	 * Child SA is for another IKE SA (ever the case?).
	 */
	WHACK_IKE,
	/* finally */
	WHACK_STOP_IKE,
};

void whack_connection_states(struct connection *c,
			     void (whack)(struct connection *c,
					  struct ike_sa **ike,
					  struct child_sa **child,
					  enum whack_state),
			     where_t where);

#endif
