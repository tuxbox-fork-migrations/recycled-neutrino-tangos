#!/bin/sh

cd /var/tuxbox/config
if [ -e neutrino.conf ]; then
	# webradio_usr.xml was moved
	sed -i "s|/var/tuxbox/config/webradio_usr.xml|/var/tuxbox/webradio/webradio_usr.xml|" neutrino.conf
	mkdir -p /var/tuxbox/webradio/
	if [ -e /var/tuxbox/config/webradio_usr.xml ]; then
		mv /var/tuxbox/config/webradio_usr.xml /var/tuxbox/webradio/
	fi

	# webtv_usr.xml was moved
	sed -i "s|/var/tuxbox/config/webtv_usr.xml|/var/tuxbox/webtv/webtv_usr.xml|" neutrino.conf
	mkdir -p /var/tuxbox/webtv/
	if [ -e /var/tuxbox/config/webtv_usr.xml ]; then
		mv /var/tuxbox/config/webtv_usr.xml /var/tuxbox/webtv/
	fi

	sort neutrino.conf > neutrino.sort
	mv neutrino.sort neutrino.conf
fi

controlscripts="\
	audioplayer.start \
	audioplayer.end \
	deepstandby.on \
	deepstandby.off \
	inactivity.on \
	movieplayer.start \
	movieplayer.end \
	neutrino.start \
	pictureviewer.start \
	pictureviewer.end \
	recording.timer \
	recording.start \
	recording.end \
	scan.start \
	scan.stop \
	standby.on \
	standby.off \
	volume.down \
	volume.up \
	mute.off \
	mute.on
"

mkdir -p /var/tuxbox/control/
for controlscript in $controlscripts; do
	if [ -e /var/tuxbox/config/$controlscript ]; then
		mv /var/tuxbox/config/$controlscript /var/tuxbox/control/
	fi
done

internalplugins="`ls -1 /usr/share/tuxbox/neutrino/plugins`"
for internalplugin in $internalplugins; do
	if [ -e /usr/share/tuxbox/neutrino/plugins/$internalplugin ] && [ -e /var/tuxbox/plugins/$internalplugin ]; then
		rm /var/tuxbox/plugins/$internalplugin
	fi
done

# these control scripts hasn't counterparts in /var
rm -f flash.start

#cleanup
rm /usr/share/tuxbox/neutrino/control/migration.sh
