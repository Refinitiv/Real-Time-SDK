package com.refinitiv.eta.transport;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.util.Collections;

import javax.net.ssl.SNIHostName;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLEngineResult;
import javax.net.ssl.SSLEngineResult.HandshakeStatus;
import javax.net.ssl.SSLEngineResult.Status;
import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLPeerUnverifiedException;

// CryptoHelper is a partial Java SocketChannel implementation that uses
// the Java SSLEngine to do encryption and decryption of application data
//
//
// Flow of encrypted/decrypted application data and handshaking data
// through the Java SSLEngine and the 4 ByteBuffers it requires:
//
//         -------------------application data---------------------
//                    ^                               |
//   (decrypted data) |                               | (data not yet encrypted)
//                    |                               v
//           +------------------+            +------------------+
//           |  _appRecvBuffer  |            |  _appSendBuffer  |
//           +------------------+            +------------------+
//                    ^                               |
//   (decrypted data) |                               | (data not yet encrypted)
//                    |                               v
//         +----------+-------------------------------+------------+
//         |+---------+-------------------------------+-----------+|
//         ||         ^                               |           ||
//         ||         |           SSLEngine           |           ||
//         ||         |                               v           ||
//         ||      unwrap()---->   ..........   ---->wrap()       ||
//         ||         ^          handshake info.      |           ||
//         ||         |                               |           ||
//         ||         |                               v           ||
//         |+---------+-------------------------------+-----------+|
//         +----------+-------------------------------+------------+
//                    ^                               |
//   (encrypted data) |                               | (encrypted data)
//                    |                               v
//           +------------------+            +------------------+ 
//           |  _netRecvBuffer  |            |  _netSendBuffer  |
//           +------------------+            +------------------+
//                    ^                               |
//   (encrypted data) |                               | (encrypted data)
//                    |                               |
//                    |                               v
//         -----------------------network data---------------------
//
// 'application data' is also known as plaintext or cleartext
// 'network data' can be handshaking data and/or encrypted data (also known as ciphertext)
//
//
// the 'application data' is the destination buffers used in RsslEncryptedSocketChannel
// and
// the 'network data' is the data from each SocketChannel read after each RsslEncryptedSocketChannel::read(ByteBuffer dst)
// or from each SocketChannel write after each RsslSocketChannel::write()

class CryptoHelper
{

    public static final String[] CLIENT_PROTOCOLS = {"TLSv1.2"};
    
    /**
     * "HTTPS" algorithm performs endpoint verification as described in 
     * https://tools.ietf.org/html/rfc2818#section-3
     * When client receives certificate from the server, client verifies that certificate
     * matches hostname of the remote server
     */
    public static final String ENDPOINT_IDENTIFICATION_ALGORITHM = "HTTPS";

    private final SSLContext cntx;

