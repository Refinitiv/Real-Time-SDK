/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import java.util.Objects;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.json.converter.ConversionResults;
import com.refinitiv.eta.json.converter.ConverterFactory;
import com.refinitiv.eta.json.converter.GetJsonMsgOptions;
import com.refinitiv.eta.json.converter.JsonConverter;
import com.refinitiv.eta.json.converter.JsonConverterBuilder;
import com.refinitiv.eta.json.converter.JsonConverterError;
import com.refinitiv.eta.json.converter.JsonConverterProperties;
import com.refinitiv.eta.json.converter.JsonMsgClasses;
import com.refinitiv.eta.json.converter.JsonProtocol;
import com.refinitiv.eta.json.converter.RWFToJsonOptions;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.WriteFlags;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;

public class JsonConverterSession 
{
	private JsonConverterOptions converterOptions;
	private DataDictionary dataDictionary;
	private boolean jsonConverterInitialized;
	private JsonConverterState converterState;
	private TransportBuffer convBuffer;
	private JsonConverter jsonConverter;
	private DecodeIterator decodeIterator;
	private Msg rsslMsg;
	private JsonConverterError converterError;
	private ConversionResults conversionResults;
	
	static String JSON_PONG_MESSAGE = "{\"Type\":\"Pong\"}";
	
	public JsonConverterSession()
	{
		converterOptions = new JsonConverterOptions();
		converterState = new JsonConverterState();
		decodeIterator = CodecFactory.createDecodeIterator();
		rsslMsg = CodecFactory.createMsg();
		converterError = ConverterFactory.createJsonConverterError();
		conversionResults = ConverterFactory.createConversionResults();
		clear();
	}
	
	public void clear()
	{
		dataDictionary(null);
		jsonConverterInitialized = false;
		convBuffer(null);
		jsonConverter(null);
		decodeIterator.clear();
		rsslMsg.clear();
		converterError.clear();
	}
	
	public JsonConverterOptions jsonConverterOptions()
	{
		return converterOptions;
	}
	
	public JsonConverterState jsonConverterState()
	{
		return converterState;
	}

	public DataDictionary dataDictionary()
	{
		return dataDictionary;
	}

	public void dataDictionary(DataDictionary dataDictionary)
	{
		this.dataDictionary = dataDictionary;
	}

	public boolean jsonConverterInitialized()
	{
		return jsonConverterInitialized;
	}

	public TransportBuffer convBuffer()
	{
		return convBuffer;
	}

	public void convBuffer(TransportBuffer convBuffer)
	{
		this.convBuffer = convBuffer;
	}

	public JsonConverter jsonConverter()
	{
		return jsonConverter;
	}

	public void jsonConverter(JsonConverter jsonConverter)
	{
		this.jsonConverter = jsonConverter;
	}
	
