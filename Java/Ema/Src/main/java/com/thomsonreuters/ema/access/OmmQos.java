///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmQos represents Quality Of Service information in Omm.
 */
public interface OmmQos extends Data
{
	/**
	 * Rate represents Qos rate.
	 */
	public static class  Rate
	{
		/**
		 * Indicates tick by tick rate
		 */
		public final static int TICK_BY_TICK = 0;

		/**
		 * Indicates just in time conflated rate
		 */
		public final static int JUST_IN_TIME_CONFLATED = 0xFFFFFF00;
	}

	/**
	 * Timeliness represents Qos timeliness.
	*/
	public static class  Timeliness
	{
		/**
		 * Indicates real time timeliness
		 */
		public final static int REALTIME = 0;

		/**
		 * Indicates timeliness with an unknown delay value
		 */
		public final static int INEXACT_DELAYED = 0xFFFFFFFF;
	}

	/**
	 * Returns the QosRate value as a string format.
	 * 
	 * @return string representation of this object Rate
	*/
	public String rateAsString();
		
	/**
	 * Returns the QosTimeliness value as a string format.
	 * 
	 * @return string representation of this object timeliness
	*/
	public String timelinessAsString();

	/**
	 * Returns Timeliness.
	 * 
	 * @return value of OmmQos Timeliness
	 */
	public int timeliness();

	/** Returns Rate.
		@return value of OmmQos Rate
	*/
	public int rate();
}