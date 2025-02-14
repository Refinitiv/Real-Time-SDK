/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;


import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.EncryptionOptions;
import org.conscrypt.Conscrypt;

import java.security.Security;
import java.util.function.Function;

public class CryptoHelperFactory
{
    public static CryptoHelper createClient(ConnectOptions options)
    {
        assert (options != null) : "options cannot be null";
        String hostName = options.unifiedNetworkInfo().address();
        int hostPort = getPort(options);

        DefaultKeyStoreFactory defaultKeyStoreManager = new DefaultKeyStoreFactory();
        Function<SSLContextFactory, SSLEngineFactory> createSSLEngineManager =
                context -> new ClientSSLEngineFactory(hostName, hostPort, options.encryptionOptions(), context);
        if(isConscrypt(options.encryptionOptions()))
        {
            EnsureConscrypt();
            return create(
                    new ConscryptClientKeyStoreFactory(defaultKeyStoreManager),
                    new ConscryptTrustManagerFactoryFactory(),
                    ClientHandshakeFactory.create(options.encryptionOptions(), new DefaultCryptoHandshakeFactory()),
                    new ConscryptKeyManagerFactoryCreator(),
                    createSSLEngineManager);
        }else
        {
            return create(
                    defaultKeyStoreManager,
                    new DefaultTrustManagerFactoryFactory(),
                    ClientHandshakeFactory.create(options.encryptionOptions(), new DefaultCryptoHandshakeFactory()),
                    new DefaultKeyManagerFactoryCreator(),
                    createSSLEngineManager);
        }
    }
    private static void EnsureConscrypt()
    {
        if(Security.getProvider("Conscrypt") == null)
        {
            Security.addProvider(Conscrypt.newProvider());
        }
    }
    public static CryptoHelper createServer(EncryptionOptions options)
    {
        assert (options != null) : "serverOptions cannot be null";
        DefaultKeyStoreFactory defaultKeyStoreManager = new DefaultKeyStoreFactory();
        Function<SSLContextFactory, SSLEngineFactory> createSSLEngineManager = context -> new ServerSSLEngineFactory(context, options);
        if(isConscrypt(options))
        {
            EnsureConscrypt();
            return create(
                    new ConscryptServerKeyStoreFactory(defaultKeyStoreManager),
                    new ConscryptTrustManagerFactoryFactory(),
                    new DefaultCryptoHandshakeFactory(),
                    new ConscryptKeyManagerFactoryCreator(),
                    createSSLEngineManager);
        }else
        {
            return create(
                    defaultKeyStoreManager,
                    new DefaultTrustManagerFactoryFactory(),
                    new DefaultCryptoHandshakeFactory(),
                    new DefaultKeyManagerFactoryCreator(),
                    createSSLEngineManager);
        }
    }


    private static CryptoHelper create(
            KeyStoreFactory keyStoreFactory,
            TrustManagerFactoryFactory trustManagerFactoryFactory,
            CryptoHandshakeFactory cryptoHandshakeFactory,
            KeyManagerFactoryCreator keyManagerFactoryCreator,
            Function<SSLContextFactory, SSLEngineFactory> createSSLEngineManager)
    {
        SSLContextFactory sslContextFactory = new SSLContextFactory(keyStoreFactory, keyManagerFactoryCreator, trustManagerFactoryFactory);
        return new CryptoHelper(createSSLEngineManager.apply(sslContextFactory), cryptoHandshakeFactory);
    }

    private static boolean isConscrypt(EncryptionOptions encryptionOptions)
    {
        return encryptionOptions.SecurityProvider() != null
                && encryptionOptions.SecurityProvider().equalsIgnoreCase("Conscrypt");
    }

    private static int getPort(ConnectOptions options)
    {
        try
        {
            // the service is specified as a port number
            return Integer.parseInt(options.unifiedNetworkInfo().serviceName());
        }
        catch (Exception e)
        {
            // the service is a name
            return com.refinitiv.eta.transport.GetServiceByName.getServiceByName(options.unifiedNetworkInfo().serviceName());
        }
    }
}