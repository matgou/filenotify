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
#include <nlist.h>
#include <config.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <dlfcn.h>

// Define some curl variables

static void *curl_handle;
static int curl_init = 0;
static CURL *(*f_init)(void) = NULL;
static CURLcode (*f_setopt)(CURL *, CURLoption, ...) = NULL;
static CURLcode (*f_perform)(CURL *) = NULL;
const char *(*f_strerror)(CURLcode errornum);
static void (*f_cleanup)(CURL *) = NULL;

/**
 * \fn void terminate_plugin()
 * \brief free alloc mem by the plugins
 */
void terminate_plugin()
{
	// do not free config if it's alredy free
	if(config != NULL) {
	        nlist_free(config);
		config = NULL;
	}
}

/**
 * \fn void init_plugin(char *p_name, nlist_t *config_ref)
 * \brief initialise the plugins by charging config and loading curl as dynamic library
 * \param p_name the plugin name
 * \param config_ref the configuration
 */
void
init_plugin(char *p_name, nlist_t *config_ref)
{
	// Duplicate config to keepit in memory
	config = nlist_dup(config_ref);
	char *error;
	// In case of error curl_init will be set to 0

	// Try to load libcurl.so.3 or libcurl.so.4
	curl_handle = dlopen ("libcurl.so.3", RTLD_LAZY);
	if(curl_handle == NULL) {
		curl_handle = dlopen ("libcurl.so.4", RTLD_LAZY);
		if(curl_handle != NULL) {
			log_msg("DEBUG", "plg_http_post(%s) use library libcurl.so.4", p_name);
		} else {
			log_msg("ERROR", "plg_http_post(%s) : unable to open a libcurl.so : %s", p_name, dlerror());
			curl_init=0;
		}
	} else {
        	log_msg("DEBUG", "plg_http_post(%s) use library libcurl.so.3", p_name);
	}

	// If curl is load as dynamic library link curl function into ptr
	if(curl_handle != NULL) {
		f_init = dlsym(curl_handle, "curl_easy_init");
		if ((error = dlerror()) != NULL)  {
			log_msg("ERROR", "plg_http_post(%s) when load curl_easy_init : %s", error);
			curl_init=0;
		}

		f_setopt = dlsym(curl_handle, "curl_easy_setopt");
		if ((error = dlerror()) != NULL)  {
			log_msg("ERROR", "plg_http_post(%s) when load curl_easy_setopt : %s", error);
			curl_init=0;
		}

		f_perform = dlsym(curl_handle, "curl_easy_perform");
		if ((error = dlerror()) != NULL)  {
			log_msg("ERROR", "plg_http_post(%s) when load curl_easy_perform : %s", error);
			curl_init=0;
		}

		f_cleanup = dlsym(curl_handle, "curl_easy_cleanup");
		if ((error = dlerror()) != NULL)  {
			log_msg("ERROR", "plg_http_post(%s) when load curl_easy_cleanup : %s", error);
			curl_init=0;
		}
		f_strerror = dlsym(curl_handle, "curl_easy_strerror");
		if ((error = dlerror()) != NULL)  {
			log_msg("ERROR", "plg_http_post(%s) when load curl_easy_cleanup : %s", error);
			curl_init=0;
		}
	}

	curl_init=1;
}

/**
 * \fn void handle_event(char *p_name, directory_t *dir, const struct inotify_event *event)
 * \brief Handle a event and write it by post request in api
 * \param p_name the plugin name use to get config
 * \param dir the directory who emit the event
 * \param event the inotify event
 */
void handle_event(char *p_name, directory_t *dir, const struct inotify_event *event)
{
        char *extra_post_data_config = "";
	int extra_post_data_config_len = 0;
	CURL *curl;

	if (event->mask & IN_ISDIR) {
		return;
	}
	if (curl_init==0) {
		return;
	}
	CURLcode res;

	/* Concat p_name and .url to build a config_url */
	int config_url_len = strlen(".url") + strlen(p_name) + 1;
	char *config_url = malloc(sizeof(char) * config_url_len);
	strcpy(config_url, p_name);
	strcat(config_url, ".url");
	config_url[config_url_len - 1] = '\0';

	/* Concat p_name and .auth to build a config_auth */
	int config_auth_len = strlen(".auth") + strlen(p_name) + 1;
	char *config_auth = malloc(sizeof(char) * config_auth_len);
	strcpy(config_auth, p_name);
	strcat(config_auth, ".auth");
	config_auth[config_auth_len - 1] = '\0';

	/* Concat p_name and .data to build a config_data */
	int config_data_len = strlen(".data") + strlen(p_name) + 1;
	char *config_data = malloc(sizeof(char) * config_data_len);
	strcpy(config_data, p_name);
	strcat(config_data, ".data");
	config_data[config_data_len - 1] = '\0';

	/* Concat dir->key and .extra_post_data to build extra_post_data */
	int extra_post_data_len = strlen("watch_directory.") + strlen(".extra_post_data") + strlen(dir->key) + 1;
	char *extra_post_data = malloc(sizeof(char) * extra_post_data_len);
	strcpy(extra_post_data, "watch_directory.");
	strcat(extra_post_data, dir->key);
	strcat(extra_post_data, ".extra_post_data");
	extra_post_data[extra_post_data_len - 1] = '\0';
	log_msg("DEBUG", "extra_post_data=%s", extra_post_data);

	/* get a curl handle */
	curl = (*f_init)();
	if(curl) {
		log_msg("DEBUG", "handle - plg_http_post");
		char *value="1";
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
		if (event->mask & IN_MOVE_SELF) {
			value="1";
		}
		if (event->mask & IN_MOVED_FROM) {
			value="0";
		}
		if (event->mask & IN_MOVED_TO) {
			value="1";
		}


		if(config_getbykey(extra_post_data)) {
			extra_post_data_config = config_getbykey(extra_post_data);
			extra_post_data_config_len = strlen(extra_post_data_config);
		} else {
			extra_post_data_config = "";
			extra_post_data_config_len = 0;
		}

		if (event->len) {
			data = malloc(sizeof(char) * (extra_post_data_config_len + strlen(config_getbykey(config_data)) + strlen(dir->name) + strlen(event->name) + strlen(value) + 1));
			sprintf(data, config_getbykey(config_data), dir->name, event->name, extra_post_data_config, value);
		} else {
			data = malloc(sizeof(char) * (extra_post_data_config_len + strlen(config_getbykey(config_data)) + strlen(event->name) + strlen(value) + 1));
			sprintf(data, config_getbykey(config_data), dir->name, "", extra_post_data_config, value);
		}

		log_msg("DEBUG", "POST %s, data: %s", config_getbykey(config_url), data);

		(*f_setopt)(curl, CURLOPT_URL, config_getbykey(config_url));
		/* Now specify the POST data */
		(*f_setopt)(curl, CURLOPT_POSTFIELDS, data);
		/* allow whatever auth the server speaks */
		if(config_getbykey(config_auth) != NULL) {
			(*f_setopt)(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
			(*f_setopt)(curl, CURLOPT_USERPWD, config_getbykey(config_auth));
		}
		//	log_msg("INFO", "[%s] %s : %s/%s %s", type, dir->key, dir->name, event->name, isdir);

		/* Perform the request, res will get the return code */
		res = (*f_perform)(curl);
		/* Check for errors */
		if(res != CURLE_OK) {
			log_msg("ERROR", "curl_easy_perform() failed: %s", (*f_strerror)(res));
		}
		/* always cleanup */
		free(extra_post_data);
		free(config_url);
		free(config_data);
		free(config_auth);
		(*f_cleanup)(curl);
	}
}
