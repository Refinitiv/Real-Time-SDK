package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.TcpOpts;

class BindOptionsImpl implements BindOptions
{
    // This class field will be used to bypass asserts when running junits.
    static boolean _runningInJunits = false;

    private String _componentVersion;
    private String _serviceName;
    private int _port;
    private String _interfaceName;
    private int _compressionType;
    private int _compressionLevel;
    private boolean _forceCompression;
    private boolean _serverBlocking;
    private boolean _channelsBlocking;
    private boolean _serverToClientPings;
    private boolean _clientToServerPings;
    private int _connectionType;
    private int _pingTimeout;
    private int _minPingTimeout;
    private int _maxFragmentSize;
    private int _maxOutputBuffers;
    private int _guaranteedOutputBuffers;
    private int _numInputBuffers;
    int _sharedPoolSize;
    private boolean _sharedPoolLock;
    private int _majorVersion;
    private int _minorVersion;
    private int _protocolType;
    private Object _userSpecObject;
    private TcpOptsImpl _tcpOpts = new TcpOptsImpl();
    private int _sysRecvBufSize;
    private String _groupAddress;
    private ServerEncryptionOptionsImpl _encryptionOptions = new ServerEncryptionOptionsImpl();
    private WSocketOptsImpl _wsocketOpts = new WSocketOptsImpl();

    BindOptionsImpl()
    {
        _compressionType = Ripc.CompressionTypes.NONE;
        _serverToClientPings = true;
        _clientToServerPings = true;
        _connectionType = ConnectionTypes.SOCKET;
        _pingTimeout = 60;
        _minPingTimeout = 20;
        _maxFragmentSize = 6144;
        _maxOutputBuffers = 50;
        _guaranteedOutputBuffers = 50;
        _numInputBuffers = 10;
        _tcpOpts = new TcpOptsImpl();
        _encryptionOptions = new ServerEncryptionOptionsImpl();
    }

    void copyTo(BindOptionsImpl copyTo)
    {
        copyTo._componentVersion = _componentVersion;
        copyTo._serviceName = _serviceName;
        copyTo._interfaceName = _interfaceName;
        copyTo._compressionType = _compressionType;
        copyTo._compressionLevel = _compressionLevel;
        copyTo._forceCompression = _forceCompression;
        copyTo._serverBlocking = _serverBlocking;
        copyTo._channelsBlocking = _channelsBlocking;
        copyTo._serverToClientPings = _serverToClientPings;
        copyTo._clientToServerPings = _clientToServerPings;
        copyTo._connectionType = _connectionType;
        copyTo._pingTimeout = _pingTimeout;
        copyTo._minPingTimeout = _minPingTimeout;
        copyTo._maxFragmentSize = _maxFragmentSize;
        copyTo._maxOutputBuffers = _maxOutputBuffers;
        copyTo._guaranteedOutputBuffers = _guaranteedOutputBuffers;
        copyTo._numInputBuffers = _numInputBuffers;
        copyTo._sharedPoolSize = _sharedPoolSize;
        copyTo._sharedPoolLock = _sharedPoolLock;
        copyTo._majorVersion = _majorVersion;
        copyTo._minorVersion = _minorVersion;
        copyTo._protocolType = _protocolType;
        copyTo._userSpecObject = _userSpecObject;
        copyTo._port = _port;
        copyTo.tcpOpts().tcpNoDelay(_tcpOpts.tcpNoDelay());
        copyTo._sysRecvBufSize = _sysRecvBufSize;
        copyTo._groupAddress = _groupAddress;
        _encryptionOptions.copy(copyTo._encryptionOptions);
        _wsocketOpts.copy(copyTo._wsocketOpts);
    }

    @Override
    public void clear()
    {
        _componentVersion = null;
        _serviceName = null;
        _interfaceName = null;
        _compressionType = Ripc.CompressionTypes.NONE;
        _compressionLevel = 0;
        _forceCompression = false;
        _serverBlocking = false;
        _channelsBlocking = false;
        _serverToClientPings = true;
        _clientToServerPings = true;
        _connectionType = ConnectionTypes.SOCKET;
        _pingTimeout = 60;
        _minPingTimeout = 20;
        _maxFragmentSize = 6144;
        _maxOutputBuffers = 50;
        _guaranteedOutputBuffers = 50;
        _numInputBuffers = 10;
        _sharedPoolSize = 0;
        _sharedPoolLock = false;
        _majorVersion = 0;
        _minorVersion = 0;
        _protocolType = 0;
        _userSpecObject = null;
        _tcpOpts.tcpNoDelay(false);
        _sysRecvBufSize = 0;
        _groupAddress = null;
        _wsocketOpts.protocols("");
        _wsocketOpts.maxMsgSize(61440);
        _encryptionOptions.clear();
    }

    @Override
    public String toString()
    {
        return "BindOptions" + "\n" +
               "\tcomponentVersion: " + _componentVersion + "\n" +
               "\tserviceName: " + _serviceName + "\n" +
               "\tinterfaceName: " + _interfaceName + "\n" +
               "\tcompressionType: " + _compressionType + "\n" +
               "\tinterfaceName: " + _interfaceName + "\n" +
               "\tcompressionType: " + _compressionType + "\n" +
               "\tcompressionLevel: " + _compressionLevel + "\n" +
               "\tforceCompression: " + _forceCompression + "\n" +
               "\tserverBlocking: " + _serverBlocking + "\n" +
               "\tchannelsBlocking: " + _channelsBlocking + "\n" +
               "\tserverToClientPings: " + _serverToClientPings + "\n" +
               "\tclientToServerPings: " + _clientToServerPings + "\n" +
               "\tconnectionType: " + _connectionType + "\n" +
               "\tpingTimeout: " + _pingTimeout + "\n" +
               "\tminPingTimeout: " + _minPingTimeout + "\n" +
               "\tmaxFragmentSize: " + _maxFragmentSize + "\n" +
               "\tmaxOutputBuffers: " + _maxOutputBuffers + "\n" +
               "\tguaranteedOutputBuffers: " + _guaranteedOutputBuffers + "\n" +
               "\tnumInputBuffers: " + _numInputBuffers + "\n" +
               "\tsharedPoolSize: " + _sharedPoolSize + "\n" +
               "\tsharedPoolLock: " + _sharedPoolLock + "\n" +
               "\tsysRecvBufSize: " + _sysRecvBufSize + "\n" +
               "\tmajorVersion: " + _majorVersion + "\n" +
               "\tminorVersion: " + _minorVersion + "\n" +
               "\tprotocolType: " + _protocolType + "\n" +
               "\tuserSpecObject: " + _userSpecObject + "\n" +
               "\tgroupAddress: " + _groupAddress + "\n" +
               "\ttcpOpts: " + _tcpOpts + "\n" +
               "\tencryptionOpts" + _encryptionOptions.toString() + "\n" +
               "\tWSocketOpts:" + _wsocketOpts;
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
    public void serviceName(String serviceName)
    {
        assert (serviceName != null) : "serviceName must be non-null";

        _serviceName = serviceName;
        // set port also
        try
        {
            // the service is specified as a port number
            _port = Integer.parseInt(serviceName);
        }
        catch (Exception e)
        {
            // the service is a name
            _port = GetServiceByName.getServiceByName(serviceName);
        }
    }

    @Override
    public String serviceName()
    {
        return _serviceName;
    }

    int port()
    {
        return _port;
    }

    @Override
    public void interfaceName(String interfaceName)
    {
        _interfaceName = interfaceName;
    }

    @Override
    public String interfaceName()
    {
        return _interfaceName;
    }

    @Override
    public void compressionType(int compressionType)
    {
        assert (compressionType >= 0 && compressionType <= Ripc.CompressionTypes.MAX_DEFINED) :
                "compressionType is out of range. Refer to CompTypes";

        _compressionType = compressionType;
    }

    @Override
    public int compressionType()
    {
        return _compressionType;
    }

    @Override
    public void compressionLevel(int compressionLevel)
    {
        assert (compressionLevel >= 0 && compressionLevel <= 9) : "compressionLevel is out of range (0-9)";

        _compressionLevel = compressionLevel;
    }

    @Override
    public int compressionLevel()
    {
        return _compressionLevel;
    }

    @Override
    public void forceCompression(boolean forceCompression)
    {
        _forceCompression = forceCompression;
    }

    @Override
    public boolean forceCompression()
    {
        return _forceCompression;
    }

    @Override
    public void serverBlocking(boolean serverBlocking)
    {
        _serverBlocking = serverBlocking;
    }

    @Override
    public boolean serverBlocking()
    {
        return _serverBlocking;
    }

    @Override
    public void channelsBlocking(boolean channelsBlocking)
    {
        _channelsBlocking = channelsBlocking;
    }

    @Override
    public boolean channelsBlocking()
    {
        return _channelsBlocking;
    }

    @Override
    public void serverToClientPings(boolean serverToClientPings)
    {
        _serverToClientPings = serverToClientPings;
    }

    @Override
    public boolean serverToClientPings()
    {
        return _serverToClientPings;
    }

    @Override
    public void clientToServerPings(boolean clientToServerPings)
    {
        _clientToServerPings = clientToServerPings;
    }

    @Override
    public boolean clientToServerPings()
    {
        return _clientToServerPings;
    }

    @Override
    public void connectionType(int connectionType)
    {
        assert (connectionType >= 0 && connectionType <= ConnectionTypes.MAX_DEFINED && connectionType != 4) :
                "connectionType is out of range. Refer to ConnectionTypes";

        assert (connectionType != ConnectionTypes.SEQUENCED_MCAST) : "Sequenced Multicast does not support binding into a Server";

        if (connectionType >= 0 && connectionType <= ConnectionTypes.MAX_DEFINED && connectionType != 4
            && connectionType != ConnectionTypes.SEQUENCED_MCAST)
            _connectionType = connectionType;
    }

    @Override
    public int connectionType()
    {
        return _connectionType;
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
    public void minPingTimeout(int minPingTimeout)
    {
        // If assertions disabled, ignore out-of-range value and keep existing value
        if (minPingTimeout == 0)
            _minPingTimeout = 1;
        else if (minPingTimeout > 255)
            _minPingTimeout = 255;
        else
            _minPingTimeout = minPingTimeout;
    }

    @Override
    public int minPingTimeout()
    {
        return _minPingTimeout;
    }

    @Override
    public void maxFragmentSize(int maxFragmentSize)
    {
        // Follow range rule coded in ETAC
        assert (maxFragmentSize >= 20 && maxFragmentSize <= 0xFFFF) : "maxFragmentSize is out of range (20-65535)";

        // If assertions disabled, ignore out-of-range value and keep existing value
        if (maxFragmentSize >= 20 && maxFragmentSize <= 0xFFFF)
            _maxFragmentSize = maxFragmentSize;
    }

    @Override
    public int maxFragmentSize()
    {
        return _maxFragmentSize;
    }

    @Override
    public void maxOutputBuffers(int maxOutputBuffers)
    {
        assert (maxOutputBuffers > 0 || _runningInJunits) : "maxOutputBuffers must be greater than zero";

        // max output buffers cannot be less than guaranteed output buffers,
        // so this method will not set max below guaranteed.
        if (maxOutputBuffers > 0)
        {
            if (maxOutputBuffers < _guaranteedOutputBuffers)
            {
                _maxOutputBuffers = _guaranteedOutputBuffers;
            }
            else
            {
                _maxOutputBuffers = maxOutputBuffers;
            }
        }
        // ignore a negative input value
    }

    @Override
    public int maxOutputBuffers()
    {
        return _maxOutputBuffers;
    }

    @Override
    public void guaranteedOutputBuffers(int guaranteedOutputBuffers)
    {
        if (guaranteedOutputBuffers <= 5)
            _guaranteedOutputBuffers = 5;
        else
        {
            _guaranteedOutputBuffers = guaranteedOutputBuffers;
        }

        // max must be at least as large as guaranteed
        if (_guaranteedOutputBuffers > _maxOutputBuffers)
        {
            _maxOutputBuffers = _guaranteedOutputBuffers;
        }
    }

    @Override
    public int guaranteedOutputBuffers()
    {
        return _guaranteedOutputBuffers;
    }

    @Override
    public void numInputBuffers(int numInputBuffers)
    {
        assert (numInputBuffers > 0) : "numImputBuffers must be greater than zero";

        _numInputBuffers = numInputBuffers;
    }

    @Override
    public int numInputBuffers()
    {
        return _numInputBuffers;
    }

    @Override
    public void sharedPoolSize(int sharedPoolSize)
    {
        assert (sharedPoolSize > 0) : "sharedPoolSize must be greater than zero";

        _sharedPoolSize = sharedPoolSize;
    }

    @Override
    public int sharedPoolSize()
    {
        return _sharedPoolSize;
    }

    @Override
    public void sharedPoolLock(boolean sharedPoolLock)
    {
        _sharedPoolLock = sharedPoolLock;
    }

    @Override
    public boolean sharedPoolLock()
    {
        return _sharedPoolLock;
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
    public void groupAddress(String groupAddress)
    {
        _groupAddress = groupAddress;
    }

    @Override
    public String groupAddress()
    {
        return _groupAddress;
    }

    @Override
    public WSocketOpts wSocketOpts() {
        return _wsocketOpts;
    }

    @Override
    public ServerEncryptionOptions encryptionOptions()
    {
        return _encryptionOptions;
    }
}