	public int initialize(Error error)
	{
		JsonConverterError converterError = ConverterFactory.createJsonConverterError();
		
		if (jsonConverterInitialized)
		{
			error.clear();
			error.errorId(CodecReturnCodes.FAILURE);
			error.text("The JsonConverter has been initialized");
			return error.errorId();
		}
		
		JsonConverterBuilder jsonConverterBuilder = ConverterFactory.createJsonConverterBuilder();
		
		jsonConverterBuilder.setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
        .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, converterOptions.jsonExpandedEnumFields())
        .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, converterOptions.catchUnknownJsonKeys())
        .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, converterOptions.catchUnknownJsonFids())
        .setProperty(JsonConverterProperties.JSON_CPC_USE_DEFAULT_DYNAMIC_QOS, true)
        .setProperty(JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, true) /* Always enable this feature */
        .setServiceConverter(converterOptions)
        .setDictionary(converterOptions.datadictionary());
		
		/* Sets the default service ID if specified by users. */
        if(converterOptions.defaultServiceId() >= 0)
        {
            jsonConverterBuilder.setProperty(JsonConverterProperties.JSON_CPC_DEFAULT_SERVICE_ID, converterOptions.defaultServiceId());
        }
        
        jsonConverter = jsonConverterBuilder.build(converterError);
        
        if(Objects.isNull(jsonConverter))
        {
        	error.clear();
			error.errorId(CodecReturnCodes.FAILURE);
			error.text(converterError.getText());
			return error.errorId();
        }
		
		return CodecReturnCodes.SUCCESS;
	}
	
	private TransportBuffer convertToJsonMsg(Channel channel, Error error)
	{
		TransportBuffer msgBuffer = null;
		rsslMsg.clear();
		
		int ret = rsslMsg.decode(decodeIterator);
		
		if(ret == CodecReturnCodes.SUCCESS)
		{
			RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
			GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
			
			rwfToJsonOptions.clear();
			rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
			
			if(jsonConverter.convertRWFToJson(rsslMsg, rwfToJsonOptions, conversionResults, converterError) != CodecReturnCodes.SUCCESS)
			{
				error.clear();
				error.channel(channel);
				error.errorId(CodecReturnCodes.FAILURE);
				error.text("Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());
				return null;
			}
			
			getJsonMsgOptions.clear();
			getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
            getJsonMsgOptions.streamId(rsslMsg.streamId());
            getJsonMsgOptions.isCloseMsg(rsslMsg.msgClass() == MsgClasses.CLOSE ? true : false );
            
            msgBuffer = channel.getBuffer(conversionResults.getLength(), false, error);
            
            if(Objects.isNull(msgBuffer))
            {
            	return null;
            }
            
            if(jsonConverter.getJsonBuffer(msgBuffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
            {
            	/* Releases the buffer when failed to get JSON message. */
            	channel.releaseBuffer(msgBuffer, error);
            	
            	error.clear();
				error.channel(channel);
				error.errorId(CodecReturnCodes.FAILURE);
				error.text("Failed to get converted JSON message. Error text: " + converterError.getText());
				return null;
            }
		}
		else
		{
			error.clear();
			error.channel(channel);
			error.errorId(CodecReturnCodes.FAILURE);
			error.text("rsslDecodeMsg() failed: " + CodecReturnCodes.toString(ret));
			return null;
		}
		
		return msgBuffer;
	}
	
	public TransportBuffer convertToJsonMsg(Channel channel, TransportBuffer rwfBuffer, Error error)
	{
		decodeIterator.clear();
		decodeIterator.setBufferAndRWFVersion(rwfBuffer, channel.majorVersion(), channel.minorVersion());
		
		/* Clears to reset the error state */
		error.clear();
		
		return convertToJsonMsg(channel, error);
	}
	
	public TransportBuffer convertToJsonMsg(Channel channel, Buffer rwfBuffer, Error error)
	{
		decodeIterator.clear();
		decodeIterator.setBufferAndRWFVersion(rwfBuffer, channel.majorVersion(), channel.minorVersion());
		
		/* Clears to reset the error state */
		error.clear();
		
		return convertToJsonMsg(channel, error);
	}
	
	int resetConverterState(TransportBuffer buffer, Error error)
	{
		int ret = CodecReturnCodes.SUCCESS;
		JsonConverterError converterError = ConverterFactory.createJsonConverterError();
		
		if(Objects.nonNull(buffer))
		{
			converterState.buffer(buffer);
		}
		
		converterState.parseJsonOptions().clear();
		converterState.parseJsonOptions().setProtocolType(JsonProtocol.JSON_JPT_JSON2);
		
		converterState.decodeJsonMsgOptions().clear();
		converterState.decodeJsonMsgOptions().setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
		
		ret = jsonConverter.parseJsonBuffer(buffer, converterState.parseJsonOptions(), converterError);
		
		if(ret != CodecReturnCodes.SUCCESS)
		{
			error.clear();
			error.errorId(CodecReturnCodes.FAILURE);
			error.text("parseJsonBuffer() failed with error text: " + converterError.getText());
		}
		
		return ret;
	}
	
	int sendJsonMessage(Channel channel, TransportBuffer buffer, int wrtFlags, Error error)
	{
		int ret = TransportReturnCodes.SUCCESS;
		WriteArgs writeArgs = TransportFactory.createWriteArgs();
		writeArgs.flags(wrtFlags);
		
		ret = channel.write(buffer, writeArgs, error);
		
		while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
		{
			if ((ret = channel.flush(error)) < TransportReturnCodes.SUCCESS)
			{
				error.text("rsslFlush() failed with error text: " + error.text());
				return ret;
			}
			
			ret = channel.write(buffer, writeArgs, error);
		}
		
		if(ret != TransportReturnCodes.SUCCESS)
		{
			error.text("rsslWrite() failed with error text: " + error.text());
		}
		
		return ret;
	}
	
	public int convertFromJsonMessage(Channel channel, TransportBuffer jsonBuffer, Error error)
	{
		int ret = CodecReturnCodes.SUCCESS;
		
		if(Objects.nonNull(jsonBuffer))
		{
			if(resetConverterState(jsonBuffer, error) != CodecReturnCodes.SUCCESS)
			{
				return CodecReturnCodes.FAILURE;
			}
		}
		
		converterState.jsonMsg().clear();
		
		ret = jsonConverter.decodeJsonMsg(converterState.jsonMsg(), converterState.decodeJsonMsgOptions(), converterError);
		
		if(ret != CodecReturnCodes.END_OF_CONTAINER)
		{
			if(ret == CodecReturnCodes.SUCCESS)
			{
				switch(converterState.jsonMsg().jsonMsgClass()) 
				{
					case JsonMsgClasses.RSSL_MESSAGE:
	                {
	                	break;
					}
					case JsonMsgClasses.PING:
	                {
	                	TransportBuffer buffer = channel.getBuffer(JSON_PONG_MESSAGE.length(), false, error);
	                	if(Objects.nonNull(buffer))
	                	{
	                		buffer.data().put(JSON_PONG_MESSAGE.getBytes());
	                		
	                		ret = sendJsonMessage(channel, buffer, WriteFlags.DIRECT_SOCKET_WRITE, error);
	                		if( ret != TransportReturnCodes.FAILURE)
	                		{
	                			ret = TransportReturnCodes.READ_PING;
	                		}
	                	}
	                	else
	                	{
	                		ret = TransportReturnCodes.FAILURE;
	                		error.clear();
	        				error.channel(channel);
	        				error.errorId(CodecReturnCodes.FAILURE);
	        				error.text("Failed to get buffer for Ping response : " + error.text());
	                	}
	                	
	                	break;
	                }
					case JsonMsgClasses.PONG:
	                {
	                	ret = TransportReturnCodes.READ_PING;
	                	break;
	                }
					case JsonMsgClasses.ERROR:
	                {
	                	error.clear();
	    				error.channel(channel);
	    				error.errorId(CodecReturnCodes.FAILURE);
	    				error.text("Received JSON error message: " + converterState.jsonMsg().jsonMsgData().toString());
	                	
	                	ret = CodecReturnCodes.FAILURE;
	                	break;
	                }
				}
			}
			else
			{
				error.clear();
				error.channel(channel);
				error.errorId(CodecReturnCodes.FAILURE);
				error.text("decodeJsonMsg() failed with error text: " + converterError.getText());
			}
		}
		
		return ret;
	}
}
