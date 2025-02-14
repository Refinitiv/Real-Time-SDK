/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import java.io.FileInputStream;
import java.io.IOException;
import java.security.KeyStore;

class DefaultKeyStoreFactory implements KeyStoreFactory {
    @Override
    public KeyStore create(EncryptionOptions options) throws IOException {
        try {
            char[] password = options.KeystorePasswd().toCharArray();
            KeyStore keyStore = KeyStore.getInstance(options.KeystoreType());
            try (FileInputStream fis = new FileInputStream(options.KeystoreFile())) {
                keyStore.load(fis, password);
            }
            return keyStore;
        } catch (Exception e) {
            throw new IOException("Error loading or repacking KeyStore for Conscrypt: " + e.getMessage(), e);
        }
    }
}
