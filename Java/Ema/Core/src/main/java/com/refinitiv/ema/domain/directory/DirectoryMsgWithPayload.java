/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.OmmInvalidUsageException;

import java.util.List;

/**
 * Base contract for RDM Directory messages whose payload is represented as a
 * list of directory services.
 * <p>
 * Filter-capable payload messages also implement {@link DirectoryMsgWithFilter}.
 *
 * @param <T> the EMA message type used to populate this directory message from
 *            or to create an EMA message view of its current RDM state
 *
 * @see DirectoryMsg
 * @see DirectoryService
 */
public interface DirectoryMsgWithPayload<T> extends DirectoryMsg<T>
{
    /**
     * Returns the current service entries represented by this directory message payload.
     *
     * @return the current service entry list
     */
    List<DirectoryService> serviceList();

    /**
     * Replaces the current service entries represented by this directory message payload.
     *
     * @param serviceList the service entries to set
     * @return this directory message instance
     * @throws OmmInvalidUsageException if {@code serviceList} is {@code null}
     */
    DirectoryMsgWithPayload<T> serviceList(List<DirectoryService> serviceList);
}
