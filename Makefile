# journal-notify - Notify about journal log entries

CC	:= gcc
MD	:= markdown
INSTALL	:= install
CP	:= cp
RM	:= rm
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs libsystemd)
CFLAGS	+= $(shell pkg-config --cflags --libs libnotify)
# this is just a fallback in case you do not use git but downloaded
# a release tarball...
VERSION := 0.0.1

all: journal-notify README.html

journal-notify: journal-notify.c
	$(CC) $(CFLAGS) -o journal-notify journal-notify.c

README.html: README.md
	$(MD) README.md > README.html

install: install-bin install-doc

install-bin: journal-notify
	$(INSTALL) -D -m0755 journal-notify $(DESTDIR)/usr/bin/journal-notify
	$(INSTALL) -D -m0755 examples/journal-notify-dhcpd.desktop $(DESTDIR)/usr/share/journal-notify/examples/journal-notify-dhcpd.desktop
	$(INSTALL) -D -m0755 examples/journal-notify-sshd.desktop $(DESTDIR)/usr/share/journal-notify/examples/journal-notify-sshd.desktop

install-doc: README.html
	$(INSTALL) -D -m0644 README.md $(DESTDIR)/usr/share/doc/journal-notify/README.md
	$(INSTALL) -D -m0644 README.html $(DESTDIR)/usr/share/doc/journal-notify/README.html
	$(INSTALL) -D -m0644 screenshot.png $(DESTDIR)/usr/share/doc/journal-notify/screenshot.png

clean:
	rm -f *.o *~ README.html journal-notify

release:
	git archive --format=tar.xz --prefix=journal-notify-$(VERSION)/ $(VERSION) > journal-notify-$(VERSION).tar.xz
	gpg -ab journal-notify-$(VERSION).tar.xz
