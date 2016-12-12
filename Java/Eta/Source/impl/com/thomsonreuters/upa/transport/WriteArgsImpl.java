package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;

public class WriteArgsImpl implements WriteArgs
{
    int _priority;
    int _flags;
    int _bytesWritten;
    int _uncompressedBytesWritten;
    long _seqNum;

    @Override
    public void priority(int priority)
    {
        assert (priority != WritePriorities.HIGH 
                || priority != WritePriorities.MEDIUM 
                || priority != WritePriorities.LOW) : "priority is out of range. Refer to WritePriorities";

        _priority = priority;
    }

    @Override
    public int priority()
    {
        return _priority;
    }

    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= WriteFlags.MAX_VALUE) : "flags are out of range. Refer to WriteFlags";

        _flags = flags;
    }

    @Override
    public int flags()
    {
        return _flags;
    }

    public void bytesWritten(int bytesWritten)
    {
        _bytesWritten = bytesWritten;
    }

    @Override
    public int bytesWritten()
    {
        return _bytesWritten;
    }

    public void uncompressedBytesWritten(int uncompressedBytesWritten)
    {
        _uncompressedBytesWritten = uncompressedBytesWritten;
    }

    @Override
    public int uncompressedBytesWritten()
    {
        return _uncompressedBytesWritten;
    }

    @Override
    public void seqNum(long seqNum)
    {
        _seqNum = seqNum;
    }

    @Override
    public long seqNum()
    {
        return _seqNum;
    }

    @Override
    public void clear()
    {
        _priority = WritePriorities.HIGH;
        _flags = WriteFlags.NO_FLAGS;
        _bytesWritten = 0;
        _uncompressedBytesWritten = 0;
        _seqNum = 0;
    }

}
