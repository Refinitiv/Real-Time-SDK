///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019,2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * 
 * OmmInvalidUsageException is thrown when application violates usage of EMA interfaces.
 * 
 * @see OmmException
 */
public abstract class OmmInvalidUsageException extends OmmException
{
	private static final long serialVersionUID = 3013521073630137359L;
	protected int _errorCode = ErrorCode.NONE;
	
	/**
	 * ErrorCode represents error codes for handling the exception.
	 */
	public static class ErrorCode
	{
		/**
		 * No specific error code.
		 */
		public final static int NONE = 0;
		
		/**
		 * General failure.
		 */
		public final static int FAILURE = -1;
		
		/**
		 * This indicates that Write was unable to send all fragments with the current call and must continue fragmenting.
		 */
		public static final int WRITE_CALL_AGAIN = -2;		
		/**
		 * There are no buffers available from the buffer pool.
		 */
		public final static int NO_BUFFERS = -3;
		
		 /**
	     * Indicates that a parameter was out of range.
	     */
		public final static int PARAMETER_OUT_OF_RANGE = -4;
		
		 /**
	     * Indicates that a parameter was invalid.
	     */
	    public static final int PARAMETER_INVALID = -5;

	    /**
	     * The interface is being improperly used.
	     */
	    public static final int INVALID_USAGE = -6;

	    /**
	     * An error was encountered during a channel operation.
	     */
	    public static final int CHANNEL_ERROR = -7;
	    
	    /**
	     * The interface is attempting to write a message to the Reactor
	     * with an invalid encoding. 
	     */
	    public static final int INVALID_ENCODING = -8;

	    /**
	     * The interface is attempting to write a message to the TunnelStream,
	     * but the persistence file is full. 
	     */
	    public static final int PERSISTENCE_FULL = -9;
	    
	    /**
	     * Indicates that the specified version is not supported
	     */
	    public static final int VERSION_NOT_SUPPORTED = -16;
		
		/**
		 * 
		 * The buffer provided (or the remaining buffer space for message packing) 
		 * does not have sufficient space to perform the operation.
		 */
		public final static int BUFFER_TOO_SMALL = -21;
		
		/**
		 * 
		 * An invalid argument was provided.
		 */
		public final static int INVALID_ARGUMENT = -22;
		
		/**
		 * No encoder is available for the data type specified.
		 */
		public final static int ENCODING_UNAVALIABLE = -23;
		
		/**
		 * The data type is unsupported, may indicate invalid containerType or primitiveType specified.
		 */
		public final static int UNSUPPORTED_DATA_TYPE = -24;
		
		/**
		 * An encoder was used in an unexpected sequence.
		 */
		public final static int UNEXPECTED_ENCODER_CALL = -25;
		
		/**
		 * Not enough data was provided.
		 */
		public final static int INCOMPLETE_DATA = -26;
		
		/**
		 * A Database containing the Set Definition for encoding the desired set was not provided.
		 */
		public final static int SET_DEF_NOT_PROVIDED = -27;
		
		/**
		 * Invalid data provided to function.
		 */
		public final static int INVALID_DATA = -29;
		
		/**
		 * Set definition is not valid.
		 */
		public final static int ILLEGAL_LOCAL_SET_DEF = -30;
		
		/**
		 * Maximum number of set definitions has been exceeded.
		 */
		public final static int TOO_MANY_LOCAL_SET_DEFS = -31;
		
		/**
		 * A duplicate set definition has been received.
		 */
		public final static int DUPLICATE_LOCAL_SET_DEFS = -32;
		
		/**
		 * Iterator is nested too deeply. There is a limit of 16 levels.
		 */
		public final static int ITERATOR_OVERRUN = -33;
		
		/**
		 * A value being encoded into a set is outside of the valid range of the type given by that set.
		 */
		public final static int VALUE_OUT_OF_RANGE = -34;
		
		/**
		 * A display string had multiple enumerated values that correspond to it.
		 */
		public final static int DICT_DUPLICATE_ENUM_VALUE = -35;
		
		/**
		 * Multicast Transport Warning: An unrecoverable packet gap was detected and some content may have been lost.
		 */
		public final static int PACKET_GAP_DETECTED = -61;
		
		/**
		 * Multicast Transport Warning: Application is consuming more slowly than data is being provided.  Gaps are likely.
		 */
		public final static int SLOW_READER = -62;
		
		/**
		 * Multicast Transport Warning: Network congestion detected.  Gaps are likely.
		 */
		public final static int CONGESTION_DETECTED = -63;
		
		/**
		 * Invalid user's operation.
		 */
		public final static int INVALID_OPERATION = -4048;
		
		/**
		 * No active channel.
		 */
		public final static int NO_ACTIVE_CHANNEL = -4049;
		
		/**
		 * Unsupported channel type.
		 */
		public final static int UNSUPPORTED_CHANNEL_TYPE = -4050;
		
		/**
		 * Unsupported server type.
		 */
		public final static int UNSUPPORTED_SERVER_TYPE = -4051;
		
		/**
		 * Login request timeout.
		 */
		public final static int LOGIN_REQUEST_TIME_OUT = -4052;
		
		/**
		 * Login request rejected from connected peer.
		 */
		public final static int LOGIN_REQUEST_REJECTED = -4053;

		/**
		 * Directory request timeout.
		 */
		public final static int DIRECTORY_REQUEST_TIME_OUT = -4054;
		
		/**
		 * Dictionary request timeout.
		 */
		public final static int DICTIONARY_REQUEST_TIME_OUT = -4055;

		/**
		 * Internal Error in EMA.
		 */
		public final static int INTERNAL_ERROR = -4060;
	}

	/**
	 * Returns the error code to describe the error case defined in the ErrorCode
	 * 
	 * @return an error code causing exception
	 */
	public int errorCode()
	{
		return _errorCode;
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
		_toString.append("Exception Type='").append(exceptionTypeAsString()).append("', Text='").append(_exceptMessage).append("', Error Code='").append(_errorCode).append("'");
		
		return _toString.toString();
	}
}