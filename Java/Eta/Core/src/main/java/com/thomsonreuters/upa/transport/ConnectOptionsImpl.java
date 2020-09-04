package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.TcpOpts;
import com.thomsonreuters.upa.transport.UnifiedNetworkInfo;

class ConnectOptionsImpl implements ConnectOptions
{
    private String _componentVersion;
    private int _connectionType;
    private int _compressionType;
    private boolean _blocking;
    private int _pingTimeout;
    private int _guaranteedOutputBuffers;
    private int _numInputBuffers;
    private int _majorVersion;
    private int _minorVersion;
    private int _protocolType;
    private Object _userSpecObject;
    private UnifiedNetworkInfoImpl _unified = new UnifiedNetworkInfoImpl();
    private SegmentedNetworkInfoImpl _segmented = new SegmentedNetworkInfoImpl();
    private TunnelingInfoImpl _tunneling = new TunnelingInfoImpl();
    private EncryptionOptionsImpl _encryptionOpts = new EncryptionOptionsImpl();
    private CredentialsInfoImpl _credentials = new CredentialsInfoImpl();
    private int _networkType;
    private TcpOptsImpl _tcpOpts = new TcpOptsImpl();
    private MCastOptsImpl _mcastOpts = new MCastOptsImpl();
    private ShmemOptsImpl _shmemOpts = new ShmemOptsImpl();
    private boolean _readLocking;
    private boolean _writeLocking;
    private int _sysSendBufSize;
    private int _sysRecvBufSize;
    private SeqMCastOptsImpl _seqMCastOpts = new SeqMCastOptsImpl();


    
    ConnectOptionsImpl()
    {
        _compressionType = Ripc.CompressionTypes.NONE;
        _connectionType = ConnectionTypes.SOCKET;
        _pingTimeout = 60;
        _guaranteedOutputBuffers = 50;
        _numInputBuffers = 10;
        _mcastOpts.packetTTL(5);
        _seqMCastOpts.maxMsgSize(3000);
        _seqMCastOpts.instanceId(0);

    }
	
    @Override
    public void clear()
    {
        _componentVersion = null;
        _connectionType = ConnectionTypes.SOCKET;
        _compressionType = Ripc.CompressionTypes.NONE;
        _blocking = false;
        _pingTimeout = 60;
        _guaranteedOutputBuffers = 50;
        _numInputBuffers = 10;
        _majorVersion = 0;
        _minorVersion = 0;
        _protocolType = 0;
        _userSpecObject = null;
        _tcpOpts.tcpNoDelay(false);
        _mcastOpts.disconnectOnGaps(false);
        _mcastOpts.packetTTL(5);
        _mcastOpts.tcpControlPort();
        _mcastOpts.portRoamRange(0);
        _shmemOpts.maxReaderLag(0);
        _networkType = 0;
        _unified.clear();
        _segmented.clear();
        _sysSendBufSize = 0;
        _sysRecvBufSize = 0;
        _readLocking = false;
        _writeLocking = false;
        _seqMCastOpts.maxMsgSize(3000);
        _seqMCastOpts.instanceId(0);
        _tunneling.clear();
        _encryptionOpts.clear();
        _credentials.clear();
    }

    @Override
    public int copy(ConnectOptions destOpts)
    {
        if (destOpts == null)
            return TransportReturnCodes.FAILURE;

        ConnectOptionsImpl destOptsImpl = (ConnectOptionsImpl)destOpts;

        if (_componentVersion != null)
            destOptsImpl._componentVersion = new String(_componentVersion);
        else
            destOptsImpl._componentVersion = null;
        
        destOptsImpl._connectionType = _connectionType;
        destOptsImpl._compressionType = _compressionType;
        destOptsImpl._blocking = _blocking;
        destOptsImpl._pingTimeout = _pingTimeout;
        destOptsImpl._guaranteedOutputBuffers = _guaranteedOutputBuffers;
        destOptsImpl._numInputBuffers = _numInputBuffers;
        destOptsImpl._majorVersion = _majorVersion;
        destOptsImpl._minorVersion = _minorVersion;
        destOptsImpl._protocolType = _protocolType;
        destOptsImpl._userSpecObject = _userSpecObject;
        _unified.copy(destOptsImpl._unified);
        _segmented.copy(destOptsImpl._segmented);
        _tunneling.copy(destOptsImpl._tunneling);
        _credentials.copy(destOptsImpl._credentials);
        destOptsImpl._networkType = _networkType;
        _tcpOpts.copy(destOptsImpl._tcpOpts);
        _mcastOpts.copy(destOptsImpl._mcastOpts);
        _shmemOpts.copy(destOptsImpl._shmemOpts);
        destOptsImpl._readLocking = _readLocking;
        destOptsImpl._writeLocking = _writeLocking;
        destOptsImpl._sysSendBufSize = _sysSendBufSize;
        destOptsImpl._sysRecvBufSize = _sysRecvBufSize;
        _seqMCastOpts.copy(destOptsImpl._seqMCastOpts);
        _encryptionOpts.copy(destOptsImpl._encryptionOpts);

        return TransportReturnCodes.SUCCESS;
    }
    
