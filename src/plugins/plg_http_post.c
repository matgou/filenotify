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


static void *curl_handle;
static int curl_init;
static CURL *curl;
static CURL *(*f_init)(void) = NULL;
static CURLcode (*f_setopt)(CURL *, CURLoption, ...) = NULL;
static CURLcode (*f_perform)(CURL *) = NULL;
const char *(*f_strerror)(CURLcode errornum);
static void (*f_cleanup)(CURL *) = NULL;

/**
 * \fn void terminate_plugin()
 * \brief free alloc mem
 */
void terminate_plugin()
{
        nlist_free(config);
}

/**
 * \fn void init_plugin()
 * \brief initialise un Plugins
 */
void
init_plugin(char *p_name, struct nlist *config_ref)
{
    char *error;
    config = nlist_dup(config_ref);
    curl_init=1;

    curl_handle = dlopen ("libcurl.so.3", RTLD_LAZY);
    if(curl_handle == NULL) {
        curl_handle = dlopen ("libcurl.so.4", RTLD_LAZY);
        if(curl_handle != NULL) {
            log_msg("DEBUG", "plg_http_post(%s) use library libcurl.so.4", p_name);
        } else {
            log_msg("ERROR", "plg_http_post(%s) : unable to open a libcurl.so : %s", p_name, dlerror());
	}
    } else {
        log_msg("DEBUG", "plg_http_post(%s) use library libcurl.so.3", p_name);
    }

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
}

/**
 * \fn void handle_event()
 * \brief Write log from received event
 */
void handle_event(char *p_name, struct directory *dir, const struct inotify_event *event)
{

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

	/* get a curl handle */
	curl = (*f_init)();
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
	        if (event->mask & IN_MOVE_SELF) {
	                value="1";
	        }
	        if (event->mask & IN_MOVED_FROM) {
	                value="0";
	        }
	        if (event->mask & IN_MOVED_TO) {
	                value="1";
		}



		if (event->len) {
			data = malloc(sizeof(char) * (strlen(get_config(config_data)) + strlen(dir->name) + strlen(event->name) + strlen(value) + 1));
			sprintf(data, get_config(config_data), dir->name, event->name, value);
		} else {
			data = malloc(sizeof(char) * (strlen(get_config(config_data)) + strlen(event->name) + strlen(value) + 1));
			sprintf(data, get_config(config_data), dir->name, "", value);
		}

		log_msg("DEBUG", "POST %s, data: %s", get_config(config_url), data);

		(*f_setopt)(curl, CURLOPT_URL, get_config(config_url));
		/* Now specify the POST data */
		(*f_setopt)(curl, CURLOPT_POSTFIELDS, data);
		/* allow whatever auth the server speaks */
		if(get_config(config_auth) != NULL) {
			(*f_setopt)(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
			(*f_setopt)(curl, CURLOPT_USERPWD, get_config(config_auth));
		}
		//	log_msg("INFO", "[%s] %s : %s/%s %s", type, dir->key, dir->name, event->name, isdir);

		/* Perform the request, res will get the return code */
		res = (*f_perform)(curl);
		/* Check for errors */
		if(res != CURLE_OK) {
			log_msg("ERROR", "curl_easy_perform() failed: %s", (*f_strerror)(res));
		}
		/* always cleanup */
		free(config_url);
		free(config_data);
		free(config_auth);
		(*f_cleanup)(curl);
	}
}
