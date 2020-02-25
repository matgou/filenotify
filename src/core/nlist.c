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
 * \fn unsigned hash()
 * \brief form hash value for string s
 * \return hash int
 */
unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

/**
 * \fn unsigned lookup()
 * \brief look for s in hashtab
 * \return list element
 */
struct nlist *lookup(struct nlist *list[], char *s)
{
    struct nlist *np;
    for (np = list[hash(s)]; np != NULL; np = np->next) {
        if (np != NULL) {
            if(np->name == 0) {
                continue;
            }
            if (strcmp(s, np->name) == 0) {
                return np; /* found */
            }
        }
    }
    return NULL; /* not found */
}

/**
 * \fn nlist install(char *name, char *defn)
 * \brief install: put (name, defn) in hashtab
 * \return
 */
struct nlist *install(struct nlist *list[], char *name, char *defn)
{
    struct nlist *np;
    unsigned hashval;
    if ((np = lookup(list, name)) == NULL) { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL)
          return NULL;
        hashval = hash(name);
        np->next = list[hashval];
        list[hashval] = np;
    } else /* already there */
        free((void *) np->defn); /*free previous defn */
    if ((np->defn = strdup(defn)) == NULL)
       return NULL;
    return np;
}
