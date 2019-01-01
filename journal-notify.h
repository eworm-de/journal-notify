/*
 * (C) 2014-2019 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
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

