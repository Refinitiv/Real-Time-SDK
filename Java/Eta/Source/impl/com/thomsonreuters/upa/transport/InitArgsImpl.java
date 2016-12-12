package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.InitArgs;

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
