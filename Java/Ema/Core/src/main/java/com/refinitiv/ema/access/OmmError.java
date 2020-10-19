///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmError represents received Omm data who fails to process properly.
 * <br>Objects of OmmError class are returned when an error is detected while processing 
 * received data.
 * <br>These objects are used for debugging purposes only.
 *
 * OmmError is a read only class.
 * 
 * @see Data
 */
public interface OmmError extends Data
{
	public static class ErrorCode
	{
		/**
		 * Indicates no error.
		 */
		public final static int NO_ERROR = 0;

		/**
		 * Indicates missing dictionary.
		 */
		public final static int NO_DICTIONARY = 1;

		/**
		 * Indicates internal iterator set failure.
		 */
		public final static int ITERATOR_SET_FAILURE = 2;

		/**
		 * Indicates internal iterator overrun failure.
		 */
		public final static int ITERATOR_OVERRUN = 3;

		/**
		 * Indicates field id was not found in dictionary.
		 */
		public final static int FIELD_ID_NOT_FOUND = 4;

		/**
		 * Indicates incomplete data.
		 */
		public final static int INCOMPLETE_DATA = 5;

		/**
		 * Indicates unsupported data type.
		 */
		public final static int UNSUPPORTED_DATA_TYPE = 6;

		/**
		 * Indicates set defined data is not present.
		 */
		public final static int NO_SET_DEFINITION = 7;

		/**
		 * Indicates unknown error.
		 */
		public final static int UNKNOWN_ERROR = 8;
	}

	/**
	 * Returns the ErrorCode value as a string format.
	 * 
	 * @return string representation of error code (e.g. "ITERATOR_SETFAILURE")
	 */
	public String errorCodeAsString();

	/**
	 * Returns ErrorCode.
	 * 
	 * @return error code
	 */
	public int errorCode();
}