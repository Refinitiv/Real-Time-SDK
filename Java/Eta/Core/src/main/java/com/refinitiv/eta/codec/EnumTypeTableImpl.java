/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.EnumType;
import com.refinitiv.eta.codec.EnumTypeTable;

class EnumTypeTableImpl implements EnumTypeTable
{
    int           _maxValue;
    EnumType[]    _enumTypes;
    int           _fidReferenceCount;
    int[]         _fidReferences;
	
    @Override
    public int maxValue()
    {
        return _maxValue;
    }
	
    void maxValue(int maxValue)
    {
        assert (maxValue >= 0 && maxValue <= 65535) : "maxValue is out of range (0-65535)"; // uint16

        _maxValue = maxValue;
    }
	
    @Override
    public EnumType[] enumTypes()
    {
        return _enumTypes;
    }
	
    void enumTypes(EnumType[] enumTypes)
    {
        assert (enumTypes != null) : "enumTypes must be non-null";

        _enumTypes = enumTypes;
    }
	
    @Override
    public int fidReferenceCount()
    {
        return _fidReferenceCount;
    }
	
    void fidReferenceCount(int fidReferenceCount)
    {
        assert (fidReferenceCount >= 0 && fidReferenceCount <= 65535) : "fidReferenceCount is out of range (0-65535)"; // uint16

        _fidReferenceCount = fidReferenceCount;
    }
	
    @Override
    public int[] fidReferences()
    {
        return _fidReferences;
    }
	
    void fidReferences(int[] fidReferences)
    {
        assert (fidReferences != null) : "fidReferences must be non-null)";

        _fidReferences = fidReferences;
    }
}
