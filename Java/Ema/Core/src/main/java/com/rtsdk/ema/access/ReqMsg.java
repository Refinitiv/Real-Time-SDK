///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.nio.ByteBuffer;

/**
 * ReqMsg allows consumer application to express its interest in an item.
 * 
 * <p>Among other attributes, ReqMsg conveys item's name, service, domain type, and
 * desired quality of service.</p>
 * 
 * <p>ReqMsg may also be used to:<br>
 * specify application interest	in a dynamic view,<br>
 * open a batch of items,<br>
 * or request a symbol list item with its data.</p>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and getting of information from ReqMsg.<br>
 * Objects of this class are not cache-able.<br>
 * Decoding of just encoded ReqMsg in the same application is not supported.
 *
 * @see Data
 * @see Msg
 */
public interface ReqMsg extends Msg
{
	/**
	 * Rate represents the Qos Rate.
	 */
	public class Rate
	{
		/**
		 * Rate is Tick By Tick, indicates every change to information is conveyed
		 */
		public final static int TICK_BY_TICK = 0;

		/**
		 * Rate is Just In Time Conflated, indicates extreme bursts of data that may be conflated.
		 */
		public final static int JIT_CONFLATED = 0xFFFFFF00;

		/**
		 * Request Rate with range from 1 millisecond conflation to maximum conflation.
		 */
		public final static int BEST_CONFLATED_RATE = 0xFFFFFFFF;

		/**
		 * Request Rate with range from tick-by-tick to maximum conflation.
		 */
		public final static int BEST_RATE = 0xFFFFFFFE;
	}

	/**
	 * Timeliness represents the Qos Timeliness.
	 */
	public class Timeliness
	{
		/**
		 * Timeliness is REALTIME,
		 * indicates information is updated as soon as new information becomes available
		 */
		public final static int REALTIME = 0;

		/**
		 * Request Timeliness with range from one second delay to maximum delay.
		 */
		public final static int BEST_DELAYED_TIMELINESS = 0xFFFFFFFF;

		/**
		 * Request Timeliness with range from real-time to maximum delay.
		 */
		public final static int BEST_TIMELINESS = 0xFFFFFFFE;
	}

	/**
	 * Returns the Rate value as a string format.
	 * 
	 * @return string representation of Qos Rate
	 */
	public String rateAsString();

	/**
	 * Returns the Timeliness value as a string format.
	 * 
	 * @return string representation of Qos Timeliness
	 */
	public String timelinessAsString();

	/**
	 * Indicates presence of Priority.<br>
	 * Priority is an optional member of ReqMsg
	 * 
	 * @return true if priority is set; false otherwise
	 */
	public boolean hasPriority();

	/**
	 * Indicates presence of Qos.<br>
	 * Qos is an optional member of ReqMsg
	 * 
	 * @return true if Qos is set; false otherwise
	 */
	public boolean hasQos();

	/**
	 * Indicates presence of View.<br>
	 * View specification is an optional member of ReqMsg
	 * 
	 * @return true if View is set; false otherwise
	 */
	public boolean hasView();

	/**
	 * Indicates presence of Batch.<br>
	 * Batch specification is an optional member of ReqMsg
	 * 
	 * @return true if Batch specification is set; false otherwise
	 */
	public boolean hasBatch();

	/**
	 * Indicates presence of the ServiceName within the MsgKey.<br>
	 * ServiceName is an optional member of ReqMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();

	/**
	 * Returns PriorityClass.<br>
	 * Calling this method must be preceded by a call to {@link #hasPriority()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPriority()} returns false
	 * 
	 * @return priority class
	 */
	public int priorityClass();

	/**
	 * Returns PriorityCount.<br>
	 * Calling this method must be preceded by a call to {@link #hasPriority()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPriority()} returns false
	 * 
	 * @return priority count
	 */
	public int priorityCount();

	/**
	 * Returns QosTimeliness.<br>
	 * Calling this method must be preceded by a call to {@link #hasQos()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasQos()} returns false
	 * 
	 * @return Qos Timeliness
	 */
	public int qosTimeliness();

	/**
	 * Returns QosRate.<br>
	 * Calling this method must be preceded by a call to {@link #hasQos()}.
	 * 
	 * @throws OmmInvalidUsageException if if {@link #hasQos()} returns false
	 * 
	 * @return Qos Rate
	 */
	public int qosRate();

	/**
	 * Returns InitialImage.
	 * 
	 * @return true if an initial image is requested; false otherwise
	 */
	public boolean initialImage();

	/**
	 * Returns InterestAfterRefresh.
	 * 
	 * @return true if an interest after refresh is requested; false otherwise
	 */
	public boolean interestAfterRefresh();

	/**
	 * Returns ConflatedInUpdates.
	 * 
	 * @return true if conflation is requested; false otherwise
	 */
	public boolean conflatedInUpdates();

