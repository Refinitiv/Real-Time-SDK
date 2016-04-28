package com.thomsonreuters.upa.perftools.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.UInt;

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
		    ret = mpMsg.fieldEntries()[i].fieldEntry().encode(_eIter,
		                                                      mpMsg.fieldEntries()[i].value());
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
