/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryStatus;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.State;

import java.nio.ByteBuffer;
import java.util.BitSet;

import static com.refinitiv.ema.access.DirectoryValidators.isValidServiceId;
import static com.refinitiv.ema.access.DirectoryValidators.validateStatus;

/**
 * Internal implementation of the EMA RDM directory status message.
 * <p>
 * Used by an OMM provider to communicate state, permission data, and
 * clear-cache changes for a directory stream through the directory-domain view
 * of a {@link StatusMsg}.
 *
 * @see DirectoryMsgWithFilterImpl
 * @see DirectoryStatus
 */
final class DirectoryStatusImpl extends DirectoryMsgWithFilterImpl<StatusMsg> implements DirectoryStatus
{
    private final State rsslState = CodecFactory.createState();
    private final Buffer stateText = CodecFactory.createBuffer();
    private final OmmStateImpl state = new OmmStateImpl();
    private final StatusMsg statusMsg = EmaFactory.createStatusMsg();
    private final Buffer permissionData = CodecFactory.createBuffer();
    private final BitSet flags = new BitSet();
    private static final int HAS_STATE_FLAG = 0;
    private static final int HAS_PERMISSION_DATA_FLAG = 1;
    private static final int CLEAR_CACHE_FLAG = 2;
    private static final int HAS_SERVICE_ID_FLAG = 3;
    private static final int HAS_FILTER_FLAG = 4;
    private int serviceId;

    DirectoryStatusImpl()
    {
        clear();
    }

    /**
     * Clears the current contents of this directory status message and prepares
     * the instance for reuse.
     *
     * @return this directory status instance
     */
    @Override
    public DirectoryStatus clear()
    {
        super.clear();
        flags.clear();

        rsslState.clear();
        stateText.data("");
        rsslState.text(stateText);

        permissionData.clear();
        statusMsg.clear();
        serviceId = 0;
        return this;
    }

