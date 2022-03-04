/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.ArrayList;
import java.util.List;

public class ReactorServiceEndpointInfo 
{
	List <String> _dataFormatList;
	String _endPoint;
	List <String> _locationList;
	String _port;
	String _provider;
	String _transport;
	
	ReactorServiceEndpointInfo()
	{
		_dataFormatList = new ArrayList <String>();
		_locationList = new ArrayList <String>();
	}
	
	public List <String> dataFormatList()
	{
		return _dataFormatList;
	}
	
	public String endPoint()
	{
		return _endPoint;
	}
	
	public List <String> locationList()
	{
		return _locationList;
	}
	
	public String port()
	{
		return _port;
	}
	
	public String provider()
	{
		return _provider;
	}
	
	public String transport()
	{
		return _transport;
	}
	
	public String toString()
	{
		return "ReactorServiceEndpointInfo\n" +
				"\tendpoint: " + _endPoint + "\n" +
				"\tport: " + _port + "\n" +
				"\tprovider: " + _provider + "\n" +
				"\tdataFormat: " + _dataFormatList + "\n" +
				"\tlocation: " + _locationList + "\n" +
				"\ttransport: " + _transport + "\n";
	}
}
