#!/bin/sh
#
# (C) 2015 by Christian Hesse <mail@eworm.de>
#
# This software may be used and distributed according to the terms
# of the GNU General Public License, incorporated herein by reference.

while getopts "i:p:m:" opt; do
	case ${opt} in
		i)
			IDENTIFIER="${OPTARG}"
			;;
		p)
			PRIORITY="${OPTARG}"
			;;
		m)
			MESSAGE="${OPTARG}"
			;;
	esac
done

# WARNING: Do NOT use this unmodified!
# Please replace mail address with your own.

mail -s "[${PRIORITY}] ${IDENTIFIER}" mail@example.com <<< "${MESSAGE}"

exit 0

# vim: set syntax=sh:
