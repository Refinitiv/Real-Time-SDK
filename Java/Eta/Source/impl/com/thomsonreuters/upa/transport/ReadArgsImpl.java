package com.thomsonreuters.upa.transport;

import java.net.InetSocketAddress;
import java.net.SocketAddress;

import com.thomsonreuters.upa.transport.ReadArgs;

public class ReadArgsImpl implements ReadArgs
{
    int _readRetVal;
    int _bytesRead;
    int _uncompressedBytesRead;
    long _seqNum;
    SocketAddress _senderAddress;
    int _instanceId;
    int _flags;

    public void readRetVal(int readRetVal)
    {
        _readRetVal = readRetVal;
    }

    @Override
    public int readRetVal()
    {
        return _readRetVal;
    }

    @Override
    public void clear()
    {
        _readRetVal = TransportReturnCodes.SUCCESS;
        _bytesRead = 0;
        _uncompressedBytesRead = 0;
        _instanceId = 0;
    }

    @Override
    public int bytesRead()
    {
        return _bytesRead;
    }

    public void bytesRead(int bytesRead)
    {
        _bytesRead = bytesRead;
    }

    @Override
    public int uncompressedBytesRead()
    {
        return _uncompressedBytesRead;
    }

    public void uncompressedBytesRead(int compressedBytesRead)
    {
        _uncompressedBytesRead = compressedBytesRead;
    }

    public long seqNum()
    {
        return _seqNum;
    }

    public void seqNum(long seqNum)
    {
        _seqNum = seqNum;
    }

    public void senderAddress(SocketAddress senderAddress)
    {
        _senderAddress = senderAddress;
    }

    public String senderAddress()
    {
        if (_senderAddress != null)
            return ((InetSocketAddress)_senderAddress).getHostString();
        else
            return null;
    }

    public int senderPort()
    {
        if (_senderAddress != null)
            return ((InetSocketAddress)_senderAddress).getPort();
        else
            return TransportReturnCodes.FAILURE;
    }

    public int instanceId()
    {
        return _instanceId;
    }

    public void instanceId(int instanceId)
    {
        _instanceId = instanceId;
    }

    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= ReadFlags.MAX_VALUE) : "flags are out of range. Refer to ReadFlags";

        _flags = flags;
    }

    @Override
    public int flags()
    {
        return _flags;
    }
}
