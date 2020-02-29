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
#include <dirent.h>
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
#include <signal.h>

// This is a hook to use older memcpy to keep compatibilty with older linux
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");


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
	fprintf(stdout, "  -i [pid_file]                      File to store pid\n");
	fprintf(stdout, "  -d                                 Daemon mode (detach)\n");
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
 * \brief Read all available inotify events from the file descriptor 'inotify_fd'.
 *        wd is the table of watch descriptors for the directories in argv.
 *        argc is the length of wd and argv.
 *        argv is the list of watched directories.
 *        Entry 0 of wd and argv is unused.
 */
void handle_events()
{
	/* Some systems cannot read integer variables if they are not
           properly aligned. On other systems, incorrect alignment may
           decrease performance. Hence, the buffer used for reading from
           the inotify file descriptor should have the same alignment as
           struct inotify_event. */

	char buf[4096]
	__attribute__ ((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;
	ssize_t len;
	char *ptr;

        /* Loop while events can be read from inotify file descriptor. */
        for (;;) {
		/* Read some events. */
		len = read(inotify_fd, buf, sizeof buf);
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
			struct directory *dir = NULL;

			struct directory *directory_lst_it = directories;
			for(directory_lst_it = directories; directory_lst_it != NULL; directory_lst_it = directory_lst_it->next) {
				if (directory_lst_it->wd == event->wd) {
					log_msg("DEBUG", "dirname=%s descriptor=%i = %i", directory_lst_it->name, directory_lst_it->wd, event->wd);
					dir = directory_lst_it;
					break;
				}
			}
			if(dir != NULL) {
				struct plugins *plugins_lst_it = plugins_lst;
                		for (plugins_lst_it = plugins_lst; plugins_lst_it != NULL; plugins_lst_it = plugins_lst_it->next) {
					plugins_lst_it->func_handle(plugins_lst_it->p_name, dir, event);
				}
            		}
		}
	}
}

struct directory *watchInotifyDirectory()
{
	struct directory *dir=NULL;
	struct nlist *watch_directories;
	int n_watch_directories=0;
	struct nlist *np;

	/* determine all directory to watch */
	watch_directories=get_configs(config, "watch_directory.");
        for(np = watch_directories; np != NULL; np = np->next) {
		DIR *d = opendir(np->defn);
		struct dirent *dir_;
		if (d)
		{
			struct directory *dir_save = dir;
			dir = (struct directory *) malloc(sizeof(struct directory));
			dir->next=dir_save;
			dir->wd = inotify_add_watch(inotify_fd, np->defn, IN_MOVE | IN_CLOSE | IN_DELETE );
			dir->name = strdup(np->defn);
			dir->key = strdup(np->name);
			dir->number = n_watch_directories;
			if(dir->wd  == -1) {
				log_msg("ERROR", "Cannot watch %s : %s", np->defn, strerror(errno));
				exit(EXIT_FAILURE);
			} else {
				log_msg("DEBUG", "inotify descriptor=%i for directory %s/", dir->wd, np->defn);
			}

			n_watch_directories++;

			while ((dir_ = readdir(d)) != NULL)
			{
				if(dir_->d_type != DT_DIR) {
					struct inotify_event *event = malloc(sizeof(struct inotify_event) + (sizeof(char) * (strlen(dir_->d_name) + 1)));
					memcpy(event->name, dir_->d_name, strlen(dir_->d_name));
					event->name[strlen(dir_->d_name)] = '\0';
					event->len = strlen(dir_->d_name);
					event->mask = IN_CLOSE_WRITE;
	                                struct plugins *plugins_lst_it = plugins_lst;
					log_msg("INFO", "Presence initiale du fichier : %s/%s (%i) (%i)", dir->name, event->name, strlen(event->name), strlen(dir->name));
        	                        for (plugins_lst_it = plugins_lst; plugins_lst_it != NULL; plugins_lst_it = plugins_lst_it->next) {
                	                        plugins_lst_it->func_handle(plugins_lst_it->p_name, dir, event);
                        	        }
					free(event);
				}
			}
			closedir(d);
		}
	}
	nlist_free(watch_directories);
	return dir;
}

/**
 * \fn int mainLoop()
 * \brief main loop of programme
 */
int mainLoop()
{
	nfds_t nfds;
	struct pollfd fds[2];

	/* Create the file descriptor for accessing the inotify API */
	inotify_fd = inotify_init();
	if (inotify_fd == -1) {
		log_msg("ERROR", "Error while init inotify : ", strerror(errno));
		exit(EXIT_FAILURE);
	}

	directories = watchInotifyDirectory();



	/* Prepare for polling */
	nfds = 1;

	/* Inotify input */
	fds[0].fd = inotify_fd;
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
				handle_events();
			}
		}

	}
	free(directories);
	return EXIT_SUCCESS;
}

