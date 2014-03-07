#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/syslimits.h>

#include <pwd.h>
#include <grp.h>

void die(const char* what);

#define LOG(x) errlog(#x, x)

void errlog(const char* name, const char* x);
const char* uname(int uid);
const char* gname(int gid);
const char* id();

#if 0

static int current_user_is_member_of_group(gid_t gid) {
	int gids_count = checked(getgroups(0, 0));
	gid_t gids[gids_count + 1];
	gids[0] = getegid();
	checked(getgroups(gids_count, gids+1));
	gids_count++;

	for(int i = 0; i < gids_count; ++i) {
		if(gid == gids[i]) return 1;
	}
	return 0;
}

#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

static const char* ExecutablePath() {
	static char path[PATH_MAX] = { 0 };

	if(path[0]) return path;

	unsigned size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		return path;

	die("_NSGetExecutablePath");
	return 0;
}

/*
 * return the basename of a path
 */
static const char* Basename(const char* path) {
   const char* basename = strrchr(path, '/');
   return basename ? basename + 1 : path;
}

static void usage(const char* arg0) {
	fprintf(stderr, "%s [ --pidfile pidfile ] command\n", arg0);
	exit(2);
}

/*
 * pidfile: the pidfile will be cleaned up upon exit.
 */
static const char* pidfile = 0;
static char keep_pidfile = 0;
static void cleanup_pidfile_and_exit(signum) {
	if(pidfile && !keep_pidfile)
		unlink(pidfile);
	exit(signum);
}

int main(int argc, char** argv) 
{
	const char* arg0 = *argv;
	
	/*
	 * Parse arguments.
	 */
	while(*++argv) {
		if(!strcmp(*argv, "--pidfile")) {
			pidfile = *++argv;
			if(!pidfile) usage(arg0);
			continue;
		}

		if(!strcmp(*argv, "--keep-pidfile")) {
			/*
			 * The --keep-pidfile option is for testing only. It prevents the pidfile 
			 * from being removed on exit.
			 */
			keep_pidfile = 1;
			continue;
		}

		break;
	}
	
	if(!*argv) usage(arg0);
	
	/*
	 * which user and group is requested? The target user is defined 
	 * by the basename of the binary.
	 */
	const char* basename = Basename(ExecutablePath());
	
	struct passwd* requested_user = getpwnam(basename);
	if(!requested_user) {
		fprintf(stderr, "No such user: '%s'\n", basename);
		exit(1);
	}

	/*
	 * Run requested command.
	 */
	void execv_as_user(struct passwd* requested_user, const char* arg0, char** argv);

	if(!pidfile) {
		execv_as_user(requested_user, *argv, argv);
		return 1;	/* execv_as_user does not return. */
	}

	/*
	 * open pidfile. We do this before we are forking because we want run-as to fail
	 * when the pidfile is in a unaccessible location.
	 */
	FILE* pidout = fopen(pidfile, "w");
	if(!pidout)
		die(pidfile);

	/*
	 * fork. 
	 */
	pid_t child_pid = fork();
	if(child_pid < 0)
		die("fork");

	if (child_pid == 0) {
    	fclose(pidout);
		execv_as_user(requested_user, *argv, argv);
	}

	/*
	 * write pidfile.
	 *
	 * [todo] The pidfile is created by the executing user. Is this correct?
	 */
	fprintf(stderr, "pid (in %s): %d\n", pidfile, child_pid);
	fprintf(pidout, "%d", child_pid);
	fclose(pidout);

	/*
	 * register signal handlers to cleanup pidfile.
	 */
	signal(SIGINT, cleanup_pidfile_and_exit);
	signal(SIGTERM, cleanup_pidfile_and_exit);

	/* 
	 * wait for child to exit, and store child's exit status 
	 */
	int status;
	wait(&status); 
	int exitstatus = WEXITSTATUS(status);

	if(exitstatus != 0)
		fprintf(stderr, "%s exited with error %d.\n", arg0, exitstatus);

	cleanup_pidfile_and_exit(exitstatus);
	return 0;
}

void execv_as_user(struct passwd* requested_user, const char* arg0, char** argv) {
	gid_t requested_gid = requested_user->pw_gid;
	uid_t requested_uid = requested_user->pw_uid;
	
	/*
	 * Switch the real user to the effective user
	 */
	if(initgroups(requested_user->pw_name, requested_gid)) die("initgroups");

	if(setregid(requested_gid, requested_gid)) die("setregid");
	if(setreuid(requested_uid, requested_uid)) die("setreuid");

	/*
	 * reset environment for new user.
	 */
	setenv("TMPDIR", "/tmp", 1);
	
	execv(arg0, argv);
	die(arg0);
}

/* --- Helpers -------------------------------------------------------------- */

const char* uname(int uid) {
	struct passwd * entry = getpwuid(uid);
	if(!entry)
		die("getpwuid");

	return entry->pw_name;
}

const char* gname(int gid) {
	struct group * entry = getgrgid(gid);
	if(!entry)
		die("getuid");

	return entry->gr_name;
}

const char* id() {
	static char buf[PATH_MAX];

	snprintf(buf, sizeof(buf), 
		"uid/euid: %s/%s(%d/%d), gid/egid: %s/%s(%d/%d)",
		uname(getuid()),	uname(geteuid()), 
		getuid(),			geteuid(),
		gname(getgid()),	gname(getegid()), 
		getgid(),			getegid()
	);

	return buf;
}

void errlog(const char* name, const char* x) {
	fprintf(stderr, "%s: %s\n", name, x ? x : "NULL");
}

void die(const char* what) {
	perror(what);
	exit(1);
}
