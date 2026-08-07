/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.*;

/**
 * Base contract for RDM Directory messages.
 * <p>
 * Implementations may be cleared and re-used, including in pooled workflows.
 * Directory message types that support a filter also implement
 * {@link DirectoryMsgWithFilter}.
 *
 * @param <T> the EMA message type used to populate this directory message from
 *            or to create an EMA message view of its current RDM state
 *
 * @see DirectoryRefresh
 * @see DirectoryRequest
 * @see DirectoryConsumerStatus
 * @see DirectoryStatus
 * @see DirectoryUpdate
 *
 * @see EmaFactory - Factory for creating RDM directory messages
 *
 */
public interface DirectoryMsg<T>
{
    /**
     * Returns the domain type of the RDM message.
     *
     * @return the RDM domain type
     */
    int domainType();

    /**
     * Populates this directory message from the specified EMA message.
     * <p>
     * Implementations validate the supplied message and replace the current
     * contents of this directory message with values decoded from it.
     *
     * @param msg the EMA message used to populate this directory message instance
     * @return this directory message instance
     *
     * @throws OmmInvalidUsageException if the supplied message is invalid for this
     *                                  directory message type or the operation fails
     */
    DirectoryMsg<T> message(T msg);

    /**
     * Creates an EMA message representation of the current RDM state.
     *
     * @return an EMA message of type {@code T} representing the current RDM state
     */
    T message();

    /**
     * Returns the stream ID of this directory message.
     *
     * @return the stream ID
     */
    int streamId();

    /**
     * Sets the stream ID of this directory message.
     *
     * @param streamId the stream ID
     * @return this directory message instance
     */
    DirectoryMsg<T> streamId(int streamId);
    
    /**
     * Clears the current contents of this directory message and prepares it for reuse.
     *
     * @return this directory message instance
     */
    DirectoryMsg<T> clear();
}

