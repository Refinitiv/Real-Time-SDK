/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.ReqMsg;

/**
 * Represents an RDM Directory Request message.
 * <p>
 * OMM consumer applications use this message to request directory information
 * for either all services or a single service. A request targets all services
 * when neither {@linkplain #serviceId() service identifier} nor
 * {@linkplain #serviceName() service name} is present. A request targets a
 * single service when exactly one of those selectors is set.
 * <p>
 * The inherited {@linkplain #filter() filter} identifies which directory
 * service filter sections are being requested. Optional selectors such as the
 * {@linkplain #serviceId() service identifier} and
 * {@linkplain #serviceName() service name} must be checked for presence before
 * they are accessed. The {@link #initialImage()} and
 * {@link #interestAfterRefresh()} flags control the request behavior encoded in
 * the underlying {@link ReqMsg}.
 * <p>
 * Instances are typically created with
 * {@link com.refinitiv.ema.access.EmaFactory.Domain#createDirectoryRequest()}.
 *
 * @see DirectoryMsgWithFilter
 * @see ReqMsg
 */
public interface DirectoryRequest extends DirectoryMsgWithFilter<ReqMsg>
{
    /**
     * Clears the current request state and resets this instance for reuse.
     * <p>
     * After this call, the request has {@code streamId = -1}, the default
     * combined directory service filter mask, no service selector, and both
     * {@link #initialImage()} and {@link #interestAfterRefresh()} set to
     * {@code true}.
     *
     * @return this directory request instance
     */
    @Override
    DirectoryRequest clear();

    @Override
    DirectoryRequest message(ReqMsg msg);

    @Override
    DirectoryRequest streamId(int streamId);

    @Override
    DirectoryRequest filter(long filter);

    /**
     * Replaces the contents of this request with a deep copy of the supplied
     * directory request.
     *
     * @param sourceRequestMsg the source directory request to copy from; cannot
     *                         be {@code null}
     * @return this directory request instance
     * @throws OmmInvalidUsageException if {@code sourceRequestMsg} is {@code null}
     */
    DirectoryRequest copy(DirectoryRequest sourceRequestMsg);

    /**
     * Returns the optional service identifier targeted by this request.
     *
     * @return the requested service identifier
     * @throws OmmInvalidUsageException if {@link #checkHasServiceId()} returns
     *                                  {@code false}
     */
    int serviceId();

    /**
     * Sets the service identifier targeted by this request.
     * <p>
     * Calling this method marks the optional service-identifier selector as
     * present. A request may identify a service by either {@code serviceId} or
     * {@code serviceName}, but not both.
     * <p>
     * The {@code serviceId} value must be between {@code 0} and {@code 65535},
     * inclusive.
     *
     * @param serviceId the requested service identifier
     * @return this directory request instance
     * @throws OmmInvalidUsageException if {@link #checkHasServiceName()} returns
     *                                  {@code true}, or if {@code serviceId} is
     *                                  outside the valid range
     */
    DirectoryRequest serviceId(int serviceId);

    /**
     * Returns the optional service name targeted by this request.
     *
     * @return the requested service name
     * @throws OmmInvalidUsageException if {@link #checkHasServiceName()} returns
     *                                  {@code false}
     */
    String serviceName();

    /**
     * Sets the service name targeted by this request.
     * <p>
     * Calling this method marks the optional service-name selector as present.
     * A request may identify a service by either {@code serviceName} or
     * {@code serviceId}, but not both.
     *
     * @param serviceName the requested service name
     * @return this directory request instance
     * @throws OmmInvalidUsageException if {@code serviceName} is {@code null} or
     *                                  if {@link #checkHasServiceId()} returns
     *                                  {@code true}
     */
    DirectoryRequest serviceName(String serviceName);

    /**
     * Indicates whether this request currently targets a service by identifier.
     *
     * @return {@code true} if {@link #serviceId()} is available; otherwise
     *         {@code false}
     */
    boolean checkHasServiceId();

    /**
     * Indicates whether this request currently targets a service by name.
     *
     * @return {@code true} if {@link #serviceName()} is available; otherwise
     *         {@code false}
     */
    boolean checkHasServiceName();

    /**
     * Indicates whether the request asks for an initial directory image.
     *
     * @return {@code true} if an initial image is requested; otherwise
     *         {@code false}
     */
    boolean initialImage();

    /**
     * Sets whether the request asks for an initial directory image.
     *
     * @param value {@code true} to request an initial image; {@code false} to
     *              clear that request flag
     * @return this directory request instance
     */
    DirectoryRequest initialImage(boolean value);

    /**
     * Indicates whether the request expresses interest after refresh.
     * <p>
     * After {@link #clear()}, this flag defaults to {@code true}.
     *
     * @return {@code true} if the request should remain open after the initial
     *         refresh; otherwise {@code false}
     */
    boolean interestAfterRefresh();

    /**
     * Sets whether the request expresses interest after refresh.
     * <p>
     * After {@link #clear()}, this flag defaults to {@code true}. Passing
     * {@code false} changes the request to a non-streaming interaction.
     *
     * @param value {@code true} to keep interest after the refresh; {@code false}
     *              to request a non-streaming interaction
     * @return this directory request instance
     */
    DirectoryRequest interestAfterRefresh(boolean value);
}
