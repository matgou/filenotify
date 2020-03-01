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
#ifndef nlist_h
#define nlist_h

/**
 * \struct nlist_t nlist.h
 * \brief A object to store simple key-value list
 */
typedef struct {
    void *next; /*!< Ptr to the next entry of chain. */
    char *name; /*!< key / name, string to identify the name of the parameters */
    char *defn; /*!< The definition value */
} nlist_t;

/* Function list */
nlist_t *nlist_dup(nlist_t *list);
nlist_t *lookup(nlist_t *list, char *s);
nlist_t *install(nlist_t *list, char *name, char *defn);
void nlist_free(nlist_t *l);

#endif
