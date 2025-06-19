/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//  
//  This example is intended to serve as a demonstration 
//  of encoding and decoding using the RSSL API.  
//  The user can set break points and step through
//  this code to get a look and feel for how to use the API.
//
//  This application is written as a basic example.  
//  Some of the techniques used can be modified for
//  best performance as needed.
//
//  Because both encode and decode sides know what 
//  was encoded, some checking is not occuring.
//  For safety, message and container masks
//  should be checked before accessing 
//  members.  
//
//  FieldList encode and decode examples are 
//  located in fieldListEncDec.h/.c.
//
//  ElementList encode and decode examples are 
//  located in elementListEncDec.h/.c.
//
//  Series encode and decode examples are 
//  located in SeriesEncDec.h/.c.
//
//  Map encode and decode examples are
//  located in mapEncDec.h/.c.  
//
//  Message encoding and decoding examples are
//	located in msgEncDec.h/.c.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "fieldListEncDec.h"
#include "elementListEncDec.h"
#include "seriesEncDec.h"
#include "mapEncDec.h"
#include "msgEncDec.h"
#include "vectorEncDec.h"
#include "filterListEncDec.h"

#ifdef __cplusplus
extern "C" {
#endif

/* perform the example for encoding/decoding a RsslFieldList Container Type */
RsslRet exampleEncodeDecodeRsslFieldList();

/* perform the example for encoding/decoding a RsslElementList Container Type */
RsslRet exampleEncodeDecodeRsslElementList();

/* perform the example for encoding/decoding a RsslSeries Container Type */
RsslRet exampleEncodeDecodeRsslSeries();

/* perform the example for encoding/decoding a RsslVector Container Type */
RsslRet exampleEncodeDecodeRsslVector();

/* perform the example for encoding/decoding a RsslFilterList Container Type */
RsslRet exampleEncodeDecodeRsslFilterList();

/* perform the example for encoding/decoding a RsslMap Container Type */
RsslRet exampleEncodeDecodeRsslMap();

/* perform the example for encoding/decoding a RsslRefreshMsg message with a Map */
RsslRet exampleEncodeDecodeMsgWithMap();

/* perform the example for encoding/decoding a RsslRefreshMsg message with a Series */
RsslRet exampleEncodeDecodeMsgWithSeries();

/* this function will encode a basic field list with several primitives embedded in it */
RsslRet exampleEncodeRsslFieldList(RsslBuffer *encBuf);

/* this function will encode a basic rsslMap which contains nested field lists */
RsslRet exampleEncodeRsslMap(RsslBuffer *encBuf);

/* this function will encode a basic rsslElementList which contains nested field lists */
RsslRet exampleEncodeRsslElementList(RsslBuffer *encBuf);

/* this function will encode a basic rsslSeries which contains nested field lists */
RsslRet exampleEncodeRsslSeries(RsslBuffer *encBuf);

/* this function will encode a basic rsslVector which contains nested element lists */
RsslRet exampleEncodeRsslVector(RsslBuffer *encBuf);

/* this function will encode a basic rsslFilterList which contains 2 nested element list and a field list */
RsslRet exampleEncodeRsslFilterList(RsslBuffer *encBuf);

/* this function will encode a simple RsslRefreshMsg message which contains a RSSL_DT_MAP containerType */
RsslRet exampleEncodeMsgWithMap(RsslBuffer *encBuf);

/* this function will encode a simple RsslRefreshMsg message which contains a RSSL_DT_SERIES containerType */
RsslRet exampleEncodeMsgWithSeries(RsslBuffer *encBuf);

/* this function will decode a basic field list with several primitives embedded in it */
RsslRet exampleDecodeRsslFieldList(RsslBuffer *decBuf);

/* this function will decode a basic rsslMap which contains nested field lists */
RsslRet exampleDecodeRsslMap(RsslBuffer *decBuf);

/* this function will decode a basic rsslElementList which contains nested field lists */
RsslRet exampleDecodeRsslElementList(RsslBuffer *decBuf);

/* this function will decode a basic rsslSeries which contains nested field lists */
RsslRet exampleDecodeRsslSeries(RsslBuffer *decBuf);

/* this function will decode a basic rsslVector which contains nested element lists */
RsslRet exampleDecodeRsslVector(RsslBuffer *decBuf);

/* this function will decode a basic rsslFilterList which contains 2 nested element list and a field list*/
RsslRet exampleDecodeRsslFilterList(RsslBuffer *decBuf);

/* this function will decode a simple RsslRefreshMsg message which contains a RSSL_DT_MAP containerType */
RsslRet exampleDecodeMsgWithMap(RsslBuffer *encBuf);

/* this function will decode a simple RsslRefreshMsg message which contains a RSSL_DT_SERIES containerType */
RsslRet exampleDecodeMsgWithSeries(RsslBuffer *encBuf);

/*
 * WINDOWS: wait for user to enter something before exiting the application - Normal exit
 */
void winUserExitNormal();

/*
 * WINDOWS: wait for user to enter something before exiting the application
 * - Encountered Error condition(s)
 */
void winUserExitError();


#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[])
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	// 
	//  This does encoding and decoding of a simple RsslFieldList 
	//  Container Type that contains a Field List with various primitive 
	//  types as payload. 
	//
	if ((retVal = exampleEncodeDecodeRsslFieldList()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslFieldList Container Type!\n");
		winUserExitError();
		return retVal;
	}
	
	//
	// perform encoding/decoding of a RsslMap Container Type containing nested RsslFieldLists */
	// 
	if ((retVal = exampleEncodeDecodeRsslMap()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslMap Container Type!\n");
		winUserExitError();
		return retVal;
	}

	// 
	//  shows encoding and decoding of a message with a map.
	//  It encodes an RsslRefreshMsg that contains an RSSL_DT_MAP.  
	//  For the payload of the message, it just calls the map encoding 
	//  function.  Same thing for decoding.   
	//
	if ((retVal = exampleEncodeDecodeMsgWithMap()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslRefreshMsg message containing an RsslMap!\n");
		winUserExitError();
		return retVal;
	}
	// 
	//  does encoding and decoding of an RsslElementList 
	//  that contains various element entries as payload. 
	//
	if ((retVal = exampleEncodeDecodeRsslElementList()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslElementList Container Type!\n");
		winUserExitError();
		return retVal;
	}

	//
	//  does encoding and decoding of an RsslSeries
	//  that contains various series entries containing 
	//  an rsslElementList as payload. 
	//
	if ((retVal = exampleEncodeDecodeRsslSeries()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslSeries Container Type!\n");
		winUserExitError();
		return retVal;
	}

	//
	//  does encoding and decoding of an RsslVector
	//  that contains various vector entries containing 
	//  an rsslElementList as payload. 
	//
	if ((retVal = exampleEncodeDecodeRsslVector()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslVector Container Type!\n");
		winUserExitError();
		return retVal;
	}

	//
	//  does encoding and decoding of an RsslFilterList
	//  that contains various filterList entries containing 
	//  an rsslElementList as payload. 
	//
	if ((retVal = exampleEncodeDecodeRsslFilterList()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslFilterList Container Type!\n");
		winUserExitError();
		return retVal;
	}

	//
	//  does encoding and decoding of a refresh msg that contains a seriesList. 
	//  Each series entry contains an rsslElementList as payload. 
	//
	if ((retVal = exampleEncodeDecodeMsgWithSeries()) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding/decoding a RsslRefreshMsg message containing a RsslSeries!\n");
		winUserExitError();
		return retVal;
	}

	winUserExitNormal();
	return RSSL_RET_SUCCESS;
}


RsslRet exampleEncodeDecodeRsslFieldList()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  //500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslFieldList Container Type */
	/* We pass in the buffer to this function with the 
	   total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer
	   for the encDecBuffer.length member */
	if ((retVal = exampleEncodeRsslFieldList(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslFieldList Container Type!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslFieldList Container Type */
	/* We will pass the same buffer directly to the 
	   decode function which will then decode it */
	if ((retVal = exampleDecodeRsslFieldList(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslFieldList Container Type!\n");
		return retVal;
	}

	return RSSL_RET_SUCCESS;
}


RsslRet exampleEncodeDecodeRsslMap()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  
	   Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  //500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslMap Container Type */
	/* We pass in the buffer to this function with the 
	   total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer
	   for the encDecBuffer.length member */
	if ((retVal = exampleEncodeRsslMap(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslMap Container Type!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslMap Container Type */
	/* We will pass the same buffer directly to the 
	   decode function which will then decode it */
	if ((retVal = exampleDecodeRsslMap(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslMap Container Type!\n");
		return retVal;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet exampleEncodeDecodeRsslElementList()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  // 500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslElementList Container Type */
	/* We pass in the buffer to this function with the 
	   total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer
	   for the encDecBuffer.length member */
	if ((retVal = exampleEncodeRsslElementList(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslElement Container Type!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslElementList Container Type */
	/* We will pass the same buffer directly to the decode function which will then decode it */
	if ((retVal = exampleDecodeRsslElementList(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslElementList Container Type!\n");
		return retVal;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet exampleEncodeDecodeRsslSeries()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  
	   Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  // 500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslSeries Container Type */
	/* We pass in the buffer to this function with the total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleEncodeRsslSeries(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslSeries Container Type!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslSeries Container Type */
	/* We will pass the same buffer directly to the decode function which will then decode it */
	if ((retVal = exampleDecodeRsslSeries(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslSeries Container Type!\n");
		return retVal;
	}
	return RSSL_RET_SUCCESS;
}

RsslRet exampleEncodeDecodeRsslVector()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  
	   Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  // 500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslVector Container Type */
	/* We pass in the buffer to this function with the total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleEncodeRsslVector(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslVector Container Type!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslVector Container Type */
	/* We will pass the same buffer directly to the decode function which will then decode it */
	if ((retVal = exampleDecodeRsslVector(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslVector Container Type!\n");
		return retVal;
	}
	return RSSL_RET_SUCCESS;
}

RsslRet exampleEncodeDecodeRsslFilterList()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  
	   Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  // 500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslFilterList Container Type */
	/* We pass in the buffer to this function with the total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleEncodeRsslFilterList(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslFilterList Container Type!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslFilterList Container Type */
	/* We will pass the same buffer directly to the decode function which will then decode it */
	if ((retVal = exampleDecodeRsslFilterList(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslFilterList Container Type!\n");
		return retVal;
	}
	return RSSL_RET_SUCCESS;
}

RsslRet exampleEncodeDecodeMsgWithMap()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  
	   Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  //500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslRefreshMsg simple message */
	/* We pass in the buffer to this function with the total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleEncodeMsgWithMap(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslRefreshMsg with a map!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslRefreshMsg simple message */
	/* We will pass the same buffer directly to the decode function which will then decode it */
	if ((retVal = exampleDecodeMsgWithMap(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslRefreshMsg message with map!\n");
		return retVal;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet exampleEncodeDecodeMsgWithSeries()
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	/* create a buffer to encode into -
	   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  
	   Typically, for performance, the transport layer can provide
	   a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	   For this example I am stack allocating the buffer */
	char buf[500] = "";  //500 bytes is large enough for the message I am encoding 

	/* create a RsslBuffer to set the buffer into */
	RsslBuffer encDecBuffer;

	/* set the data members to encDecBuffer buf and the length I created */
	encDecBuffer.data = buf;
	encDecBuffer.length = 500;
	
	/* perform the example for encoding a RsslRefreshMsg simple message */
	/* We pass in the buffer to this function with the total length available.  When the function finishes,
	   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleEncodeMsgWithSeries(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for encoding a RsslRefreshMsg message with Series!\n");
		return retVal;
	}
	
	/* perform the example for decoding a RsslRefreshMsg simple message */
	/* We will pass the same buffer directly to the decode function which will then decode it */
	if ((retVal = exampleDecodeMsgWithSeries(&encDecBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("Error running the example for decoding a RsslRefreshMsg with a Series!\n");
		return retVal;
	}

	return RSSL_RET_SUCCESS;
}


/* Example for encoding a RsslFieldList Container Type */
RsslRet exampleEncodeRsslFieldList(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in 
	   pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the simple message */
	printf("\nBegin RSSL Fieldlist Encoding Example\n");

	if ((retVal = exampleEncodeFieldList(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding field list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeFieldList().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Fieldlist Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}



/* Example for encoding a RsslMap Container Type */
RsslRet exampleEncodeRsslMap(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the simple message */
	printf("\nBegin RSSL Map Encoding Example\n");

	if ((retVal = exampleEncodeMap(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding map.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeMap().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Map Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/* Example for encoding a RsslElementList Container Type */
RsslRet exampleEncodeRsslElementList(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in 
	   pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the simple message */
	printf("\nBegin RSSL ElementList Encoding Example\n");

	printf("\tElementList Encoding Begin\n");
	if ((retVal = exampleEncodeElementList(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding ElementList.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeElementList().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL ElementList Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/* Example for encoding a RsslSeries Container Type */
RsslRet exampleEncodeRsslSeries(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the simple message */
	printf("\nBegin RSSL Series Encoding Example\n");

	if ((retVal = exampleEncodeSeries(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding Series.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeSeries().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Series Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/* Example for encoding a RsslVector Container Type */
RsslRet exampleEncodeRsslVector(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the simple message */
	printf("\nBegin RSSL Vector Encoding Example\n");

	if ((retVal = exampleEncodeVector(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding Vector.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeVector().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Vector Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/* Example for encoding a RsslFilterList Container Type */
RsslRet exampleEncodeRsslFilterList(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the simple message */
	printf("\nBegin RSSL FilterList Encoding Example\n");

	if ((retVal = exampleEncodeFilterList(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding FilterList.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeFilterList().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL FilterList Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}


/* Example for encoding a RsslRefreshMsg with map */
RsslRet exampleEncodeMsgWithMap(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the message */
	printf("\nBegin RsslRefreshMsg message with Map Encoding Example\n");

	if ((retVal = exampleEncodeRefreshMsgWithMap(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding map.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeMsg().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RsslRefreshMsg message with map Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/* Example for encoding a RsslRefreshMsg with a series */
RsslRet exampleEncodeMsgWithSeries(RsslBuffer *encBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encBuf)->data points to 
	   sufficient memory and (&encBuf)->length indicates number of bytes available in pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, encBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetEncodeIteratorRWFVersion(&encodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetEncodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content encoding now that iterator is prepared.  */

	/* Encode the message */
	printf("\nBegin RsslRefreshMsg message with Series Encoding Example\n");

	if ((retVal = exampleEncodeRefreshMsgWithSeries(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding series.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeMsgWithSeries().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RsslRefreshMsg message with series Encoding Example Complete\n");

	/* When encoding is complete, set the pBuffer->length to the number of bytes Encoded */
	encBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}


/* Example for decoding a RsslFieldList Container Type */
RsslRet exampleDecodeRsslFieldList(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both 
	   as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	/* Decode the FieldList */
	printf("\nBegin RSSL Fieldlist Decoding Example\n");

	/* We pass in the buffer to this function with the total length available.  When the function finishes,
       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleDecodeFieldList(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding field list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeFieldList().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Fieldlist Decoding Example Complete\n");

	return RSSL_RET_SUCCESS;
}



/* Example for decoding a RsslMap Container Type */
RsslRet exampleDecodeRsslMap(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both 
	   as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	/* Decode the Map */
	printf("\nBegin RSSL Map Decoding Example\n");

	/* We pass in the buffer to this function with the total length available.  When the function finishes,
       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleDecodeMap(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding map.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeMap().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Map Decoding Example Complete\n");

	return RSSL_RET_SUCCESS;
}


/* Example for decoding a RsslElementList Container Type */
RsslRet exampleDecodeRsslElementList(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both 
	   as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	/* Decode the ElementList */
	printf("\nBegin RSSL ElementList Decoding Example\n");

	/* We pass in the buffer to this function with the 
       total length available.  When the function finishes,
       it will set the actual length encoded into the buffer
       for the encDecBuffer.length member */
	if ((retVal = exampleDecodeElementList(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding ElementList.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeElementList().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL ElementList Decoding Example Complete\n");

	return RSSL_RET_SUCCESS;
}

/* Example for decoding a RsslSeries Container Type */
RsslRet exampleDecodeRsslSeries(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	printf("\nBegin RSSL Series Decoding Example\n");

	/* We pass in the buffer to this function with the total length available.  When the function finishes,
       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleDecodeSeries(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding Series.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeSeries().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Series Decoding Example Complete\n");

	return RSSL_RET_SUCCESS;
}

/* Example for decoding a RsslVector Container Type */
RsslRet exampleDecodeRsslVector(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	printf("\nBegin RSSL Vector Decoding Example\n");

	/* We pass in the buffer to this function with the total length available.  When the function finishes,
       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleDecodeVector(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding Vector.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeVector().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL Vector Decoding Example Complete\n");

	return RSSL_RET_SUCCESS;
}

/* Example for decoding a RsslFilterList Container Type */
RsslRet exampleDecodeRsslFilterList(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	printf("\nBegin RSSL FilterList Decoding Example\n");

	/* We pass in the buffer to this function with the total length available.  When the function finishes,
       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
	if ((retVal = exampleDecodeFilterList(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding FilterList.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeFilterList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RSSL FilterList Decoding Example Complete\n");

	return RSSL_RET_SUCCESS;
}

/* Example for decoding a RsslRefreshMsg message with a map */
RsslRet exampleDecodeMsgWithMap(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both 
	   as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	/* Decode the message with map */
	printf("\nBegin RsslRefreshMsg message with Map Decoding Example\n");

	/* We pass in the buffer to this function with the 
       total length available.  When the function finishes,
       it will set the actual length encoded into the buffer
       for the encDecBuffer.length member */
	if ((retVal = exampleDecodeRefreshMsgWithMap(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding map.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeMsg().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RsslRefreshMsg message with Map Decoding Example Complete\n");
	return RSSL_RET_SUCCESS;
}

/* Example for decoding a RsslRefreshMsg message with a series */
RsslRet exampleDecodeMsgWithSeries(RsslBuffer *decBuf)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and initialize decode iterator */
	/* Note: Iterator must be initialized before decoding begins - 
	   Here, I will use the clear method later on when decoding begins.
	   Typically, one function would not perform both the encode and the decode,
	   but as there is no network in between this example will do both 
	   as a demonstration */
	RsslDecodeIterator decodeIter;

	/* clear the decode iterator with the clear method - this 
	   is useful to reuse iterators as the static initializer can
	   only be used on declaration */
	rsslClearDecodeIterator(&decodeIter);

	/* associate buffer and iterator, code assumes that decBuf->data points to 
	   encoded contents to decode */
	if ((retVal = rsslSetDecodeIteratorBuffer(&decodeIter, decBuf)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* set proper protocol version information on iterator, this can typically be obtained from 
	   the RsslChannel associated with the connection once it becomes active */
	if ((retVal = rsslSetDecodeIteratorRWFVersion(&decodeIter, majorVersion, minorVersion)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslSetDecodeIteratorRWFVersion().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* Perform all content decoding now that iterator is prepared.  */

	/* Decode the message with series */
	printf("\nBegin RsslRefreshMsg message with Series Decoding Example\n");

	/* We pass in the buffer to this function with the 
       total length available.  When the function finishes,
       it will set the actual length encoded into the buffer
       for the encDecBuffer.length member */
	if ((retVal = exampleDecodeRefreshMsgWithSeries(&decodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding series.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeMsgWithSeries().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("RsslRefreshMsg message with Series Decoding Example Complete\n");
	return RSSL_RET_SUCCESS;
}

/*
 * WINDOWS: wait for user to enter something before exiting the application
 * - Normal exit
 */
void winUserExitNormal()
{
	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(RSSL_RET_SUCCESS);
}

/*
 * WINDOWS: wait for user to enter something before exiting the application
 * - Encountered Error condition(s)
 */
void winUserExitError()
{
	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(RSSL_RET_FAILURE);
}

