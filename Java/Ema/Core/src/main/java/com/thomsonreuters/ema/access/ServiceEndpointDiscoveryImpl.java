///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.ServiceEndpointDiscovery;
import com.thomsonreuters.ema.access.ServiceEndpointDiscoveryClient;
import com.thomsonreuters.ema.access.ServiceEndpointDiscoveryOption;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDiscoveryDataFormatProtocol;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDiscoveryTransportProtocol;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorServiceDiscoveryOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorServiceEndpointEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorServiceEndpointEventCallback;
import com.thomsonreuters.upa.valueadd.reactor.ReactorServiceEndpointInfo;

class ServiceEndpointDiscoveryImpl implements ServiceEndpointDiscovery, ReactorServiceEndpointEventCallback
{
	private Reactor _reactor;
	private ReactorErrorInfo _reactorErrorInfo = ReactorFactory.createReactorErrorInfo();
	private StringBuilder _strBuilder;
	private ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
	private ReactorServiceDiscoveryOptions _reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private ServiceEndpointDiscoveryRespImpl _serviceEndpointDiscoveryRespImpl = new ServiceEndpointDiscoveryRespImpl();
	private ServiceEndpointDiscoveryEventImpl _ServiceEndpointDiscoveryEventImpl = new ServiceEndpointDiscoveryEventImpl();
	private ServiceEndpointDiscoveryClient _client;
	private Buffer _tokenServiceURLBuf = CodecFactory.createBuffer();
	private Buffer _serviceDiscoveryUrlBuf = CodecFactory.createBuffer();
	
	ServiceEndpointDiscoveryImpl()
	{
		ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
		reactorOptions.clear();
		_reactorErrorInfo.clear();
		
		reactorOptions.userSpecObj(this);
		_reactor = ReactorFactory.createReactor(reactorOptions, _reactorErrorInfo);
		if(_reactorErrorInfo.code() != ReactorReturnCodes.SUCCESS)
		{
			strBuilder().append("Failed to create ServiceEndpointDiscoveryImpl (ReactorFactory.createReactor).")
			.append("' Error Id='").append(_reactorErrorInfo.error().errorId()).append("' Internal sysError='")
			.append(_reactorErrorInfo.error().sysError()).append("' Error Location='")
			.append(_reactorErrorInfo.location()).append("' Error Text='")
			.append(_reactorErrorInfo.error().text()).append("'. ");
			
			throw ommIUExcept().message(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);
		}
	}
	
	ServiceEndpointDiscoveryImpl(String tokenServiceUrl, String serviceDiscoveryUrl)
	{
		ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
		reactorOptions.clear();
		_reactorErrorInfo.clear();
		
		reactorOptions.userSpecObj(this);
		
		if ((tokenServiceUrl != null) && (!tokenServiceUrl.isEmpty()))
		{
			_tokenServiceURLBuf.data(tokenServiceUrl);
			reactorOptions.tokenServiceURL(_tokenServiceURLBuf);
		}
		
		if((serviceDiscoveryUrl != null) && (!serviceDiscoveryUrl.isEmpty()))
		{
			_serviceDiscoveryUrlBuf.data(serviceDiscoveryUrl);
			reactorOptions.serviceDiscoveryURL(_serviceDiscoveryUrlBuf);
		}
		
		_reactor = ReactorFactory.createReactor(reactorOptions, _reactorErrorInfo);
		if(_reactorErrorInfo.code() != ReactorReturnCodes.SUCCESS)
		{
			strBuilder().append("Failed to create ServiceEndpointDiscoveryImpl (ReactorFactory.createReactor).")
			.append("' Error Id='").append(_reactorErrorInfo.error().errorId()).append("' Internal sysError='")
			.append(_reactorErrorInfo.error().sysError()).append("' Error Location='")
			.append(_reactorErrorInfo.location()).append("' Error Text='")
			.append(_reactorErrorInfo.error().text()).append("'. ");
			
			throw ommIUExcept().message(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);
		}
	}
	
	@Override
	public void registerClient(ServiceEndpointDiscoveryOption params, ServiceEndpointDiscoveryClient client)
	{
		registerClient(params, client, null);
	}

	@Override
	public void registerClient(ServiceEndpointDiscoveryOption params, ServiceEndpointDiscoveryClient client, Object closure)
	{
		_userLock.lock();
		try
		{
			ServiceEndpointDiscoveryOptionImpl serviceEndpointDiscoveryOptionImpl = (ServiceEndpointDiscoveryOptionImpl)params;
			_reactorErrorInfo.clear();
			_reactorServiceDiscoveryOptions.clear();
			
			_reactorServiceDiscoveryOptions.userName(serviceEndpointDiscoveryOptionImpl._userName);
			_reactorServiceDiscoveryOptions.password(serviceEndpointDiscoveryOptionImpl._password);
			_reactorServiceDiscoveryOptions.clientId(serviceEndpointDiscoveryOptionImpl._clientId);
			_reactorServiceDiscoveryOptions.clientSecret(serviceEndpointDiscoveryOptionImpl._clientSecret);
			_reactorServiceDiscoveryOptions.tokenScope(serviceEndpointDiscoveryOptionImpl._tokenScope);
			_reactorServiceDiscoveryOptions.transport(serviceEndpointDiscoveryOptionImpl._transport);
			_reactorServiceDiscoveryOptions.dataFormat(serviceEndpointDiscoveryOptionImpl._dataFormat);
			_reactorServiceDiscoveryOptions.takeExclusiveSignOnControl(serviceEndpointDiscoveryOptionImpl._takeExclusiveSignOnControl);
			
			if(serviceEndpointDiscoveryOptionImpl._proxyHostName != null && !serviceEndpointDiscoveryOptionImpl._proxyHostName.isEmpty())
			{
				_reactorServiceDiscoveryOptions.proxyHostName().data(serviceEndpointDiscoveryOptionImpl._proxyHostName);
			}
			
			if(serviceEndpointDiscoveryOptionImpl._proxyPort != null && !serviceEndpointDiscoveryOptionImpl._proxyPort.isEmpty())
			{
				_reactorServiceDiscoveryOptions.proxyPort().data(serviceEndpointDiscoveryOptionImpl._proxyPort);
			}
			
			if(serviceEndpointDiscoveryOptionImpl._proxyUserName != null && !serviceEndpointDiscoveryOptionImpl._proxyUserName.isEmpty())
			{
				_reactorServiceDiscoveryOptions.proxyUserName().data(serviceEndpointDiscoveryOptionImpl._proxyUserName);
			}
			
			if(serviceEndpointDiscoveryOptionImpl._proxyPassword != null && !serviceEndpointDiscoveryOptionImpl._proxyPassword.isEmpty())
			{
				_reactorServiceDiscoveryOptions.proxyPassword().data(serviceEndpointDiscoveryOptionImpl._proxyPassword);
			}
			
			if(serviceEndpointDiscoveryOptionImpl._proxyDomain != null && !serviceEndpointDiscoveryOptionImpl._proxyDomain.isEmpty())
			{
				_reactorServiceDiscoveryOptions.proxyDomain().data(serviceEndpointDiscoveryOptionImpl._proxyDomain);
			}
			
			if(serviceEndpointDiscoveryOptionImpl.proxyLocalHostName() != null && !serviceEndpointDiscoveryOptionImpl.proxyLocalHostName().isEmpty())
			{
				_reactorServiceDiscoveryOptions.proxyLocalHostName().data(serviceEndpointDiscoveryOptionImpl.proxyLocalHostName());
			}
			
			if(serviceEndpointDiscoveryOptionImpl._proxyKrb5ConfigFile != null && !serviceEndpointDiscoveryOptionImpl._proxyKrb5ConfigFile.isEmpty())
			{
				_reactorServiceDiscoveryOptions.proxyKRB5ConfigFile().data(serviceEndpointDiscoveryOptionImpl._proxyKrb5ConfigFile);
			}
			
			_reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(this);
			_client = client;
			
			if(closure != null) _reactorServiceDiscoveryOptions.userSpecObj(closure);
			
			switch(serviceEndpointDiscoveryOptionImpl._transport)
			{
			case ServiceEndpointDiscoveryOption.TransportProtocol.UNKNOWN:
				break;
			case ServiceEndpointDiscoveryOption.TransportProtocol.TCP:
				_reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
				break;
			case ServiceEndpointDiscoveryOption.TransportProtocol.WEB_SOCKET:
				_reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET);
				break;
			default:
				strBuilder().append("Invalid transport protocol ").append(serviceEndpointDiscoveryOptionImpl._transport)
					.append(" specified in ServiceEndpointDiscoveryOption.transport()");
				
				throw ommIUExcept().message(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}
			
			switch(serviceEndpointDiscoveryOptionImpl._dataFormat)
			{
			case ServiceEndpointDiscoveryOption.DataformatProtocol.UNKNOWN:
				break;
			case ServiceEndpointDiscoveryOption.DataformatProtocol.RWF:
				_reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);
				break;
			case ServiceEndpointDiscoveryOption.DataformatProtocol.JSON2:
				_reactorServiceDiscoveryOptions.transport(ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2);
				break;
			default:
				strBuilder().append("Invalid dataformat protocol ").append(serviceEndpointDiscoveryOptionImpl._dataFormat)
					.append(" specified in ServiceEndpointDiscoveryOption.dataFormat()");
				
				throw ommIUExcept().message(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}
			
			if (_reactor.queryServiceDiscovery(_reactorServiceDiscoveryOptions, _reactorErrorInfo) != ReactorReturnCodes.SUCCESS)
			{
				strBuilder().append("Failed to query service discovery (Reactor.queryServiceDiscovery).")
				.append("' Error Id='").append(_reactorErrorInfo.error().errorId()).append("' Internal sysError='")
				.append(_reactorErrorInfo.error().sysError()).append("' Error Location='")
				.append(_reactorErrorInfo.location()).append("' Error Text='")
				.append(_reactorErrorInfo.error().text()).append("'. ");
				
				throw ommIUExcept().message(_strBuilder.toString(), _reactorErrorInfo.code());
			}
		}
		finally
		{
			_userLock.unlock();
		}
	}

	@Override
	public void uninitialize()
	{	
		_userLock.lock();
		try
		{
			if(!_reactor.isShutdown())
			{
				_reactorErrorInfo.clear();
				_reactor.shutdown(_reactorErrorInfo);
			}
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();

		return _ommIUExcept;
	}
	
	StringBuilder strBuilder()
	{
		if(_strBuilder == null)
		{
			_strBuilder = new StringBuilder(255);
		}
		else
		{
			_strBuilder.setLength(0);
		}
		
		return _strBuilder;
	}

	@Override
	public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event)
	{
		ReactorServiceEndpointInfo reactorServiceEndpointInfo;
		ServiceEndpointDiscoveryInfoImpl serviceEndpointDiscoveryInfoImpl;
		
		_ServiceEndpointDiscoveryEventImpl._closure = event.userSpecObject();
		_ServiceEndpointDiscoveryEventImpl._serviceEndpointDiscovery = this;
		if(event.errorInfo().code() == ReactorReturnCodes.SUCCESS)
		{
			_serviceEndpointDiscoveryRespImpl._serviceEndpointInfoList.clear();
			for(int index = 0 ; index < event.serviceEndpointInfo().size() ; index++ )
			{
				reactorServiceEndpointInfo = event.serviceEndpointInfo().get(index);
				serviceEndpointDiscoveryInfoImpl = new ServiceEndpointDiscoveryInfoImpl();
				
				for(int valueIndex = 0; valueIndex < reactorServiceEndpointInfo.dataFormatList().size(); valueIndex++)
				{
					serviceEndpointDiscoveryInfoImpl._dataFormatList.add(reactorServiceEndpointInfo.dataFormatList().get(valueIndex).toString());
				}
				
				for(int valueIndex = 0; valueIndex < reactorServiceEndpointInfo.locationList().size(); valueIndex++)
				{
					serviceEndpointDiscoveryInfoImpl._locationList.add(reactorServiceEndpointInfo.locationList().get(valueIndex).toString());
				}
				
				serviceEndpointDiscoveryInfoImpl._endpoint = reactorServiceEndpointInfo.endPoint();
				serviceEndpointDiscoveryInfoImpl._port = reactorServiceEndpointInfo.port();
				serviceEndpointDiscoveryInfoImpl._provider = reactorServiceEndpointInfo.provider();
				serviceEndpointDiscoveryInfoImpl._transport = reactorServiceEndpointInfo.transport();
				_serviceEndpointDiscoveryRespImpl._serviceEndpointInfoList.add(serviceEndpointDiscoveryInfoImpl);
			}
			
			_client.onSuccess(_serviceEndpointDiscoveryRespImpl, _ServiceEndpointDiscoveryEventImpl);
		}
		else
		{
			_client.onError(event.errorInfo().error().text(), _ServiceEndpointDiscoveryEventImpl);
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
}
