///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.net.InetAddress;
import java.net.UnknownHostException;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;

public class ServiceEndpointDiscoveryOptionImpl implements ServiceEndpointDiscoveryOption
{
	Buffer _userName = CodecFactory.createBuffer();
	Buffer _password = CodecFactory.createBuffer();
	Buffer _clientId = CodecFactory.createBuffer();
	int _transport, _dataFormat;
	String _proxyPort;
	String _proxyHostName;
	String _proxyUserName;
	String _proxyPassword;
	String _proxyDomain;
	private String _proxyLocalHostName;
	String _proxyKrb5ConfigFile;
	
	ServiceEndpointDiscoveryOptionImpl()
	{
		clear();
	}
		
	@Override
	public ServiceEndpointDiscoveryOption clear()
	{
		_userName.clear();
		_password.clear();
		_clientId.clear();
		_transport = ServiceEndpointDiscoveryOption.TransportProtocol.UNKNOWN;
		_dataFormat = ServiceEndpointDiscoveryOption.DataformatProtocol.UNKNOWN;
		_proxyHostName = null;
		_proxyPort = null;
		_proxyUserName = null;
		_proxyPassword = null;
		_proxyDomain = null;
		_proxyLocalHostName = null;
		_proxyKrb5ConfigFile = null;
		
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption username(String username)
	{
		_userName.data(username);
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption password(String password)
	{
		_password.data(password);
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption clientId(String clientId)
	{
		_clientId.data(clientId);
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption transport(int transport)
	{
		_transport = transport;
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption dataFormat(int dataFormat)
	{
		_dataFormat = dataFormat;
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption proxyHostName(String proxyHostName)
	{
		_proxyHostName = proxyHostName;
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption proxyPort(String proxyPort)
	{
		_proxyPort = proxyPort;
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption proxyUserName(String proxyUserName)
	{
		_proxyUserName = proxyUserName;
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption proxyPassword(String proxyPassword)
	{
		_proxyPassword = proxyPassword;
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption proxyDomain(String proxyDomain)
	{
		_proxyDomain = proxyDomain;
		return this;
	}

	@Override
	public ServiceEndpointDiscoveryOption proxyLocalHostName(String proxyLocalHostName)
	{
		_proxyLocalHostName = proxyLocalHostName;
		return this;
	}
	
	String proxyLocalHostName()
	{
		if(_proxyLocalHostName == null)
		{
			String localIPAddress = null;
			String proxyLocalHostName = "localhost";
			
			try
	        {
	        	localIPAddress = InetAddress.getLocalHost().getHostAddress();
	        	proxyLocalHostName = InetAddress.getLocalHost().getHostName();
	        }
	        catch (UnknownHostException e)
	        {
	        	_proxyLocalHostName = localIPAddress;
	        }
			
			_proxyLocalHostName = proxyLocalHostName;
		}
		
		return _proxyLocalHostName;
	}

	@Override
	public ServiceEndpointDiscoveryOption proxyKRB5ConfigFile(String proxyKrb5ConfigFile)
	{
		_proxyKrb5ConfigFile = proxyKrb5ConfigFile;
		return this;
	}
}
