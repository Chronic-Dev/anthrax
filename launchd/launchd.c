#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <hfs/hfs_mount.h>

void console_print(const char *msg);
void mount_hfs(const char *partition, const char *mnt_point, unsigned int mnt_opts);
//void sleep(int period);

int main(int argc, const char *argv[], const char *env[]) {
	struct stat check;
	int console;
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	console = open("/dev/console", O_RDWR);
	
	dup2(console, STDIN_FILENO);
	dup2(console, STDOUT_FILENO);
	dup2(console, STDERR_FILENO);
	
	while(stat("/dev/disk0", &check) != 0) {
		// *** 
	}

	console_print("Disk mounted\n");
	
	mount_hfs("/dev/disk0s1", "/mnt", MNT_ROOTFS | MNT_RDONLY);
	
	while(stat("/dev/rdisk0s1", &check) != 0) {
		// ***
	}
	console_print("Filesystem mounted, fscking\n");
	
	//exec_cmd((const char *[]){ "/mnt/sbin/fsck_hfs", "-fy", "/dev/disk0s1" , (const char *)NULL }, env);
	
	console_print("\n\n\nWoot\n");
	while(1) {
		
	}
	
	return 0;
}

void console_print(const char *msg) {
	// *** Investigate: why are we writing 1 byte at a time?
	while(*msg != '\0') {
		write(STDOUT_FILENO, msg, 1);
		msg++;
	}
}

void mount_hfs(const char *partition, const char *mnt_point, unsigned int mnt_opts) {
	struct hfs_mount_args args;
	args.fspec = (char *)partition;
	
	if(mount("hfs", mnt_point, mnt_opts, &args) != 0) {
		console_print("Error mounting disk...\n");
		reboot(0); // reboot, we failed.
	}
}

void exec_cmd(char * argv[], char *env[]) {
	if(vfork() != 0) {
		while(wait4(-1, NULL, WNOHANG, NULL) <= 0) {
			
		}
	} else {
		execve(argv[0], argv, env);
	}
}

//
//void sleep(int period) {
//	
//}