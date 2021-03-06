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
#include <filenotify_engine.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <filenotify.h>
#include <string.h>
#include <config.h>
#include <log.h>
#include <poll.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/stat.h>

#define RUNNING_ON_VALGRIND 1


/**
 * Vars to store nb_thread_actif
 */
unsigned int nb_thread_actif = 1;
/* lock update of nb_thread_actif */
pthread_mutex_t nb_thread_actif_mutex;
pthread_mutex_t nb_thread_actif_mod_mutex;
/* check if nb_thread_actif < max_thread */
pthread_cond_t nb_thread_actif_cond;
/* nb_max_thread form config with a default value */
unsigned int nb_max_thread;
/* Thread array (max 32 threads TODO) */
pthread_t threads[32];

/* FD to read event */
int engine_fd;

/**
 * \fn int filenotify_mainloop()
 * \brief main loop of program wait for inotify event and send it to handle
 */
int filenotify_mainloop()
{

    struct pollfd fds[2];

    /* Create the file descriptor for accessing the inotify API */
    engine_fd = engine_init();
    if (engine_fd == -1) {
	log_msg("ERROR", "Error while init engine %s : ",
		FILENOTIFY_ENGINE, strerror(errno));
	exit(EXIT_FAILURE);
    }

    directories = engine_subscribedirectory();

    /* Inotify input */
    fds[0].fd = engine_fd;
    fds[0].events = POLLIN;

    log_msg("INFO", "Listening for %s events...", FILENOTIFY_ENGINE);
    while (1) {
	int poll_num = poll(fds, 1, -1);
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
		engine_handleevents(engine_fd);
	    }
	}

    }
    // never reach
    return EXIT_SUCCESS;
}

/**
 * \fn void filenotify_displayhelp()
 * \brief Display help page
 *
 */
void filenotify_displayhelp()
{
    fprintf(stdout, "Package : %s-%s (please report bug to %s)\n",
	    PACKAGE_STRING, FILENOTIFY_ENGINE, PACKAGE_BUGREPORT);
    fprintf(stdout, "Usage : filenotify -c [config] \n");
    fprintf(stdout, "Options :\n");
    fprintf(stdout,
	    "  -c [config]                        Configuration file\n");
    fprintf(stdout,
	    "  -i [pid_file]                      File to store pid\n");
    fprintf(stdout,
	    "  -d                                 Daemon mode (detach)\n");
}

/**
 * \fn void filenotify_displaywelcome()
 * \brief Function to say the welcome banner
 *
 */
void filenotify_displaywelcome()
{
    log_msg("INFO", " *** Welcome in filenotifier (%s) *** ",
	    FILENOTIFY_ENGINE);
}

/**
 * \fn struct stat *filenotify_get_filestat(char *dirname, char *filename)
 * \brief get file stat form filename
 */
struct stat *filenotify_get_filestat(char *dirname, char *filename)
{
    struct stat *ptr_stat = malloc(sizeof(struct stat));
    char *fullpath = malloc(strlen(dirname) + strlen(filename) + 2);
    // calculate full path
    sprintf(fullpath, "%s/%s", dirname, filename);

    if (lstat(fullpath, ptr_stat) != 0) {
	log_msg("ERROR", "lstat error on (%s) : %s", fullpath,
		strerror(errno));
	free(ptr_stat);
	ptr_stat=NULL;
    }

    free(fullpath);
    return ptr_stat;
}

/**
 * \fn void filenotify_execplugins(directory *dir, const struct inotify_event *event)
 * \brief Exec plugins for an event
 */
