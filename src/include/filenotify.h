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
#ifndef filenotify_h
#define filenotify_h
#include <sys/inotify.h>

/**
 * \struct directory_t
 * \brief a object to store directory monitrored by filenotify
 */
typedef struct { 
	char *name; /*!< the directory name */
	char *key;  /*!< the directory key (in config) */
	int wd;     /*!< the inotify descriptor */
	int number; /*!< the number of the directory */
	void *next; /*!< ptr to the next directory */
} directory_t;

/**
 * \struct plugin_t
 * \brief a object to store plugin call after file modification
 */
typedef struct {
	void *next; /*!< ptr to the next plugin */
	void *plugin; /*!< ptr to the plugin object after dlopen */
	char *plugin_name; /*!< the plugin file name (so file) */
        char *p_name; /*!< the plugin name from configfile */
	void (*func_handle)(char *p_name, directory_t *dir, const struct inotify_event *event); /*!< ptr to the function to call after inotify event */
	void (*func_terminate)(); /*!< ptr to the function to call when close plugin */
} plugin_t;

/**
 * \struct plugin_arg_t
 * \brief a object to store args to pass to plugin function handle_event
 */
typedef struct {
	plugin_t *plugin;
	directory_t *dir;
	struct inotify_event *event;
} plugin_arg_t;
 

plugin_t *plugins_lst;
directory_t *directories;
int inotify_fd;

// Function list
int main(int argc, char *argv[]);
void filenotify_displaywelcome();
void filenotify_displayhelp();
void filenotify_handleevents();
int filenotify_mainloop();
plugin_t *filenotify_loadplugins();
directory_t *filenotify_subscribedirectory();
void filenotify_execplugins(directory_t *dir, const struct inotify_event *event);
void *filenotify_execplugin(void *ptrc);

// To free directory list chain
void filenotify_directory_free(directory_t *l);
// To free plugin list chain
void filenotify_plugins_free(plugin_t *l);

void filenotify_exit(int code);
void filenotify_sighandler(int signo);

// Ptr to store string of config file path
char *filenotify_config_file;
// Ptr to store string of pid file path
char *filenotify_pid_filepath;

#endif
