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
 * \file plg_notify_log.c
 * \brief Plugin pour la notification via le fichier de log
 * \author Goulin.M
 * \version 0.1
 * \date 2019/02/22
 *
 */
#include <sys/inotify.h>
#include <filenotify.h>
#include <plg_notify.h>
#include <config.h>
#include <log.h>

void init_plugin(struct nlist **config_ref)
{
	config = config_ref;
}

/**
 * \fn void handle_event()
 * \brief Write log from received event
 */
void handle_event(struct directory *dir, const struct inotify_event *event)
{
	char *type;
	char *isdir;
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

	/* Print type of filesystem object */
	if (event->mask & IN_ISDIR) {
		isdir=" [directory]";
	} else {
		isdir=" [file]";
	}
	if (event->len) {
		log_msg("INFO", "[%s] %s : %s/%s %s", type, dir->key, dir->name, event->name, isdir);
	} else {
		log_msg("INFO", "[%s] %s : %s/ %s", type, dir->key, dir->name, isdir);
	}
}
