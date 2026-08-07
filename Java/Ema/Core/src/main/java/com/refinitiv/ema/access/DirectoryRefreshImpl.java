/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryRefresh;
import com.refinitiv.ema.domain.directory.DirectoryService;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.*;

import java.util.*;

import static com.refinitiv.ema.access.DirectoryValidators.isValidServiceId;
import static com.refinitiv.ema.access.DirectoryValidators.validateStatus;


/**
 * Internal implementation of the EMA RDM directory refresh message.
 * <p>
 * OMM provider applications use directory refresh messages to publish the
 * current directory image for one or more services. The inherited
 * {@link #serviceList()} payload contains the service entries carried by the
 * refresh.
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
 *
 * @see DirectoryMsgWithPayloadImpl
 * @see DirectoryRefresh
 * @see DirectoryService
 */
final class DirectoryRefreshImpl extends DirectoryMsgWithPayloadImpl<RefreshMsg> implements DirectoryRefresh
{
    private final State rsslState = CodecFactory.createState();
    private final Buffer stateText = CodecFactory.createBuffer();
    private final OmmStateImpl state = new OmmStateImpl();
    private final RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
    private final BitSet flags = new BitSet();
    private static final int HAS_SEQUENCE_NUMBER_FLAG = 0;
    private static final int CLEAR_CACHE_FLAG = 1;
    private static final int DO_NOT_CACHE_FLAG = 2;
    private static final int COMPLETE_FLAG = 3;
    private static final int SOLICITED_FLAG = 4;
    private static final int HAS_SERVICE_ID_FLAG = 5;
    private long sequenceNumber;
    private int serviceId;

    DirectoryRefreshImpl()
    {
        clear();
    }

    /**
     * Clears the current contents of this directory refresh and prepares the
     * instance for reuse.
     *
     * @return this directory refresh instance
     */
    @Override
    public DirectoryRefresh clear()
    {
        super.clear();
        sequenceNumber = 0;
        serviceId = 0;
        flags.clear();

        rsslState.clear();
        rsslState.streamState(StreamStates.OPEN);
        rsslState.dataState(DataStates.OK);
        rsslState.code(StateCodes.NONE);
        stateText.data("");
        rsslState.text(stateText);

        refreshMsg.clear();
        return this;
    }

    /**
     * Populates this directory refresh from the supplied {@link RefreshMsg}.
     * <p>
     * The message must not be {@code null}, must have the
     * {@link EmaRdm#MMT_DIRECTORY} domain type, and must carry a
     * {@link DataType.DataTypes#MAP} payload containing directory service
     * entries.
     *
     * @param msg the refresh message used to populate this directory refresh
     * @return this directory refresh instance
     * @throws OmmInvalidUsageException if {@code msg} is {@code null}, has an
     *                                  unexpected domain type, or does not carry
     *                                  a map payload
     */
    @Override
    public DirectoryRefresh message(RefreshMsg msg)
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

