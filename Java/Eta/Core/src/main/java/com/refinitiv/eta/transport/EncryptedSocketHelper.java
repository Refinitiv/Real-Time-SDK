package com.refinitiv.eta.transport;

import java.io.IOException;
import java.net.SocketAddress;
import java.nio.ByteBuffer;

public class EncryptedSocketHelper extends SocketHelper
{
    private CryptoHelper _crypto;
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
        _crypto = new CryptoHelper(options);
        super.initialize(options);
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
}
