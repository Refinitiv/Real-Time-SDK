/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.OmmInvalidUsageException;

import java.util.List;

/**
 * Represents the RDM Source Directory service link filter.
 * <p>
 * This filter conveys the set of upstream sources associated with a service.
 * Its payload is encoded as a {@link Map} whose keys are link names and whose
 * entry payloads are the {@link com.refinitiv.ema.access.ElementList} values
 * produced by {@link DirectoryServiceLink#encode()}.
 * <p>
 * The {@linkplain #action() action} inherited from {@link DirectoryServiceFilter}
 * uses {@link com.refinitiv.ema.access.MapEntry.MapAction} values, and
 * {@link #filterId()} always returns
 * {@link com.refinitiv.ema.rdm.EmaRdm#SERVICE_LINK_ID}.
 *
 * @see DirectoryServiceFilter
 */
public interface DirectoryServiceLinkInfo extends DirectoryServiceFilter<Map>
{
    /**
     * Clears this service link-information filter and resets it to its default state.
     * <p>
     * This removes all link entries and restores the default map-entry action for
     * the enclosing link filter.
     *
     * @return this directory service link-info filter instance
     */
    @Override
    DirectoryServiceLinkInfo clear();

    /**
     * Sets the map-entry action associated with this service link-information filter.
     * <p>
     * Valid values typically come from
     * {@link com.refinitiv.ema.access.MapEntry.MapAction}.
     *
     * @param action the map-entry action
     * @return this directory service link-info filter instance
     */
    @Override
    DirectoryServiceLinkInfo action(int action);

    /**
     * Decodes a {@link Map} payload into this service link-information filter.
     * <p>
     * Implementations replace the current link list with the decoded link-map
     * content.
     *
     * @param struct encoded {@link Map} representing the service link-information filter
     * @return this directory service link-info filter instance
     */
    @Override
    DirectoryServiceLinkInfo decode(Map struct);

    /**
     * Returns the service-link entries that describe the service's upstream
     * sources.
     * <p>
     * Each entry in the returned list corresponds to one link-map entry, keyed
     * by {@link DirectoryServiceLink#name()}.
     *
     * @return list of service-link entries
     */
    List<DirectoryServiceLink> linkList();

    /**
     * Replaces this filter's service-link entries with the supplied list.
     * <p>
     * Implementations clear the current list before adding the supplied
     * entries. The list reference itself is not retained.
     *
     * @param linkList list of service-link entries
     * @return this directory service link-info filter instance
     * @throws OmmInvalidUsageException if {@code linkList} is {@code null}
     */
    DirectoryServiceLinkInfo linkList(List<DirectoryServiceLink> linkList);

    /**
     * Replaces the contents of this object with a deep copy of another service
     * link filter.
     * <p>
     * On success, this object is cleared and then populated with the source
     * filter's {@linkplain #action() action} and deep copies of each
     * {@link DirectoryServiceLink} entry.
     *
     * @param sourceServiceLinkInfo source service link filter to copy from
     * @return this directory service link-info filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceLinkInfo} is {@code null}
     */
    DirectoryServiceLinkInfo copy(DirectoryServiceLinkInfo sourceServiceLinkInfo);
}
