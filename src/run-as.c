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

int main(int argc, char** argv) 
{
	const char* pidfile;
	const char* arg0 = *argv;
	
	/*
	 * Parse arguments.
	 */
	while(*++argv) {
		if(!strcmp(*argv, "--pidfile") || !strcmp(*argv, "-pidfile")) {
			pidfile = *++argv;
			if(!pidfile) usage(arg0);
			++argv;
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

	gid_t requested_gid = requested_user->pw_gid;
	uid_t requested_uid = requested_user->pw_uid;
	
	/*
	 * Switch the real user to the effective user
	 */
	// fprintf(stderr, "Switching away from %s\n", id());

	if(initgroups(requested_user->pw_name, requested_gid)) die("initgroups");

	if(setregid(requested_gid, requested_gid)) die("setregid");
	if(setreuid(requested_uid, requested_uid)) die("setreuid");

	/*
	 * reset environment for new user.
	 */
	setenv("TMPDIR", "/tmp", 1);
	
	execv(*argv, argv);
	die(*argv);

	return 0;
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
