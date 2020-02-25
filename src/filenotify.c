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
#include <dlfcn.h>


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
static void handle_events(int fd, int n_watch_directories, struct directory *directories)
{
	/* Some systems cannot read integer variables if they are not
           properly aligned. On other systems, incorrect alignment may
           decrease performance. Hence, the buffer used for reading from
           the inotify file descriptor should have the same alignment as
           struct inotify_event. */

	char buf[4096]
             __attribute__ ((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;
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
			log_msg("DEBUG", "event inotify from descriptor %i", event->wd);
			char *type;
			char *dirname;
			char *key;

			/* Print event type */
                   	if (event->mask & IN_OPEN) {
				type="IN_OPEN";
			}
			if (event->mask & IN_CLOSE_NOWRITE) {
				type="IN_CLOSE_NOWRITE";
			}
			if (event->mask & IN_CLOSE_WRITE) {
				type="IN_CLOSE_WRITE";
			}
			if (event->mask & IN_DELETE) {
				type="IN_DELETE";
			}

			for (int i = 0; i < n_watch_directories; ++i) {
				if (directories[i].wd == event->wd) {
					log_msg("DEBUG", "dirname=%s descriptor=%i = %i", directories[i].name, directories[i].wd, event->wd);
					dirname = directories[i].name;
					key = directories[i].key;
					break;
				}
			}

			/* Print type of filesystem object */
			if (event->mask & IN_ISDIR) {
				isdir=" [directory]";
			} else {
				isdir=" [file]";
			}
			if (event->len) {
				log_msg("INFO", "[%s] %s : %s/%s %s", type, key, dirname, event->name, isdir);
			} else {
				log_msg("INFO", "[%s] %s : %s/ %s", type, key, dirname, isdir);
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
	int fd;
	nfds_t nfds;
	struct pollfd fds[2];
	struct directory *directories;
	int n_watch_directories=0;
	struct nlist **watch_directories;
	struct nlist *np;

	/* Create the file descriptor for accessing the inotify API */
	fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
		log_msg("ERROR", "Error while init inotify : ", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* determine all directory to watch */
	watch_directories=get_configs(config, "watch_directory.");
	for (int i = 0; i < HASHSIZE; i++)
	{
		for (np = watch_directories[i]; np != NULL; np = np->next)
		{
			n_watch_directories++;
			directories = (struct directory *) realloc(directories, sizeof(struct directory) * n_watch_directories);
			directories[n_watch_directories - 1].wd = inotify_add_watch(fd, np->defn, IN_CLOSE | IN_DELETE );
			directories[n_watch_directories - 1].name = np->defn;
			directories[n_watch_directories - 1].key = np->name;
			if(directories[n_watch_directories - 1].wd  == -1) {
				log_msg("ERROR", "Cannot watch %s : %s", np->defn, strerror(errno));
				exit(EXIT_FAILURE);
			} else {
				log_msg("DEBUG", "inotify descriptor=%i for directory %s/", directories[n_watch_directories - 1].wd, np->defn);
			}
		}
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
				handle_events(fd, n_watch_directories, directories);
                   	}
               }

	}
	free(directories);
	return EXIT_SUCCESS;
}

void loadPlugins()
{
	char *plugin_name = "plg_notify_log.so";
	char *plugin_path = (char *) malloc( strlen(plugin_name) + strlen(get_config("plugins_dir")) + 1 );
	strcpy(plugin_path, get_config("plugins_dir"));
	strcat(plugin_path, plugin_name);
	log_msg("INFO", "Chargement du plugins : %s", plugin_path);
	void *plugin = dlopen(plugin_path, RTLD_NOW);
	if (!plugin)
	{
		log_msg("ERROR", "Cannot load %s: %s", plugin_name, dlerror ());
		exit(EXIT_FAILURE);
	}
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
	display_allconfig(config);
	loadPlugins();
	displayWelcome();
	return mainLoop();
}
