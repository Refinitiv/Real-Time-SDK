/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This decodes real-time data for the EDF example application.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "rsslRealTimeDataDecoder.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"

static void displayHexData(int length, const char* buffer); // for debugging

void decodeRealTimeData(RsslChannel* chnl, RsslDecodeIterator *dIter)
{
	RsslRet ret = 0;
	RsslMsg msg = RSSL_INIT_MSG;
	
	ret = rsslDecodeMsg(dIter, &msg);				
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nrsslDecodeMsg(): Error %d on SessionData fd=%d\n", ret, chnl->socketId);
		exit(-1);
	}

	switch ( msg.msgBase.domainType )
	{
		case RSSL_DMT_MARKET_PRICE:
			if (processMarketPriceResponse(&msg, dIter) != RSSL_RET_SUCCESS)
				exit(-1);
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			if (processMarketByOrderResponse(&msg, dIter) != RSSL_RET_SUCCESS)
				exit(-1);
			break;
		default:
			printf("Unhandled Domain Type: %d\n", msg.msgBase.domainType);
			break;
	}
}

#define HEX_LINE_SIZE 16
static void displayHexData(int length, const char* buffer)
{
	int line_num = (length + HEX_LINE_SIZE -1) / HEX_LINE_SIZE;

	int i = 0;
	int k = 0;
	int byte = 0;
	int line = 0;

	for (line = 0; line < line_num && i < length; line++)
	{
		printf("%4.4X: ", i);

		k = i;
		byte = 0;
	
		for (; byte < HEX_LINE_SIZE && i < length; byte++)
		{
			printf("%2.2X ", buffer[i++] & 0xFF);

			if ((byte + 1) % (HEX_LINE_SIZE / 2) == 0)
				printf( " " );
		}

		while (byte++ < HEX_LINE_SIZE)
		{
			if (byte % (HEX_LINE_SIZE / 2) == 0)
				printf( " " );
			printf( "   " );
		}

		printf( " " );
		for (byte = 0; byte < HEX_LINE_SIZE && k < length; byte++, k++)
		{
			printf("%c", isprint(buffer[k]) ? buffer[k] : '.');
			if ( (byte + 1) % (HEX_LINE_SIZE / 2) == 0)
				printf( " " );
		}

		printf("\n");
	}
}
