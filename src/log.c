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
 * \brief Function to log
 * \author Goulin.M
 * \version 0.1
 *
 * One file by parameter
 */
#include <log.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>


/**
 * \fn int displayLog()
 * \brief verify if loglevel is more than tag level
 */
int
displayLog(char *tag)
{
	return 0;
	if(strcmp("DEBUG", get_config("loglevel"))==0) {
		return 1;
	}
	if(strcmp("INFO", get_config("loglevel")) == 0) {
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
 * \fn int log_msg()
 * \brief Display line on stdout and logfile
 * \return 0 in case of success
 */
int
log_msg(char *tag, char* msg, ...)
{
	if(!displayLog(tag)) {
		return 0;
	}
	char *separator=" : ";
	char *end="\n";

	time_t curtime = time(0);
	char *timeString=ctime(&curtime);
	timeString[strlen(timeString)-1]='\0';
	int message_len = strlen(msg) + 1 + strlen(tag) + strlen(timeString) + strlen(separator)*2 + strlen(end);
	char *format = (char*) malloc(message_len * sizeof(char));
	
	strcpy(format, timeString);
	strcat(format, separator);
	strcat(format, tag);
	strcat(format, separator);
	strcat(format, msg);
	strcat(format, end);
  	va_list args;
	va_start (args, msg);
	
	vfprintf(stdout, format, args );
	va_end (args);
	va_start (args, msg);

	if(logFilePointer == NULL)
	{
		logFilePointer=fopen(get_config("logfile"), "a+");
	}
	if(logFilePointer != NULL)
	{
		vfprintf(logFilePointer, format, args );
	}
	
	va_end (args);
	free(format);
	return 0;
}
