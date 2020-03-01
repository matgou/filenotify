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
 * \fn nlist_t *nlist_dup()
 * \brief duplicate a nlist object
 * \return fist list element
 */
nlist_t *nlist_dup(nlist_t *list)
{
	if(list != NULL) {
		// Duplicate list element
		nlist_t *l = malloc(sizeof(nlist_t));
		l->name = strdup(list->name);
		l->defn = strdup(list->defn);
		l->next = nlist_dup(list->next);	
		return l;
	}
	
	// If list is NULL
	return NULL;
}

/**
 * \fn unsigned lookup()
 * \brief look for a string in nlist
 * \return list element
 */
nlist_t *lookup(nlist_t *list, char *s)
{
	nlist_t *list_it = list;
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
nlist_t *install(nlist_t *list, char *name, char *defn)
{
	nlist_t *np = NULL;
	if ((np = lookup(list, name)) == NULL) { /* not found */
		np = (nlist_t *) malloc(sizeof(nlist_t));
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
 * \fn void nlist_free()
 * \brief free a nlist object
 */
void nlist_free(nlist_t *l) {
	// recursive
	if(l != NULL) {
		if(l->next != NULL) {
			nlist_free(l->next);
			l->next=NULL;
		}
		// free object
		free((void *) l->defn);
		free((void *) l->name);
		free(l);
	}
}
