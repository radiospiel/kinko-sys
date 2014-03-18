/*
 * run something as some kinko account.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#ifdef __APPLE__
#include <sys/syslimits.h>
#endif

#ifdef __linux__
#include <linux/limits.h>
#include <sys/wait.h>
#endif

#include <sys/stat.h>

#include <pwd.h>
#include <grp.h>

#if !defined(STRICT_MODE)
#define STRICT_MODE 1
#endif

/*
 * Note that some functions here, namely vstrcat and Realpath, leak memory.
 * This is intentional: this process will terminate shortly anyways, there
 * is no point in establishing a convoluted memory management thingy here.
 */

void die(const char* what) __attribute__((noreturn)) ;

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

	return 0;
}

#endif

/*
 * return the basename of a path
 */
static const char* Basename(const char* path) {
   const char* basename = strrchr(path, '/');
   return basename ? basename + 1 : path;
}

static char *Realpath(const char *pathname) {
    char absname[PATH_MAX+1];
	char* r = realpath(pathname, absname);
	if(!r) die(pathname);
	return strdup(r);
};

static char *Cwd() {
    char buf[PATH_MAX+1];
	char* cwd = getcwd(buf, sizeof(buf));
	if(!cwd) die("getcwd");
	return Realpath(cwd);
};

static void usage(const char* arg0) {
	fprintf(stderr, "%s [ --pidfile pidfile ] command\n", arg0);
#if !STRICT_MODE
	fprintf(stderr, "Note: %s was compiled in lax mode.\n", arg0);
#endif	
	exit(2);
}

/*
 * pidfile: the pidfile will be cleaned up upon exit.
 */
static const char* pidfile = 0;
static char keep_pidfile = 0;
static pid_t child_pid = 0;
static void cleanup_pidfile_and_exit(signum) {
	/*
	 * In the --pidfile case, the child must be terminated as well, when
	 * the parent is being terminated.
	 */
	if(child_pid) {
		kill(child_pid, SIGTERM);
	}
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
	child_pid = fork();
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

	child_pid = 0;

	cleanup_pidfile_and_exit(exitstatus);
	return 0;
}

void execv_as_user(struct passwd* requested_user, const char* arg0, char** argv) {
#if STRICT_MODE
	gid_t requested_gid = requested_user->pw_gid;
	uid_t requested_uid = requested_user->pw_uid;
	
	// fprintf(stderr, "uid: %d\n", getuid());
	// fprintf(stderr, "euid: %d\n", geteuid());
	// fprintf(stderr, "requested_uid: %d\n", requested_uid);
	
	/*
	 * Switch the real user to the effective user
	 */
	if(initgroups(requested_user->pw_name, requested_gid)) die("initgroups");

	if(setregid(requested_gid, requested_gid)) die("setregid");
	if(setreuid(requested_uid, requested_uid)) die("setreuid");
#endif
	
	/*
	 * Fetch system settings to build environment for new process.
	 */
	const char* pw_name = requested_user->pw_name;
	if(!*pw_name) pw_name = 0;
	if(!pw_name) pw_name = "nobody";

	const char* pw_shell = requested_user->pw_shell;
	if(!*pw_shell) pw_shell = 0;
	if(!pw_shell) pw_shell = "/bin/false";

	const char* homedir = requested_user->pw_dir;
	if(!*homedir) homedir = 0;
	if(!homedir) homedir = "/var/empty";
	
	/*
	 * Clear environment. This implements a simple variant of sudo's env_reset
	 * function.
	 *
	 * Note: while sudo falls back to setting TERM, PATH, HOME, MAIL, SHELL,
	 * LOGNAME, USER, USERNAME we skp TERM and MAIL.
	 */

	static const int MAX_ENV_ENTRIES = 128;
	char* env[MAX_ENV_ENTRIES];
	char** envp = env;

	/*
	 * Note that vstrcat leaks memory, see comment above.
	 */
	char* vstrcat(const char *first, ...);

	/*
	 * get initial path.
	 *
	 * Note that /usr/local/bin is in the path, because that allows for a
	 * system-wide installation of various interpreters even when they are
	 * not packaged by the system's package manager.
	 */
	const char* path = "/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin";

	/*
	 * set ruby environment.
	 *
	 * The following code should make --no-print-directory sure that the new user will have gems
	 * installed in a private location. Note that a proper ruby installation
	 * must be installed system-wide; if nothing else works then in
	 * /usr/local/bin.
	 */

	path = vstrcat(homedir, "/bin"           ":", path, 0);
	path = vstrcat(homedir, "/var/bin"       ":", path, 0);
	path = vstrcat(homedir, "/var/gems/bin"  ":", path, 0);

	*envp++ = vstrcat("GEM_HOME=", homedir, "/var/gems", 0);
	*envp++ = vstrcat("GEM_PATH=", homedir, "/var/gems", 0);

	/*
	 * If there is a Gemfile in $homedir/Gemfile, we remember that in the
	 * environment. This make --no-print-directorys sure that bundler finds its Gemfile even
	 * when in the wrong directory; but it also prevents to run multiple
	 * ruby-based apps with different gemsets.
	 */
	/*
	{
		const char* gemfile = vstrcat(homedir, "/Gemfile", 0);

		struct stat stat_buf;
		if(!stat(gemfile, &stat_buf)) {
			(void)0;
			*envp++ = vstrcat("BUNDLE_GEMFILE=", gemfile, 0);
		}
	}
	*/

	*envp = 0;

	/*
	 * set KINKO environment entries
	 */
	const char* kinko_root = getenv("KINKO_ROOT");
	if(!kinko_root) {
		kinko_root = Cwd();
		fprintf(stderr, "KINKO_ROOT environment setting missing; falling back to current dir: %s\n", kinko_root);
	}
	*envp++ = vstrcat("KINKO_ROOT=", kinko_root, 0);
	path = vstrcat(kinko_root, "/sbin:", path, 0);
	path = vstrcat(kinko_root, "/bin:", path, 0);
	*envp++ = vstrcat("JIT_HOME=", homedir, "/var/jit", 0);

	/*
	 * set base entries
	 */
	
	*envp++ = "TMPDIR=/tmp";
	*envp++ = vstrcat("PATH=", path, 0);
	*envp++ = vstrcat("HOME=", homedir, 0);
	*envp++ = vstrcat("SHELL=", pw_shell, 0);
	*envp++ = vstrcat("LOGNAME=", pw_name, 0);
	*envp++ = vstrcat("USER=", pw_name, 0);
	*envp++ = vstrcat("USERNAME=", pw_name, 0);

	*envp = 0;

	/*
	 * change into homedir
	 */
	if(chdir(homedir)) {
		fprintf(stderr, "Cannot chdir into %s's homedir.\n", pw_name);
		perror(homedir);
	}

	/*
	 * set umask. By default an application should not create files writeable
	 * by others.
	 */
	umask(S_IWGRP | S_IWOTH);

	/*
	 * start program.
	 */
	execve(arg0, argv, env);
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

	if(retbuf == NULL) {
		die("malloc");
		return NULL;	/* error */
	}

	(void)strcpy(retbuf, first);

	va_start(argp, first);		/* restart; 2nd scan */

	while((p = va_arg(argp, char *)) != NULL) {
		(void)strcat(retbuf, p);
	}

	va_end(argp);

	return retbuf;
}
