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
 * \file filenotify.c
 * \brief Programme de surveillance de fichier et de notification
 * \author Goulin.M
 * \version 0.1
 * \date 2019/02/22
 *
 */
#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <filenotify.h>
#include <config.h>
#include <log.h>


/**
 * \fn void displayHelp()
 * \brief Display help page 
 *
 */
void
displayHelp ()
{
	fprintf(stdout, "Usage : filenotify -c [config] \n");
	fprintf(stdout, "Options :\n");
	fprintf(stdout, "  -c [config]                        Configuration file\n");
}

/**
 * \fn void displayWelcome()
 * \brief Function to say the welcome banner
 *
 */
void
displayWelcome ()
{
	log_msg("INFO", " *** Welcome in filenotifier *** ");
}

/**
 * \fn int main ()
 * \brief entry of the filenotify process
 */
int main(int argc, char *argv[])
{
	int c;
	char *configFilePath = NULL;

	while ((c = getopt (argc, argv, "c:")) != -1)
		switch (c)
		{
      			case 'c':
        			configFilePath = optarg;
				break;
			case '?':
				if (optopt == 'c') {
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
					displayHelp();
                                        return 255;
				} else if (isprint (optopt)) {
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				} else {
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
					displayHelp();
					return 255;
				}
		}
	if(loadConfig(configFilePath) != 0) {
		fprintf (stderr, "Critical error while loading config, exit\n");
		return 255;
	}
	
	displayWelcome();
}
