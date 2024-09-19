/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Combination of bit values that indicates the presence of optional field list content.
 * 
 * @see FieldList
 */
public class FieldListFlags
{
    private FieldListFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) None of the optional flags are set */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates that dictionaryId and fieldListNum members are present,
     * which should be provided as part of the initial refresh message on a
     * stream or on the first refresh message after issuance of a CLEAR_CACHE command.
     */
    public static final int HAS_FIELD_LIST_INFO = 0x01;

    /**
     * (0x02) Indicates that the {@link FieldList} contains set-defined data.
     * This value can be set in addition to {@link #HAS_STANDARD_DATA} if both
     * standard and set-defined data are present in this {@link FieldList}. If
     * no entries are present in the {@link FieldList}, this flag value should not be set.
     */
    public static final int HAS_SET_DATA = 0x02;

    /**
     * (0x04) Indicates the presence of a setId, used to determine the set
     * definition used for encoding or decoding the set data on this {@link FieldList}.
     */
    public static final int HAS_SET_ID = 0x04;

    /**
     * (0x08) Indicates that the {@link FieldList} contains standard
     * fieldId-value pair encoded data. This value can be set in addition to
     * {@link #HAS_SET_DATA} if both standard and set-defined data are present
     * in this {@link FieldList}. If no entries are present in the
     * {@link FieldList}, this flag value should not be set.
     */
    public static final int HAS_STANDARD_DATA = 0x08;
}
