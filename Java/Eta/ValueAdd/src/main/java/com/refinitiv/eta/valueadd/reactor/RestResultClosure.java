package com.refinitiv.eta.valueadd.reactor;

class RestResultClosure 
{
	private RestCallback _restCallback;
	private Object _userSpecObj;
	
	
	RestResultClosure(RestCallback callback, Object userSpecObj)
	{
		_restCallback = callback;
		_userSpecObj = userSpecObj;
	}
	
	RestCallback restCallback()
	{
		return _restCallback;
	}
	
	Object userSpecObj()
	{
		return _userSpecObj;
	}

}