struct plugins *loadPlugins()
{
	struct plugins *plugins_lst_ptr = NULL;
	struct nlist *plugins_config;
	struct nlist *np;
	void (*func_init)(char *p_name, struct nlist *config);

	/* determine all plugins to load */
	plugins_config=get_configs(config, "plugins.");
	for(np = plugins_config; np != NULL; np = np->next) {
		char *plugin_name = strdup(np->defn);
		char *plugin_path = (char *) malloc( strlen(plugin_name) + strlen(get_config("plugins_dir")) + 1 );
		strcpy(plugin_path, get_config("plugins_dir"));
		strcat(plugin_path, plugin_name);
		log_msg("INFO", "Chargement du plugins : %s", plugin_path);
		// Charging .so
		void *plugin = dlopen(plugin_path, RTLD_LAZY);
		if (!plugin)
		{
			log_msg("ERROR", "Cannot load %s: %s", plugin_name, dlerror ());
			exit(EXIT_FAILURE);
		}
		free(plugin_path);

		*(void**)(&func_init) = dlsym(plugin, "init_plugin");
		if (!func_init) {
			/* no such symbol */
			log_msg("ERROR", "Error: %s", dlerror());
			dlclose(plugin);
			exit(EXIT_FAILURE);
		}

		// Init du plugins
		func_init(np->name, config);
		struct plugins *plugins_lst_save = plugins_lst_ptr;
		plugins_lst_ptr = malloc(sizeof(struct plugins));
		plugins_lst_ptr->next=plugins_lst_save;
		plugins_lst_ptr->func_handle = dlsym(plugin, "handle_event");
		if (!plugins_lst_ptr->func_handle) {
                        /* no such symbol */
                        log_msg("ERROR", "Error: %s", dlerror());
                        dlclose(plugin);
                        exit(EXIT_FAILURE);
                }
		plugins_lst_ptr->func_terminate = dlsym(plugin, "terminate_plugin");
		if (!plugins_lst_ptr->func_terminate) {
                        /* no such symbol */
                        log_msg("ERROR", "Error: %s", dlerror());
                        dlclose(plugin);
                        exit(EXIT_FAILURE);
                }

		plugins_lst_ptr->plugin = plugin;
		plugins_lst_ptr->plugin_name = plugin_name;
		plugins_lst_ptr->p_name = strdup(np->name);
		if (!plugins_lst_ptr->func_handle) {
			/* no such symbol */
			log_msg("ERROR", "Error: %s", dlerror());
			dlclose(plugin);
			exit(EXIT_FAILURE);
		}
	}

	nlist_free(plugins_config);
	return plugins_lst_ptr;
}

/**
 * \fn void sig_handler
 * \brief trap signal
 */
void sig_handler(int signo)
{
	if (signo == SIGUSR1) {
		log_msg("INFO", "received SIGUSR1");

		// free old plugins
		free_plugins(plugins_lst);
		// free old directories
		free_directories(directories);
		// free old config
		nlist_free(config);

		// Switch config
		if((config = loadConfig(filenotify_config_file)) == 0) {
			fprintf (stderr, "Critical error while loading config, exit\n");
			exit(EXIT_FAILURE);
		}

		// Display config
		display_allconfig(config);

		// Init and load new plugin list
		plugins_lst = loadPlugins();
		directories = watchInotifyDirectory();
	} else if (signo == SIGUSR2) {
		log_msg("INFO", "received SIGUSR2");
	} else if (signo == SIGINT) {
		log_msg("INFO", "received SIGING => exit");
		prg_exit(EXIT_SUCCESS);
	} else if (signo == SIGTERM) {
		log_msg("INFO", "received SIGTERM => exit");
		prg_exit(EXIT_SUCCESS);
	}
}