    @Override
    public String toString()
    {
        return "ConnectOptions" + "\n" + 
               "\tcomponentVersion: " + _componentVersion + "\n" +
               "\tconnectionType: " + _connectionType + "\n" + 
               "\tcompressionType: " + _compressionType + "\n" + 
               "\tblocking: " + _blocking + "\n" + 
               "\tpingTimeout: " + _pingTimeout + "\n" + 
               "\tguaranteedOutputBuffers: " + _guaranteedOutputBuffers + "\n" + 
               "\tnumInputBuffers: " + _numInputBuffers + "\n" + 
               "\tsysSendBufSize: " + _sysSendBufSize + "\n" + 
               "\tsysRecvBufSize: " + _sysRecvBufSize + "\n" + 
               "\tmajorVersion: " + _majorVersion + "\n" + 
               "\tminorVersion: " + _minorVersion + "\n" + 
               "\tprotocolType: " + _protocolType + "\n" + 
               "\tuserSpecObject: " + _userSpecObject + "\n" + 
               "\tnetworkType: " + _networkType + "\n" + 
               "\ttcpOpts: " + _tcpOpts + "\n" + 
               "\tunifiedNetworkInfo: " + _unified + "\n" + 
               "\tsegmentedNetworkInfo: " + _segmented + 
               "\tseqMCastOpts: " + _seqMCastOpts;
    }

    @Override
    public void componentVersion(String componentVersion)
    {
        _componentVersion = componentVersion;
    }

    @Override
    public String componentVersion()
    {
        return _componentVersion;
    }       
    
    @Override
    public void connectionType(int connectionType)
    {
        assert (connectionType >= 0 && connectionType <= ConnectionTypes.MAX_DEFINED) : "connectionType is out of range. Refer to ConnectionTypes";

        _connectionType = connectionType;
    }

    @Override
    public int connectionType()
    {
        return _connectionType;
    }

    @Override
    public void compressionType(int compressionType)
    {
        assert (compressionType >= 0 && compressionType <= Ripc.CompressionTypes.MAX_DEFINED) : "compressionType is out of range. Refer to CompTypes";

        _compressionType = compressionType;
    }

    @Override
    public int compressionType()
    {
        return _compressionType;
    }

    @Override
    public void blocking(boolean blocking)
    {
        _blocking = blocking;
    }

    @Override
    public boolean blocking()
    {
        return _blocking;
    }

    @Override
    public void pingTimeout(int pingTimeout)
    {
        // If assertions disabled, ignore out-of-range value and keep existing value
        if (pingTimeout == 0)
            _pingTimeout = 1;
        else if (pingTimeout > 255)
            _pingTimeout = 255;
        else
            _pingTimeout = pingTimeout;
    }

    @Override
    public int pingTimeout()
    {
        return _pingTimeout;
    }

    @Override
    public void guaranteedOutputBuffers(int guaranteedOutputBuffers)
    {
        if (guaranteedOutputBuffers <= 5)
            _guaranteedOutputBuffers = 5;
        else
            _guaranteedOutputBuffers = guaranteedOutputBuffers;
    }

    @Override
    public int guaranteedOutputBuffers()
    {
        return _guaranteedOutputBuffers;
    }

    @Override
    public void numInputBuffers(int numInputBuffers)
    {
        assert (numInputBuffers > 0) : "numInputBuffers must be greater than zero";

        _numInputBuffers = numInputBuffers;
    }

    @Override
    public int numInputBuffers()
    {
        return _numInputBuffers;
    }

    @Override
    public void majorVersion(int majorVersion)
    {
        _majorVersion = majorVersion;
    }

    @Override
    public int majorVersion()
    {
        return _majorVersion;
    }

    @Override
    public void minorVersion(int minorVersion)
    {
        _minorVersion = minorVersion;
    }

    @Override
    public int minorVersion()
    {
        return _minorVersion;
    }

    @Override
    public void protocolType(int protocolType)
    {
        assert (protocolType >= 0) : "protocolType must be positive";

        _protocolType = protocolType;
    }

    @Override
    public int protocolType()
    {
        return _protocolType;
    }

    @Override
    public void userSpecObject(Object userSpecObject)
    {
        assert (userSpecObject != null) : "userSpecObject must be non-null";

        _userSpecObject = userSpecObject;
    }

    @Override
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    @Override
    public TcpOpts tcpOpts()
    {
        return _tcpOpts;
    }

    @Override
    public MCastOpts multicastOpts()
    {
        return _mcastOpts;
    }

    @Override
    public ShmemOpts shmemOpts()
    {
        return _shmemOpts;
    }

    @Override
    public UnifiedNetworkInfo unifiedNetworkInfo()
    {
        return _unified;
    }

    @Override
    public SegmentedNetworkInfo segmentedNetworkInfo()
    {
        return _segmented;
    }

    @Override
    public TunnelingInfo tunnelingInfo()
    {
        return _tunneling;
    }

    @Override
    public CredentialsInfo credentialsInfo()
    {
        return _credentials;
    }
    
    @Override
    public EncryptionOptions encryptionOptions()
    {
        return _encryptionOpts;
    }

    @Override
    public void channelReadLocking(boolean locking)
    {
        _readLocking = locking;
    }

    @Override
    public boolean channelReadLocking()
    {
        return _readLocking;
    }

    @Override
    public void channelWriteLocking(boolean locking)
    {
        _writeLocking = locking;
    }

    @Override
    public boolean channelWriteLocking()
    {
        return _writeLocking;
    }   
    
    @Override
    public void sysSendBufSize(int sysSendBufSize)
    {
        assert (sysSendBufSize >= 0) : "sysSendBufSize must be greater than or equal to 0";

        _sysSendBufSize = sysSendBufSize;
    }

    @Override
    public int sysSendBufSize()
    {
        return _sysSendBufSize;
    }

    @Override
    public void sysRecvBufSize(int sysRecvBufSize)
    {
        assert (sysRecvBufSize >= 0) : "sysRecvBufSize must be greater than or equal to 0";

        _sysRecvBufSize = sysRecvBufSize;
    }

    @Override
    public int sysRecvBufSize()
    {
        return _sysRecvBufSize;
    }

    @Override
    public SeqMCastOpts seqMCastOpts()
    {
        return _seqMCastOpts;
    }
}
