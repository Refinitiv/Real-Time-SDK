package com.thomsonreuters.upa.perftools.common;

import java.io.IOException;

import org.xmlpull.v1.XmlPullParserException;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;

/** Manages message data information from an XML file. */
public class XmlMsgData
{
	class XmlMsgDataReader extends XMLReader
	{
		private final int INT_SIZE = 8;
		private final int UINT_SIZE = 8;
		private final int FLOAT_SIZE = 4;
		private final int DOUBLE_SIZE = 8;
		private final int REAL_SIZE = 16;
		private final int DATE_SIZE = 4;
		private final int TIME_SIZE = 6;
		private final int DATETIME_SIZE = 10;
		private final int QOS_SIZE = 8;
		private final int STATE_SIZE = 12;
		private final int ENUM_SIZE = 2;
		private final int BUFFER_SIZE = 8;

		/** XML Pull Parser beginSectionRead(). */
		void beginSectionRead() throws IOException, XmlPullParserException
		{
			String tag = XPP.getName();
			
			if (tag.equals("marketPriceMsgList"))
	    	{
				// get msg counts (updates, posts, and generic msgs)
				getFileMessageCounts();

				_marketPriceUpdateMsgs = new MarketPriceMsg[_fileUpdateCount];
				_marketPricePostMsgs = new MarketPriceMsg[_filePostCount];
				_marketPriceGenMsgs = new MarketPriceMsg[_fileGenMsgCount];
				_parsingMPMsgs = true;
				
				_hasMarketPrice = true;
	    	}
			
			if (_parsingMPMsgs)
			{
				if (tag.equals("refreshMsg"))
				{
					_parsingRefreshMsg = true;
				}
				else if (tag.equals("updateMsg") && XPP.getDepth() == 3)
				{
					_parsingUpdateMsg = true;
				}
				else if (tag.equals("postMsg"))
				{
					_parsingPostMsg = true;
				}
				else if (tag.equals("genMsg"))
				{
					_parsingGenMsg = true;
				}
				else if (tag.equals("fieldList"))
				{
					// get field entry count
					getFieldEntryCount();

					if (_parsingRefreshMsg)
					{
						_marketPriceRefreshMsg = new MarketPriceMsg(_fileFieldEntryCount);
					}
					else if (_parsingUpdateMsg)
					{
						_updateMsg = new MarketPriceMsg(_fileFieldEntryCount);
					}
					else if (_parsingPostMsg)
					{
						_postMsg = new MarketPriceMsg(_fileFieldEntryCount);
					}
					else if (_parsingGenMsg)
					{
						_genMsg = new MarketPriceMsg(_fileFieldEntryCount);
					}
				}
				else if (tag.equals("fieldEntry"))
				{
		     		if (getAttributeCount() == 0)
		     			return;

		     		// get field entry information
		     		MarketField marketField = getFieldEntryInfo();

		     		// add market field to appropriate message
					if (_parsingRefreshMsg)
					{
						if (_fieldCount < _marketPriceRefreshMsg.fieldEntries().length)
						{
							_marketPriceRefreshMsg.fieldEntries()[_fieldCount++] = marketField;
						}
					}
					else if (_parsingUpdateMsg)
					{
						if (_fieldCount < _updateMsg.fieldEntries().length)
						{
							_updateMsg.fieldEntries()[_fieldCount++] = marketField;
						}
					}
					else if (_parsingPostMsg)
					{
						if (_fieldCount < _postMsg.fieldEntries().length)
						{
							_postMsg.fieldEntries()[_fieldCount++] = marketField;
						}
					}
					else if (_parsingGenMsg)
					{
						if (_fieldCount < _genMsg.fieldEntries().length)
						{
							_genMsg.fieldEntries()[_fieldCount++] = marketField;
						}
					}
				}
			}
		}	

