package com.rtsdk.eta.transport;

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
