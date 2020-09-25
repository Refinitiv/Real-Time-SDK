package com.refinitiv.eta.transport;

import java.util.List;

public class ChannelInfoImpl implements ChannelInfo
{
    int _maxFragmentSize;
    int _maxOutputBuffers;
    int _guaranteedOutputBuffers;
    int _numInputBuffers;
    int _pingTimeout;
    boolean _clientToServerPings;
    boolean _serverToClientPings;
    int _sysSendBufSize;
    int _sysRecvBufSize;
    int _compressionType;
    int _compressionThreshold;
    String _priorityFlushStrategy;
    String _clientIP;
    String _clientHostname;
    MCastStats _multicastStats = new MCastStatsImpl();
	
    /* Hold list of received Component Info during RIPC handshake */
    List<ComponentInfo> _receivedComponentInfoList;

    ChannelInfoImpl()
    {
    }
	
    @Override
    public String toString()
    {
        // concatenate the _receivedComponentInfo version(s).
        StringBuilder sb = new StringBuilder();
        if (_receivedComponentInfoList != null)
        {
            sb.append("\n");
            int ciIndex = 0;
            for (ComponentInfo ci : _receivedComponentInfoList)
            {
                if (ci.componentVersion() != null)
                {
                    sb.append("\t\tComponentInfo[");
                    sb.append(ciIndex);
                    sb.append("]: ");
                    sb.append(ci.componentVersion().toString());
                    sb.append("\n");
                    ++ciIndex;
                }
            }
        }

        return "ChannelInfo" + "\n" + 
               "\tmaxFragmentSize: " + _maxFragmentSize + "\n" + 
               "\tmaxOutputBuffers: " + _maxOutputBuffers + "\n" + 
               "\tguaranteedOutputBuffers: " + _guaranteedOutputBuffers + "\n" + 
               "\tnumInputBuffers: " + _numInputBuffers + "\n" +
               "\tpingTimeout: " + _pingTimeout + "\n" + 
               "\tclientToServerPings: " + _clientToServerPings + "\n" + 
               "\tserverToClientPings: " + _serverToClientPings + "\n" + 
               "\tsysSendBufSize: " + _sysSendBufSize + "\n" + 
               "\tsysRecvBufSize: " + _sysRecvBufSize + "\n" + 
               "\tcompressionType: " + _compressionType + "\n" + 
               "\tcompressionThreshold: " + _compressionThreshold + "\n" + 
               "\tpriorityFlushStrategy: " + _priorityFlushStrategy + "\n" + 
               "\tclientIP: " + _clientIP + "\n" + 
               "\tclientHostname: " + _clientHostname + "\n" +
               "\tmulticastStats: " + _multicastStats.toString() + "\n" + 
               "\tComponentInfo: " + sb.toString();
    }

    public void maxFragmentSize(int maxFragmentSize)
    {
        _maxFragmentSize = maxFragmentSize;
    }

    @Override
    public int maxFragmentSize()
    {
        return _maxFragmentSize;
    }

    public void maxOutputBuffers(int maxOutputBuffers)
    {
        _maxOutputBuffers = maxOutputBuffers;
    }

    @Override
    public int maxOutputBuffers()
    {
        return _maxOutputBuffers;
    }

    public void guaranteedOutputBuffers(int guaranteedOutputBuffers)
    {
        _guaranteedOutputBuffers = guaranteedOutputBuffers;
    }

    @Override
    public int guaranteedOutputBuffers()
    {
        return _guaranteedOutputBuffers;
    }

    public void numInputBuffers(int numInputBuffers)
    {
        _numInputBuffers = numInputBuffers;
    }

    @Override
    public int numInputBuffers()
    {
        return _numInputBuffers;
    }

    public void pingTimeout(int pingTimeout)
    {
        _pingTimeout = pingTimeout;
    }

    @Override
    public int pingTimeout()
    {
        return _pingTimeout;
    }

    public void clientToServerPings(boolean clientToServerPings)
    {
        _clientToServerPings = clientToServerPings;
    }

    @Override
    public boolean clientToServerPings()
    {
        return _clientToServerPings;
    }

    public void serverToClientPings(boolean serverToClientPings)
    {
        _serverToClientPings = serverToClientPings;
    }

    @Override
    public boolean serverToClientPings()
    {
        return _serverToClientPings;
    }

    public void sysSendBufSize(int sysSendBufSize)
    {
        _sysSendBufSize = sysSendBufSize;
    }

    @Override
    public int sysSendBufSize()
    {
        return _sysSendBufSize;
    }

    public void sysRecvBufSize(int sysRecvBufSize)
    {
        _sysRecvBufSize = sysRecvBufSize;
    }

    @Override
    public int sysRecvBufSize()
    {
        return _sysRecvBufSize;
    }

    public void compressionType(int compressionType)
    {
        _compressionType = compressionType;
    }

    @Override
    public int compressionType()
    {
        return _compressionType;
    }

    public void compressionThreshold(int compressionThreshold)
    {
        _compressionThreshold = compressionThreshold;
    }

    @Override
    public int compressionThreshold()
    {
        return _compressionThreshold;
    }

    public void priorityFlushStrategy(String priorityFlushStrategy)
    {
        _priorityFlushStrategy = priorityFlushStrategy;
    }

    @Override
    public String priorityFlushStrategy()
    {
        return _priorityFlushStrategy;
    }

    @Override
    public void clear()
    {
        _maxOutputBuffers = 0;
        _guaranteedOutputBuffers = 0;
        _numInputBuffers = 0;
        _pingTimeout = 0;
        _clientToServerPings = false;
        _serverToClientPings = false;
        _sysSendBufSize = 0;
        _sysRecvBufSize = 0;
        _compressionType = 0;
        _compressionThreshold = 0;
        _clientIP = null;
        _clientHostname = null;
    }

    @Override
    public List<ComponentInfo> componentInfo()
    {
        return _receivedComponentInfoList;
    }

    @Override
    public String clientIP()
    {
        return _clientIP;
    }

    public void clientIP(String clientIP)
    {
        _clientIP = clientIP;
    }

    @Override
    public String clientHostname()
    {
        return _clientHostname;
    }

    public void clientHostname(String clientHostname)
    {
        _clientHostname = clientHostname;
    }

    @Override
    public MCastStats multicastStats()
    {
        return _multicastStats;
    }

    public void multicastStats(MCastStats multicastStats)
    {
        ((MCastStatsImpl)_multicastStats).mcastSent(multicastStats.mcastSent());
        ((MCastStatsImpl)_multicastStats).mcastRcvd(multicastStats.mcastRcvd());
        ((MCastStatsImpl)_multicastStats).gapsDetected(multicastStats.gapsDetected());
        ((MCastStatsImpl)_multicastStats).retransPktsRcvd(multicastStats.retransPktsRcvd());
        ((MCastStatsImpl)_multicastStats).retransPktsSent(multicastStats.retransPktsSent());
        ((MCastStatsImpl)_multicastStats).retransReqRcvd(multicastStats.retransReqRcvd());
        ((MCastStatsImpl)_multicastStats).retransReqSent(multicastStats.retransReqSent());
        ((MCastStatsImpl)_multicastStats).unicastSent(multicastStats.unicastSent());
    }

    /* used by JNI to set componentInfo from UPAC */
    public void addCompVersionString(String compVersionStr)
    {
        ComponentInfo ci = new ComponentInfoImpl();
        ci.componentVersion().data(compVersionStr);
        _receivedComponentInfoList.add(ci);
    }
}