		/** XML Pull Parser endSectionRead(). */
		void endSectionRead() throws IOException, XmlPullParserException
		{
			String tag = XPP.getName();
			
			if (tag.equals("marketPriceMsgList"))
			{
				_parsingMPMsgs = false;
			}

			if (_parsingMPMsgs)
			{
				if (tag.equals("refreshMsg"))
				{
					_marketPriceRefreshMsg.fieldEntryCount(_fieldCount);
					getEstimatedFieldListContentLength(_marketPriceRefreshMsg);
					_fieldCount = 0;
					_parsingRefreshMsg = false;
				}
				else if (tag.equals("updateMsg") && XPP.getDepth() == 3)
				{
					_updateMsg.fieldEntryCount(_fieldCount);
					getEstimatedFieldListContentLength(_updateMsg);
					if (_updateCount < _marketPriceUpdateMsgs.length)
					{
						_marketPriceUpdateMsgs[_updateCount++] = _updateMsg;
					}
					_fieldCount = 0;
					_parsingUpdateMsg = false;
				}
				else if (tag.equals("postMsg"))
				{
					_postMsg.fieldEntryCount(_fieldCount);
					getEstimatedFieldListContentLength(_postMsg);
					if (_postCount < _marketPricePostMsgs.length)
					{
					    _marketPricePostMsgs[_postCount++] = _postMsg;
					}
					_fieldCount = 0;
					_parsingPostMsg = false;
				}
				else if (tag.equals("genMsg"))
				{
					_genMsg.fieldEntryCount(_fieldCount);
					getEstimatedFieldListContentLength(_genMsg);
					if (_genMsgCount < _marketPriceGenMsgs.length)
					{
					    _marketPriceGenMsgs[_genMsgCount++] = _genMsg;
					}
					_fieldCount = 0;
					_parsingGenMsg = false;
				}
			}
		}

		/* Get msg counts (updates, posts, and generic msgs). */
		private void getFileMessageCounts()
		{
			for (int i = 0; i < _xmlReader.getAttributeCount(); i++)
			{
				String ntag = XPP.getAttributeName(i);
				String nvalue = XPP.getAttributeValue(i);

				if (ntag.equals("updateMsgCount"))
				{
					_fileUpdateCount = Integer.parseInt(nvalue);
				}
				else if (ntag.equals("postMsgCount"))
				{
					_filePostCount = Integer.parseInt(nvalue);	    				
				}
				else if (ntag.equals("genMsgCount"))
				{
					_fileGenMsgCount = Integer.parseInt(nvalue);	    				
				}
			}
		}

		/* Get field entry count. */
		private void getFieldEntryCount()
		{
			for (int i = 0; i < getAttributeCount(); i++)
			{
				String ntag = XPP.getAttributeName(i);
				String nvalue = XPP.getAttributeValue(i);

				if (ntag.equals("entryCount"))
				{
					_fileFieldEntryCount = Integer.parseInt(nvalue);
				}
			}
		}

		/* Get field entry information. */
		private MarketField getFieldEntryInfo()
		{
			MarketField marketField = new MarketField();
			Qos qos = null;
			State state = null;
			
	 		for (int i = 0; i < getAttributeCount(); i++)
	 		{
				String ntag = XPP.getAttributeName(i);
				String nvalue = XPP.getAttributeValue(i);
				
				if (ntag.equals("fieldId"))
				{
					marketField.fieldEntry().fieldId(Integer.parseInt(nvalue));
				}
				else if (ntag.equals("dataType"))
				{
					marketField.fieldEntry().dataType(dataTypeValue(nvalue));
					if (marketField.fieldEntry().dataType() == DataTypes.QOS)
					{
						qos = CodecFactory.createQos();
						marketField.value(qos);
					}
					else if (marketField.fieldEntry().dataType() == DataTypes.STATE)
					{
						state = CodecFactory.createState();
						marketField.value(state);
					}
				}
				else if (ntag.equals("data"))
				{
					marketField.value(createFieldData(marketField.fieldEntry().dataType(), nvalue));
				}
				else if (ntag.equals("qosRate"))
				{
					if (qos != null)
					{
						qos.rate(qosRateValue(nvalue));
					}
				}
				else if (ntag.equals("qosRateInfo"))
				{
					if (qos != null)
					{
						qos.rateInfo(Integer.parseInt(nvalue));
					}
				}
				else if (ntag.equals("qosTimeliness"))
				{
					if (qos != null)
					{
						qos.timeliness(qosTimelinessValue(nvalue));
					}
				}
				else if (ntag.equals("qosTimeInfo"))
				{
					if (qos != null)
					{
						qos.timeInfo(Integer.parseInt(nvalue));
					}
				}
				else if (ntag.equals("qosDynamic"))
				{
					if (qos != null)
					{
						qos.dynamic(((Integer.parseInt(nvalue) > 0) ? true : false));
					}
				}
				else if (ntag.equals("streamState"))
				{
					if (state != null)
					{
						state.streamState(streamStateValue(nvalue));
					}
				}
				else if (ntag.equals("dataState"))
				{
					if (state != null)
					{
						state.dataState(dataStateValue(nvalue));
					}
				}
				else if (ntag.equals("code"))
				{
					if (state != null)
					{
						state.code(codeValue(nvalue));
					}
				}
				else if (ntag.equals("text"))
				{
					if (state != null)
					{
						state.text(textValue(nvalue));
					}
				}
	 		}
	 		
	 		return marketField;
		}

