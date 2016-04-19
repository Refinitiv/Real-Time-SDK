///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * RefreshMsg conveys item image, state, permission and group information.
 * 
 * <p>RefreshMsg is sent when item data needs to be synchronized.
 * This happens as a response to received ReqMsg or when upstream source requires it.</p>
 * <p>RefreshMsg sent as a response to ReqMsg is called a solicited refresh,
 * while an unsolicited refresh is sent when upstream source requires
 * synchronization of downstream consumers.</p>
 * 
 * @see Data
 * @see Msg
 */
public interface RefreshMsg extends Msg
{
	/**
	 * Indicates presence of Qos.
	 * Qos is an optional member of RefreshMsg.
	 * 
	 * @return true if Qos is set; false otherwise
	 */
	public boolean hasQos();

	/**
	 * Indicates presence of SeqNum.
	 * Sequence number (SeqNum) is an optional member of RefreshMsg.
	 * 
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();

	/**
	 * Indicates presence of PartNum.
	 * Part number is an optional member of RefreshMsg.
	 * 
	 * @return true if part number is set; false otherwise
	 */
	public boolean hasPartNum();

	/**
	 * Indicates presence of PermissionData.
	 * Permission data is optional member of RefreshMsg.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();

	/**
	 * Indicates presence of PublisherId.
	 * Publisher id is an optional member of RefreshMsg.
	 * 
	 * @return true if publisher id is set; false otherwise
	 */
	public boolean hasPublisherId();

	/**
	 * Indicates presence of the ServiceName within the MsgKey.
	 * Service name is an optional member of RefreshMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();

	/**
	 * Returns State ({@link com.thomsonreuters.ema.access.OmmState}).
	 * 
	 * @return state of item (e.g. Open / Ok)
	 */
	public OmmState state();

	/**
	 * Returns Qos ({@link com.thomsonreuters.ema.access.OmmQos}).
	 * <br>Calling this method must be preceded by a call to {@link #hasQos()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasQos()} returns false
	 * 
	 * @return Qos of item
	 */
	public OmmQos qos();

	/**
	 * Returns SeqNum.
	 * <br>Calling this method must be preceded by a call to {@link #hasSeqNum()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasSeqNum()} returns false
	 * 
	 * @return sequence number
	 */
	public long seqNum();

	/**
	 * Returns PartNum.
	 * <br>Calling this method must be preceded by a call to {@link #hasPartNum()}
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
	 * Returns PermissionData.
 	 * <br>Calling this method must be preceded by a call to {@link #hasPermissionData()}
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
	 * Returns PublisherIdUserAddress.
  	 * <br>Calling this method must be preceded by a call to {@link #hasPublisherId()}
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
	 * Returns the ServiceName within the MsgKey.
	 * <br>Calling this method must be preceded by a call to {@link #hasServiceName()}
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
	 * @param streamId stream id
	 * @return reference to this object
	 */
	public RefreshMsg streamId(int streamId);

	/**
	 * Specifies Domain Type.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is greater than 255
	 * 
	 * @param domainType specifies RDM Message Model Type
	 *        (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#MMT_MARKET_PRICE})
	 * @return reference to this object
	 */
	public RefreshMsg domainType(int domainType);

	/**
	 * Specifies Name.
	 * 
	 * @param name an String object containing item name
	 * @return reference to this object
	 */
	public RefreshMsg name(String name);

	/**
	 * Specifies NameType.
	 * 
	 * @param nameType specifies RDM Instrument NameType 
	 *        (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#INSTRUMENT_NAME_RIC})
	 * @return reference to this object
	 */
	public RefreshMsg nameType(int nameType);

	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set
	 * 
	 * @param serviceName an String object containing service name
	 * @return reference to this object
	 */
	public RefreshMsg serviceName(String serviceName);

	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 * 
	 * @param serviceId service identifier
	 * @return reference to this object
	 */
	public RefreshMsg serviceId(int serviceId);

