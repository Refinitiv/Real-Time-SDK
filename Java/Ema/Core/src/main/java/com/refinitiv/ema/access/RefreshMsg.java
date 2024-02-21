///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import com.refinitiv.ema.rdm.DataDictionary;

/**
 * RefreshMsg conveys item image, state, permission and group information.
 * 
 * <p>RefreshMsg is sent when item data needs to be synchronized.<br>
 * This happens as a response to received ReqMsg or when upstream source requires it.</p>
 * 
 * <p>RefreshMsg sent as a response to ReqMsg is called a solicited refresh,<br>
 * while an unsolicited refresh is sent when upstream source requires
 * synchronization of downstream consumers.</p>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and getting of information from RefreshMsg.<br>
 * Objects of this class are not cache-able.<br>
 * Decoding of just encoded RefreshMsg in the same application is not supported.
 *
 * @see Data
 * @see Msg
 */
public interface RefreshMsg extends Msg
{
	/**
	 * Indicates presence of Qos.<br>
	 * Qos is an optional member of RefreshMsg.
	 * 
	 * @return true if Qos is set; false otherwise
	 */
	public boolean hasQos();

	/**
	 * Indicates presence of SeqNum.<br>
	 * Sequence number (SeqNum) is an optional member of RefreshMsg.
	 * 
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();

	/**
	 * Indicates presence of PartNum.<br>
	 * Part number is an optional member of RefreshMsg.
	 * 
	 * @return true if part number is set; false otherwise
	 */
	public boolean hasPartNum();

	/**
	 * Indicates presence of PermissionData.<br>
	 * Permission data is optional member of RefreshMsg.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();

	/**
	 * Indicates presence of PublisherId.<br>
	 * Publisher id is an optional member of RefreshMsg.
	 * 
	 * @return true if publisher id is set; false otherwise
	 */
	public boolean hasPublisherId();

	/**
	 * Indicates presence of the ServiceName within the MsgKey.<br>
	 * Service name is an optional member of RefreshMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();

	/**
	 * Returns State ({@link com.refinitiv.ema.access.OmmState}).
	 * 
	 * @return state of item (e.g. Open / Ok)
	 */
	public OmmState state();

	/**
	 * Returns Qos ({@link com.refinitiv.ema.access.OmmQos}).<br>
	 * Calling this method must be preceded by a call to {@link #hasQos()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasQos()} returns false
	 * 
	 * @return Qos of item
	 */
	public OmmQos qos();

	/**
	 * Returns SeqNum.<br>
	 * Calling this method must be preceded by a call to {@link #hasSeqNum()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasSeqNum()} returns false
	 * 
	 * @return sequence number
	 */
	public long seqNum();

	/**
	 * Returns PartNum.<br>
	 * Calling this method must be preceded by a call to {@link #hasPartNum()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPartNum()} returns false
	 * 
	 * @return part number
	 */
	public int partNum();

	/**
	 * Returns ItemGroup.
	 * 
	 * @return ByteBuffer containing item group information
	 */
	public ByteBuffer itemGroup();

	/**
	 * Returns PermissionData.<br>
 	 * Calling this method must be preceded by a call to {@link #hasPermissionData()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPermissionData()} returns false
	 * 
	 * @return ByteBuffer containing permission data
	 */
	public ByteBuffer permissionData();

	/**
	 * Returns PublisherIdUserId.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPublisherId()} returns false
	 * 
	 * @return publisher's user Id
	 */
	public long publisherIdUserId();

	/**
	 * Returns PublisherIdUserAddress.<br>
  	 * Calling this method must be preceded by a call to {@link #hasPublisherId()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPublisherId()} returns false
	 * 
	 * @return publisher's user address
	 */
	public long publisherIdUserAddress();

	/**
	 * Returns Solicited.
	 * 
	 * @return true if this is solicited refresh; false otherwise
	 */
	public boolean solicited();

	/**
	 * Returns DoNotCache.
	 * 
	 * @return true if this refresh must not be cached; false otherwise
	 */
	public boolean doNotCache();

	/**
	 * Returns Complete.
	 * 
	 * @return true if this is the last part of the multi part refresh message
	 */
	public boolean complete();

	/**
	 * Returns ClearCache.
	 * 
	 * @return true if cache needs to be cleared before applying this refresh;
	 *         false otherwise
	 */
	public boolean clearCache();

	/**
	 * Returns PrivateStream.
	 * 
	 * @return true if this is private stream item
	 */
	public boolean privateStream();

	/**
	 * Returns the ServiceName within the MsgKey.<br>
	 * Calling this method must be preceded by a call to {@link #hasServiceName()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasServiceName()} returns false
	 * 
	 * @return String containing service name
	 */
	public String serviceName();

	/**
	 * Clears the RefreshMsg.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public RefreshMsg clear();

	/**
	 * Specifies StreamId.
	 * 
	 * @throws OmmOutOfRangeException if streamId is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param streamId stream id
	 * @return reference to this object
	 */
	public RefreshMsg streamId(int streamId);

	/**
	 * Specifies Domain Type.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is is {@literal < 0 or > 255}
	 * 
	 * @param domainType specifies RDM Message Model Type
	 * @return reference to this object
	 */
	public RefreshMsg domainType(int domainType);

	/**
	 * Specifies Name.
	 * 
	 * @throws OmmInvalidUsageException if name is null
	 * 
	 * @param name an String object containing item name
	 * @return reference to this object
	 */
	public RefreshMsg name(String name);

