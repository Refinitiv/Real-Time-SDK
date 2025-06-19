/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

class RsslEncryptedSocketChannel extends RsslHttpSocketChannel
{
    RsslEncryptedSocketChannel(SocketProtocol transport, Pool channelPool)
    {
        super(transport, channelPool, true);
    }

    @Override
    public int connectionType()
    {
        return ConnectionTypes.ENCRYPTED;
    }
}
