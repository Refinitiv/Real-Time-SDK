/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryRequest;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.BitSet;

import static com.refinitiv.ema.access.DirectoryValidators.isValidServiceId;

/**
 * Mutable implementation of {@link DirectoryRequest} backed by an EMA
 * {@link ReqMsg} view.
 * <p>
 * An OMM consumer can use this request to ask for directory information for all
 * services or for a single service. The request targets all services when
 * neither {@code serviceId} nor {@code serviceName} is set. When a specific
 * service is requested, exactly one of those selectors may be present.
 * <p>
 * The inherited filter identifies which service filter sections are requested,
 * while {@link #initialImage()} and {@link #interestAfterRefresh()} control the
 * behavior encoded into the generated {@link ReqMsg}.
 *
 * @see DirectoryMsgWithFilterImpl
 * @see DirectoryRequest
 */
final class DirectoryRequestImpl extends DirectoryMsgWithFilterImpl<ReqMsg> implements DirectoryRequest
{
    private final ReqMsg requestMsg = EmaFactory.createReqMsg();
    private final BitSet flags = new BitSet();
    private static final int HAS_SERVICE_ID_FLAG = 0;
    private static final int HAS_SERVICE_NAME_FLAG = 1;
    private static final int INITIAL_IMAGE_FLAG = 2;
    private static final int INTEREST_AFTER_REFRESH_FLAG = 3;
    private String serviceName;
    private int serviceId;

    DirectoryRequestImpl()
    {
        clear();
    }

