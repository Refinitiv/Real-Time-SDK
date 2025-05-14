/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.ServerInfo;

public class ServerInfoImpl implements ServerInfo
{
    private int _currentBufferUsage;
    private int _peakBufferUsage;

    ServerInfoImpl()
    {
    }

    @Override
    public String toString()
    {
        return "ServerInfo" + "\n" + 
               "\tcurrentBufferUsage: " + _currentBufferUsage + "\n" + 
               "\tpeakBufferUsage: " + _peakBufferUsage;
    }

    public void currentBufferUsage(int currentBufferUsage)
    {
        _currentBufferUsage = currentBufferUsage;
    }

    @Override
    public int currentBufferUsage()
    {
        return _currentBufferUsage;
    }

    public void peakBufferUsage(int peakBufferUsage)
    {
        _peakBufferUsage = peakBufferUsage;
    }

    @Override
    public int peakBufferUsage()
    {
        return _peakBufferUsage;
    }
    
    @Override
    public void clear()
    {
        _currentBufferUsage = 0;
        _peakBufferUsage = 0;
    }
}
