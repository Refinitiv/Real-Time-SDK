/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.ElementSetDefEntry;

class ElementSetDefEntryImpl implements ElementSetDefEntry
{
    final Buffer   _name = CodecFactory.createBuffer();
    int         _dataType;
	
    @Override
    public void clear()
    {
        _name.clear();
        _dataType = 0;
    }

    @Override
    public Buffer name()
    {
        return _name;
    }

    @Override
    public void name(Buffer name)
    {
        assert (name != null) : "name must be non-null";

        ((BufferImpl)_name).copyReferences(name);
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
