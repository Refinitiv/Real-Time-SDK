/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
