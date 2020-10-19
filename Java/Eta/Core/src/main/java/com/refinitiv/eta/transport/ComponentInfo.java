package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Buffer;

/**
 * Connected Component Information, used to identify components from across the connection.
 *
 * @see ChannelInfo
 */
public interface ComponentInfo
{
    /**
     * Get the Buffer holding the component version.
     * @return {@link Buffer}
     */
    public Buffer componentVersion();
}
