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
#include <filenotify.h>
#include <plg_notify.h>
#include <nlist.h>
#include <config.h>
#include <log.h>
#include <tools.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <clamav.h>
#include <string.h>
#include <errno.h>

// Define some variables
static void *clamav_handle;
static int clamav_init = 0;
static struct cl_engine *clamav_engine;
// clamav function pointer
static int *(*f_cl_init)(unsigned int options) = NULL;
static struct cl_engine *(*f_cl_engine_new)(void) = NULL;
static int (*f_cl_engine_free)(struct cl_engine *engine) = NULL;
static int (*f_cl_load)(const char *path, struct cl_engine *engine,
            unsigned int *signo, unsigned int options) = NULL;
extern const char *(*f_cl_strerror)(int ret) = NULL;
int (*f_cl_engine_compile)(struct cl_engine *engine) = NULL;
int (*f_cl_scanfile)(const char *filename, const char **virname, unsigned long int *scanned, const struct cl_engine *engine, struct cl_scan_options *options) = NULL;


/**
 * \fn void terminate_plugin()
 * \brief free alloc mem by the plugins
 */
void terminate_plugin()
{
    (*f_cl_engine_free) (clamav_engine);

    // do not free config if it's alredy free
    if (config != NULL) {
	nlist_free(config);
	config = NULL;
    }
}

void *plg_clamav_extern_fnc_load(char *fnc_name)
{
	char *error;
	void *ptr = dlsym(clamav_handle, fnc_name);
	if ((error = dlerror()) != NULL) {
		clamav_init = 0;
		log_msg("ERROR", "plg_clamav(%s) when load %s : %s", fnc_name, error);
		return NULL;
	}
	return ptr;
}

void plg_clamav_lib_load(char *p_name)
{
    // Try to load libclamav.so.3
    clamav_handle = dlopen("libclamav.so", RTLD_LAZY);
    if (clamav_handle == NULL) {
	log_msg("ERROR", "plg_clamav(%s) : unable to open a libclamav.so.3 : %s", p_name, dlerror());
	clamav_init = 0;
	return;
    } else {
        f_cl_init = plg_clamav_extern_fnc_load("cl_init");
	f_cl_engine_new = plg_clamav_extern_fnc_load("cl_engine_new");
        f_cl_engine_free = plg_clamav_extern_fnc_load("cl_engine_free");
        f_cl_strerror = plg_clamav_extern_fnc_load("cl_strerror");
        f_cl_load = plg_clamav_extern_fnc_load("cl_load");
	f_cl_engine_compile = plg_clamav_extern_fnc_load("cl_engine_compile");
	f_cl_scanfile = plg_clamav_extern_fnc_load("cl_scanfile");
    }
}

/**
 * \fn void init_plugin(char *p_name, nlist_t *config_ref)
 * \brief initialise the plugins by charging config and loading clamav as dynamic library
 * \param p_name the plugin name
 * \param config_ref the configuration
 */
void init_plugin(char *p_name, nlist_t * config_ref)
{
    // Duplicate config to keepit in memory
    config = nlist_dup(config_ref);
    unsigned int sigs = 0;
    char *error;
    // In case of error clamav_init will be set to 0
    clamav_init = 1;

    // Try to load libclamav.so
    plg_clamav_lib_load(p_name);
    if (clamav_init != 0) {
	// init clamav api
	if((*f_cl_init) (CL_INIT_DEFAULT) != 0) {
		log_msg("ERROR", "plg_clamav(%s) init error when start cl_init : %s", p_name, strerror(errno));
		clamav_init = 0;
		return;
	}
	// load engine
	clamav_engine = (*f_cl_engine_new) ();
	if(clamav_engine == NULL) {
		log_msg("ERROR", "plg_clamav(%s) failed to init engine cl_engine_new : %s", p_name, strerror(errno));
		clamav_init = 0;
	}

	// load database
	int ret = (*f_cl_load) (config_getbykey("clamav.dbdir"), clamav_engine, &sigs, CL_DB_STDOPT);
	if(ret != CL_SUCCESS) {
        	log_msg("ERROR", "cl_load() error: %s", (*f_cl_strerror)(ret));
        	(*f_cl_engine_free)(clamav_engine);
		clamav_init = 0;
        	return ;
    	}
	// compile engine
    	if((ret = (*f_cl_engine_compile)(clamav_engine)) != CL_SUCCESS) {
        	log_msg("ERROR", "cl_engine_compile() error: %s", (*f_cl_strerror)(ret));
        	(*f_cl_engine_free)(clamav_engine);
		clamav_init = 0;
        	return ;
    	}
    }

}

/**
 * \fn void handle_event(char *p_name, directory_t *dir, char *filename, uint32_t mask)
 * \brief Handle a event and write it by post request in api
 * \param p_name the plugin name use to get config
 * \param dir the directory who emit the event
 * \param event the inotify event
 */
void handle_event(char *p_name, plugin_arg_t * event)
{
    if (event->event_mask & IN_ISDIR) {
	return;
    }
    if (event->event_mask & IN_DELETE) {
        return;
    }
    if (event->event_mask & IN_MOVED_FROM) {
        return;
    }
    if (event->event_mask & IN_CLOSE_NOWRITE) {
  	return; 
    }
    if (clamav_init == 0) {
	return;
    }

    // build args
    nlist_t *log_args = tools_nlist_from_plugin_arg(event);
    log_args = install(log_args, "{{ nom_plugin }}", p_name);
    directory_t *dir = event->dir;

    char *path = tools_str_from_template("{{ dirname }}/{{ filename }}", log_args);

    const char *virname;
    int ret;
    char *error;
    struct cl_scan_options options;

    /* scan file descriptor */
    memset(&options, 0, sizeof(struct cl_scan_options));
    options.parse |= ~0;                           /* enable all parsers */
    options.general |= CL_SCAN_GENERAL_HEURISTICS; /* enable heuristic alert options */
    if((ret = (*f_cl_scanfile)(path, &virname, NULL, clamav_engine, &options)) == CL_VIRUS) {
        log_msg("INFO", "plg_clamav(%s) : Virus detected: %s", p_name, virname);
    } else {
        log_msg("INFO", "plg_clamav(%s) : No virus detected", p_name);
        if(ret != CL_CLEAN) {
            error = (*f_cl_strerror)(ret);
            log_msg("ERROR", "plg_clamav(%s) : error: %s", p_name, error);
	}
    }

    free(path);
    nlist_free(log_args);
}
