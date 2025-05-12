/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import javax.net.ssl.TrustManagerFactory;
import java.io.IOException;
import java.security.KeyStore;

interface TrustManagerFactoryFactory {
    TrustManagerFactory create(EncryptionOptions options, KeyStore keyStore) throws IOException;
}
