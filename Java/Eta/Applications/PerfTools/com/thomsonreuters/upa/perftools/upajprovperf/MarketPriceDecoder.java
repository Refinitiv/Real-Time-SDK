package com.thomsonreuters.upa.perftools.upajprovperf;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EnumType;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.perftools.common.ProviderThreadInfo;

/**
 * This is the market price decoder for the upajProvPerf application.
 */
public class MarketPriceDecoder
{
	private static final int TIM_TRK_3_FID = 3904; /* Field TIM_TRK_3 is used to send generic message latency. */

	private FieldList _fList; /* field list */
	private FieldEntry _fEntry; /* field entry */
    private UInt _fidUIntValue; /* storage for UInt */
    private Int _fidIntValue; /* storage for Int */
    private Real _fidRealValue; /* storage for Real */
    private com.thomsonreuters.upa.codec.Enum _fidEnumValue; /* storage for Enum */
    private Date _fidDateValue; /* storage for Date */
    private Time _fidTimeValue; /* storage for Time */
    private DateTime _fidDateTimeValue; /* storage for DateTime */
    private com.thomsonreuters.upa.codec.Float _fidFloatValue; /* storage for Float */
    private com.thomsonreuters.upa.codec.Double _fidDoubleValue; /* storage for Double */
    private Qos _fidQosValue; /* storage for QOS */
    private State _fidStateValue; /* storage for State */
    private Buffer _fidBufferValue; /* storage for Buffer */
    private EnumType _enumTypeInfo; /* EnumType dictionary information  */
    
    {
    	_fList = CodecFactory.createFieldList();
    	_fEntry = CodecFactory.createFieldEntry();
    	_fidUIntValue = CodecFactory.createUInt();
    	_fidIntValue = CodecFactory.createInt();
    	_fidRealValue = CodecFactory.createReal();
    	_fidEnumValue = CodecFactory.createEnum();
    	_fidDateValue = CodecFactory.createDate();
    	_fidTimeValue = CodecFactory.createTime();
    	_fidDateTimeValue = CodecFactory.createDateTime();
    	_fidFloatValue = CodecFactory.createFloat();
    	_fidDoubleValue = CodecFactory.createDouble();
    	_fidQosValue = CodecFactory.createQos();
    	_fidStateValue = CodecFactory.createState();
    	_fidBufferValue = CodecFactory.createBuffer();
    }
    
    public MarketPriceDecoder()
    {
	}

	/* Update the latency statistics. */
    private void updateLatencyStats(ProviderThreadInfo provThreadInfo, long timeTracker, int msgClass)
    {
    	long currentTime;
    	long unitsPerMicro;

    	currentTime = System.nanoTime()/1000;
    	unitsPerMicro = 1;
/*
        switch(msgClass)
    	{
    	    case MsgClasses.UPDATE:
            case MsgClasses.POST:
            case MsgClasses.GENERIC:
    	}
*/
    	provThreadInfo.timeRecordSubmit(provThreadInfo.genMsgLatencyRecords(), timeTracker, currentTime, unitsPerMicro);
    }
    
    /** Decode the update. */
	public int decodeUpdate(DecodeIterator iter, Msg msg, ProviderThreadInfo provThreadInfo)
	{
		int ret = 0;
		DictionaryEntry dictionaryEntry = null;

		long timeTracker = 0;

		/* decode field list */
		if ((ret = _fList.decode(iter, null)) == CodecReturnCodes.SUCCESS)
		{
			if (ret != CodecReturnCodes.SUCCESS) return ret;

			while ((ret = _fEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
			{	
				/* get dictionary entry */
				dictionaryEntry = provThreadInfo.dictionary().entry(_fEntry.fieldId());

				if  (dictionaryEntry == null)
				{
					System.out.printf("Error: Decoded field ID %d not present in dictionary.\n", _fEntry.fieldId());
					return CodecReturnCodes.FAILURE;
				}

				/* decode fid value */
				if ((ret = decodeFidValue(dictionaryEntry, iter, provThreadInfo)) < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}

				if (msg.msgClass() == MsgClasses.GENERIC && ret != CodecReturnCodes.BLANK_DATA)
				{
					if(_fEntry.fieldId() == TIM_TRK_3_FID)
						timeTracker = _fidUIntValue.toLong();
				}
			}
		}
		else
		{
			return ret;
		}
		
		if (timeTracker > 0)
			updateLatencyStats(provThreadInfo, timeTracker, MsgClasses.GENERIC);

		return CodecReturnCodes.SUCCESS;
	}

	/* Decode FID value. */
	private int decodeFidValue(DictionaryEntry dictionaryEntry, DecodeIterator iter, ProviderThreadInfo provThreadInfo)
	{
		int ret = CodecReturnCodes.SUCCESS;
		
		int dataType = dictionaryEntry.rwfType();
		switch (dataType)
		{
			case DataTypes.INT:
				if((ret = _fidIntValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.UINT:
				if((ret = _fidUIntValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.FLOAT:
				if ((ret = _fidFloatValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.DOUBLE:
				if ((ret = _fidDoubleValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.REAL:
				if ((ret = _fidRealValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.DATE:
				if ((ret = _fidDateValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.TIME:
				if ((ret = _fidTimeValue.decode(iter)) < CodecReturnCodes.SUCCESS )
					return ret;
				break;
			case DataTypes.DATETIME:
				if ((ret = _fidDateTimeValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.QOS:
				if ((ret = _fidQosValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.STATE:
				if ((ret = _fidStateValue.decode(iter)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			case DataTypes.ENUM:
			{
				if ((ret = _fidEnumValue.decode(iter)) < CodecReturnCodes.SUCCESS )
					return ret;

				if (ret == CodecReturnCodes.BLANK_DATA)
					break;

				_enumTypeInfo = provThreadInfo.dictionary().entryEnumType(dictionaryEntry, _fidEnumValue);
				if (_enumTypeInfo != null)
					_fidBufferValue = _enumTypeInfo.display();
				break;
			}
			case DataTypes.BUFFER:
			case DataTypes.ASCII_STRING:
			case DataTypes.UTF8_STRING:
			case DataTypes.RMTES_STRING:
				if ((ret = _fidBufferValue.decode(iter)) < CodecReturnCodes.SUCCESS )
					return ret;
				break;
			default:
				System.out.printf("Error: Unhandled data type %s(%d) in field with ID %d.\n", 
						DataTypes.toString(dataType), dataType, _fEntry.fieldId());
				return CodecReturnCodes.FAILURE;
		}
		
		return ret;
	}

}
