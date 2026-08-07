/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryMsgWithFilter;
import com.refinitiv.ema.rdm.EmaRdm;

/**
 * Shared base implementation for directory messages that support filters.
 * <p>
 * This class extends {@link DirectoryMsgImpl} with common filter storage and
 * default filter accessor behavior for filter-capable directory message
 * implementations. Subclasses may override these methods when their filter
 * semantics are more specific, such as optional filter presence.
 */
abstract class DirectoryMsgWithFilterImpl<T> extends DirectoryMsgImpl<T> implements DirectoryMsgWithFilter<T>
{
    private final StringBuilder stringBuilder = new StringBuilder();
    /** Filter value for the current directory message. */
    protected long filter;

    /**
     * Returns the current filter value.
     *
     * @return the current filter value
     */
    @Override
    public long filter()
    {
        return filter;
    }

    /**
     * Sets the filter value for the current directory message.
     *
     * @param filter the filter value to store
     * @return this directory message instance
     */
    @Override
    public DirectoryMsgWithFilter<T> filter(long filter)
    {
        this.filter = filter;
        return this;
    }

    /**
     * Clears the current contents of this directory message, including the stored filter,
     * and prepares it for reuse.
     *
     * @return this directory message instance
     */
    @Override
    public DirectoryMsgWithFilter<T> clear()
    {
        super.clear();
        filter = 0;
        return this;
    }

    @Override
    public String filterAsString()
    {
        stringBuilder.setLength(0);

        stringBuilder.append(TAB);
        stringBuilder.append("filter: ");
        boolean addOr = false;
        if (filter == 0)
        {
            stringBuilder.append("N/A");
        }
        else
        {
            if ((filter & EmaRdm.SERVICE_INFO_FILTER) != 0)
            {
                stringBuilder.append("INFO");
                addOr = true;
            }
            if ((filter & EmaRdm.SERVICE_DATA_FILTER) != 0)
            {
                if (addOr)
                    stringBuilder.append(" | ");
                stringBuilder.append("DATA");
                addOr = true;
            }
            if ((filter & EmaRdm.SERVICE_GROUP_FILTER) != 0)
            {
                if (addOr)
                    stringBuilder.append(" | ");
                stringBuilder.append("GROUP");
                addOr = true;
            }
            if ((filter & EmaRdm.SERVICE_LINK_FILTER) != 0)
            {
                if (addOr)
                    stringBuilder.append(" | ");
                stringBuilder.append("LINK");
                addOr = true;
            }
            if ((filter & EmaRdm.SERVICE_LOAD_FILTER) != 0)
            {
                if (addOr)
                    stringBuilder.append(" | ");
                stringBuilder.append("LOAD");
                addOr = true;
            }
            if ((filter & EmaRdm.SERVICE_STATE_FILTER) != 0)
            {
                if (addOr)
                    stringBuilder.append(" | ");
                stringBuilder.append("STATE");
            }
        }
        stringBuilder.append(EOL);

        return stringBuilder.toString();
    }
}

