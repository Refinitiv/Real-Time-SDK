/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

class EtaNode
{
    EtaNode _next = null;
    Pool _pool = null;
    boolean _inPool = false;

    EtaNode next()
    {
        return _next;
    }

    void next(EtaNode next)
    {
        _next = next;
    }

    Pool pool()
    {
        return _pool;
    }

    void pool(Pool pool)
    {
        _pool = pool;
    }

    void returnToPool()
    {
        if (!_inPool)
            _pool.add(this);
    }

}
