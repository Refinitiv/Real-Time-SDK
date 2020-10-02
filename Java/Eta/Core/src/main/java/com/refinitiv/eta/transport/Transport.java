package com.refinitiv.eta.transport;

import java.net.*;
import java.nio.ByteBuffer;
import java.util.Enumeration;
import java.util.jar.Manifest;

import com.refinitiv.eta.transport.DummyLock;
import com.refinitiv.eta.transport.LibraryVersionInfoImpl;
import com.refinitiv.eta.transport.Lock;
import com.refinitiv.eta.transport.ReentrantLock;
import com.refinitiv.eta.transport.SocketProtocol;

/**
 * ETA transport is used by an OMM Interactive Provider to create listening
 * sockets and by OMM consumer and NIP applications to establish outbound
 * connections to a listening socket.
 */
public class Transport
{
    static private int _numInitCalls;
    static boolean _globalLocking;
    static Lock _globalLock;
    static ReentrantLock _initLock; // lock used during initialize() and uninitialize()
    static private LibraryVersionInfoImpl _libVersionInfo = new LibraryVersionInfoImpl();
    static ByteBuffer _defaultComponentVersionBuffer;
    
    static
    {
        Package thisPackage = Transport.class.getPackage();
        _libVersionInfo.productInternalVersion(thisPackage.getImplementationVersion());
        _libVersionInfo.productVersion(thisPackage.getSpecificationVersion());

        try {
            URLClassLoader cl = (URLClassLoader) Transport.class.getClassLoader();
            Enumeration<URL> urls = cl.findResources("META-INF/MANIFEST.MF");
            while(urls.hasMoreElements()) {
                URL url = urls.nextElement();
                if(url.getPath().contains("upa-")){
                    Manifest manifest = new Manifest(url.openStream());
                    String val = manifest.getMainAttributes().getValue("Build-Date");
                    _libVersionInfo.productDate(val);
                    break;
                }
            }
        } catch (Exception e) {
            _libVersionInfo.productDate(null);
        }

        if (_libVersionInfo.productInternalVersion() == null)
        {
            _libVersionInfo.productInternalVersion("ETA Java Edition");
        }
        if (_libVersionInfo.productVersion() == null)
        {
            _libVersionInfo.productVersion("ETA Java Edition");
        }
        if (_libVersionInfo.productDate() == null)
        {
            _libVersionInfo.productDate("N/A");
        }

        _defaultComponentVersionBuffer = ByteBuffer.wrap(_libVersionInfo.productInternalVersion().getBytes());
        _initLock = new ReentrantLock();
    }

    // The _transports is indexed by the connection type
    // _transports[0] - socket,
    // _transports[1] - Encrypted,
    // _transports[2] - HTTP,
    // _transports[3] - shared memory,
    // _transports[4] - mcast,
    // _transports[6] - sequenced mcast
    static Protocol[] _transports = new Protocol[ConnectionTypes.MAX_DEFINED + 1];  // should be private, but is not for junit
    static Protocol[] _encryptedTransports = new Protocol[ConnectionTypes.MAX_DEFINED + 1];

    private static final int HIDDEN_TCP_JNI = 111; // JNI TCP implementation (used only for testing)
    private static Protocol _hiddenTcpJni;         // JNI TCP implementation (used only for testing)

    /**
     * Initializes the ETA transport API and all internal members.<BR>
     * 
     * This is the first method called when using the ETA. It initializes internal data structures.
     * 
     * @param initArgs Arguments for initialize
     * @param error ETA Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see InitArgs
     */
    public static int initialize(InitArgs initArgs, Error error)
    {
        assert (initArgs != null) : "initArgs cannot be null";
        assert (error != null) : "error cannot be null";

        int ret = TransportReturnCodes.SUCCESS;
        _initLock.lock();
        if (_numInitCalls > 0) // subsequent initialize() call
        {
            if (_globalLocking == initArgs.globalLocking())
            {
                ++_numInitCalls;
            }
            else
            {
                error.channel(null);
                error.errorId(TransportReturnCodes.INIT_NOT_INITIALIZED);
                error.sysError(0);
                error.text("initialize() attempting to change locking from " + _globalLocking + " to " + initArgs.globalLocking());
                ret = TransportReturnCodes.FAILURE;
            }
        }
        else // first initialize() call
        {
            if (initArgs.globalLocking())
                _globalLock = new ReentrantLock();
            else
                _globalLock = new DummyLock();
            
            _globalLocking = initArgs.globalLocking();
            ++_numInitCalls;
        }

        _initLock.unlock();
        return ret;
    }

