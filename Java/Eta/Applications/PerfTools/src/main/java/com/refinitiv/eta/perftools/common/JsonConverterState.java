/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.json.converter.ConverterFactory;
import com.refinitiv.eta.json.converter.DecodeJsonMsgOptions;
import com.refinitiv.eta.json.converter.JsonMsg;
import com.refinitiv.eta.json.converter.ParseJsonOptions;
import com.refinitiv.eta.transport.TransportBuffer;

public class JsonConverterState 
{
	private DecodeJsonMsgOptions	decodeOptions;	
	private ParseJsonOptions	parseOptions;
	private JsonMsg	jsonMsg;
	private boolean	failedToConvertJSONMsg;
	private TransportBuffer	buffer;
	
	public JsonConverterState()
	{
		decodeOptions = ConverterFactory.createDecodeJsonMsgOptions();
		parseOptions = ConverterFactory.createParseJsonOptions();
		jsonMsg = ConverterFactory.createJsonMsg();
		failedToConvertJSONMsg(false);
		buffer(null);
	}
	
	public void clear()
	{
		decodeOptions.clear();
		parseOptions.clear();
		jsonMsg.clear();
		failedToConvertJSONMsg(false);
		buffer(null);
	}
	
	public DecodeJsonMsgOptions decodeJsonMsgOptions()
	{
		return decodeOptions;
	}
	
	public ParseJsonOptions parseJsonOptions()
	{
		return parseOptions;
	}
	
	public JsonMsg jsonMsg()
	{
		return jsonMsg;
	}

	public boolean failedToConvertJSONMsg()
	{
		return failedToConvertJSONMsg;
	}

	public void failedToConvertJSONMsg(boolean failedToConvertJSONMsg)
	{
		this.failedToConvertJSONMsg = failedToConvertJSONMsg;
	}

	public TransportBuffer buffer()
	{
		return buffer;
	}

	public void buffer(TransportBuffer buffer)
	{
		this.buffer = buffer;
	}
}