void filenotify_execplugins(directory_t * dir, plugin_arg_t * event_)
{
    plugin_t *plugins_lst_it = plugins_lst;
    pthread_attr_t thread_attr;

    if (pthread_attr_init(&thread_attr) != 0) {
	log_msg("ERROR", "pthread_attr_init error : %s", strerror(errno));
	return;
    }

    if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED)
	!= 0) {
	log_msg("ERROR", "pthread_attr_setdetachstate error : %s",
		strerror(errno));
	return;
    }

    for (plugins_lst_it = plugins_lst; plugins_lst_it != NULL;
	 plugins_lst_it = plugins_lst_it->next) {
	log_msg("DEBUG",
		"Start plugin execution (%s) in separate thread (nbthread = %i/%i)",
		plugins_lst_it->p_name, nb_thread_actif, nb_max_thread);

	// plugins_lst_it->func_handle(plugins_lst_it->p_name, dir, event);
	plugin_arg_t *ptr = malloc(sizeof(plugin_arg_t));
	ptr->plugin = plugins_lst_it;
	ptr->dir = dir;
	snprintf(ptr->event_filename, 4096, "%s", event_->event_filename);
	ptr->event_mask = event_->event_mask;
	if (!(ptr->event_mask & IN_DELETE)) {
	    ptr->event_filestat =
		filenotify_get_filestat(dir->name, event_->event_filename);
	} else {
	    ptr->event_filestat = NULL;
	}

	int thread_n = increase_thread_actif();
	ptr->pthread_n = thread_n;
	if (pthread_create
	    (&threads[thread_n], &thread_attr, &filenotify_execplugin,
	     ptr) == -1) {
	    printf("ERROR : Error when create thread : %s",
		   strerror(errno));
	}

    }
    //pthread_attr_destroy(&thread_attr);
    return;
}

/**
 * \fn void *filenotify_execplugin(void *ptrc)
 * \brief Thread start_routine() decode args and start func_handle
 */
void *filenotify_execplugin(void *ptrc)
{
    // decode ptr
    plugin_arg_t *ptr = (plugin_arg_t *) ptrc;
    plugin_t *p = ptr->plugin;
    directory_t *dir = ptr->dir;

    // Exec plugins
    p->func_handle(p->p_name, ptr);

    // Exit thread
    if(ptr->event_filestat) {
        free(ptr->event_filestat);
    }
    free(ptr);

    decrease_thread_actif();
    log_msg("DEBUG",
	    "End of execution plugin (%s) in separate thread (nbthread = %i/%i)",
	    p->p_name, nb_thread_actif, nb_max_thread);
    pthread_exit(NULL);
}


int increase_thread_actif()
{
    /* check if nb_thread_actif < nb_max_thread */
    if (nb_thread_actif >= nb_max_thread) {
	pthread_mutex_lock(&nb_thread_actif_mutex);	/* On verrouille le mutex */
	pthread_cond_wait(&nb_thread_actif_cond, &nb_thread_actif_mutex);
	pthread_mutex_unlock(&nb_thread_actif_mutex);	/* On verrouille le mutex */
    }

    pthread_mutex_lock(&nb_thread_actif_mod_mutex);
    nb_thread_actif = nb_thread_actif + 1;
    pthread_mutex_unlock(&nb_thread_actif_mod_mutex);

    return nb_thread_actif - 1;
}

void decrease_thread_actif()
{
    pthread_mutex_lock(&nb_thread_actif_mod_mutex);
    nb_thread_actif = nb_thread_actif - 1;

    /* liberate one thread */
    if (nb_thread_actif < nb_max_thread) {
	pthread_mutex_lock(&nb_thread_actif_mutex);	/* On verrouille le mutex */
	pthread_cond_signal(&nb_thread_actif_cond);
	pthread_mutex_unlock(&nb_thread_actif_mutex);	/* On verrouille le mutex */
    }

    pthread_mutex_unlock(&nb_thread_actif_mod_mutex);
    return;
}



/**
 * \fn plugin_t *filenotify_loadplugins()
 * \brief Load plugins, open library, call init_plugin and put in memory
 */
