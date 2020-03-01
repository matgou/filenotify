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
 * \file plg_notify_exec.c
 * \brief Plugin to send to subprocess inotify event
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
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/types.h> 

/**
 * \fn void init_plugin()
 * \brief Initialise the plugin 
 */
void init_plugin(char *p_name, nlist_t *config_ref)
{
        config = nlist_dup(config_ref);
	log_msg("DEBUG", "Init plugins : plg_notify_exec(%s)", p_name);
}

/**
 * \fn void terminate_plugin()
 * \brief free alloc mem
 */
void terminate_plugin()
{
	if(config != NULL) {
	        nlist_free(config);
		config = NULL;
	}
}


/**
 * \fn void handle_event()
 * \brief Write log from received event
 */
void handle_event(char *p_name, directory_t *dir, const struct inotify_event *event)
{
        if (event->mask & IN_ISDIR) {
                return;
        }

        /* Concat p_name and .cmd to build a config_cmd */
        int config_cmd_len = strlen(".cmd") + strlen(p_name) + 1;
        char *config_cmd = malloc(sizeof(char) * config_cmd_len);
        strcpy(config_cmd, p_name);
        strcat(config_cmd, ".cmd");
        config_cmd[config_cmd_len - 1] = '\0';
	if(config_getbykey(config_cmd) == NULL) {
		log_msg("ERROR","Unable to find '%s' key in config file", config_cmd);
		return ;
	}

	log_msg("DEBUG", "handle - plg_notify_exec");
	char *value;
	/* Print event type */
	value="";
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


	char *cmd = malloc(sizeof(char) * (strlen(config_getbykey(config_cmd)) + strlen(dir->name) + strlen(event->name) + strlen(value) + 1));
	sprintf(cmd, config_getbykey(config_cmd), dir->name, event->name, value);
	log_msg("DEBUG", "Execute cmd: %s", cmd);
        char *args[]={"/bin/bash","-c",cmd,NULL};
	int status;
	pid_t pid = fork();
	if (pid == -1){
		log_msg("ERROR", "can't fork, error occured");
		exit(EXIT_FAILURE);
	} else if (pid == 0){
		// child process
		int execpid=execvp(args[0],args);
	        if(execpid < 0)
		{
			log_msg("ERROR", "plg_notify_exec.c: Le retour de exevp est KO");
		}
		exit(EXIT_SUCCESS);
	}
	while (wait(&status) != pid)       /* wait for completion  */
               ;
	log_msg("DEBUG", "Cmd return status = %i", status);
	free(config_cmd);
	free(cmd);
	//	log_msg("INFO", "[%s] %s : %s/%s %s", type, dir->key, dir->name, event->name, isdir);
}