    /**
     * Uninitializes the ETA API and all internal members.<BR>
     * 
     * This is the last method called by an application when using the ETA. 
     * If multiple threads call initialize() on Transport, they have to
     * call uninitialize() when the thread finishes.
     * The last uninitialize() call releases all internally pooled resources to GC.
     * 
     * @return {@link TransportReturnCodes}
     */
    public static int uninitialize()
    {
        int ret = TransportReturnCodes.SUCCESS;
        _initLock.lock();
        if (_numInitCalls == 0)
        {
            ret = TransportReturnCodes.INIT_NOT_INITIALIZED;
        }
        else if (--_numInitCalls == 0)
        {
            _globalLock.lock();
            for (int i = 0; i < _transports.length; i++)
            {
                if (_transports[i] != null)
                {
                    _transports[i].uninitialize();
                    _transports[i] = null;
                }
            }
            
            for (int i = 0; i < _encryptedTransports.length; i++)
            {
                if (_encryptedTransports[i] != null)
                {
                	_encryptedTransports[i].uninitialize();
                	_encryptedTransports[i] = null;
                }
            }
            if (_hiddenTcpJni != null)
            {
                _hiddenTcpJni.uninitialize();
                _hiddenTcpJni = null;
            }
            _globalLock.unlock();
        }
        
        _initLock.unlock();
        return ret;
    }

    /**
     * Instantiates a new transport.
     */
    // Transport class cannot be instantiated
    private Transport()
    {
        throw new AssertionError();
    }

    /**
     * Gets the IP address of a hostname.<BR>
     * 
     * This method gets the IP address of a hostname in host byte order.
     * 
     * @param hostName Hostname to get IP address for
     * 
     * @return The IP address
     */
    public static InetSocketAddress hostByName(String hostName)
    {
        assert (hostName != null) : "hostName cannot be null";

        return new InetSocketAddress(hostName, 0);
    }

    /**
     * Gets the user name.<BR>
     * 
     * This method gets the user name that the owner of the current process is logged in under.
     * 
     * @return User name of user
     */
    public static String userName()
    {
        return System.getProperty("user.name");
    }