plugin_t *filenotify_loadplugins()
{
    plugin_t *plugins_lst_ptr = NULL;
    nlist_t *plugins_config;
    nlist_t *np;
    void (*func_init)(char *p_name, nlist_t * config);

    /* determine all plugins to load */
    plugins_config = config_getbyprefix(config, "plugins.");
    for (np = plugins_config; np != NULL; np = np->next) {
	char *plugin_name = strdup(np->defn);
	int plugin_path_len =
	    strlen(plugin_name) + strlen(config_getbykey("plugins_dir")) +
	    1;
	char *plugin_path =
	    (char *) malloc(plugin_path_len * sizeof(char));
	sprintf(plugin_path, "%s%s", config_getbykey("plugins_dir"),
		plugin_name);

	log_msg("INFO", "Chargement du plugins : %s", plugin_path);
	// Charging .so
	void *plugin = dlopen(plugin_path, RTLD_LAZY);
	if (!plugin) {
	    log_msg("ERROR", "Cannot load %s: %s", plugin_name, dlerror());
	    exit(EXIT_FAILURE);
	}
	free(plugin_path);

	*(void **) (&func_init) = dlsym(plugin, "init_plugin");
	if (!func_init) {
	    /* no such symbol */
	    log_msg("ERROR", "Error: %s", dlerror());
	    dlclose(plugin);
	    exit(EXIT_FAILURE);
	}
	// Init du plugins
	func_init(np->name, config);
	plugin_t *plugins_lst_save = plugins_lst_ptr;
	plugins_lst_ptr = malloc(sizeof(plugin_t));
	plugins_lst_ptr->next = plugins_lst_save;
	plugins_lst_ptr->func_handle = dlsym(plugin, "handle_event");
	if (!plugins_lst_ptr->func_handle) {
	    /* no such symbol */
	    log_msg("ERROR", "Error: %s", dlerror());
	    dlclose(plugin);
	    exit(EXIT_FAILURE);
	}
	plugins_lst_ptr->func_terminate =
	    dlsym(plugin, "terminate_plugin");
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
 * \fn void filenotify_sighandler(int signo)
 * \brief trap signal and stop or reload config
 * \param signo the number of signal
 */
void filenotify_sighandler(int signo)
{
    if (signo == SIGUSR1) {
	log_msg("INFO", "received SIGUSR1");

	// free old plugins
	filenotify_plugins_free(plugins_lst);
	// free old directories
	filenotify_directory_free(directories);
	// free old config
	nlist_free(config);

	// Switch config
	if ((config = config_loadfromfile(filenotify_config_file)) == 0) {
	    fprintf(stderr, "Critical error while loading config, exit\n");
	    exit(EXIT_FAILURE);
	}
	// Display config
	config_displayall(config);

	// Init and load new plugin list
	plugins_lst = filenotify_loadplugins();
	directories = engine_subscribedirectory();
    } else if (signo == SIGUSR2) {
	log_msg("INFO", "received SIGUSR2");
    } else if (signo == SIGINT) {
	log_msg("INFO", "received SIGING => exit");
	filenotify_exit(EXIT_SUCCESS);
    } else if (signo == SIGTERM) {
	log_msg("INFO", "received SIGTERM => exit");
	filenotify_exit(EXIT_SUCCESS);
    }
}

/**
 * \fn void filenotify_directory_free(directory_t *l)
 * \brief Recursive free struct directory_t and stop swatch
 */
void filenotify_directory_free(directory_t * l)
{
    if (l == NULL) {
	return;
    }
    if (l->next != NULL) {
	filenotify_directory_free(l->next);
    }
    engine_rm_watch(l->wd);
    free(l->name);
    free(l->key);
    free(l);
}

/**
 * \fn void filenotify_plugins_free(plugin_t *l)
 * \brief Recursive free plugins structure and close library
 */
void filenotify_plugins_free(plugin_t * l)
{
    if (l != NULL) {
	if (l->next != NULL) {
	    filenotify_plugins_free(l->next);
	}
	log_msg("DEBUG", "Close plugin : %s", l->plugin_name);
	l->func_terminate();
	if (!RUNNING_ON_VALGRIND) dlclose(l->plugin);
	free(l->p_name);
	free(l->plugin_name);
	free(l);
    }
}

/**
 * \fn filenotify_exit(int code)
 * \brief Exit programm with cleaning mem
 */
void filenotify_exit(int code)
{
    filenotify_directory_free(directories);
    filenotify_plugins_free(plugins_lst);
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

    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&nb_thread_actif_mutex, NULL);
    pthread_mutex_init(&nb_thread_actif_mod_mutex, NULL);
    pthread_cond_init(&nb_thread_actif_cond, NULL);

    // by default no fork
    int filenotify_daemon_mode = 0;
    // The pid of daemon
    int pid = getpid();

    while ((c = getopt(argc, argv, "c:i:d")) != -1) {
	switch (c) {
	case 'c':
	    // Store config file path
	    filenotify_config_file = optarg;
	    if (filenotify_config_file == NULL) {
		filenotify_displayhelp();
		return 255;
	    }
	    break;
	case 'i':
	    // Store pid file path
	    filenotify_pid_filepath = optarg;
	    if (filenotify_config_file == NULL) {
		filenotify_displayhelp();
		return 255;
	    }
	    break;
	case 'd':
	    // Fork
	    filenotify_daemon_mode = 1;
	    break;
	case '?':
	    if (optopt == 'c') {
		fprintf(stderr, "Option -%c requires an argument.\n",
			optopt);
		filenotify_displayhelp();
		return 255;
	    } else if (isprint(optopt)) {
		fprintf(stderr, "Unknown option `-%c'.\n", optopt);
		filenotify_displayhelp();
		return 255;
	    } else {
		fprintf(stderr, "Unknown option character `\\x%x'.\n",
			optopt);
		filenotify_displayhelp();
		return 255;
	    }
	}
    }
    config = NULL;

    if ((config = config_loadfromfile(filenotify_config_file)) == 0) {
	fprintf(stderr, "Critical error while loading config, exit\n");
	filenotify_displayhelp();
	return 255;
    }
    config_displayall(config);
    plugins_lst = filenotify_loadplugins();
    filenotify_displaywelcome();

    /*
       Configure to trap all signal
     */
    if (signal(SIGUSR1, filenotify_sighandler) == SIG_ERR) {
	log_msg("ERROR", "can't catch SIGUSR1");
    }
    if (signal(SIGUSR2, filenotify_sighandler) == SIG_ERR) {
	log_msg("ERROR", "can't catch SIGUSR2");
    }
    if (signal(SIGINT, filenotify_sighandler) == SIG_ERR) {
	log_msg("ERROR", "can't catch SIGINT");
    }
    if (signal(SIGTERM, filenotify_sighandler) == SIG_ERR) {
	log_msg("ERROR", "can't catch SIGTERM");
    }

    /*
       Manage if daemon mode is enable
     */
    if (filenotify_daemon_mode) {
	int ppid = fork();
	if (ppid > 0) {
	    pid = ppid;
	} else {
	    pid = 0;
	}
    }

    /*
       Convert config max_thread to int
     */
    char *max_thread_string = config_getbykey("max_thread");
    if (max_thread_string != NULL) {
	log_msg("INFO", "max_thread=%s", max_thread_string);
	nb_max_thread = atoi(max_thread_string);
    } else {
	nb_max_thread = 16;
    }

    /*
       If in the parent process
     */
    if (filenotify_pid_filepath != NULL && pid > 0) {
	/*
	   Store pid in file
	 */
	log_msg("INFO", "Store pid in : %s", filenotify_pid_filepath);
	FILE *filenotify_pid_file;
	if ((filenotify_pid_file =
	     fopen(filenotify_pid_filepath, "w")) == NULL) {
	    log_msg("ERROR", "can't open pid file path for writing");
	} else {
	    fprintf(filenotify_pid_file, "%d", pid);
	    fclose(filenotify_pid_file);
	}
    }
    /*
       If in the child process
     */
    if (pid <= 0) {
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
    }
    /*
       If pid > 0, in parent process
     */
    if (filenotify_daemon_mode && pid > 0) {
	return 0;
    }


    return filenotify_mainloop();
}
