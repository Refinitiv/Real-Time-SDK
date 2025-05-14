/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

class ComponentInfoImpl implements ComponentInfo
{
    Buffer _componentVersion = CodecFactory.createBuffer();

    @Override
    public Buffer componentVersion()
    {
        return _componentVersion;
    }
    
    /* Create a clone of the ComponentInfo. 
     * This is a deep copy where new memory is created for all members.
     * 
     * Returns a new ComponentInfo object.
     */
    @Override
    protected ComponentInfo clone()
    {
        ComponentInfoImpl clonedCi = new ComponentInfoImpl();

        clonedCi.componentVersion(componentVersion());

        return clonedCi;
    }
    
    /* This method will copy the specified componentVersion into the internal componentVersion.
     * Backing memory will be allocated and data from the specified componentVersion will be copied to internal.
     * 
     * returns TransportReturnCodes.SUCCESS or TransportReturnCodes.FAILURE
     */
    int componentVersion(Buffer componentVersion)
    {
        assert (componentVersion != null);

        int len = componentVersion.length();
        ByteBuffer backingBuffer = ByteBuffer.allocate(componentVersion.length());

        int ret = componentVersion.copy(backingBuffer);
        if (ret != TransportReturnCodes.SUCCESS)
            return ret;

        _componentVersion.data(backingBuffer, 0, len);

        return TransportReturnCodes.SUCCESS;
    }
}
