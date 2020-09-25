///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.List;

public class ServiceEndpointDiscoveryRespImpl implements ServiceEndpointDiscoveryResp
{
	List<ServiceEndpointDiscoveryInfo> _serviceEndpointInfoList = new ArrayList<ServiceEndpointDiscoveryInfo>();
	StringBuilder _strBuilder;
	
	@Override
	public List<ServiceEndpointDiscoveryInfo> serviceEndpointInfoList()
	{
		return _serviceEndpointInfoList;
	}
	
	@Override
	public String toString()
	{
		if(_strBuilder == null)
			_strBuilder = new StringBuilder(1024);
		else
			_strBuilder.setLength(0);
		
		Utilities.addIndent(_strBuilder.append("Services : \n"), 1);
		for(int index = 0; index < _serviceEndpointInfoList.size(); index++)
		{
			Utilities.addIndent(_strBuilder.append(((ServiceEndpointDiscoveryInfoImpl)_serviceEndpointInfoList.get(index)).toString(2))
					.append("\n"), 1);
		}
		
		return _strBuilder.toString();
	}
}
