/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include "rtr/ripchttp.h"
#include "rtr/ripcutils.h"
#include "rtr/ripcflip.h"
#ifndef _WIN32
#include <errno.h>
#include <string.h>
#endif

#include "rtr/rsslErrors.h"

static char CHAR_CR     = '\r';
static char CHAR_LF     = '\n';

extern void (*ripcDumpInFunc)(char *buf, size_t len, RsslUInt64 opaque);
extern void(*ripcDumpOutFunc)(char *buf, size_t len, RsslUInt64 opaque);
extern void(*ripcDumpTextFunc)(char *text, RsslUInt64 opaque);

/*-------------------------------------------------------------------
 * iseof
 *
 * Returns number of characters if end-of-line is detected at the
 * specified offset. According to HTTP specs Web servers should
 * recognize both LF and CR-LF as the end-of-line.
 * Returns 0 if there is no end-of-line at offset.
 *-----------------------------------------------------------------*/
static RsslInt32 iseof(char *data, RsslInt32 offset, RsslInt32 datalen)
{
    if (data[offset] == CHAR_CR)
    {
        if (offset >= datalen-1) return 1;
        if (data[offset+1] == CHAR_LF) return 2;
        return 1;
    }
    if (data[offset] == CHAR_LF)
    {
        return 1;
    }
    return 0;
}

RsslInt32 ripcHttpHdrToUpper(char* data, RsslInt32 datalen, RsslInt32 startOffset)
{
	RsslInt32 i = startOffset, eoflen, slen = 0;

	if (datalen == 0) return 0;

	while (i < datalen)
	{
		eoflen = iseof(data, i, datalen);
		if (eoflen == 0)
		{
			slen++;
			i++;
			data[i] = toupper((RsslInt32)data[i]);
			continue;
		}

		/* we did hit eof, let's see what's here */

		if (slen == 0) /* empty line designates end of http headers */
		{
			return i + eoflen;
		}

		/* else we just passed yet another line */
		i += eoflen;
		slen = 0;
	}

	/* here we are if not yet complete */
	return 0;
}

/*------------------------------------------------------------------
 * ipcHttpHdrComplete
 *
 * This checks if HTTP request headers are completely received, i.e.
 * if there is an empty line at the end of headers. If so, returns
 * total length of the headers (in bytes) otherwise returns 0.
 *
 * startOffset is necessary because as we read headers piece by piece
 * we dont want to check them every time from the very beginning.
 *----------------------------------------------------------------*/
RsslInt32 ipcHttpHdrComplete(char *data, RsslInt32 datalen, RsslInt32 startOffset)
{
	RsslInt32 i = startOffset, eoflen, slen = 0;
    if (datalen == 0) return 0;

    while(startOffset > 0 && iseof(data,startOffset,datalen) > 0)
        startOffset--;
    
    i = startOffset;

    while(i < datalen)
    {
        eoflen = iseof(data,i,datalen);
        if (eoflen == 0)
        {
            slen++;
            i++;
            continue;
        }
        
        /* we did hit eof, let's see what's here */
        
        if (slen == 0) /* empty line designates end of http headers */
        {
            return i+eoflen;
        }
        
        /* else we just passed yet another line */
        i += eoflen;
        slen = 0;
    }
    
    /* here we are if not yet complete */
    return 0;
}

static RsslInt32 URLdecode(char *encoded, RsslInt32 length, char *pDecode)
{
	char *pDecBeg;
	char *pEncode;
	char *pEnd=encoded+length;
	RsslInt32 Hi;
	RsslInt32 Lo;
	RsslInt32 Result;

	pDecBeg=pDecode;
	pEncode=encoded;

	while (pEncode < pEnd)
	{
		if (*pEncode == '+')
		{
			*pDecode++ = ' ';
			pEncode++;
		}
		else if (*pEncode == '%')
		{
			pEncode++;
			if (isxdigit(pEncode[0]) && (isxdigit(pEncode[1])))
			{
				Hi = pEncode[0];
				if ('0' <= Hi && Hi <= '9')
					Hi -= '0';
				else if ('a' <= Hi && Hi <= 'f')
					Hi -= ('a'-10);
				else if ('A' <= Hi && Hi <= 'F')
					Hi -= ('A'-10);

				Lo = pEncode[1];
				if ('0' <= Lo && Lo <= '9')
					Lo -= '0';
				else if ('a' <= Lo && Lo <= 'f')
					Lo -= ('a'-10);
				else if ('A' <= Lo && Lo <= 'F')
					Lo -= ('A'-10);

				Result = Lo + (16 * Hi);
				*pDecode++ = (char)Result;
				pEncode += 2;
			}
			else
				return(-1);
		}
		else
		{
			*pDecode++ = *pEncode;
			pEncode++;
		}
	}
	return (RsslInt32)(pDecode - pDecBeg);
}


RsslInt32 ipcGetHttpConnHeader(char *httpBuf, RsslInt32 length, RsslUInt16 headerSize, char *url, RsslUInt32 id1, RsslUInt32 id2)
{
	RsslInt32 httpHeader;	

	if (url)
	{
		if ((RsslInt32)(strlen(url) + 150 + headerSize) > length)
			return 0;
		httpHeader = snprintf(httpBuf, (size_t)length, "POST %s HTTP/1.0\r\n", url);
	} else
        httpHeader = snprintf(httpBuf, (size_t)length, "POST / HTTP/1.0\r\n");
	
	httpHeader += snprintf(httpBuf + httpHeader, (size_t)(length-httpHeader), "Pragma: no-cache\r\n");
	httpHeader += snprintf(httpBuf + httpHeader, (size_t)(length-httpHeader), "Accept: image/gif, image/x-xbitmao, image/jpeg, image/pjpeg, image/png, */*\r\n");
	httpHeader += snprintf(httpBuf + httpHeader, (size_t)(length-httpHeader), "Accept-Encoding: gzip\r\n");
	httpHeader += snprintf(httpBuf + httpHeader, (size_t)(length-httpHeader), "Accept-Language: en\r\n");
	httpHeader += snprintf(httpBuf + httpHeader, (size_t)(length-httpHeader), "Accept-Charset: iso-8859-1,*,utf-8\r\n");
	httpHeader += snprintf(httpBuf + httpHeader, (size_t)(length-httpHeader), "Content-Length: %u\r\n", (headerSize+10));
	httpHeader += snprintf(httpBuf + httpHeader, (size_t)(length-httpHeader), "Proxy-Connecion: Keep-Alive\r\n\r\n");

	move_u32((httpBuf+httpHeader), &id1);
	httpHeader += 4;
	move_u32((httpBuf+httpHeader), &id2);
	httpHeader += 4;
	_move_u16_swap((httpBuf+httpHeader),&headerSize); /* Offset for id's */
	httpHeader += 2;   /* for header lenght which we will include when we know it */

	return httpHeader;
}

