#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/syslimits.h>

#include <pwd.h>
#include <grp.h>

void die(const char* what);

#ifdef __APPLE__
#include <mach-o/dyld.h>

static const char* ExecutablePath() {
	static char path[PATH_MAX] = { 0 };

	if(path[0]) return path;

	unsigned size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		return path;

	die("_NSGetExecutablePath");
	return 0;
}

#else

static const char* ExecutablePath() {
	static char path[PATH_MAX] = { 0 };

	if(path[0]) return path;

	if(readlink("/proc/self/exe", path, sizeof(path)) > 0)
		return path;

	die("/proc/self/exe");
}

#endif

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
	 * reset environment for new user. Set HOME, USER, USERNAME.
	 */
	setenv("TMPDIR", "/tmp", 1);

	const char* pw_name = requested_user->pw_name;
	if(!*pw_name) pw_name = 0;
	if(pw_name) {
		setenv("USER", pw_name, 1);
		setenv("USERNAME", pw_name, 1);
	}

	const char* homedir = requested_user->pw_dir;
	if(!*homedir) homedir = 0;

	if(homedir)
		setenv("HOME", homedir, 1);

	/*
	 * (re)set ruby environment.
	 *
	 * The following code should make sure that the new user will have gems installed
	 * in a location writable by this user.
	 * 
	 * Note that vstrcat leaks memory. This is not a problem because this process
	 * is exited pretty soon anyways.
	 */

	char* vstrcat(const char *first, ...);

	if(homedir) {
		setenv("GEM_HOME", vstrcat(homedir, "/var/gems", 0), 1);
		setenv("GEM_PATH", vstrcat(homedir, "/var/gems", 0), 1);
		setenv("PATH",     vstrcat(homedir, "/var/gems/bin:", getenv("PATH"), 0), 1);
	}

	unsetenv("RUBYLIB");
	unsetenv("RUBYOPT");
	unsetenv("BUNDLE_GEMFILE");
	unsetenv("BUNDLE_BIN_PATH");
	unsetenv("_ORIGINAL_GEM_PATH"); 

	/*
	 * (re)set ruby environment.
	 */
	
	if(homedir) {
		if(chdir(homedir)) {
			fprintf(stderr, "Cannot chdir into %s's homedir.\n", pw_name);
			perror(homedir);
		}
	}
	
	execv(arg0, argv);
	die(arg0);
}

/* --- Helpers -------------------------------------------------------------- */

void die(const char* what) {
	perror(what);
	exit(1);
}

/*
 * from http://c-faq.com/~scs/cgi-bin/faqcat.cgi?sec=varargs#varargs1
 */

#include <stdarg.h>

char* vstrcat(const char *first, ...) {
	size_t len;
	char *retbuf;
	va_list argp;
	char *p;

	if(first == NULL)
		return NULL;

	len = strlen(first);

	va_start(argp, first);

	while((p = va_arg(argp, char *)) != NULL) 
        len += strlen(p);

	va_end(argp);

	retbuf = malloc(len + 1);	/* +1 for trailing \0 */

	if(retbuf == NULL)
		return NULL;		/* error */

	(void)strcpy(retbuf, first);

	va_start(argp, first);		/* restart; 2nd scan */

	while((p = va_arg(argp, char *)) != NULL) {
		(void)strcat(retbuf, p);
	}

	va_end(argp);

	return retbuf;
}
