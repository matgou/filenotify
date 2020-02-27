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
#include <stdio.h>
#include <stdlib.h>
#include <nlist.h>
#include <string.h>

/**
 * \fn unsigned lookup()
 * \brief look for a string in nlist
 * \return list element
 */
struct nlist *lookup(struct nlist *list, char *s)
{
	struct nlist *list_it = list;
	for(list_it = list; list_it != NULL; list_it = list_it->next) {
		if(list_it->name == 0) {
			continue;
		}
		if (strcmp(s, list_it->name) == 0) {
			return list_it; /* found */
		}
	}

	return NULL; /* not found */
}

/**
 * \fn nlist install()
 * \brief install: put (name, defn) in hashtab
 * \return
 */
struct nlist *install(struct nlist *list, char *name, char *defn)
{
	struct nlist *np = NULL;
	if ((np = lookup(list, name)) == NULL) { /* not found */
		np = (struct nlist *) malloc(sizeof(struct nlist));
		np->next = list;
        if (np == NULL || (np->name = strdup(name)) == NULL) {
			return NULL;
		}
		if ((np->defn = strdup(defn)) == NULL) {
			return NULL;
		}
		/* replace list main object */

		list = np;
	} else { /* already there */
        	free((void *) np->defn); /*free previous defn */
		if ((np->defn = strdup(defn)) == NULL) {
			return NULL;
		}
	}

	return np;
}

/**
 * \fn void free_nlist()
 * \brief free a nlist object
 */
void free_nlist(struct nlist *l) {
	// recursive
	if(l->next != NULL) {
		free_nlist(l->next);
	}
	// free object
	free(l->name);
	free(l->defn);
	free(l);
}
