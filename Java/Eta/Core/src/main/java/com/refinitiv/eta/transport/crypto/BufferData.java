/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import javax.net.ssl.SSLSession;
import java.nio.ByteBuffer;

public class BufferData {
    public final SafeBuffer netRecv;
    public final SafeBuffer appRecv;
    public final SafeBuffer appSend;
    public final SafeBuffer netSend;

    private BufferData(int netBufferSize, int appBufferSize) {
        netRecv = new SafeBuffer(ByteBuffer.allocateDirect(4 * netBufferSize));
        appRecv = new SafeBuffer(ByteBuffer.allocateDirect(4 * appBufferSize));
        appSend = new SafeBuffer(ByteBuffer.allocateDirect(2 * appBufferSize));
        netSend = new SafeBuffer(ByteBuffer.allocateDirect(2 * netBufferSize));
    }

    public static BufferData create(SSLSession sslSession) {
        // get the largest possible buffer size for the application data buffers that are used for Java SSLEngine
        final int appBufferSize = sslSession.getApplicationBufferSize();

        // get the largest possible buffer size for the network data buffers that are used for Java SSLEngine
        final int sslBufferSize = sslSession.getPacketBufferSize();

        return new BufferData(sslBufferSize, appBufferSize);
    }
}
