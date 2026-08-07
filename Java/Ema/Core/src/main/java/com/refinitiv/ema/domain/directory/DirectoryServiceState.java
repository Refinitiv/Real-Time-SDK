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
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.rdm.EmaRdm;

/**
 * Represents the RDM Source Directory service state filter.
 * <p>
 * This filter conveys whether a service is up or down, whether it is currently
 * accepting new item requests, and an optional status that applies to all items
 * provided by the service.
 *
 * @see DirectoryServiceFilter
 */
public interface DirectoryServiceState extends DirectoryServiceFilter<ElementList>
{
    /**
     * Clears this service-state filter and resets it to its default state.
     * <p>
     * This removes all optional fields, resets {@link #action()} to the default
     * filter-entry action, and restores {@link #serviceState()} to its default
     * value.
     *
     * @return this directory service state filter instance
     */
    @Override
    DirectoryServiceState clear();

    /**
     * Sets the filter-entry action associated with this service-state filter.
     * <p>
     * Valid values are defined by the concrete implementation and typically come
     * from {@link com.refinitiv.ema.access.FilterEntry.FilterAction}.
     *
     * @param action the filter-entry action
     * @return this directory service state filter instance
     */
    @Override
    DirectoryServiceState action(int action);

    /**
     * Decodes an {@link ElementList} payload into this service-state filter.
     * <p>
     * Implementations replace the current content with the decoded state-filter
     * data.
     *
     * @param struct encoded {@link ElementList} representing the service-state filter
     * @return this directory service state filter instance
     */
    @Override
    DirectoryServiceState decode(ElementList struct);

    /**
     * Replaces the contents of this object with a deep copy of another service
     * state filter.
     * <p>
     * Implementations typically clear the current content before copying values
     * from {@code sourceServiceState}.
     *
     * @param sourceServiceState source service state filter to copy from
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceState} is {@code null}
     */
    DirectoryServiceState copy(DirectoryServiceState sourceServiceState);

    /**
     * Returns the current service state.
     * <p>
     * This field is always available. After {@link #clear()}, the default value
     * is {@link EmaRdm#SERVICE_UP}.
     *
     * @return {@link EmaRdm#SERVICE_UP} if the service is up, or
     * {@link EmaRdm#SERVICE_DOWN} if the service is down.
     */
    int serviceState();

    /**
     * Sets the current service state.
     *
     * @param serviceState the service state. Valid values are
     *                     {@link EmaRdm#SERVICE_UP} and {@link EmaRdm#SERVICE_DOWN}
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if serviceState is not one of the supported values
     */
    DirectoryServiceState serviceState(int serviceState);

    /**
     * Returns whether the service is accepting new item requests.
     * <p>
     * This is an optional field. Call {@link #checkHasAcceptingRequests()} before
     * calling this method.
     *
     * @return {@link EmaRdm#SERVICE_YES} if the service accepts new requests, or
     * {@link EmaRdm#SERVICE_NO} if it does not.
     * @throws OmmInvalidUsageException if {@link #checkHasAcceptingRequests()}
     *                                  returns {@code false}
     */
    int acceptingRequests();


    /**
     * Sets whether the service is accepting new item requests.
     * <p>
     * Calling this method marks the optional {@code acceptingRequests} field as
     * present.
     *
     * @param acceptingRequests the accepting requests flag. Valid values are
     *                          {@link EmaRdm#SERVICE_YES} and {@link EmaRdm#SERVICE_NO}
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if acceptingRequests is not one of the supported values
     */
    DirectoryServiceState acceptingRequests(int acceptingRequests);

    /**
     * Indicates whether the optional {@code acceptingRequests} field is present.
     *
     * @return {@code true} if {@code acceptingRequests} is present; otherwise,
     *         {@code false}
     */
    boolean checkHasAcceptingRequests();

    /**
     * Returns the status that applies to all items provided by this service.
     * <p>
     * This is an optional field. Call {@link #checkHasStatus()} before calling
     * this method.
     *
     * @return service status
     * @throws OmmInvalidUsageException if {@link #checkHasStatus()} returns
     *                                  {@code false}
     */
    OmmState status();

    /**
     * Sets the status that applies to all items provided by this service.
     * <p>
     * Calling this method marks the optional {@code status} field as present.
     *
     * @param status service status
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code status} is {@code null}
     */
    DirectoryServiceState status(OmmState status);

    /**
     * Sets the status that applies to all items provided by this service.
     * <p>
     * Calling this method marks the optional {@code status} field as present.
     *
     * @param streamState {@link OmmState.StreamState} value
     * @param dataState {@link OmmState.DataState} value
     * @param statusCode {@link OmmState.StatusCode} value
     * @param statusText status text
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null}
     */
    DirectoryServiceState status(int streamState, int dataState, int statusCode, String statusText);

    /**
     * Indicates whether the optional {@code status} field is present.
     *
     * @return {@code true} if {@code status} is present; otherwise,
     *         {@code false}
     */
    boolean checkHasStatus();
}
