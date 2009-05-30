#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

#include "xalloc.h"
#include "debug.h"
#include "bool.h"

/*
 * Default sleeping time in seconds.
 */
#define SLEEP_TIME 3

#define DEFAULT_LOGFILE "/tmp/foo"

static void daemonize (void);
static void fail (const char *);
static void log_str (const char *, const char *);
static void handle_sgn (const int);
static void help (const char *);
static int checkpath (char **);
static void launch (const char *);

static bool loop = false;

/*
 * Start a parent process, fork and kill the parent so that the child will be
 * tie to init.
 */
static void
daemonize (void) {
   pid_t pid;

   if (getppid () == 1)
      return;

   pid = fork ();

   if (pid < 0)
      fail ("fork ()");

   /*
    * Leave the father.
    */
   if (pid > 0)
      exit (EXIT_SUCCESS);

   umask (0222);

   /*
    * New session, so that the child will not receive signal addressed to the
    * father.
    */
   if (setsid () < 0)
      fail ("setsid ()");

   /*
    * Now, possible to umount partition from where the daemon was started.
    */
   if (chdir ("/") < 0)
      fail ("chdir ()");

   fprintf (stderr, "Started with pid: %d.\n", getpid ());

   /*
    * Get rid of i/o.
    */
#if 0
   freopen ("/dev/null", "r", stdin);
   freopen ("/dev/null", "w", stdout);
   freopen ("/dev/null", "w", stderr);
#endif

   syslog (LOG_INFO, "daemon started (fear)");

   /*
    * Catch signals
    */
   signal (SIGHUP, handle_sgn);
   signal (SIGTERM, handle_sgn);
   signal (42, handle_sgn);
}

static void
fail (const char *str) {
   syslog (LOG_ERR, "'%s' failed, errno = %d (%s).", str,
         errno, strerror (errno));
   exit (EXIT_FAILURE);
}

static void
log_str (const char *file, const char *str) {
   FILE *h = NULL;
   h = fopen (file, "a");
   if (h == NULL)
      syslog (LOG_NOTICE, "cannot open '%s'", file);
   else {
      fprintf (h, "%s\n", str);
      fclose (h);
   }
}

static void
handle_sgn (const int s) {
   switch (s) {
      case SIGHUP:
         syslog (LOG_NOTICE, "Hangup signal received.");
         break;
      case SIGTERM:
         syslog (LOG_NOTICE, "Terminate signal received.");
         exit (EXIT_SUCCESS);
         break;
      default:
         syslog (LOG_NOTICE, "Unknown signal %d received.", s);
         break;
   }
}

static void
help (const char *exe) {
   printf ("%s options: \n", exe);
   puts ("\t-f, --file <path/to/file> : name of the log file");
   printf ("\t (default is '%s')\n", DEFAULT_LOGFILE);
   printf ("\t-e, --exec <path/to/exe> : name of the executable\n");
   puts ("\t-h, --help: display this help");
}

static int
checkpath (char **s) {
   char *pwd = NULL;
   char *tmp = NULL;
   size_t size;
   assert (*s != NULL);
   if (*s[0] == '/')
      return 1;
   /* end of string and '/' */
   size = strlen (getenv ("PWD")) + strlen (*s) + 1 + 1;
   pwd = xcalloc (size, 1);
   strcpy (pwd, getenv ("PWD"));

   *s = realloc (*s, size);

   tmp = xcalloc (strlen (*s) + 1, 1);
   strcpy (tmp, *s);

   strcpy (*s, strcat (strcat (pwd, "/"), tmp));

   xfree (pwd);
   xfree (tmp);
   return 0;
}

void
launch (const char *cmd) {
   int ret = system (cmd);
   if (ret != 0) {
      syslog (LOG_INFO, "An error has occured when executing '%s'", cmd);
      exit (EXIT_FAILURE);
   }
   syslog (LOG_INFO, "'%s' executed.", cmd);
}

int
main (int argc, char **argv) {
   char *cmd = NULL;
   char *logfile = NULL;
   int i;
   unsigned int time = SLEEP_TIME;
   char *exe_name = argv[0];
   for (i = 1; i < argc; i++) {
      if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0) {
         help (exe_name);
         return EXIT_SUCCESS;
      }
      else if (strcmp (argv[i], "-f") == 0 || strcmp (argv[i], "--file") == 0) {
         if (++i >= argc) {
            fprintf (stderr, "%s: argument required.\n", argv[--i]);
            return EXIT_FAILURE;
         }
         else {
            logfile = xcalloc (strlen (argv[i]) + 1, 1);
            strcpy (logfile, argv[i]);
         }
      }
      else if (strcmp (argv[i], "-e") == 0 || strcmp (argv[i], "--exec") == 0) {
         if (++i >= argc) {
            fprintf (stderr, "%s: argument required.\n", argv[--i]);
            return EXIT_FAILURE;
         }
         else {
            cmd = xcalloc (strlen (argv[i]) + 1, 1);
            strcpy (cmd, argv[i]);
         }
      }
      else if (strcmp (argv[i], "-t") == 0 || strcmp (argv[i], "--time") == 0) {
         if (++i >= argc) {
            fprintf (stderr, "%s: argument required.\n", argv[--i]);
            return EXIT_FAILURE;
         }
         else {
            time = atoi (argv[i]);
            printf ("time %d\n", time);
         }
      }
      else if (strcmp (argv[i], "-l") == 0 || strcmp (argv[i], "--loop") == 0)
         loop = true;
      else {
         fprintf (stderr, "Error: Unknown option '%s'.\n", argv[i]);
         help (exe_name);
         return EXIT_FAILURE;
      }
   }

   if (cmd == NULL) {
      fprintf (stderr, "No command specified.\n");
      help (exe_name);
      return EXIT_FAILURE;
   }

   if (logfile == NULL) {
      logfile = xcalloc (strlen (DEFAULT_LOGFILE) + 1, 1);
      strcpy (logfile, DEFAULT_LOGFILE);
   }

   /*
    * Get full path.
    */
   checkpath (&cmd);
   checkpath (&logfile);

   /*
    * Read and write for user.
    */
   umask (022);
   log_str (logfile, cmd);
   fprintf (stderr, "Log to '%s'.\n", logfile);
   fprintf (stderr, "Sleep time set to '%d'.\n", time);

   cmd = realloc (cmd, strlen (cmd) + strlen (logfile) + 4 + 1);
   strcat (strcat (cmd, " >> "), logfile);;

   openlog ("daemon", LOG_PID, LOG_DAEMON);
   daemonize ();
   if (loop)
      for (;;) {
         launch (cmd);
         sleep (time);
      }
   else
      launch (cmd);
   closelog ();

   xfree (cmd);
   xfree (logfile);
   return EXIT_SUCCESS;
}
