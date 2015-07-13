/*
 * (C) 2014-2015 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "journal-notify.h"

const char * program = NULL;

const static char optstring[] = "aehi:m:nor:t:T:vVx:X:";
const static struct option options_long[] = {
	/* name			has_arg			flag	val */
	{ "and",		no_argument,		NULL,	'a' },
	{ "extended-regex",	no_argument,		NULL,	'e' },
	{ "help",		no_argument,		NULL,	'h' },
	{ "icon",		required_argument,	NULL,	'i' },
	{ "match",		required_argument,	NULL,	'm' },
	{ "no-case",		no_argument,		NULL,	'n' },
	{ "or",			no_argument,		NULL,	'o' },
	{ "regex",		required_argument,	NULL,	'r' },
	{ "timeout",		required_argument,	NULL,	't' },
	{ "throttle",		required_argument,	NULL,	'T' },
	{ "verbose",		no_argument,		NULL,	'v' },
	{ "version",		no_argument,		NULL,	'V' },
	{ "execute",		required_argument,	NULL, 	'x' },
	{ "execute-only",	required_argument,	NULL, 	'X' },
	{ 0, 0, 0, 0 }
};

/*** notify ***/
int notify(const char * identifier, const char * message, uint8_t priority,
		const char * icon, int timeout) {
	NotifyNotification * notification;
	char * identifier_markup = NULL, * message_markup = NULL;
	uint8_t urgency;
	int rc = -1;

	if ((message_markup = g_markup_escape_text(message, -1)) == NULL)
		goto out10;
	if ((identifier_markup = g_markup_escape_text(identifier, -1)) == NULL)
		goto out10;

	switch(priority) {
		case 0:
		case 1:
		case 2:
			urgency = NOTIFY_URGENCY_CRITICAL;
			break;
		case 3:
		case 4:
			urgency = NOTIFY_URGENCY_NORMAL;
			break;
		default: /* this catches priority 5, 6 and 7 */
			urgency = NOTIFY_URGENCY_LOW;
			break;
	}

	notification =
#if NOTIFY_CHECK_VERSION(0, 7, 0)
		notify_notification_new(identifier, message, icon);
#else
		notify_notification_new(identifier, message, icon, NULL);
#endif

	if (notification == NULL)
		goto out10;

	/* NOTIFY_EXPIRES_NEVER == 0 */
	if (timeout >= 0)
		notify_notification_set_timeout(notification, timeout);

	notify_notification_set_urgency(notification, urgency);

	if (notify_notification_show(notification, NULL) == FALSE)
		goto out20;

	rc = 0;

out20:
	g_object_unref(G_OBJECT(notification));

out10:
	if (message_markup)
		free(message_markup);
	if (identifier_markup)
		free(identifier_markup);

	return rc;
}

