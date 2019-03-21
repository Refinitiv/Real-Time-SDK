package com.thomsonreuters.upa.valueadd.reactor;

import java.util.HashMap;
import java.util.Map;


class RestRequest {

	 /** (0x00000) No flags set. */
    public static final int NONE = 0x0000;

    public static final int HAS_QUERY_PARAMS = 0x0001;

    public static final int HAS_HEADER_ATTRIB = 0x0002;

    private int _flags;
    private String _httpMethod;
    private HashMap<String,String> _queryParameter;
    private HashMap<String,String> _headerAttribute;
	
	public RestRequest()
	{
		clear();
	}
	
	public RestRequest clear() {
		_flags = RestRequest.NONE;
		_httpMethod = "GET";
		_queryParameter = null;
		_headerAttribute = null;
		
		return this;
	}

	public RestRequest HeaderAttribute(Map<String, String> headerAttribute) {
		_headerAttribute = (HashMap<String, String>) headerAttribute;
		_flags |= RestRequest.HAS_HEADER_ATTRIB;
		return this;
	}
	
	public RestRequest queryParameter(Map<String, String> parameters) {
		_queryParameter = (HashMap<String, String>) parameters;
		_flags |= RestRequest.HAS_QUERY_PARAMS;
		return this;
	}

	public boolean hasQueryParameter() {
		return (_flags & RestRequest.HAS_QUERY_PARAMS) != 0;
	}

	public boolean hasHeaderAttribute() {
		return (_flags & RestRequest.HAS_HEADER_ATTRIB) != 0;
	}
	
	public Map<String, String> queryParameter() {
		return _queryParameter;
	}

	public Map<String, String> headerAttribute() {
		return _headerAttribute;
	}
	
	public String toString()
	{
		 return "RestRequest" + "\n" + 
	               "\thttpMethod: " + _httpMethod + "\n" + 
	               "\theaderAttribute: " + _headerAttribute == null ? "" : _headerAttribute.toString() + "\n" + 
	               "\tqueryParameter: " + _queryParameter == null ? "" : _queryParameter.toString() + "\n";
	}
}
