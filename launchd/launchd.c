#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <hfs/hfs_mount.h>
#include <sys/types.h>

#define HANG() while(1){}
#define HANG_LIMIT(x) { \
	for(int i=0;i<x;++i){} \
} \

void setup_console();
void exec_cmd(char * argv[], char *env[]);
int setup_disk();
void welcome_msg();
void memset_(void *b, int c, int len);
void fb_new_page();
void console_write_msg(int fd, const char *msg);
void console_print(const char *msg);
int mount_hfs(const char *partition, const char *mnt_point, unsigned int mnt_opts);

int main(int argc, const char *argv[], const char *env[]) {	
	HANG_LIMIT(20000000);
	
	setup_console();
	fb_new_page();
	welcome_msg();
	
	if(setup_disk() != 0) {
		console_write_msg(STDERR_FILENO, "*couldn't setup disk\n");
		HANG();
	}
	console_print("*disk set up\n");
	
	HANG();
	
	return 0;
}

void setup_console() {
	int console;
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	console = open("/dev/console", O_RDWR);
	
	dup2(console, STDIN_FILENO);
	dup2(console, STDOUT_FILENO);
	dup2(console, STDERR_FILENO);
}

void welcome_msg() {
	console_print("*******************\n");
	console_print("Welcome to Anthrax\n");
	console_print("*******************\n");
	console_print("\n\n");
}

int setup_disk() {
	struct stat st;
	int encrypted, fsck_exists;
	
	console_print("*waiting for flash...");
	while(stat("/dev/disk0", &st) != 0) {
		HANG_LIMIT(1000);
	}
	console_print("done\n");
	
	console_print("*checking for /sbin/fsck_hfs...");
	if(stat("/sbin/fsck_hfs", &st) != 0) {
		console_print("no\n");
		fsck_exists = 0;
	} else {
		console_print("yes\n");
		chmod("/sbin/fsck_hfs", 0755);
		fsck_exists = 1;
	}

	console_print("*mounting rootfs(RD, /mnt)...");
	if(mount_hfs("/dev/disk0s1", "/mnt", MNT_RDONLY) != 0) {
		console_write_msg(STDERR_FILENO, "failed\n");
		return -1;
	}
	console_print("done\n");
	
	console_print("*waiting for rdisk0s1...");
	while(stat("/dev/rdisk0s1", &st) != 0) {
		HANG_LIMIT(1000);
	}
	console_print("done\n");
	
	if(fsck_exists) {
		exec_cmd((char *[]){"/sbin/fsck_hfs", "-fy", "/dev/rdisk0s1", (char *)0}, NULL);
	}
	
	console_print("*checking for /encrypted...");
	if(stat("/encrypted", &st) == 0) {
		encrypted = 1;
		console_print("yes\n");
		
		console_print("*waiting for rdisk0s2s1...");
		while(stat("/dev/rdisk0s2s1", &st) != 0) {
			HANG_LIMIT(1000);
		}
		console_print("done\n");
		
		if(fsck_exists) {
			exec_cmd((char *[]){"/sbin/fsck_hfs", "-fy", "/dev/rdisk0s2s1", (char *)0}, NULL);
		}
	} else {
		encrypted = 0;
		console_print("no\n");
	
		console_print("*waiting for rdisk0s2...");
		while(stat("/dev/rdisk0s2", &st) != 0) {
			HANG_LIMIT(1000);
		}
		console_print("done\n");
		
		if(fsck_exists) {
			exec_cmd((char *[]){"/sbin/fsck_hfs", "-fy", "/dev/rdisk0s2", (char *)0}, NULL);
		}
	}
	
	console_print("*mounting rootfs(RDWR, /mnt)...");
	if(mount_hfs("/dev/disk0s1", "/mnt", MNT_ROOTFS | MNT_UPDATE) != 0) {
		console_write_msg(STDERR_FILENO, "failed\n");
		return -1;
	}
	console_print("done\n");
	
	console_print("*mounting devfs(/mnt/dev)...");
	if(mount("devfs", "/mnt/dev", 0, NULL) != 0) {
		console_write_msg(STDERR_FILENO, "failed\n");
		return -1;
	}
	console_print("done\n");
	
	if(encrypted) {
		console_print("*mounting disk0s2s1(/mnt/var)...");
		if(mount_hfs("/dev/disk0s2s1", "/mnt/private/var", 0) != 0) {
			console_write_msg(STDERR_FILENO, "failed\n");
			return -1;
		}
		console_print("done\n");
	} else {
		console_print("*mounting disk0s2(/mnt/var)...");
		if(mount_hfs("/dev/disk0s2", "/mnt/private/var", 0) != 0) {
			console_write_msg(STDERR_FILENO, "failed\n");
			return -1;
		}
		console_print("done\n");
	}
	
	return 0;
}

void console_write_msg(int fd, const char *msg) {
	if(fd < 0 || msg == NULL) return;
	
	while(*msg != '\0') {
		write(fd, msg, 1);
		msg++;
	}
}

void console_print(const char *msg) {
	console_write_msg(STDOUT_FILENO, msg);
}

void fb_new_page() {
	for(int i=0;i<100;++i) {
		console_print("\n");
	}
}

int mount_hfs(const char *partition, const char *mnt_point, unsigned int mnt_opts) {
	struct hfs_mount_args args;
	memset_(&args, '\0', sizeof(args));
	args.fspec = (char *)partition;
	
	if(mount("hfs", mnt_point, mnt_opts, &args) != 0) {
		return -1;
	}
	
	return 0;
}

void memset_(void *b, int c, int len) {
	int index;
	for(index = 0; index < len; ++index) {
		((char *)b)[index] = c;
	}
}

void exec_cmd(char * argv[], char *env[]) {
	if(vfork() != 0) {
		while(wait4(-1, NULL, WNOHANG, NULL) <= 0) {
			HANG_LIMIT(1000);
		}
	} else {
		execve(argv[0], argv, env);
	}
}
