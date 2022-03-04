/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * ETA Message Key houses a variety of attributes used to help identify the
 * contents flowing within a particular stream. This information, in conjunction
 * with domainType and quality of service information, can be used to uniquely
 * identify a data stream.
 * 
 * @see Msg
 * @see MsgKeyFlags
 */
public interface MsgKey
{
    /**
     * Checks the presence of the Service Id presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasServiceId();

    /**
     * Checks the presence of the Name presence flag.
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasName();

    /**
     * Checks the presence of the Name Type presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasNameType();

    /**
     * Checks the presence of the Filter presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasFilter();

    /**
     * Checks the presence of the Identifier presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasIdentifier();

    /**
     * Checks the presence of the Attribute presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasAttrib();

    /**
     * Applies the Service Id presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasServiceId();

    /**
     * Applies the Name presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasName();

    /**
     * Applies the Name Type presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasNameType();

    /**
     * Applies the Filter presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasFilter();

    /**
     * Applies the Identifier presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasIdentifier();

    /**
     * Applies the Attribute presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasAttrib();

    /**
     * Clears {@link MsgKey} object. Useful for object reuse during
     * encoding. While decoding, {@link MsgKey} object can be reused without
     * using {@link #clear()}.
     */
    public void clear();

    /**
     * The identifier associated with a service (a logical mechanism that provides
     * or enables access to a set of capabilities). This value should correspond
     * to the service content being requested or provided. In ETA, a service
     * corresponds to a subset of  content provided by a component, where the
     * Source Directory domain defines specific attributes associated with each
     * service. These attributes include information such as QoS, the specific
     * domain types available, and any dictionaries required to consume information
     * from the service. The Source Directory domain model can obtain this and
     * other types of information. Must be in the range of 0 - 65535.
     * 
     * @param serviceId the serviceId to set
     */
    public void serviceId(int serviceId);

    /**
     * The identifier associated with a service (a logical mechanism that provides
     * or enables access to a set of capabilities). This value should correspond
     * to the service content being requested or provided. In ETA, a service
     * corresponds to a subset of  content provided by a component, where the
     * Source Directory domain defines specific attributes associated with each
     * service. These attributes include information such as QoS, the specific
     * domain types available, and any dictionaries required to consume information
     * from the service. The Source Directory domain model can obtain this and
     * other types of information.
     * 
     * @return the serviceId
     */
    public int serviceId();

    /**
     * Numeric value, typically enumerated, that indicates the type of the name
     * member. Examples are User Name or RIC (i.e., the Reuters Instrument
     * Code). name types are defined on a per-domain model basis.
     * Must be in the range of 0 - 255.
     * 
     * @param nameType the nameType to set
     */
    public void nameType(int nameType);

    /**
     * Numeric value, typically enumerated, that indicates the type of the name
     * member. Examples are User Name or RIC (i.e., the Reuters Instrument
     * Code). name types are defined on a per-domain model basis.
     * 
     * @return the nameType
     */
    public int nameType();

    /**
     * The name associated with the contents of the stream. Specific name type
     * and contents should comply with the rules associated with the nameType member.
     * 
     * @param name the name to set
     */
    public void name(Buffer name);

    /**
     * The name associated with the contents of the stream. Specific name type
     * and contents should comply with the rules associated with the nameType member.
     * 
     * @return the name
     */
    public Buffer name();

    /**
     * Combination of filterId bit values that describe content for domain model
     * types with a {@link FilterList} payload. Filter identifier values are
     * defined by the corresponding domain model specification. Must be in the
     * range of 0 - 4294967296 (2^32).
     * <ul>
     * <li>When specified in {@link RequestMsg}, filter conveys information
     * about desired entries in responses</li>
     * <li>When specified on a message housing a FilterList payload, filter
     * conveys information about which filter entries are present.</li>
     * </ul>
     * 
     * @param filter the filter to set
     */
    public void filter(long filter);

