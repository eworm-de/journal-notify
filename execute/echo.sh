#!/bin/sh
#
# (C) 2014-2025 by Christian Hesse <mail@eworm.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

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

echo "IDENTIFIER: ${IDENTIFIER}"
echo "PRIORITY:   ${PRIORITY}"
echo "MESSAGE:    ${MESSAGE}"

exit 0

# vim: set syntax=sh:
