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
#include <sys/inotify.h>
#include <filenotify.h>
#include <plg_notify.h>
#include <log.h>

/**
 * \fn void handle_event()
 * \brief Write log from received event
 */
static void handle_event(struct directory *dir, struct inotify_event *event)
{
	log_msg("INFO", "Hello from plg_notify_log.c");
}
