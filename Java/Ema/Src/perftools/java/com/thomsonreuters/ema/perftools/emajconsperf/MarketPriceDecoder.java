package com.thomsonreuters.ema.perftools.emajconsperf;

import java.util.Iterator;

import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmAscii;
import com.thomsonreuters.ema.access.OmmBuffer;
import com.thomsonreuters.ema.access.OmmDate;
import com.thomsonreuters.ema.access.OmmDateTime;
import com.thomsonreuters.ema.access.OmmDouble;
import com.thomsonreuters.ema.access.OmmEnum;
import com.thomsonreuters.ema.access.OmmError;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmFloat;
import com.thomsonreuters.ema.access.OmmInt;
import com.thomsonreuters.ema.access.OmmQos;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmRmtes;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.OmmTime;
import com.thomsonreuters.ema.access.OmmUInt;
import com.thomsonreuters.ema.access.OmmUtf8;
import com.thomsonreuters.ema.perftools.common.PostUserInfo;
import com.thomsonreuters.ema.access.DataType.DataTypes;

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
    private OmmInt _fidIntValue; /* storage for Int */
    private OmmReal _fidRealValue; /* storage for Real */
    private OmmEnum  _fidEnumValue; /* storage for Enum */
    private OmmDate _fidDateValue; /* storage for Date */
    private OmmTime _fidTimeValue; /* storage for Time */
    private OmmDateTime _fidDateTimeValue; /* storage for DateTime */
    private OmmFloat _fidFloatValue; /* storage for Float */
    private OmmDouble _fidDoubleValue; /* storage for Double */
    private OmmQos _fidQosValue; /* storage for QOS */
    private OmmState _fidStateValue; /* storage for State */
    private OmmBuffer _fidBufferValue; /* storage for Buffer */
    private OmmRmtes _fidRmtesValue; /* storage for Buffer */
    private OmmUtf8 _fidUtf8Value; /* storage for Buffer */
    private OmmError _fidErrorValue; /* storage for Buffer */
    private OmmArray _fidArrayValue; /* storage for Buffer */
    private OmmAscii _fidAsciiValue; /* storage for Buffer */
    private PostUserInfo _postUserInfo; /* post user information */
    
    public MarketPriceDecoder(PostUserInfo postUserInfo)
    {
    	_postUserInfo = postUserInfo;
	}

    /** Decode the update. */
	public boolean decodeResponse(int msgType, FieldList fieldList, ConsumerThreadInfo _consThreadInfo, boolean downcastDecoding)
	{
		long timeTracker = 0;
		long postTimeTracker = 0;
        long genMsgTimeTracker = 0;

    	try
		{
    		
			/* decode field list */
	        Iterator<FieldEntry> iter = fieldList.iterator();
	        
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
						
						if (msgType == DataTypes.UPDATE_MSG)
						{
							if(_fEntry.fieldId() == TIM_TRK_1_FID)
								timeTracker = _fidUIntValue.longValue();
							if(_fEntry.fieldId() == TIM_TRK_2_FID)
								postTimeTracker = _fidUIntValue.longValue();
						}
						else if(msgType == DataTypes.GENERIC_MSG)
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
						
						if (msgType == DataTypes.UPDATE_MSG)
						{
							if(_fEntry.fieldId() == TIM_TRK_1_FID)
								timeTracker = _fidUIntValue.longValue();
							if(_fEntry.fieldId() == TIM_TRK_2_FID)
								postTimeTracker = _fidUIntValue.longValue();
						}
						else if(msgType == DataTypes.GENERIC_MSG)
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
		
		//TODO
//		if(postTimeTracker > 0 && checkPostUserInfo(msg))
//			_consThreadInfo.timeRecordSubmit(_consThreadInfo.genMsgLatencyRecords(), timeTracker, System.nanoTime()/1000, 1);
//        if(genMsgTimeTracker > 0)
//            _consThreadInfo.timeRecordSubmit(_consThreadInfo.postLatencyRecords(), timeTracker, System.nanoTime()/1000, 1);

		return true;
	}

	/* Checks for PostUserInfo. */
	private boolean checkPostUserInfo(Msg msg)
	{
		//TODO
		return true;
	}
}
