PLATFORM=/Developer/Platforms/iPhoneOS.platform
SDKVER=4.0
SDK=$(PLATFORM)/Developer/SDKs/iPhoneOS$(SDKVER).sdk
CC=$(PLATFORM)/Developer/usr/bin/gcc-4.2
LD=$(PLATFORM)/Developer/usr/bin/ld
AS=$(PLATFORM)/Developer/usr/bin/as
BIN2C=../tools/bin2c
STRIP=$(PLATFORM)/Developer/usr/bin/strip

#******** Please fill these in for yourself *********
#RAMDISK_IV=6800a52c8761b30b001327ea9b918653 
#RAMDISK_KEY=53e72495ea0d5dba53a1eae7529d6a95e8eb1d5d9185298c0f3323bc2c27f26f 
#RAMDISK_TEMPLATE=018-7074-092.dmg

LAUNCHD_SOURCES=src/launchd.c src/utils.c src/syscalls.S

RAMDISK_LABEL=pois0ndisk
RAMDISK_VOLUME=/Volumes/$(RAMDISK_LABEL)

LAUNCHD_CFLAGS=-arch armv6 -isysroot=$(SDK) -I./include -I$(SDK)/usr/include -static -nostartfiles -nodefaultlibs -nostdlib -Wl,-e,_main
LAUNCHD_LDFLAGS=

UNAME := $(shell uname -s)
PWD = $(shell pwd)

ifeq ($(UNAME),Darwin)
all: launchd ramdisk encrypt stage

clean:
	rm -rf ramdisk.img3 ramdisk.dmg
	rm -rf src/*.o src/launchd
	
else
all: stage
clean:
endif

launchd:
	$(CC) -o src/launchd $(LAUNCHD_SOURCES) $(LAUNCHD_CFLAGS) $(LAUNCHD_LDFLAGS)
	strip src/launchd
	ldid -S src/launchd

ramdisk: launchd
	cp template.dmg ramdisk.dmg
	hdiutil attach ramdisk.dmg
	cp src/launchd $(RAMDISK_VOLUME)/sbin/
	cp -R files/* $(RAMDISK_VOLUME)/files/
	sync
	hdiutil detach $(RAMDISK_VOLUME)

encrypt:
	xpwntool ramdisk.dmg ramdisk.img3 -k $(RAMDISK_KEY) -iv $(RAMDISK_IV) -t $(RAMDISK_TEMPLATE)
	xpwntool ramdisk.img3 ramdisk.dmg -k $(RAMDISK_KEY) -iv $(RAMDISK_IV) -decrypt
	
stage:
	$(BIN2C) ramdisk.dmg ../injector/resources/ramdisk.h ramdisk
