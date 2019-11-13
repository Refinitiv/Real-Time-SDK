///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.ArrayList;
import java.util.List;

public class ServiceEndpointDiscoveryInfoImpl implements ServiceEndpointDiscoveryInfo
{
	List<String> _dataFormatList = new ArrayList<String>(2);
	List<String> _locationList = new ArrayList<String>(2);
	String _endpoint;
	String _port;
	String _provider;
	String _transport;
	StringBuilder _strBuilder;
	
	
	@Override
	public List<String> dataFormatList()
	{
		return _dataFormatList;
	}

	@Override
	public String endpoint()
	{
		return _endpoint;
	}

	@Override
	public List<String> locationList()
	{
		return _locationList;
	}

	@Override
	public String port()
	{
		return _port;
	}

	@Override
	public String provider()
	{
		return _provider;
	}

	@Override
	public String transport() 
	{
		return _transport;
	}
	
	@Override
	public String toString()
	{
		return toString(0);
	}
	
	public String toString(int indent)
	{
		if(_strBuilder == null)
			_strBuilder = new StringBuilder(64);
		else
			_strBuilder.setLength(0);
		
		Utilities.addIndent(_strBuilder.append("Service : \n"), indent);
		Utilities.addIndent(_strBuilder.append("Provider : ").append(_provider).append("\n"), indent);
		Utilities.addIndent(_strBuilder.append("Transport : ").append(_transport).append("\n"), indent);
		Utilities.addIndent(_strBuilder.append("Endpoint : ").append(_endpoint).append("\n"), indent);
		Utilities.addIndent(_strBuilder.append("Port : ").append(_port).append("\n"), indent);
		
		_strBuilder.append("Data Format : ");
		for(int index = 0; index < _dataFormatList.size(); index++)
			_strBuilder.append(_dataFormatList.get(index)).append("  ");
		
		Utilities.addIndent(_strBuilder.append("\n"), indent);
		_strBuilder.append("Location : ");
		for(int index = 0; index < _locationList.size(); index++)
			_strBuilder.append(_locationList.get(index)).append("  ");
		
		return _strBuilder.toString();
	}
}