    /* Initializes the client side */
    CryptoHelper(ConnectOptions options) throws IOException
    {
        assert (options != null) : "options cannot be null";

        KeyStore clientKS;
        javax.net.ssl.KeyManagerFactory clientKMF;
        javax.net.ssl.TrustManagerFactory clientTMF;

        String keystorePassword;
        String keystoreFile;
        String keystoreType;
        String securityProvider;
        String trustManagerAlgorithm;
        String securityProtocol;
        String keyManagerAlgorithm;
        
        EncryptionOptionsImpl encOpts = (EncryptionOptionsImpl)options.encryptionOptions();

                
    	if(options.tunnelingInfo().tunnelingType().equalsIgnoreCase("None"))
    	{
    		keystorePassword = encOpts.KeystorePasswd();
    		keystoreFile = encOpts.KeystoreFile();
    		keystoreType = encOpts.KeystoreType();
    		securityProvider = encOpts.SecurityProvider();
    		trustManagerAlgorithm = encOpts.TrustManagerAlgorithm();
    		securityProtocol = encOpts.SecurityProtocol();
    		keyManagerAlgorithm = encOpts.KeyManagerAlgorithm();
    	}
    	else
    	{
    		keystorePassword = options.tunnelingInfo().KeystorePasswd();
            keystoreFile = options.tunnelingInfo().KeystoreFile();
            keystoreType = options.tunnelingInfo().KeystoreType();
            securityProvider = options.tunnelingInfo().SecurityProvider();
            trustManagerAlgorithm = options.tunnelingInfo().TrustManagerAlgorithm();
            securityProtocol = options.tunnelingInfo().SecurityProtocol();
            keyManagerAlgorithm = options.tunnelingInfo().KeyManagerAlgorithm();
    	}
        
        char[] keystorePasswordChars = keystorePassword != null ? keystorePassword.toCharArray() : null;
        if (keystoreFile != null && !keystoreFile.isEmpty())
        {

        		clientKS = initializeClientKeystore(keystorePasswordChars, keystoreFile, keystoreType);
        }
        else
        {
            //If KeyStore is set to null then TrustManagerFactory will be initialized with certificates from
            //default trusted keystore
            clientKS = null;
        }
        // create a TrustManagerFactory
        try
        {
            if (trustManagerAlgorithm == null || trustManagerAlgorithm.equals(""))
            {
            	if(securityProvider == null || securityProvider.equals(""))
	                // get default trust management algorithm for security provider
	                // (default: PKIX for security provider SunJSSE)
	                clientTMF = javax.net.ssl.TrustManagerFactory.getInstance(encOpts._defaultTrustManagerAlgorithm, encOpts._defaultSecurityProvider);
            	else
            		clientTMF = javax.net.ssl.TrustManagerFactory.getInstance(encOpts._defaultTrustManagerAlgorithm, securityProvider);
            }
            else
                clientTMF = javax.net.ssl.TrustManagerFactory.getInstance(trustManagerAlgorithm,
                		securityProvider);
        }
        catch (NoSuchAlgorithmException | NoSuchProviderException e)
        {
            throw new IOException("Error when creating TrustManagerFactory: " + e.getMessage());
        }

        // initialize the above TrustManagerFactory
        try
        {
            clientTMF.init(clientKS);
        }
        catch (KeyStoreException e)
        {
            throw new IOException("Error when initializing TrustManagerFactory:  " + e.getMessage());
        }

        // create a Java SSLContext object
        try
        {
        	if(securityProtocol == null || securityProtocol.equals(""))
        		cntx = SSLContext.getInstance(encOpts._defaultSecurityProtocol);
        	else
        		cntx = SSLContext.getInstance(securityProtocol);

        	
            if (keyManagerAlgorithm == null || keyManagerAlgorithm.equals(""))
            {
            	if(securityProvider == null || securityProvider.equals(""))
	            	// get default key management algorithm for security provider
	                // (default: SunX509 for security provider SunJSSE)
            		clientKMF = javax.net.ssl.KeyManagerFactory.getInstance(encOpts._defaultKeyManagerAlgorithm, encOpts._defaultSecurityProvider);
            	else
                    clientKMF = javax.net.ssl.KeyManagerFactory.getInstance(encOpts._defaultKeyManagerAlgorithm, securityProvider);

            }
            else
            {
                clientKMF = javax.net.ssl.KeyManagerFactory.getInstance(options.tunnelingInfo().KeyManagerAlgorithm(),
                		securityProvider);
            }
            clientKMF.init(clientKS, keystorePasswordChars);

            cntx.init(clientKMF.getKeyManagers(), clientTMF.getTrustManagers(), null);
        }
        catch (NoSuchAlgorithmException | NoSuchProviderException | KeyStoreException e)
        {
            throw new IOException("Error when initializing SSLContext:  " + e.getMessage());
        } catch (UnrecoverableKeyException e)
        {
            throw new IOException("UnrecoverableKeyException when initializing SSLContext:  " + e.getMessage());
        }
        catch (KeyManagementException e)
        {
            throw new IOException("KeyManagementException when initializing SSLContext:  " + e.getMessage());
        }
        
        _connectionKeyManagerAlgorithm = keyManagerAlgorithm;
        _hostName = options.unifiedNetworkInfo().address();
        try
        {
            // the service is specified as a port number
            _hostPort = Integer.parseInt(options.unifiedNetworkInfo().serviceName());
        }
        catch (Exception e)
        {
            // the service is a name
            _hostPort = GetServiceByName.getServiceByName(options.unifiedNetworkInfo().serviceName());
        }
        
        _server = false;
    }
    
    /* Initializes the server side */
    CryptoHelper(BindOptions options, SSLContext context) throws IOException
    {
        assert (options != null) : "options cannot be null";

        cntx = context;
        
        _connectionKeyManagerAlgorithm = options.encryptionOptions().keyManagerAlgorithm();
       
        _server = true;
    }

