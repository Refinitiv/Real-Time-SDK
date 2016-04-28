package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.codec.Buffer;

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
