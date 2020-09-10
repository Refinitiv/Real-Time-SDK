package com.rtsdk.eta.transport;

class ShmemOptsImpl implements ShmemOpts
{
    private long _maxReaderLag;

    /* Make a deep copy of this object object to the specified object.
     * 
     * destOpts is the destination object.
     */
    void copy(ShmemOptsImpl destOpts)
    {
        destOpts._maxReaderLag = _maxReaderLag;
    }

    @Override
    public String toString()
    {
        return "ShmemOpts" + "\n" + "\t\tmaxReaderLag: " + _maxReaderLag;
    }

    @Override
    public void maxReaderLag(long maxReaderLag)
    {
        _maxReaderLag = maxReaderLag;
    }

    @Override
    public long maxReaderLag()
    {
        return _maxReaderLag;
    }
}
