/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

class OmmLoggerClient
{
	final static String CR 						= "\n\t";
	final static String INFO_STRING 			= "Info";
	final static String WARNING_STRING 			= "Warning";
	final static String ERROR_STRING 			= "Error";
	final static String DEBUG_STRING 			= "Debug";
	final static String TRACE_STRING 			= "Trace";
	final static String DEFAULTSEVERITY_STRING 	= "Unknown Severity";
	
	final class Severity {
		final static int TRACE 		= 0; 
		final static int DEBUG 		= 1;
		final static int INFO 		= 2; 
		final static int WARNING 	= 3;
		final static int ERROR 		= 4;
	}
	
	static String loggerSeverityAsString(int severity)
	{
		switch (severity)
		{
			case Severity.INFO :
				return INFO_STRING;
			case Severity.WARNING :
				return WARNING_STRING;
			case Severity.ERROR :
				return ERROR_STRING;
			case Severity.DEBUG :
				return DEBUG_STRING;
			case Severity.TRACE :
				return TRACE_STRING;
			default :
				return DEFAULTSEVERITY_STRING + severity;
		}
	}
}

