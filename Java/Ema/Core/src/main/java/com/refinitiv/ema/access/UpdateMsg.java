///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import com.refinitiv.ema.rdm.DataDictionary;

/**
 * UpdateMsg conveys changes to item data.<br>
 * 
 * <p>
 * Calling an accessor method on an optional member of UpdateMsg must be<br>
 * preceded by a call to respective has***() method.<br>
 * example: calling {@link #serviceName()} must be preceded by a call to {@link #hasServiceName()}</p>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and getting of information from RefreshMsg.<br>
 * Objects of this class are not cache-able.<br>
 * Decoding of just encoded UpdateMsg in the same application is not supported.
 *
 */
public interface UpdateMsg extends Msg
{
	/**
	 * Indicates presence of SeqNum.<br>
	 * Sequence number is an optional member of UpdateMsg.
	 * 
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();
	
	/**
	 * Indicates presence of PermissionData.<br>
	 * Permission data is an optional member of UpdateMsg.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Indicates presence of Conflated.
	 * 
	 * @return true if update contains conflated data; false otherwise
	 */
	public boolean hasConflated();
	
	/**
	 * Indicates presence of PublisherId.<br>
	 * Publisher id is an optional member of UpdateMsg.
	 * 
	 * @return true if publisher id is set; false otherwise
	 */
	public boolean hasPublisherId();
	
	/**
	 * Indicates presence of the ServiceName within the MsgKey.<br>
	 * Service name is an optional member of UpdateMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();
	
	/** Returns UpdateTypeNum.
	 * 
	 * @return update type number (e.g. {@link com.refinitiv.ema.rdm.EmaRdm#INSTRUMENT_UPDATE_QUOTE})
	 */
	public int updateTypeNum();
	
	/**
	 * Returns SeqNum.<br>
	 * Calling this method must be preceded by a call to {@link #hasSeqNum()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasSeqNum()} returns false
	 * 
	 * @return sequence number 
	 */
	public long seqNum();
	
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
	 * Returns ConflatedTime.<br>
	 * Calling this method must be preceded by a call to {@link #hasConflated()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasConflated()} returns false
	 * 
	 * @return time conflation was on
	 */
	public int conflatedTime();
	
	/**
	 * Returns ConflatedCount.<br>
	 * Calling this method must be preceded by a call to {@link #hasConflated()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasConflated()} returns false
	 * 
	 * @return number of conflated updates
	 */
	public int conflatedCount();
	
	/**
	 * Returns PublisherIdUserId.<br>
	 * Calling this method must be preceded by a call to {@link #hasPublisherId()}
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
	 * 
	 * @return true if this update must not be cached; false otherwise
	 */
	public boolean doNotCache();
	
	/** Returns DoNotConflate.
	 * 
	 *  @return true if this update must not be conflated; false otherwise
	 */
	public boolean doNotConflate();
	
	/**
	 * Returns DoNotRipple.
	 * 
	 * @return true if this update does not ripple; false otherwise
	 */
	public boolean doNotRipple();
	
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
	 * Clears the UpdateMsg.<br>
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public UpdateMsg clear();
	
	/**
	 * Specifies StreamId.
	 * 
	 * @throws OmmOutOfRangeException if streamId is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public UpdateMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is is {@literal < 0 or > 255}
	 * 
	 * @param domainType specifies RDM Message Model Type
	 * @return reference to this object
	 */
	public UpdateMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * 
	 * @throws OmmInvalidUsageException if name is null
	 * 
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public UpdateMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * 
	 * @throws OmmOutOfRangeException if nameType is {@literal < 0 or > 255}
	 * 
	 * @param nameType specifies RDM Instrument NameType
	 * @return reference to this object
	 */
	public UpdateMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set or if name is null
	 * 
	 * @param serviceName specifies service name
	 * @return reference to this object
	 */
	public UpdateMsg serviceName(String serviceName);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 *                               or if serviceId is {@literal < 0 or > 65535}
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public UpdateMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * 
	 * @throws OmmOutOfRangeException if id is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public UpdateMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 4294967295L}
	 * 
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public UpdateMsg filter(long filter);
	
	/**
	 * Specifies UpdateTypeNum.
	 *
	 * @throws OmmOutOfRangeException if updateTypeNum is {@literal < 0 or > 255}
	 * 
	 * @param updateTypeNum specifies update type number
	 *  (e.g. {@link com.refinitiv.ema.rdm.EmaRdm#INSTRUMENT_UPDATE_QUOTE})
	 * 
	 * @return reference to this object
	 */
	public UpdateMsg updateTypeNum(int updateTypeNum);
	
	/**
	 * Specifies SeqNum.
	 * 
	 * @throws OmmOutOfRangeException if seqNum is {@literal < 0 or > 4294967295L}
	 * 
	 * @param seqNum specifies sequence number
	 * @return reference to this object
	 */
	public UpdateMsg seqNum(long seqNum);
	
	/**
	 * Specifies PermissionData.
	 * 
	 * @throws OmmInvalidUsageException if permissionData is null
	 * 
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public UpdateMsg permissionData(ByteBuffer permissionData);
	
	/**
	 * Specifies Conflated.
	 * 
	 * @param count specifies how many updates were conflated
	 * @param time specifies how long the conflation was on
	 * 
	 * @return reference to this object
	 */
	public UpdateMsg conflated(int count, int time);
	
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
	public UpdateMsg publisherId(long userId, long userAddress);
	
	/**
	 * Specifies Attrib.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public UpdateMsg attrib(ComplexType data);
	
	/**
	 * Specifies Payload.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public UpdateMsg payload(ComplexType data);
	
	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @throws OmmInvalidUsageException if buffer is null
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public UpdateMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies DoNotCache.
	 * 
	 * @param doNotCache true if this update must not be cached; false otherwise
	 * @return reference to this object
	 */
	public UpdateMsg doNotCache(boolean doNotCache);
	
	/**
	 * Specifies DoNotConflate.
	 * 
	 * @param doNotConflate true if this update must not be conflated; false otherwise
	 * @return reference to this object
	 */
	public UpdateMsg doNotConflate(boolean doNotConflate);
	
	/**
	 * Specifies DoNotRipple.
	 * 
	 * @param doNotRipple true if this update does not ripple; false otherwise
	 * @return reference to this object
	 */
	public UpdateMsg doNotRipple(boolean doNotRipple);

	/**
	 *  Returns a string representation of the class instance.
	 * @param dictionary use for toString() conversion
	 * @return string representation of the class instance
	 */
	public String toString(DataDictionary dictionary);
}