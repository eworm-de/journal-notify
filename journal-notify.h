/*
 * (C) 2014-2023 by Christian Hesse <mail@eworm.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef _JOURNAL_NOTIFY_H
#define _JOURNAL_NOTIFY_H

#define _GNU_SOURCE

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <systemd/sd-journal.h>

#include <libnotify/notify.h>

#include "config.h"
#include "version.h"

#define PROGNAME	"journal-notify"

#define DEFAULTICON	"dialog-information"

const char * priorities[] = {
	"EMERG", /* 0 */
	"ALERT",
	"CRIT",
	"ERR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG" /* 7 */};

/*** notify ***/
int notify(const char * identifier, const char * message, uint8_t priority,
		const char * icon, int timeout);

/*** main ***/
int main(int argc, char **argv);

#endif /* _JOURNAL_NOTIFY_H */

// vim: set syntax=c:

