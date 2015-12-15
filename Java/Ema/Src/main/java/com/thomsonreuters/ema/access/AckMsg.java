///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * AckMsg indicates success or failure of PostMsg.
 * 
 * <p>If requested, AckMsg is sent by provider acting on PostMsg received from consumer.
 * AckMsg indicates success or failure of caching / processing of received PostMsg.</p>
 * 
 * <p>AckMsg is optional.
 * Consumer requests provider to send AckMsg by calling {@link com.thomsonreuters.ema.access.PostMsg#solicitAck(boolean)} with true.</p>	
 */
public interface AckMsg extends Msg
{
	/**
	 * NackCode represents negative acknowledgement code.
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
	 * Indicates presence of SeqNum.
	 * Sequence number is an optional member of AckMsg.
	 * 
	 * @return true if sequence number is set; false otherwise
	 */
	public boolean hasSeqNum();
	
	/**
	 * Indicates presence of NackCode.
	 * Negative acknowledgement code is an optional member of AckMsg.
	 * 
	 * @return true if NackCode is set; false otherwise
	 */
	public boolean hasNackCode();
	
	/**
	 * Indicates presence of Text.
	 * Text is an optional member of AckMsg.
	 * 
	 * @return true if text is set; false otherwise
	 */
	public boolean hasText();
	
	/**
	 * Indicates presence of the ServiceName within the MsgKey.
	 * Service name is an optional member of AckMsg.
	 * 
	 * @return true if service name is set; false otherwise
	 */
	public boolean hasServiceName();
	
	/**
	 * Returns SeqNum.
	 * <br>Calling this method must be preceded by a call to {@link #hasSeqNum()}
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasSeqNum()} returns false
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
	 * Returns NackCode.
	 * <br>Calling this method must be preceded by a call to {@link #hasNackCode()}
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasNackCode()} returns false
	 * 
	 * @return NackCode
	 */
	public int nackCode();
	
	/**
	 * Returns Text.
	 * <br>Calling this method must be preceded by a call to {@link #hasText()}
	 *
	 * @throw OmmInvalidUsageException if {@link #hasText()} returns false
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
	 * Returns the ServiceName within the MsgKey.
	 * <br>Calling this method must be preceded by a call to {@link #hasServiceName()}
	 * 
	 * @throw OmmInvalidUsageException if {@link #hasServiceName()} returns false
	 * 
	 * @return String containing service name
	 */
	public String serviceName();

	/**
	 * Clears the AckMsg.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public AckMsg clear();
	
	/**
	 * Specifies StreamId.
	 * 
	 * @param streamId - specifies stream id
	 * @return reference to this object
	 */
	public AckMsg streamId(int streamId);
	
	/**
	 * Specifies DomainType.
	 * 
	 * @throw OmmUnsupportedDomainTypeException if domainType is greater than 255
	 * 
	 * @param domainType - specifies RDM Message Model Type (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#MMT_MARKET_PRICE})
	 * @return reference to this object
	 */
	public AckMsg domainType(int domainType);
	
	/**
	 * Specifies Name.
	 * 
	 * @param name - specifies item name
	 * @return reference to this object
	 */
	public AckMsg name(String name);
	
	/**
	 * Specifies NameType.
	 * 
	 * @param nameType - specifies RDM Instrument NameType (default value is {@link com.thomsonreuters.ema.rdm.EmaRdm#INSTRUMENT_NAME_RIC})
	 * @return reference to this object
	 */
	public AckMsg nameType(int nameType);
	
	/**
	 * Specifies ServiceName.
	 * 
	 * @throw OmmInvalidUsageException if service id is already set
	 * 
	 * @param serviceName - specifies service name
	 * @return reference to this object
	 */
	public AckMsg serviceName(String serviceName);
	
	/**
	 * Specifies ServiceId.
	 * 
	 * @throw OmmInvalidUsageException if service name is already set
	 * 
	 * @param serviceId - specifies service id
	 * @return reference to this object
	 */
	public AckMsg serviceId(int serviceId);
	
	/**
	 * Specifies Id.
	 * 
	 * @param id - specifies Id
	 * @return reference to this object
	 */
	public AckMsg id(int id);
	
	/**
	 * Specifies Filter.
	 * 
	 * @param filter - specifies filter
	 * @return reference to this object
	 */
	public AckMsg filter(long filter);
	
	/**
	 * Specifies SeqNum.
	 * 
	 * @param seqNum - specifies sequence number
	 * @return reference to this object
	 */
	public AckMsg seqNum(long seqNum);
	
	/**
	 * Specifies AckId.
	 * 
	 * @param ackId - specifies ack id
	 * @return reference to this object
	 */
	public AckMsg ackId(long ackId);
	
	/**
	 * Specifies NackCode.
	 * 
	 * @param nackCode - specifies negative acknowledgement code
	 * @return reference to this object
	 */
	public AckMsg nackCode(int nackCode);
	
	/**
	 * Specifies Text.
	 * 
	 * @param text - specifies message text information
	 * @return reference to this object
	 */
	public AckMsg text(String text);
	
	/**
	 * Specifies Attrib.
	 * 
	 * @param attrib - an object of ComplexType
	 * @return reference to this object
	 */
	public AckMsg attrib(ComplexType attrib);
	
	/**
	 * Specifies Payload.
	 * 
	 * @param payload - an object of ComplexType
	 * @return reference to this object
	 */
	public AckMsg payload(ComplexType payload);
	
	/**
	 * Specifies ExtendedHeader.
	 * 
	 * @param buffer - a ByteBuffer containing extendedHeader information
	 * @return reference to this object
	 */
	public AckMsg extendedHeader(ByteBuffer buffer);
	
	/**
	 * Specifies PrivateStream.
	 * 
	 * @param privateStream - specifies if this is a private stream (default is false)
	 * @return reference to this object
	 */
	public AckMsg privateStream(boolean privateStream);
}