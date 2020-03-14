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

#include <filenotify.h>
#include <filenotify_engine.h>
#include <stdio.h>
#include <stdlib.h>
#include <log.h>
#include <nlist.h>
#include <config.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/* Config of timer freq */
#define __TIMER_WAIT_SEC__ 1

/* The pipe */
int timer_fd[2];

/* The timer_thread */
pthread_t timer_thread;

/* The filelist */
typedef struct {
        char *path; /*!< the path of filename */
        plugin_arg_t *event;  /*!< the event of creation */
        void *next;     /*!< the next element descriptor */
} timer_engine_plugin_arg_list_t;

timer_engine_plugin_arg_list_t *files_list;

static void timer_engine_send_events();

static void *timer_engine_loop(void *arg)
{
    for(;;) {
        log_msg("DEBUG", "Timer_engine tic ... ");
        timer_engine_send_events();

        sleep(__TIMER_WAIT_SEC__);
    }
    return NULL;
}

timer_engine_plugin_arg_list_t *timer_engine_find(char *dirpath, char *filename) {
    // build path
    char *filepath = malloc(strlen(dirpath) + strlen("/") + strlen(filename) + 1);
    sprintf(filepath, "%s/%s", dirpath, filename);
    timer_engine_plugin_arg_list_t *e;
    // Search for elem
    for(e = files_list; e != NULL; e = e->next) {
        if(strcmp(filepath, e->path) == 0) {
                free(filepath);
                return e;
        }
    }
    // free and return
    free(filepath);
    return e;
}

static void timer_engine_send_events() {
	directory_t *dir;
	struct dirent *dir_;

    /* Search for new file in elem */
	for(dir = directories; dir != NULL; dir = dir->next) {
		DIR *d = opendir(dir->name);
		while ((dir_ = readdir(d)) != NULL)
		{
			if(dir_->d_type != DT_DIR) {
                if(timer_engine_find(dir->name, dir_->d_name) == NULL) {
                    plugin_arg_t *event = malloc(sizeof(plugin_arg_t));
                    event->dir = dir;
                    event->event_filename = strdup(dir_->d_name);
                    event->event_mask = IN_CLOSE_WRITE;
                    log_msg("INFO", "Presence du fichier : %s/%s", dir->name, event->event_filename);

                    write(timer_fd[1], event, sizeof(plugin_arg_t));

                    timer_engine_plugin_arg_list_t *elem = (timer_engine_plugin_arg_list_t *) malloc(sizeof(timer_engine_plugin_arg_list_t));
                    elem->event = event;
                    elem->path = malloc(strlen(dir->name) + strlen("/") + strlen(event->event_filename) + 1);
                    sprintf(elem->path, "%s/%s", dir->name, event->event_filename);
                    elem->next = files_list;
                    files_list = elem;
                    // TODO
                    //free(elem->path)
                    //free(elem)
                    //free(event->event_filename);
                    //free(event);
                }
			}
		}
		closedir(d);
	}

	/* Check if file are delete */
    timer_engine_plugin_arg_list_t *e;
    timer_engine_plugin_arg_list_t *e_prev = NULL;

    for(e = files_list; e != NULL; e = e->next) {
        if(access(e->path, F_OK) != 0) {
                plugin_arg_t *event = e->event;
                event->event_mask = IN_DELETE;
                write(timer_fd[1], event, sizeof(plugin_arg_t));
                free(event);
                if(e_prev != NULL) {
                    e_prev->next = e->next;
                } else {
                    files_list = e->next;
                }
                free(e->path);
                free(e);
        } else {
            e_prev = e;
        }
    }
}

int engine_init()
{
    // init pipe to send event to master process
    if (pipe(timer_fd) == -1) {
        log_msg("ERROR", "ERROR while init pipe : %s", strerror(errno));
        return -1;
    }

  	pthread_attr_t thread_attr;

	if (pthread_attr_init (&thread_attr) != 0) {
		log_msg("ERROR", "pthread_attr_init error : %s", strerror(errno));
		return -1;
  	}

	if (pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED) != 0) {
		log_msg("ERROR", "pthread_attr_setdetachstate error : %s", strerror(errno));
		return -1;
  	}

    if(pthread_create(&timer_thread, &thread_attr, &timer_engine_loop, NULL) == -1) {
		printf("ERROR : Error when create thread : %s", strerror(errno));
		return -1;
    }

    return timer_fd[0];
}


/**
 * \fn void filenotify_handleevents()
 * \brief Read all available events from the file descriptor 'engine_fd'.
 */
void engine_handleevents(int engine_fd)
{
//	/* Some systems cannot read integer variables if they are not
//           properly aligned. On other systems, incorrect alignment may
//           decrease performance. Hence, the buffer used for reading from
//           the inotify file descriptor should have the same alignment as
//           struct inotify_event. */
//
	char buf[4096];
//	__attribute__ ((aligned(__alignof__(struct inotify_event))));
    plugin_arg_t *event;
	ssize_t len;
	char *ptr;
//
//    /* Loop while events can be read from inotify file descriptor. */
    for (;;) {
//		/* Read some events. */
		len = read(engine_fd, buf, sizeof buf);
		if (len == -1 && errno != EAGAIN) {
			log_msg("ERROR", "Error while read inotify event : ", strerror(errno));
			exit(EXIT_FAILURE);
		}
//
//		/* If the nonblocking read() found no events to read, then
//                   it returns -1 with errno set to EAGAIN. In that case,
//                   we exit the loop. */
		if (len <= 0) {
                   break;
		}
//
//		/* Loop over all events in the buffer */
		for (ptr = buf; ptr < buf + len;
		     ptr += sizeof(plugin_arg_t)) {
			event = (plugin_arg_t *) ptr;
			directory_t *dir = event->dir;
			log_msg("DEBUG", "event from directory %s", dir->name);

			filenotify_execplugins(dir, event);

		}
	}
}


/**
 * \fn directory_t *filenotify_subscribedirectory()
 * \brief read config and start inotify to monitor each directory
 */
directory_t *engine_subscribedirectory()
{
	directory_t *dir=NULL;
	nlist_t *watch_directories;
	int n_watch_directories=0;
	nlist_t *np;

	/* determine all directory to watch */
	watch_directories=config_getbyprefix(config, "watch_directory.");
	for(np = watch_directories; np != NULL; np = np->next) {
		if(np->defn == NULL) {
			continue;
		}
		DIR *d = opendir(np->defn);
		struct dirent *dir_;
		if (d)
		{
			directory_t *dir_save = dir;
			dir = (directory_t *) malloc(sizeof(directory_t));
			dir->next=dir_save;
			dir->name = strdup(np->defn);
			dir->key = strdup(np->name);
			dir->number = n_watch_directories;
			if(dir->wd  == -1) {
				log_msg("ERROR", "Cannot watch %s : %s", np->defn, strerror(errno));
				exit(EXIT_FAILURE);
			} else {
				log_msg("DEBUG", "Start watch directory %s/", np->defn);
			}

			n_watch_directories++;

		}
	}
	nlist_free(watch_directories);
	return dir;
}

void engine_rm_watch(int watch_descriptor) {
//    inotify_rm_watch(engine_fd, watch_descriptor);
}