    /**
     * Initialize transport defined in opts if not initialized.<BR>
     * 
     * Connects a client to a listening server.<BR>
     * 
     * 1. Initialize ConnectOptions<BR>
     * 2. Set ConnectOptions to desired values<BR>
     * 3. Call connect to create Channel<BR>
     * 4. Read or write with the Channel<BR>
     * 
     * @param opts Options used when connecting
     * @param error ETA Error, to be populated in event of an error
     * 
     * @return Connected ETA channel or NULL
     * 
     * @see ConnectOptions
     * @see Channel
     */
    public static Channel connect(ConnectOptions opts, Error error)
    {
        assert (opts != null) : "opts cannot be null";
        assert (error != null) : "error cannot be null";

        Channel channel = null;

        if (_numInitCalls == 0)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.INIT_NOT_INITIALIZED);
            error.sysError(0);
            error.text("Transport is not initialized");
        }
        else
        {
            try
            {
                _globalLock.lock();
                if (opts.connectionType() > _transports.length - 1 &&
                    opts.connectionType() != HIDDEN_TCP_JNI)
                {
                    error.channel(null);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Unsupported transport type");
                }
                else
                {
                    Protocol transport = null;
                    if (opts.connectionType() != HIDDEN_TCP_JNI)
                    {
                    	if(opts.connectionType() == ConnectionTypes.ENCRYPTED)
                    	{
                    		if(opts.tunnelingInfo().tunnelingType().equalsIgnoreCase("none"))
                    			transport = _encryptedTransports[opts.encryptionOptions().connectionType()];
                    		else
                    			transport = _encryptedTransports[ConnectionTypes.HTTP];
                    	}
                    	else
                    	{
                    		transport = _transports[opts.connectionType()];
                    	}
                    }
                    switch (opts.connectionType())
                    {
                    	case ConnectionTypes.ENCRYPTED:
                    		// Verify that the encryption type has been set correctly.  Error out if this is not the case.
                    		if(opts.tunnelingInfo().tunnelingType().equalsIgnoreCase("None"))
                    		{
                    			if(opts.encryptionOptions().connectionType() != ConnectionTypes.SOCKET && opts.encryptionOptions().connectionType() != ConnectionTypes.HTTP )
                    			{
                    				 error.channel(null);
                                     error.errorId(TransportReturnCodes.FAILURE);
                                     error.sysError(0);
                                     error.text("Unsupported sub-protocol type configured in opts.encryptionOptions().connectionType()");
                    			}
                    		}
                    		if (transport == null) // not initialized yet - first connection for this transport
                            {
                                transport = new SocketProtocol(opts);
                                _encryptedTransports[opts.connectionType()] = transport;
                            }
                            channel = transport.channel(opts, error);
                            break;
                        case ConnectionTypes.SOCKET:
                        case ConnectionTypes.HTTP:
                            if (transport == null) // not initialized yet - first connection for this transport
                            {
                                transport = new SocketProtocol(opts);
                                _transports[opts.connectionType()] = transport;
                            }
                            channel = transport.channel(opts, error);
                            break;
                        case ConnectionTypes.RELIABLE_MCAST:
                        case ConnectionTypes.UNIDIR_SHMEM:
                            if (transport == null) // not initialized yet - first connection for this transport
                            {
                                transport = new JNIProtocol();
                                _transports[opts.connectionType()] = transport;
                            }
                            channel = transport.channel(opts, error);
                            break;
                        case ConnectionTypes.SEQUENCED_MCAST:
                            if (transport == null) // Not initialized yet- first connection for this transport
                            {
                                transport = new SequencedMulticastProtocol(opts);
                                _transports[opts.connectionType()] = transport;
                            }
                            channel = transport.channel(opts, error);
                            break;
                        case HIDDEN_TCP_JNI: // back door for TCP JNI (used only for testing)
                            if (_hiddenTcpJni == null)
                            {
                                _hiddenTcpJni = new JNIProtocol();
                            }
                            opts.connectionType(ConnectionTypes.SOCKET);
                            channel = _hiddenTcpJni.channel(opts, error);
                            break;
                        default:
                            error.channel(null);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("Unsupported transport type");
                    }
                }
            }
            finally
            {
                _globalLock.unlock();
            }
        }
        return channel;
    }

    /**
     * Creates a ETA Server by binding to a port.<BR>
     * 
     * 1. Initialize BindOptions<BR>
     * 2. Set BindOptions to desired values<BR>
     * 3. Call bind to create {@link Server}<BR>
     * 
     * @param opts Options used when binding
     * @param error ETA Error, to be populated in event of an error
     * 
     * @return Bound ETA server or NULL
     * 
     * @see BindOptions
     * @see Server
     */
    public static Server bind(BindOptions opts, Error error)
    {
        assert (opts != null) : "opts cannot be null";
        assert (error != null) : "error cannot be null";

        Server server = null;
        if (_numInitCalls == 0)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.INIT_NOT_INITIALIZED);
            error.sysError(0);
            error.text("Transport is not initialized");
        }
        else
        {
            try
            {
                _globalLock.lock();
                if (opts.connectionType() > _transports.length - 1 &&
                    opts.connectionType() != HIDDEN_TCP_JNI)
                {
                    error.channel(null);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Unsupported transport type");
                }
                else
                {
                    Protocol transport = null;
                    if (opts.connectionType() != HIDDEN_TCP_JNI)
                    {
                        transport = _transports[opts.connectionType()];
                    }
                    switch (opts.connectionType())
                    {
                        case ConnectionTypes.HTTP:
                        case ConnectionTypes.SOCKET:
                            if (transport == null) // not initialized yet -/ first connection for this transport
                            {
                                transport = new SocketProtocol();
                                _transports[opts.connectionType()] = transport;
                            }
                            server = transport.server(opts, error);
                            break;
                        case ConnectionTypes.RELIABLE_MCAST:
                        case ConnectionTypes.UNIDIR_SHMEM:
                            if (transport == null) // not initialized yet - first connection for this transport
                            {
                                transport = new JNIProtocol();
                                _transports[opts.connectionType()] = transport;
                            }
                            server = transport.server(opts, error);
                            break;
                        case ConnectionTypes.SEQUENCED_MCAST:
                            if (transport == null) // Not initialized yet- first connection for this transport
                            {
                                transport = new SequencedMulticastProtocol();
                                _transports[opts.connectionType()] = transport;
                            }
                            server = transport.server(opts, error);
                            break;
                        case HIDDEN_TCP_JNI: // back door for TCP JNI (used only for testing)
                            if (_hiddenTcpJni == null)
                            {
                                _hiddenTcpJni = new JNIProtocol();
                            }
                            opts.connectionType(ConnectionTypes.SOCKET);
                            server = _hiddenTcpJni.server(opts, error);
                            break;
                        default:
                            error.channel(null);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("Unsupported transport type");
                    }
                }
            }
            finally
            {
                _globalLock.unlock();
            }
        }
        return server;
    }

    /**
     * Programmatically extracts library and product version information that is compiled into this library.<BR>
     * 
     * User can call this method to programmatically extract version information.<BR>
     *
     * @return the library version info
     * @see LibraryVersionInfo
     */
    public static LibraryVersionInfo queryVersion()
    {
        return _libVersionInfo;
    }

    /**
     * <b>WARNING: creates garbage</b>
     * Returns a hex representation of the data in the provided {@link ByteBuffer}.
     *
     * @param buffer Contains the data to print
     * @param startPosition The starting position of the data in the buffer
     * @param length The length of the data
     * @return String the hex representation of the data.
     */
    public static String toHexString(ByteBuffer buffer, final int startPosition, final int length)
    {
        assert (buffer != null) : "buffer cannot be null";

        final int charsPerLine = 16;
        StringBuilder asString = new StringBuilder();
        StringBuilder currentLine = new StringBuilder();
        StringBuilder all = new StringBuilder();

        boolean processedFirst = false;
        int lineNo = 0;
        int currentChar = 0;

        // visit all the characters in the range
        for (int i = startPosition; i < (startPosition + length); i++)
        {
            if (!(currentChar < charsPerLine))
            {
                // complete this line:
                if (processedFirst)
                {
                    all.append(String.format("\n%04X: ", lineNo++));
                }
                else
                {
                    all.append(String.format("%04X: ", lineNo++));
                    processedFirst = true;
                }

                all.append(currentLine.toString()); // hex
                all.append("  "); // spacer
                all.append(asString.toString());

                // reset to prepare for the next line
                currentLine.setLength(0);
                asString.setLength(0);
                currentChar = 0;
            }

            byte b = buffer.get(i);
            currentLine.append(String.format("%02X ", b)); // convert the current byte to hex

            // prepare the byte to be printed as a string
            if (b > 31 && b < 127)
                asString.append((char)b);
            else
                asString.append('.');

            if (currentChar == 7)
            {
                currentLine.append(" "); // add an extra space after 8 chars
            }
            ++currentChar;
        }

        // process the last remaining line, if required
        if (currentLine.length() > 0)
        {
            if (processedFirst)
            {
                all.append("\n");
            }
            all.append(String.format("%04X: ", lineNo++)); // append the current line number
            // fill in any unused chars
            int fill = currentChar;
            while (fill < charsPerLine)
            {
                currentLine.append("   ");

                if (fill == 7)
                {
                    currentLine.append(" "); // add an extra space after 8 chars
                }
                ++fill;
            }

            all.append(currentLine.toString()); // hex
            all.append("  "); // spacer
            all.append(asString.toString());
        }

        return all.toString();
    }
}