    /**
     * Clears the current request state and resets this instance for reuse.
     * <p>
     * After this call, the request has {@code streamId = -1}, the default
     * combined directory service filter mask ({@link EmaRdm#SERVICE_INFO_FILTER},
     * {@link EmaRdm#SERVICE_STATE_FILTER}, {@link EmaRdm#SERVICE_GROUP_FILTER},
     * {@link EmaRdm#SERVICE_LOAD_FILTER}, {@link EmaRdm#SERVICE_DATA_FILTER},
     * and {@link EmaRdm#SERVICE_LINK_FILTER}), no service selector
     * ({@link #checkHasServiceId()} and {@link #checkHasServiceName()} both
     * return {@code false}), {@code initialImage = true}, and
     * {@code interestAfterRefresh = true}. The cached {@link ReqMsg} view is
     * also cleared.
     *
     * @return this directory request instance
     */
    @Override
    public DirectoryRequest clear()
    {
        super.clear();
        flags.clear();
        requestMsg.clear();
        initialImage(true);
        interestAfterRefresh(true);
        filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER | EmaRdm.SERVICE_GROUP_FILTER |
                EmaRdm.SERVICE_LOAD_FILTER | EmaRdm.SERVICE_DATA_FILTER | EmaRdm.SERVICE_LINK_FILTER);
        serviceName = "";
        serviceId = 0;
        return this;
    }

    /**
     * Replaces the contents of this request with values decoded from the
     * supplied EMA request message.
     * <p>
     * The supplied message must be non-{@code null} and must use the Directory
     * domain. Any existing state in this instance is discarded before the new
     * values are applied.
     *
     * @param msg the EMA request message to decode into this directory request
     * @return this directory request instance
     * @throws OmmInvalidUsageException if {@code msg} is {@code null} or does
     *                                  not have the Directory domain type
     */
    @Override
    public DirectoryRequest message(ReqMsg msg)
    {
        if (msg == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("msg can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (msg.domainType() != EmaRdm.MMT_DIRECTORY)
        {
            throw new OmmInvalidUsageExceptionImpl().message("Domain type must be Directory.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        streamId(msg.streamId());
        if (msg.hasFilter())
        {
            filter(msg.filter());
        }
        if (msg.hasServiceId())
        {
            serviceId(msg.serviceId());
        }
        if (msg.hasServiceName())
        {
            serviceName(msg.serviceName());
        }
        initialImage(msg.initialImage());
        interestAfterRefresh(msg.interestAfterRefresh());

        return this;
    }

    @Override
    public DirectoryRequest streamId(int streamId)
    {
        super.streamId(streamId);
        return this;
    }

    @Override
    public DirectoryRequest filter(long filter)
    {
        super.filter(filter);
        return this;
    }

    /**
     * Creates an EMA {@link ReqMsg} view of the current directory request state.
     * <p>
     * The returned message is cleared and repopulated on each call, then updated
     * with the current domain type, stream ID, filter, optional service
     * selector, and request flags.
     *
     * @return a {@link ReqMsg} containing the current directory request state
     */
    @Override
    public ReqMsg message()
    {
        requestMsg.clear();
        requestMsg.domainType(domainType());
        requestMsg.streamId(streamId());
        requestMsg.filter(filter);
        if(checkHasServiceId())
        {
            requestMsg.serviceId(serviceId);
        }
        if(checkHasServiceName())
        {
            requestMsg.serviceName(serviceName);
        }
        requestMsg.initialImage(initialImage());
        requestMsg.interestAfterRefresh(interestAfterRefresh());

        return requestMsg;
    }

    /**
     * Replaces the contents of this request with a deep copy of the supplied
     * directory request.
     *
     * @param sourceStatusMsg the source directory request to copy from; cannot
     *                        be {@code null}
     * @return this directory request instance
     * @throws OmmInvalidUsageException if {@code sourceStatusMsg} is {@code null}
     */
    @Override
    public DirectoryRequest copy(DirectoryRequest sourceStatusMsg)
    {
        if (sourceStatusMsg == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceStatusMsg can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceStatusMsg == this)
        {
            return this;
        }

        clear();

        streamId(sourceStatusMsg.streamId());
        filter(sourceStatusMsg.filter());
        if (sourceStatusMsg.checkHasServiceId())
        {
            serviceId(sourceStatusMsg.serviceId());
        }
        if (sourceStatusMsg.checkHasServiceName())
        {
            serviceName(sourceStatusMsg.serviceName());
        }
        initialImage(sourceStatusMsg.initialImage());
        interestAfterRefresh(sourceStatusMsg.interestAfterRefresh());
        return this;
    }

    /**
     * Returns the service identifier targeted by this request.
     *
     * @return the requested service identifier
     * @throws OmmInvalidUsageException if {@link #checkHasServiceId()} returns
     *                                  {@code false}
     */
    @Override
    public int serviceId()
    {
        if (!checkHasServiceId())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SERVICE_ID + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return serviceId;
    }

    /**
     * Sets the service identifier targeted by this request.
     * <p>
     * A request may identify a service by either {@code serviceId} or
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
    @Override
    public DirectoryRequest serviceId(int serviceId)
    {
        if (checkHasServiceName())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SERVICE_NAME +
                    " element is already set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        if (!isValidServiceId(serviceId))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid serviceId value of " + serviceId,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.serviceId = serviceId;
        applyHasServiceId();
        return this;
    }

    /**
     * Returns the service name targeted by this request.
     *
     * @return the requested service name
     * @throws OmmInvalidUsageException if {@link #checkHasServiceName()} returns
     *                                  {@code false}
     */
    @Override
    public String serviceName()
    {
        if (!checkHasServiceName())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SERVICE_NAME +
                            " element is not set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return serviceName;
    }

    /**
     * Sets the service name targeted by this request.
     * <p>
     * A request may identify a service by either {@code serviceName} or
     * {@code serviceId}, but not both.
     *
     * @param serviceName the requested service name
     * @return this directory request instance
     * @throws OmmInvalidUsageException if {@code serviceName} is {@code null} or
     *                                  if {@link #checkHasServiceId()} returns
     *                                  {@code true}
     */
    @Override
    public DirectoryRequest serviceName(String serviceName)
    {
        if (checkHasServiceId())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SERVICE_ID +
                            " element is already set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        if (serviceName == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("serviceName can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.serviceName = serviceName;
        applyHasServiceName();
        return this;
    }

    /**
     * Indicates whether this request currently targets a service by identifier.
     *
     * @return {@code true} if {@link #serviceId()} is available; otherwise
     *         {@code false}
     */
    @Override
    public boolean checkHasServiceId()
    {
        return flags.get(HAS_SERVICE_ID_FLAG);
    }

    /** Marks this request as having a service identifier selector. */
    private void applyHasServiceId()
    {
        flags.set(HAS_SERVICE_ID_FLAG);
    }

    /**
     * Indicates whether this request currently targets a service by name.
     *
     * @return {@code true} if {@link #serviceName()} is available; otherwise
     *         {@code false}
     */
    @Override
    public boolean checkHasServiceName()
    {
        return flags.get(HAS_SERVICE_NAME_FLAG);
    }

    /** Marks this request as having a service name selector. */
    private void applyHasServiceName()
    {
        flags.set(HAS_SERVICE_NAME_FLAG);
    }

    /**
     * Indicates whether this request asks for an initial directory image.
     *
     * @return {@code true} if an initial image is requested; otherwise
     *         {@code false}
     */
    @Override
    public boolean initialImage()
    {
        return flags.get(INITIAL_IMAGE_FLAG);
    }

    /**
     * Sets whether this request asks for an initial directory image.
     *
     * @param value {@code true} to request an initial image; {@code false} to
     *              clear that request flag
     * @return this directory request instance
     */
    @Override
    public DirectoryRequest initialImage(boolean value)
    {
        if (value)
        {
            flags.set(INITIAL_IMAGE_FLAG);
        }
        else
        {
            flags.clear(INITIAL_IMAGE_FLAG);
        }
        return this;
    }

    /**
     * Indicates whether this request expresses interest after refresh.
     * <p>
     * After {@link #clear()}, this flag defaults to {@code true}.
     *
     * @return {@code true} if the request should remain open after the initial
     *         refresh; otherwise {@code false}
     */
    @Override
    public boolean interestAfterRefresh()
    {
        return flags.get(INTEREST_AFTER_REFRESH_FLAG);
    }

    /**
     * Sets whether this request expresses interest after refresh.
     * <p>
     * After {@link #clear()}, this flag defaults to {@code true}. Passing
     * {@code false} changes the request to a non-streaming interaction.
     *
     * @param value {@code true} to keep interest after the refresh;
     *              {@code false} to request a non-streaming interaction
     * @return this directory request instance
     */
    @Override
    public DirectoryRequest interestAfterRefresh(boolean value)
    {
        if (value)
        {
            flags.set(INTEREST_AFTER_REFRESH_FLAG);
        }
        else
        {
            flags.clear(INTEREST_AFTER_REFRESH_FLAG);
        }
        return this;
    }

    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "DirectoryRequest: " + EOL);

        if (checkHasServiceId())
        {
            stringBuilder.append(TAB)
                    .append("serviceId: ")
                    .append(serviceId())
                    .append(EOL);
        }

        if (checkHasServiceName())
        {
            stringBuilder.append(TAB)
                    .append("serviceName: ")
                    .append(serviceName())
                    .append(EOL);
        }

        stringBuilder.append(TAB)
                .append("initialImage: ")
                .append(initialImage())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("interestAfterRefresh: ")
                .append(interestAfterRefresh())
                .append(EOL);

        stringBuilder.append(filterAsString());

        return stringBuilder.toString();
    }
}
