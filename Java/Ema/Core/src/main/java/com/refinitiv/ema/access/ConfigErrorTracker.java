/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;

import org.slf4j.Logger;

import com.refinitiv.ema.access.OmmLoggerClient.Severity;

class ConfigError 
{
	String text;
	int level;
}

class ConfigErrorTracker 
{
	ArrayList<ConfigError> errorPool;
	ArrayList<ConfigError> _errorList;
	StringBuilder _errorMaker; 
	
	ConfigErrorTracker()
	{
		errorPool = new ArrayList<ConfigError>(64);
		for(int i = 0; i < errorPool.size(); i++)
		{
			ConfigError error = new ConfigError();
			errorPool.add(error);
		}
		
		_errorList = new ArrayList<ConfigError>();
		_errorMaker = new StringBuilder(1024);
	}
	
	void add(String errorText, int severity) 
	{
		ConfigError error = null;
				
		if( errorPool.size() > 0 )
			error = errorPool.remove(0);
		else
			error = new ConfigError();
		
		error.text = errorText;
		error.level = severity;
		
		_errorList.add(error);
	}

	ConfigErrorTracker append(String string) 
	{
		_errorMaker.append(string);
		return this;
	}

	ConfigErrorTracker append(int number) 
	{
		_errorMaker.append(number);
		return this;
	}
	
	void create(int severity) 
	{
		String error = _errorMaker.toString();
		add(error,severity);
		_errorMaker.setLength(0);
	}

	String text() 
	{
		String error = _errorMaker.toString();
		_errorMaker.setLength(0);
		return error;
	}

	boolean hasErrors() 
	{
		if(_errorList.size() > 0 )
			return true;
		else
			return false;
	}
	
	ConfigError removeNextError()
	{
		if(_errorList.size() == 0 )
			return null;
		
		ConfigError error = _errorList.remove(0);
		errorPool.add(error);
		return error;
	}
	
	void flushErrors()
	{
		if(_errorList.size() == 0 )
			return;

		System.out.println("==== Flushing Errors =========================");
		
		ConfigError error = null;
		while( (error=removeNextError()) != null )
		{
			System.out.format("%s - %s\n",error.text,OmmLoggerClient.loggerSeverityAsString(error.level));
		}
		System.out.println("==============================================");
	}
	
	void log(OmmCommonImpl logMessage, Logger loggerClient)
	{
		if(_errorList.size() == 0 )
			return;
		
		for (ConfigError error : _errorList)
		{
			switch (error.level)
			{
			case OmmLoggerClient.Severity.DEBUG:
				if (loggerClient.isDebugEnabled())
					loggerClient.debug(logMessage.formatLogMessage("EmaConfig", error.text, Severity.DEBUG));
				break;
			case OmmLoggerClient.Severity.TRACE:
				if (loggerClient.isTraceEnabled())
					loggerClient.trace(logMessage.formatLogMessage("EmaConfig", error.text, Severity.TRACE));
				break;
			case OmmLoggerClient.Severity.INFO:
				if (loggerClient.isInfoEnabled())
					loggerClient.info(logMessage.formatLogMessage("EmaConfig", error.text, Severity.INFO));
				break;
			case OmmLoggerClient.Severity.WARNING:
				if (loggerClient.isWarnEnabled())
					loggerClient.warn(logMessage.formatLogMessage("EmaConfig", error.text, Severity.WARNING));
				break;
			case OmmLoggerClient.Severity.ERROR:
				if (loggerClient.isErrorEnabled())
					loggerClient.error(logMessage.formatLogMessage("EmaConfig", error.text, Severity.ERROR));
				break;
			default:
				if (loggerClient.isErrorEnabled())
					loggerClient.error(logMessage.formatLogMessage("EmaConfig", "Invalid error level", Severity.ERROR));
			}
		}
	}
}
