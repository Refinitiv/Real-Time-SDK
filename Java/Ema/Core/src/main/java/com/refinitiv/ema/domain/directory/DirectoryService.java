/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.*;

import java.util.List;

/**
 * Represents a single RDM directory service entry and its optional service filters.
 * <p>
 * A service may contain {@link DirectoryServiceInfo}, {@link DirectoryServiceState},
 * {@link DirectoryServiceGroup}, {@link DirectoryServiceLoad},
 * {@link DirectoryServiceData}, and {@link DirectoryServiceLinkInfo} content.
 * The {@link #serviceId()} and {@link #action()} values identify the enclosing
 * directory map entry and are not encoded by {@link #encode()}.
 *
 * @see DirectoryServiceInfo
 * @see DirectoryServiceState
 * @see DirectoryServiceGroup
 * @see DirectoryServiceLoad
 * @see DirectoryServiceData
 * @see DirectoryServiceLinkInfo
 */
public interface DirectoryService
{

    /**
     * Clears all optional filter content from this service.
     * <p>
     * This resets all filter presence flags, clears the group-state list, restores
     * {@link #action()} to its default add state, and resets {@link #serviceId()} to
     * its default value.
     *
     * @return this directory service instance
     */
    DirectoryService clear();

    /**
     * Returns the action associated with this service's enclosing directory map entry.
     * <p>
     * Typical values are defined by {@link MapEntry.MapAction}.
     *
     * @return the map entry action for this service
     */
    int action();

    /**
     * Sets the action associated with this service's enclosing directory map entry.
     * <p>
     * Typical values are defined by {@link MapEntry.MapAction}.
     *
     * @param action the map entry action for this service
     * @return this directory service instance
     */
    DirectoryService action(int action);

    /**
     * Checks the presence of the info field.
     *
     * @return true - if info field exists, false - if not.
     */
    boolean checkHasInfo();

    /**
     * Checks the presence of the data field.
     *
     * @return true - if data field exists, false - if not.
     */
    boolean checkHasData();

    /**
     * Checks the presence of the load field.
     *
     * @return true if the load field exists; false otherwise
     */
    boolean checkHasLoad();

    /**
     * Checks the presence of the link field.
     *
     * @return true - if link field exists, false - if not.
     */
    boolean checkHasLink();

    /**
     * Checks the presence of the state field.
     *
     * @return true - if state field exists, false - if not.
     */
    boolean checkHasState();

    /**
     * Encodes this service's filter payload as an EMA {@link FilterList}.
     * <p>
     * The returned {@link FilterList} contains only the service filters currently present on
     * this object. The enclosing directory map entry metadata, including {@link #serviceId()}
     * and {@link #action()}, is not part of the encoded data produced by this method.
     *
     * @return a {@link FilterList} representing this service's filter payload
     */
    FilterList encode();

    /**
     * Decodes an EMA {@link FilterList} into this service's filter payload.
     * <p>
     * This implementation preserves the current {@link #serviceId()} and {@link #action()} values,
     * clears the object, decodes only the supplied filter payload, and then restores the preserved
     * metadata. The supplied {@link FilterList} therefore affects only service filter content.
     *
     * @param filterList the {@link FilterList} representing a service filter payload
     * @return this directory service instance
     *
     * @throws OmmInvalidUsageException if {@code filterList} is {@code null} or decoding fails
     */
    DirectoryService decode(FilterList filterList);

    /**
     * Returns the numeric identifier for this service.
     * <p>
     * This value identifies the enclosing directory map entry and is not part of the
     * {@link FilterList} produced by {@link #encode()} or consumed by {@link #decode(FilterList)}.
     *
     * @return the service identifier
     */
    int serviceId();

    /**
     * Sets the numeric identifier for this service.
     * <p>
     * This value identifies the enclosing directory map entry and is not part of the
     * {@link FilterList} produced by {@link #encode()} or consumed by {@link #decode(FilterList)}.
     *
     * @param serviceId the service identifier
     * @return this directory service instance
     */
    DirectoryService serviceId(int serviceId);

    /**
     * Returns the info filter for this service.
     *
     * @return the info filter
     * @throws OmmInvalidUsageException if {@link #checkHasInfo()} returns {@code false}
     */
    DirectoryServiceInfo info();

    /**
     * Sets the info filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param info the info filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code info} is {@code null} or the copy operation fails
     */
    DirectoryService info(DirectoryServiceInfo info);

    /**
     * Returns the state filter for this service.
     *
     * @return the state filter
     * @throws OmmInvalidUsageException if {@link #checkHasState()} returns {@code false}
     */
    DirectoryServiceState state();

    /**
     * Sets the state filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param state the service state filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code state} is {@code null} or the copy operation fails
     */
    DirectoryService state(DirectoryServiceState state);

    /**
     * Returns the current group-state entries for this service.
     * <p>
     * The returned list is the mutable list used by this service; changes made through the
     * returned list are reflected by this object.
     *
     * @return the current group-state entry list
     */
    List<DirectoryServiceGroup> groupStateList();

    /**
     * Replaces the current group-state entries for this service.
     * <p>
     * Passing an empty list removes all current group-state entries. The supplied list must be
     * non-{@code null}.
     *
     * @param groupStateList the group-state entry list to set
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code groupStateList} is {@code null}
     */
    DirectoryService groupStateList(List<DirectoryServiceGroup> groupStateList);

    /**
     * Returns the load filter for this service.
     *
     * @return the load filter
     * @throws OmmInvalidUsageException if {@link #checkHasLoad()} returns {@code false}
     */
    DirectoryServiceLoad load();

    /**
     * Sets the load filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param load the service load filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code load} is {@code null} or the copy operation fails
     */
    DirectoryService load(DirectoryServiceLoad load);

    /**
     * Returns the data filter for this service.
     *
     * @return the data filter
     * @throws OmmInvalidUsageException if {@link #checkHasData()} returns {@code false}
     */
    DirectoryServiceData data();

    /**
     * Sets the data filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param data the service data filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null} or the copy operation fails
     */
    DirectoryService data(DirectoryServiceData data);

    /**
     * Returns the link filter for this service.
     *
     * @return the link filter
     * @throws OmmInvalidUsageException if {@link #checkHasLink()} returns {@code false}
     */
    DirectoryServiceLinkInfo link();

    /**
     * Sets the link filter for this service.
     * <p>
     * The supplied filter is copied into this service.
     *
     * @param link the service link filter
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code link} is {@code null} or the copy operation fails
     */
    DirectoryService link(DirectoryServiceLinkInfo link);

    /**
     * Performs a deep copy of another {@link DirectoryService} into this object.
     * <p>
     * If {@code sourceService} is this object, the call is a no-op. Otherwise, this service is
     * {@link #clear() cleared} and then replaced with the source service's current state,
     * including {@link #action()}, {@link #serviceId()}, all filters present on the source, and
     * deep-copied group-state entries. Filters absent on the source are removed from this object.
     *
     * @param sourceService the service to copy from
     * @return this directory service instance
     * @throws OmmInvalidUsageException if {@code sourceService} is {@code null}
     */
    DirectoryService copy(DirectoryService sourceService);
}

