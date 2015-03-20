/*
 * (C) 2014-2015 by Christian Hesse <mail@eworm.de>
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

#include <systemd/sd-journal.h>

#include <libnotify/notify.h>

#include "version.h"

#define DEFAULTICON	"dialog-information"

/*** notify ***/
int notify(const char * summary, const char * body, const char * icon,
		int timeout, uint8_t urgency);

/*** main ***/
int main(int argc, char **argv);

#endif /* _JOURNAL_NOTIFY_H */

// vim: set syntax=c:

