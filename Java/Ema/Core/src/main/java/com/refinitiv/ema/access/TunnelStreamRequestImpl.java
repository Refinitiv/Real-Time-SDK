/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */


package com.refinitiv.ema.access;

public class TunnelStreamRequestImpl implements TunnelStreamRequest
{
	private int _domainType;
	private int _serviceId;
	private int _responseTimeout;
	private int _guaranteedOutputBuffers;
	private boolean _serviceIdSet;
	private boolean _serviceNameSet;
	private boolean _nameSet;
	private String _serviceName;
	private String _name;
	private ClassOfService _cos;
	private TunnelStreamLoginReqMsg _tunnelStreamLoginReqMsg;
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private OmmUnsupportedDomainTypeExceptionImpl _ommUDTExcept;

	TunnelStreamRequestImpl()
	{
		_responseTimeout = 60;
		_guaranteedOutputBuffers = 50;
	}

	@Override
	public TunnelStreamRequest clear()
	{
		_domainType = 0;
		_serviceId = 0;
		_responseTimeout = 60;
		_guaranteedOutputBuffers = 50;
		_serviceIdSet = false;
		_serviceNameSet = false;
		_nameSet = false;
		_cos = null;

		return this;
	}

	@Override
	public TunnelStreamRequest domainType(int domainType)
	{
		if (domainType > 255)
			throw ommUDTExcept().message(domainType, "Passed in domain type is not supported.");

		_domainType = domainType;
		return this;
	}

	@Override
	public TunnelStreamRequest serviceId(int serviceId)
	{
		if (_serviceNameSet)
			throw ommIUExcept().message("Attempt to set serviceId while serviceName is already set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		_serviceIdSet = true;
		_serviceId = serviceId;
		return this;

	}

	@Override
	public TunnelStreamRequest serviceName(String serviceName)
	{
		if (_serviceIdSet)
			throw ommIUExcept().message("Attempt to set serviceName while serviceId is already set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		_serviceNameSet = true;
		_serviceName = serviceName;
		return this;
	}

	@Override
	public TunnelStreamRequest name(String name)
	{
		_nameSet = true;
		_name = name;
		return this;
	}

	@Override
	public TunnelStreamRequest responseTimeout(int timeout)
	{
		_responseTimeout = timeout;
		return this;
	}

	@Override
	public TunnelStreamRequest guaranteedOutputBuffers(int outputBuffers)
	{
		_guaranteedOutputBuffers = outputBuffers;
		return this;
	}

	@Override @Deprecated
	public TunnelStreamRequest guaranteedOuputBuffers(int outputBuffers)
	{
		return guaranteedOutputBuffers(outputBuffers);
	}

	@Override
	public TunnelStreamRequest classOfService(ClassOfService cos)
	{
		_cos = cos;
		return this;
	}

	@Override
	public TunnelStreamRequest loginReqMsg(ReqMsg loginReqMsg)
	{
		if (_tunnelStreamLoginReqMsg == null)
		{
			_tunnelStreamLoginReqMsg = new TunnelStreamLoginReqMsg();
		}

		_tunnelStreamLoginReqMsg.loginReqMsg(loginReqMsg);
		return this;
	}

	@Override
	public boolean hasServiceId()
	{
		return _serviceIdSet;
	}

	@Override
	public boolean hasServiceName()
	{
		return _serviceNameSet;
	}

	@Override
	public boolean hasName()
	{
		return _nameSet;
	}

	@Override
	public boolean hasLoginReqMsg()
	{
		if (_tunnelStreamLoginReqMsg != null)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	@Override
	public int domainType()
	{
		return _domainType;
	}

	@Override
	public int serviceId()
	{

		if (_serviceIdSet != true)
			throw ommIUExcept().message("Attempt to get servieId while it is not set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return _serviceId;
	}

	@Override
	public String serviceName()
	{
		if (_serviceNameSet == false)
			throw ommIUExcept().message("Attempt to get serviceName while it is not set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return _serviceName;
	}

	@Override
	public String name()
	{
		if (_nameSet == false)
			throw ommIUExcept().message("Attempt to get Name while it is not set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return _name;
	}

	@Override
	public int responseTimeOut() 
	{
		return _responseTimeout;
	}

	@Override
	public int guaranteedOutputBuffers() 
	{
		return _guaranteedOutputBuffers;
	}

	@Override
	public ClassOfService classOfService() 
	{
		return _cos;
	}

	@Override
	public ReqMsg loginReqMsg() 
	{

		if (_tunnelStreamLoginReqMsg == null)
			throw ommIUExcept().message("Attempt to get loginReqMsg while it is not set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return _tunnelStreamLoginReqMsg.loginReqMsg();
	}
	
	public TunnelStreamLoginReqMsg tunnelStreamLoginReqMsg()
	{
		return _tunnelStreamLoginReqMsg;
	}

	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();

		return _ommIUExcept;
	}

	OmmUnsupportedDomainTypeExceptionImpl ommUDTExcept()
	{
		if (_ommUDTExcept == null)
			_ommUDTExcept = new OmmUnsupportedDomainTypeExceptionImpl();

		return _ommUDTExcept;
	}
}
