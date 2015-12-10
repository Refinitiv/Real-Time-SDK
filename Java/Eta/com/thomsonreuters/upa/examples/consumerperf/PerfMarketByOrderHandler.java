package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.UInt;

public class PerfMarketByOrderHandler
{
	private final int CPU_FREQ_FID  = 3901;
	private final int TIM_TRK_1_FID  = 3902;
	
	private final FieldList _fList = CodecFactory.createFieldList();
	private final FieldEntry _fEntry = CodecFactory.createFieldEntry();
	private final Map _map = CodecFactory.createMap();
	private final MapEntry _mEntry = CodecFactory.createMapEntry();
	private final Buffer _key = CodecFactory.createBuffer();
	private final UInt _fidUIntValue = CodecFactory.createUInt();
	private final Int _fidIntValue = CodecFactory.createInt();
	private final Real _fidRealValue = CodecFactory.createReal();
	private final Enum _fidEnumValue  = CodecFactory.createEnum();
	private final DateTime _fidDateTimeValue = CodecFactory.createDateTime();

	public int decodeUpdate(int decodeType, DecodeIterator iter, Msg msg, ConsumerInfo cInfo)
	{
		switch(decodeType)
		{
			case DecodeType.PARTIAL: return partialDecode(iter, msg, cInfo); 
			case DecodeType.HEADER_ONLY: return headerDecode(iter, msg, cInfo); 
			case DecodeType.FULL: return fullDecode(iter, msg, cInfo); 
			default: return CodecReturnCodes.FAILURE;
		}
	}
	
