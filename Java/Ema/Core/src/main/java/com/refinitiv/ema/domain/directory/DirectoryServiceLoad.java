/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.OmmInvalidUsageException;

/**
 * Represents the RDM Source Directory service load filter.
 * <p>
 * This filter conveys service capacity and workload information, including the
 * maximum number of items a consumer may open, the maximum number of
 * outstanding item requests, and the current load factor of the service.
 *
 * @see DirectoryServiceFilter
 */
public interface DirectoryServiceLoad extends DirectoryServiceFilter<ElementList>
{
    /**
     * Clears this service-load filter and resets it to its default state.
     * <p>
     * This removes all optional load fields, resets {@link #action()} to the
     * default filter-entry action, and restores internal values used by the load
     * filter implementation.
     *
     * @return this directory service load filter instance
     */
    @Override
    DirectoryServiceLoad clear();

    /**
     * Sets the filter-entry action associated with this service-load filter.
     * <p>
     * Valid values are defined by the concrete implementation and typically come
     * from {@link com.refinitiv.ema.access.FilterEntry.FilterAction}.
     *
     * @param action the filter-entry action
     * @return this directory service load filter instance
     */
    @Override
    DirectoryServiceLoad action(int action);

    /**
     * Decodes an {@link ElementList} payload into this service-load filter.
     * <p>
     * Implementations replace the current content with the decoded load-filter
     * data.
     *
     * @param struct encoded {@link ElementList} representing the service-load filter
     * @return this directory service load filter instance
     */
    @Override
    DirectoryServiceLoad decode(ElementList struct);

    /**
     * Returns the maximum number of items a consumer is allowed to open from
     * this service.
     * <p>
     * This is an optional field. Call {@link #checkHasOpenLimit()} before
     * calling this method.
     *
     * @return open limit
     * @throws OmmInvalidUsageException if {@link #checkHasOpenLimit()} returns
     *                                  {@code false}
     */
    long openLimit();

    /**
     * Sets the maximum number of items a consumer is allowed to open from this
     * service.
     * <p>
     * Calling this method marks the optional {@code openLimit} field as present.
     *
     * @param openLimit the open limit
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code openLimit} is outside the range
     *                                  {@code 0} to {@code 4294967295L}
     */
    DirectoryServiceLoad openLimit(long openLimit);

    /**
     * Indicates whether the optional {@code openLimit} field is present.
     *
     * @return {@code true} if {@code openLimit} is present; otherwise,
     *         {@code false}
     */
    boolean checkHasOpenLimit();

    /**
     * Returns the maximum number of item requests a consumer may have
     * outstanding, meaning they are still waiting for a refresh, from this
     * service.
     * <p>
     * This is an optional field. Call {@link #checkHasOpenWindow()} before
     * calling this method.
     *
     * @return open window
     * @throws OmmInvalidUsageException if {@link #checkHasOpenWindow()} returns
     *                                  {@code false}
     */
    long openWindow();

    /**
     * Sets the maximum number of item requests a consumer may have
     * outstanding, meaning they are still waiting for a refresh, from this
     * service.
     * <p>
     * Calling this method marks the optional {@code openWindow} field as present.
     *
     * @param openWindow the open window
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code openWindow} is outside the
     *                                  range {@code 0} to {@code 4294967295L}
     */
    DirectoryServiceLoad openWindow(long openWindow);

    /**
     * Indicates whether the optional {@code openWindow} field is present.
     *
     * @return {@code true} if {@code openWindow} is present; otherwise,
     *         {@code false}
     */
    boolean checkHasOpenWindow();

    /**
     * Returns the load factor, which indicates the current workload of the
     * source providing the data.
     * <p>
     * This is an optional field. Call {@link #checkHasLoadFactor()} before
     * calling this method.
     *
     * @return load factor
     * @throws OmmInvalidUsageException if {@link #checkHasLoadFactor()} returns
     *                                  {@code false}
     */
    long loadFactor();

    /**
     * Sets the load factor, which indicates the current workload of the source
     * providing the data.
     * <p>
     * Calling this method marks the optional {@code loadFactor} field as present.
     *
     * @param loadFactor the load factor
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code loadFactor} is outside the
     *                                  range {@code 0} to {@code 65535}
     */
    DirectoryServiceLoad loadFactor(long loadFactor);

    /**
     * Indicates whether the optional {@code loadFactor} field is present.
     *
     * @return {@code true} if {@code loadFactor} is present; otherwise,
     *         {@code false}
     */
    boolean checkHasLoadFactor();

    /**
     * Replaces the contents of this object with a deep copy of another service
     * load filter.
     * <p>
     * Implementations typically clear the current content before copying values
     * from {@code sourceServiceLoad}.
     *
     * @param sourceServiceLoad source service load filter to copy from
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceLoad} is {@code null}
     */
    DirectoryServiceLoad copy(DirectoryServiceLoad sourceServiceLoad);
}