	/**
	 * Specifies NameType.
	 * 
	 * @throws OmmOutOfRangeException if nameType is {@literal < 0 or > 255}
	 * 
	 * @param nameType specifies RDM Instrument NameType 
	 * @return reference to this object
	 */
	public RefreshMsg nameType(int nameType);

	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set or if name is null
	 * 
	 * @param serviceName an String object containing service name
	 * @return reference to this object
	 */
	public RefreshMsg serviceName(String serviceName);

	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 *                               or if serviceId is {@literal < 0 or > 65535}
	 * 
	 * @param serviceId service identifier
	 * @return reference to this object
	 */
	public RefreshMsg serviceId(int serviceId);

	/**
	 * Specifies Id.
	 * 
	 * @throws OmmOutOfRangeException if id is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public RefreshMsg id(int id);

	/**
	 * Specifies Filter.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 4294967295L}
	 * 
	 * @param filter specifies Filter
	 * @return reference to this object
	 */
	public RefreshMsg filter(long filter);

	/**
	 * Specifies Qos.
	 * 
	 * @param timeliness specifies Qos Timeliness
	 * @param rate specifies Qos rate
	 * 
	 * @return reference to this object
	 */
	public RefreshMsg qos(int timeliness, int rate);

	/**
	 * Specifies State.<br>
	 * Defaults: statusCode=OmmState.StatusCode.NONE, statusText=DataImpl.EMPTY_STRING
	 * 
	 * @param streamState conveys item stream state value
	 * @param dataState conveys item data state value
	 * 
	 * @return reference to this object
	 */
	public RefreshMsg state(int streamState, int dataState);
	
	/**
	 * Specifies State.<br>
	 * Default: statusText=DataImpl.EMPTY_STRING
	 * 
	 * @param streamState conveys item stream state value
	 * @param dataState conveys item data state value
	 * @param statusCode conveys specific item state code
	 * 
	 * @return reference to this object
	 */
	public RefreshMsg state(int streamState, int dataState, int statusCode);
	
	/**
	 * Specifies State.
	 * 
	 * @param streamState conveys item stream state value
	 * @param dataState conveys item data state value
	 * @param statusCode conveys specific item state code
	 * @param statusText conveys item status explanation
	 * 
	 * @return reference to this object
	 */
	public RefreshMsg state(int streamState, int dataState, int statusCode,	String statusText);

	/**
	 * Specifies SeqNum.
	 * 
	 * @throws OmmOutOfRangeException if seqNum is {@literal < 0 or > 4294967295L}
	 * 
	 * @param seqNum specifies sequence number
	 * @return reference to this object
	 */
	public RefreshMsg seqNum(long seqNum);

	/**
	 * Specifies PartNum.
	 * 
	 * @throws OmmOutOfRangeException if partNum is {@literal < 0 or > 32767}
	 * 
	 * @param partNum specifies part number
	 * @return reference to this object
	 */
	public RefreshMsg partNum(int partNum);

	/**
	 * Specifies ItemGroup.
	 *
	 * @throws OmmInvalidUsageException if itemGroup is null
	 * 
	 * @param itemGroup a ByteBuffer object with item group information
	 * @return reference to this object
	 */
	public RefreshMsg itemGroup(ByteBuffer itemGroup);

	/**
	 * Specifies PermissionData.
	 *
	 * @throws OmmInvalidUsageException if permissionData is null
	 * 
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public RefreshMsg permissionData(ByteBuffer permissionData);

	/**
	 * Specifies PublisherId.
	 * 
	 * @throws OmmOutOfRangeException if userId is {@literal < 0 or > 4294967295L}
	 *                             or if userAddress is {@literal < 0 or > 4294967295L}
	 *                             
	 * @param userId specifies publisher's user id
	 * @param userAddress specifies publisher's user address
	 * 
	 * @return reference to this object
	 */
	public RefreshMsg publisherId(long userId, long userAddress);

	/**
	 * Specifies Attrib.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public RefreshMsg attrib(ComplexType data);

	/**
	 * Specifies Payload.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public RefreshMsg payload(ComplexType data);

	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @throws OmmInvalidUsageException if buffer is null
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public RefreshMsg extendedHeader(ByteBuffer buffer);

	/**
	 * Specifies Solicited.
	 * 
	 * @param solicited true if this refresh is solicited; false otherwise
	 * @return reference to this object
	 */
	public RefreshMsg solicited(boolean solicited);

	/**
	 * Specifies DoNotCache.
	 * 
	 * @param doNotCache true if this refresh must not be cached; false otherwise
	 * @return reference to this object
	 */
	public RefreshMsg doNotCache(boolean doNotCache);

	/**
	 * Specifies ClearCache.
	 * 
	 * @param clearCache true if cache needs to be cleared; false otherwise
	 * @return reference to this object
	 */
	public RefreshMsg clearCache(boolean clearCache);

	/**
	 * Specifies RefreshComplete.
	 * 
	 * @param complete true if this is the last part of multi part refresh
	 *            or single part refresh; false otherwise
	 * @return reference to this object
	 */
	public RefreshMsg complete(boolean complete);

	/**
	 * Specifies PrivateStream.
	 * 
	 * @param privateStream true if private stream; false otherwise
	 * @return reference to this object
	 */
	public RefreshMsg privateStream(boolean privateStream);

	/**
	 *  Returns a string representation of the class instance.
	 * @param dictionary use for toString() conversion
	 * @return string representation of the class instance
	 */
	public String toString(DataDictionary dictionary);
}