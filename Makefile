# journal-notify - Notify about journal log entries

# commands
CC	:= gcc
MD	:= markdown
INSTALL	:= install
CP	:= cp
RM	:= rm

# flags
CFLAGS	+= -std=c11 -O2 -fPIC -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs libsystemd)
CFLAGS	+= $(shell pkg-config --cflags --libs libnotify)
LDFLAGS	+= -Wl,-z,now -Wl,-z,relro -pie

# this is just a fallback in case you do not use git but downloaded
# a release tarball...
VERSION := 0.1.2

all: journal-notify README.html

journal-notify: journal-notify.c journal-notify.h config.h version.h
	$(CC) journal-notify.c $(CFLAGS) $(LDFLAGS) -o journal-notify

config.h:
	$(CP) config.def.h config.h

version.h: $(wildcard .git/HEAD .git/index .git/refs/tags/*) Makefile
	printf "#ifndef VERSION\n#define VERSION \"%s\"\n#endif\n" $(shell git describe --long 2>/dev/null || echo ${VERSION}) > $@

README.html: README.md
	$(MD) README.md > README.html

install: install-bin install-doc

install-bin: journal-notify
	$(INSTALL) -D -m0755 journal-notify $(DESTDIR)/usr/bin/journal-notify
	$(INSTALL) -D -m0644 desktop/journal-notify-crit-alert-emerg.desktop $(DESTDIR)/usr/share/journal-notify/desktop/journal-notify-crit-alert-emerg.desktop
	$(INSTALL) -D -m0644 desktop/journal-notify-dhcpd.desktop $(DESTDIR)/usr/share/journal-notify/desktop/journal-notify-dhcpd.desktop
	$(INSTALL) -D -m0644 desktop/journal-notify-sshd.desktop $(DESTDIR)/usr/share/journal-notify/desktop/journal-notify-sshd.desktop
	$(INSTALL) -D -m0755 execute/echo.sh $(DESTDIR)/usr/share/journal-notify/execute/echo.sh
	$(INSTALL) -D -m0755 execute/mail.sh $(DESTDIR)/usr/share/journal-notify/execute/mail.sh
	$(INSTALL) -D -m0644 systemd/journal-notify-sshd.service $(DESTDIR)/usr/share/journal-notify/systemd/journal-notify-sshd.service

install-doc: README.html
	$(INSTALL) -D -m0644 README.md $(DESTDIR)/usr/share/doc/journal-notify/README.md
	$(INSTALL) -D -m0644 README.html $(DESTDIR)/usr/share/doc/journal-notify/README.html
	$(INSTALL) -D -m0644 screenshots/notification.png $(DESTDIR)/usr/share/doc/journal-notify/screenshots/notification.png
	$(INSTALL) -D -m0644 screenshots/throttling.png $(DESTDIR)/usr/share/doc/journal-notify/screenshots/throttling.png

clean:
	$(RM) -f *.o *~ README.html journal-notify version.h

distclean:
	$(RM) -f *.o *~ README.html journal-notify version.h config.h

release:
	git archive --format=tar.xz --prefix=journal-notify-$(VERSION)/ $(VERSION) > journal-notify-$(VERSION).tar.xz
	gpg --armor --detach-sign --comment journal-notify-$(VERSION).tar.xz journal-notify-$(VERSION).tar.xz
	git notes --ref=refs/notes/signatures/tar add -C $$(git archive --format=tar --prefix=journal-notify-$(VERSION)/ $(VERSION) | gpg --armor --detach-sign --comment journal-notify-$(VERSION).tar | git hash-object -w --stdin) $(VERSION)
