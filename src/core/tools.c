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
#include <stdlib.h>
#include <sys/stat.h>
#include <filenotify.h>
#include <tools.h>
#include <stdio.h>

/**
 * \fn char *tools_ctime_from_stat (nlist_t * list)
 * \brief return string from stat represent ctime timestamp
 * \return string
 */
char *
tools_ctime_from_stat (struct stat *fstat)
{
  char *ctime_str;

  if(fstat != NULL) {
      ctime_str=malloc(sizeof(char) * 255);
      sprintf(ctime_str, "%i", fstat->st_ctime);
  } else {
      ctime_str=malloc(1);
      ctime_str[0]='\0';
  }

  return ctime_str;
}

/**
 * \fn char *tools_str_from_mask (uint32_t mask)
 * \brief return string from the event mask
 * \return string
 */
const char *
tools_str_from_mask (uint32_t mask)
{
  /* Print event type */
  if (mask & IN_OPEN)
    {
      return "IN_OPEN";
    }
  if (mask & IN_CLOSE_NOWRITE)
    {
      return "IN_CLOSE_NOWRITE";
    }
  if (mask & IN_CLOSE_WRITE)
    {
      return "IN_CLOSE_WRITE";
    }
  if (mask & IN_DELETE)
    {
      return "IN_DELETE";
    }
  if (mask & IN_MOVE_SELF)
    {
      return "IN_MOVE_SELF";
    }
  if (mask & IN_MOVED_FROM)
    {
      return "IN_MOVED_FROM";
    }
  if (mask & IN_MOVED_TO)
    {
      return "IN_MOVED_TO";
    }
}
