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


struct directory { /* table entry: */
	char *name; /* directory name */
	char *key;  /* directory key */
	int wd;     /* inotify descriptor */
};

struct plugins {
	struct plugins *next;
	void (*func_handle)(struct directory *dir, struct inotify_event *event);
};
struct plugins *plugins_lst;

// Function list
int main(int argc, char *argv[]);
void displayWelcome();
void displayHelp();
void handle_events(int fd, int n_watch_directories, struct directory **directories);
int mainLoop();
void prg_exit(int code);
void sig_handler(int signo);
char *configFilePath;

#endif
