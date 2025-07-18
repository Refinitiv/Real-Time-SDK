/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.HashMap;
import java.util.Map;

import org.apache.hc.core5.http.Header;
import org.apache.hc.core5.http.ProtocolVersion;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import com.refinitiv.eta.codec.CodecReturnCodes;

class RestResponse {

	private int _statusCode;
	private String _statusText;
	private HashMap<String,String> _headerAttribute = new HashMap<>();
	private Object _bodyInJSon;
	private ProtocolVersion _protocolVersion;
	private String _contentType;
	
	public int statusCode() {
		return _statusCode;
	}
	
	public String statusText() {
		return _statusText;
	}
	
	public Map<String, String> headerAttribute() {
		return _headerAttribute;
	}
	
	public JSONObject jsonObject() {
		
		if (_bodyInJSon instanceof JSONObject)
			return (JSONObject)_bodyInJSon;
		
		return null;
	}
	
	public JSONArray jsonArray() {
		
		if (_bodyInJSon instanceof JSONArray)
			return (JSONArray)_bodyInJSon;
		
		return null;
	}
	
	public Object body()
	{
		return _bodyInJSon;
	}
	
	public String protocolVersion()
	{
		return _protocolVersion.toString();
	}
	
	public String contentType()
	{
		return _contentType;
	}
	
	public String toString()
	{
		StringBuilder builder = new StringBuilder(50);
		
		builder.append("\nStatusCode : " + _statusCode + "\n");
		builder.append("StatusText : " + _statusText + "\n");
		
		if (_protocolVersion != null)
			builder.append("Protocol Version : " + protocolVersion() + "\n");
		
		if (!_headerAttribute.isEmpty())
			builder.append("Message Headers : \n");
		
		for (Map.Entry<String,String> entry : _headerAttribute.entrySet())
		{
			builder.append(entry.getKey() + " : " + entry.getValue()).append("\n");
		}
		
		if ( jsonObject() != null)
		{
			builder.append("\n");
			builder.append("Message body : \n");
			builder.append(((JSONObject)_bodyInJSon).toString(2));
		}
		else if ( jsonArray() != null)
		{
			builder.append("\n");
			builder.append("Message body : \n");
			builder.append(((JSONArray)_bodyInJSon).toString(2));
		}
		
		return builder.toString();
	}

	void body(String respString, ReactorErrorInfo errorInfo)
	{
		if (respString == null)
			return;
		
		try
		{
			_bodyInJSon = entityStringToJSON(respString, errorInfo);
		}
		catch (JSONException e)
		{
			errorInfo.clear();
	        errorInfo.code(CodecReturnCodes.FAILURE).location("RestResponse.body");
	        errorInfo.error().errorId(CodecReturnCodes.FAILURE);
	        if (e.getLocalizedMessage() != null)
	        	errorInfo.error().text(e.getLocalizedMessage());
		}
	}
	
	void statusCode(int statusCode) {
		_statusCode = statusCode;
	}
	
	void statusText(String statusText) {
		_statusText = statusText;
	}
	
	void headerAttribute(Header[] headers) {
		if (headers.length == 0)
			return;
		
		for (Map.Entry<String,String> entry : _headerAttribute.entrySet())
		{
			_headerAttribute.put(entry.getKey(), entry.getValue());
		}
	}
	
	void protocolVersion(ProtocolVersion protocolVersion)
	{
		_protocolVersion = new ProtocolVersion(protocolVersion.getProtocol(), protocolVersion.getMajor(), protocolVersion.getMinor());
	}
	
	void contentType(String contentType)
	{
		_contentType = contentType;
	}
	
	Object entityStringToJSON(String entityString, ReactorErrorInfo errorInfo)
	{
		try {
			switch(entityString.trim().charAt(0))
			{
			case 91: 
			{
				return new JSONArray(entityString);
			}	
			case 123: 
			{
				return new JSONObject(entityString);
			}
			default:
				return null;
			}	
		} catch (Exception e) {
			errorInfo.clear();
	        errorInfo.code(CodecReturnCodes.FAILURE).location("RestResponse.entityStringToJSON");
	        errorInfo.error().errorId(CodecReturnCodes.FAILURE);
	        if (e.getLocalizedMessage() != null)
	        	errorInfo.error().text(e.getLocalizedMessage());
	        
	        return null;
		}
	}
}
