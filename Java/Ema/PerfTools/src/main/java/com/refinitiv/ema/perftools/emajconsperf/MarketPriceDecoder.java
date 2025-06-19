/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.emajconsperf;

import java.util.Iterator;

import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmAscii;
import com.refinitiv.ema.access.OmmBuffer;
import com.refinitiv.ema.access.OmmDate;
import com.refinitiv.ema.access.OmmDateTime;
import com.refinitiv.ema.access.OmmDouble;
import com.refinitiv.ema.access.OmmEnum;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmFloat;
import com.refinitiv.ema.access.OmmInt;
import com.refinitiv.ema.access.OmmQos;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmRmtes;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.OmmTime;
import com.refinitiv.ema.access.OmmUInt;
import com.refinitiv.ema.access.OmmUtf8;
import com.refinitiv.ema.perftools.common.PostUserInfo;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.UpdateMsg;

/**
 * This is the market price decoder for the emajConsPerf application.
 */
public class MarketPriceDecoder
{
	private static final int TIM_TRK_1_FID = 3902; /* Field TIM_TRK_1 is used to send update latency. */
	private static final int TIM_TRK_2_FID = 3903; /* Field TIM_TRK_2 is used to send post latency. */
    private static final int TIM_TRK_3_FID = 3904; /* Field TIM_TRK_3 is used to send post latency. */

    private FieldEntry _fEntry; /* field entry */
    private OmmUInt _fidUIntValue; /* storage for UInt */
    @SuppressWarnings("unused")
	private OmmInt _fidIntValue; /* storage for Int */
    @SuppressWarnings("unused")
	private OmmReal _fidRealValue; /* storage for Real */
    @SuppressWarnings("unused")
	private OmmEnum  _fidEnumValue; /* storage for Enum */
    @SuppressWarnings("unused")
	private OmmDate _fidDateValue; /* storage for Date */
    @SuppressWarnings("unused")
	private OmmTime _fidTimeValue; /* storage for Time */
    @SuppressWarnings("unused")
	private OmmDateTime _fidDateTimeValue; /* storage for DateTime */
    @SuppressWarnings("unused")
	private OmmFloat _fidFloatValue; /* storage for Float */
    @SuppressWarnings("unused")
    private OmmDouble _fidDoubleValue; /* storage for Double */
    @SuppressWarnings("unused")
    private OmmQos _fidQosValue; /* storage for QOS */
    @SuppressWarnings("unused")
    private OmmState _fidStateValue; /* storage for State */
    @SuppressWarnings("unused")
    private OmmBuffer _fidBufferValue; /* storage for Buffer */
    @SuppressWarnings("unused")
    private OmmRmtes _fidRmtesValue; /* storage for Buffer */
    @SuppressWarnings("unused")
    private OmmUtf8 _fidUtf8Value; /* storage for Buffer */
    @SuppressWarnings("unused")
    private OmmAscii _fidAsciiValue; /* storage for Buffer */
    private PostUserInfo _postUserInfo; /* post user information */
    
    /**
     * Instantiates a new market price decoder.
     *
     * @param postUserInfo the post user info
     */
    public MarketPriceDecoder(PostUserInfo postUserInfo)
    {
    	_postUserInfo = postUserInfo;
	}

