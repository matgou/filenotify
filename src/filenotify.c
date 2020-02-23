/******************************************************************************
*   Copyright (C)      Mathieu Goulin <mathieu.goulin@gadz.org>               *
*                                                                             *
*   This program is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Lesser General Public                *
*   License as published by the Free Software Foundation; either              *
*   version 2.1 of the License, or (at your option) any later version.        *
*                                                                             *
*   The GNU C Library is distributed in the hope that it will be useful,      *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
*   Lesser General Public License for more details.                           *
*                                                                             *
*   You should have received a copy of the GNU Lesser General Public          *
*   License along with the GNU C Library; if not, write to the Free           *
*   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA         *
*   02111-1307 USA.                                                           *
******************************************************************************/
/**
 * \file filenotify.c
 * \brief Programme de surveillance de fichier et de notification
 * \author Goulin.M
 * \version 0.1
 * \date 2019/02/22
 *
 */
#include <sys/inotify.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/inotify.h>
#include <errno.h>
#include <filenotify.h>
#include <config.h>
#include <log.h>
#include <poll.h>


/**
 * \fn void displayHelp()
 * \brief Display help page 
 *
 */
void
displayHelp ()
{
	fprintf(stdout, "Usage : filenotify -c [config] \n");
	fprintf(stdout, "Options :\n");
	fprintf(stdout, "  -c [config]                        Configuration file\n");
}

/**
 * \fn void displayWelcome()
 * \brief Function to say the welcome banner
 *
 */
void
displayWelcome ()
{
	log_msg("INFO", " *** Welcome in filenotifier *** ");
}

/**
 * \fn void handle_events()
 * \brief Read all available inotify events from the file descriptor 'fd'.
 *        wd is the table of watch descriptors for the directories in argv.
 *        argc is the length of wd and argv.
 *        argv is the list of watched directories.
 *        Entry 0 of wd and argv is unused.
 */
static void handle_events(int fd, int *wd)
{
	/* Some systems cannot read integer variables if they are not
           properly aligned. On other systems, incorrect alignment may
           decrease performance. Hence, the buffer used for reading from
           the inotify file descriptor should have the same alignment as
           struct inotify_event. */

	char buf[4096]
             __attribute__ ((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;
	int i;
	char *isdir;
	ssize_t len;
	char *ptr;

        /* Loop while events can be read from inotify file descriptor. */
        for (;;) {
		/* Read some events. */
		len = read(fd, buf, sizeof buf);
		if (len == -1 && errno != EAGAIN) {
			log_msg("ERROR", "Error while read inotify event : ", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* If the nonblocking read() found no events to read, then
                   it returns -1 with errno set to EAGAIN. In that case,
                   we exit the loop. */
		if (len <= 0) {
                   break;
		}

		/* Loop over all events in the buffer */
		for (ptr = buf; ptr < buf + len;
		     ptr += sizeof(struct inotify_event) + event->len) {
			event = (const struct inotify_event *) ptr;
			char *type;

			/* Print event type */
                   	if (event->mask & IN_OPEN) {
				type="IN_OPEN: ";
			}
			if (event->mask & IN_CLOSE_NOWRITE) {
				type="IN_CLOSE_NOWRITE: ";
			}
			if (event->mask & IN_CLOSE_WRITE) {
				type="IN_CLOSE_WRITE: ";
			}
			if (event->mask & IN_DELETE) {
				type="IN_DELETE: ";
			}

			/* Print type of filesystem object */
			if (event->mask & IN_ISDIR) {
				isdir=" [directory]";
			} else {
				isdir=" [file]";
			}
			if (event->len) {
				log_msg("INFO", "%s %s %s", type, event->name, isdir);
			} else {
				log_msg("INFO", "%s %s", type, isdir);
			}
		}
	}
}

/**
 * \fn int mainLoop()
 * \brief main loop of programme
 */ 
int mainLoop()
{
	char buf;
	int fd, i, poll_num;
	int *wd;
	nfds_t nfds;
	struct pollfd fds[2];

	/* Create the file descriptor for accessing the inotify API */
	fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
		log_msg("ERROR", "Error while init inotify : ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	wd = inotify_add_watch(fd, "/tmp", IN_CLOSE | IN_DELETE );
	if(wd  == -1) {
		log_msg("ERROR", "Cannot watch /tmp : %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Prepare for polling */
	nfds = 1;

	/* Inotify input */
	fds[0].fd = fd;
	fds[0].events = POLLIN;

	log_msg("INFO", "Listening for events...");
	while (1) {
               int poll_num = poll(fds, nfds, -1);
		if (poll_num == -1) {
			if (errno == EINTR) {
				continue;
			}
			log_msg("ERROR", "Cannot poll event : %s", strerror(errno));
                   	exit(EXIT_FAILURE);
		}
		if (poll_num > 0) {
			if (fds[0].revents & POLLIN) {
				/* Inotify events are available */
				handle_events(fd, wd);
                   	}
               }

	}
	close(wd);
	free(wd);
	return EXIT_SUCCESS;
}

/**
 * \fn int main ()
 * \brief entry of the filenotify process
 */
int main(int argc, char *argv[])
{
	int c;
	char *configFilePath = NULL;

	while ((c = getopt (argc, argv, "c:")) != -1)
		switch (c)
		{
      			case 'c':
        			configFilePath = optarg;
				if(configFilePath == NULL) {
					displayHelp();
                                        return 255;
				}
				break;
			case '?':
				if (optopt == 'c') {
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
					displayHelp();
                                        return 255;
				} else if (isprint (optopt)) {
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
					displayHelp();
                                        return 255;
				} else {
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
					displayHelp();
					return 255;
				}
		}
	if(loadConfig(configFilePath) != 0) {
		fprintf (stderr, "Critical error while loading config, exit\n");
		displayHelp();
		return 255;
	}
	
	displayWelcome();
	return mainLoop();
}
