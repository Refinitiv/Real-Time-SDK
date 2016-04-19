///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmState represents State information in Omm.
 * <p>OmmState is used to represent state of item, item group and service.</p>
 * <p>OmmState encapsulates stream state, data state, status code and status text information.</p>
 * 
 * <p>OmmState is a read only class.
 * <br>OmmState is used for extraction of OmmState info only.</p>
 */
public interface OmmState extends Data
{
	/** 
	 * StreamState represents item stream state.
	 */
	public static class StreamState
	{
		/**
		 * Indicates the stream is opened and will incur interest after the final refresh.
		 */
		public final static int OPEN			 = 1;
	
		/**
		 * Indicates the item will not incur interest after	the final refresh.
		 */
		public final static int NON_STREAMING	 = 2;
	
		/**
		 * Indicates the stream is closed, typically unobtainable or 
		 * identity indeterminable due to a comms outage.
		 * The item may be available in the future.
		 */
		public final static int	CLOSED_RECOVER	 = 3;
	
		/**
		 * Indicates the stream is closed.
		 */
		public final static int CLOSED			 = 4;
	
		/**
		 * Indicates the stream is closed and has been renamed.
		 * The stream is available with another name.
		 * This stream state is only valid for refresh messages.
		 * The new item name is in the Name get accessor methods.
		 */
		public final static int CLOSED_REDIRECTED = 5;
	}
	
	/** 
	 * DataState represents item data state.
	 */
	public static class DataState
	{
		/**
		 * Indicates the health of the data item did not change.
		 */
		public final static int NO_CHANGE	= 0;
	
		/**
		 * Indicates the entire data item is healthy.
		 */
		public final static int OK			= 1;
	
		/**
		 * Indicates the health of some or all of the item's data is stale or unknown.
		 */
		public final static int SUSPECT		= 2;
	}
	
	/** 
	 * StatusCode represents status code.
	 */
	public static class  StatusCode
	{
		/** None */
		public final static int NONE						= 0;

		/** Not Found */
		public final static int NOT_FOUND					= 1;
		
		/** Timeout */
		public final static int TIMEOUT						= 2;
		
		/** Not Authorized */
		public final static int NOT_AUTHORIZED				= 3;
		
		/** Invalid Argument */
		public final static int INVALID_ARGUMENT			= 4;
		
		/** Usage Error */
		public final static int USAGE_ERROR					= 5;
		
		/** Pre-empted */
		public final static int PREEMPTED					= 6;
		
		/** Just In Time Filtering Started */
		public final static int JUST_IN_TIME_CONFLATION_STARTED	= 7;
		
		/** Tick By Tick Resumed */
		public final static int TICK_BY_TICK_RESUMED		= 8;
		
		/** Fail-over Started */
		public final static int FAILOVER_STARTED			= 9;
		
		/** Fail-over Completed */
		public final static int FAILOVER_COMPLETED			= 10;
		
		/** Gap Detected */
		public final static int GAP_DETECTED				= 11;
		
		/** No Resources */
		public final static int NO_RESOURCES				= 12;
		
		/** Too Many Items */
		public final static int TOO_MANY_ITEMS				= 13;
		
		/** Already Open */
		public final static int ALREADY_OPEN				= 14;
		
		/** Source Unknown */
		public final static int SOURCE_UNKNOWN				= 15;
		
		/** Not Open */
		public final static int NOT_OPEN					= 16;
		
		/** Non Updating Item */
		public final static int NON_UPDATING_ITEM			= 19;
		
		/** Unsupported View Type */
		public final static int UNSUPPORTED_VIEW_TYPE		= 20;
		
		/** Invalid View */
		public final static int INVALID_VIEW				= 21;
		
		/** Full View Provided */
		public final static int FULL_VIEW_PROVIDED			= 22;
		
		/** Unable To Request As Batch */
		public final static int UNABLE_TO_REQUEST_AS_BATCH		= 23;
		
		/** Request does not support batch or view */
		public final static int NO_BATCH_VIEW_SUPPORT_IN_REQ	= 26;
		
		/** Exceeded maximum number of mounts per user */
		public final static int EXCEEDED_MAX_MOUNTS_PER_USER	= 27;
		
		/** Internal error from sender */
		public final static int ERROR							= 28;
		
		/** Connection to DACS down, users are not allowed to connect */
		public final static int DACS_DOWN						= 29;
		
		/** User unknown to permissioning system, it could be DACS, AAA or EED */
		public final static int USER_UNKNOWN_TO_PERM_SYS		= 30;
		
		/** Maximum logins reached */
		public final static int DACS_MAX_LOGINS_REACHED			= 31;
		
		/** User is not allowed to use application */
		public final static int DACS_USER_ACCESS_TO_APP_DENIED	= 32;
		
		public final static int INVALID_FORMED_MSG				= 256;
		public final static int CHANNEL_UNAVAILABLE				= 257;
		public final static int SERVICE_UNAVAILABLE				= 258;
		public final static int SERVICE_DOWN					= 259;
		public final static int SERVICE_NOT_ACCEPTING_REQUESTS  = 260;
		public final static int LOGIN_CLOSED					= 261;
		public final static int DIRECTORY_CLOSED				= 262;
		public final static int ITEM_NOT_FOUND					= 263;
		public final static int DICTIONARY_UNAVAILABLE			= 264;
		public final static int FIELD_ID_NOT_FOUND_DICTIONARY_UNAVAILABLE = 265;
		public final static int ITEM_REQUEST_TIMEOUT			= 266;
	}
	
	/**
	 * Returns the StreamState value as a string format.
	 * 
	 * @return string representation of this object's StreamState
	 */
	public String streamStateAsString();
		
	/**
	 * Returns the DataState value as a string format.
	 * 
	 * @return string representation of this object's DataState
	*/
	public String dataStateAsString();
		
	/**
	 * Returns the StatusCode value as a string format.
	 * 
	 * @return string representation of this object's StatusCode
	 */
	public String statusCodeAsString();
		
	/**
	 * Returns StreamState.
	 * 
	 * @return value of StreamState
	 */
	public int streamState();
	
	/**
	 * Returns DataState.
	 * 
	 * @return value of DataState
	*/
	public int dataState();
	
	/**
	 * Returns StatusCode.
	 * 
	 * @return value of StatusCode
	 */
	public int statusCode();
	
	/**
	 * Returns StatusText.
	 * 
	 * @return String containing status text information
	 */
	public String statusText();
}