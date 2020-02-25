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
 * \file plg_http_post.c
 * \brief Plugin pour envoyer un http post a chaque modification de fichier
 * \author Goulin.M
 * \version 0.1
 * \date 2019/02/25
 *
 */
#include <sys/inotify.h>
#include <filenotify.h>
#include <plg_notify.h>
#include <config.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>


/**
 * \fn void init_plugin()
 * \brief initialise un Plugins
 */
void
init_plugin(struct nlist *config_ref[HASHSIZE])
{

	for(int i = 0; i < HASHSIZE; i++) {
		config[i] = config_ref[i];
	}
}

/**
 * \fn void handle_event()
 * \brief Write log from received event
 */
void handle_event(struct directory *dir, struct inotify_event *event)
{
	log_msg("DEBUG", "handle - plg_http_post");
	char *value;
	/* Print event type */
	if (event->mask & IN_OPEN) {
		value="1";
	}
	if (event->mask & IN_CLOSE_NOWRITE) {
		value="1";
	}
	if (event->mask & IN_CLOSE_WRITE) {
		value="1";
	}
	if (event->mask & IN_DELETE) {
		value="0";
	}

	char *data = malloc(sizeof(char) * (strlen(get_config("http_post.data")) + strlen(dir->name) + strlen(event->name) + strlen(value) + 1));
	sprintf(data, get_config("http_post.data"), dir->name, event->name, value);
	log_msg("DEBUG", "POST %s, data: %s", get_config("http_post.url"), data);
	//	log_msg("INFO", "[%s] %s : %s/%s %s", type, dir->key, dir->name, event->name, isdir);
}