    public void initializeEngine(SocketChannel socketChannel) throws IOException
    {
        _socketChannel = socketChannel;

        // setup Java SSLEngine to be used, depending if it's a server or client.
        if(!_server)
        {
	        _engine = cntx.createSSLEngine(_hostName, _hostPort);
	        _engine.setUseClientMode(true);
	
	        SSLParameters sslParameters = new SSLParameters();
	        sslParameters.setProtocols(CLIENT_PROTOCOLS);
	        sslParameters.setEndpointIdentificationAlgorithm(ENDPOINT_IDENTIFICATION_ALGORITHM);
	        sslParameters.setServerNames(Collections.singletonList(new SNIHostName(_hostName)));
	        sslParameters.setNeedClientAuth(false);
	        _engine.setSSLParameters(sslParameters);
        }
        else
        {
        	_engine = cntx.createSSLEngine();
        	_engine.setUseClientMode(false);
        	SSLParameters sslParameters = new SSLParameters();
	        sslParameters.setProtocols(CLIENT_PROTOCOLS);
	        sslParameters.setNeedClientAuth(false);
	        _engine.setSSLParameters(sslParameters);
        }

        // get the largest possible buffer size for the application data buffers that are used for Java SSLEngine
        final int appBufferSize = _engine.getSession().getApplicationBufferSize();

        // get the largest possible buffer size for the network data buffers that are used for Java SSLEngine
        final int sslBufferSize = _engine.getSession().getPacketBufferSize();

        // allocate the buffers used for Java SSLEngine
        // (doubling the receive size for cases where the application data size is very large (e.g. data dictionary)
        _netRecvBuffer = ByteBuffer.allocateDirect(4 * sslBufferSize); // receive buffers
        _appRecvBuffer = ByteBuffer.allocateDirect(4 * appBufferSize);
        _appSendBuffer = ByteBuffer.allocateDirect(2 * appBufferSize); // send buffers
        _netSendBuffer = ByteBuffer.allocateDirect(2 * sslBufferSize);
    }

    private KeyStore initializeClientKeystore(char[] clientKeystorePassword, String keystoreFile, String keystoreType) throws IOException
    {
        KeyStore ClientKS;
        try
        {
            // ClientKS=KeyStore.getInstance("JKS"); //JKS=JavaKeyStore
            if (keystoreType.equals(""))
            {
                // get JavaKeyStore from java.security file (default: keystore.type=jks)
                ClientKS = KeyStore.getInstance(KeyStore.getDefaultType());
            } else
            {
                ClientKS = KeyStore.getInstance(keystoreType);
            }
        } catch (KeyStoreException e)
        {
            throw new IOException("Error when getting keystore type  " + e.getMessage());
        }

        // load the keystore from the keystore file
        try
        {
            FileInputStream _fstream = new FileInputStream(keystoreFile);

            ClientKS.load(_fstream, clientKeystorePassword);
        }
        catch (IOException | NoSuchAlgorithmException e)
        {
            throw new IOException("Error when loading keystore from certificate file  " + e.getMessage());
        }
        catch (CertificateException e)
        {
            throw new IOException("CertificateException when loading keystore from certificate file  " + e.getMessage());
        }
        return ClientKS;
    }

    // Implementation of SocketChannel::read(ByteBuffer dst)
    final int read(final ByteBuffer dst) throws IOException
    {
        checkEngine();
        decryptNetworkData(true);

        // this checks whether the server side initiated key renegotiation and thus new handshake started
        if ((_engine.getHandshakeStatus() != HandshakeStatus.FINISHED) && (_engine.getHandshakeStatus() != HandshakeStatus.NOT_HANDSHAKING))
        {
            performHandshake();
        }

        readCount = 0;
        // if _appRecvBuffer has data, then empty it into destination buffers dst
        if (_appRecvBuffer.position() > 0)
        {
            // make _appRecvBuffer readable
            _appRecvBuffer.flip();

            // empty _appRecvBuffer into the destination buffer dst
            {
                // first empty _appRecvBuffer into dst
                byteTransfer(_appRecvBuffer, dst);
            }

            // setup _appRecvBuffer back to writable (with any 'un-got' data at beginning of _appRecvBuffer)
            _appRecvBuffer.compact();
        }

        return readCount;
    }

    // Transfer bytes remaining in ByteBuffer src to ByteBuffer dst (until dst.remaining()==0)
    private void byteTransfer(ByteBuffer src, ByteBuffer dst)
    {
        while (dst.hasRemaining() && src.hasRemaining())
        {
            dst.put(src.get());
            readCount++;
        }
    }

