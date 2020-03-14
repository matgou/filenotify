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
#include <stdint.h>
#include "../../filenotify_config.h"

/* Check if linux inotify engine exist */
#if USE_INOTIFY
#define FILENOTIFY_ENGINE "INOTIFY_ENGINE"
#else
#define FILENOTIFY_ENGINE "TIMER_ENGINE"
#endif

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
	void (*func_handle)(char *p_name, directory_t *dir, char *filename, uint32_t mask); /*!< ptr to the function to call after inotify event */
	void (*func_terminate)(); /*!< ptr to the function to call when close plugin */
} plugin_t;

/**
 * \struct plugin_arg_t
 * \brief a object to store args to pass to plugin function handle_event
 */
typedef struct {
	plugin_t *plugin;
	directory_t *dir;
	int pthread_n;
	char *event_filename;
	unsigned int event_mask;
} plugin_arg_t;


plugin_t *plugins_lst;
directory_t *directories;

// Function list
int main(int argc, char *argv[]);
void filenotify_displaywelcome();
void filenotify_displayhelp();
int filenotify_mainloop();
plugin_t *filenotify_loadplugins();
void filenotify_execplugins(directory_t *dir, plugin_arg_t *event_);
void *filenotify_execplugin(void *ptrc);

// To free directory list chain
void filenotify_directory_free(directory_t *l);
// To free plugin list chain
void filenotify_plugins_free(plugin_t *l);
void filenotify_exit(int code);
void filenotify_sighandler(int signo);
int increase_thread_actif();
void decrease_thread_actif();


// Ptr to store string of config file path
char *filenotify_config_file;
// Ptr to store string of pid file path
char *filenotify_pid_filepath;

/***********************************************************************************************/
/*                       Inotify Mask                                                          */
/***********************************************************************************************/

/* Supported events suitable for MASK parameter of INOTIFY_ADD_WATCH.  */
#define IN_ACCESS         0x00000001        /* File was accessed.  */
#define IN_MODIFY         0x00000002        /* File was modified.  */
#define IN_ATTRIB         0x00000004        /* Metadata changed.  */
#define IN_CLOSE_WRITE         0x00000008        /* Writtable file was closed.  */
#define IN_CLOSE_NOWRITE 0x00000010        /* Unwrittable file closed.  */
#define IN_CLOSE         (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) /* Close.  */
#define IN_OPEN                 0x00000020        /* File was opened.  */
#define IN_MOVED_FROM         0x00000040        /* File was moved from X.  */
#define IN_MOVED_TO      0x00000080        /* File was moved to Y.  */
#define IN_MOVE                 (IN_MOVED_FROM | IN_MOVED_TO) /* Moves.  */
#define IN_CREATE         0x00000100        /* Subfile was created.  */
#define IN_DELETE         0x00000200        /* Subfile was deleted.  */
#define IN_DELETE_SELF         0x00000400        /* Self was deleted.  */
#define IN_MOVE_SELF         0x00000800        /* Self was moved.  */

/* Events sent by the kernel.  */
#define IN_UNMOUNT         0x00002000        /* Backing fs was unmounted.  */
#define IN_Q_OVERFLOW         0x00004000        /* Event queued overflowed.  */
#define IN_IGNORED         0x00008000        /* File was ignored.  */

/* Helper events.  */
#define IN_CLOSE         (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)        /* Close.  */
#define IN_MOVE                 (IN_MOVED_FROM | IN_MOVED_TO)                /* Moves.  */

/* Special flags.  */
#define IN_ONLYDIR         0x01000000        /* Only watch the path if it is a
                                           directory.  */
#define IN_DONT_FOLLOW         0x02000000        /* Do not follow a sym link.  */
#define IN_MASK_ADD         0x20000000        /* Add to the mask of an already
                                           existing watch.  */
#define IN_ISDIR         0x40000000        /* Event occurred against dir.  */
#define IN_ONESHOT         0x80000000        /* Only send event once.  */

/* All events which a program can wait on.  */
#define IN_ALL_EVENTS         (IN_ACCESS | IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE  \
                          | IN_CLOSE_NOWRITE | IN_OPEN | IN_MOVED_FROM              \
                          | IN_MOVED_TO | IN_CREATE | IN_DELETE                      \
                          | IN_DELETE_SELF | IN_MOVE_SELF)


#endif
