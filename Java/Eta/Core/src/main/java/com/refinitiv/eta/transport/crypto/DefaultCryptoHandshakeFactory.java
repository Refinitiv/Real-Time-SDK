/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import javax.net.ssl.SSLEngine;
import java.io.IOException;

class DefaultCryptoHandshakeFactory implements CryptoHandshakeFactory {
    public CryptoHandshake create(SSLEngine engine, BufferData bufferData) throws IOException {
        return new DefaultCryptoHandshake(engine, bufferData);
    }
}
