/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

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
