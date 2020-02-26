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
#include <curl/curl.h>


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

	CURL *curl;
	CURLcode res;

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {
		log_msg("DEBUG", "handle - plg_http_post");
		char *value;
		char *data;
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

		if (event->len) {
			data = malloc(sizeof(char) * (strlen(get_config("http_post.data")) + strlen(dir->name) + strlen(event->name) + strlen(value) + 1));
			sprintf(data, get_config("http_post.data"), dir->name, event->name, value);
		} else {
			data = malloc(sizeof(char) * (strlen(get_config("http_post.data")) + strlen(event->name) + strlen(value) + 1));
			sprintf(data, get_config("http_post.data"), dir->name, "", value);
		}

		log_msg("DEBUG", "POST %s, data: %s", get_config("http_post.url"), data);

		curl_easy_setopt(curl, CURLOPT_URL, get_config("http_post.url"));
		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		/* allow whatever auth the server speaks */
		if(get_config("http_post.auth") != NULL) {
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
			curl_easy_setopt(curl, CURLOPT_USERPWD, get_config("http_post.auth"));
		}
		//	log_msg("INFO", "[%s] %s : %s/%s %s", type, dir->key, dir->name, event->name, isdir);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK) {
			log_msg("ERROR", "curl_easy_perform() failed: %s", curl_easy_strerror(res));
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}
