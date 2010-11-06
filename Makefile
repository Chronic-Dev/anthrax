###
# GreenPois0n Anthrax - Makefile
# Copyright (C) 2010 Chronic-Dev Team
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
###
 
RM = rm -rf
CP = cp
SYNC = sync
HDIUTIL = hdiutil
XPWNTOOL = /usr/local/bin/xpwntool

LAUNCHD_TARGET = src/launchd
RAMDISK_TARGET = ramdisk.dmg
ENCRYPT_TARGET = ramdisk.img3

TARGETS = $(LAUNCHD_TARGET) $(RAMDISK_TARGET) $(ENCRYPT_TARGET)

RAMDISK_LABEL=pois0ndisk
RAMDISK_VOLUME=/Volumes/$(RAMDISK_LABEL)

#******** Please fill these in for yourself *********
#RAMDISK_IV=6800a52c8761b30b001327ea9b918653 
#RAMDISK_KEY=53e72495ea0d5dba53a1eae7529d6a95e8eb1d5d9185298c0f3323bc2c27f26f 
#RAMDISK_TEMPLATE=018-7074-092.dmg

all: $(TARGETS)

clean:
	$(MAKE) -C src clean
	$(RM) ramdisk.img3 ramdisk.dmg

src/launchd:
	$(MAKE) -C src launchd

ramdisk.dmg: src/launchd
	$(CP) template.dmg ramdisk.dmg
	$(HDIUTIL) attach ramdisk.dmg
	$(CP) src/launchd $(RAMDISK_VOLUME)/sbin/
	$(CP) -R files/* $(RAMDISK_VOLUME)/files/
	$(SYNC)
	$(HDIUTIL) detach $(RAMDISK_VOLUME)

ramdisk.img3: ramdisk.dmg
	$(XPWNTOOL) ramdisk.dmg ramdisk.dmg -k $(RAMDISK_KEY) -iv $(RAMDISK_IV) -t $(RAMDISK_TEMPLATE)
	$(XPWNTOOL) ramdisk.dmg ramdisk.img3 -k $(RAMDISK_KEY) -iv $(RAMDISK_IV) -decrypt