/**
 * \fn void free_struct()
 * \brief Recursive free structure
 */
void free_directories(struct directory *l) {
	if(l->next != NULL) {
		free_directories(l->next);
	}
	inotify_rm_watch(inotify_fd,l->wd);
	free(l->name);
	free(l->key);
	free(l);
}

/**
 * \fn void free_struct()
 * \brief Recursive free structure
 */
void free_plugins(struct plugins *l) {
	if(l != NULL) {
		if(l->next != NULL) {
			free_plugins(l->next);
		}
		log_msg("DEBUG", "Close plugin : %s", l->plugin_name);
		l->func_terminate();
		dlclose(l->plugin);
		free(l->p_name);
		free(l->plugin_name);
		free(l);
	}
}

/**
 * \fn prg_exit()
 * \brief Exit programm with cleaning mem
 */
void prg_exit(int code) {
	free_directories(directories);
	free_plugins(plugins_lst);
	nlist_free(config);
	exit(code);
}

/**
 * \fn int main ()
 * \brief entry of the filenotify process
 */
int main(int argc, char *argv[])
{
	int c;
	// by default no fork
	int filenotify_daemon_mode=0;
	// The pid of daemon
	int pid = getpid();

	while ((c = getopt (argc, argv, "c:i:d")) != -1) {
		switch (c)
		{
			case 'c':
				// Store config file path
				filenotify_config_file = optarg;
				if(filenotify_config_file == NULL) {
					displayHelp();
					return 255;
				}
				break;
			case 'i':
				// Store pid file path
				filenotify_pid_filepath = optarg;
				if(filenotify_config_file == NULL) {
                                        displayHelp();
                                        return 255;
                                }
                                break;
			case 'd':
				// Fork
				filenotify_daemon_mode=1;
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
	}
	config = NULL;

	if((config = loadConfig(filenotify_config_file)) == 0) {
		fprintf (stderr, "Critical error while loading config, exit\n");
		displayHelp();
		return 255;
	}
	display_allconfig(config);
	plugins_lst = loadPlugins();
	displayWelcome();

	/*
	Configure to trap all signal
	 */
	if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
		log_msg("ERROR", "can't catch SIGUSR1");
	}
	if (signal(SIGUSR2, sig_handler) == SIG_ERR) {
		log_msg("ERROR", "can't catch SIGUSR2");
	}
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		log_msg("ERROR", "can't catch SIGINT");
	}
	if (signal(SIGTERM, sig_handler) == SIG_ERR) {
		log_msg("ERROR", "can't catch SIGTERM");
	}

	/*
	Manage if daemon mode is enable
	 */
	if(filenotify_daemon_mode) {
		int ppid = fork();
		if(ppid > 0) {
			pid=ppid;
		} else {
			pid=0;
		}
	}

	/*
	If in the parent process
	 */
	if(filenotify_pid_filepath != NULL && pid > 0) {
	/*
	Store pid in file
	 */
		log_msg("INFO", "Store pid in : %s", filenotify_pid_filepath);
		FILE *filenotify_pid_file;
		if((filenotify_pid_file = fopen(filenotify_pid_filepath, "w")) == NULL) {
			log_msg("ERROR", "can't open pid file path for writing");
		} else {
			fprintf(filenotify_pid_file, "%d", pid); 
			fclose(filenotify_pid_file);
		}
	}
	/*
	If in the child process
	*/
	if(pid <= 0) {
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
	}	
	/*
	If pid > 0, in parent process
	 */
	if(filenotify_daemon_mode && pid > 0) {
		return 0;
	}
	return mainLoop();
}
