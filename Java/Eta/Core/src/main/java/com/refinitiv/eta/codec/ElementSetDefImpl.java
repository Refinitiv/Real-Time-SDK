/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.ElementSetDef;
import com.refinitiv.eta.codec.ElementSetDefEntry;

class ElementSetDefImpl implements ElementSetDef
{
    int                 _setId;
    int                 _count;
    ElementSetDefEntry _entries[];
	
    @Override
    public void clear()
    {
        _setId = 0;
        _count = 0;
        _entries = null;
    }

    @Override
    public int setId()
    {
        return _setId;
    }

    @Override
    public void setId(int setId)
    {
        assert (setId >= 0 && setId <= 32767) : "setId is out of range (0-32767)"; // 0x8000 (u15rb)

        _setId = setId;
    }

    @Override
    public int count()
    {
        return _count;
    }

    @Override
    public void count(int count)
    {
        assert (count >= 0 && count <= 255) : "count is out of range (0-255)"; // uint8

        _count = count;
    }

    @Override
    public ElementSetDefEntry[] entries()
    {
        return _entries;
    }

    @Override
    public void entries(ElementSetDefEntry entries[])
    {
        assert (entries != null) : "entries must be non-null";

        _entries = entries;
    }
	
    @Override
    public void copy(ElementSetDef elementSetDef)
    {
        ElementSetDefImpl newElementSetDef = (ElementSetDefImpl)elementSetDef;
        newElementSetDef._count = _count;
        newElementSetDef._entries = _entries;
        newElementSetDef.setId(_setId);
        elementSetDef = newElementSetDef;
    }
}