		/* Attempts to estimate the size needed to encode the given data. */
	    private void getEstimatedFieldListContentLength(MarketPriceMsg marketPriceMsg)
	    {
	    	int estimatedContentLength = 1;

	    	for(int i = 0; i < marketPriceMsg.fieldEntryCount(); ++i)
	    	{
	    		MarketField field = marketPriceMsg.fieldEntries()[i];

	    		estimatedContentLength += 2;

	    		switch(field.fieldEntry().dataType())
	    		{
	    			case DataTypes.INT: 
	    				estimatedContentLength += INT_SIZE; 
	    				break;
	    			case DataTypes.UINT: 
	    				estimatedContentLength += UINT_SIZE; 
	    				break;
	    			case DataTypes.FLOAT: 
	    				estimatedContentLength += FLOAT_SIZE; 
	    				break;
	    			case DataTypes.DOUBLE: 
	    				estimatedContentLength += DOUBLE_SIZE; 
	    				break;
	    			case DataTypes.REAL: 
	    				estimatedContentLength += REAL_SIZE; 
	    				break;
	    			case DataTypes.DATE: 
	    				estimatedContentLength += DATE_SIZE; 
	    				break;
	    			case DataTypes.TIME: 
	    				estimatedContentLength += TIME_SIZE; 
	    				break;
	    			case DataTypes.DATETIME: 
	    				estimatedContentLength += DATETIME_SIZE; 
	    				break;
	    			case DataTypes.QOS: 
	    				estimatedContentLength += QOS_SIZE; 
	    				break;
	    			case DataTypes.STATE: 
	    				estimatedContentLength += STATE_SIZE; 
	    				estimatedContentLength += ((State)field.value()).text().length();
	    				break;
	    			case DataTypes.ENUM:
	    				estimatedContentLength += ENUM_SIZE; 
	    				break;
	    			case DataTypes.BUFFER:
	    			case DataTypes.ASCII_STRING:
	    			case DataTypes.UTF8_STRING:
	    			case DataTypes.RMTES_STRING:
	    				estimatedContentLength += BUFFER_SIZE; 
	    				estimatedContentLength += ((Buffer)field.value()).length();
	    				break;
	    			default: break;

	    		}
	    	}

	    	marketPriceMsg.estimatedContentLength(estimatedContentLength);		
		}
	    
