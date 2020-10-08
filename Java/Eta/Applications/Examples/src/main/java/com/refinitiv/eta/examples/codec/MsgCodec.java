package com.refinitiv.eta.examples.codec;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.DomainTypes;

/**
 * This is used for encoding and decoding a RefreshMsg with a Map or with a Series
*/
public class MsgCodec
{
	/* This method encodes a simple RefreshMsg message
	that contains a MAP.  For the payload of the message, 
	just have it call the map encoding method.  */
	int exampleEncodeRefreshWithMap(EncodeIterator encIter)
	{
		MapCodec mapCodec = new MapCodec(); // the map codec
		
		/* use this to store and check return codes */
		int retVal;
		boolean success = true;

		/* Populate and encode a refreshMsg */
		RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
		refreshMsg.msgClass(MsgClasses.REFRESH);  /* message is a refresh */
		refreshMsg.domainType(DomainTypes.MARKET_BY_ORDER);
		refreshMsg.containerType(DataTypes.MAP);

		/* Use the stream Id corresponding to the request, because it is in reply to a request, its solicited */
		refreshMsg.streamId(6); 

		/* Populate flags for refresh message members and behavior because this in response to a request
		   This should be solicited, msgKey should be present, single part refresh so it is complete,
		   and also want the concrete qos of the stream */
		refreshMsg.flags(RefreshMsgFlags.SOLICITED | RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.REFRESH_COMPLETE |
			RefreshMsgFlags.HAS_QOS | RefreshMsgFlags.CLEAR_CACHE);

		/* Populate msgKey to specify a serviceId, a name with type of RIC (which is default nameType) */
		refreshMsg.msgKey().flags(MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME);
		refreshMsg.msgKey().serviceId(1);
		/* Specify name and length of name.  Because this is a RIC, no nameType is required.  */
		refreshMsg.msgKey().name().data("TRI");

		// set RefreshMsg state information
		refreshMsg.state().streamState(StreamStates.OPEN);
		refreshMsg.state().dataState(DataStates.OK);
		refreshMsg.state().code(StateCodes.NONE);

		/* Qos */
		refreshMsg.qos().dynamic(false);
		refreshMsg.qos().rate(QosRates.TICK_BY_TICK);
		refreshMsg.qos().timeliness(QosTimeliness.REALTIME);
		
		/* Now that everything is set on my message I can encode the message header */

		/* Since I have message payload of a Map that I am going to encode after this,
		  I should use the EncodeMsgInit method */

		/* begin encoding of message - assumes that encIter is already populated with
		   buffer and version information, store return value to determine success or failure */
		/* data max encoded size is unknown so 0 is used */
		if ((retVal = refreshMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = false;
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeMsgInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}

		System.out.printf("\tMessage Header Encoded for Item %s\n", refreshMsg.msgKey().name().toString());

		/* I am now ready to populate and encode my Map and its contents */
		if ((retVal = mapCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = false;
			System.out.printf("Error encoding map.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeMap.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}
		
		/* since our Map encoder completed successfully, lets complete encode the message */
		if ((retVal = refreshMsg.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
		{
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeMsgComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.printf("\tMessage Encoding Complete\n");

		return CodecReturnCodes.SUCCESS;
	}
	
	/* This method decodes a a simple RefreshMsg message
	that contains a MAP.  For the payload of the message, 
	just have it call the map decoding method.  */
	int exampleDecodeRefreshWithMap(DecodeIterator decIter)
	{
		MapCodec mapCodec = new MapCodec(); // the map codec
		
		/* use this to store and check return codes */
		int retVal;

		/* Create a Msg to decode into */
		Msg msg = CodecFactory.createMsg(); 

		/* decode the message header */

		/* This method will only decode the message header.  Any message 
		   payload will be contained inside of the message structure encodedData member */
		if ((retVal = msg.decode(decIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding message header.\n");
			return retVal;
		}
		
		System.out.printf("Message Header Decoded for Item %s\n", msg.msgKey().name().toString());

		if ((retVal = mapCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			System.out.printf("Error decoding map.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeMap.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}

		System.out.printf("\tMessage Decoding Complete\n");
			
		return CodecReturnCodes.SUCCESS;
	}

	/* This method encodes a a simple RefreshMsg message
	that contains a SERIES.  For the payload of the message, 
	just have it call the series encoding method.  */
	int exampleEncodeRefreshWithSeries(EncodeIterator encIter)
	{
		SeriesCodec seriesCodec = new SeriesCodec(); // the series codec
		
		/* use this to store and check return codes */
		int retVal;
		boolean success = true;

		/* Populate and encode a refreshMsg */
		RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
		refreshMsg.msgClass(MsgClasses.REFRESH);  /* message is a refresh */
		refreshMsg.domainType(DomainTypes.MARKET_BY_ORDER);
		refreshMsg.containerType(DataTypes.SERIES);

		/* Use the stream Id corresponding to the request, because it is in reply to a request, its solicited */
		refreshMsg.streamId(7); 

		/* Populate flags for refresh message members and behavior because this in response to a request
		   This should be solicited, msgKey should be present, single part refresh so it is complete,
		   and also want the concrete qos of the stream */
		refreshMsg.flags(RefreshMsgFlags.SOLICITED | RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.REFRESH_COMPLETE |
			RefreshMsgFlags.HAS_QOS | RefreshMsgFlags.CLEAR_CACHE);

		/* Populate msgKey to specify a serviceId, a name with type of RIC (which is default nameType) */
		refreshMsg.msgKey().flags(MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME);
		refreshMsg.msgKey().serviceId(1);
		/* Specify name and length of name.  Because this is a RIC, no nameType is required.  */
		refreshMsg.msgKey().name().data("TRI");

		// set RefreshMsg state information
		refreshMsg.state().streamState(StreamStates.OPEN);
		refreshMsg.state().dataState(DataStates.OK);
		refreshMsg.state().code(StateCodes.NONE);

		/* Qos */
		refreshMsg.qos().dynamic(false);
		refreshMsg.qos().rate(QosRates.TICK_BY_TICK);
		refreshMsg.qos().timeliness(QosTimeliness.REALTIME);
		
		/* Now that everything is set on my message I can encode the message header */

		/* Since I have message payload of a Map that I am going to encode after this,
		  I should use the EncodeMsgInit method */

		/* begin encoding of message - assumes that (*encIter) is already populated with
		   buffer and version information, store return value to determine success or failure */
		/* data max encoded size is unknown so 0 is used */
		if ((retVal = refreshMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = false;
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeMsgInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}

		System.out.printf("\tMessage Header Encoded for Item %s\n", refreshMsg.msgKey().name().toString());

		/* I am now ready to populate and encode my Map and its contents */
		if ((retVal = seriesCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = false;
			System.out.printf("Error encoding series.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeSeries.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}
		
		/* since our Map encoder completed successfully, lets complete encode the message */
		if ((retVal = refreshMsg.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
		{
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeMsgComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.printf("\tMessage Encoding Complete\n");

		return CodecReturnCodes.SUCCESS;
	}
	
	/* This method decodes a a simple RefreshMsg message
	that contains a SERIES.  For the payload of the message, 
	just have it call the map decoding method.  */
	int exampleDecodeRefreshWithSeries(DecodeIterator decIter)
	{
		SeriesCodec seriesCodec = new SeriesCodec(); // the series codec
		
		/* use this to store and check return codes */
		int retVal;

		/* Create a Msg to decode into */
		Msg msg = CodecFactory.createMsg(); 

		/* decode the message header */

		/* This method will only decode the message header.  Any message 
		   payload will be contained inside of the message structure encodedData member */
		if ((retVal = msg.decode(decIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding message header.\n");
			return retVal;
		}
			
		System.out.printf("Message Header Decoded for Item %s\n", msg.msgKey().name().toString());

		if ((retVal = seriesCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			System.out.printf("Error decoding series.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeSeries.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}

		System.out.printf("\tMessage Decoding Complete\n");
			
		return CodecReturnCodes.SUCCESS;
	}
}
