///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * Msg interface is for all message representing classes.
 */
public interface Msg extends ComplexType
{
	/** 
	 * Indicates presence of the MsgKey.
	 * 
	 * @return true if name, name type, service id, service name, id, filter, or attribute is set; false otherwise
	 */
	public boolean hasMsgKey();

	/**
	 * Indicates presence of the Name within the MsgKey.
	 * 
	 * @return true if name is set; false otherwise
	 */
	public boolean hasName();

	/**
	 * Indicates presence of the NameType within the MsgKey.
	 * 
	 * @return true if name type is set; false otherwise
	 */
	public boolean hasNameType();

	/**
	 * Indicates presence of the ServiceId within the MsgKey.
	 * 
	 * @return true if service id is set; false otherwise
	 */
	public boolean hasServiceId();

	/**
	 * Indicates presence of the Identifier within the MsgKey.
	 * 
	 * @return true if Id is set; false otherwise
	 */
	public boolean hasId();

	/**
	 * Indicates presence of the Filter within the MsgKey.
	 * 
	 * @return true if filter is set; false otherwise
	 */
	public boolean hasFilter();

	/**
	 * Indicates presence of the ExtendedHeader.
	 * 
	 * @return true if ExtendedHeader is set; false otherwise
	 */
	public boolean hasExtendedHeader();

	/**
	 * Returns the StreamId, which is the unique open message stream identifier on the wire.
	 * 
	 * @return stream id value
	 */
	public int streamId();

	/**
	 * Returns the domain type, which is the unique identifier of a domain.
	 * 
	 * @return domain type value
	 */
	public int domainType();

	/**
	 * Returns the Name within the MsgKey.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasName()} returns false
	 * 
	 * @return String containing the Name
	 */
	public String name();

	/**
	 * Returns the name type within the MsgKey.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasNameType()} returns false
	 * 
	 * @return name type value
	 */
	public int nameType();

	/**
	 * Returns the ServiceId within the MsgKey.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasServiceId()} returns false
	 * 
	 * @return service id value
	 */
	public int serviceId();

	/**
	 * Returns the Identifier within the MsgKey.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasId()} returns false
	 * 
	 * @return id value
	 */
	public int id();

	/**
	 * Returns the Filter within the MsgKey.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasFilter()} returns false
	 * 
	 * @return filter value
	 */
	public long filter();

	/**
	 * Returns the ExtendedHeader.
	 * 
	 * @throws OmmInvalidUsageException if {@link #hasExtendedHeader()} returns false
	 * 
	 * @return ByteBuffer containing extendedHeader info value
	 */
	public ByteBuffer extendedHeader();

	/**
	 * Returns the contained attributes Data based on the attribute's DataType.
	 * <p>Attrib contains no data if {@link com.thomsonreuters.ema.access.Attrib#dataType()}
	 * returns {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA}.</p>
	 * 
	 * @return reference to Attrib object
	 */
	public Attrib attrib();

	/**
	 * Returns the contained payload Data based on the payload DataType.
	 * <p>Payload contains no data if {@link com.thomsonreuters.ema.access.Payload#dataType()}
	 * returns {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA}.</p>
	 * 
	 * @return reference to Payload object
	 */
	public Payload payload();

}