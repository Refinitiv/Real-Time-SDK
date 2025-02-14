/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;
import java.io.IOException;
import java.security.*;

class SSLContextFactory {
    private KeyStoreFactory _keyStoreFactory;
    private KeyManagerFactoryCreator _keyManagerFactoryCreator;
    private TrustManagerFactoryFactory _trustManagerFactoryFactory;

    public SSLContextFactory(KeyStoreFactory keyStoreFactory,
                             KeyManagerFactoryCreator keyManagerFactoryCreator,
                             TrustManagerFactoryFactory trustManagerFactoryFactory) {
        _keyStoreFactory = keyStoreFactory;
        _keyManagerFactoryCreator = keyManagerFactoryCreator;
        _trustManagerFactoryFactory = trustManagerFactoryFactory;
    }
    public SSLContext create(EncryptionOptions options) throws IOException {
        KeyStore keyStore = _keyStoreFactory.create(options);
        KeyManagerFactory keyManagerFactory = _keyManagerFactoryCreator.create(options, keyStore);
        TrustManagerFactory trustManagerFactory = _trustManagerFactoryFactory.Create(options, keyStore);

        String securityProtocol = options.SecurityProtocol() == null || options.SecurityProtocol().equals("")
                ? options.DefaultSecurityProtocol()
                : options.SecurityProtocol();

        String securityProvider = options.SecurityProvider() == null || options.SecurityProvider().equals("")
                ? options.DefaultSecurityProvider()
                : options.SecurityProvider();

        try {
            SSLContext sslContext = SSLContext.getInstance(securityProtocol, securityProvider);
            sslContext.init(keyManagerFactory.getKeyManagers(), trustManagerFactory.getTrustManagers(), null);
            return sslContext;
        } catch (NoSuchAlgorithmException e) {
            throw new IOException(e);
        } catch (KeyManagementException e) {
            throw new IOException(e);
        } catch (NoSuchProviderException e) {
            throw new IOException(e);
        }
    }
}