		/* Converts data type string to data type value. */
		private int dataTypeValue(String dataTypeString)
	    {
	        int retVal = 0;

	        if (dataTypeString.equals("RSSL_DT_INT"))
	        {
	            retVal = DataTypes.INT;
	        }
	        else if (dataTypeString.equals("RSSL_DT_UINT"))
	        {
	            retVal = DataTypes.UINT;
	        }
	        else if (dataTypeString.equals("RSSL_DT_FLOAT"))
	        {
	            retVal = DataTypes.FLOAT;
	        }
	        else if (dataTypeString.equals("RSSL_DT_DOUBLE"))
	        {
	            retVal = DataTypes.DOUBLE;
	        }
	        else if (dataTypeString.equals("RSSL_DT_REAL"))
	        {
	            retVal = DataTypes.REAL;
	        }
	        else if (dataTypeString.equals("RSSL_DT_DATE"))
	        {
	            retVal = DataTypes.DATE;
	        }
	        else if (dataTypeString.equals("RSSL_DT_TIME"))
	        {
	            retVal = DataTypes.TIME;
	        }
	        else if (dataTypeString.equals("RSSL_DT_DATETIME"))
	        {
	            retVal = DataTypes.DATETIME;
	        }
	        else if (dataTypeString.equals("RSSL_DT_QOS"))
	        {
	            retVal = DataTypes.QOS;
	        }
	        else if (dataTypeString.equals("RSSL_DT_STATE"))
	        {
	            retVal = DataTypes.STATE;
	        }
	        else if (dataTypeString.equals("RSSL_DT_ENUM"))
	        {
	            retVal = DataTypes.ENUM;
	        }
	        else if (dataTypeString.equals("RSSL_DT_BUFFER"))
	        {
	            retVal = DataTypes.BUFFER;
	        }
	        else if (dataTypeString.equals("RSSL_DT_ASCII_STRING"))
	        {
	            retVal = DataTypes.ASCII_STRING;
	        }
	        else if (dataTypeString.equals("RSSL_DT_UTF8_STRING"))
	        {
	            retVal = DataTypes.UTF8_STRING;
	        }
	        else if (dataTypeString.equals("RSSL_DT_RMTES_STRING"))
	        {
	            retVal = DataTypes.RMTES_STRING;
	        }

	        return retVal;
	    }

		/* Creates field entry data from file data. */ 
	    private Object createFieldData(int type, String value)
	    {
	    	Object retVal = null;
	    	
	        switch (type)
	        {
	            case DataTypes.INT:
	                retVal = CodecFactory.createInt();
	                ((Int)retVal).value(value);
	                break;
	            case DataTypes.UINT:
	                retVal = CodecFactory.createUInt();
	                ((UInt)retVal).value(value);
	                break;
	            case DataTypes.FLOAT:
	                retVal = CodecFactory.createFloat();
	                ((com.thomsonreuters.upa.codec.Float)retVal).value(value);
	                break;
	            case DataTypes.DOUBLE:
	                retVal = CodecFactory.createDouble();
	                ((com.thomsonreuters.upa.codec.Double)retVal).value(value);
	                break;
	            case DataTypes.REAL:
	                retVal = CodecFactory.createReal();
	                ((Real)retVal).value(value);
	                break;
	            case DataTypes.DATE:
	                retVal = CodecFactory.createDate();
	                ((Date)retVal).value(value);
	                break;
	            case DataTypes.TIME:
	                retVal = CodecFactory.createTime();
	                ((Time)retVal).value(value);
	                break;
	            case DataTypes.DATETIME:
	                retVal = CodecFactory.createDateTime();
	                ((DateTime)retVal).value(value);
	                break;
	            case DataTypes.ENUM:
	                retVal = CodecFactory.createEnum();
	                ((com.thomsonreuters.upa.codec.Enum)retVal).value(value);
	                break;
	            case DataTypes.BUFFER:
	                retVal = CodecFactory.createBuffer();
	                ((Buffer)retVal).data(value);
	                break;
	            case DataTypes.ASCII_STRING:
	                retVal = CodecFactory.createBuffer();
	                ((Buffer)retVal).data(value);
	                break;
	            case DataTypes.UTF8_STRING:
	                retVal = CodecFactory.createBuffer();
	                ((Buffer)retVal).data(value);
	                break;
	            case DataTypes.RMTES_STRING:
	                retVal = CodecFactory.createBuffer();
	                ((Buffer)retVal).data(value);
	                break;
	            default:
	                break;
	        }
	    	
	    	return retVal;
	    }
	    
		/* Creates state text data from file data. */ 
		private Buffer textValue(String value)
		{
			Buffer retVal = CodecFactory.createBuffer();
			
			retVal.data(value);
			
			return retVal;
		}

