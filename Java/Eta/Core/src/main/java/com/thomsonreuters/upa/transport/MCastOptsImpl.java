package com.thomsonreuters.upa.transport;

class MCastOptsImpl implements MCastOpts
{
    private boolean _disconnectOnGaps;
    private int _packetTTL;

    private String      _tcpControlPort;
    private int         _portRoamRange;

    /* make a deep copy of this object to the specified object.
     * 
     * destOpts is the destination object.
     */
    void copy(MCastOptsImpl destOpts)
    {
        destOpts._disconnectOnGaps = _disconnectOnGaps;
        destOpts._packetTTL = _packetTTL;

        if (_tcpControlPort != null)
            destOpts._tcpControlPort = new String(_tcpControlPort);
        else
            destOpts._tcpControlPort = null;

        destOpts._portRoamRange = _portRoamRange;
    }

    public String toString()
    {
        return "MCastOpts" + "\n" + 
               "\t\tdisconnectOnGaps: " + _disconnectOnGaps + 
               "\t\tpacketTTL: " + _packetTTL + 
               "\t\ttcpControlPort: " + _tcpControlPort + 
               "\t\tportRoamRange: " + _portRoamRange;
    }

    @Override
    public void disconnectOnGaps(boolean disconnectOnGaps)
    {
        _disconnectOnGaps = disconnectOnGaps;
    }

    @Override
    public boolean disconnectOnGaps()
    {
        return _disconnectOnGaps;
    }

    @Override
    public void packetTTL(int packetTTL)
    {
        _packetTTL = packetTTL;
    }

    @Override
    public int packetTTL()
    {
        return _packetTTL;
    }

    @Override
    public void tcpControlPort(String tcpControlPort)
    {
        _tcpControlPort = tcpControlPort;
    }

    @Override
    public String tcpControlPort()
    {
        return _tcpControlPort;
    }

    @Override
    public void portRoamRange(int portRoamRange)
    {
        _portRoamRange = portRoamRange;
    }

    @Override
    public int portRoamRange()
    {
        return _portRoamRange;
    }
}