    // Implementation of SocketChannel::write(ByteBuffer src)
    // (src should be immediately readable)
    final int write(final ByteBuffer src) throws IOException
    {
        checkEngine();
        // clear _appSendBuffer before using it
        _appSendBuffer.clear();

        // mark the src buffer so we can reset (src.reset()) if we want to indicate that nothing was read
        src.mark();

        // empty the source buffer src into _appSendBuffer
        int writeCount = 0;
        while (src.hasRemaining() && _appSendBuffer.hasRemaining())
        {
            _appSendBuffer.put(src.get());
            writeCount++;
        }

        // make _appSendBuffer readable
        _appSendBuffer.flip();

        // encrypt (wrap) the application plaintext (currently in _appSendBuffer)
        SSLEngineResult result = encryptApplicationData();

        // this checks whether the server side initiated key renegotiation and thus new handshake started
        if ((_engine.getHandshakeStatus() != HandshakeStatus.FINISHED) && (_engine.getHandshakeStatus() != HandshakeStatus.NOT_HANDSHAKING))
        {
            performHandshake();
        }

        // check how many bytes we've just used from the src buffer
        if (result.bytesConsumed() == 0)
        {
            // if we didn't consume any bytes, clear _appSendBuffer and reset src
            _appSendBuffer.clear();
            src.reset();

            return 0;
        }

        // return the number of bytes that we used from the src buffer.
        return writeCount;
    }

    // Implementation of AbstractSelectableChannel::write(final ByteBuffer[] srcs, final int offset, final int length)
    final long write(final ByteBuffer[] srcs, final int offset, final int length) throws IOException
    {
        long writeCount = 0;
        for (int i = offset; i < length; i++)
        {
            writeCount += write(srcs[i]);
            if (srcs[i].hasRemaining())
                break;
        }

        return writeCount;
    }

    // We must send the appropriate alerts to indicate to the peer that we intend to close the TLS/SSL connection.
    void cleanup() throws IOException
    {
        if(_appSendBuffer != null){
            _appSendBuffer.clear();
        }

        if (_engine != null)
            _engine.closeOutbound();
        _engine = null;
    }

    // Process the encrypted or decrypted data or process the handshake information
    void performHandshake() throws IOException
    {
        checkEngine();
        boolean handshakeComplete = false;
        // continue handshaking until we've finished.
        while (!handshakeComplete)
        {
            switch (_engine.getHandshakeStatus())
            {
                case FINISHED:
                case NOT_HANDSHAKING:
                    handshakeComplete = true;
                    break;
                case NEED_TASK:
                    executeDelegatedTasks();
                    try
                    {
                        Thread.sleep(100);
                    }
                    catch (InterruptedException e)
                    {
                    }
                    break;
                case NEED_UNWRAP:
                    decryptNetworkData(false);
                    try
                    {
                        Thread.sleep(100);
                    }
                    catch (InterruptedException e)
                    {
                    }
                    break;
                case NEED_WRAP:
                    encryptApplicationData();
                    try
                    {
                        Thread.sleep(100);
                    }
                    catch (InterruptedException e)
                    {
                    }
                    break;
                default:
                	throw new IOException("Invalid handshake status for CryptoHelper.performHandshake()");
            }
        }

        // check certificate(s) validity (maybe has expired?) after handshake is done(a null value implies "X509" algorithm)
        if (_server == false && (_connectionKeyManagerAlgorithm == null || _connectionKeyManagerAlgorithm.endsWith("X509")))
        {
            try
            {
                X509Certificate[] cert = (X509Certificate[])(_engine.getSession().getPeerCertificates());

                for (int i = 0; i < cert.length; i++)
                {
                    try
                    {
                        cert[i].checkValidity();
                    }
                    catch (CertificateExpiredException e) // certificate has expired
                    {
                        throw new IOException("Invalid certificate (Expired):   " + e.getMessage());
                    }
                    catch (CertificateNotYetValidException e) // certificate not yet valid
                    {
                        throw new IOException("Invalid certificate (Not Yet Valid):   " + e.getMessage());
                    }
                }
            }
            catch (SSLPeerUnverifiedException e)
            {
            	 throw new IOException("Missing certificate:   " + e.getMessage());
            }
        }
    }

    // Execute all of the delegated tasks awaiting execution from the Java SSLEngine
    private void executeDelegatedTasks()
    {
        Runnable task;
        while ((task = _engine.getDelegatedTask()) != null)
        {
            task.run();
        }
    }

