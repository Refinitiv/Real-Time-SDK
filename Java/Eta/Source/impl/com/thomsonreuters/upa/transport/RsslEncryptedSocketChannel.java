package com.thomsonreuters.upa.transport;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;

import javax.net.ssl.SSLContext;

import com.thomsonreuters.proxy.authentication.ProxyAuthenticationException;

class RsslEncryptedSocketChannel extends RsslHttpSocketChannel
{
    KeyStore ClientKS = null;

    javax.net.ssl.KeyManagerFactory ClientKMF = null;

    javax.net.ssl.TrustManagerFactory ClientTMF = null;

    FileInputStream _fstream;

    RsslEncryptedSocketChannel(SocketProtocol transport, Pool channelPool)
    {
        super(transport, channelPool);

        _encrypted = true;
    }

    /* TEST ONLY: This is not a valid constructor. (only for JUnit tests) */
    protected RsslEncryptedSocketChannel()
    {
        super();

        _encrypted = true;
    }

    @Override
    public int connectionType()
    {
        return ConnectionTypes.ENCRYPTED;
    }

    /* This method is overridden by the JUnit tests to simulate reading from a
     * real java.nio.channels.SocketChannel
     * 
     * Reads a sequence of bytes from this channel into the given buffer.
     * An attempt is made to read up to r bytes from the channel, where r is
     * the number of bytes remaining in the buffer, that is, dst.remaining(),
     * at the moment this method is invoked.
     * 
     * Suppose that a byte sequence of length n is read, where 0 <= n <= r.
     * This byte sequence will be transferred into the buffer so that the
     * first byte in the sequence is at index p and the last byte is at
     * index p + n - 1, where p is the buffer's position at the moment
     * this method is invoked. Upon return the buffer's position will be
     * equal to p + n; its limit will not have changed.
     * 
     * A read operation might not fill the buffer, and in fact it might not
     * read any bytes at all. Whether or not it does so depends upon the
     * nature and state of the channel. A socket channel in non-blocking mode,
     * for example, cannot read any more bytes than are immediately available
     * from the socket's input buffer; similarly, a file channel cannot read
     * any more bytes than remain in the file. It is guaranteed, however, that
     * if a channel is in blocking mode and there is at least one byte
     * remaining in the buffer then this method will block until at least one byte is read.
     * 
     * This method may be invoked at any time. If another thread has already
     * initiated a read operation upon this channel, however, then an
     * invocation of this method will block until the first operation is complete.
     * Specified by: read(...) in ReadableByteChannel
     * 
     * dst is the buffer into which bytes are to be transferred
     * 
     * Returns the number of bytes read, possibly zero, or -1 if the channel has reached end-of-stream
     * 
     * Throws IOException if some other I/O error occurs
     */
    protected int read(ByteBuffer dst) throws IOException
    {
        if (_crypto == null)
            throw new IOException("Encryption engine is not set up, check configuration.");

        return _crypto.read(dst);
    }

    /* This method is overridden by the JUnit tests to simulate writing to a real java.nio.channels.SocketChannel
     * 
     * srcs is the buffers from which bytes are to be retrieved
     * offset is the offset within the buffer array of the first buffer from which bytes are to be retrieved;
     *        must be non-negative and no larger than srcs.length
     * length is the maximum number of buffers to be accessed;
     *        must be non-negative and no larger than srcs.length - offset
     * 
     * Returns the number of bytes written, possibly zero
     * 
     * Throws IOException if some other I/O error occurs
     */
    protected long write(ByteBuffer[] srcs, int offset, int length) throws IOException
    {
        if (_crypto == null)
            throw new IOException("Encryption engine is not set up, check configuration.");

        _bytesWritten = _crypto.write(srcs, offset, length);
        _totalWriteTunnelCount += _bytesWritten;
        return _bytesWritten;
    }

    protected void initializeEngine() throws IOException
    {
        try
        {
            initializeTLS(_cachedConnectOptions);
        }
        catch (IOException e)
        {
            // System.out.println("IOException   EncryptedSocketChannel::initializeEngine()  initializeTLS: " + e.getMessage());
            throw e;
        }
    }