	private int partialDecode(DecodeIterator iter, Msg msg, ConsumerInfo consInfo)
	{
		_fList.clear();
		_fEntry.clear();
		_map.clear();
		_mEntry.clear();
		int ret = 0;
		_key.clear();
		_fidUIntValue.clear();
		double timeTracker = 0;
		double cpuTicks = 0;

		/* decode field list */
		if((ret = _map.decode(iter)) == CodecReturnCodes.SUCCESS)
		{
			if(_map.checkHasSetDefs() == true)
			{
				consInfo.fListSetDef.clear();
				if((ret = consInfo.fListSetDef.decode(iter)) != CodecReturnCodes.SUCCESS)
					return ret;
			}
			
			if(_map.checkHasSummaryData() == true)
			{
				if((ret = _fList.decode(iter, consInfo.fListSetDef)) == CodecReturnCodes.SUCCESS)
				{
					while ((ret = _fEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
					{	
						if(_fEntry.fieldId() == TIM_TRK_1_FID)
							if((ret = _fidUIntValue.decode(iter)) == CodecReturnCodes.SUCCESS)
								timeTracker = _fidUIntValue.toLong();
							else
								return ret;
						
						if(_fEntry.fieldId() == CPU_FREQ_FID)
							if((ret = _fidUIntValue.decode(iter)) == CodecReturnCodes.SUCCESS)
								cpuTicks = _fidUIntValue.toLong();
							else
								return ret;
					}
				}
				else
				{
					return ret;
				}
			}
			
			while((ret = _mEntry.decode(iter, _key)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if(_mEntry.action() != MapEntryActions.DELETE)
				{
					if ((ret = _fList.decode(iter, consInfo.fListSetDef)) == CodecReturnCodes.SUCCESS)
					{
						while ((ret = _fEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
						{	
						}
					}
					else
					{
						return ret;
					}
				}
			}
		}
		else
		{
			return ret;
		}
		consInfo.stats.updateCount++;

		if(msg.streamId() == consInfo.latStreamId)
		{
			long currentTime = RsslTimeUtilsJNI.getTicks();
			double msgLat = ((double)currentTime - timeTracker)/cpuTicks;
			double newMean;

			consInfo.stats.latencyCount++;

			newMean = consInfo.stats.latencyAvg + ((msgLat - consInfo.stats.latencyAvg)/(double)consInfo.stats.latencyCount);
			consInfo.stats.latencyRunningSum += (msgLat - consInfo.stats.latencyAvg) * (msgLat - newMean);
			consInfo.stats.latencyAvg = newMean;

			if(consInfo.stats.latencyMax < msgLat)
				consInfo.stats.latencyMax = msgLat;
			if(consInfo.stats.latencyMin > msgLat)
				consInfo.stats.latencyMin = msgLat;
			
			consInfo.handlePing();
		}
		return CodecReturnCodes.SUCCESS;
	}

	private int headerDecode(DecodeIterator iter, Msg msg, ConsumerInfo consInfo)
	{
		_fList.clear();
		_fEntry.clear();
		_map.clear();
		int ret = 0;
		_fidUIntValue.clear();
		double timeTracker = 0;
		double cpuTicks = 0;
		
		consInfo.stats.updateCount++;
		
		consInfo.fListSetDef.clear();

		if(msg.streamId() == consInfo.latStreamId)
		{
			/* decode field list */
			if((ret = _map.decode(iter)) == CodecReturnCodes.SUCCESS)
			{
				if(_map.checkHasSetDefs() == true)
				{
					if((ret = consInfo.fListSetDef.decode(iter)) != CodecReturnCodes.SUCCESS)
						return ret;
				}
				
				if(_map.checkHasSummaryData() == true)
				{
					if((ret = _fList.decode(iter, null)) == CodecReturnCodes.SUCCESS)
					{
						while ((ret = _fEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
						{	
							if(_fEntry.fieldId() == TIM_TRK_1_FID)
								if((ret = _fidUIntValue.decode(iter)) == CodecReturnCodes.SUCCESS)
									timeTracker = _fidUIntValue.toLong();
								else
									return ret;
							
							if(_fEntry.fieldId() == CPU_FREQ_FID)
								if((ret = _fidUIntValue.decode(iter)) == CodecReturnCodes.SUCCESS)
									cpuTicks = _fidUIntValue.toLong();
								else
									return ret;
						}
					}
					else
					{
						return ret;
					}
				}

				if(timeTracker > 0 && cpuTicks > 0)
				{					
					long currentTime = RsslTimeUtilsJNI.getTicks();
					double msgLat = ((double)currentTime - timeTracker)/cpuTicks;
					double newMean;
									
					consInfo.stats.latencyCount++;

					newMean = consInfo.stats.latencyAvg + ((msgLat - consInfo.stats.latencyAvg)/(double)consInfo.stats.latencyCount);
					consInfo.stats.latencyRunningSum += (msgLat - consInfo.stats.latencyAvg) * (msgLat - newMean);
					consInfo.stats.latencyAvg = newMean;

					if(consInfo.stats.latencyMax < msgLat)
						consInfo.stats.latencyMax = msgLat;
					if(consInfo.stats.latencyMin > msgLat)
						consInfo.stats.latencyMin = msgLat;
					
					consInfo.handlePing();
					
					return CodecReturnCodes.SUCCESS;
				}
			}
			else
			{
				return ret;
			}	
		}

		return CodecReturnCodes.SUCCESS;
	}

	private int fullDecode(DecodeIterator iter, Msg msg, ConsumerInfo consInfo)
	{
		_fList.clear();
		_fEntry.clear();
		_map.clear();
		_mEntry.clear();
		int ret = 0;
		_key.clear();
		int dataType;
		_fidUIntValue.clear();
		_fidIntValue.clear();
		_fidRealValue.clear();
		_fidEnumValue.clear();
		_fidDateTimeValue.clear();
		DictionaryEntry dictionaryEntry = null;

		double timeTracker = 0;
		double cpuTicks = 0;
		
		consInfo.stats.updateCount++;

		/* decode field list */
				
		if((ret = _map.decode(iter)) == CodecReturnCodes.SUCCESS)
		{
			if(_map.checkHasSetDefs() == true)
			{
				consInfo.fListSetDef.clear();
				if((ret = consInfo.fListSetDef.decode(iter)) != CodecReturnCodes.SUCCESS)
					return ret;
			}
			if(_map.checkHasSummaryData() == true)
			{
				if((ret = _fList.decode(iter, consInfo.fListSetDef)) == CodecReturnCodes.SUCCESS)
				{
					while ((ret = _fEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
					{	
						/* get dictionary entry */
						dictionaryEntry = consInfo.fieldDictionary.entry(_fEntry.fieldId());
						/* decode and print out fid value */
						dataType = dictionaryEntry.rwfType();
						switch (dataType)
						{
							case DataTypes.UINT:
								if((ret = _fidUIntValue.decode(iter)) < CodecReturnCodes.SUCCESS)
									return ret;
								else
									break;
							case DataTypes.INT:
								if((ret = _fidIntValue.decode(iter)) < CodecReturnCodes.SUCCESS)
									return ret;
								else
									break;
							case DataTypes.REAL:
								if ((ret = _fidRealValue.decode(iter)) < CodecReturnCodes.SUCCESS)
									return ret;
								else
									break;
							case DataTypes.ENUM:
								if ((ret = _fidEnumValue.decode(iter)) < CodecReturnCodes.SUCCESS )
									return ret;
								else
									break;
							case DataTypes.DATE:
								if ((ret = _fidDateTimeValue.date().decode(iter)) < CodecReturnCodes.SUCCESS)
									return ret;
								else
									break;
							case DataTypes.TIME:
								if ((ret = _fidDateTimeValue.time().decode(iter)) < CodecReturnCodes.SUCCESS )
									return ret;
								else
									break;
							case DataTypes.BUFFER:
							case DataTypes.ASCII_STRING:
							case DataTypes.UTF8_STRING:
							case DataTypes.RMTES_STRING:
							default:
								break;
						}

					if(_fEntry.fieldId() == TIM_TRK_1_FID)
						timeTracker = _fidUIntValue.toLong();
					if(_fEntry.fieldId() == CPU_FREQ_FID)
						cpuTicks = _fidUIntValue.toLong();
					}
				}
				else
				{
					return ret;
				}
			}

			while((ret = _mEntry.decode(iter, _key)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if (_mEntry.action() != MapEntryActions.DELETE)
				{
					if((ret = _fList.decode(iter, consInfo.fListSetDef)) == CodecReturnCodes.SUCCESS)
					{
						while ((ret = _fEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
						{	
							/* get dictionary entry */
							dictionaryEntry = consInfo.fieldDictionary.entry(_fEntry.fieldId());
							/* decode and print out fid value */
							dataType = dictionaryEntry.rwfType();
							
							switch (dataType)
							{
								case DataTypes.UINT:
									if((ret = _fidUIntValue.decode(iter)) < CodecReturnCodes.SUCCESS)
										return ret;
									else
										break;
								case DataTypes.INT:
									if((ret = _fidIntValue.decode(iter)) < CodecReturnCodes.SUCCESS)
										return ret;
									else
										break;
								case DataTypes.REAL:
									if ((ret = _fidRealValue.decode(iter)) < CodecReturnCodes.SUCCESS)
										return ret;
									else
										break;
								case DataTypes.ENUM:
									if ((ret = _fidEnumValue.decode(iter)) < CodecReturnCodes.SUCCESS )
										return ret;
									else
										break;
								case DataTypes.DATE:
									if ((ret = _fidDateTimeValue.date().decode(iter)) < CodecReturnCodes.SUCCESS)
										return ret;
									else
										break;
								case DataTypes.TIME:
									if ((ret = _fidDateTimeValue.time().decode(iter)) < CodecReturnCodes.SUCCESS )
										return ret;
									else							
										break;
								case DataTypes.BUFFER:
								case DataTypes.ASCII_STRING:
								case DataTypes.UTF8_STRING:
								case DataTypes.RMTES_STRING:
								default:
									break;
							}
						}
					}
					else
					{
						return ret;
					}
				}
			}
		}
		else
		{
			return ret;
		}
		
		if(msg.streamId() == consInfo.latStreamId)
		{
			long currentTime = RsslTimeUtilsJNI.getTicks();
			double msgLat = ((double)currentTime - timeTracker)/cpuTicks;
			double newMean;
			
			consInfo.stats.latencyCount++;

			newMean = consInfo.stats.latencyAvg + ((msgLat - consInfo.stats.latencyAvg)/(double)consInfo.stats.latencyCount);
			consInfo.stats.latencyRunningSum += (msgLat - consInfo.stats.latencyAvg) * (msgLat - newMean);
			consInfo.stats.latencyAvg = newMean;

			if(consInfo.stats.latencyMax < msgLat)
				consInfo.stats.latencyMax = msgLat;
			if(consInfo.stats.latencyMin > msgLat)
				consInfo.stats.latencyMin = msgLat;
			
			consInfo.handlePing();
		}
		return CodecReturnCodes.SUCCESS;
	}
}
