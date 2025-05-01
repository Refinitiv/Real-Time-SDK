/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.rdm;

/** Directory specific RDM definitions */
public class Directory
{
    // Directory class cannot be instantiated
    private Directory()
    {
        throw new AssertionError();
    }

	/**
	 * Explains the content of the Data.
	 */
	public static class DataTypes
	{
	    // DataTypes class cannot be instantiated
	    private DataTypes()
	    {
	        throw new AssertionError();
	    }

	    /** None */
	    public static final int NONE = 0;
	    /** Time */
	    public static final int TIME = 1;
	    /** Alert */
	    public static final int ALERT = 2;
	    /** Headline */
	    public static final int HEADLINE = 3;
	    /** Status */
	    public static final int STATUS = 4;
	}
	
	/**
	 * Provides additional status information about the upstream source.
	 */
	public static class LinkCodes
	{
	    // LinkCodes class cannot be instantiated
	    private LinkCodes()
	    {
	        throw new AssertionError();
	    }

	    /** None */
	    public static final int NONE = 0;
	    /** Ok */
	    public static final int OK = 1;
	    /** Recovery Started */
	    public static final int RECOVERY_STARTED = 2;
	    /** Recovery Completed */
	    public static final int RECOVERY_COMPLETED = 3;
	}

	/**
	 * Indicates whether the upstream source that provides data to a service is up
	 * or down.
	 */
	public static class LinkStates
	{
	    // LinkStates class cannot be instantiated
	    private LinkStates()
	    {
	        throw new AssertionError();
	    }

	    /** Down */
	    public static final int DOWN = 0;
	    /** Up */
	    public static final int UP = 1;
	}
	
	/**
	 * Indicates whether the upstream source is interactive or broadcast. This does
	 * not describe whether the service itself is interactive or broadcast.
	 */
	public static class LinkTypes
	{
	    // LinkTypes class cannot be instantiated
	    private LinkTypes()
	    {
	        throw new AssertionError();
	    }

	    /** Interactive */
	    public static final int INTERACTIVE = 1;
	    /** Broadcast */
	    public static final int BROADCAST = 2;
	}
	
	/**
	 * Combination of bit values to indicate specific information about a RDM directory service.
	 */
	public static class ServiceFilterFlags
	{
	    // ServiceFilterFlags class cannot be instantiated
	    private ServiceFilterFlags()
	    {
	        throw new AssertionError();
	    }

	    /** Source Info Filter Mask */
	    public static final int INFO = 0x00000001;
	    /** Source State Filter Mask */
	    public static final int STATE = 0x00000002;
	    /** Source Load Filter Mask */
	    public static final int GROUP = 0x00000004;
	    /** Source Load Filter Mask */
	    public static final int LOAD = 0x00000008;
	    /** Source Data Filter Mask */
	    public static final int DATA = 0x00000010;
	    /** Source Communication Link Information */
	    public static final int LINK = 0x00000020;
	    /** Sequenced Multicast Information */
	    public static final int SEQ_MCAST = 0x00000040;
	}
	
	/**
	 * Filter IDs for RDM directory service.
	 */
	public static class ServiceFilterIds
	{
	    // ServiceFilterIds class cannot be instantiated
	    private ServiceFilterIds()
	    {
	        throw new AssertionError();
	    }

	    /** Service Info Filter ID */
	    public static final int INFO = 1;
	    /** Source State Filter ID */
	    public static final int STATE = 2;
	    /** Source Load Filter ID */
	    public static final int GROUP = 3;
	    /** Source Load Filter ID */
	    public static final int LOAD = 4;
	    /** Source Data Filter ID */
	    public static final int DATA = 5;
	    /** Communication Link Filter ID */
	    public static final int LINK = 6;
	    /** Sequenced Multicast Filter ID */
	    public static final  int SEQ_MCAST = 7;
	}

	/**
	 * Indicates whether the original provider of the data is available to respond
	 * to new requests. If the service is down, requests for data may be handled by
	 * the immediate upstream provider (to which the consumer is directly
	 * connected). However, because the most current data might be serviced from a
	 * cached copy while the source is down, the most current data may not be
	 * immediately available.Indicates whether the original provider of the data is
	 * available to respond to new requests. If the service is down, requests for
	 * data may be handled by the immediate upstream provider (to which the consumer
	 * is directly connected). However, because the most current data might be
	 * serviced from a cached copy while the source is down, the most current data
	 * may not be immediately available.
	 */
	public static class ServiceStates
	{
	    // ServiceStates class cannot be instantiated
	    private ServiceStates()
	    {
	        throw new AssertionError();
	    }

	    /** Service state down */
	    public static final int DOWN = 0;
	    /** Service state up */
	    public static final int UP = 1;
	}

	/**
	 * Indicates how the downstream component is using the service.
	 */
	public static class SourceMirroringMode
	{
	    // SourceMirroringMode class cannot be instantiated
	    private SourceMirroringMode()
	    {
	        throw new AssertionError();
	    }

	    /**
	     * The downstream device is using the data from this service, and is not
	     * receiving it from any other service.
	     */
	    public static final int ACTIVE_NO_STANDBY = 0;

	    /**
	     * The downstream device is using the data from this service, but is also
	     * getting it from another service.
	     */
	    public static final int ACTIVE_WITH_STANDBY = 1;

	    /**
	     * The downstream device is getting data from this service, but is actually
	     * using data from another service.
	     */
	    public static final int STANDBY = 2;
	}
	
	/**
	 *  Indicates the warm standby service type 
	 */
	public static class WarmStandbyDirectoryServiceTypes
	{
		/**
		 * Indicates that the provider for this service is the active server.
		 */
		public static final int ACTIVE = 0;
		
		/**
		 * Indicates that the provider for this service is the standby server.
		 */
		public static final int STANDBY = 1;
		
	}
}
