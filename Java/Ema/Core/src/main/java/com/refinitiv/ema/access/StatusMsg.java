///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024-2025 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import com.refinitiv.ema.rdm.DataDictionary;

/**
 * StatusMsg conveys item state information.
 * <p>StatusMsg is used to convey item state information, permission change or item group id change.</p>
 * 
 * <p>
 * Calling an accessor method on an optional member of StatusMsg must be<br>
 * preceded by a call to respective has***() method.<br>
 * example: calling {@link #serviceName()} must be preceded by a call to {@link #hasServiceName()}</p>
 *
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and getting of information from RefreshMsg.<br>
 * Objects of this class are not cache-able.<br>
 * Decoding of just encoded StatusMsg in the same application is not supported.
 *
 * @see Data
 * @see Msg
 */
public interface StatusMsg extends Msg
{
	/**
	 * Indicates presence of ItemGroup.<br>
	 * Item Group is an optional member of StatusMsg.
	 * 
	 * @return true if Item Group information is set; false otherwise
	 */
	public boolean hasItemGroup();
	
	/**
	 * Indicates presence of State.<br>
	 * Item State is an optional member of StatusMsg.
	 * 
	 * @return true if state information is set; false otherwise
	*/
	public boolean hasState();
	
	/**
	 * Indicates presence of PermissionData.<br>
	 * Permission data is an optional member of StatusMsg.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Indicates presence of PublisherId.<br>
	 * Publisher id is an optional member of StatusMsg.
	 * 
	 * @return true if publisher id is set; false otherwise
	 */
	public boolean hasPublisherId();
	
	/**
	 * Indicates presence of the ServiceName within the MsgKey.<br>
	 * Service name is an optional member of StatusMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();
	
	/**
	 * Returns State.<br>
	 * Calling this method must be preceded by a call to {@link #hasState()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasState()} returns false
	 * 
	 * @return state of item (e.g. Open / Ok)
	 */
	public OmmState state();
	
	/**
	 * Returns ItemGroup.<br>
	 * Calling this method must be preceded by a call to {@link #hasItemGroup()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasItemGroup()} returns false
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
	 * @return ByteBuffer containing permission data information
	 */
	public ByteBuffer permissionData();
	
	/**
	 * Returns PublisherIdUserId.<br>
	 * Calling this method must be preceded by a call to {@link #hasPublisherId()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPublisherId()} returns false
	 * 
	 * @return publisher's user Id
	 */
	public  long publisherIdUserId();
	
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
	 * Returns ClearCache.
	 * 
	 * @return true if cache needs to be cleared on receipt of this status message; false otherwise
	 */
	public boolean clearCache();
	
	/**
	 * Returns PrivateStream.
	 * 
	 * @return true if this is private stream item; false otherwise
	 */
	public boolean privateStream();
	
	/**
	 * Returns the ServiceName within the MsgKey.<br>
	 * Calling this method must be preceded by a call to {@link #hasServiceName()}
	 * 
	 * @return String containing service name
	 */
	public String serviceName();
	
	/**
	 * Clears the StatusMsg.<br>
	 * Invoking clear() method clears all the values and resets all the defaults
	 * 
	 * @return reference to this object
	 */
	public StatusMsg clear();
	
	/**
	 * Specifies StreamId.
	 * 
	 * @throws OmmOutOfRangeException if streamId is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public StatusMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is is {@literal < 0 or > 255}
	 * 
	 * @param domainType specifies RDM Message Model Type
	 * @return reference to this object
	 */
	public StatusMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * 
	 * @throws OmmInvalidUsageException if name is null
	 * 
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public StatusMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * 
	 * @throws OmmOutOfRangeException if nameType is {@literal < 0 or > 255}
	 * 
	 * @param nameType specifies RDM Instrument NameType
	 * @return reference to this object
	 */
	public StatusMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set or if name is null
	 *  
	 * @param serviceName specifies service name
	 * @return reference to this object
	 */
	public StatusMsg serviceName(String serviceName);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 *                               or if serviceId is {@literal < 0 or > 65535}
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public StatusMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * 
	 * @throws OmmOutOfRangeException if id is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public StatusMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 4294967295L}
	 * 
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public StatusMsg filter(long filter);
	
	/**
	 * Specifies State.<br>
	 * Defaults: statusCode=OmmState.StatusCode.NONE, statusText=DataImpl.EMPTY_STRING
	 * 
	 * @param streamState conveys item stream state value
	 * @param dataState conveys item data state value
	 * 
	 * @return reference to this object
	 */
	public StatusMsg state(int streamState, int dataState);
	
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
	public StatusMsg state(int streamState, int dataState, int statusCode);
	
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
	public StatusMsg state(int streamState, int dataState,	int statusCode, String statusText);
	
	/**
	 * Specifies ItemGroup.
	 *
	 * @throws OmmInvalidUsageException if itemGroup is null
	 * 
	 * @param itemGroup a ByteBuffer object with item group information
	 * @return reference to this object
	 */
	public StatusMsg itemGroup(ByteBuffer itemGroup);
	
	/**
	 * Specifies PermissionData.
	 *
	 * @throws OmmInvalidUsageException if permissionData is null
	 * 
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public StatusMsg permissionData(ByteBuffer permissionData);
	
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
	public StatusMsg publisherId(long userId, long userAddress);
	
	/**
	 * Specifies Attrib.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public StatusMsg attrib(ComplexType data);
	
	/**
	 * Specifies Payload.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public StatusMsg payload(ComplexType data);
	
	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @throws OmmInvalidUsageException if buffer is null
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public StatusMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies ClearCache.
	 * 
	 * @param clearCache true if cache needs to be cleared; false otherwise
	 * @return reference to this object
	 */
	public StatusMsg clearCache(boolean clearCache);
	
	/**
	 * Specifies PrivateStream.
	 * 
	 * @param privateStream true if private stream; false otherwise
	 * @return reference to this object
	 */
	public StatusMsg privateStream(boolean privateStream);

	/**
	 *  Returns a string representation of the class instance.
	 * @param dictionary use for toString() conversion
	 * @return string representation of the class instance
	 */
	public String toString(DataDictionary dictionary);
	
	/**
	 *  Performs a deep copy of {@link StatusMsg} into the passed in object.
	 * @param destStatusMsg to copy StatusMsg into. 
	 * @throws OmmInvalidUsageException if this object is not used for decoding routine.
	 * @throws OmmInvalidUsageException if the destStatusMsg is not created from {@link EmaFactory#createStatusMsg(int initialSize)}
	 */
	public void copy(StatusMsg destStatusMsg);
}
