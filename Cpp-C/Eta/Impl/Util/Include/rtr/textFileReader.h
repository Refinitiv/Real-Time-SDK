/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEXT_FILE_READER_H
#define TEXT_FILE_READER_H

#include <stdio.h>
#include "rtr/os.h"
#include <stdlib.h>
#include <string.h>

/*** Class for reading text files line-by-line. Grows to accomodate long lines. ***/
typedef struct
{
	int allocatedLength;	/* Length allocated for currentLine, usrString, and usrString2 */
	FILE *file;				/* File being read */
	int currentLineLength;	/* Length of currently read line */
	char *currentLine;		/* Current line read from file */
	char *usrString;		/* Temporary string memory for general use by user. Allocated length is same as currentLine */
	char *usrString2;		/* Temporary string memory for general use by user. Allocated length is same as currentLine. */
} TextFileReader;

/*** Interface ***/

/* Initialize file reader. */
static int textFileReaderInit(TextFileReader *pReader, FILE *file, RwfBuffer *errorText);

/* Cleanup file reader memory. */
static void textFileReaderCleanup(TextFileReader *pReader);

/* Read a line from the given file. 
 * Returns:
 *   a positive value, if there is something to read. Call textFileReaderReadLine again to get more.
 *   0, if the end of the file is reached.
 *  -1, if an error occurs. */
static int textFileReaderReadLine(TextFileReader *pReader, RwfBuffer *errorText);

/*** Implementation ***/

static int _textFileReaderReallocateStrings(TextFileReader *pReader, int length, RwfBuffer *errorText)
{
	if ((pReader->currentLine = (char*)realloc((void*)pReader->currentLine, length)) == NULL
			|| (pReader->usrString = (char*)realloc((void*)pReader->usrString, length)) == NULL
			|| (pReader->usrString2 = (char*)realloc((void*)pReader->usrString2, length)) == NULL)
	{
#if (defined WIN32) && (_MSC_VER < 1900)
		_snprintf
#else
		snprintf
#endif
			(errorText->data, errorText->length, "Failed to allocate memory for line parsing");
		return -1;
	}

	pReader->allocatedLength = length;
	return 0;
}

static int textFileReaderInit(TextFileReader *pReader, FILE *file, RwfBuffer *errorText)
{
	memset(pReader, 0, sizeof(TextFileReader));
	pReader->file = file;
	return _textFileReaderReallocateStrings(pReader, 256, errorText);
}

static void textFileReaderCleanup(TextFileReader *pReader)
{
	if (pReader->currentLine != NULL) free(pReader->currentLine);
	if (pReader->usrString != NULL) free(pReader->usrString);
	if (pReader->usrString2 != NULL) free(pReader->usrString2);
}

static int textFileReaderReadLine(TextFileReader *pReader, RwfBuffer *errorText)
{
	int ret;
	pReader->currentLineLength = 0;
	while (fgets(pReader->currentLine + pReader->currentLineLength, pReader->allocatedLength - pReader->currentLineLength, pReader->file) != NULL)
	{
		pReader->currentLineLength = (int)strlen(pReader->currentLine);

		/* We should have an end-of-line. Replace it with a terminator and return. */
		if (pReader->currentLineLength > 0 && pReader->currentLine[pReader->currentLineLength-1] == '\n')
		{
			pReader->currentLineLength -= 1;
			pReader->currentLine[pReader->currentLineLength] = '\0';

			/* If a carriage return precedes the newline, take that off too. */
			if (pReader->currentLineLength > 1 && pReader->currentLine[pReader->currentLineLength-1] == '\r')
			{
				pReader->currentLineLength -= 1;
				pReader->currentLine[pReader->currentLineLength] = '\0';
			}

			return 1;
		}
		else if (pReader->currentLineLength == pReader->allocatedLength - 1)
		{
			/* If no newline, we may not have reached the end of the line. Reallocate and read remainder of line. */
			if ((ret = _textFileReaderReallocateStrings(pReader, pReader->allocatedLength * 2, errorText)) != 0)
				return ret;
		}
		else
			return pReader->currentLineLength;
	}

	/* Stop at end of file */
	return pReader->currentLineLength;
}


#endif

