package com.rtsdk.eta.transport;

import com.rtsdk.eta.transport.InitArgs;

class InitArgsImpl implements InitArgs
{
    boolean _globalLocking;

    @Override
    public void globalLocking(boolean globalLocking)
    {
        _globalLocking = globalLocking;
    }

    @Override
    public boolean globalLocking()
    {
        return _globalLocking;
    }

    @Override
    public void clear()
    {
        _globalLocking = false;
    }
}
