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
import java.io.IOException;
import java.security.*;

class DefaultKeyManagerFactoryCreator implements KeyManagerFactoryCreator {
    public KeyManagerFactory create(EncryptionOptions options, KeyStore keyStore) throws IOException {
        String keyManagerAlgorithm = options.KeyManagerAlgorithm() == null || options.KeyManagerAlgorithm().equals("")
                ? options.DefaultKeyManagerAlgorithm()
                : options.KeyManagerAlgorithm();

        String securityProvider = options.SecurityProvider() == null || options.SecurityProvider().equals("")
                ? options.DefaultSecurityProvider()
                : options.SecurityProvider();

        try {
            KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(keyManagerAlgorithm, securityProvider);
            keyManagerFactory.init(keyStore, options.KeystorePasswd().toCharArray());
            return keyManagerFactory;
        } catch (NoSuchAlgorithmException | NoSuchProviderException e) {
            throw new IOException("Error when initializing SSLContext:  " + e.getMessage());
        } catch (UnrecoverableKeyException e) {
            throw new RuntimeException(e);
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        }
    }
}
