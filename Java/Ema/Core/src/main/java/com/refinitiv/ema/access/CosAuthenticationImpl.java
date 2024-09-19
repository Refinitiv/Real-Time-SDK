///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------
package com.refinitiv.ema.access;

public class CosAuthenticationImpl implements CosAuthentication
{
	private int _type = CosAuthenticationType.NOT_REQUIRED;
	private OmmOutOfRangeExceptionImpl _ommOORExcept;

	public CosAuthenticationImpl()
	{
		
	}
	@Override
	public CosAuthentication clear()
	{
		_type = CosAuthenticationType.NOT_REQUIRED;

		return this;
	}

	@Override
	public CosAuthentication type(int type) 
	{

		if (type >= CosAuthenticationType.NOT_REQUIRED || type <= CosAuthenticationType.OMM_LOGIN)
		{
			_type = type;
		}
		
		else
		{
			if (_ommOORExcept == null)
				_ommOORExcept = new OmmOutOfRangeExceptionImpl();
			
			throw _ommOORExcept.message("Passed in CosFlowControl Type is not supported.");
		}
		return this;
	}

	@Override
	public int type()
	{

		return _type;
	}
}
