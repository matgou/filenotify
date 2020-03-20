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
#include <string.h>

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

/**
 * \fn char *tools_value_str_from_mask (uint32_t mask)
 * \brief return string from the event mask
 * \return string
 */
const char *
tools_value_str_from_mask (uint32_t mask)
{
  /* Print event type */
  if (mask & IN_OPEN)
    {
      return "1";
    }
  if (mask & IN_CLOSE_NOWRITE)
    {
      return "1";
    }
  if (mask & IN_CLOSE_WRITE)
    {
      return "1";
    }
  if (mask & IN_DELETE)
    {
      return "0";
    }
  if (mask & IN_MOVE_SELF)
    {
      return "1";
    }
  if (mask & IN_MOVED_FROM)
    {
      return "0";
    }
  if (mask & IN_MOVED_TO)
    {
      return "1";
    }
  return "";
}

/**
 * \fn char *tools_str_replace (char *orig, char *rep, char *with);
 * \brief return string with remplacing *rep by *with
 * \return string
 */
char *tools_str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

/**
 * \fn char *tools_str_from_template (char *template_str, nlist_t *args);
 * \brief return string from the template completed with args
 * \return string
 */
char *tools_str_from_template(char *template_str, nlist_t *args)
{
	nlist_t *e;
	char *return_str = malloc(sizeof(char) * (strlen(template_str) + 1));
    strcpy(return_str, template_str);
	for(e=args; e != NULL; e=e->next) {
		char *save_str=return_str;
		return_str = tools_str_replace(return_str, e->name, e->defn);
		free(save_str);
	}
	
	return return_str;
}

/**
 * \fn char *tools_str_from_template (char *template_str, nlist_t *args);
 * \brief return a nlist_t string of any args ready to build string from template
 * \return nlist_t
 */
nlist_t *tools_nlist_from_plugin_arg(plugin_arg_t *event)
{
  nlist_t *log_args = NULL;
  log_args = install(log_args, "{{ directory_id }}", event->dir->key);
  log_args = install(log_args, "{{ dirname }}", event->dir->name);
  log_args = install(log_args, "{{ filename }}", event->event_filename);
  char *ctime_str = tools_ctime_from_stat(event->event_filestat);
  log_args = install(log_args, "{{ file_ctime }}", ctime_str);
  
  char *type = (char *) tools_str_from_mask(event->event_mask);
  log_args = install(log_args, "{{ event_type }}", type);
  
  free(ctime_str);
  return log_args;
}