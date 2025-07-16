/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.InitArgs;

class InitArgsImpl implements InitArgs
{
    boolean _globalLocking;
    int _socketProtocolPoolLimit;

    InitArgsImpl()
    {
        clear();
    }

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
    public void socketProtocolPoolLimit(int socketProtocolPoolLimit)
    {
        _socketProtocolPoolLimit = socketProtocolPoolLimit;
    }

    @Override
    public int socketProtocolPoolLimit()
    {
        return _socketProtocolPoolLimit;
    }

    @Override
    public void clear()
    {
        _globalLocking = false;
        _socketProtocolPoolLimit = -1;
    }
}
