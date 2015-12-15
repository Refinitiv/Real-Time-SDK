///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * GenericMsg allows applications to bidirectionally send messages without any implied
 * message semantics.
 * <br>GenericMsg may be sent on any item stream using {@link OmmConsumer#submit(GenericMsg, long)}.
 * 
 * <p>Objects of this class are intended to be short lived or rather transitional.
 * <br>This class is designed to efficiently perform setting and getting of information from GenericMsg.
 * <br>Objects of this class are not cache-able.
 * <br>Decoding of just encoded GenericMsg in the same application is not supported.
 */
public interface GenericMsg extends Msg
{
	/**
	 * Indicates presence of SeqNum.
	 * <br>Sequence number is an optional member of GenericMsg.
	 * 
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();
	
	/** 
	 * Indicates presence of SecondarySeqNum.
	 * <br>Secondary sequence number is an optional member of GenericMsg.
	 * 
	 * @return true if secondary sequence number is set; false otherwise
	 */
	public boolean hasSecondarySeqNum();
	
	/**
	 * Indicates presence of PartNum.
	 * <br>Part number is an optional member of GenericMsg.
	 * 
	 * @return true if part number is set; false otherwise
	 */
	public boolean hasPartNum();
	
	/**
	 * Indicates presence of PermissionData.
	 * <br>Permission data is optional on GenericMsg.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Returns SeqNum.
	 * <br>Calling this method must be preceded by a call to {@link #hasSeqNum()}.
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasSeqNum()} returns false
	 * 
	 * @return sequence number
	 */
	public long seqNum();
	
	/**
	 * Returns SecondarySeqNum.
	 * <br>Calling this method must be preceded by a call to {@link #hasSecondarySeqNum()}.
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasSecondarySeqNum()} returns false
	 * 
	 * @return secondary sequence number
	 */
	public long secondarySeqNum();
	
	/** 
	 * Returns PartNum.
	 * <br>Calling this method must be preceded by a call to {@link #hasPartNum()}.
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasPartNum()} returns false
	 * 
	 * @return part number
	 */
	public int partNum();
	
	/**
	 * Returns PermissionData.
	 * <br>Calling this method must be preceded by a call to {@link #hasPermissionData()}.
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasPermissionData()} returns false
	 * 
	 * @return ByteBuffer containing permission data
	*/
	public ByteBuffer permissionData();
	
	/**
	 * Returns Complete.
	 * 
	 * @return true if this is a one part generic message or the final part of the multi part generic message.
	 */
	public boolean complete();

	/**
	 * Clears the GenericMsg.
	 * <br>Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public GenericMsg clear();
	
	/**
	 * Specifies StreamId.
	 * 
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public GenericMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throw OmmUnsupportedDomainTypeException if domainType is greater than 255
	 * 
	 * @param domainType specifies RDM Message Model Type
	 *        (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#MMT_MARKET_PRICE})
	 * 
	 * @return reference to this object
	 */
	public GenericMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * 
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public GenericMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * 
	 * @param nameType specifies RDM Instrument NameType
	 *        (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#INSTRUMENT_NAME_RIC})
	 *        
	 * @return reference to this object
	 */
	public GenericMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public GenericMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public GenericMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * 
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public GenericMsg filter(long filter);
	
	/**
	 * Specifies SeqNum.
	 * 
	 * @param seqNum specifies sequence number
	 * @return reference to this object
	 */
	public GenericMsg seqNum(long seqNum);
	
	/**
	 * Specifies SecondarySeqNum.
	 * 
	 * @param secondarySeqNum specifies secondary sequence number
	 * @return reference to this object
	 */
	public GenericMsg secondarySeqNum(long secondarySeqNum);
	
	/**
	 * Specifies PartNum.
	 * 
	 * @param partNum specifies part number
	 * @return reference to this object
	 */
	public GenericMsg partNum(int partNum);
	
	/**
	 * Specifies PermissionData.
	 * 
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public GenericMsg permissionData(ByteBuffer permissionData);
	
	/**
	 * Specifies Attrib.
	 * 
	 * @param attrib an object of ComplexType
	 * @return reference to this object
	 */
	public GenericMsg attrib(ComplexType data);
	
	/**
	 * Specifies Payload.
	 * 
	 * @param payload an object of ComplexType
	 * @return reference to this object
	 */
	public GenericMsg payload(ComplexType data);
	
	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @param buffer ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public GenericMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies Complete.
	 * <br>Must be set to true for one part generic message.
	 * 
	 * @param complete specifies if this is the last part of the multi part generic message
	 * @return reference to this object
	 */
	public GenericMsg complete(boolean complete);
}