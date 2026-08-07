/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.UpdateMsg;

/**
 * Represents an RDM Directory Update message.
 * <p>
 * OMM provider applications use this message to publish incremental changes for
 * one or more directory services. The inherited {@link #serviceList()} payload
 * contains the service entries carried by the update, and each
 * {@link DirectoryService} identifies whether it represents an add, update, or
 * delete action.
 * <p>
 * The inherited {@linkplain #filter() filter} identifies which directory
 * service filter sections are represented by the update when a filter is
 * present. Optional members such as the {@linkplain #filter() filter} and
 * {@linkplain #sequenceNumber() sequence number} must be checked for presence
 * before they are accessed.
 * <p>
 * Instances are typically created with
 * {@link com.refinitiv.ema.access.EmaFactory.Domain#createDirectoryUpdate()}.
 *
 * @see DirectoryMsgWithPayload
 * @see DirectoryMsgWithFilter
 * @see DirectoryService
 * @see UpdateMsg
 */
public interface DirectoryUpdate extends DirectoryMsgWithPayload<UpdateMsg>, DirectoryMsgWithFilter<UpdateMsg>
{
    @Override
    DirectoryUpdate clear();

    @Override
    DirectoryUpdate message(UpdateMsg msg);

    @Override
    DirectoryUpdate streamId(int streamId);

    @Override
    DirectoryUpdate filter(long filter);

    @Override
    DirectoryUpdate serviceList(java.util.List<DirectoryService> serviceList);

    /**
     * Indicates whether this update currently carries a sequence number.
     *
     * @return {@code true} if {@link #sequenceNumber()} is available; otherwise {@code false}
     */
    boolean checkHasSequenceNumber();

    /**
     * Indicates whether this update currently carries an explicit filter.
     *
     * @return {@code true} if {@link #filter()} is available; otherwise {@code false}
     */
    boolean checkHasFilter();

    /**
     * Indicates whether the do-not-cache flag is set on this update message.
     * <p>
     * When this method returns {@code true}, receivers should not cache this
     * update.
     *
     * @return {@code true} if this update should not be cached; otherwise
     *         {@code false}
     */
    boolean doNotCache();

    /**
     * Sets or clears the do-not-cache flag on this update message.
     * <p>
     * When encoded with {@code true}, this instructs receivers that the update
     * should not be cached.
     *
     * @param value {@code true} to mark this update as non-cacheable;
     *              {@code false} to allow normal caching
     * @return this directory update message instance
     */
    DirectoryUpdate doNotCache(boolean value);

    /**
     * Indicates whether the do-not-conflate flag is set on this update message.
     * <p>
     * When this method returns {@code true}, receivers should not conflate this
     * update with other updates.
     *
     * @return {@code true} if this update should not be conflated; otherwise
     *         {@code false}
     */
    boolean doNotConflate();

    /**
     * Sets or clears the do-not-conflate flag on this update message.
     * <p>
     * When encoded with {@code true}, this instructs receivers not to conflate
     * the update with other updates.
     *
     * @param value {@code true} to mark this update as non-conflatable;
     *              {@code false} to allow normal conflation
     * @return this directory update message instance
     */
    DirectoryUpdate doNotConflate(boolean value);

    /**
     * Returns the optional sequence number carried by this message.
     *
     * @return the sequence number
     * @throws OmmInvalidUsageException if {@link #checkHasSequenceNumber()} returns {@code false}
     */
    long sequenceNumber();

    /**
     * Sets the sequence number for this message.
     * <p>
     * Calling this method marks the optional sequence-number field as present.
     *
     * @param sequenceNumber the sequence number
     * @return this directory update message instance
     */
    DirectoryUpdate sequenceNumber(long sequenceNumber);

    /**
     * Replaces the contents of this message with a deep copy of the supplied
     * directory update message.
     *
     * @param sourceUpdateMsg the source directory update message to copy from;
     *                        cannot be {@code null}
     * @return this directory update message instance
     * @throws OmmInvalidUsageException if {@code sourceUpdateMsg} is {@code null}
     */
    DirectoryUpdate copy(DirectoryUpdate sourceUpdateMsg);
}
