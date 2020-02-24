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
 * \version 0.1
 *
 * One file by parameter
 */
#include <config.h>
#include <log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct nlist *
save_config(char *key, char* value)
{
	return install(config, key, value);
}

/**
 * \fn get_config()
 * \brief return an element in config
 * \return char* value from key
 */
char *
get_config(char *key)
{
	struct nlist *ptr = lookup(config, key);
	if(ptr != NULL)
	{
		return ptr->defn;
	}
	return NULL;
}

/**
 * \fn void loadConfig()
 * \brief display all config in memory
 */
void
display_allconfig(struct nlist *list[])
{
	struct nlist *np;
	for (int i = 0; i < HASHSIZE; i++)
	{
		for (np = list[i]; np != NULL; np = np->next)
		{
			if(np != NULL) {
				log_msg("INFO", " Config : %s=%s ", np->name, np->defn);
			}
		}
	}
}

/**
 * \fn void get_configs()
 * \brief return list of config who contains prefix
 */
struct nlist **
get_configs(struct nlist *list[], char *prefix)
{
	static struct nlist *configs[HASHSIZE];
	//configs  = malloc(HASHS!IZE * sizeof(struct nlist *));
	struct nlist *np;

	for (int i = 0; i < HASHSIZE; i++)
	{
		for (np = list[i]; np != NULL; np = np->next)
		{
			if(strncmp(np->name, prefix, strlen(prefix)) == 0) {
				char *new_name=np->name + sizeof(char)*strlen(prefix);
				install(configs, new_name, np->defn);
			}
        }
	}

	return configs;
}

/**
 * \fn int loadConfig()
 * \brief load all config file in config structure
 * \return 1 in case of success
 */
int
loadConfig (char *configFilePath)
{
	FILE *configFile = NULL;
	configFile = fopen(configFilePath,  "r");
	char configLine[TAILLE_MAX] = "";

	if (configFile == NULL) {
		log_msg("ERROR", "Failed to open config file : %s", configFilePath);
		return 255;
	}

	while (fgets(configLine, TAILLE_MAX, configFile) != NULL)
	{
		if(configLine[0]=='#')
		{
			continue;
		}
		char end=configLine[strlen(configLine)-1];
		if(end == '\n') {
			configLine[strlen(configLine)-1] = '\0';
		}
		char *value = strchr(configLine, '=');
		if(value != NULL)
		{
			value[0]='\0';
			// on enleve le =
			value = value+1;
			save_config(configLine, value);
		}
	}

	fclose(configFile);

	return 0;
}
