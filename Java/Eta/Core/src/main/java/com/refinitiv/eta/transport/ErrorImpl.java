/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;

class ErrorImpl implements Error
{
    private Channel _channel;
    private int _errorId;
    private int _sysError;
    private String _text;

    // This class field will be used to bypass asserts when running junits.
    static boolean _runningInJunits = false;

    ErrorImpl()
    {
    }

    @Override
    public String toString()
    {
        return "Error" + "\n" + 
               "\tchannel: " + _channel + "\n" + 
               "\tErrorId: " + _errorId + "\n" + 
               "\tsysError: " + _sysError + "\n" + 
               "\ttext: " + _text;
    }

    @Override
    public void channel(Channel channel)
    {
        _channel = channel;
    }

    @Override
    public Channel channel()
    {
        return _channel;
    }

    @Override
    public void errorId(int errorId)
    {
        _errorId = errorId;
    }

    @Override
    public int errorId()
    {
        return _errorId;
    }

    @Override
    public void sysError(int sysError)
    {
        _sysError = sysError;
    }

    @Override
    public int sysError()
    {
        return _sysError;
    }

    @Override
    public void text(String text)
    {
        _text = ((text != null) ? text : "");
    }

    @Override
    public String text()
    {
        return _text;
    }

    @Override
    public void clear()
    {
        _channel = null;
        _errorId = 0;
        _sysError = 0;
        _text = null;
    }

}
