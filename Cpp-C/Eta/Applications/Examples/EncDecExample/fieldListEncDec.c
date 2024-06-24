/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */



#include "fieldListEncDec.h"

/* These functions are used by Encoding and Decoding for RsslFieldList Container Type */

/* this function will encode a basic field list with several primitives
   embedded in it */
RsslRet exampleEncodeFieldList(RsslEncodeIterator *encIter)
{
	/* used to store and check return values */
	RsslRet retVal;

	/* create and initialize field list structure */
	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;

	/* populate field list structure prior to call to rsslEncodeFieldListInit */
	/* NOTE: the fieldId, dictionaryId and fieldListNum values used for this example 
	* do not correspond to actual id values */

	/* indicate that standard data will be encoded and that dictionaryId and fieldListNum are included */
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
	/* populate dictionaryId and fieldListNum with info needed to cross-reference fieldIds and cache */
	fieldList.dictionaryId = 2; 
	fieldList.fieldListNum = 5;

	/* begin encoding of field list - assumes that the RsslEncodeIterator pointed by the encIter pointer is already populated with
	   buffer and version information */

	/* Please note: here for simplicity, we did not use success parameter for rsslEncodeFieldListInit/rsslEncodeFieldListComplete calls. 
	   We are just simply displaying an error if it occurs and exit, thus RSSL_TRUE is used in replacement for success parameter */

	if ((retVal = rsslEncodeFieldListInit(encIter, &fieldList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeFieldListInit().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	else
	{
		/* fieldListInit encoding was successful */
		/* create a single RsslFieldEntry and reuse for each entry */
		RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;

		RsslBool success = RSSL_TRUE;

		/* stack allocate a date and populate {day, month, year} */
		RsslDate rsslDate = {30, 11, 2010};
		RsslReal rsslReal = RSSL_INIT_REAL;
		RsslTime rsslTime = RSSL_INIT_TIME;
		RsslArray rsslArray = RSSL_INIT_ARRAY;
		RsslDateTime rsslDateTime = RSSL_INIT_DATETIME;
		RsslQos rsslQos = RSSL_INIT_QOS;
		RsslState rsslState = RSSL_INIT_STATE;
		RsslEnum rsslEnum = 0;
		RsslBuffer rsslBuffer = RSSL_INIT_BUFFER;
	
		RsslUInt uInt = 23456;
		RsslInt Int = 65432;
		RsslFloat Float = 3.14f;
		RsslDouble Double = 3.1416;

		/* create a buffer for uInt to encode into -
		   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  Typically, for performance, the transport layer can provide
	       a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	       For this example I am stack allocating the buffer */
		char buf[10] = "";  //10 bytes is large enough for UInt encoding 

		/* create a RsslBuffer to set the buffer into */
		RsslBuffer encDecBuffer;
		RsslBuffer *pEncUInt = 0;

		/* set the data members to encDecBuffer buf and the length I created */
		encDecBuffer.data = buf;
		encDecBuffer.length = 10;

		printf("\tFieldList Header Encoded\n");
	
		/* FIRST Field Entry: encode entry from the RsslDate primitive type */
		/* populate and encode field entry with fieldId and dataType information for this field */
		fieldEntry.fieldId = 16; 
		fieldEntry.dataType = RSSL_DT_DATE;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslDate)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded Date: %d-%d-%d\n", fieldEntry.fieldId, rsslDate.month, rsslDate.day, rsslDate.year);


		/* SECOND Field Entry: encode entry from the RsslUInt primitive type */
		fieldEntry.fieldId = 1080; 
		fieldEntry.dataType = RSSL_DT_UINT;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &uInt)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded Unsigned Integer: " RTR_LLU "\n", fieldEntry.fieldId, uInt);


		/* THIRD Field Entry: encode entry from preencoded buffer containing an encoded RsslUInt type */
		/* populate and encode field entry with fieldId and dataType information for this field */
		/* because we are re-populating all values on RsslFieldEntry, there is no need to clear it */
		fieldEntry.fieldId = 1081; 
		fieldEntry.dataType = RSSL_DT_UINT;

		/* assuming pEncUInt is an RsslBuffer with length and data properly populated */
		if ((retVal = getPreEncodedRsslUIntBuffer(&encDecBuffer, uInt)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with getPreEncodedRsslUIntBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		pEncUInt = &encDecBuffer;

		fieldEntry.encData.length = pEncUInt->length;
		fieldEntry.encData.data = pEncUInt->data;
		/* void* parameter is passed in as NULL because pre-encoded data is set on RsslFieldEntry itself */
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, NULL)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded Unsigned Integer: from preencoded buffer\n", fieldEntry.fieldId);


		/* FOURTH Field Entry: encode entry as a blank RsslReal primitive type */
		/* populate and encode field entry with fieldId and dataType information for this field */
		/* need to ensure that RsslFieldEntry is appropriatley cleared
		 * - clearing will ensure that encData is properly emptied */          
		rsslClearFieldEntry(&fieldEntry);

		fieldEntry.fieldId = 22; 
		fieldEntry.dataType = RSSL_DT_REAL;
		/* void* parameter is passed in as NULL and encData is empty due to clearing */
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, NULL)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded RsslReal as blank.\n", fieldEntry.fieldId);


		/* FIFTH Field Entry: encode entry for a RsslReal primitive type */
		fieldEntry.fieldId = 24; 
		fieldEntry.dataType = RSSL_DT_REAL; 
		rsslReal.hint = RSSL_RH_EXPONENT_2;
		rsslReal.value = 227;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslReal)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded RsslReal: hint: %d  value: " RTR_LLD "\n", fieldEntry.fieldId, rsslReal.hint, rsslReal.value);

		
		/* SIXTH Field Entry: encode entry for another RsslReal primitive type */
		fieldEntry.fieldId = 25; 
		fieldEntry.dataType = RSSL_DT_REAL;  
		rsslReal.hint = RSSL_RH_EXPONENT_4;
		rsslReal.value = 22801;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslReal)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded RsslReal: hint: %d  value: " RTR_LLD "\n", fieldEntry.fieldId, rsslReal.hint, rsslReal.value);


		/* SEVENTH Field Entry: encode entry for another RsslTime primitive type */
		fieldEntry.fieldId = 18; 
		fieldEntry.dataType = RSSL_DT_TIME;  
		rsslTime.hour = 8;
		rsslTime.minute = 39;
		rsslTime.second = 24;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslTime)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}
		
		printf("\t\tFID %d  Encoded RsslTime: %d:%d:%d\n", fieldEntry.fieldId, rsslTime.hour, rsslTime.minute, rsslTime.second);


		/* EIGHTH Field Entry: encode entry from the RsslInt primitive type */
		fieldEntry.fieldId = FID_INT; 
		fieldEntry.dataType = RSSL_DT_INT;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &Int)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded signed Integer: " RTR_LLD "\n", fieldEntry.fieldId, Int);


		/* NINETH Field Entry: encode entry from the RsslFloat primitive type */
		fieldEntry.fieldId = FID_FLOAT; 
		fieldEntry.dataType = RSSL_DT_FLOAT;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &Float)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded float: %f\n", fieldEntry.fieldId, Float);


		/* TENTH Field Entry: encode entry from the RsslDouble primitive type */
		fieldEntry.fieldId = FID_DOUBLE; 
		fieldEntry.dataType = RSSL_DT_DOUBLE;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &Double)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded Double: %f\n", fieldEntry.fieldId, Double);


		/* ELEVENTH Field Entry: encode entry from the RsslDateTime primitive type */
		fieldEntry.fieldId = FID_DATETIME; 
		fieldEntry.dataType = RSSL_DT_DATETIME;
		rsslDateTime.date.month = 11;
		rsslDateTime.date.day = 15;
		rsslDateTime.date.year = 2011;
		rsslDateTime.time.hour = 8;
		rsslDateTime.time.minute = 39;
		rsslDateTime.time.second = 24;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslDateTime)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded DateTime: %d-%d-%d %d:%d:%d\n", fieldEntry.fieldId, 
			rsslDateTime.date.month, rsslDateTime.date.day, rsslDateTime.date.year, rsslDateTime.time.hour, rsslDateTime.time.minute, rsslDateTime.time.second);

		/* TWELVETH Field Entry: encode entry from the RsslQos primitive type */
		fieldEntry.fieldId = FID_QOS; 
		fieldEntry.dataType = RSSL_DT_QOS;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslQos)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded QOS: %d %d %d %d %d\n", fieldEntry.fieldId, rsslQos.timeliness, rsslQos.rate, rsslQos.dynamic, rsslQos.rateInfo, rsslQos.timeInfo);


		/* THIRTEENTH Field Entry: encode entry from the RsslState primitive type */
		fieldEntry.fieldId = FID_STATE; 
		fieldEntry.dataType = RSSL_DT_STATE;
		rsslState.streamState = RSSL_STREAM_OPEN;
		rsslState.dataState = RSSL_DATA_OK;
		rsslState.code = RSSL_SC_NONE;
		rsslState.text.data = (char *)"Succeeded";
		rsslState.text.length = 10;		// include the null for printing
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslState)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded State: %d %d %d %s\n", fieldEntry.fieldId, rsslState.streamState, rsslState.dataState, rsslState.code, rsslState.text.data);

		/* FOURTEENTH Field Entry: encode entry from the RsslBuffer primitive type */
		fieldEntry.fieldId = FID_BUFFER; 
		fieldEntry.dataType = RSSL_DT_BUFFER;
		rsslBuffer.data = (char *)"BUFFEREXAMPLE";
		rsslBuffer.length = 14;			// include the null terminator to make it easier to print
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslBuffer)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded Buffer: %s\n", fieldEntry.fieldId, rsslBuffer.data);


		/* FIFTEENTH Field Entry: encode entry from the RsslEnum primitive type */
		fieldEntry.fieldId = FID_ENUM; 
		fieldEntry.dataType = RSSL_DT_ENUM;
		rsslEnum = 999;
		if ((retVal = rsslEncodeFieldEntry(encIter, &fieldEntry, &rsslEnum)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

		printf("\t\tFID %d  Encoded Enum: %d\n", fieldEntry.fieldId, rsslEnum);


		/* SIXTEENTH Field Entry: encode entry as a complex type, RsslArray primitive */
		/* populate and encode field entry with fieldId and dataType information for this field */
		/* need to ensure that RsslFieldEntry is appropriatley cleared
		 * - clearing will ensure that encData is properly emptied */          
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = 1021; 
		fieldEntry.dataType = RSSL_DT_ARRAY;
		/* begin complex field entry encoding, we are not sure of the approximate max encoding length */
		if ((retVal = rsslEncodeFieldEntryInit(encIter, &fieldEntry, 0)) < RSSL_RET_SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			/* print out message with return value string, value, and text */
			printf("Error %s (%d) encountered with rsslEncodeFieldEntryInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}
		else
		{
			/* now encode nested container using its own specific encode functions */
			/* encode RsslReal values into the array */
			rsslArray.primitiveType = RSSL_DT_UINT;
			/* values are variable length */
			rsslArray.itemLength = 2;
			/* begin encoding of array - using same encIterator as field list */
			if ((retVal = rsslEncodeArrayInit(encIter, &rsslArray)) < RSSL_RET_SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = RSSL_FALSE;
				/* print out message with return value string, value, and text */
				printf("Error %s (%d) encountered with rsslEncodeArrayInit().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			}
			else
			{
				/*----- Continue encoding array entries. ---- */
				RsslUInt uInt1 = 10, uInt2 = 20, uInt3 = 30, uInt4 = 40;

				/* array encoding was successful */

				printf("\t\tFID %d Encoded RsslArray: [", fieldEntry.fieldId);

				/* encode first entry from a UInt from a primitive type */
				if ((retVal = rsslEncodeArrayEntry(encIter, NULL, &uInt1)) < RSSL_RET_SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = RSSL_FALSE;
					printf("Error %s (%d) encountered with rsslEncodeArrayEntry().  Error Text: %s\n", 
						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
				}
				printf(" " RTR_LLU " ", uInt1);

				/* encode second entry from a UInt from a primitive type */
				if ((retVal = rsslEncodeArrayEntry(encIter, NULL, &uInt2)) < RSSL_RET_SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = RSSL_FALSE;
					printf("Error %s (%d) encountered with rsslEncodeArrayEntry().  Error Text: %s\n", 
						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
				}
				printf(" " RTR_LLU " ", uInt2);

				/* encode third entry from a UInt from a primitive type */
				if ((retVal = rsslEncodeArrayEntry(encIter, NULL, &uInt3)) < RSSL_RET_SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = RSSL_FALSE;
					printf("Error %s (%d) encountered with rsslEncodeArrayEntry().  Error Text: %s\n", 
						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
				}
				printf(" " RTR_LLU " ", uInt3);

				/* encode forth entry from a UInt from a primitive type */
				if ((retVal = rsslEncodeArrayEntry(encIter, NULL, &uInt4)) < RSSL_RET_SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = RSSL_FALSE;
					printf("Error %s (%d) encountered with rsslEncodeArrayEntry().  Error Text: %s\n", 
						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); ;
				}
				printf(" " RTR_LLU " ", uInt4);

				/* encode fifth entry from a UInt from pre-encoded integer contained in a buffer */
				/* this buffer.data should point to encoded int and the length should be number of bytes encoded */
				if ((retVal = rsslEncodeArrayEntry(encIter, pEncUInt, NULL)) < RSSL_RET_SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = RSSL_FALSE;
					printf("Error %s (%d) encountered with rsslEncodeArrayEntry().  Error Text: %s\n", 
						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
				}
				printf(" <Preencoded> ]\n");
					
			}

			/* complete array encoding.  If success parameter is true, this will finalize encoding.  
			   If success parameter is false, this will roll back encoding prior to rsslEncodeArrayInit */
			if ((retVal = rsslEncodeArrayComplete(encIter, success)) < RSSL_RET_SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = RSSL_FALSE;
				printf("Error %s (%d) encountered with rsslEncodeArrayComplete().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			}

		}
		/* complete encoding of complex field entry.  If any array encoding failed, success is false */
		if ((retVal = rsslEncodeFieldEntryComplete(encIter, success)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslEncodeFieldEntryComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			return retVal;
		}

	}

	/* complete fieldList encoding.  If success parameter is true, this will finalize encoding.  
	   If success parameter is false, this will roll back encoding prior to rsslEncodeFieldListInit */
	
	/* Please note: here for simplicity, we did not use success parameter for rsslEncodeFieldListInit/rsslEncodeFieldListComplete calls. 
	   We are just simply displaying an error if it occurs and exit, thus RSSL_TRUE is used in replacement for success parameter */

	if ((retVal = rsslEncodeFieldListComplete(encIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFieldListComplete().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tFieldList Encoding Complete\n");

	return RSSL_RET_SUCCESS;

}


/* this function will decode a basic field list with several primitives embedded in it */
RsslRet exampleDecodeFieldList(RsslDecodeIterator *decIter)
{
	/* used to store and check return codes */
	RsslRet retVal;

	/* create a field list to decode into - 
	   since set data is not supported yet, there is nothing to put 
	   here except the count of the number of fields encoded in this 
	   message*/
	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;

	/* create field entry to decode into */
	RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;

	/* create our primitives we will decode into */
	RsslDate rsslDate = RSSL_INIT_DATE;
	RsslTime rsslTime = RSSL_INIT_TIME;
	RsslReal rsslReal = RSSL_INIT_REAL;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslDateTime rsslDateTime = RSSL_INIT_DATETIME;
	RsslQos rsslQos = RSSL_INIT_QOS;
	RsslState rsslState = RSSL_INIT_STATE;
	RsslBuffer rsslBuffer = RSSL_INIT_BUFFER;
	RsslEnum rsslEnum = 0;

	RsslUInt uInt;
	RsslInt Int;
	RsslBuffer arrayBuffer;
	RsslFloat Float;
	RsslDouble Double;

	/* decode into the field list structure */
	/* decode the field list header */
	if ((retVal = rsslDecodeFieldList(decIter, &fieldList, 0)) >= RSSL_RET_SUCCESS)
	{
		printf("\tField List Header Decoded\n");

		/* decode each field entry  */
		/* since this succeeded, we can decode fields until we
		reach the end of the fields - until RSSL_RET_END_OF_CONTAINER is returned */
		while ((retVal = rsslDecodeFieldEntry(decIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retVal < RSSL_RET_SUCCESS)
			{
				/* decoding failure tends to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeFieldEntry().  Error Text: %s\n", 
						rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
				return retVal;
			}
			else
			{
				/* A ETA application will typically use a field dictionary to decode field list content.  
				When using, code similar to the following can be used to look up type in field dictionary 
				and call correct primitive decode function */ 

				/*switch (fieldDict->entriesArray[fEntry->fieldId]->rwfType)
				{	
						case RSSL_DT_REAL:
							retVal = rsslDecodeReal(&decIter, &rsslReal);
						break;
						case RSSL_DT_DATE:
							retVal = rsslDecodeDate(&decIter, &rsslDate);
						break;
						/* full switch statement omitted to shorten sample code * /

				}*/

				/* decode our entry - this is typically based on the FID and what type 
				   it is as designated by the field dictionary.  Because we know
				   the FIDs we used to encode, we can handle them here without
	               looking up in the dictionary */

				/* FID encodings: 
				   FID 16 is RsslDate
				   FID 1080 is RsslUInt
				   FID 1081 is RsslUnit
				   FID 22 is RsslReal
		           FID 24 is RsslReal
				   FID 25 is RsslReal
				   FID 18 is RsslTime
				   FID 1021 is RsslArray
				   negative FIDs are user defined FIDs
	            */

				switch (fieldEntry.fieldId)
				{
					case 16: //RsslDate
					{
						if ((retVal = rsslDecodeDate(decIter, &rsslDate)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rsslDate.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d  RsslDate Decoded: %d-%d-%d\n", fieldEntry.fieldId, rsslDate.month, rsslDate.day, rsslDate.year);
					}
					break;

					case 22:
					case 24:
					case 25: //RsslReal
					{
						if ((retVal = rsslDecodeReal(decIter, &rsslReal)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rsslReal.\n", __FILE__, __LINE__);
							return retVal;
						}

						/* Handle blank case.  This should be done across all types, however in this example
						 * we know the only field sent as blank is FID 22 */
						if (retVal == RSSL_RET_BLANK_DATA)
							printf("\t\tFID %d  RsslReal Decoded as blank.\n", fieldEntry.fieldId);
						else
							printf("\t\tFID %d  RsslReal Decoded: hint: %d  value: " RTR_LLD "\n", fieldEntry.fieldId, rsslReal.hint, rsslReal.value);
					}
					break;

					case 1080:
					case 1081: //RsslUInt
					{
						if ((retVal = rsslDecodeUInt(decIter, &uInt)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rssl unsigned integer.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d Rssl Unsigned Integer Decoded: " RTR_LLU "\n", fieldEntry.fieldId, uInt);
					}
					break;

					
					case 18: //RsslTime
					{
						if ((retVal = rsslDecodeTime(decIter, &rsslTime)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rsslTime.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d  RsslTime Decoded: %d:%d:%d\n", fieldEntry.fieldId, rsslTime.hour, rsslTime.minute, rsslTime.second);
					}
					break;

					/* full switch statement omitted to shorten sample code */

					case 1021: //RsslArray
					{
						/* decode into the array structure header */
						if ((retVal = rsslDecodeArray(decIter, &rsslArray)) >= RSSL_RET_SUCCESS)
						{
						/* decode each array entry  */
							printf("\t\tFID %d RsslArray Decoded: [", fieldEntry.fieldId);
							while ((retVal = rsslDecodeArrayEntry(decIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (retVal < RSSL_RET_SUCCESS)
								{
									/* decoding failure tends to be unrecoverable */
									printf("Error %s (%d) encountered with rsslDecodeArrayEntry().  Error Text: %s\n", 
											rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
									return retVal;
								}
								else
								{
									/* decode array entry into primitive type */
									/* we can use the same decode iterator, or set the encoded
									   entry buffer onto a new iterator */
									if ((retVal = rsslDecodeUInt(decIter, &uInt)) < RSSL_RET_SUCCESS)
									{
										printf("<%s:%d> Error decoding rssl unsigned integer.\n", __FILE__, __LINE__);
										return retVal;
									}

									printf(" " RTR_LLU " ", uInt);
								}
							}
							printf("]\n");
						}
						else
						{
							/* decoding failure tends to be unrecoverable */
							printf("Error %s (%d) encountered with rsslDecodeArray().  Error Text: %s\n", 
								rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
							return retVal;
						}

					}
					break;

					case FID_INT:
					{
						if ((retVal = rsslDecodeInt(decIter, &Int)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rssl signed integer.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d Rssl signed Integer Decoded: " RTR_LLD "\n", fieldEntry.fieldId, Int);
					}
					break;

					case FID_FLOAT:
					{
						if ((retVal = rsslDecodeFloat(decIter, &Float)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rssl float.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d Rssl float Decoded: %f\n", fieldEntry.fieldId, Float);
					}
					break;

					case FID_DOUBLE:
					{
						if ((retVal = rsslDecodeDouble(decIter, &Double)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rssl double.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d Rssl double Decoded: %lf\n", fieldEntry.fieldId, Double);
					}
					break;

					case FID_DATETIME:
					{
						if ((retVal = rsslDecodeDateTime(decIter, &rsslDateTime)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rsslDateTime.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d  RsslDateTime Decoded: %d-%d-%d %d:%d:%d\n", fieldEntry.fieldId, 
									rsslDateTime.date.month, rsslDateTime.date.day, rsslDateTime.date.year, 
									rsslDateTime.time.hour, rsslDateTime.time.minute, rsslDateTime.time.second);
					}
					break;

					case FID_QOS:
					{
						if ((retVal = rsslDecodeQos(decIter, &rsslQos)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding QOS.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d  Decoded QOS: %d %d %d %d %d\n", fieldEntry.fieldId, rsslQos.timeliness, rsslQos.rate, rsslQos.dynamic, rsslQos.rateInfo, rsslQos.timeInfo);
					}
					break;

					case FID_STATE:
					{
						if ((retVal = rsslDecodeState(decIter, &rsslState)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rsslState.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d  Decoded State: %d %d %d %s\n", fieldEntry.fieldId, rsslState.streamState, rsslState.dataState, rsslState.code, rsslState.text.data);
					}
					break;

					case FID_BUFFER:
					{
						if ((retVal = rsslDecodeBuffer(decIter, &rsslBuffer)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rsslBuffer.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d  RsslBuffer Decoded: %s\n", fieldEntry.fieldId, rsslBuffer.data);	// printf assumes its a null terminated character string 
					}
					break;

					case FID_ENUM:
					{
						if ((retVal = rsslDecodeEnum(decIter, &rsslEnum)) < RSSL_RET_SUCCESS)
						{
							printf("<%s:%d> Error decoding rsslEnum.\n", __FILE__, __LINE__);
							return retVal;
						}

						printf("\t\tFID %d  RsslEnum Decoded: %d\n", fieldEntry.fieldId, rsslEnum);
					}
					break;

					default:
						printf("\t\tUnexpected FID %d encountered!\n", fieldEntry.fieldId);
				}
			}
		}
	}
	else
	{
		/* decoding failure tends to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeFieldList().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}

	printf("\tFieldList Decoding Complete\n");

	return RSSL_RET_SUCCESS;
}


/* this function returns a preencoded buffer containing an encoded RsslUInt type */
/* assuming pEncUInt is an RsslBuffer with length and data properly populated */
RsslRet getPreEncodedRsslUIntBuffer(RsslBuffer *pEncUInt, RsslUInt uInt)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encDecBuffer)->data points to 
	   sufficient memory and (&encDecBuffer)->length indicates number of bytes available in 
	   pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, pEncUInt)) < RSSL_RET_SUCCESS)
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

	if ((retVal = rsslEncodeUInt(&encodeIter, &uInt)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeUInt().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	//this is important!
	/* When encoding is complete, set the pEncUInt->length to the number of bytes Encoded */
	pEncUInt->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}


/* this function returns a preencoded buffer containing an encoded RsslFieldList type */
/* assuming pEncFieldList RsslBuffer contains the pre-encoded payload with data and length populated */
RsslRet getPreEncodedRsslFieldListBuffer(RsslBuffer *pEncUInt)
{
	RsslRet retVal;  //used to store and check the return value from the function calls

	RsslUInt8 majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	RsslUInt8 minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded

	/* create and clear iterator to prepare for encoding */
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* associate buffer and iterator, code assumes that (&encDecBuffer)->data points to 
	   sufficient memory and (&encDecBuffer)->length indicates number of bytes available in 
	   pBuffer->data */
	if ((retVal = rsslSetEncodeIteratorBuffer(&encodeIter, pEncUInt)) < RSSL_RET_SUCCESS)
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

	if ((retVal = exampleEncodeFieldList(&encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with exampleEncodeFieldList().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	//this is important!
	/* When encoding is complete, set the pEncUInt->length to the number of bytes Encoded */
	pEncUInt->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

