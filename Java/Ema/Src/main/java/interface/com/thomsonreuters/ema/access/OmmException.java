///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmException is a parent class for all exception types thrown by EMA.
 * 
 * @see OmmInvalidHandleException
 * @see OmmInvalidUsageException
 * @see OmmOutOfRangeException
 * @see OmmUnsupportedDomainTypeException
 * @see OmmInvalidConfigurationException
 * @see OmmConsumerErrorClient
 */
public abstract class OmmException extends RuntimeException
{
	private static final long serialVersionUID = -7319103509448033081L;
	protected StringBuilder _toString = new StringBuilder();
	protected String _exceptMessage;

	/**
	 * ExceptionType represents exception type.
	 */
	public class ExceptionType
	{
		/**
		 * Indicates invalid usage exception
		 */
		public static final int OmmInvalidUsageException = 1;
		
		/**
		 * Indicates invalid configuration exception
		 */
		public static final int OmmInvalidConfigurationException = 2;

		/**
		 * Indicates out of range exception
		 */
		public static final int OmmOutOfRangeException = 3;
		
		/**
		 * Indicates invalid handle exception
		 */
		public static final int OmmInvalidHandleException = 4;
		
		/**
		 *  Indicates unsupported domain type exception
		 */
		public static final int OmmUnsupportedDomainTypeException = 5;
	}

	
	/**
	 * Returns the ExceptionType value as a string format.
	 * 
	 * @return string representation of this object's exception type as string
	 */
	public abstract String exceptionTypeAsString();

	/**
	 * Returns ExceptionType.
	 * 
	 * @return exception type value
	 */
	public abstract int exceptionType();

	/**
	 * Returns Text.
	 * 
	 * @return String with exception text information
	 */
	@Override
	public String getMessage()
	{
		return _exceptMessage;
	}
	/**
	 * Returns a string representation of the class instance.
	 * 
	 * @return string representation of the class instance
	 */
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("Exception Type='").append(exceptionTypeAsString()).append("', Text='").append(_exceptMessage).append("'");
		
		return _toString.toString();
	}
}