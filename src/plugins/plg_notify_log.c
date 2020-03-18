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
#include <config.h>
#include <log.h>

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
handle_event (char *p_name, directory_t * dir, char *filename, uint32_t mask)
{
  char *type = "";
  /* Print event type */
  if (mask & IN_OPEN)
    {
      type = "IN_OPEN";
    }
  if (mask & IN_CLOSE_NOWRITE)
    {
      type = "IN_CLOSE_NOWRITE";
    }
  if (mask & IN_CLOSE_WRITE)
    {
      type = "IN_CLOSE_WRITE";
    }
  if (mask & IN_DELETE)
    {
      type = "IN_DELETE";
    }
  if (mask & IN_MOVE_SELF)
    {
      type = "IN_MOVE_SELF";
    }
  if (mask & IN_MOVED_FROM)
    {
      type = "IN_MOVED_FROM";
    }
  if (mask & IN_MOVED_TO)
    {
      type = "IN_MOVED_TO";
    }

  /* Print type of filesystem object */
  log_msg ("INFO", "[%s][%s] %s : %s/%s", p_name, type, dir->key, dir->name,
	   filename);
}