    void initializeTLS(ConnectOptions options) throws IOException
    {
        assert (options != null) : "options cannot be null";

        // keystore password
        char[] ClientKeystorePassword = options.tunnelingInfo().KeystorePasswd().toCharArray();

        // keys password
        // get a JKS KeyStore
        try
        {
            // ClientKS=KeyStore.getInstance("JKS"); //JKS=JavaKeyStore
            if (options.tunnelingInfo().KeystoreType().equals(""))
            {
                // get JavaKeyStore from java.security file (default: keystore.type=jks)
                if (ClientKS == null)
                    ClientKS = KeyStore.getInstance(KeyStore.getDefaultType());
            }
            else if (ClientKS == null)
                ClientKS = KeyStore.getInstance(options.tunnelingInfo().KeystoreType());
        }
        catch (KeyStoreException e)
        {
            throw new IOException("Error when getting keystore type  " + e.getMessage());
        }

        // load the keystore from the keystore file
        try
        {
            if (_fstream == null)
            {
                _fstream = new FileInputStream(options.tunnelingInfo().KeystoreFile());

                ClientKS.load(_fstream, ClientKeystorePassword);
            }
        }
        catch (IOException e)
        {
            throw new IOException("Error when loading keystore from certificate file  " + e.getMessage());
        }
        catch (NoSuchAlgorithmException e)
        {
            throw new IOException("Error when loading keystore from certificate file  " + e.getMessage());
        }
        catch (CertificateException e)
        {
            throw new IOException("CertificateException when loading keystore from certificate file  " + e.getMessage());
        }

        // create a TrustManagerFactory
        try
        {
            if (options.tunnelingInfo().TrustManagerAlgorithm().equals(""))
            {
                // get default trust management algorithm for security provider
                // (default: PKIX for security provider SunJSSE)
                ClientTMF = javax.net.ssl.TrustManagerFactory.getInstance(javax.net.ssl.TrustManagerFactory.getDefaultAlgorithm(), 
                                                                          options.tunnelingInfo().SecurityProvider());
            }
            else
                ClientTMF = javax.net.ssl.TrustManagerFactory.getInstance(options.tunnelingInfo().TrustManagerAlgorithm(),
                                                                          options.tunnelingInfo().SecurityProvider());
        }
        catch (NoSuchAlgorithmException e)
        {
            throw new IOException("Error when creating TrustManagerFactory:  " + e.getMessage());
        }
        catch (NoSuchProviderException e)
        {
            throw new IOException("Error when creating TrustManagerFactory: " + e.getMessage());
        }

        // initialize the above TrustManagerFactory
        try
        {
            ClientTMF.init(ClientKS);
        }
        catch (KeyStoreException e)
        {
            throw new IOException("Error when initializing TrustManagerFactory:  " + e.getMessage());
        }

        // create a Java SSLContext object
        try
        {
            SSLContext cntx = SSLContext.getInstance(options.tunnelingInfo().SecurityProtocol());
            if (options.tunnelingInfo().KeyManagerAlgorithm().equals(""))
            {
                // get default key management algorithm for security provider
                // (default: SunX509 for security provider SunJSSE)
                ClientKMF = javax.net.ssl.KeyManagerFactory.getInstance(javax.net.ssl.KeyManagerFactory.getDefaultAlgorithm(),
                                                                        options.tunnelingInfo().SecurityProvider());
            }
            else
            {
                ClientKMF = javax.net.ssl.KeyManagerFactory.getInstance(options.tunnelingInfo().KeyManagerAlgorithm(),
                                                                        options.tunnelingInfo().SecurityProvider());
            }
            ClientKMF.init(ClientKS, ClientKeystorePassword);

            cntx.init(ClientKMF.getKeyManagers(), ClientTMF.getTrustManagers(), null);

            _crypto = new CryptoHelper(this, cntx, _scktChannel);
        }
        catch (NoSuchAlgorithmException e)
        {
            throw new IOException("Error when initializing SSLContext:  " + e.getMessage());
        }
        catch (NoSuchProviderException e)
        {
            throw new IOException("Error when initializing SSLContext:  " + e.getMessage());
        }
        catch (KeyStoreException e)
        {
            throw new IOException("Error when initializing SSLContext:  " + e.getMessage());
        }
        catch (UnrecoverableKeyException e)
        {
            throw new IOException("UnrecoverableKeyException when initializing SSLContext:  " + e.getMessage());
        }
        catch (KeyManagementException e)
        {
            // System.out.println("Error:"+e.getMessage());
            throw new IOException("KeyManagementException when initializing SSLContext:  " + e.getMessage());
        }
        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            System.out.println("Initialize for TLS/SSL engine completed.");
    }

    /* Handle proxy reply to HTTP CONNECT and do any necessary proxy authentication. */
    protected int initChnlWaitProxyAck(InProgInfo inProg, Error error) throws IOException, ProxyAuthenticationException
    {
        int cc;

        cc = initProxyChnlReadFromChannel(_initChnlReadBuffer, error);
        // System.out.println(Transport.toHexString(_initChnlReadBuffer, 0, _initChnlReadBuffer.position()));

        if (cc == TransportReturnCodes.FAILURE)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Could not read HTTP OK (reply to HTTP CONNECT)");
            return TransportReturnCodes.FAILURE;
        }

        _totalBytesRead += cc;

        // do authentication handling
        readHttpConnectResponse(_initChnlReadBuffer, inProg, error);
        _initChnlReadBuffer.clear();

