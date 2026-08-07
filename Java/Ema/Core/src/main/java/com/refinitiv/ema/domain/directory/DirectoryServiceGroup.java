/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.OmmBuffer;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmState;

import java.nio.ByteBuffer;

/**
 * Represents one RDM Source Directory service group-state entry.
 * <p>
 * A service group-state entry conveys status information for all items whose
 * {@code ItemGroup} matches {@link #group()}. The {@code Group} element is the
 * primary identifier of this filter entry, while {@code MergedToGroup} and
 * {@code Status} are optional fields whose presence can be checked through
 * {@link #checkHasMergedToGroup()} and {@link #checkHasStatus()}.
 * <p>
 * After {@link #clear()}, optional fields are removed, {@link #action()} is reset
 * to {@link com.refinitiv.ema.access.FilterEntry.FilterAction#SET}, and
 * {@link #group()} returns an empty buffer until explicitly set.
 *
 * @see DirectoryServiceFilter
 */
public interface DirectoryServiceGroup extends DirectoryServiceFilter<ElementList>
{
    /**
     * Clears this service group-state filter and resets it to its default state.
     * <p>
     * This removes optional fields, clears stored group data, resets the cached
     * status information, and restores the default filter-entry action.
     *
     * @return this directory service group filter instance
     */
    @Override
    DirectoryServiceGroup clear();

    /**
     * Sets the filter-entry action associated with this service group-state filter.
     * <p>
     * Valid values are defined by the concrete implementation and typically come
     * from {@link com.refinitiv.ema.access.FilterEntry.FilterAction}.
     *
     * @param action the filter-entry action
     * @return this directory service group filter instance
     */
    @Override
    DirectoryServiceGroup action(int action);

    /**
     * Decodes an {@link ElementList} payload into this service group-state filter.
     * <p>
     * Implementations replace the current content with the decoded group-state
     * data.
     *
     * @param struct encoded {@link ElementList} representing the service group-state filter
     * @return this directory service group filter instance
     */
    @Override
    DirectoryServiceGroup decode(ElementList struct);

    /**
     * Checks the presence of the status field.
     *
     * @return {@code true} if the status field is present; {@code false} otherwise
     */
    boolean checkHasStatus();

    /**
     * Checks the presence of the mergedToGroup field.
     *
     * @return {@code true} if the mergedToGroup field is present; {@code false} otherwise
     */
    boolean checkHasMergedToGroup();

    /**
     * Returns the item-group identifier associated with this service group-state entry.
     * <p>
     * This method always returns an {@link OmmBuffer}. If no group has been set yet,
     * the returned buffer is empty.
     *
     * @return the group buffer
     */
    OmmBuffer group();

    /**
     * Sets the item-group identifier for this service group-state entry.
     * <p>
     * The supplied buffer content is copied into this object.
     *
     * @param group the group buffer
     * @return this directory service group filter instance
     *
     * @throws OmmInvalidUsageException if {@code group} is {@code null}
     */
    DirectoryServiceGroup group(ByteBuffer group);

    /**
     * Returns the merged-to group value.
     * <p>
     * This optional field indicates the target group associated with this entry when
     * {@link #checkHasMergedToGroup()} is {@code true}.
     *
     * @return the mergedToGroup buffer
     *
     * @throws OmmInvalidUsageException if {@link #checkHasMergedToGroup()} returns {@code false}
     */
    OmmBuffer mergedToGroup();

    /**
     * Sets the merged-to group value.
     * <p>
     * The supplied buffer content is copied into this object and marks the
     * {@code mergedToGroup} field as present.
     *
     * @param mergedToGroup the merged to group buffer
     * @return this directory service group filter instance
     *
     * @throws OmmInvalidUsageException if {@code mergedToGroup} is {@code null}
     */
    DirectoryServiceGroup mergedToGroup(ByteBuffer mergedToGroup);

    /**
     * Returns the status to be applied to all items whose {@code ItemGroup}
     * matches {@link #group()}.
     *
     * @return status
     *
     * @throws OmmInvalidUsageException if {@link #checkHasStatus()} returns {@code false}
     */
    OmmState status();

    /**
     * Sets the status to be applied to all items whose {@code ItemGroup}
     * matches {@link #group()}.
     * <p>
     * The supplied status is copied into this object and marks the {@code status}
     * field as present.
     *
     * @param status the status
     * @return this directory service group filter instance
     *
     * @throws OmmInvalidUsageException if {@code status} is {@code null}
     */
    DirectoryServiceGroup status(OmmState status);

    /**
     * Sets the status to be applied to all items whose {@code ItemGroup}
     * matches {@link #group()}.
     * <p>
     * Calling this method marks the {@code status} field as present.
     *
     * @param streamState represents {@link OmmState.StreamState}
     * @param dataState represents {@link OmmState.DataState}
     * @param statusCode represents {@link OmmState.StatusCode}
     * @param statusText string representing the state text
     * @return this directory service group filter instance
     *
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or if any
     *                                  state value is invalid
     */
    DirectoryServiceGroup status(int streamState, int dataState, int statusCode, String statusText);

    /**
     * Replaces this object with a deep copy of another {@link DirectoryServiceGroup}.
     * <p>
     * On success, this object is cleared and then populated with the source object's
     * action, group, and any optional fields currently present on the source.
     *
     * @param sourceServiceGroup the service group-state entry to copy from
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceGroup} is {@code null}
     */
    DirectoryServiceGroup copy(DirectoryServiceGroup sourceServiceGroup);
}
