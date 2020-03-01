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
#ifndef config_h
#define config_h
#include "nlist.h"

#define CONFIG_DIR "etc/"
#define TAILLE_MAX 1000

nlist_t *config;

nlist_t *config_loadfromfile (char *configFilePath);
void freeconfig ();
nlist_t *save_config(char *key, char* value);
char *config_getbykey(char *key);
void config_displayall(nlist_t *list);
nlist_t *config_getbyprefix(nlist_t *list, char *prefix);

#endif
