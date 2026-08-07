/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.OmmInvalidUsageException;

/**
 * The RDM Directory base message contract for message types that support a filter.
 *
 * @param <T> the EMA message type used to encode or decode the directory message
 *
 * @see DirectoryMsg
 */
public interface DirectoryMsgWithFilter<T> extends DirectoryMsg<T>
{
    /**
     * Filter indicating which filters may appear on this stream. Where possible,
     * this should match the consumer's request. Populated by
     * {@link com.refinitiv.ema.rdm.EmaRdm}.
     *
     * @return filter
     * @throws OmmInvalidUsageException if the filter is optional and not present for this message type
     */
    long filter();

    /**
     * Filter indicating which filters may appear on this stream. Where possible,
     * this should match the consumer's request. Populated by
     * {@link com.refinitiv.ema.rdm.EmaRdm}.
     *
     * @param filter the filter
     * @return this directory message instance
     */
    DirectoryMsgWithFilter<T> filter(long filter);

    /**
     * Returns the filter as a formatted string containing the selected
     * directory service filter names.
     *
     * @return formatted filter string
     */
    String filterAsString();
}

