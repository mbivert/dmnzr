#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DLOG "/tmp/dmnzr.log"

static char *logfile;

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

	logfile = DLOG;

	i = 1;
	/* at least an executable name is required */
	if (argc < i+1)
		goto help;

	if (argv[i][0] == '-')
	if (argv[i][1] == 'f') {
		/* skip -f */
		i++;
		/* look for both logfile and executable */
		if (argc < i+2)
			goto help;
		logfile = argv[i];
		/* skip logfile */
		i++;
	}

	executable = argv[i];

	if (daemonize() < 0)
		return 2;

	execvp(executable, argv+i);
	perror("execvp");

	return 3;
help:
	help(argv[0]);
	return 1;
}
