/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import java.io.IOException;
import java.security.*;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.AbstractMap;
import java.util.Enumeration;
import java.util.Map;

class ConscryptServerKeyStoreFactory implements KeyStoreFactory {
    private KeyStoreFactory wrapped;

    public ConscryptServerKeyStoreFactory(KeyStoreFactory wrapped) {
        this.wrapped = wrapped;
    }

    @Override
    public KeyStore create(EncryptionOptions options) throws IOException {
        try {
            KeyStore originalKeyStore = wrapped.create(options);
            Map.Entry<PrivateKey, X509Certificate> entry = null;
            KeyStore keyStore = null;
            try {
                entry = getCertificateWithPrivateKey(originalKeyStore, options.KeystorePasswd().toCharArray());
                keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
                keyStore.load(null, null);
            } catch (Exception e) {
                throw new IOException("Error when loading keystore:  " + e.getMessage());
            }
            keyStore.setKeyEntry("server", entry.getKey(), options.KeystorePasswd().toCharArray(), new java.security.cert.Certificate[]{entry.getValue()});
            return keyStore;
        } catch (Exception e) {
            throw new IOException("Error loading or repacking KeyStore for Conscrypt: " + e.getMessage(), e);
        }
    }

    private static Map.Entry<PrivateKey, X509Certificate> getCertificateWithPrivateKey(KeyStore keyStore, char[] keyPassword) throws Exception {
        try {
            Enumeration<String> aliases = keyStore.aliases();
            while (aliases.hasMoreElements()) {
                String alias = aliases.nextElement();
                if (keyStore.isKeyEntry(alias)) {
                    Key key = keyStore.getKey(alias, keyPassword);
                    if (key instanceof PrivateKey) {
                        Certificate certificate = keyStore.getCertificate(alias);
                        // Ensure the certificate is an X509Certificate
                        if (certificate instanceof X509Certificate && key instanceof PrivateKey) {
                            return new AbstractMap.SimpleEntry<>((PrivateKey) key, (X509Certificate) certificate);
                        } else {
                            throw new IOException("The retrieved certificate is not an X509Certificate or PrivateKey is not present");
                        }
                    }
                }
            }
        } catch (UnrecoverableKeyException e) {
            throw new IOException("Key password is wrong");
        } catch (KeyStoreException | RuntimeException e) {
            throw new RuntimeException(e);
        }
        throw new IOException("The retrieved certificate is not an X509Certificate or PrivateKey is not present");
    }
}
