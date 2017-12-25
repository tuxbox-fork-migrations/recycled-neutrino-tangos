#!/bin/sh

HOST=$(hostname)
DATE=$(date +%Y-%m-%d_%H-%M-%S)
USRF="/var/tuxbox/config/tobackup.conf"
if [ "$2" == "" ]; then
BAKF="$1/settings_${HOST}_${DATE}.tar"
else
BAKF="$1/$2"
Z=z
fi


if [ -e "${USRF}" ]; then
# read user-files from $USRF
	TOBACKUP="${USRF}"
	while read i
		do [ "${i:0:1}" = "#" ] || TOBACKUP="$TOBACKUP ${i%%#*}"
		done < $USRF

else
	TOBACKUP="/var/tuxbox/config/"
fi

# check existence
RES=""
for i in $TOBACKUP
	do [ -e "$i" ] && RES="$RES $i"
	done

TOBACKUP=$(echo $RES)

echo Backup to ${BAKF}

tar -c${Z}f "${BAKF}" $TOBACKUP 2>&1 >/dev/null