		/* Converts state code string to state code value. */
		private int codeValue(String value)
		{
	        int retVal = 0;
	        
	    	if (value.equals("RSSL_SC_NONE"))
	    	{
	    		retVal = StateCodes.NONE;
	    	}
	    	else if (value.equals("RSSL_SC_NOT_FOUND"))
	    	{
	    		retVal = StateCodes.NOT_FOUND;
	    	}
	    	else if (value.equals("RSSL_SC_TIMEOUT"))
	    	{
	    		retVal = StateCodes.TIMEOUT;
	    	}
	    	else if (value.equals("RSSL_SC_NOT_ENTITLED"))
	    	{
	    		retVal = StateCodes.NOT_ENTITLED;
	    	}
	    	else if (value.equals("RSSL_SC_INVALID_ARGUMENT"))
	    	{
	    		retVal = StateCodes.INVALID_ARGUMENT;
	    	}
	    	else if (value.equals("RSSL_SC_USAGE_ERROR"))
	    	{
	    		retVal = StateCodes.USAGE_ERROR;
	    	}
	    	else if (value.equals("RSSL_SC_PREEMPTED"))
	    	{
	    		retVal = StateCodes.PREEMPTED;
	    	}
	    	else if (value.equals("RSSL_SC_JIT_CONFLATION_STARTED"))
	    	{
	    		retVal = StateCodes.JIT_CONFLATION_STARTED;
	    	}
	    	else if (value.equals("RSSL_SC_REALTIME_RESUMED"))
	    	{
	    		retVal = StateCodes.REALTIME_RESUMED;
	    	}
	    	else if (value.equals("RSSL_SC_FAILOVER_STARTED"))
	    	{
	    		retVal = StateCodes.FAILOVER_STARTED;
	    	}
	    	else if (value.equals("RSSL_SC_FAILOVER_COMPLETED"))
	    	{
	    		retVal = StateCodes.FAILOVER_COMPLETED;
	    	}
	    	else if (value.equals("RSSL_SC_GAP_DETECTED"))
	    	{
	    		retVal = StateCodes.GAP_DETECTED;
	    	}
	    	else if (value.equals("RSSL_SC_NO_RESOURCES"))
	    	{
	    		retVal = StateCodes.NO_RESOURCES;
	    	}
	    	else if (value.equals("RSSL_SC_TOO_MANY_ITEMS"))
	    	{
	    		retVal = StateCodes.TOO_MANY_ITEMS;
	    	}
	    	else if (value.equals("RSSL_SC_ALREADY_OPEN"))
	    	{
	    		retVal = StateCodes.ALREADY_OPEN;
	    	}
	    	else if (value.equals("RSSL_SC_SOURCE_UNKNOWN"))
	    	{
	    		retVal = StateCodes.SOURCE_UNKNOWN;
	    	}
	    	else if (value.equals("RSSL_SC_NOT_OPEN"))
	    	{
	    		retVal = StateCodes.NOT_OPEN;
	    	}
	    	else if (value.equals("RSSL_SC_NON_UPDATING_ITEM"))
	    	{
	    		retVal = StateCodes.NON_UPDATING_ITEM;
	    	}
	    	else if (value.equals("RSSL_SC_UNSUPPORTED_VIEW_TYPE"))
	    	{
	    		retVal = StateCodes.UNSUPPORTED_VIEW_TYPE;
	    	}
	    	else if (value.equals("RSSL_SC_INVALID_VIEW"))
	    	{
	    		retVal = StateCodes.INVALID_VIEW;
	    	}
	    	else if (value.equals("RSSL_SC_FULL_VIEW_PROVIDED"))
	    	{
	    		retVal = StateCodes.FULL_VIEW_PROVIDED;
	    	}
	    	else if (value.equals("RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH"))
	    	{
	    		retVal = StateCodes.UNABLE_TO_REQUEST_AS_BATCH;
	    	}
			else if (value.equals("RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ"))
			{
				retVal = StateCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ;
			}
			else if (value.equals("RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER"))
			{
				retVal = StateCodes.EXCEEDED_MAX_MOUNTS_PER_USER;
			}
			else if (value.equals("RSSL_SC_ERROR"))
			{
				retVal = StateCodes.ERROR;
			}
			else if (value.equals("RSSL_SC_DACS_DOWN"))
			{
				retVal = StateCodes.DACS_DOWN;
			}
			else if (value.equals("RSSL_SC_USER_UNKNOWN_TO_PERM_SYS"))
			{
				retVal = StateCodes.USER_UNKNOWN_TO_PERM_SYS;
			}
			else if (value.equals("RSSL_SC_DACS_MAX_LOGINS_REACHED"))
			{
				retVal = StateCodes.DACS_MAX_LOGINS_REACHED;
			}
			else if (value.equals("RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED"))
			{
				retVal = StateCodes.DACS_USER_ACCESS_TO_APP_DENIED;
			}
			else if (value.equals("GAP_FILL"))
			{
				retVal = StateCodes.GAP_FILL;				
			}
			else if (value.equals("APP_AUTHORIZATION_FAILED"))
			{
				retVal = StateCodes.APP_AUTHORIZATION_FAILED;
			}
	    					
			
			return retVal;
		}

