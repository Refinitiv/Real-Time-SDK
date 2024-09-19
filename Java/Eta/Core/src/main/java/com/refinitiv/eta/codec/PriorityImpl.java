/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

class PriorityImpl implements Priority
{
    int _class;
    int _count;
	
    @Override
    public void clear()
    {
        _class = 0;
        _count = 0;
    }

    @Override
    public void priorityClass(int priorityClass)
    {
        assert (priorityClass >= 0 && priorityClass <= 255) : "priorityClass is out of range (0-255)"; // uint8

        _class = priorityClass;
    }

    @Override
    public int priorityClass()
    {
        return _class;
    }

    @Override
    public void count(int count)
    {
        assert (count >= 0 && count <= 65535) : "priorityCount is out of range (0-65535)"; // uint16

        _count = count;
    }

    @Override
    public int count()
    {
        return _count;
    }
}
