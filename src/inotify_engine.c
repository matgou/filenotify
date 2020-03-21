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

#include <filenotify_engine.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <config.h>
#include <nlist.h>
#include <log.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>


int inotify_fd;

int engine_init()
{
    inotify_fd = inotify_init();

    return inotify_fd;
}

/**
 * \fn void filenotify_handleevents()
 * \brief Read all available inotify events from the file descriptor 'engine_fd'.
 */
void engine_handleevents(int engine_fd)
{
    /* Some systems cannot read integer variables if they are not
       properly aligned. On other systems, incorrect alignment may
       decrease performance. Hence, the buffer used for reading from
       the inotify file descriptor should have the same alignment as
       struct inotify_event. */

    char buf[16384]
	__attribute__((aligned(__alignof__(struct inotify_event))));
    struct inotify_event *event;
    ssize_t len;
    char *ptr;

    /* Loop while events can be read from inotify file descriptor. */
    for (;;) {
	/* Read some events. */
	len = read(engine_fd, buf, sizeof buf);
	if (len == -1 && errno != EAGAIN) {
	    log_msg("ERROR", "Error while read inotify event : ",
		    strerror(errno));
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
	    event = (struct inotify_event *) ptr;
	    log_msg("DEBUG", "event inotify from descriptor %i",
		    event->wd);
	    directory_t *dir = NULL;

	    directory_t *directory_lst_it = directories;
	    for (directory_lst_it = directories; directory_lst_it != NULL;
		 directory_lst_it = directory_lst_it->next) {
		if (directory_lst_it->wd == event->wd) {
		    log_msg("DEBUG", "dirname=%s descriptor=%i = %i",
			    directory_lst_it->name, directory_lst_it->wd,
			    event->wd);
		    dir = directory_lst_it;
		    break;
		}
	    }
	    if (dir != NULL && event->len > 0) {
		plugin_arg_t *event_ = malloc(sizeof(plugin_arg_t));
		event_->dir = dir;
		snprintf(event_->event_filename, 4096, "%s", event->name);
		event_->event_mask = event->mask;

		// exec plugins
		filenotify_execplugins(dir, event_);

		free(event_);
	    }
	}
    }
}


/**
 * \fn directory_t *filenotify_subscribedirectory()
 * \brief read config and start inotify to monitor each directory
 */
directory_t *engine_subscribedirectory()
{
    directory_t *dir = NULL;
    nlist_t *watch_directories;
    int n_watch_directories = 0;
    nlist_t *np;

    /* determine all directory to watch */
    watch_directories = config_getbyprefix(config, "watch_directory.");
    for (np = watch_directories; np != NULL; np = np->next) {
	if (np->defn == NULL) {
	    continue;
	}
	DIR *d = opendir(np->defn);
	struct dirent *dir_;
	if (d) {
	    directory_t *dir_save = dir;
	    dir = (directory_t *) malloc(sizeof(directory_t));
	    dir->next = dir_save;
	    dir->wd =
		inotify_add_watch(inotify_fd, np->defn,
				  IN_MOVE | IN_CLOSE | IN_DELETE);
	    dir->name = strdup(np->defn);
	    dir->key = strdup(np->name);
	    dir->number = n_watch_directories;
	    if (dir->wd == -1) {
		log_msg("ERROR", "Cannot watch %s : %s", np->defn,
			strerror(errno));
		exit(EXIT_FAILURE);
	    } else {
		log_msg("DEBUG", "inotify descriptor=%i for directory %s/",
			dir->wd, np->defn);
	    }

	    n_watch_directories++;

	    while ((dir_ = readdir(d)) != NULL) {
		if (dir_->d_type != DT_DIR) {
		    plugin_arg_t *event = malloc(sizeof(plugin_arg_t));
		    snprintf(event->event_filename, 4096, "%s", dir_->d_name);
		    event->event_mask = IN_CLOSE_WRITE;
		    event->dir = dir;
		    log_msg("INFO", "Presence initiale du fichier : %s/%s",
			    dir->name, event->event_filename);
		    filenotify_execplugins(dir, event);
		    free(event);
		}
	    }
	    closedir(d);
	}
    }
    nlist_free(watch_directories);
    return dir;
}

void engine_rm_watch(int watch_descriptor)
{
    inotify_rm_watch(inotify_fd, watch_descriptor);
}
