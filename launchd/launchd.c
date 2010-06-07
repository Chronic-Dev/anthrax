#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <hfs/hfs_mount.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

void console_print(const char *text);
void my_sleep(int seconds);
void memset_(void *b, int c, int len);
void mount_hfs_partition(const char *partition, const char *mnt_point, unsigned int mnt_opts);

int main() {
	close(0); // close stdin
	close(1); // close stdout
	close(2); // close stderr
	
	int console = open("/dev/console", O_WRONLY); // open /dev/console for write only
	dup2(console, 0); // direct stdin->/dev/console
	dup2(console, 1); // direct stdout->/dev/console
	dup2(console, 2); // direct stderr->/dev/console
	
	struct stat check;
	while(stat("/dev/disk0", &check) != 0) { // wait for the drive to mount(NAND)
		console_print("Waiting for NAND to appear...\n");
		my_sleep(1);
	}
	
	// rootfs mount
	mount_hfs_partition("/dev/disk0s1", "/mnt", MNT_ROOTFS | MNT_RDONLY); // mount root filesystem as readonly @ /mnt
	
	while(stat("/dev/disk0s1", &check)) { // wait for root filesystem to be mounted
		console_print("Waiting for filesystem to mount...\n");
		my_sleep(1);
	}
	
	console_print("Well, we did it guys\n");
	
	while(1) {
		my_sleep(1);
	}
	
	return 0;
}

void console_print(const char *text) {
	const char *copy = text; // create a matching pointer 
	while((*copy) != 0) { // as long as this pointer's position is NOT on the NULL-term byte...
		write(0x1, copy, 0x1); // write current byte of string to stdout
		copy++; // increment pointer position
	}
}

// Quick implementation of memset() (since we don't have libc)
void memset_(void *b, int c, int len) {
	int index;
	for(index = 0; index < len; ++index) {
		((char *)b)[index] = c;
	}
}

void mount_hfs_partition(const char *partition, const char *mnt_point, unsigned int mnt_opts) {
	struct hfs_mount_args args;
	args.fspec = (char *)partition;
	
	if(mount("hfs", mnt_point, mnt_opts, &args) != 0) {
		console_print("Error mounting disk...\n");
		reboot(0); // reboot, we failed.
	}
}

void my_sleep(int seconds) {
	static mach_timebase_info_data_t sTimebaseInfo = {0, 0};
	
	if(sTimebaseInfo.denom == 0) {
		mach_timebase_info(&sTimebaseInfo);
	}
	
	mach_wait_until(mach_absolute_time() + (((unsigned int)seconds) * 6000000));
}