	/**
	 * Specifies Id.
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public RefreshMsg id(int id);

	/**
	 * Specifies Filter.
	 * 
	 * @param filter specifies Filter
	 * @return reference to this object
	 */
	public RefreshMsg filter(long filter);

	/**
	 * Specifies Qos.
	 * 
	 * @param timeliness specifies Qos Timeliness
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Timeliness#REALTIME})
	 * @param rate specifies Qos rate
	 *        (default value is	{@link com.thomsonreuters.ema.access.OmmQos.Rate#TICK_BY_TICK})
	 * @return reference to this object
	 */
	public RefreshMsg qos(int timeliness, int rate);

	/**
	 * Specifies State.
	 * 
	 * @param streamState conveys item stream state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState conveys item data state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#SUSPECT})
	 * @return reference to this object
	 */
	public RefreshMsg state(int streamState, int dataState);
	
	/**
	 * Specifies State.
	 * 
	 * @param streamState conveys item stream state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState conveys item data state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#SUSPECT})
	 * @param statusCode conveys specific item state code
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @return reference to this object
	 */
	public RefreshMsg state(int streamState, int dataState, int statusCode);
	
	/**
	 * Specifies State.
	 * 
	 * @param streamState conveys item stream state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState conveys item data state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#SUSPECT})
	 * @param statusCode conveys specific item state code
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @param statusText conveys item status explanation (default value is 'empty string')
	 * @return reference to this object
	 */
	public RefreshMsg state(int streamState, int dataState, int statusCode,	String statusText);

	/**
	 * Specifies SeqNum.
	 * 
	 * @param seqNum specifies sequence number
	 * @return reference to this object
	 */
	public RefreshMsg seqNum(long seqNum);

	/**
	 * Specifies PartNum.
	 * 
	 * @param partNum specifies part number
	 * @return reference to this object
	 */
	public RefreshMsg partNum(int partNum);

	/**
	 * Specifies ItemGroup.
	 * 
	 * @param itemGroup a ByteBuffer object with item group information
	 * @return reference to this object
	 */
	public RefreshMsg itemGroup(ByteBuffer itemGroup);

	/**
	 * Specifies PermissionData.
	 * 
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public RefreshMsg permissionData(ByteBuffer permissionData);

	/**
	 * Specifies PublisherId.
	 * 
	 * @param userId specifies publisher's user id
	 * @param userAddress specifies publisher's user address
	 * @return reference to this object
	 */
	public RefreshMsg publisherId(long userId, long userAddress);

	/**
	 * Specifies Attrib.
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public RefreshMsg attrib(ComplexType data);

	/**
	 * Specifies Payload.
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public RefreshMsg payload(ComplexType data);

	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public RefreshMsg extendedHeader(ByteBuffer buffer);

	/**
	 * Specifies Solicited.
	 * 
	 * @param solicited true if this refresh is solicited; false otherwise
	 *            (default value is true)
	 * @return reference to this object
	 */
	public RefreshMsg solicited(boolean solicited);

	/**
	 * Specifies DoNotCache.
	 * 
	 * @param doNotCache true if this refresh must not be cached; false otherwise
	 *        (default value is false)
	 * @return reference to this object
	 */
	public RefreshMsg doNotCache(boolean doNotCache);

	/**
	 * Specifies ClearCache.
	 * 
	 * @param clearCache true if cache needs to be cleared; false otherwise
	 *            (default value is false)
	 * @return reference to this object
	 */
	public RefreshMsg clearCache(boolean clearCache);

	/**
	 * Specifies RefreshComplete.
	 * 
	 * @param complete true if this is the last part of multi part refresh
	 *            or single part refresh; false otherwise (default value is true)
	 * @return reference to this object
	 */
	public RefreshMsg complete(boolean complete);

	/**
	 * Specifies PrivateStream.
	 * 
	 * @param privateStream true if private stream; false otherwise
	 *        (default value is false)
	 * @return reference to this object
	 */
	public RefreshMsg privateStream(boolean privateStream);
}