/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.StatusMsg;

import java.nio.ByteBuffer;

/**
 * Represents an RDM Directory Status message.
 * <p>
 * OMM provider applications use this message to communicate state,
 * permission-data, and cache-related changes for a directory stream.
 * <p>
 * The inherited {@linkplain #filter() filter} identifies which directory
 * service filter sections the status applies to when a filter is present.
 * Optional members such as the {@linkplain #serviceId() service identifier},
 * {@linkplain #state() state}, and {@linkplain #permissionData() permission
 * data} must be checked for presence before they are accessed.
 * <p>
 * When {@link #checkHasServiceId()} returns {@code true}, the status carries a
 * service identifier that scopes the message to a specific service. When no
 * service identifier is present, the status applies to the directory stream as
 * a whole.
 * <p>
 * Instances are typically created with
 * {@link com.refinitiv.ema.access.EmaFactory.Domain#createDirectoryStatus()}.
 *
 * @see DirectoryMsgWithFilter
 * @see StatusMsg
 */
public interface DirectoryStatus extends DirectoryMsgWithFilter<StatusMsg>
{
    @Override
    DirectoryStatus clear();

    @Override
    DirectoryStatus message(StatusMsg msg);

    @Override
    DirectoryStatus streamId(int streamId);

    @Override
    DirectoryStatus filter(long filter);

    /**
     * Replaces the contents of this message with a deep copy of the supplied
     * directory status message.
     *
     * @param sourceStatusMsg the source directory status message to copy from;
     *                        cannot be {@code null}
     * @return this directory status message instance
     * @throws OmmInvalidUsageException if {@code sourceStatusMsg} is {@code null}
     */
    DirectoryStatus copy(DirectoryStatus sourceStatusMsg);

    /**
     * Indicates whether this status message currently carries an explicit
     * filter.
     *
     * @return {@code true} if {@link #filter()} is available; otherwise
     *         {@code false}
     */
    boolean checkHasFilter();

    /**
     * Indicates whether this status message currently carries a service
     * identifier.
     *
     * @return {@code true} if {@link #serviceId()} is available; otherwise
     *         {@code false}
     */
    boolean checkHasServiceId();

    /**
     * Indicates whether this status message currently carries a state.
     *
     * @return {@code true} if a state is present; otherwise {@code false}
     */
    boolean checkHasState();

    /**
     * Indicates whether this status message currently carries permission data.
     *
     * @return {@code true} if permission data is present; otherwise {@code false}
     */
    boolean checkHasPermissionData();

    /**
     * Indicates whether the clear-cache flag is set on this status message.
     * <p>
     * When this method returns {@code true}, receivers should discard any
     * cached directory information associated with the stream before applying
     * this status.
     *
     * @return {@code true} if cached directory data for the stream should be
     *         cleared before this status is applied; otherwise {@code false}
     */
    boolean clearCache();

    /**
     * Sets or clears the clear-cache flag on this status message.
     * <p>
     * When encoded with {@code true}, this instructs receivers to discard any
     * cached directory information associated with the stream before applying
     * the status.
     *
     * @param value {@code true} to request cache clearing before the status is
     *              applied; {@code false} to leave cached data intact
     * @return this directory status message instance
     */
    DirectoryStatus clearCache(boolean value);

    /**
     * Returns the optional service identifier carried by this status message.
     *
     * @return the service identifier
     * @throws OmmInvalidUsageException if {@link #checkHasServiceId()} returns
     *                                  {@code false}
     */
    int serviceId();

    /**
     * Sets the service identifier carried by this status message.
     * <p>
     * Calling this method marks the optional service-identifier field as
     * present. The {@code serviceId} value must be between {@code 0} and
     * {@code 65535}, inclusive.
     *
     * @param serviceId the service identifier
     * @return this directory status message instance
     * @throws OmmInvalidUsageException if {@code serviceId} is
     *                                  outside the valid range
     */
    DirectoryStatus serviceId(int serviceId);

    /**
     * Returns the current state carried by this directory status message.
     *
     * @return the current {@link OmmState}
     * @throws OmmInvalidUsageException if {@link #checkHasState()} returns {@code false}
     */
    OmmState state();

    /**
     * Sets the state for this directory status message.
     *
     * @param state the state to copy from
     * @return this directory status message instance
     * @throws OmmInvalidUsageException if {@code state} is {@code null}, if it
     *                                  contains a {@code null} status text, or
     *                                  if any state component is not supported
     */
    DirectoryStatus state(OmmState state);

    /**
     * Sets the state for this directory status message using individual
     * {@link OmmState} components.
     *
     * @param streamState the {@link OmmState.StreamState} value
     * @param dataState the {@link OmmState.DataState} value
     * @param statusCode the {@link OmmState.StatusCode} value
     * @param statusText the human-readable status text
     * @return this directory status message instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or
     *                                  any state component is not supported
     */
    DirectoryStatus state(int streamState, int dataState, int statusCode, String statusText);

    /**
     * Returns the optional permission data associated with all content on this
     * stream.
     *
     * @return the permission data buffer associated with this stream
     * @throws OmmInvalidUsageException if {@link #checkHasPermissionData()} returns {@code false}
     */
    ByteBuffer permissionData();

    /**
     * Sets the permission data for this directory status message.
     * <p>
     * Calling this method marks the optional permission-data field as present.
     *
     * @param permissionData the permission data buffer to copy from
     * @return this directory status message instance
     * @throws OmmInvalidUsageException if {@code permissionData} is {@code null}
     */
    DirectoryStatus permissionData(ByteBuffer permissionData);
}
