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
 * \file log.c
 * \brief Function to print string in logfile
 * \author Goulin.M
 * \version 1.0
 */
#include <log.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

// For thread safe
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * \fn int log_isleveldisplay(char *tag)
 * \brief Check if loglevel in config let display the TAG in parameters
 * \param tag the loglevel to check
 * \return 0 if no display, 1 if display
 */
int
log_isleveldisplay(char *tag)
{
	if(config_getbykey("loglevel") == NULL) {
		printf("[NOLOG] ");
		return 1;
	}
	if(strcmp("DEBUG", config_getbykey("loglevel"))==0) {
		return 1;
	}
	if(strcmp("INFO", config_getbykey("loglevel")) == 0) {
		if(strcmp(tag, "DEBUG")==0) {
			return 0;
		} else {
			return 1;
		}
	}
	if(strcmp("ERROR", tag)==0) {
		return 1;
	}
	return 0;
}

/**
 * \fn int log_msg(char *tag, char* msg, ...)
 * \brief Display line on stdout and save it in logfile
 * \param tag the level of log DEBUG, INFO, ERROR
 * \param msg string representing the format of message (can contain %s, %i...)
 * \param ... argument to the message
 * \return 0 in case of success
 */
int
log_msg(char *tag, char* msg, ...)
{
	// Check if log must be display
	if(!log_isleveldisplay(tag)) {
		return 0;
	}

	/* lock mutex */
	pthread_mutex_lock(&log_mutex);

	// Parameters : 
	const char *separator=" : ";
	// Get curtime and put in in string
	time_t curtime = time(0);
	char *timeString = malloc(sizeof(char) * strlen(ctime(&curtime)) + 1 );
	sprintf(timeString, "%s", ctime(&curtime));
	
	// remove ending \n
	timeString[strlen(timeString)-1]='\0';

	// Calculate the full message with concat tag, timestring, separator, message, end
	int message_len = strlen(msg) + 2 + strlen(tag) + strlen(timeString) + strlen(separator)*2 ;
	char *format = (char *) malloc(message_len * sizeof(char));
	sprintf(format, "%s%s%s%s%s\n", timeString, separator, tag, separator, msg);

	// use va_list to use many args
  	va_list args;
	va_start (args, msg);
	
	// print to stdout
	vfprintf(stdout, format, args );
	va_end (args);

	// print to a logfile
	va_start (args, msg);
	if(logFilePointer == NULL)
	{
		logFilePointer=fopen(config_getbykey("logfile"), "a+");
	}
	if(logFilePointer != NULL)
	{
		vfprintf(logFilePointer, format, args );
	}
	fflush(logFilePointer);
	va_end (args);

	// free alloc format
	free(format);
	free(timeString);
	// unlock mutex
	pthread_mutex_unlock(&log_mutex);

	// return 0
	return 0;
}
