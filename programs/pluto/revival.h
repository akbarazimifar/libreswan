/* revive connection, for libreswan
 *
 * Copyright (C) 2021 Andrew Cagney
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

#ifndef REVIVAL_H
#define REVIVAL_H

struct logger;
struct state;
struct connection;
struct child_sa;
struct timer_event;

void revive_connection(struct connection *c, const char *subplot,
		       const threadtime_t *inception);
bool should_revive(struct state *st);
void schedule_revival(struct state *st, const char *subplot);

bool should_revive_child(struct child_sa *child);
void schedule_child_revival(struct ike_sa *ike, struct child_sa *child, const char *subplot);

void delete_revival(const struct connection *c);

void init_revival_timer(void);

#endif
