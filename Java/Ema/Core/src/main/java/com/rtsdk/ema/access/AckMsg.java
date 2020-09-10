///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.nio.ByteBuffer;

/**
 * AckMsg indicates success or failure of PostMsg.
 * 
 * <p>If requested, AckMsg is sent by provider acting on PostMsg received from consumer.<br>
 * AckMsg indicates success or failure of caching / processing of received PostMsg.</p>
 * 
 * <p>AckMsg is optional.<br>
 * Consumer requests provider to send AckMsg by calling<br>
 * {@link com.rtsdk.ema.access.PostMsg#solicitAck(boolean)} with true.</p>
 * 
 * @see Data
 * @see Msg
 */
public interface AckMsg extends Msg
{
	/**
	 * NackCode represents negative acknowledgment code.
	 */
	public static class NackCode
	{
		/**
		 * Indicates no nack code
		 */
		public final static int NONE = 0;

		/**
		 * Indicates access denied
		 */
		public final static int ACCESS_DENIED = 1;

		/**
		 * Indicates denied by source
		 */
		public final static int DENIED_BY_SOURCE = 2;

		/**
		 * Indicates source is down
		 */
		public final static int SOURCE_DOWN = 3;

		/**
		 * Indicates source is unknown
		 */
		public final static int SOURCE_UNKNOWN = 4;

		/**
		 * Indicates no resources
		 */
		public final static int NO_RESOURCES = 5;

		/**
		 * Indicates no response
		 */
		public final static int NO_RESPONSE = 6;

		/**
		 * Indicates gateway down
		 */
		public final static int GATEWAY_DOWN = 7;

		/**
		 * Indicates unknown symbol
		 */
		public final static int SYMBOL_UNKNOWN = 10;

		/**
		 * Indicates not open
		 */
		public final static int NOT_OPEN = 11;

		/**
		 * Indicates invalid content
		 */
		public final static int INVALID_CONTENT = 12;
	}

	/**
	 * Returns the NackCode value in a string format.
	 * 
	 * @return string representation of the NackCode
	 */
	public String nackCodeAsString();
	
	/**
	 * Indicates presence of SeqNum.<br>
	 * Sequence number is an optional member of AckMsg.
	 * 
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();
	
	/**
	 * Indicates presence of NackCode.<br>
	 * Negative acknowledgment code is an optional member of AckMsg.
	 * 
	 * @return true if NackCode is set; false otherwise
	 */
	public boolean hasNackCode();
	
	/**
	 * Indicates presence of Text.<br>
	 * Text is an optional member of AckMsg.
	 * 
	 * @return true if text is set; false otherwise
	 */
	public boolean hasText();
	
	/**
	 * Indicates presence of the ServiceName within the MsgKey.<br>
	 * Service name is an optional member of AckMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();
	
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
	 * Returns the AckId.
	 * 
	 * @return ack id
	 */
	public long ackId();
	
	/**
	 * Returns NackCode.<br>
	 * Calling this method must be preceded by a call to {@link #hasNackCode()}
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasNackCode()} returns false
	 * 
	 * @return NackCode
	 */
	public int nackCode();
	
	/**
	 * Returns Text.<br>
	 * Calling this method must be preceded by a call to {@link #hasText()}
	 *
	 * @throws OmmInvalidUsageException if {@link #hasText()} returns false
	 * 
	 * @return text
	 */
	public String text();
	
	/** 
	 * Returns PrivateStream.
	 * 
	 * @return true if this is a private stream; false otherwise
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
	 * Clears the AckMsg.<br>
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public AckMsg clear();
	
	/**
	 * Specifies StreamId.
	 * 
	 * @throws OmmOutOfRangeException if streamId is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param streamId specifies stream id
	 * @return reference to this object
	 */
	public AckMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throws OmmUnsupportedDomainTypeException if domainType is is {@literal < 0 or > 255}
	 * 
	 * @param domainType specifies RDM Message Model Type
	 * @return reference to this object
	 */
	public AckMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * 
	 * @throws OmmInvalidUsageException if name is null
	 * @param name specifies item name
	 * @return reference to this object
	 */
	public AckMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * 
	 * @throws OmmOutOfRangeException if nametype is {@literal < 0 or > 255}

	 * @param nameType specifies RDM Instrument NameType
	 * @return reference to this object
	 */
	public AckMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceName.
	 * 
	 * @throws OmmInvalidUsageException if serviceName is null or if service id is already set
	 * 
	 * @param serviceName specifies service name
	 * @return reference to this object
	 */
	public AckMsg serviceName(String serviceName);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @throws OmmInvalidUsageException if service name is already set
	 * @throws OmmOutOfRangeException if serviceId is {@literal < 0 or > 65535}
	 * 
	 * @param serviceId specifies service id
	 * @return reference to this object
	 */
	public AckMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * 
	 * @throws OmmOutOfRangeException if id is {@literal < -2147483648 or > 2147483647}
	 * 
	 * @param id specifies Id
	 * @return reference to this object
	 */
	public AckMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 65535}
	 * 
	 * @param filter specifies filter
	 * @return reference to this object
	 */
	public AckMsg filter(long filter);
	
	/**
	 * Specifies SeqNum.
	 * 
	 * @throws OmmOutOfRangeException if filter is {@literal < 0 or > 4294967295L}
	 * 
	 * @param seqNum specifies sequence number
	 * @return reference to this object
	 */
	public AckMsg seqNum(long seqNum);
	
	/**
	 * Specifies AckId.
	 * 
	 * @throws OmmOutOfRangeException if ackId is {@literal < 0 or > 4294967295L}
	 * 
	 * @param ackId specifies ack id
	 * @return reference to this object
	 */
	public AckMsg ackId(long ackId);
	
	/**
	 * Specifies NackCode.
	 * 
	 * @throws OmmOutOfRangeException if nackCode is {@literal < 0 or > 4294967295L}
	 * 
	 * @param nackCode specifies negative acknowledgment code
	 * @return reference to this object
	 */
	public AckMsg nackCode(int nackCode);
	
	/**
	 * Specifies Text.
	 * 
	 * @throws OmmInvalidUsageException if text is null
	 * 
	 * @param text specifies message text information
	 * @return reference to this object
	 */
	public AckMsg text(String text);
	
	/**
	 * Specifies Attrib.
	 * 
	 * @throws OmmInvalidUsageException if attrib is null
	 * 
	 * @param attrib an object of ComplexType
	 * @return reference to this object
	 */
	public AckMsg attrib(ComplexType attrib);
	
	/**
	 * Specifies Payload.
	 * 
	 * @throws OmmInvalidUsageException if payload is null
	 * 
	 * @param payload an object of ComplexType
	 * @return reference to this object
	 */
	public AckMsg payload(ComplexType payload);
	
	/**
	 * Specifies ExtendedHeader.
	 *
	 * @throws OmmInvalidUsageException if buffer is null
	 * 
	 * @param buffer a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public AckMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies PrivateStream.
	 * 
	 * @param privateStream specifies if this is a private stream (default is false)
	 * @return reference to this object
	 */
	public AckMsg privateStream(boolean privateStream);
}