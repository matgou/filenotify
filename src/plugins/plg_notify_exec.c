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
#include <filenotify.h>
#include <plg_notify.h>
#include <config.h>
#include <log.h>
#include <tools.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * \fn void init_plugin()
 * \brief Initialise the plugin
 */
void
init_plugin (char *p_name, nlist_t * config_ref)
{
  config = nlist_dup (config_ref);
  log_msg ("DEBUG", "Init plugins : plg_notify_exec(%s)", p_name);
}

/**
 * \fn void terminate_plugin()
 * \brief free alloc mem
 */
void
terminate_plugin ()
{
  if (config != NULL)
    {
      nlist_free (config);
      config = NULL;
    }
}


/**
 * \fn void handle_event()
 * \brief Write log from received event
 */
void
handle_event (char *p_name, plugin_arg_t *event)
{
  // build args
  nlist_t *log_args = tools_nlist_from_plugin_arg(event);
  log_args = install(log_args, "{{ nom_plugin }}", p_name);
  
  directory_t * dir = event->dir;
  char *extra_post_data_config = "";

  if (event->event_mask & IN_ISDIR)
    {
      return;
    }

  /* Concat p_name and .cmd to build a config_cmd */
  int config_cmd_len = strlen (".cmd") + strlen (p_name) + 1;
  char *config_cmd = malloc (sizeof (char) * config_cmd_len);
  sprintf (config_cmd, "%s.cmd", p_name);

  if (config_getbykey (config_cmd) == NULL)
    {
      log_msg ("ERROR", "Unable to find '%s' key in config file", config_cmd);
      return;
    }

  /* Concat dir->key and .extra_post_data to build extra_post_data */
  int extra_post_data_len =
    strlen ("watch_directory.") + strlen (".extra_post_data") +
    strlen (dir->key) + 1;
  char *extra_post_data = malloc (sizeof (char) * extra_post_data_len);
  sprintf (extra_post_data, "watch_directory.%s.extra_post_data", dir->key);
  log_msg ("DEBUG", "extra_post_data=%s", extra_post_data);

  if (config_getbykey (extra_post_data))
    {
      extra_post_data_config = config_getbykey (extra_post_data);
    }
  else
    {
      extra_post_data_config = "";
    }
  log_args = install(log_args, "{{ extra_post_data }}", extra_post_data_config);

  log_msg ("DEBUG", "handle - plg_notify_exec");
  // build from template
  char *cmd = tools_str_from_template(config_getbykey (config_cmd), log_args);
  log_msg ("DEBUG", "Execute cmd: %s", cmd);
  
  // exec system
  int status = -99;
  status = system (cmd);

  // free
  log_msg ("DEBUG", "Cmd return status = %i", status);
  nlist_free(log_args);
  free (extra_post_data);
  free (config_cmd);
  free (cmd);
  //      log_msg("INFO", "[%s] %s : %s/%s %s", type, dir->key, dir->name, filename, isdir);
}