    /**
     * Populates this directory status from the supplied {@link StatusMsg}.
     * <p>
     * The message must not be {@code null} and must use the
     * {@link EmaRdm#MMT_DIRECTORY} domain type.
     *
     * @param msg the status message used to populate this directory status
     * @return this directory status instance
     * @throws OmmInvalidUsageException if {@code msg} is {@code null} or has an
     *                                  unexpected domain type
     */
    @Override
    public DirectoryStatus message(StatusMsg msg)
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
        if (msg.hasState())
        {
            state(msg.state());
        }
        if (msg.hasPermissionData())
        {
            permissionData(msg.permissionData());
        }
        clearCache(msg.clearCache());
        return this;
    }

    @Override
    public DirectoryStatus streamId(int streamId)
    {
        super.streamId(streamId);
        return this;
    }

    @Override
    public StatusMsg message()
    {
        statusMsg.clear();
        statusMsg.domainType(domainType());
        statusMsg.streamId(streamId());
        if (checkHasFilter())
        {
            statusMsg.filter(filter);
        }
        if (checkHasServiceId())
        {
            statusMsg.serviceId(serviceId);
        }
        if (checkHasState())
        {
            statusMsg.state(rsslState.streamState(), rsslState.dataState(), rsslState.code(), rsslState.text().toString());
        }
        if(checkHasPermissionData())
        {
            statusMsg.permissionData(permissionData.data());
        }
        statusMsg.clearCache(clearCache());

        return statusMsg;
    }

    /**
     * Replaces the contents of this message with a deep copy of the supplied
     * directory status message.
     *
     * @param sourceStatusMsg the source directory status message to copy from;
     *                        cannot be {@code null}
     * @return this directory status instance
     * @throws OmmInvalidUsageException if {@code sourceStatusMsg} is {@code null}
     */
    @Override
    public DirectoryStatus copy(DirectoryStatus sourceStatusMsg)
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
        clearCache(sourceStatusMsg.clearCache());
        if (sourceStatusMsg.checkHasFilter())
        {
            filter(sourceStatusMsg.filter());
        }
        if (sourceStatusMsg.checkHasServiceId())
        {
            serviceId(sourceStatusMsg.serviceId());
        }
        if (sourceStatusMsg.checkHasPermissionData())
        {
            permissionData(sourceStatusMsg.permissionData());
        }
        if (sourceStatusMsg.checkHasState())
        {
            state(sourceStatusMsg.state());
        }
        return this;
    }

    /**
     * Indicates whether this status message currently carries an explicit
     * filter.
     *
     * @return {@code true} if {@link #filter()} is available; otherwise
     *         {@code false}
     */
    @Override
    public boolean checkHasFilter()
    {
        return flags.get(HAS_FILTER_FLAG);
    }

    /** Marks status message as having an explicit filter. */
    private void applyHasFilter()
    {
        flags.set(HAS_FILTER_FLAG);
    }

    /**
     * Indicates whether this status message currently carries a service
     * identifier.
     *
     * @return {@code true} if {@link #serviceId()} is available; otherwise
     *         {@code false}
     */
    @Override
    public boolean checkHasServiceId()
    {
        return flags.get(HAS_SERVICE_ID_FLAG);
    }

    /** Marks status message as having a service identifier selector. */
    private void applyHasServiceId()
    {
        flags.set(HAS_SERVICE_ID_FLAG);
    }

    /**
     * Indicates whether this status message currently carries a state.
     *
     * @return {@code true} if a state is present; otherwise {@code false}
     */
    @Override
    public boolean checkHasState()
    {
        return flags.get(HAS_STATE_FLAG);
    }

    /**
     * Marks this message as carrying a state.
     */
    private void applyHasState()
    {
        flags.set(HAS_STATE_FLAG);
    }

    /**
     * Indicates whether this status message currently carries permission data.
     *
     * @return {@code true} if permission data is present; otherwise {@code false}
     */
    @Override
    public boolean checkHasPermissionData()
    {
        return flags.get(HAS_PERMISSION_DATA_FLAG);
    }

    /**
     * Marks this message as carrying permission data.
     */
    private void applyHasPermissionData()
    {
        flags.set(HAS_PERMISSION_DATA_FLAG);
    }

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
    @Override
    public boolean clearCache()
    {
        return flags.get(CLEAR_CACHE_FLAG);
    }

    /**
     * Sets or clears the clear-cache flag on this status message.
     * <p>
     * When encoded with {@code true}, this instructs receivers to discard any
     * cached directory information associated with the stream before applying
     * the status.
     *
     * @param value {@code true} to request cache clearing before the status is
     *              applied; {@code false} to leave cached data intact
     * @return this directory status instance
     */
    @Override
    public DirectoryStatus clearCache(boolean value)
    {
        if (value)
        {
            flags.set(CLEAR_CACHE_FLAG);
        }
        else
        {
            flags.clear(CLEAR_CACHE_FLAG);
        }
        return this;
    }

    /**
     * Returns the current filter value.
     *
     * @return the current filter value
     * @throws OmmInvalidUsageException if {@link #checkHasFilter()} returns
     *                                  {@code false}
     */
    @Override
    public long filter()
    {
        if (!checkHasFilter())
        {
            throw new OmmInvalidUsageExceptionImpl().message("Filter element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return filter;
    }

    /**
     * Sets the filter value for the current directory message.
     *
     * @param filter the filter value to store
     * @return this directory status instance
     */
    @Override
    public DirectoryStatus filter(long filter)
    {
        this.filter = filter;
        applyHasFilter();
        return this;
    }

    /**
     * Returns the service identifier carried by this status message.
     *
     * @return the service identifier
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
     * Sets the service identifier carried by this status message.
     * <p>
     * The {@code serviceId} value must be between {@code 0} and {@code 65535},
     * inclusive.
     *
     * @param serviceId the service identifier
     * @return this directory status instance
     * @throws OmmInvalidUsageException if {@code serviceId} is
     *                                  outside the valid range
     */
    @Override
    public DirectoryStatus serviceId(int serviceId)
    {
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
     * Returns the current state carried by this directory status message.
     *
     * @return the stream state
     * @throws OmmInvalidUsageException if {@link #checkHasState()} returns {@code false}
     */
    @Override
    public OmmState state()
    {
        if (!checkHasState())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_STATE + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        state.decode(rsslState);

        return state;
    }

    /**
     * Sets the state for this directory status message.
     *
     * @param state the state to copy from
     * @return this directory status instance
     * @throws OmmInvalidUsageException if {@code state} is {@code null}, if it
     *                                  contains a {@code null} status text, or
     *                                  if any state component is not supported
     */
    @Override
    public DirectoryStatus state(OmmState state)
    {
        if (state == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("state can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        return state(state.streamState(), state.dataState(), state.statusCode(), state.statusText());
    }

    /**
     * Sets the state for this directory status message using individual
     * {@link OmmState} components.
     *
     * @param streamState the {@link OmmState.StreamState} value
     * @param dataState the {@link OmmState.DataState} value
     * @param statusCode the {@link OmmState.StatusCode} value
     * @param statusText the human-readable status text
     * @return this directory status instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or
     *                                  if any state component is not supported
     */
    @Override
    public DirectoryStatus state(int streamState, int dataState, int statusCode, String statusText)
    {
        if (statusText == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("statusText can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        validateStatus(streamState, dataState, statusCode);

        rsslState.streamState(streamState);
        rsslState.dataState(dataState);
        rsslState.code(statusCode);
        stateText.data(statusText);
        rsslState.text(stateText);

        applyHasState();
        return this;
    }

    /**
     * Returns the permission data associated with all content on this stream.
     *
     * @return a defensive copy of the permission data buffer
     * @throws OmmInvalidUsageException if {@link #checkHasPermissionData()} returns {@code false}
     */
    @Override
    public ByteBuffer permissionData()
    {
        if (!checkHasPermissionData())
        {
            throw new OmmInvalidUsageExceptionImpl().message("PermissionData element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return Utilities.copyFromPool(permissionData, null, null);
    }

    /**
     * Sets the permission data for this directory status message.
     *
     * @param permissionData the permission data buffer to copy from
     * @return this directory status instance
     * @throws OmmInvalidUsageException if {@code permissionData} is {@code null}
     */
    @Override
    public DirectoryStatus permissionData(ByteBuffer permissionData)
    {
        if (permissionData == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("permissionData can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        Utilities.copy(permissionData, this.permissionData);
        applyHasPermissionData();
        return this;
    }

    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "DirectoryStatus: " + EOL);

        if (checkHasServiceId())
        {
            stringBuilder.append(TAB)
                    .append("serviceId: ")
                    .append(serviceId())
                    .append(EOL);
        }

        if (checkHasFilter())
        {
            stringBuilder.append(filterAsString());
        }

        if (checkHasState())
        {
            stringBuilder.append(TAB)
                    .append("state: ")
                    .append(state())
                    .append(EOL);
        }

        stringBuilder.append(TAB)
                .append("clearCache: ")
                .append(clearCache())
                .append(EOL);

        if (checkHasPermissionData())
        {
            stringBuilder.append(TAB)
                    .append("permissionData: ");
            Utilities.asHexString(stringBuilder, permissionData.data());
            stringBuilder.append(EOL);
        }

        return stringBuilder.toString();
    }
}
