///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.OmmInvalidConfigurationException;
import com.thomsonreuters.ema.access.OmmInvalidHandleException;
import com.thomsonreuters.ema.access.OmmInvalidUsageException;
import com.thomsonreuters.ema.access.OmmOutOfRangeException;
import com.thomsonreuters.ema.access.OmmUnsupportedDomainTypeException;

class OmmInvalidUsageExceptionImpl extends OmmInvalidUsageException
{
	private static final long serialVersionUID = 187450409103580543L;
	private static final String OMMINVALIDUSAGE_EXCEPTION_STRING = "OmmInvalidUsageException";
	
	public String exceptionTypeToString()
	{
		return OMMINVALIDUSAGE_EXCEPTION_STRING;
	}

	public int exceptionType()
	{
		return ExceptionType.OmmInvalidUsageException;
	}

	OmmInvalidUsageException message(String exceptMessage)
	{
		_exceptMessage = exceptMessage;
		return this;
	}
}

class OmmInvalidConfigurationExceptionImpl extends OmmInvalidConfigurationException
{
	private static final long serialVersionUID = 6748536997975623165L;
	private static final String OMMINVALIDCONFIGURATION_EXCEPTION_STRING = "OmmInvalidConfigurationException";

	public String exceptionTypeToString()
	{
		return OMMINVALIDCONFIGURATION_EXCEPTION_STRING;
	}

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
	
	public String exceptionTypeToString()
	{
		return OMMOUTOFRANGE_EXCEPTION_STRING;
	}

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
	
	public String exceptionTypeToString()
	{
		return OMMINVALIDHANDLE_EXCEPTION_STRING;
	}

	public int exceptionType()
	{
		return ExceptionType.OmmInvalidHandleException;
	}
	
	/** Returns the invalid handle.
	@return UInt64 value of handle causing exception
	 */
	public long handle()
	{
		return _handle;
	}
	
	OmmInvalidHandleException message(String exceptMessage)
	{
		_exceptMessage = exceptMessage;
		return this;
	}
	
	OmmInvalidHandleException handle(long handle)
	{
		_handle = handle;
		return this;
	}
}

class OmmUnsupportedDomainTypeExceptionImpl extends OmmUnsupportedDomainTypeException
{
	private static final long serialVersionUID = -6027042776461020763L;
	private static final String OMMUNSUPPORTEDDOMAINTYPE_EXCEPTION_STRING = "OmmUnsupportedDomainTypeException";
	
	private int _domainType;
	
	public String exceptionTypeToString()
	{
		return OMMUNSUPPORTEDDOMAINTYPE_EXCEPTION_STRING;
	}

	public int exceptionType()
	{
		return ExceptionType.OmmUnsupportedDomainTypeException;
	}
	
	public int domainType()
	{
		return _domainType;
	}
	
	OmmUnsupportedDomainTypeException message(int domainType, String exceptMessage)
	{
		_domainType = domainType;
		_exceptMessage = exceptMessage;
		return this;
	}
}
