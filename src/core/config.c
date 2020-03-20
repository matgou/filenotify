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
 * \file config.c
 * \brief Function to manipulate configuration.
 * \author Goulin.M
 * \version 1.0
 */
#include <config.h>
#include <log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * \fn char *config_getbykey(char *key)
 * \brief return a text-string value of a config parameter by his key, if not found return NULL
 * \param key the key to search in config
 * \return char* value from key
 */
char *config_getbykey(char *key)
{
    nlist_t *ptr = lookup(config, key);
    if (ptr != NULL) {
	return ptr->defn;
    }
    return NULL;
}

/**
 * \fn void config_displayall(nlist_t *list)
 * \brief use log_msg to display (INFO) all key-value from a config-nlist
 * \param list the list-config to display
 */
void config_displayall(nlist_t * list)
{
    nlist_t *np = list;
    for (np = list; np != NULL; np = np->next) {
	log_msg("INFO", " Config : %s=%s ", np->name, np->defn);
    }
}

/**
 * \fn void config_getbyprefix(nlist_t *list, char *prefix)
 * \brief return a new nlist(key-value) from a config, this function filter config from prefix. 
 * \param list the initial list to filter
 * \param prefix the prefix used to filter
 */
nlist_t *config_getbyprefix(nlist_t * list, char *prefix)
{
    nlist_t *configs = NULL;

    nlist_t *np;
    for (np = list; np != NULL; np = np->next) {
	if (strncmp(np->name, prefix, strlen(prefix)) == 0) {
	    // calculate the new name with decal prefix
	    char *new_name = np->name + sizeof(char) * strlen(prefix);
	    configs = install(configs, new_name, np->defn);
	}
    }

    return configs;
}

/**
 * \fn nlist_t *config_loadfromfile (char *config_filepath)
 * \brief Load all config from a file and return a nlist-config
 * \param config_filepath a string to identify config_filepath
 * \return the nlist with value from file
 */
nlist_t *config_loadfromfile(char *config_filepath)
{
    nlist_t *config_ptr = NULL;

    FILE *config_file = NULL;
    config_file = fopen(config_filepath, "r");
    char config_line[TAILLE_MAX] = "";

    if (config_file == NULL) {
	log_msg("ERROR", "Failed to open config file : %s",
		config_filepath);
	return NULL;
    }

    while (fgets(config_line, TAILLE_MAX, config_file) != NULL) {
	if (config_line[0] == '#') {
	    continue;
	}
	char end = config_line[strlen(config_line) - 1];
	if (end == '\n') {
	    config_line[strlen(config_line) - 1] = '\0';
	}
	char *value = strchr(config_line, '=');
	if (value != NULL) {
	    value[0] = '\0';
	    // on enleve le =
	    value = value + 1;
	    config_ptr = install(config_ptr, config_line, value);
	}
    }

    fclose(config_file);

    return config_ptr;
}
