package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.InitArgs;

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
