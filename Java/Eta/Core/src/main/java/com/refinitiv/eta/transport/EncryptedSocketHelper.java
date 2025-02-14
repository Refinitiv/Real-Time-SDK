package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.crypto.CryptoHelper;
import com.refinitiv.eta.transport.crypto.CryptoHelperFactory;

import java.io.IOException;
import java.net.SocketAddress;
import java.nio.ByteBuffer;

public class EncryptedSocketHelper extends SocketHelper
{
    protected CryptoHelper _crypto;
    private boolean _completedHandshake = false;

    @Override
    public int read(ByteBuffer dst) throws IOException
    {
        checkCrypto();

        return _crypto.read(dst);
    }

    @Override
    public long write(ByteBuffer[] srcs, int offset, int length) throws IOException
    {
        checkCrypto();

        return _crypto.write(srcs, offset, length);
    }

    @Override
    public int write(ByteBuffer src) throws IOException
    {
        checkCrypto();

        return _crypto.write(src);
    }

    @Override
    public boolean connect(SocketAddress remote, boolean proxy) throws IOException
    {
        checkCrypto();
        boolean result = super.connect(remote, proxy);
        if(proxy == false) {
            _crypto.initializeEngine(_socket);
            _completedHandshake = false;
            _completedProxy = true;
        }
        return result;
    }

    @Override
    public void initialize(ConnectOptions options) throws IOException
    {
        _crypto = CryptoHelperFactory.createClient(options);
        super.initialize(options);
        _completedHandshake = false;
    }


    private static EncryptionOptions convert(ServerEncryptionOptions serverOptions)
    {
        EncryptionOptionsImpl options = new EncryptionOptionsImpl();
        options.KeyManagerAlgorithm(serverOptions.keyManagerAlgorithm());
        options.TrustManagerAlgorithm(serverOptions.trustManagerAlgorithm());
        options.KeystoreType(serverOptions.keystoreType());
        options.KeystorePasswd(serverOptions.keystorePasswd());
        options.KeystoreFile(serverOptions.keystoreFile());
        options.SecurityProvider(serverOptions.securityProvider());
        options.SecurityProtocol(serverOptions.securityProtocol());
        options.SecurityProtocolVersions(serverOptions.securityProtocolVersions());
        return options;
    }
    
    @Override
    public void initialize(BindOptions options) throws IOException
    {
        _crypto = CryptoHelperFactory.createServer(convert(options.encryptionOptions()));
        super.initialize(options);
        postProxyInit();
        _completedHandshake = false;
    }

    @Override
    public void close() throws IOException
    {
        if (_crypto != null)
            _crypto.cleanup();
        super.close();
    }

    @Override
    public boolean finishConnect() throws IOException
    {
        boolean connected = super.finishConnect();
        if (connected && _completedProxy == true && !_completedHandshake)
        {
            checkCrypto();
            _crypto.doHandshake();
            _completedHandshake = true;
        }
        return connected;
    }

    public boolean postProxyInit() throws IOException
    {
        _completedProxy = true;
        _crypto.initializeEngine(_socket);
        _completedHandshake = false;
        return true;
    }

    private void checkCrypto() throws IOException
    {
        if (_crypto == null)
            throw new IOException("Encryption engine is not set up, check configuration.");
    }
    
    public String getActiveTLSVersion() throws IOException
    {
        if (_crypto == null)
            throw new IOException("Encryption engine is not set up, check configuration.");
        return _crypto.getActiveTLSVersion();
    }
    
    
    /* Assumption here is that the objects are both EncryptedSocketHelpers.  */
    @Override
    public void copy(SocketHelper dstSocket)
    {
    	super.copy(dstSocket);
    	((EncryptedSocketHelper)dstSocket)._crypto = _crypto;
    	((EncryptedSocketHelper)dstSocket)._completedHandshake = _completedHandshake;
    }

}
