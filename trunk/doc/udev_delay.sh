#!/bin/sh

# Use this script in a udev RUN+= rule

# It backgrounds and returns immediately, then the child sleeps for a bit

# The purpose is that you're [almost] garaunteeing that if you boot
#    with a gps and an OBDII dongle attached, that gpsd will be lanched
#    and handled before obdgpslogger is launched

# Here's the udev rule, put it in /etc/udev/rules.d/40-obd_delay.rules
# SUBSYSTEM=="tty", ACTION=="add", SYSFS{idVendor}=="0403", SYSFS{idProduct}=="6001", ENV{OBD_DEVICE}="/dev/%k", RUN+="/usr/local/bin/udev_delay.sh"


# You need to configure a mountpoint for your removable media in
#   fstab, then set MEDIA_MOUNTPOINT to be the mountpoint.
MEDIA_MOUNTPOINT=/media/usb

PATH="$MEDIA_MOUNTPOINT/bin":$PATH
export PATH

if [ ! -n "$OBD_DEVICE" ]
then
	echo "No OBD_DEVICE env var set. Exiting"
	exit 1
fi

if [ ! -n "$1" ]
then
	echo "Executing new udev_delay"
	("$0" cookies &)&
	exit 0
fi

sleep 10

echo "Child woke up"

OBD_CONFIGFILE="$MEDIA_MOUNTPOINT/obdgpslogger.conf"
export OBD_CONFIGFILE

echo "OBD_DEVICE: $OBD_DEVICE"

echo "Mounting $MEDIA_MOUNTPOINT"
mount "$MEDIA_MOUNTPOINT" || exit 1

echo "Launching logger"
obdgpslogger -s "$OBD_DEVICE" -d "$MEDIA_MOUNTPOINT/carlog.db"
echo "Logger exited. Umounting media"

umount "$MEDIA_MOUNTPOINT" || echo "WARNING. Couldn't unmount $MEDIA_MOUNTPOINT"
echo "Done"

