///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * StatusMsg conveys item state information.
 * <p>StatusMsg is used to convey item state information, permission change or item group id change.</p>
 */
public interface StatusMsg extends Msg
{
	/**
	 * Indicates presence of ItemGroup.
	 * Item Group is an optional member of StatusMsg.
	 * 
	 * @return true if Item Group information is set; false otherwise
	 */
	public boolean hasItemGroup();
	
	/**
	 * Indicates presence of State.
	 * Item State is an optional member of StatusMsg.
	 * 
	 * @return true if state information is set; false otherwise
	*/
	public boolean hasState();
	
	/**
	 * Indicates presence of PermissionData.
	 * Permission data is optional member of StatusMsg.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Indicates presence of PublisherId.
	 * Publisher id is an optional member of StatusMsg.
	 * 
	 * @return true if publisher id is set; false otherwise
	 */
	public boolean hasPublisherId();
	
	/**
	 * Indicates presence of the ServiceName within the MsgKey.
	 * Service name is an optional member of StatusMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();
	
	/**
	 * Returns State.
	 * <br>Calling this method must be preceded by a call to {@link #hasState()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasState()} returns false
	 * 
	 * @return state of item (e.g. Open / Ok)
	 */
	public OmmState state();
	
	/**
	 * Returns ItemGroup.
	 * <br>Calling this method must be preceded by a call to {@link #hasItemGroup()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasItemGroup()} returns false
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
	 * @return ByteBuffer containing permission data information
	 */
	public ByteBuffer permissionData();
	
	/**
	 * Returns PublisherIdUserId.
	 * <br>Calling this method must be preceded by a call to {@link #hasPublisherId()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPublisherId()} returns false
	 * 
	 * @return publisher's user Id
	 */
	public  long publisherIdUserId();
	
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
	 * Returns the ServiceName within the MsgKey.
	 * <br>Calling this method must be preceded by a call to {@link #hasPermissionData()}
	 * 
	 * @return String containing service name
	 */
	public String serviceName();
	
	/**
	 * Clears the StatusMsg.
	 * Invoking clear() method clears all the values and resets all the defaults
	 * 
	 * @return reference to this object
	 */
	public StatusMsg clear();
	
	/**
	 * Specifies StreamId.
	 * 
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public StatusMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is greater than 255
	 * 
	 * @param domainType specifies RDM Message Model Type (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#MMT_MARKET_PRICE})
	 * 
	 * @return reference to this object
	 */
	public StatusMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * 
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public StatusMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * 
	 * @param nameType specifies RDM Instrument NameType
	 *        (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#INSTRUMENT_NAME_RIC})
	 * @return reference to this object
	 */
	public StatusMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set
	 * 
	 * @param serviceName specifies service name
	 * @return reference to this object
	 */
	public StatusMsg serviceName(String serviceName);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public StatusMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public StatusMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * 
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public StatusMsg filter(long filter);
	
	/**
	 * Specifies State.
	 * 
	 * @param streamState conveys item stream state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState conveys item data state value
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#SUSPECT})
	 * @return reference to this object
	 */
	public StatusMsg state(int streamState, int dataState);
	
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
	public StatusMsg state(int streamState, int dataState, int statusCode);
	
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
	public StatusMsg state(int streamState, int dataState,	int statusCode, String statusText);
	
	/**
	 * Specifies ItemGroup.
	 * 
	 * @param itemGroup a ByteBuffer object with item group information
	 * @return reference to this object
	 */
	public StatusMsg itemGroup(ByteBuffer itemGroup);
	
	/**
	 * Specifies PermissionData.
	 * 
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public StatusMsg permissionData(ByteBuffer permissionData);
	
	/**
	 * Specifies PublisherId.
	 * 
	 * @param userId specifies publisher's user id
	 * @param userAddress specifies publisher's user address
	 * @return reference to this object
	 */
	public StatusMsg publisherId(long userId, long userAddress);
	
	/**
	 * Specifies Attrib.
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public StatusMsg attrib(ComplexType data);
	
	/**
	 * Specifies Payload.
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public StatusMsg payload(ComplexType data);
	
	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public StatusMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies ClearCache.
	 * 
	 * @param clearCache true if cache needs to be cleared; false otherwise (default value is false)
	 * @return reference to this object
	 */
	public StatusMsg clearCache(boolean clearCache);
	
	/**
	 * Specifies PrivateStream.
	 * @param privateStream true if private stream; false otherwise (default value is false)
	 * @return reference to this object
	 */
	public StatusMsg privateStream(boolean privateStream);
}