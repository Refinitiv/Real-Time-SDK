package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.TcpOpts;

// This impl class contains the "replay" property, which may be used during debugging to specify a NetworkReplay input file.
// It must be temporarily made public (but not checked-in) to be accessible in "Consumer or Consumer Perf" code
class TcpOptsImpl implements TcpOpts
{
    private boolean _tcpNoDelay;
    private String _replayFilename = null;

    TcpOptsImpl()
    {
    }

    /* Make a deep copy of this object to the specified object.
     * 
     * destOpts is the destination object.
     */
    void copy(TcpOptsImpl destOpts)
    {
        destOpts._tcpNoDelay = _tcpNoDelay;
        if (_replayFilename != null)
            destOpts._replayFilename = new String(_replayFilename);
        else
            destOpts._replayFilename = null;
    }

    @Override
    public String toString()
    {
        return "TcpOpts" + "\n" + "\t\ttcp_nodelay: " + _tcpNoDelay;
    }

    @Override
    public void tcpNoDelay(boolean tcpNoDelay)
    {
        _tcpNoDelay = tcpNoDelay;
    }

    @Override
    public boolean tcpNoDelay()
    {
        return _tcpNoDelay;
    }

    /* Used only for debugging.
     * Returns the name of the NetworkReplay file that should be used instead of a real data source.
     * Returns null if a real data source should be used.
     */
    String replay()
    {
        return _replayFilename;
    }

    /* Used only for debugging.
     * Returns the name of the NetworkReplay file that should be used instead of a real data source.
     * Specify null if a real data source should be used.
     * 
     * replayFilename is the name of the NetworkReplay file, or null if a real data source should be used
     */
    void replay(String replayFilename)
    {
        _replayFilename = replayFilename;
    }
}
