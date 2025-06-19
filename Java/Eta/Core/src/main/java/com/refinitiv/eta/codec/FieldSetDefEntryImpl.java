/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.FieldSetDefEntry;

class FieldSetDefEntryImpl implements FieldSetDefEntry
{
    /* The field identifier */
    int     _fieldId;
    
    /* The field data type */
    int     _dataType;
	
    @Override
    public void clear()
    {
        _fieldId = 0;
        _dataType = 0;
    }

    @Override
    public int fieldId()
    {
        return _fieldId;
    }

    @Override
    public void fieldId(int fieldId)
    {
        assert (fieldId >= -32768 && fieldId <= 32767) : "fieldId is out of range (-32768-32767)"; // uint16
        _fieldId = fieldId;
    }

    @Override
    public int dataType()
    {
        return _dataType;
    }

    @Override
    public void dataType(int dataType)
    {
        assert (dataType > DataTypes.UNKNOWN && dataType <= DataTypes.LAST) : "dataType is out of range. Refer to DataTypes";
        _dataType = dataType;
    }
}
