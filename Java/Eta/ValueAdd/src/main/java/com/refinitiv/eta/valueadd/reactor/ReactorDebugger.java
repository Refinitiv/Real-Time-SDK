/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2025 LSEG. All rights reserved.    --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.beans.Introspector;
import java.beans.PropertyDescriptor;
import java.io.OutputStream;
import java.util.Arrays;

interface ReactorDebugger {

    String CONNECTION_SERVER_ACCEPT = "Reactor@%d, Server@%d accepts reactor channel@%d on channel fd=%d.]\n";
    String CONNECTION_CHANNEL_DOWN = "Reactor@%d, Reactor channel@%d is DOWN on channel fd=%d.]\n";
    String CONNECTION_CHANNEL_CLOSE = "Reactor@%d, closing reactor channel@%d on channel fd=%d.]\n";
    String CONNECTION_CHANNEL_CLOSE_NUM_OF_CALLS = "Reactor@%d, Reactor channel@%d, number of closing call %d and number of dispatching per channel call %d.]\n";
    String CONNECTION_CHANNEL_UP = "Reactor@%d, Reactor channel@%d is UP on channel fd=%d.]\n";
    String CONNECTION_SESSION_STARTUP_DONE = "Reactor@%d, Reactor channel@%d session startup executed on channel fd=%d, session management enabled: %b]\n";
    String CONNECTION_CONNECTING_PERFORMED = "Reactor@%d, Reactor channel@%d connection executed on channel fd=%d.]\n";
    String CONNECTION_DISCONNECT = "Reactor@%d, Reactor channel@%d disconnected with status = %s on channel fd=%d.]\n";
    String CONNECTION_RECONNECT = "Reactor@%d, Reactor channel@%d reconnecting on channel fd=%d.]\n";
    String CONNECTION_RECONNECT_RDP = "Reactor@%d, Reactor channel@%d reconnecting to RDP on channel fd=%d.]\n";

    String EVENTQUEUE_COUNT_REACTOR = "Reactor@%d, reactor event count %d to be dispatched.]\n";
    String EVENTQUEUE_COUNT_ALL = "Reactor@%d, %d events to be dispatched from active channels event queue.]\n";
    String EVENTQUEUE_COUNT_SPECIFIED = "Reactor@%d, Per Reactor channel@%d event count %d to be dispatched on channel fd=%d.]\n";

    String TUNNELSTREAM_ACCEPT = "Reactor@%d, Reactor channel@%d accepts a tunnel stream request (stream ID=%d) on channel fd=%d.]\n";
    String TUNNELSTREAM_DISPATCH_NOW = "Reactor@%d, Reactor channel@%d put a tunnel stream DISPATCH_NOW event on channel fd=%d.]\n";
    String TUNNELSTREAM_STREAM_REQUEST = "Reactor@%d, Reactor channel@%d receives a tunnel stream request (stream ID=%d) on channel fd=%d.]\n";
    String TUNNELSTREAM_STREAM_REJECT = "Reactor@%d, Reactor channel@%d rejects a tunnel stream request (stream ID=%d) on channel fd=%d.]\n";
    String TUNNELSTREAM_SEND_STREAM_OPEN = "Reactor@%d, Reactor channel@%d submitted a tunnel stream open response (stream ID=%d) on channel fd=%d.]\n";
    String TUNNELSTREAM_STREAM_ESTABLISHED = "Reactor@%d, Reactor channel@%d, Tunnel stream established (stream ID=%d) on channel fd=%d.]\n";
    String TUNNELSTREAM_STREAM_CLOSE = "Reactor@%d, Reactor channel@%d handles tunnel stream closes (stream ID=%d) on channel fd=%d.]\n";

    String PREFERRED_HOST_ALREADY_CONNECTED = "Reactor@%d, Reactor channel@%d is already connected to the preferred host.]\\n";
    String PREFERRED_HOST_CANNOT_SWITCH_DURING_RECONNECTING = "Reactor@%d, Reactor channel@%d cannot fallback to preferred host during reconnection or down state.]\\n";
    /**
     * Gets the fd value associated with the underlying channel
     * @param reactorChannel the ReactorChannel instance whose Channel's fd is evaluated
     * @return the fd value or -1 in case the method was not able to evaluate it
     */
    static int getChannelId(ReactorChannel reactorChannel) {
        try {
            if (reactorChannel.selectableChannel() != null) {
                Integer fd =  (Integer) Arrays.stream(Introspector.getBeanInfo(reactorChannel.selectableChannel().getClass()).getPropertyDescriptors())
                        .filter(pd -> pd.getDisplayName().equals("FDVal"))
                        .map(PropertyDescriptor::getReadMethod)
                        .findFirst()
                        .orElse(null)
                        .invoke(reactorChannel.selectableChannel());
                return fd != null ? fd : -1;
            } else {
                return -1;
            }
        } catch (Exception e) {
            return -1;
        }
    }

    /**
     * Writes the debugging message into the underlying output stream.
     * @param msg generic debugging message
     * @param args arguments to be inserted into the generic debugging message
     */
    void writeDebugInfo(String msg, Object... args);

    /**
     * Increases the CLOSE calls count by 1
     */
    void incNumOfCloseCalls();

    /**
     * Getter for the count of the CLOSE calls
     * @return the number of CLOSE calls
     */
    int getNumOfCloseCalls();

    /**
     * Increases the count of single channel dispatch calls by 1
     */
    void incNumOfDispatchCalls();

    /**
     * Getter for the nymber of single channel dispatch calls
     * @return the nymber of single channel dispatch calls
     */
    int getNumOfDispatchCalls();

    /**
     * Getter for the output stream used by the ReactorDebugger instance
     * @return the OutputStream instance
     */
    OutputStream getOutputStream();

    /**
     * In case the underlying stream is an instance of the ByteArrayOutputStream, returns the byte array containing debugging messages
     * written up to this point from the beginning of debugging or since the last call to this method. The call to this method discards all
     * messages present in the stream.
     * @return byte array with debugging information or null in case the underlying stream is not an instance of ByteArrayOutputStream
     */
    byte[] toByteArray();
}
