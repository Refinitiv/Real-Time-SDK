/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

class ElementSetDefDbImpl implements ElementSetDefDb
{
    /* Maximum local message scope set identifier */
    int MAX_LOCAL_ID;
    
    /* Blank set identifier */
    final static int BLANK_ID = 65536;

    ElementSetDefImpl[] _definitions;

    /* Maximum set definition index */
    int maxSetId = 0;
    
    int maxLocalId = MAX_LOCAL_ID;
    
    ElementSetDefDbImpl(int maxID)
    {
        MAX_LOCAL_ID = maxID;
        _definitions = new ElementSetDefImpl[MAX_LOCAL_ID + 1];
        maxSetId = 0;
    }

    @Override
    public void clear()
    {
        for (int i = 0; i < maxLocalId; ++i)
        {
            _definitions[i]._setId = BLANK_ID;
        }

        maxSetId = 0;
    }

    @Override
    public ElementSetDef[] definitions()
    {
        return _definitions;
    }

    @Override
    public int maxSetId()
    {
        return maxSetId;
    }
    
    @Override
    public void maxSetId(int setMaxSetId)
    {
        maxSetId = setMaxSetId;
    }
    
}
