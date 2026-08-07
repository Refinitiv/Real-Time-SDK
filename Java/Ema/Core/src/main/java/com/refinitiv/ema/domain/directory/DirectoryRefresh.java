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
import com.refinitiv.ema.access.RefreshMsg;

/**
 * Represents an RDM Directory Refresh message.
 * <p>
 * OMM provider applications use this message to publish the current directory
 * image for one or more services. The inherited {@link #serviceList()} payload
 * contains the service entries carried by the refresh.
 * <p>
 * The inherited {@linkplain #filter() filter} identifies which directory
 * service filter sections may be present in the refresh payload. Optional
 * members such as the {@linkplain #serviceId() service identifier} and
 * {@linkplain #sequenceNumber() sequence number} must be checked for presence
 * before they are accessed.
 * <p>
 * When {@link #checkHasServiceId()} returns {@code true}, the refresh carries a
 * service identifier that scopes the refresh to a specific service. When no
 * service identifier is present, the refresh may describe multiple services.
 * <p>
 * Instances are typically created with
 * {@link com.refinitiv.ema.access.EmaFactory.Domain#createDirectoryRefresh()}.
 *
 * @see DirectoryMsgWithPayload
 * @see DirectoryMsgWithFilter
 * @see DirectoryService
 * @see RefreshMsg
 */
public interface DirectoryRefresh extends DirectoryMsgWithPayload<RefreshMsg>, DirectoryMsgWithFilter<RefreshMsg>
{
    @Override
    DirectoryRefresh clear();

    @Override
    DirectoryRefresh message(RefreshMsg msg);

    @Override
    DirectoryRefresh streamId(int streamId);

    @Override
    DirectoryRefresh filter(long filter);

    @Override
    DirectoryRefresh serviceList(java.util.List<DirectoryService> serviceList);

    /**
     * Replaces the contents of this message with a deep copy of the supplied
     * directory refresh message.
     *
     * @param sourceRefreshMsg the source directory refresh message to copy from;
     *                         cannot be {@code null}
     * @return this directory refresh message instance
     * @throws OmmInvalidUsageException if {@code sourceRefreshMsg} is {@code null}
     */
    DirectoryRefresh copy(DirectoryRefresh sourceRefreshMsg);

    /**
     * Indicates whether this refresh currently carries a service identifier.
     *
     * @return {@code true} if {@link #serviceId()} is available; otherwise
     *         {@code false}
     */
    boolean checkHasServiceId();

    /**
     * Indicates whether this refresh currently carries a sequence number.
     *
     * @return {@code true} if {@link #sequenceNumber()} is available; otherwise {@code false}
     */
    boolean checkHasSequenceNumber();

    /**
     * Indicates whether the clear-cache flag is set on this refresh message.
     * <p>
     * When this method returns {@code true}, receivers should clear any cached
     * directory information associated with the stream before applying this
     * refresh.
     *
     * @return {@code true} if cached directory data for the stream should be
     *         cleared before this refresh is applied; otherwise {@code false}
     */
    boolean clearCache();

    /**
     * Sets or clears the clear-cache flag on this refresh message.
     * <p>
     * When encoded with {@code true}, this instructs receivers to discard any
     * cached directory information associated with the stream before applying
     * the refresh.
     *
     * @param value {@code true} to request cache clearing before the refresh is
     *              applied; {@code false} to leave cached data intact
     * @return this directory refresh message instance
     */
    DirectoryRefresh clearCache(boolean value);

    /**
     * Indicates whether the do-not-cache flag is set on this refresh message.
     * <p>
     * When this method returns {@code true}, receivers should not cache this
     * refresh.
     *
     * @return {@code true} if this refresh should not be cached; otherwise
     *         {@code false}
     */
    boolean doNotCache();

    /**
     * Sets or clears the do-not-cache flag on this refresh message.
     * <p>
     * When encoded with {@code true}, this instructs receivers that the refresh
     * should not be cached.
     *
     * @param value {@code true} to mark this refresh as non-cacheable;
     *              {@code false} to allow normal caching
     * @return this directory refresh message instance
     */
    DirectoryRefresh doNotCache(boolean value);

    /**
     * Indicates whether the refresh-complete flag is set on this message.
     * <p>
     * A return value of {@code true} means this refresh is complete for the
     * stream, either because it is a single-part refresh or because it is the
     * final part of a multipart refresh.
     *
     * @return {@code true} if this message completes the refresh for the
     *         stream; otherwise {@code false}
     */
    boolean complete();

    /**
     * Sets or clears the refresh-complete flag on this refresh message.
     * <p>
     * When encoded with {@code true}, this marks the message as a complete
     * refresh for the stream or as the final part of a multipart refresh.
     *
     * @param value {@code true} to mark this refresh as complete;
     *              {@code false} to indicate that additional refresh parts may
     *              follow
     * @return this directory refresh message instance
     */
    DirectoryRefresh complete(boolean value);

    /**
     * Indicates whether the solicited flag is currently set on this refresh
     * message.
     * <p>
     * A solicited refresh is sent in response to a request.
     *
     * @return {@code true} if this refresh was sent in response to a request;
     *         otherwise {@code false}
     */
    boolean solicited();

    /**
     * Sets or clears the solicited flag on this refresh message.
     * <p>
     * When encoded, a solicited refresh indicates that it was sent in response
     * to a request.
     *
     * @param value {@code true} to mark the refresh as solicited;
     *              {@code false} to clear the solicited flag
     * @return this directory refresh message instance
     */
    DirectoryRefresh solicited(boolean value);

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
     * @return this directory refresh message instance
     */
    DirectoryRefresh sequenceNumber(long sequenceNumber);

    /**
     * Returns the optional service identifier carried by this message.
     *
     * @return the service identifier
     * @throws OmmInvalidUsageException if {@link #checkHasServiceId()} returns
     *                                  {@code false}
     */
    int serviceId();

    /**
     * Sets the service identifier carried by this message.
     * <p>
     * Calling this method marks the optional service-identifier field as
     * present. The {@code serviceId} value must be between {@code 0} and
     * {@code 65535}, inclusive.
     *
     * @param serviceId the service identifier
     * @return this directory refresh message instance
     * @throws OmmInvalidUsageException if {@code serviceId} is
     *                                  outside the valid range
     */
    DirectoryRefresh serviceId(int serviceId);

    /**
     * Returns the current state carried by this directory refresh message.
     *
     * @return the current {@link OmmState}
     */
    OmmState state();

    /**
     * Sets state for the directory refresh message.
     *
     * @param state the state to copy from
     * @return this directory refresh message instance
     * @throws OmmInvalidUsageException if {@code state} is {@code null}, if it
     *                                  contains a {@code null} status text, or
     *                                  if any state component is not supported
     */
    DirectoryRefresh state(OmmState state);

    /**
     * Sets state for the directory refresh message.
     *
     * @param streamState the {@link OmmState.StreamState} value
     * @param dataState the {@link OmmState.DataState} value
     * @param statusCode the {@link OmmState.StatusCode} value
     * @param statusText the human-readable status text
     * @return this directory refresh message instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or
     *                                  if any state component is not supported
     */
    DirectoryRefresh state(int streamState, int dataState, int statusCode, String statusText);
}