/*** main ***/
int main(int argc, char **argv) {
	int i, rc = EXIT_FAILURE;
	uint8_t verbose = 0;

	uint8_t have_regex = 0;
	regex_t regex;
	int regex_flags = REG_NOSUB;

	sd_journal * journal;
	uint8_t old_entry = 1;
	const void * data;
	size_t length;

	char * identifier, * message;
	uint8_t priority;
	const char * priorityname,
	      * icon = DEFAULTICON;
	int timeout = NOTIFICATION_TIMEOUT;
	int throttle = THROTTLE_THRESHOLD;

	uint8_t executeonly = 0;
	char * execute = NULL;
	pid_t child_pid, wpid;
	int status;

	struct timeval tv_last = (struct timeval){ 0 }, tv_now = (struct timeval){ 0 };
	unsigned int notification_count = 0;

	unsigned int version = 0, help = 0;

	program = argv[0];

	/* get command line options - part I
	 * just get -h (help), -e and -n (regex options) here */
	while ((i = getopt_long(argc, argv, optstring, options_long, NULL)) != -1) {
		switch (i) {
			case 'e':
				regex_flags |= REG_EXTENDED;
				break;
			case 'h':
				help++;
				break;
			case 'n':
				regex_flags |= REG_ICASE;
				break;
			case 'v':
				verbose++;
				break;
			case 'V':
				verbose++;
				version++;
				break;
		}
	}

	/* say hello */
	if (verbose > 0)
		printf("%s: %s v%s (compiled: " __DATE__ ", " __TIME__ ")\n", program, PROGNAME, VERSION);

	if (help > 0)
		fprintf(stderr, "usage: %s [-e] [-h] [-i ICON] [-m MATCH] [-o -m MATCH] [-a -m MATCH] [-n] [-r REGEX] [-t SECONDS] [-T THROTTLE] [-v[v]] [-V] [-x EXECUTE] [-X EXECUTE]\n", program);

	if (version > 0 || help > 0)
		return EXIT_SUCCESS;

	/* open journal */
	if ((rc = sd_journal_open(&journal, SD_JOURNAL_LOCAL_ONLY + SD_JOURNAL_SYSTEM)) < 0) {
		fprintf(stderr, "Failed to open journal: %s\n", strerror(-rc));
		goto out10;
	}

	/* seek to the end of the journal */
	if ((rc = sd_journal_seek_tail(journal)) < 0) {
		fprintf(stderr, "Failed to seek to the tail: %s\n", strerror(-rc));
		goto out20;
	}

	/* we are behind the last entry, so use previous one */
	if ((rc = sd_journal_previous(journal)) < 0) {
		fprintf(stderr, "Failed to iterate to previous entry: %s\n", strerror(-rc));
		goto out20;
	}

	/* reinitialize getopt() by resetting optind to 0 */
	optind = 0;

	/* get command line options - part II */
	while ((i = getopt_long(argc, argv, optstring, options_long, NULL)) != -1) {
		switch (i) {
			case 'a':
				if (verbose > 1)
					printf("Adding logical AND to match...\n");

				if ((rc = sd_journal_add_conjunction(journal)) < 0) {
					fprintf(stderr, "Failed to add logical AND to match.\n");
					goto out30;
				}

				break;
			case 'i':
				icon = optarg;
				break;
			case 'm':
				if (verbose > 1)
					printf("Adding match '%s'...\n", optarg);

				if ((rc = sd_journal_add_match(journal, optarg, 0)) < 0) {
					fprintf(stderr, "Failed to add match '%s': %s\n", optarg, strerror(-rc));
					goto out30;
				}

				break;
			case 'o':
				if (verbose > 1)
					printf("Adding logical OR to match...\n");

				if ((rc = sd_journal_add_disjunction(journal)) < 0) {
					fprintf(stderr, "Failed to add logical OR to match.\n");
					goto out30;
				}

				break;
			case 'r':
				if (verbose > 1)
					printf("Adding regular expression '%s'...\n", optarg);

				if (have_regex > 0) {
					fprintf(stderr, "Only one regex allowed!\n");
					rc = EXIT_FAILURE;
					goto out30;
				}

				if ((rc = regcomp(&regex, optarg, regex_flags)) != 0) {
					fprintf(stderr, "Could not compile regex\n");
					goto out20;
				}
				have_regex++;

				break;
			case 't':
				timeout = atoi(optarg);
				if (verbose > 1)
					printf("Notifications will be displayed for %d seconds.\n", timeout);
				timeout *= 1000;

				break;
			case 'T':
				throttle = atoi(optarg);
				if (verbose > 1)
					printf("Notifications will be throttled starting with the %dth.\n", throttle);
				timeout *= 1000;

				break;
			case 'X':
				executeonly = 1;
			case 'x':
				execute = optarg;
				if (verbose > 1)
					printf("Command used for execution: %s\n", execute);

				break;
		}
	}

	if (executeonly == 0) {
		if (notify_init(program) == FALSE) {
			fprintf(stderr, "Failed to initialize notify.\n");
			rc = EXIT_FAILURE;
			goto out30;
		}
	}

	/* main loop */
	while (1) {
		if ((rc = sd_journal_next(journal)) < 0) {
			fprintf(stderr, "Failed to iterate to next entry: %s\n", strerror(-rc));
			goto out40;
		} else if (rc == 0) {
			old_entry = 0;
			if (verbose > 2)
				printf("Waiting...\n");
			if ((rc = sd_journal_wait(journal, (uint64_t) -1)) < 0) {
				fprintf(stderr, "Failed to wait for changes: %s\n", strerror(-rc));
				goto out40;
			}
			continue;
		}

		gettimeofday(&tv_now, NULL);

		if (tv_now.tv_sec != tv_last.tv_sec) {
			tv_last = tv_now;
			notification_count = 0;
		}

		/* Looks like there is a bug in libsystemd journal handling
		 * that jumps to wrong entries. Just skip old entries until we
		 * have a recent one. */
		if (old_entry > 0) {
			fprintf(stderr, "This is an old entry, ignoring.\n");
			continue;
		}

		/* get MESSAGE field */
		if ((rc = sd_journal_get_data(journal, "MESSAGE", &data, &length)) < 0) {
			fprintf(stderr, "Failed to read message field: %s\n", strerror(-rc));
			continue;
		}
		message = malloc(length - 8 + 1);
		memcpy(message, data + 8, length - 8);
		message[length - 8] = 0;

		/* get SYSLOG_IDENTIFIER field */
		if ((rc = sd_journal_get_data(journal, "SYSLOG_IDENTIFIER", &data, &length)) < 0) {
			fprintf(stderr, "Failed to read syslog identifier field: %s\n", strerror(-rc));
			continue;
		}
		identifier = malloc(length - 18 + 1);
		memcpy(identifier, data + 18, length - 18);
		identifier[length - 18] = 0;

		/* get PRIORITY field */
		if ((rc = sd_journal_get_data(journal, "PRIORITY", &data, &length)) < 0) {
			fprintf(stderr, "Failed to read syslog identifier field: %s\n", strerror(-rc));
			continue;
		}
		priority = atoi(data + 9);
		priorityname = priorities[priority];

		if (verbose > 2)
			printf("Received message from journal: %s\n", message);

		if (have_regex == 0 || regexec(&regex, message, 0, NULL, 0) == 0) {
			/* throttling */
			notification_count++;
			if (notification_count >= throttle) {
				if (verbose)
					fprintf(stderr, "This is the %uth notification, throttling!\n", notification_count);
				if (executeonly == 0 && notification_count == throttle) {
					if ((rc = notify(program, "Throttling notification! View your journal for complete log.", 0, "dialog-warning", timeout)) > 0) {
						fprintf(stderr, "Failed to show notification.\n");
					}
				}
				continue;
			}

			/* show notification */
			if (executeonly == 0) {
				for (i = 0; i < 3; i++) {
					if (verbose > 0)
						printf("Showing notification: %s: %s\n", identifier, message);

					if ((rc = notify(identifier, message, priority, icon, timeout)) == 0)
						break;

					fprintf(stderr, "Failed to show notification, reinitializing libnotify.\n");
					notify_uninit();
					usleep(500 * 1000);
					if (notify_init(program) == FALSE) {
						fprintf(stderr, "Failed to initialize notify.\n");
						rc = EXIT_FAILURE;
					}
				}
				if (rc != 0)
					goto out40;
			}

			/* execute command */
			if (execute) {
				if (verbose > 1)
					printf("Executing: %s -i %s -p %s -m %s\n",
						execute, identifier, priorityname, message);

				if ((child_pid = fork()) < 0) {
					fprintf(stderr, "fork() failed\n");
				} else if (child_pid == 0) { /* This is the child */
					rc = execlp(execute, execute, "-i", identifier,
						"-p", priorityname, "-m", message, NULL);
					/* execlp() should replace the process, so anything failed */
					fprintf(stderr, "Failed to execute '%s': %s\n", execute, strerror(-rc));
					goto out10;
				} else { /* This is the parent */
					do {
						if ((wpid = waitpid(child_pid, &status, WUNTRACED|WCONTINUED)) < 0) {
							perror("waitpid");
							goto out40;
						}

						if (WIFEXITED(status)) {
							if (WEXITSTATUS(status) > 0 || verbose > 1)
								printf("child exited, status %d\n", WEXITSTATUS(status));
						} else if (WIFSIGNALED(status)) {
							printf("child killed (signal %d)\n", WTERMSIG(status));
						} else if (WIFSTOPPED(status)) {
							printf("child stopped (signal %d)\n", WSTOPSIG(status));
						} else if (WIFCONTINUED(status)) {
							printf("child continued\n");
						} else {
							printf("Unexpected status (0x%x)\n", status);
						}
					} while (!WIFEXITED(status) && !WIFSIGNALED(status));
				}
			}
		}

		free(identifier);
		free(message);
	}

	rc = EXIT_SUCCESS;

out40:
	if (executeonly == 0)
		notify_uninit();

out30:
	if (have_regex > 0)
		regfree(&regex);

out20:
	sd_journal_close(journal);

out10:
	return rc;
}

// vim: set syntax=c:
