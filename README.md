journal-notify
==============

**Notify about journal log entries**

This runs in background and produces notifications whenever systemd journal
logs an entry. Notifications look like this:

![Notification](screenshot.png)

Requirements
------------

To compile and run `journal-notify` you need:

* [libnotify](http://library.gnome.org/devel/notification-spec/)
* [libsystemd](http://www.freedesktop.org/wiki/Software/systemd/)
* [markdown](http://daringfireball.net/projects/markdown/) (HTML
 documentation)
* `gnome-icon-theme` (or whatever includes the icons you want to use)

To use `journal-notify` you probably want `systemd-journald`.

Some systems may require additional development packages for the libraries.
Look for `libnotify-devel`, `libsystemd-devel` or similar.

Build and install
-----------------

Building and installing is very easy. Just run:

> make

followed by:

> make install

This will place an executable at `/usr/bin/journal-notify`,
documentation can be found in `/usr/share/doc/journal-notify/`.

Usage
-----

To use `journal-notify` you need access to the journal log files. Make sure
you get the expexted log entries by running `journalctl`. Possibly you have
to add yourself to group `systemd-journal` or similar.

Just running `journal-notify` without parameter will show *all* log entries.
Be warned: This can flood your disktop with notifications.

`journal-notify` accepts some options:

* *-a*: combine matches with a logical AND
* *-e*: use extended regular expressions
* *-h*: show help
* *-i ICON*: icon to use
* *-m MATCH*: This can be specified more than once. The option accepts matches
 to systemd journal fields. (see `man 7 systemd.journal-fields`)
* *-n*: no case sensitive regular expressions
* *-o*: combine matches with a logical OR
* *-r REGEX*: This add a regular expression match for the message field.
* *-t SECONDS*: seconds to show the notification, 0 is forever
* *-v* verbose output, can be specified multiple times

The screenshot shown above resulted from this command:

> journal-notify -m SYSLOG_IDENTIFIER=sshd -e -r "^(error:|Accepted)"
-i security-high

Additionally example desktop files are installed to
`/usr/share/journal-notify/examples`. You should copy them to
`~/.config/autostart/` to enable autostart or create your own desktop files
there.

### Upstream

URL: [GitHub.com](https://github.com/eworm-de/journal-notify)  
Mirror: [eworm.de](http://git.eworm.de/cgit.cgi/journal-notify/)