        if (_proxyAuthenticator.isAuthenticated())
        {
            _httpPOSTwriteBuffer = setupClientPOSThttpRequest(_initChnlWriteBuffer);
            if (_httpPOSTwriteBuffer == null)
                return TransportReturnCodes.FAILURE;

            if (_crypto._engine == null) // needed for possible recovery
                throw new IOException("Invalid SSLEngine during initChnlWaitProxyAck");

            _crypto.startHandshake();
            _crypto.write(_httpPOSTwriteBuffer);

            _initChnlWriteBuffer.clear();

            _initChnlState = InitChnlState.CLIENT_WAIT_HTTP_ACK;
        }
        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
    }
    
    /* Write POST message.
     * Called when in the init state HTTP_CONNECTING. */
    int initChnlHttpConnecting() throws IOException
    {
        httpOKretry = 0; // needed for recovery through a proxy

        _httpPOSTwriteBuffer = setupClientPOSThttpRequest(_initChnlWriteBuffer);

        if(_httpPOSTwriteBuffer == null)
            return TransportReturnCodes.FAILURE;
        try
        {
            if (_crypto._engine == null) // needed for possible recovery
                throw new IOException("Invalid SSLEngine during initChnlHTTPConnecting");

            _crypto.startHandshake();
            _crypto.write(_httpPOSTwriteBuffer);
        }
        catch(Exception e)
        {
            return TransportReturnCodes.FAILURE;
        }
      
        _initChnlState = InitChnlState.CLIENT_WAIT_HTTP_ACK;
      
        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS; 
    }

    protected int initChnlSendClientAck(InProgInfo inProg, Error error)
            throws IndexOutOfBoundsException, IOException
    {
        _initChnlWriteBuffer.clear();
        _crypto.write(_ipcProtocol.encodeClientKey(_initChnlWriteBuffer, error));

        _initChnlState = InitChnlState.ACTIVE;
        _state = ChannelState.ACTIVE;
        _readBufStateMachine.ripcVersion(_ipcProtocol.ripcVersion());

        // set shared key - client side would be calculated here
        _shared_key = _ipcProtocol.protocolOptions()._shared_key;
        return TransportReturnCodes.SUCCESS;
    }

    /* Reconnect and bridge connections.
     * After reconnectClient(Error) is completed, on the next read/performReadIO call
     * we will keep reading from _oldScktChannel until we receive 0x01 0x0D 0x0A 0x0D 0x0A
     * and then set rcvEndOfResponseOldChannel to true and return TransportReturnCodes.READ_FD_CHANGE to the application.
     * The application will then cancel the old channel read select and add the new channel read/write select
     * (e.g. see ChannelSession::readInt(PingHandler, ResponseCallback, Error)
     * and all the next reads will be from the new _scktChannel. */
    public int reconnectClient(Error error)
    {
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.SUCCESS;

        try
        {
            lockReadWriteLocks();

            int ret = flush(error);
            if (ret < TransportReturnCodes.SUCCESS)
                System.out.println("http channel flush failed (during EncryptedSocketChannel::reconnectClient) with returned code: " + ret + " - " + error.text());

            _oldScktChannel = _scktChannel;
            _oldCrypto = _crypto;
            _oldCrypto.setChannel(_oldScktChannel);

            // open new connection
            connectHTTPreconnectState(_cachedConnectOptions, error);

            boolean connected = false;
            while (!connected)
            {
                try
                {
                    if (_scktChannel.finishConnect())
                        connected = true;
                }
                catch (Exception e)
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text(e.getLocalizedMessage());
                    retVal = TransportReturnCodes.FAILURE;
                }
            }

            _httpReconnectState = true;
            if (_httpProxy)
            {
                _httpReconnectProxyActive = false;
                // for the new connection do complete proxy authentication if necessary
                doProxyAuthenticationHTTPreconnectState(error);
            }
            else
            {
                // initialize TLS/SSL engine
                try
                {
                    initializeTLS(_cachedConnectOptions);
                }
                catch (IOException e)
                {
                    System.out.println("IOException   forceReconnect()  initializeTLS: " + e.getMessage());
                }
            }

            // send http+13 with 64h opCode on new connection and check back for an ACK
            try
            {
                reconnectACK(error);
                System.out.println("after EncryptedSocketChannel::reconnectACK(error) new socketChannel is:" + _scktChannel);
            }
            catch (Exception e)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text(e.getLocalizedMessage());
                retVal = TransportReturnCodes.FAILURE;
            }
            _totalWriteTunnelCount = 0;
        }
        finally
        {
            unlockReadWriteLocks();
        }

        return retVal;
    }

    /* Only for httpReconnectState.
     * First handshake on new connection (send HTTP POST ... and get back an ACK). */
    private void reconnectACK(Error error) throws IOException
    {
        setupClientPOSThttpRequestReconnectState(_initChnlWriteBuffer);
        boolean connected = false;
        while (!connected)
        {
            if (_scktChannel.finishConnect())
            {
                connected = true;
                _crypto.startHandshake();
                _crypto.write(_initChnlWriteBuffer);
            }
        }

        _initChnlReadBuffer.clear();
        while (_initChnlReadBuffer.position() < HTTP_RECONNECT_ACK_SIZE)
        {
            _crypto.read(_initChnlReadBuffer);
        }
        boolean rcvACK = readHTTPreconnectResponseACK(_initChnlReadBuffer);
        if (rcvACK)
            rcvACKnewChannel = true;
        else
            throw new IOException("rcvACK==" + rcvACK);
    }
}