		/* Converts data state string to data state value. */
		private int dataStateValue(String value)
		{
	        int retVal = 0;

	    	if (value.equals("RSSL_DATA_NO_CHANGE"))
	    	{
	    		retVal = DataStates.NO_CHANGE;
	    	}
	    	else if (value.equals("RSSL_DATA_OK"))
	    	{
	    		retVal = DataStates.OK;
	    	}
	    	else if (value.equals("RSSL_DATA_SUSPECT"))
	    	{
	    		retVal = DataStates.SUSPECT;
	    	}

	    	return retVal;
		}

		/* Converts stream state string to stream state value. */
		private int streamStateValue(String value)
		{
	        int retVal = 0;
			
	    	if (value.equals("RSSL_STREAM_UNSPECIFIED"))
	    	{
	    		retVal = StreamStates.UNSPECIFIED;
	    	}
	    	else if (value.equals("RSSL_STREAM_OPEN"))
	    	{
	    		retVal = StreamStates.OPEN;
	    	}
	    	else if (value.equals("RSSL_STREAM_NON_STREAMING"))
	    	{
	    		retVal = StreamStates.NON_STREAMING;
	    	}
	    	else if (value.equals("RSSL_STREAM_CLOSED_RECOVER"))
	    	{
	    		retVal = StreamStates.CLOSED_RECOVER;
	    	}
	    	else if (value.equals("RSSL_STREAM_CLOSED"))
	    	{
	    		retVal = StreamStates.CLOSED;
	    	}
	    	else if (value.equals("RSSL_STREAM_REDIRECTED"))
	    	{
	    		retVal = StreamStates.REDIRECTED;
	    	}

	    	return retVal;
		}

		/* Converts qos timeliness string to qos timeliness value. */
		private int qosTimelinessValue(String value)
		{
	        int retVal = 0;
			
	    	if (value.equals("RSSL_QOS_TIME_UNSPECIFIED"))
	    	{
	    		retVal = QosTimeliness.UNSPECIFIED;
	    	}
	    	else if (value.equals("RSSL_QOS_TIME_REALTIME"))
	    	{
	    		retVal = QosTimeliness.REALTIME;
	    	}
	    	else if (value.equals("RSSL_QOS_TIME_DELAYED_UNKNOWN"))
	    	{
	    		retVal = QosTimeliness.DELAYED_UNKNOWN;
	    	}
	    	else if (value.equals("RSSL_QOS_TIME_DELAYED"))
	    	{
	    		retVal = QosTimeliness.DELAYED;
	    	}

			return retVal;
		}

		/* Converts qos rate string to qos rate value. */
		private int qosRateValue(String value)
		{
	        int retVal = 0;
			
	    	if (value.equals("RSSL_QOS_RATE_UNSPECIFIED"))
	    	{
	    		retVal = QosRates.UNSPECIFIED;
	    	}
	    	else if (value.equals("RSSL_QOS_RATE_TICK_BY_TICK"))
	    	{
	    		retVal = QosRates.TICK_BY_TICK;
	    	}
	    	else if (value.equals("RSSL_QOS_RATE_JIT_CONFLATED"))
	    	{
	    		retVal = QosRates.JIT_CONFLATED;
	    	}
	    	else if (value.equals("RSSL_QOS_RATE_TIME_CONFLATED"))
	    	{
	    		retVal = QosRates.TIME_CONFLATED;
	    	}

	    	return retVal;
		}

	}
	
