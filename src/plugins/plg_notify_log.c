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
#include <filenotify.h>
#include <plg_notify.h>
#include <stdlib.h>
#include <config.h>
#include <log.h>
#include <tools.h>

/**
 * \fn void init_plugin()
 * \brief Initialise the plugin
 */
void
init_plugin (char *p_name, nlist_t * config_ref)
{
  config = nlist_dup (config_ref);
  log_msg ("DEBUG", "Init plugins : plg_notify_log(%s)", p_name);
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
  
  // format string from template
  char *template = "[{{ nom_plugin }}][{{ event_type }}] {{ directory_id }} : {{ dirname }}/{{ filename }} (ctime={{ file_ctime }})";
  char *msg = tools_str_from_template(template, log_args);
  
  // print
  log_msg ("INFO", msg);
  
  // free
  nlist_free(log_args);
  free(msg);
}
