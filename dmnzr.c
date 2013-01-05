#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DLOG "/tmp/dmnzr.log"
#define PATH_MAX 256

static char logfile[PATH_MAX];

static void
help(char *argv0)
{
	printf("%s [-f logfile] args\n", argv0);
}

static void
handlesig(int s)
{
	if (s == SIGTERM || s == SIGHUP)
		exit(0);
}

int
daemonize(void)
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}

	/* exit father */
	if (pid > 0)
		exit(0);

	/* new session for signals */
	if (setsid() < 0) {
		perror("setsid");
		return -1;
	}

	/* ensure permission */
	if (chmod(logfile, 0644) < 0)
		fprintf(stderr, "can't write to %s\n", logfile);

	/* log stdin & stdout */
	freopen(logfile, "a", stdout);
	freopen(logfile, "a", stderr);

	signal(SIGTERM, handlesig);

	return 0;
}

int
main(int argc, char **argv)
{
	char *executable;
	int i;

	strcpy(logfile, DLOG);

	/* at least executable is required */
	i = 1;
	if (argc < i+1) {
		help(argv[0]);
		return 1;
	}

	if (argv[i][0] == '-')
	if (argv[i][1] == 'f') {
		i++;
		/* look for both logfile and executable */
		if (argc < 4) {
			help(argv[0]);
			return 1;
		}
		strcpy(logfile, argv[i]);
	}
	/* skip '-f' or logfile */
	i++;
	executable = argv[i];

	if (daemonize() < 0)
		return 2;

	execvp(executable, argv+i);
	perror("execvp");
	return 3;
}