	private final int MAX_UPDATE_MSGS = 10;
	private final int MAX_POST_MSGS = 10;
	private final int MAX_GEN_MSGS = 10;
	private final int MAX_FIELD_ENTRIES = 100;

	public XmlMsgDataReader _xmlReader = new XmlMsgDataReader();
	private MarketPriceMsg _marketPriceRefreshMsg; /* market price refresh message */
	private MarketPriceMsg _updateMsg; /* market price update message */
	private MarketPriceMsg _postMsg; /* market price post message */
	private MarketPriceMsg _genMsg; /* market price generic message */
	private MarketPriceMsg[] _marketPriceUpdateMsgs; /* array of market price update messages */
	private MarketPriceMsg[] _marketPricePostMsgs; /* array of market price post messages */
	private MarketPriceMsg[] _marketPriceGenMsgs; /* array of market price generic messages */
	
	private int _updateCount; /* current update message count */
	private int _postCount; /* current post message count */
	private int _genMsgCount; /* current generic message count */
	private int _fieldCount; /* current field entry count */
	private int _fileUpdateCount; /* update message count from file */
	private int _filePostCount; /* post message count from file */
	private int _fileGenMsgCount; /* generic message count from file */
	private int _fileFieldEntryCount; /* field entry count from file */
	
	private boolean _parsingMPMsgs; /* flag to indicate currently parsing market price messages */
	private boolean _parsingRefreshMsg; /* flag to indicate currently parsing a refresh message */
	private boolean _parsingUpdateMsg; /* flag to indicate currently parsing an update message */
	private boolean _parsingPostMsg; /* flag to indicate currently parsing a post message */
	private boolean _parsingGenMsg; /* flag to indicate currently parsing a generic message */
	
	private boolean _hasMarketPrice;
	
	/** Initialize the list. */
	public XmlMsgData()
	{
		_fileUpdateCount = MAX_UPDATE_MSGS;
		_filePostCount = MAX_POST_MSGS;
		_fileGenMsgCount = MAX_GEN_MSGS;
		_fileFieldEntryCount = MAX_FIELD_ENTRIES;
	}

	/**
	 * Parses xml message data file.
	 *
	 * @param filename the filename
	 * @return {@link PerfToolsReturnCodes}
	 */
	public int parseFile( String filename) 
	{
		return _xmlReader.parseFile(filename);
	}

	/**
	 *  Market price refresh message.
	 *
	 * @return the market price msg
	 */
	public MarketPriceMsg marketPriceRefreshMsg()
	{
		return _marketPriceRefreshMsg;
	}

	/**
	 *  Array of market price update messages.
	 *
	 * @return the market price msg[]
	 */
	public MarketPriceMsg[] marketPriceUpdateMsgs() 
	{
		return _marketPriceUpdateMsgs;
	}

	/**
	 *  Array of market price post messages.
	 *
	 * @return the market price msg[]
	 */
	public MarketPriceMsg[] marketPricePostMsgs()
	{
		return _marketPricePostMsgs;
	}

	/**
	 *  Array of market price generic messages.
	 *
	 * @return the market price msg[]
	 */
	public MarketPriceMsg[] marketPriceGenMsgs()
	{
		return _marketPriceGenMsgs;
	}
	
	/**
	 *  Market price update message count.
	 *
	 * @return the int
	 */
	public int marketPriceUpdateMsgCount()
	{
		return _updateCount;
	}

	/**
	 *  Market price post message count.
	 *
	 * @return the int
	 */
	public int marketPricePostMsgCount()
	{
		return _postCount;
	}

	/**
	 *  Market price generic message count.
	 *
	 * @return the int
	 */
	public int marketPriceGenMsgCount()
	{
		return _genMsgCount;
	}
	
	/**
	 * Checks for market price.
	 *
	 * @return true, if there is market data in the xml.
	 */
    public boolean hasMarketPrice()
    {
        return _hasMarketPrice;
    }
}
