/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.nio.channels.SocketChannel;

import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;

public class InProgInfoImpl implements InProgInfo
{
    private int _flags;
    private java.nio.channels.SocketChannel _oldScktChannel;
    private java.nio.channels.SelectableChannel _newScktChannel;
    private java.nio.channels.SelectableChannel _oldSelectableChannel;
    private java.nio.channels.SelectableChannel _newSelectableChannel;

    InProgInfoImpl()
    {
        _flags = InProgFlags.NONE;
    }
	
    @Override
    public String toString()
    {
        return "InProgInfo" + "\n" + 
               "\tflags: " + _flags + "\n" + 
               "\toldSelectableChannel: " + _oldSelectableChannel + "\n" +
               "\tnewSelectableChannel: " + _newSelectableChannel;
    }

    public void flags(int flags)
    {
        _flags = flags;
    }

    @Override
    public int flags()
    {
        return _flags;
    }

    public void oldScktChannel(SocketChannel oldScktChannel)
    {
        _oldScktChannel = oldScktChannel;
    }

    @Override @Deprecated
    public java.nio.channels.SocketChannel oldScktChannel()
    {
        return _oldScktChannel;
    }

    public void newScktChannel(java.nio.channels.SelectableChannel newScktChannel)
    {
        _newScktChannel = newScktChannel;
    }

    @Override @Deprecated
    public java.nio.channels.SelectableChannel newScktChannel()
    {
        return _newScktChannel;
    }

    public void oldSelectableChannel(java.nio.channels.SelectableChannel oldSelectableChannel)
    {
        _oldSelectableChannel = oldSelectableChannel;
    }

    @Override
    public java.nio.channels.SelectableChannel oldSelectableChannel()
    {
        return _oldSelectableChannel;
    }

    public void newSelectableChannel(java.nio.channels.SelectableChannel newSelectableChannel)
    {
        _newSelectableChannel = newSelectableChannel;
    }

    @Override
    public java.nio.channels.SelectableChannel newSelectableChannel()
    {
        return _newSelectableChannel;
    }

    @Override
    public void clear()
    {
        _flags = InProgFlags.NONE;
        _oldScktChannel = null;
        _newScktChannel = null;
        _oldSelectableChannel = null;
        _newSelectableChannel = null;
    }
}
