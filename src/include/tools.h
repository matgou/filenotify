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
#ifndef tools_h
#define tools_h
#include <sys/stat.h>
#include <nlist.h>

char *tools_ctime_from_stat(struct stat *fstat);
const char *tools_str_from_mask(uint32_t mask);
const char *tools_value_str_from_mask(uint32_t mask);
char *tools_str_replace(char *orig, char *rep, char *with);
char *tools_str_from_template(char *template_str, nlist_t * args);
nlist_t *tools_nlist_from_plugin_arg(plugin_arg_t * event);
#endif