    // Encrypt (wrap) the application plaintext into network encrypted data or encrypt (wrap) any handshake information.
    private SSLEngineResult encryptApplicationData() throws IOException
    {
        // encrypt (wrap) the application data or the handshake information
        SSLEngineResult result = _engine.wrap(_appSendBuffer, _netSendBuffer);

        // setup the application buffer for writing more to it
        _appSendBuffer.compact();

        // handle exceptional circumstances
        if (result.getStatus() == Status.BUFFER_OVERFLOW)
        {
            // SSLEngine thinks there is insufficient room for the encrypted application data.
            // Since we empty the buffer (_netSendBuffer) into the channel after every wrap(),
            // there is nothing we can do to fix this issue.
            throw new BufferOverflowException();
        }

        // only write if the last wrap operation produced anything
        if (result.bytesProduced() != 0)
        {
            // make _netSendBuffer readable
            _netSendBuffer.flip();

            // empty _netSendBuffer into the channel
            try
            {
                // empty _netSendBuffer
                while (_netSendBuffer.hasRemaining())
                    _socketChannel.write(_netSendBuffer);
            }
            catch (IOException e)
            {
                // disconnect will be caught earlier in etaj connection
                _netSendBuffer.clear();
                return result;
            }
            // setup _netSendBuffer back to writable
            _netSendBuffer.compact();
        }

        return result;
    }

    // Decrypt (unwrap) the network encrypted data into application plaintext.
    private void decryptNetworkData(boolean keepReading) throws IOException
    {
        int bytesReadFromChannel = readFromChannel();
        // System.out.println("CryptoHelper   bytesReadFromChannel=="+bytesReadFromChannel);

        if (bytesReadFromChannel == -1)
            throw new IOException("Tunnel Channel disconnected");

        // make _netRecvBuffer readable
        _netRecvBuffer.flip();

        // decrypt (unwrap) the encrypted data into plaintext first decryption
        SSLEngineResult result = _engine.unwrap(_netRecvBuffer, _appRecvBuffer);

        // check the result of this decryption
        checkUnwrapEngineResultStatus(result);

        // move all decrypted data (that is in _netRecvBuffer) into _appRecvBuffer
        // i.e. keep decrypting (unwrap) until _netRecvBuffer is empty
        // or until we get a BUFFER_UNDERFLOW on the last decryption
        // (we need to later read more encrypted data from the channel into _netRecvBuffer)
        if (keepReading)
        {
            while ((_netRecvBuffer.remaining() != 0) && (result.getStatus() != Status.BUFFER_UNDERFLOW)
                    && (result.getStatus() != Status.BUFFER_OVERFLOW))
            {
                result = _engine.unwrap(_netRecvBuffer, _appRecvBuffer);
                // check the result of this decryption
                checkUnwrapEngineResultStatus(result);
            }
        }

        // setup _netRecvBuffer back to writable
        _netRecvBuffer.compact();
    }

    private void checkUnwrapEngineResultStatus(SSLEngineResult result) throws IOException
    {
        switch (result.getStatus())
        {
            case CLOSED:
                throw new IOException("Tunnel Channel Closed");
                // case BUFFER_OVERFLOW:
                // either _appRecvBuffer is too small for a handshake message,
                // or the client is not properly emptying the application buffer.
                // throw new IOException("Tunnel Channel BufferOverflow");
                // case BUFFER_UNDERFLOW:
                // we need more encrypted data, but we didn't manage to read any from the channel
                // break;
                // case OK:
                // we got all encrypted data?
                // break;
            default:
                break;
        }
    }

    // Attempt to fill _netRecvBuffer from the channel and return the number of bytes filled
    private int readFromChannel() throws IOException
    {
        // fill the _netRecvBuffer
        return _socketChannel.read(_netRecvBuffer);
    }
    
    // Start TLS/SSL handshaking with the server peer.
    void doHandshake() throws IOException
    {
        checkEngine();
        _engine.beginHandshake();
        performHandshake();
    }
    
    private void checkEngine()
    {
        if(_engine == null){
            throw new IllegalStateException("SSL engine is not initialized");
        }
    }

    private String _connectionKeyManagerAlgorithm;
    private SocketChannel _socketChannel;
    private String _hostName;
    private int _hostPort;
    private boolean _server;

    public SSLEngine _engine;

    // the Java SSLEngine buffers
    private ByteBuffer _netRecvBuffer; // for the data (handshake and encrypted) received directly from the network.
                                       // emptied by SSLEngine::unwrap()

    private ByteBuffer _appRecvBuffer; // for the decrypted data received from the other peer.
                                       // filled by SSLEngine::unwrap() with decrypted application data
                                       // and emptied by the application

    private ByteBuffer _appSendBuffer; // for the data (not yet encrypted) to be sent to the other peer.
                                       // filled by the application
                                       // and then emptied by SSLEngine::wrap()

    private ByteBuffer _netSendBuffer; // for the data (handshake and encrypted) to be sent to the network.
                                       // filled by SSLEngine::wrap()
                                       // and emptied by writing it to SocketChannel

    private int readCount; // number of bytes read after each read(ByteBuffer[], offset, length) call
}
