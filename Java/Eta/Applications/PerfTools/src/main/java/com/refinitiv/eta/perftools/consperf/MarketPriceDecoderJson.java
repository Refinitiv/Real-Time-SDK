/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.consperf;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EnumType;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.PostUserInfo;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.codec.UpdateMsg;

import com.fasterxml.jackson.databind.JsonNode;

import java.util.Map;
import java.util.Set;
import java.util.ArrayList;

/**
 * This is the market price decoder for the ConsPerf application.
 */
public class MarketPriceDecoderJson
{
	private static final int TIM_TRK_1_FID = 3902; /* Field TIM_TRK_1 is used to send update latency. */
	private static final int TIM_TRK_2_FID = 3903; /* Field TIM_TRK_2 is used to send post latency. */
	private static final int TIM_TRK_3_FID = 3904; /* Field TIM_TRK_3 is used to send post latency. */

	/* "TIM_TRK_1":null,"TIM_TRK_2":null,"TIM_TRK_3":null */
	private static final String TIM_TRK_1_NAME = "TIM_TRK_1"; /* Field TIM_TRK_1 is used to send update latency. */
	private static final String TIM_TRK_2_NAME = "TIM_TRK_2"; /* Field TIM_TRK_2 is used to send post latency. */
	private static final String TIM_TRK_3_NAME = "TIM_TRK_3"; /* Field TIM_TRK_3 is used to send post latency. */

    private PostUserInfo _postUserInfo; /* post user information */

    /**
     * Instantiates a new market price decoder.
     *
     * @param postUserInfo the post user info
     */
    public MarketPriceDecoderJson(PostUserInfo postUserInfo)
    {
        _postUserInfo = postUserInfo;
    }

	/* Update the latency statistics. */
    private void updateLatencyStats(ConsumerThreadInfo consumerThread, long timeTracker, int msgClass)
    {
    	long currentTime;
    	long unitsPerMicro;

    	currentTime = System.nanoTime()/1000;
    	unitsPerMicro = 1;

    	switch(msgClass)
    	{
    	case MsgClasses.UPDATE:
          consumerThread.timeRecordSubmit(consumerThread.latencyRecords(), timeTracker, currentTime, unitsPerMicro);
          break;
    	case MsgClasses.GENERIC:
            consumerThread.timeRecordSubmit(consumerThread.genMsgLatencyRecords(), timeTracker, currentTime, unitsPerMicro);
            break;
    	case MsgClasses.POST:
            consumerThread.timeRecordSubmit(consumerThread.postLatencyRecords(), timeTracker, currentTime, unitsPerMicro);
            break;
        default:
            break;
        }
    }
    
    /**
     *  Decode the update.
     *
     * @param jsonNode the JSON node to decode
     * @param msg the msg
     * @param consumerThread the consumer thread
     * @return the int
     */
	public int decodeUpdate(JsonNode jsonNode, Msg msg, ConsumerThreadInfo consumerThread)
	{
		int ret = 0;
		DictionaryEntry dictionaryEntry = null;

		long timeTracker = 0;
		long postTimeTracker = 0;
		long genMsgTimeTracker = 0;

		/* decode field list */
		JsonNode fieldsNode = jsonNode.get("Fields");
		if (!fieldsNode.isObject()) {
			return CodecReturnCodes.FAILURE;
		}

		Set<Map.Entry<String, JsonNode>> fields = fieldsNode.properties();
		for (Map.Entry<String, JsonNode> field : fields)
		{
			String fieldName = field.getKey();
			JsonNode fieldNode = field.getValue();

			/* get dictionary entry */

			/* decode fid value */
			long fidUIntValue = 0;
			double fidDoubleValue = 0;
			String fidStringValue = null;
			boolean fidBlankData = false;

			if (fieldNode.isIntegralNumber())
			{
				fidUIntValue = fieldNode.longValue();
			}
			else if (fieldNode.isFloatingPointNumber())
			{
				fidDoubleValue = fieldNode.doubleValue();
			}
			else if (fieldNode.isTextual())
			{
				fidStringValue = fieldNode.textValue();
			}
			else if (fieldNode.isNull())
			{
				fidBlankData = true;
			}
			else
			{
				System.out.printf("Error: Unhandled JSON data type in field %s.\n", fieldName);
				return CodecReturnCodes.FAILURE;
			}

			/* check latency fields */
			if (msg.msgClass() == MsgClasses.UPDATE && !fidBlankData)
			{
				if(fieldName == TIM_TRK_1_NAME)
					timeTracker = fidUIntValue;
				if(fieldName == TIM_TRK_2_NAME)
					postTimeTracker = fidUIntValue;
			}
			else if(msg.msgClass() == MsgClasses.GENERIC && !fidBlankData)
			{
				if(fieldName == TIM_TRK_3_NAME)
					genMsgTimeTracker = fidUIntValue;
			}
		}

		if (timeTracker > 0)
			updateLatencyStats(consumerThread, timeTracker, MsgClasses.UPDATE);
		if(postTimeTracker > 0 && checkPostUserInfo(msg))
			updateLatencyStats(consumerThread, postTimeTracker, MsgClasses.POST);
		if(genMsgTimeTracker > 0)
			updateLatencyStats(consumerThread, genMsgTimeTracker, MsgClasses.GENERIC);

		return CodecReturnCodes.SUCCESS;
	}

	/* Decode FID value. */
	private int decodeFidValue(DictionaryEntry dictionaryEntry, JsonNode jsonNode, ConsumerThreadInfo consumerThread)
	{
		int ret = CodecReturnCodes.SUCCESS;
		return ret;
	}

	/* Checks for PostUserInfo. */
	private boolean checkPostUserInfo(Msg msg)
	{
		/* If post user info is present, make sure it matches our info.
		 * Otherwise, assume any posted information present came from us anyway(return true). */
		switch(msg.msgClass())
		{
			case MsgClasses.REFRESH:
				RefreshMsg refreshMsg = (RefreshMsg)msg;
				return (!refreshMsg.checkHasPostUserInfo() ||
				 refreshMsg.postUserInfo().userAddr() == _postUserInfo.userAddr() &&
				 refreshMsg.postUserInfo().userId() == _postUserInfo.userId());
			case MsgClasses.UPDATE:
				UpdateMsg updateMsg = (UpdateMsg)msg;
				return (!updateMsg.checkHasPostUserInfo() ||
				 updateMsg.postUserInfo().userAddr() == _postUserInfo.userAddr() &&
				 updateMsg.postUserInfo().userId() == _postUserInfo.userId());
			default:
				return true;
		}
	}
}