    /**
     *  Decode the update.
     *
     * @param msg the EMA msg
     * @param fieldList the field list
     * @param _consThreadInfo the cons thread info
     * @param downcastDecoding the downcast decoding
     * @return true, if successful
     */
	public boolean decodeResponse(Msg msg, FieldList fieldList, ConsumerThreadInfo _consThreadInfo, boolean downcastDecoding)
	{
		long timeTracker = 0;
		long postTimeTracker = 0;
        long genMsgTimeTracker = 0;

    	try
		{
    		
			/* decode field list */
	        Iterator<FieldEntry> iter = fieldList.iteratorByRef();
	        
	        if (downcastDecoding)
	        {
				while (iter.hasNext())
				{
					_fEntry =  iter.next();
					if (Data.DataCode.NO_CODE == _fEntry.code())
					{
						switch (_fEntry.loadType())
						{
							case DataTypes.INT:
								_fidIntValue = (OmmInt)_fEntry.load();
								break;
							case DataTypes.UINT:
								_fidUIntValue = (OmmUInt)_fEntry.load();;
								break;
							case DataTypes.FLOAT:
								_fidFloatValue = (OmmFloat)_fEntry.load();
								break;
							case DataTypes.DOUBLE:
								_fidDoubleValue = (OmmDouble)_fEntry.load();
								break;
							case DataTypes.REAL:
								_fidRealValue = (OmmReal)_fEntry.load();
								break;
							case DataTypes.DATE:
								_fidDateValue = (OmmDate)_fEntry.load();
								break;
							case DataTypes.TIME:
								_fidTimeValue = (OmmTime)_fEntry.load(); 
								break;
							case DataTypes.DATETIME:
								_fidDateTimeValue = (OmmDateTime)_fEntry.load();
								break;
							case DataTypes.QOS:
								_fidQosValue = (OmmQos)_fEntry.load();
								break;
							case DataTypes.STATE:
								_fidStateValue = (OmmState)_fEntry.load();
								break;
							case DataTypes.ENUM:
								_fidEnumValue = (OmmEnum)_fEntry.load();
								break;
							case DataTypes.BUFFER:
								_fidBufferValue = (OmmBuffer)_fEntry.load();
								break;
							case DataTypes.ASCII:
								_fidAsciiValue = (OmmAscii)_fEntry.load();
								break;
							case DataTypes.UTF8:
								_fidUtf8Value = (OmmUtf8)_fEntry.load();
								break;
							case DataTypes.RMTES:
								_fidRmtesValue = (OmmRmtes)_fEntry.load();
								break;
							default: 
								System.out.printf("Error: Unhandled data type %s(%d) in field with ID %d.\n", DataType.asString(_fEntry.loadType()), _fEntry.loadType(), _fEntry.fieldId());
								return false;
						}
						
						if (msg.dataType() == DataTypes.UPDATE_MSG)
						{
							if(_fEntry.fieldId() == TIM_TRK_1_FID)
								timeTracker = _fidUIntValue.longValue();
							if(_fEntry.fieldId() == TIM_TRK_2_FID)
								postTimeTracker = _fidUIntValue.longValue();
						}
						else if(msg.dataType() == DataTypes.GENERIC_MSG)
						{
						    if(_fEntry.fieldId() == TIM_TRK_3_FID)
						        genMsgTimeTracker = _fidUIntValue.longValue();
						}
					}
				}
			}
	        else
	        {
	        	while (iter.hasNext())
				{
					_fEntry =  iter.next();
					if (Data.DataCode.NO_CODE == _fEntry.code())
					{
						switch (_fEntry.loadType())
						{
							case DataTypes.INT:
								_fidIntValue = _fEntry.ommIntValue();
								break;
							case DataTypes.UINT:
								_fidUIntValue = _fEntry.ommUIntValue();
								break;
							case DataTypes.FLOAT:
								_fidFloatValue = _fEntry.ommFloatValue();
								break;
							case DataTypes.DOUBLE:
								_fidDoubleValue = _fEntry.ommDoubleValue();
								break;
							case DataTypes.REAL:
								_fidRealValue = _fEntry.real();
								break;
							case DataTypes.DATE:
								_fidDateValue = _fEntry.date();
								break;
							case DataTypes.TIME:
								_fidTimeValue = _fEntry.time();
								break;
							case DataTypes.DATETIME:
								_fidDateTimeValue = _fEntry.dateTime();
								break;
							case DataTypes.QOS:
								_fidQosValue = _fEntry.qos();
								break;
							case DataTypes.STATE:
								_fidStateValue = _fEntry.state();
								break;
							case DataTypes.ENUM:
								_fidEnumValue = _fEntry.ommEnumValue();
								break;
							case DataTypes.BUFFER:
								_fidBufferValue = _fEntry.buffer();
								break;
							case DataTypes.ASCII:
								_fidAsciiValue = _fEntry.ascii();
								break;
							case DataTypes.UTF8:
								_fidUtf8Value = _fEntry.utf8();
								break;
							case DataTypes.RMTES:
								_fidRmtesValue = _fEntry.rmtes();
								break;
							default: 
								System.out.printf("Error: Unhandled data type %s(%d) in field with ID %d.\n", DataType.asString(_fEntry.loadType()), _fEntry.loadType(), _fEntry.fieldId());
								return false;
						}
						
						if (msg.dataType() == DataTypes.UPDATE_MSG)
						{
							if(_fEntry.fieldId() == TIM_TRK_1_FID)
								timeTracker = _fidUIntValue.longValue();
							if(_fEntry.fieldId() == TIM_TRK_2_FID)
								postTimeTracker = _fidUIntValue.longValue();
						}
						else if(msg.dataType() == DataTypes.GENERIC_MSG)
						{
						    if(_fEntry.fieldId() == TIM_TRK_3_FID)
						        genMsgTimeTracker = _fidUIntValue.longValue();
						}
					}
				}
	        }
		}
		catch (OmmException excp)
		{
			System.out.println(excp);
		}
		
		if (timeTracker > 0)
			_consThreadInfo.timeRecordSubmit(_consThreadInfo.latencyRecords(), timeTracker, System.nanoTime()/1000, 1);
		
		if(postTimeTracker > 0 && checkPostUserInfo(msg))
			_consThreadInfo.timeRecordSubmit(_consThreadInfo.postLatencyRecords(), postTimeTracker, System.nanoTime()/1000, 1);
        if(genMsgTimeTracker > 0)
            _consThreadInfo.timeRecordSubmit(_consThreadInfo.genMsgLatencyRecords(), genMsgTimeTracker, System.nanoTime()/1000, 1);

		return true;
	}

	/* Checks for PostUserInfo. */
	private boolean checkPostUserInfo(Msg msg)
	{
		/* If post user info is present, make sure it matches our info.
		 * Otherwise, assume any posted information present came from us anyway(return true). */
		switch(msg.dataType())
		{
			case DataTypes.REFRESH_MSG:
				RefreshMsg refreshMsg = (RefreshMsg)msg;
				return (!refreshMsg.hasPublisherId() ||
				 refreshMsg.publisherIdUserAddress() == _postUserInfo.userAddr &&
				 refreshMsg.publisherIdUserId() == _postUserInfo.userId);
			case DataTypes.UPDATE_MSG:
				UpdateMsg updateMsg = (UpdateMsg)msg;
				return (!updateMsg.hasPublisherId() ||
				 updateMsg.publisherIdUserAddress() == _postUserInfo.userAddr &&
				 updateMsg.publisherIdUserId() == _postUserInfo.userId);
			default:
				return true;
		}
	}
}
