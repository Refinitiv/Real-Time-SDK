/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryMsg;
import com.refinitiv.ema.rdm.EmaRdm;

/**
 * Minimal shared base implementation for directory messages that do not require
 * filter-specific behavior.
 * <p>
 * This class provides the common directory domain type, shared stream ID
 * storage and accessor implementations, a base {@link #clear()} implementation
 * that resets the stored stream ID, and shared formatting helpers used by
 * concrete directory message implementations. Directory message types that
 * support filters extend {@link DirectoryMsgWithFilterImpl}.
 */
abstract class DirectoryMsgImpl<T> implements DirectoryMsg<T>
{
    /** Stored stream ID for the current directory message instance. */
    private int streamId = -1;
    
    private final StringBuilder stringBuilder = new StringBuilder();
    protected final static String EOL = System.lineSeparator();
    protected final static String TAB = "\t";

    @Override
    public int domainType()
    {
        return EmaRdm.MMT_DIRECTORY;
    }
    
    @Override
    public int streamId()
    {
        return streamId;
    }
    
    /**
     * Sets the stream ID of this directory message.
     *
     * @param streamId the stream ID
     * @return this directory message instance
     */
    @Override
    public DirectoryMsg<T> streamId(int streamId)
    {
        this.streamId = streamId;
        return this;
    }
    
    /**
     * Clears the current contents of this directory message and prepares it for reuse.
     *
     * @return this directory message instance
     */
    @Override
    public DirectoryMsg<T> clear()
    {
        streamId = -1;
        return this;
    }
    
    protected StringBuilder buildStringBuilder()
    {
        stringBuilder.setLength(0);

        stringBuilder.append(TAB)
                .append("streamId: ")
                .append(streamId() == -1 ? "N/A" : streamId())
                .append(EOL);

        return stringBuilder;
    }
}