	/**
	 * Returns Pause.
	 * 
	 * @return true if pause is requested; false otherwise
	 */
	public boolean pause();

	/**
	 * Returns PrivateStream.
	 * 
	 * @return true if private stream is requested; false otherwise
	 */
	public boolean privateStream();

	/**
	 * Returns the ServiceName within the MsgKey.<br>
	 * Calling this method must be preceded by a call to {@link #hasServiceName()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasServiceName()} returns false
	 * 
	 * @return service name
	 */
	public String serviceName();

	/**
	 * Clears the ReqMsg.<br>
	 * Invoking clear() method clears all the values and resets all the defaults
	 * 
	 * @return reference to this object
	 */
	public ReqMsg clear();

	/**
	 * Specifies StreamId.
	 * 
	 * @throws OmmOutOfRangeException if streamId is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public ReqMsg streamId(int streamId);

	/**
	 * Specifies DomainType.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is is {@literal < 0 or > 255}
	 * 
	 * @param domainType specifies RDM Message Model Type
	 *        (e.g. {@link com.rtsdk.ema.rdm.EmaRdm#MMT_MARKET_PRICE})
	 * @return reference to this object
	 */
	public ReqMsg domainType(int domainType);

	/**
	 * Specifies Name.
	 * 
	 * @throws OmmInvalidUsageException if name is null
	 * 
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public ReqMsg name(String name);

	/**
	 * Specifies NameType.
	 * 
	 * @throws OmmOutOfRangeException if nameType is {@literal < 0 or > 255}
	 * 
	 * @param nameType specifies RDM Instrument NameType
	 *        (e.g. {@link com.rtsdk.ema.rdm.EmaRdm#INSTRUMENT_NAME_RIC})
	 * @return reference to this object
	 */
	public ReqMsg nameType(int nameType);

	/**
	 * Specifies ServiceName.<br>
	 * One service identification must be set, either id or name.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set or if name is null
	 *  
	 * @param serviceName specifies service name
	 * @return reference to this object
	 */
	public ReqMsg serviceName(String serviceName);

	/**
	 * Specifies ServiceId.<br>
	 * One service identification must be set, either id or name.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 *                               or if serviceId is {@literal < 0 or > 65535}
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public ReqMsg serviceId(int serviceId);

	/**
	 * Specifies Id.
	 * 
	 * @throws OmmOutOfRangeException if id is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public ReqMsg id(int id);

	/**
	 * Specifies Filter.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 4294967295L}
	 * 
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public ReqMsg filter(long filter);

	/**
	 * Specifies Priority.
	 * 
	 * @throws OmmOutOfRangeException if priorityClass is {@literal < 0 or > 255}
	 * @throws OmmOutOfRangeException if priorityCount is {@literal < 0 or > 65535}
	 * 
	 * @param priorityClass specifies priority class
	 * @param priorityCount specifies priority count within priority class
	 * 
	 * @return reference to this object
	 */
	public ReqMsg priority(int priorityClass, int priorityCount);

	/**
	 * Specifies Qos as timeliness and rate.
	 * 
	 * @param timeliness specifies Qos Timeliness (e.g. {@link Timeliness#BEST_TIMELINESS})
	 * @param rate specifies Qos rate (e.g. {@link Rate#BEST_RATE})
	 * 
	 * @return reference to this object
	 */
	public ReqMsg qos(int timeliness, int rate);

	/**
	 * Specifies Attrib.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public ReqMsg attrib(ComplexType data);

	/**
	 * Specifies Payload.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public ReqMsg payload(ComplexType data);

	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @throws OmmInvalidUsageException if buffer is null
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public ReqMsg extendedHeader(ByteBuffer buffer);

	/**
	 * Specifies InitialImage.
	 * 
	 * @param initialImage specifies if initial image / refresh is requested
	 * @return reference to this object
	 */
	public ReqMsg initialImage(boolean initialImage);

	/**
	 * Specifies InterestAfterRefresh.
	 * 
	 * @param interestAfterRefresh specifies if streaming or snapshot item is requested
	 * @return reference to this object
	 */
	public ReqMsg interestAfterRefresh(boolean interestAfterRefresh);

	/**
	 * Specifies Pause.
	 * 
	 * @param pause specifies if pause is requested
	 * @return reference to this object
	 */
	public ReqMsg pause(boolean pause);

	/**
	 * Specifies ConflatedInUpdates.
	 * 
	 * @param conflatedInUpdates specifies if conflated update is requested
	 * @return reference to this object
	 */
	public ReqMsg conflatedInUpdates(boolean conflatedInUpdates);

	/**
	 * Specifies PrivateStream.
	 * 
	 * @param privateStream specifies if private stream is requested
	 * @return reference to this object
	 */
	public ReqMsg privateStream(boolean privateStream);
}