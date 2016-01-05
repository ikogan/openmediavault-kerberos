#!/bin/bash
set -e

. /etc/default/openmediavault
. /usr/share/openmediavault/scripts/helper-functions

echo "Building plugin..."

cd /source
fakeroot debian/rules clean binary

dpkg -i /openmediavault-kerberos*.deb || true
apt-get -f install -y
/etc/init.d/openmediavault-engined stop

omv-engined -d -f
