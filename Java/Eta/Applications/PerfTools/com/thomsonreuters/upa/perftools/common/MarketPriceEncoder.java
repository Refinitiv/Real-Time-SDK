package com.thomsonreuters.upa.perftools.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Float;
import com.thomsonreuters.upa.codec.Double;

/**
 * Provides encoding of a MarketPrice domain data payload.
 */
public class MarketPriceEncoder
{
	private final int TIM_TRK_1_FID = 3902; // Field TIM_TRK_1 is used to send update latency.
	private final int TIM_TRK_2_FID = 3903; // Field TIM_TRK_2 is used to send post latency.
	private final int TIM_TRK_3_FID = 3904; // Field TIM_TRK_3 is used to send generic msg latency.

    private XmlMsgData _xmlMsgData;         // XML file message data
	private FieldList _fList;               // field list
	private FieldEntry _fEntry;             // field entry
	private UInt _tempUInt;                 // temporary storage for UInt
	
	{
		_fList = CodecFactory.createFieldList();
		_fEntry = CodecFactory.createFieldEntry();
		_tempUInt = CodecFactory.createUInt();
	}

	public MarketPriceEncoder(XmlMsgData msgData)
	{
		_xmlMsgData = msgData;
	}

	/** Get the next MarketPrice post(moves over the list). */
	public MarketPriceMsg nextPost(MarketPriceItem mpItem)
	{ 
		int mpItemIndex = mpItem.iMsg();
		MarketPriceMsg mpMsg = _xmlMsgData.marketPricePostMsgs()[mpItemIndex++];
		
		if (mpItemIndex == _xmlMsgData.marketPricePostMsgCount())
			mpItemIndex = 0;
		
		mpItem.iMsg(mpItemIndex);
			
		return mpMsg;
	}

	/** Get the next MarketPrice generic msg(moves over the list). */
	public MarketPriceMsg nextGenMsg(MarketPriceItem mpItem)
	{ 
		int mpItemIndex = mpItem.iMsg();
		MarketPriceMsg mpMsg = _xmlMsgData.marketPriceGenMsgs()[mpItemIndex++];
		
		if (mpItemIndex == _xmlMsgData.marketPriceGenMsgCount())
			mpItemIndex = 0;
		
		mpItem.iMsg(mpItemIndex);
			
		return mpMsg;
	}
	
	/** Encodes a MarketPrice data body for a message. */
	public int encodeDataBody(EncodeIterator _eIter, MarketPriceMsg mpMsg, int msgClass, long encodeStartTime)
	{
		// encode field list
		_fList.clear();
		_fEntry.clear();
		_fList.applyHasStandardData();
		int ret = _fList.encodeInit(_eIter, null, 0);
		if (ret < CodecReturnCodes.SUCCESS)
			return ret;

		for(int i = 0; i < mpMsg.fieldEntryCount(); ++i)
		{
		    switch (mpMsg.fieldEntries()[i].fieldEntry().dataType())
		    {
		        case DataTypes.INT:
		            ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Int)mpMsg.fieldEntries()[i].value());
		            break;
                case DataTypes.UINT:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (UInt)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.REAL:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Real)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.DATE:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Date)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.TIME:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Time)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.DATETIME:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (DateTime)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.QOS:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Qos)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.STATE:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (State)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.ENUM:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Enum)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.BUFFER:
                case DataTypes.RMTES_STRING:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Buffer)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.FLOAT:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Float)mpMsg.fieldEntries()[i].value());
                    break;
                case DataTypes.DOUBLE:
                    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter, (Double)mpMsg.fieldEntries()[i].value());
                    break;
                default:
                    ret = CodecReturnCodes.FAILURE;
                    break;
		    }
		    
			if (ret < CodecReturnCodes.SUCCESS)
				return ret;
		}

		// Include the latency time fields in refreshes.
		if (msgClass == MsgClasses.REFRESH)
		{
			_fEntry.fieldId(TIM_TRK_1_FID);
			_fEntry.dataType(DataTypes.UINT);
			if ((ret = _fEntry.encodeBlank(_eIter)) < CodecReturnCodes.SUCCESS)
				return ret;

			_fEntry.fieldId(TIM_TRK_2_FID);
			_fEntry.dataType(DataTypes.UINT);
			ret = _fEntry.encodeBlank(_eIter);
			if (ret < CodecReturnCodes.SUCCESS)
				return ret;

			_fEntry.fieldId(TIM_TRK_3_FID);
			_fEntry.dataType(DataTypes.UINT);
			ret = _fEntry.encodeBlank(_eIter);
			if (ret < CodecReturnCodes.SUCCESS)
				return ret;
		}


		if (encodeStartTime > 0)
		{
			// Encode the latency timestamp.
			_fEntry.fieldId((msgClass == MsgClasses.UPDATE) ? TIM_TRK_1_FID : (msgClass == MsgClasses.GENERIC) ? TIM_TRK_3_FID : TIM_TRK_2_FID);
			_fEntry.dataType(DataTypes.UINT);
			_tempUInt.value(encodeStartTime);
			ret = _fEntry.encode(_eIter, _tempUInt);
			if (ret < CodecReturnCodes.SUCCESS)
				return ret;
		}

		// complete encode field list
		ret = _fList.encodeComplete(_eIter, true);
		if (ret < CodecReturnCodes.SUCCESS)
			return ret;

		return CodecReturnCodes.SUCCESS;
	}
}
