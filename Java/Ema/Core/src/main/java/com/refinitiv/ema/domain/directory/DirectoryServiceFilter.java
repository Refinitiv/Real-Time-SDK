/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.rdm.EmaRdm;

/**
 * Base contract for an RDM Source Directory service filter.
 * <p>
 * A service filter encapsulates one portion of a Source Directory service definition,
 * such as service info, state, load, data, group, or link information.
 * Implementations expose their content as an EMA container type that can be encoded
 * into, or decoded from, a directory payload.
 *
 * @param <T> EMA container type used to represent the encoded filter payload,
 *            for example {@link com.refinitiv.ema.access.ElementList} or
 *            {@link com.refinitiv.ema.access.Map}
 */
public interface DirectoryServiceFilter<T>
{
    /**
     * Clears this service filter and resets it to its default state.
     *
     * @return this service filter instance
     */
    DirectoryServiceFilter<T> clear();

    /**
     * Returns the action associated with this service filter.
     * <p>
     * The valid action values depend on the concrete implementation. Most service
     * filters use {@link com.refinitiv.ema.access.FilterEntry.FilterAction} values.
     *
     * @return action
     */
    int action();

    /**
     * Sets the action associated with this service filter.
     * <p>
     * The valid action values depend on the concrete implementation. Most service
     * filters use {@link com.refinitiv.ema.access.FilterEntry.FilterAction} values.
     *
     * @param action the action
     * @return this service filter instance
     * @throws OmmInvalidUsageException if {@code action} is not one of the
     *                                  supported actions
     */
    DirectoryServiceFilter<T> action(int action);

    /**
     * Returns the fixed RDM filter identifier for this service filter type.
     * <p>
     * The returned value corresponds to one of the service filter identifiers defined
     * by {@link EmaRdm}.
     *
     * @return filterId
     */
    int filterId();

    /**
     * Encodes this service filter into its EMA container representation.
     *
     * @return encoded container representing this service filter
     */
    T encode();

    /**
     * Decodes an EMA container representation into this service filter.
     * <p>
     * Implementations typically replace their current content with the decoded data.
     *
     * @param struct encoded container representing the service filter
     * @return this service filter instance
     *
     * @throws OmmInvalidUsageException if {@code struct} is invalid or the filter
     *                                  cannot be decoded
     */
    DirectoryServiceFilter<T> decode(T struct);
}
