///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * UpdateMsg conveys changes to item data.
 */
public interface UpdateMsg extends Msg
{
	/**
	 * Indicates presence of SeqNum.
	 * Sequence number is an optional member of UpdateMsg.
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();
	
	/**
	 * Indicates presence of PermissionData.
	 * Permission data is an optional member of UpdateMsg.
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Indicates presence of Conflated.
	 * @return true if update contains conflated data; false otherwise
	 */
	public boolean hasConflated();
	
	/**
	 * Indicates presence of PublisherId.
	 * Publisher id is an optional member of UpdateMsg.
	 * @return true if publisher id is set; false otherwise
	 */
	public boolean hasPublisherId();
	
	/**
	 * Indicates presence of the ServiceName within the MsgKey.
	 * Service name is an optional member of UpdateMsg.
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();
	
	/** Returns UpdateTypeNum.
	 * @return update type number (e.g. {@link com.thomsonreuters.ema.rdm.EmaRdm#INSTRUMENT_UPDATE_QUOTE})
	 */
	public int updateTypeNum();
	
	/**
	 * Returns SeqNum.
	  <br>Calling this method must be preceded by a call to {@link #hasSeqNum()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasSeqNum()} returns false
	 * 
	 * @return sequence number 
	 */
	public long seqNum();
	
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
	 * Returns ConflatedTime.
	 * <br>Calling this method must be preceded by a call to {@link #hasConflated()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasConflated()} returns false
	 * 
	 * @return time conflation was on
	 */
	public int conflatedTime();
	
	/**
	 * Returns ConflatedCount.
	 * <br>Calling this method must be preceded by a call to {@link #hasConflated()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasConflated()} returns false
	 * 
	 * @return number of conflated updates
	 */
	public int conflatedCount();
	
	/**
	 * Returns PublisherIdUserId.
	 * <br>Calling this method must be preceded by a call to {@link #hasPublisherId()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPublisherId()} returns false
	 * 
	 * @return publisher's user Id
	 */
	public long publisherIdUserId();
	
	/**
	 * Returns PublisherIdUserAddress.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPublisherId()} returns false
	 * 
	 * @return publisher's user address
	 */
	public long publisherIdUserAddress();
	
	/**
	 * Returns DoNotCache.
	 * @return true if this update must not be cached; false otherwise
	 */
	public boolean doNotCache();
	
	/** Returns DoNotConflate.
	 *  @return true if this update must not be conflated; false otherwise
	 */
	public boolean doNotConflate();
	
	/**
	 * Returns DoNotRipple.
	 * @return true if this update does not ripple; false otherwise
	 */
	public boolean doNotRipple();
	
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
	 * Clears the UpdateMsg.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * @return reference to this object
	 */
	public UpdateMsg clear();
	
	/**
	 * Specifies StreamId.
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public UpdateMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is greater than 255
	 * 
	 * @param domainType specifies RDM Message Model Type
	 *        (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#MMT_MARKET_PRICE})
	 * @return reference to this object
	 */
	public UpdateMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public UpdateMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * @param nameType specifies RDM Instrument NameType
	 *        (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#INSTRUMENT_NAME_RIC})
	 * @return reference to this object
	 */
	public UpdateMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set
	 * 
	 * @param serviceName specifies service name
	 * @return reference to this object
	 */
	public UpdateMsg serviceName(String serviceName);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public UpdateMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public UpdateMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public UpdateMsg filter(long filter);
	
	/**
	 * Specifies UpdateTypeNum.
	 * @param updateTypeNum specifies update type number (e.g. {@link com.thomsonreuters.ema.rdm.EmaRdm#INSTRUMENT_UPDATE_QUOTE})
	 * 
	 * @return reference to this object
	 */
	public UpdateMsg updateTypeNum(int updateTypeNum);
	
	/**
	 * Specifies SeqNum.
	 * @param seqNum specifies sequence number
	 * @return reference to this object
	 */
	public UpdateMsg seqNum(long seqNum);
	
	/**
	 * Specifies PermissionData.
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public UpdateMsg permissionData(ByteBuffer permissionData);
	
	/**
	 * Specifies Conflated.
	 * @param count specifies how many updates were conflated
	 * @param time specifies how long the conflation was on
	 * @return reference to this object
	 */
	public UpdateMsg conflated(int count, int time);
	
	/**
	 * Specifies PublisherId.
	 * @param userId specifies publisher's user id
	 * @param userAddress specifies publisher's user address
	 * @return reference to this object
	 */
	public UpdateMsg publisherId(long userId, long userAddress);
	
	/**
	 * Specifies Attrib.
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public UpdateMsg attrib(ComplexType data);
	
	/**
	 * Specifies Payload.
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public UpdateMsg payload(ComplexType data);
	
	/**
	 * Specifies ExtendedHeader.
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public UpdateMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies DoNotCache.
	 * @param doNotCache true if this update must not be cached; false otherwise (default value is false)
	 * @return reference to this object
	 */
	public UpdateMsg doNotCache(boolean doNotCache);
	
	/**
	 * Specifies DoNotConflate.
	 * @param doNotConflate true if this update must not be conflated; false otherwise (default value is false)
	 * @return reference to this object
	 */
	public UpdateMsg doNotConflate(boolean doNotConflate);
	
	/**
	 * Specifies DoNotRipple.
	 * @param doNotRipple true if this update does not ripple; false otherwise (default value is false)
	 * @return reference to this object
	 */
	public UpdateMsg doNotRipple(boolean doNotRipple);
}