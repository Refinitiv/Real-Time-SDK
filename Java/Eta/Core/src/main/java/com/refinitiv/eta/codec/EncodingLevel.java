/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

class EncodingLevel
{
    int       _countWritePos;     /* The position of the count pointer in the buffer */
    int       _initElemStartPos;  /* The start position of an initialize element */
    int       _containerStartPos; /* The start position of a container */
    int       _currentCount;      /* Current number of elements */
    int       _encodingState;     /* The current encoding state */
    int       _containerType;     /* The containerType of the list */
    int       _flags;             /* Internal flag values used for various purposes */
    int       _reservedBytes;     /* Bytes reserved for data that will need to be written later
                                     (e.g. count for a container's entries that is written after summary data is encoded). */
    Object    _listType;          /* Pointer to actual list type */
    FieldSetDefImpl   _fieldListSetDef; /* RsslFieldListSetDef, used to encode this level */
    ElementSetDefImpl _elemListSetDef;  /* RsslElementListSetDef, used to decode this level */
	
    /* Buffer size mark used internally */
    final EncodeSizeMark    _internalMark = new EncodeSizeMark();
	
    /* Second Internal mark */
    final EncodeSizeMark    _internalMark2 = new EncodeSizeMark();
	
    void init(int dFormat, int eState, Object lType, int cStartPos)
    {
        _countWritePos = 0;
        _initElemStartPos = 0;
        _currentCount = 0;
        _internalMark._sizePos = 0;
        _internalMark._sizeBytes = 0;
        _internalMark2._sizePos = 0;
        _internalMark2._sizeBytes = 0;
        _fieldListSetDef = null;
        _elemListSetDef = null;
        _encodingState = eState;
        _containerType = dFormat;
        _reservedBytes = 0;
        _listType = lType;
        _containerStartPos = cStartPos;
    }
	
    void realighBuffer(int offset)
    {
        _countWritePos += offset;
        _initElemStartPos += offset;
        _internalMark._sizePos += offset;
        _internalMark2._sizePos += offset;
        _containerStartPos += offset;
    }
}


