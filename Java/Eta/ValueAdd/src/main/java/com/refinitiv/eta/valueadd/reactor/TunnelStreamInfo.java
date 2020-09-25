package com.refinitiv.eta.valueadd.reactor;

/**
 * Tunnel stream Info available through {@link TunnelStream#info(TunnelStreamInfo, ReactorErrorInfo)} method call.
 *
 * @see TunnelStream
 */
public interface TunnelStreamInfo {

    /**
     * Get buffers used count.
     *
     * @return total buffers used - both big and ordinary
     */
    int buffersUsed();

    /**
     * Get buffers used count.
     *
     * @return ordinary buffers used
     */
    int ordinaryBuffersUsed();

    /**
     * Get buffers used count.
     *
     * @return big buffers used
     */
    int bigBuffersUsed();

    /**
     * Clears buffer Info.
     */
    void clear();
}
