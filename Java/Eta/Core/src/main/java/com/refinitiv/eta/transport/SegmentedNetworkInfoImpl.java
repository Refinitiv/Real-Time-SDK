package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.SegmentedNetworkInfo;

class SegmentedNetworkInfoImpl implements SegmentedNetworkInfo
{
    private String _recvAddress;
    private String _recvServiceName;
    private String _unicastServiceName;
    private String _interfaceName;
    private String _sendAddress;
    private String _sendServiceName;

    SegmentedNetworkInfoImpl()
    {
    }

    /* Make a deep copy of this object to the specified object.
     * 
     * destSegmented is the destination object.
     */
    void copy(SegmentedNetworkInfoImpl destSegmented)
    {
        if (_recvAddress != null)
            destSegmented._recvAddress = new String(_recvAddress);
        else
            destSegmented._recvAddress = null;

        if (_recvServiceName != null)
            destSegmented._recvServiceName = new String(_recvServiceName);
        else
            destSegmented._recvServiceName = null;

        if (_unicastServiceName != null)
            destSegmented._unicastServiceName = new String(_unicastServiceName);
        else
            destSegmented._unicastServiceName = null;

        if (_interfaceName != null)
            destSegmented._interfaceName = new String(_interfaceName);
        else
            destSegmented._interfaceName = null;

        if (_sendAddress != null)
            destSegmented._sendAddress = new String(_sendAddress);
        else
            destSegmented._sendAddress = null;

        if (_sendServiceName != null)
            destSegmented._sendServiceName = new String(_sendServiceName);
        else
            destSegmented._sendServiceName = null;
    }

    @Override
    public String toString()
    {
        return "SegmentedNetworkInfo" + "\n" + 
               "\t\t\trecvAddress: " + _recvAddress + "\n" +
               "\t\t\trecvServiceName: " + _recvServiceName + "\n" + 
               "\t\t\tunicastServiceName: " + _unicastServiceName + "\n" + 
               "\t\t\tinterfaceName: " + _interfaceName + "\n" + 
               "\t\t\tsendAddress: " + _sendAddress + "\n" + 
               "\t\t\tsendServiceName: " + _sendServiceName;
    }

    @Override
    public SegmentedNetworkInfo recvAddress(String recvAddress)
    {
        assert (recvAddress != null) : "recvAddress must be non-null";

        _recvAddress = recvAddress;
        return this;
    }

    @Override
    public String recvAddress()
    {
        return _recvAddress;
    }

    @Override
    public SegmentedNetworkInfo recvServiceName(String recvServiceName)
    {
        assert (recvServiceName != null) : "recvServiceName must be non-null";

        _recvServiceName = recvServiceName;
        return this;
    }

    @Override
    public String recvServiceName()
    {
        return _recvServiceName;
    }

    @Override
    public SegmentedNetworkInfo unicastServiceName(String unicastServiceName)
    {
        assert (unicastServiceName != null) : "unicastServiceName must be non-null";

        _unicastServiceName = unicastServiceName;
        return this;
    }

    @Override
    public String unicastServiceName()
    {
        return _unicastServiceName;
    }

    @Override
    public SegmentedNetworkInfo interfaceName(String interfaceName)
    {
        assert (interfaceName != null) : "interfaceName must be non-null";

        _interfaceName = interfaceName;
        return this;
    }

    @Override
    public String interfaceName()
    {
        return _interfaceName;
    }

    @Override
    public SegmentedNetworkInfo sendAddress(String sendAddress)
    {
        assert (sendAddress != null) : "sendAddress must be non-null";

        _sendAddress = sendAddress;
        return this;
    }

    @Override
    public String sendAddress()
    {
        return _sendAddress;
    }

    @Override
    public SegmentedNetworkInfo sendServiceName(String sendServiceName)
    {
        assert (sendServiceName != null) : "sendServiceName must be non-null";

        _sendServiceName = sendServiceName;
        return this;
    }

    @Override
    public String sendServiceName()
    {
        return _sendServiceName;
    }

    void clear()
    {
        _interfaceName = null;
        _unicastServiceName = null;
        _recvAddress = null;
        _recvServiceName = null;
        _sendAddress = null;
        _sendServiceName = null;
    }

}
