///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * PostMsg allows consumer applications to contribute content.
 * 
 * <p>PostMsg may be submitted on any market item stream or login stream.<br>
 * Submission on a market item stream is referred to as the "on stream posting"
 * while submission on a login stream is considered as the "off stream posting".<br>
 * On stream posting content is related to the item on whose stream the posting happens,
 * while the off stream posting	may contain info about any item.</p>
 * 
 * <p>PostMsg may be submitted using {@link OmmConsumer#submit(PostMsg, long)}.</p>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and getting of information from PostMsg.<br>
 * Objects of this class are not cache-able.<br>
 * Decoding of just encoded PostMsg in the same application is not supported.
 *
 * @see Data
 * @see Msg
 */
public interface PostMsg extends Msg
{
	/**
	 * PostUserRights represents post user rights.
	 */
	public static class PostUserRights
	{
		/**
		 * Specifies ability to create records.
		 */
		public final static int CREATE = 0x01;

		/**
		 * Specifies ability to delete records.
		 */
		public final static int DELETE = 0x02;
	
		/**
		 * Specifies ability to modify permissions.
		 */
		public final static int MODIFY_PERMISSION = 0x04;
	}
	
	
	/** 
	 * Returns the PostUserRights value as a string format.
	 * 
	 * @return string representation of PostUserRights
	 */
	public String postUserRightsAsString();
	
	/** 
	 * Indicates presence of SeqNum.<br>
	 * Sequence number is an optional member of PostMsg.
	 * 
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();
	
	/**
	 * Indicates presence of the PostId.<br>
	 * Post id is an optional member of PostMsg.
	 * 
	 * @return true if PostId is set; false otherwise
	 */
	public boolean hasPostId();
	
	/**
	 * Indicates presence of PartNum.<br>
	 * Part number is an optional member of PostMsg.
	 * 
	 * @return true if part number is set; false otherwise
	 */
	public boolean hasPartNum();
	
	/**
	 * Indicates presence of PostUserRights.<br>
	 * Post user rights is an optional member of PostMsg.
	 * 
	 * @return true if PostUserRights are set; false otherwise
	 */
	public boolean hasPostUserRights();
	
	/**
	 * Indicates presence of PermissionData.<br>
	 * Permission data is an optional member of PostMsg.
	 * 
	 * @return true if permission data is set; false otherwise
	 */
	public boolean hasPermissionData();
	
	/**
	 * Indicates presence of the ServiceName within the MsgKey.<br>
	 * Service name is an optional member of PostMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();
	
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
	 * Returns the PostId.<br>
	 * Calling this method must be preceded by a call to {@link #hasPostId()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPostId()} returns false
	 * 
	 * @return post id
	 */
	public long postId();
	
	/**
	 * Returns PartNum.<br>
	 * Calling this method must be preceded by a call to {@link #hasPartNum()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPartNum()} returns false
	 * 
	 * @return part number
	 */
	public int partNum();
	
	/**
	 * Returns PostUserRights.<br>
	 * Calling this method must be preceded by a call to {@link #hasPostUserRights()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPostUserRights()} returns false
	 * 
	 * @return PostUserRights
	 */
	public int postUserRights();
	
	/**
	 * Returns PermissionData.<br>
	 * Calling this method must be preceded by a call to {@link #hasPermissionData()}.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasPermissionData()} returns false
	 * 
	 * @return ByteBuffer containing permission data
	 */
	public ByteBuffer permissionData();
	
	/**
	 * Returns PublisherIdUserId.
	 * 
	 * @return publisher's user Id
	 */
	public long publisherIdUserId();
	
	/**
	 * Returns PublisherIdUserAddress.
	 * 
	 * @return publisher's user address
	 */
	public long publisherIdUserAddress();
	
	/**
	 * Indicates that acknowledgment is requested.
	 * 
	 * @return true if acknowledgment is requested; false otherwise
	 */
	public boolean solicitAck();
	
	/**
	 * Returns Complete.
	 * 
	 * @return true if this is the last part of the multi part post message
	 */
	public boolean complete();
	
	/**
	 * Returns the ServiceName within the MsgKey.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasServiceName()} returns false
	 * @return String containing service name
	 */
	public String serviceName();

	/**
	 * Clears the PostMsg.<br>
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public PostMsg clear();
	
	/**
	 * Specifies StreamId.
	 * 
	 * @throws OmmOutOfRangeException if streamId is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public PostMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is is {@literal < 0 or > 255}
	 * 
	 * @param domainType specifies RDM Message Model Type
	 * @return reference to this object
	 */
	public PostMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * 
	 * @throws OmmInvalidUsageException if name is null
	 * 
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public PostMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * 
	 * @throws OmmOutOfRangeException if nameType is {@literal < 0 or > 255}
	 * 
	 * @param nameType specifies RDM Instrument NameType
	 * @return reference to this object
	 */
	public PostMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if service id is already set or if name is null
	 *  
	 * @param name specifies service name
	 * @return reference to this object
	 */
	public PostMsg serviceName(String name);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set or if serviceId is {@literal < 0 or > 65535}
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public PostMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * 
	 * @throws OmmOutOfRangeException if id is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public PostMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 4294967295L}
	 * 
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public PostMsg filter(long filter);
	
	/**
	 * Specifies SeqNum.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 4294967295L}
	 * 
	 * @param seqNum specifies sequence number
	 * @return reference to this object
	 */
	public PostMsg seqNum(long seqNum);
	
	/**
	 * Specifies PostId.
	 * 
	 * @throws OmmOutOfRangeException if postId is {@literal < 0 or > 4294967295L}
	 * 
	 * @param postId specifies post id
	 * @return reference to this object
	 */
	public PostMsg postId(long postId);
	
	/**
	 * Specifies PartNum.
	 * 
	 * @throws OmmOutOfRangeException if partNum is {@literal < 0 or > 32767}
	 *  
	 * @param partNum specifies part number
	 * @return reference to this object
	 */
	public PostMsg partNum(int partNum);
	
	/**
	 * Specifies PostUserRights.
	 * 
	 * @throws OmmOutOfRangeException if postUserRights is {@literal < 0 or > 32767}
	 *  
	 * @param postUserRights specifies post user rights
	 * @return reference to this object
	 */
	public PostMsg postUserRights(int postUserRights);
	
	/**
	 * Specifies PermissionData.
	 * 
	 * @throws OmmInvalidUsageException if permissionData is null
	 * 
	 * @param permissionData a ByteBuffer object with permission data information
	 * @return reference to this object
	 */
	public PostMsg permissionData(ByteBuffer permissionData);
	
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
	public PostMsg publisherId(long userId, long userAddress);
	
	/**
	 * Specifies Attrib.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public PostMsg attrib(ComplexType data);
	
	/**
	 * Specifies Payload.
	 * 
	 * @throws OmmInvalidUsageException if data is null
	 * 
	 * @param data an object of ComplexType
	 * @return reference to this object
	 */
	public PostMsg payload(ComplexType data);
	
	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @throws OmmInvalidUsageException if buffer is null
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public PostMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies acknowledgment.
	 * 
	 * @param ack specifies if an acknowledgment is requested
	 * @return reference to this object
	 */
	public PostMsg solicitAck(boolean ack);
	
	/**
	 * Specifies Complete.<br>
	 * Must be set to true for one part post message
	 * 
	 * @param complete specifies if this is the last part of the multi part post message
	 * @return reference to this object
	 */
	public PostMsg complete(boolean complete);
}