        if (msg.payload().dataType() != DataType.DataTypes.MAP)
        {
            throw new OmmInvalidUsageExceptionImpl().message("Payload data type should be Map.",
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
        if (msg.hasSeqNum())
        {
            sequenceNumber(msg.seqNum());
        }
        complete(msg.complete());
        clearCache(msg.clearCache());
        doNotCache(msg.doNotCache());
        solicited(msg.solicited());
        state(msg.state());
        decodeServiceList(msg.payload().map());
        return this;
    }

    @Override
    public DirectoryRefresh streamId(int streamId)
    {
        super.streamId(streamId);
        return this;
    }

    @Override
    public DirectoryRefresh filter(long filter)
    {
        super.filter(filter);
        return this;
    }

    @Override
    public DirectoryRefresh serviceList(List<DirectoryService> serviceList)
    {
        super.serviceList(serviceList);
        return this;
    }

    /**
     * Creates a {@link RefreshMsg} view of the current directory-refresh state.
     * <p>
     * The returned message reflects the current stream ID, filter, state,
     * optional service identifier, optional sequence number, refresh flags, and
     * encoded service-list payload.
     *
     * @return the encoded EMA refresh message representation of this directory refresh
     */
    @Override
    public RefreshMsg message()
    {
        refreshMsg.clear();
        refreshMsg.domainType(domainType());
        refreshMsg.streamId(streamId());
        if (checkHasSequenceNumber())
        {
            refreshMsg.seqNum(sequenceNumber);
        }
        if (checkHasServiceId())
        {
            refreshMsg.serviceId(serviceId);
        }
        refreshMsg.state(rsslState.streamState(), rsslState.dataState(), rsslState.code(), rsslState.text().toString());
        refreshMsg.filter(filter);
        refreshMsg.solicited(solicited());
        refreshMsg.complete(complete());
        refreshMsg.clearCache(clearCache());
        refreshMsg.doNotCache(doNotCache());
        encodeServiceList();
        refreshMsg.payload(payload());

        return refreshMsg;
    }

    /**
     * Replaces the contents of this object with a deep copy of the supplied
     * {@link DirectoryRefresh}.
     * <p>
     * Service entries are copied into newly created {@link DirectoryServiceImpl}
     * instances so that the copied message does not share mutable state with the
     * source.
     *
     * @param sourceRefreshMsg the source directory refresh to copy from; cannot
     *                         be {@code null}
     * @return this directory refresh instance
     * @throws OmmInvalidUsageException if {@code sourceRefreshMsg} is {@code null}
     */
    @Override
    public DirectoryRefresh copy(DirectoryRefresh sourceRefreshMsg)
    {
        if (sourceRefreshMsg == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceRefreshMsg can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceRefreshMsg == this)
        {
            return this;
        }

        clear();

        streamId(sourceRefreshMsg.streamId());
        filter(sourceRefreshMsg.filter());
        if (sourceRefreshMsg.checkHasServiceId())
        {
            serviceId(sourceRefreshMsg.serviceId());
        }
        if (sourceRefreshMsg.checkHasSequenceNumber())
        {
            sequenceNumber(sourceRefreshMsg.sequenceNumber());
        }
        complete(sourceRefreshMsg.complete());
        clearCache(sourceRefreshMsg.clearCache());
        doNotCache(sourceRefreshMsg.doNotCache());
        solicited(sourceRefreshMsg.solicited());
        state(sourceRefreshMsg.state());

        for (DirectoryService service : sourceRefreshMsg.serviceList())
        {
            DirectoryServiceImpl sourceRDMService = new DirectoryServiceImpl();
            sourceRDMService.copy(service);
            serviceList().add(sourceRDMService);
        }
        return this;
    }

    /**
     * Indicates whether this refresh message currently carries a service
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

    /** Marks refresh message as having a service identifier selector. */
    private void applyHasServiceId()
    {
        flags.set(HAS_SERVICE_ID_FLAG);
    }

    /**
     * Indicates whether this refresh currently carries a sequence number.
     *
     * @return {@code true} if {@link #sequenceNumber()} is available; otherwise
     *         {@code false}
     */
    @Override
    public boolean checkHasSequenceNumber()
    {
        return flags.get(HAS_SEQUENCE_NUMBER_FLAG);
    }

    /**
     * Marks the optional sequence-number member as present.
     */
    private void applyHasSequenceNumber()
    {
        flags.set(HAS_SEQUENCE_NUMBER_FLAG);
    }

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
    @Override
    public boolean clearCache()
    {
        return flags.get(CLEAR_CACHE_FLAG);
    }

    /**
     * Sets or clears the clear-cache flag on this refresh message.
     * <p>
     * When encoded with {@code true}, this instructs receivers to discard any
     * cached directory information associated with the stream before applying
     * the refresh.
     *
     * @param value {@code true} to request cache clearing before the refresh is
     *              applied; {@code false} to leave cached data intact
     * @return this directory refresh instance
     */
    @Override
    public DirectoryRefresh clearCache(boolean value)
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
     * Indicates whether the do-not-cache flag is set on this refresh message.
     * <p>
     * When this method returns {@code true}, receivers should not cache this
     * refresh.
     *
     * @return {@code true} if this refresh should not be cached; otherwise
     *         {@code false}
     */
    @Override
    public boolean doNotCache()
    {
        return flags.get(DO_NOT_CACHE_FLAG);
    }

    /**
     * Sets or clears the do-not-cache flag on this refresh message.
     * <p>
     * When encoded with {@code true}, this instructs receivers that the refresh
     * should not be cached.
     *
     * @param value {@code true} to mark this refresh as non-cacheable;
     *              {@code false} to allow normal caching
     * @return this directory refresh instance
     */
    @Override
    public DirectoryRefresh doNotCache(boolean value)
    {
        if (value)
        {
            flags.set(DO_NOT_CACHE_FLAG);
        }
        else
        {
            flags.clear(DO_NOT_CACHE_FLAG);
        }
        return this;
    }

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
    @Override
    public boolean complete()
    {
        return flags.get(COMPLETE_FLAG);
    }

    /**
     * Sets or clears the refresh-complete flag on this refresh message.
     * <p>
     * When encoded with {@code true}, this marks the message as a complete
     * refresh for the stream or as the final part of a multipart refresh.
     *
     * @param value {@code true} to mark this refresh as complete;
     *              {@code false} to indicate that additional refresh parts may
     *              follow
     * @return this directory refresh instance
     */
    @Override
    public DirectoryRefresh complete(boolean value)
    {
        if (value)
        {
            flags.set(COMPLETE_FLAG);
        }
        else
        {
            flags.clear(COMPLETE_FLAG);
        }
        return this;
    }

    /**
     * Indicates whether the solicited flag is currently set on this refresh
     * message.
     * <p>
     * A solicited refresh is sent in response to a request.
     *
     * @return {@code true} if this refresh was sent in response to a request;
     *         otherwise {@code false}
     */
    @Override
    public boolean solicited()
    {
        return flags.get(SOLICITED_FLAG);
    }

    /**
     * Sets or clears the solicited flag on this refresh message.
     * <p>
     * When encoded, a solicited refresh indicates that it was sent in response
     * to a request.
     *
     * @param value {@code true} to mark the refresh as solicited;
     *              {@code false} to clear the solicited flag
     * @return this directory refresh instance
     */
    @Override
    public DirectoryRefresh solicited(boolean value)
    {
        if (value)
        {
            flags.set(SOLICITED_FLAG);
        }
        else
        {
            flags.clear(SOLICITED_FLAG);
        }
        return this;
    }

    /**
     * Returns the optional service identifier carried by this refresh message.
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
     * Sets the service identifier carried by this refresh message.
     * <p>
     * Calling this method marks the optional service-identifier field as
     * present. The {@code serviceId} value must be between {@code 0} and
     * {@code 65535}, inclusive.
     *
     * @param serviceId the service identifier
     * @return this directory refresh instance
     * @throws OmmInvalidUsageException if {@code serviceId} is
     *                                  outside the valid range
     */
    @Override
    public DirectoryRefresh serviceId(int serviceId)
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
     * Returns the optional sequence number carried by this message.
     *
     * @return the sequence number
     * @throws OmmInvalidUsageException if {@link #checkHasSequenceNumber()}
     *                                  returns {@code false}
     */
    @Override
    public long sequenceNumber()
    {
        if (!checkHasSequenceNumber())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SEQ_NUM + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return sequenceNumber;
    }

    /**
     * Sets the sequence number for this message.
     * <p>
     * Calling this method marks the optional sequence-number member as present.
     *
     * @param sequenceNumber the sequence number
     * @return this directory refresh instance
     */
    @Override
    public DirectoryRefresh sequenceNumber(long sequenceNumber)
    {
        this.sequenceNumber = sequenceNumber;

        applyHasSequenceNumber();
        return this;
    }

    /**
     * Returns the current state carried by this directory refresh message.
     *
     * @return the current {@link OmmState}
     */
    @Override
    public OmmState state()
    {
        state.decode(rsslState);

        return state;
    }

    /**
     * Sets the state for this directory refresh message.
     *
     * @param state the state to copy from
     * @return this directory refresh instance
     * @throws OmmInvalidUsageException if {@code state} is {@code null}, if it
     *                                  contains a {@code null} status text, or
     *                                  if any state component is not supported
     */
    @Override
    public DirectoryRefresh state(OmmState state)
    {
        if (state == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("state can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        return state(state.streamState(), state.dataState(), state.statusCode(), state.statusText());
    }

    /**
     * Sets the state for this directory refresh message using individual
     * {@link OmmState} components.
     *
     * @param streamState the {@link OmmState.StreamState} value
     * @param dataState the {@link OmmState.DataState} value
     * @param statusCode the {@link OmmState.StatusCode} value
     * @param statusText the human-readable status text
     * @return this directory refresh instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or
     *                                  if any state component is not supported
     */
    @Override
    public DirectoryRefresh state(int streamState, int dataState, int statusCode, String statusText)
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
        return this;
    }

    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "DirectoryRefresh: " + EOL);

        if (checkHasServiceId())
        {
            stringBuilder.append(TAB)
                    .append("serviceId: ")
                    .append(serviceId())
                    .append(EOL);
        }

        stringBuilder.append(TAB)
                .append("state: ")
                .append(state())
                .append(EOL);

        if (checkHasSequenceNumber())
        {
            stringBuilder.append(TAB)
                    .append("sequenceNumber: ")
                    .append(sequenceNumber())
                    .append(EOL);
        }

        stringBuilder.append(TAB)
                .append("complete: ")
                .append(complete())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("clearCache: ")
                .append(clearCache())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("doNotCache: ")
                .append(doNotCache())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("solicited: ")
                .append(solicited())
                .append(EOL);

        stringBuilder.append(filterAsString());

        for(DirectoryService service : serviceList())
        {
            stringBuilder.append(service);
        }

        return stringBuilder.toString();
    }
}
