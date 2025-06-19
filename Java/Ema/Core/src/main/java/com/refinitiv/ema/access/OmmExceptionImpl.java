/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;


class OmmInvalidUsageExceptionImpl extends OmmInvalidUsageException
{
	private static final long serialVersionUID = 187450409103580543L;
	private static final String OMMINVALIDUSAGE_EXCEPTION_STRING = "OmmInvalidUsageException";
	
	@Override
	public String exceptionTypeAsString()
	{
		return OMMINVALIDUSAGE_EXCEPTION_STRING;
	}

	@Override
	public int exceptionType()
	{
		return ExceptionType.OmmInvalidUsageException;
	}

	OmmInvalidUsageException message(String exceptMessage, int errorCode)
	{
		_exceptMessage = exceptMessage;
		_errorCode = errorCode;
		return this;
	}
}

class OmmInvalidConfigurationExceptionImpl extends OmmInvalidConfigurationException
{
	private static final long serialVersionUID = 6748536997975623165L;
	private static final String OMMINVALIDCONFIGURATION_EXCEPTION_STRING = "OmmInvalidConfigurationException";

	@Override
	public String exceptionTypeAsString()
	{
		return OMMINVALIDCONFIGURATION_EXCEPTION_STRING;
	}

	@Override
	public int exceptionType()
	{
		return ExceptionType.OmmInvalidConfigurationException;
	}
	
	OmmInvalidConfigurationException message(String exceptMessage)
	{
		_exceptMessage = exceptMessage;
		return this;
	}
}

class OmmOutOfRangeExceptionImpl extends OmmOutOfRangeException
{
	private static final long serialVersionUID = -5693762909775005037L;
	private static final String OMMOUTOFRANGE_EXCEPTION_STRING = "OmmOutOfRangeException";
	
	@Override
	public String exceptionTypeAsString()
	{
		return OMMOUTOFRANGE_EXCEPTION_STRING;
	}

	@Override
	public int exceptionType()
	{
		return ExceptionType.OmmOutOfRangeException;
	}
	
	OmmOutOfRangeException message(String exceptMessage)
	{
		_exceptMessage = exceptMessage;
		return this;
	}
}

class OmmInvalidHandleExceptionImpl extends OmmInvalidHandleException
{
	private static final long serialVersionUID = 5597133832203009589L;
	private static final String OMMINVALIDHANDLE_EXCEPTION_STRING = "OmmInvalidHandleException";

	private long _handle;
	
	@Override
	public String exceptionTypeAsString()
	{
		return OMMINVALIDHANDLE_EXCEPTION_STRING;
	}

	@Override
	public int exceptionType()
	{
		return ExceptionType.OmmInvalidHandleException;
	}
	
	/** Returns the invalid handle.
	@return long value of handle causing exception
	 */
	public long handle()
	{
		return _handle;
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("Exception Type='").append(exceptionTypeAsString()).append("', Text='").append(_exceptMessage)
						.append("', Handle='").append(_handle).append("'");
		
		return _toString.toString();
	}

	OmmInvalidHandleException message(String exceptMessage, long handle) {
		_exceptMessage = exceptMessage;
		_handle = handle;
		return this;
	}
}

class OmmUnsupportedDomainTypeExceptionImpl extends OmmUnsupportedDomainTypeException
{
	private static final long serialVersionUID = -6027042776461020763L;
	private static final String OMMUNSUPPORTEDDOMAINTYPE_EXCEPTION_STRING = "OmmUnsupportedDomainTypeException";
	
	private int _domainType;
	
	@Override
	public String exceptionTypeAsString()
	{
		return OMMUNSUPPORTEDDOMAINTYPE_EXCEPTION_STRING;
	}

	@Override
	public int exceptionType()
	{
		return ExceptionType.OmmUnsupportedDomainTypeException;
	}
	
	@Override
	public int domainType()
	{
		return _domainType;
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("Exception Type='").append(exceptionTypeAsString()).append("', Text='").append(_exceptMessage)
						.append("', DomainType='").append(_domainType).append("'");
		
		return _toString.toString();
	}
	
	OmmUnsupportedDomainTypeException message(int domainType, String exceptMessage)
	{
		_domainType = domainType;
		_exceptMessage = exceptMessage;
		return this;
	}
}

class OmmJsonConverterExceptionImpl extends OmmJsonConverterException {

	private static final long serialVersionUID = -2146394732735338988L;
	private static final String OMMJSONCONVERTER_EXCEPTION_STRING = "OmmJsonConverterException";
	private int errorCode;
	private transient SessionInfo sessionInfo;

	@Override
	public int getErrorCode() {
		return errorCode;
	}

	@Override
	public SessionInfo getSessionInfo() {
		return sessionInfo;
	}

	@Override
	public String exceptionTypeAsString() {
		return OMMJSONCONVERTER_EXCEPTION_STRING;
	}

	@Override
	public int exceptionType() {
		return ExceptionType.OmmJsonConverterException;
	}

	OmmJsonConverterExceptionImpl message(SessionInfo sessionInfo, int errorCode, String exceptMessage) {
		this.errorCode = errorCode;
		this._exceptMessage = exceptMessage;
		this.sessionInfo = sessionInfo;
		return this;
	}

	@Override
	public String toString()
	{
		String sessionInfoTxt = "unavailable";
		
		if(sessionInfo != null && sessionInfo.getChannelInformation() != null)
		{
			sessionInfoTxt = sessionInfo.getChannelInformation().toString();
		}
		
		_toString.setLength(0);
		_toString.append("Exception Type='").append(exceptionTypeAsString()).append("', Text='").append(_exceptMessage)
				.append("', ErrorCode='").append(errorCode)
				.append("', SessionInfo='").append(sessionInfoTxt)
				.append("'");

		return _toString.toString();
	}
}
