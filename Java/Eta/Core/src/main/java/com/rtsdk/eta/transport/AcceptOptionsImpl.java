package com.rtsdk.eta.transport;

import com.rtsdk.eta.transport.AcceptOptions;

class AcceptOptionsImpl implements AcceptOptions
{
    private boolean _nakMount;
    private Object _userSpecObject;
    private boolean _readLocking;
    private boolean _writeLocking;
    private int _sysSendBufSize;

    AcceptOptionsImpl()
    {
    }

    public String toString()
    {
        return "AcceptOptions" + "\n" + 
               "\tnakMount: " + _nakMount + "\n" + 
               "\tsysSendBufSize: " + _sysSendBufSize + "\n" + 
               "\tuserSpecObject: " + _userSpecObject;
    }

    @Override
    public void clear()
    {
        _nakMount = false;
        _userSpecObject = null;
        _sysSendBufSize = 0;
    }

    @Override
    public void nakMount(boolean nakMount)
    {
        _nakMount = nakMount;
    }

    @Override
    public boolean nakMount()
    {
        return _nakMount;
    }

    @Override
    public void userSpecObject(Object userSpecObject)
    {
        assert (userSpecObject != null) : "userSpecObject must be non-null";

        _userSpecObject = userSpecObject;
    }

    @Override
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    @Override
    public void channelReadLocking(boolean locking)
    {
        _readLocking = locking;
    }

    @Override
    public boolean channelReadLocking()
    {
        return _readLocking;
    }

    @Override
    public void channelWriteLocking(boolean locking)
    {
        _writeLocking = locking;
    }

    @Override
    public boolean channelWriteLocking()
    {
        return _writeLocking;
    }

    @Override
    public void sysSendBufSize(int sysSendBufSize)
    {
        assert (sysSendBufSize >= 0) : "sysSendBufSize must be greater than or equal to 0";

        _sysSendBufSize = sysSendBufSize;
    }

    @Override
    public int sysSendBufSize()
    {
        return _sysSendBufSize;
    }
}