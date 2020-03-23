/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */


#include <stdio.h>
//#include <iostream>
#include <string.h>
#include "rtr/rjchexdmp.h"

//using namespace std;
// STLREM: iostream and std namespace removal

int RJCHexDump::buffer_size_needed( int len )
{
	return (((int)(len / 16) + 1) * char_per_line + 10);
};

void RJCHexDump::hex_dump(	const char *ptr, int len,
							const char *put_buf, unsigned int max_buf)
{
	unsigned char	byte;
	char*   cur_position=(char*)put_buf;
	long    cur_cursor = 0;

	outchar[0] = '\0';

	if (!check_size((char*)put_buf,cur_position,max_buf))
		return;

	while (cur_cursor < len)
	{
		byte = *ptr++;
		cur_position = addline(cur_position,byte);
		cur_cursor++;

		if ((cur_cursor % 16) == 0)
		{
			cur_position = startline(cur_position,0);
			if (!check_size((char*)put_buf,cur_position,max_buf))
				return;
		}
	}
	if ((cur_cursor % 16) != 0)
		cur_position = startline(cur_position,cur_cursor);

	*(char*)(cur_position) = '\0';

		/* If we have gone over a boundary, write this. */
	if (cur_position >= (put_buf + max_buf))
	{
		//cout<<"Hex dump has gone over a boundary. Memory";
		//cout<<"is most likely been corrupted."<<endl;
		// STLREM: replaced cout with printf
		printf("Hex dump has gone over a boundry. Memory has most likely been corrupted.\n");
	}
};

short RJCHexDump::check_size(	char* start,
								char* current,
								unsigned int max)
{
	unsigned long cur_chars;

	cur_chars = (unsigned long)(current - start);
	if ((cur_chars + (unsigned int) char_per_line) > max)
	{
		if ((cur_chars + 4) <= max)
		{
			sprintf((char*)current,"INC");
			return 0;
		}
	}
	return 1;
};

char* RJCHexDump::startline(char* loc,
									unsigned long cursor )
{
	short left;

	if ((cursor > 0)&&((left = (short)(cursor % 16)) != 0))
	{
		if (eobyte & 1)
		{
			sprintf((char*)loc,"   ");
			loc += 3;
			left += 1;
		}
		for (;left < 16;left += 2)
		{
			sprintf((char*)loc,"     ");
			loc += 5;
		}
	}
	sprintf((char*)loc,"   %s\n",outchar);
	loc += 20;
	outchar[0] = '\0';
	eobyte = 0;

	return loc;
};

char* RJCHexDump::addline(	char* location,
									unsigned char	byte )
{
	int	t1;
	char* return_value;

	sprintf(location,(eobyte & 1 ? "%2.2x " : "%2.2x"),byte);
	if (eobyte & 1)
		return_value = location + 3;
	else
		return_value = location + 2;

	eobyte ^= 1;

	t1 = (int) strlen(outchar);
	if ( byte >= ' ' && byte < 0x7f)
	{
		outchar[t1++] = byte;
		outchar[t1] = '\0';
	}
	else
	{
		outchar[t1++] = '.';
		outchar[t1] = '\0';
	}

	return return_value;
};