    /**
     * Combination of filterId bit values that describe content for domain model
     * types with a {@link FilterList} payload. Filter identifier values are
     * defined by the corresponding domain model specification.
     * <ul>
     * <li>When specified in {@link RequestMsg}, filter conveys information
     * about desired entries in responses</li>
     * <li>When specified on a message housing a FilterList payload, filter
     * conveys information about which filter entries are present.</li>
     * </ul>
     * 
     * @return the filter
     */
    public long filter();

    /**
     * User specified numeric identifier, is defined on a per-domain
     * model basis. Must be in the range of -2147483648 - 2147483647.
     * 
     * @param identifier the identifier to set
     */
    public void identifier(int identifier);

    /**
     * User specified numeric identifier, is defined on a per-domain model basis.
     * 
     * @return the identifier
     */
    public int identifier();

    /**
     * Container Type of the msgKey attributes. Must be a container type from
     * the {@link DataTypes} enumeration in the range {@link DataTypes#CONTAINER_TYPE_MIN}
     * to 255. Can indicate the presence of a ETA container type (value
     * {@link DataTypes#NO_DATA} - 224) or some type of customer-defined container
     * type (value 225 - 255).
     * 
     * @param attribContainerType the attribContainerType to set
     */
    public void attribContainerType(int attribContainerType);

    /**
     * Container Type of the msgKey attributes. Must be a container type from
     * the {@link DataTypes} enumeration. Can indicate the presence of a ETA
     * container type (value {@link DataTypes#NO_DATA} - 224) or some type of
     * customer-defined container type (value 225 - 255).
     * 
     * @return the attribContainerType
     */
    public int attribContainerType();

    /**
     * Encoded MsgKey attribute information, used for additional identification
     * attributes. Contents are typically specified in the domain model. Type is
     * specified by attribContainerType.
     * 
     * @param encodedAttrib the encodedAttrib to set
     */
    public void encodedAttrib(Buffer encodedAttrib);

    /**
     * Encoded MsgKey attribute information, used for additional identification
     * attributes. Contents are typically specified in the domain model. Type is
     * specified by attribContainerType.
     * 
     * @return the encodedAttrib
     */
    public Buffer encodedAttrib();

    /**
     * Compares two MsgKey structures to determine if they are the same.
     * 
     * @param thatKey - the other key to compare to this one
     * 
     * @return returns true if keys match, false otherwise.
     */
    public boolean equals(MsgKey thatKey);

    /**
     * Performs a deep copy of a MsgKey. Expects all memory to be owned and
     * managed by user. If the memory for the buffers (i.e. name, attrib)
     * are not provided, they will be created.
     * 
     * @param destKey Destination to copy into. It cannot be null.
     * 
     * @return {@link CodecReturnCodes#SUCCESS}, if the key is copied successfully,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the buffer provided is too small
     */
    public int copy(MsgKey destKey);

    /**
     * Adds a FilterId to the key filter.
     * 
     * @param filterId The FilterId you want added to the filter. (e.g. STATE)
     * 
     * @return Failure if this key does not contain filter element, INVALID_DATA if
     * the filterId is greater then maximum filterID value (31), and SUCCESS otherwise
     *
     * @see com.refinitiv.eta.rdm.Directory.ServiceFilterIds
     */
    public int addFilterId(int filterId);

    /**
     * Checks if FilterId is present in key filter.
     * 
     * @param filterId The FilterId you want to check for
     * 
     * @return true - if exists; false if does not exist.
     * 
     * @see com.refinitiv.eta.rdm.Directory.ServiceFilterIds
     */
    public boolean checkHasFilterId(int filterId);

    /**
     * Sets all the flags applicable to this message key. Must be in the
     * range of 0 - 32767.
     * 
     * @param flags An integer containing all the flags applicable to this
     * message key
     * 
     * @see MsgKeyFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this message key.
     * 
     * @return All the flags applicable to this message key
     * 
     * @see MsgKeyFlags
     */
    public int flags();
}