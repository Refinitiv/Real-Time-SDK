/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryService;
import com.refinitiv.ema.domain.directory.DirectoryUpdate;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.BitSet;

/**
 * Internal {@link DirectoryUpdate} implementation.
 * <p>
 * OMM provider applications use directory update messages to publish
 * incremental changes for one or more services. The inherited
 * {@link #serviceList()} payload contains the affected services, and each
 * {@link DirectoryService} entry indicates whether it represents an add,
 * update, or delete action.
 * <p>
 * Optional members such as the sequence number and filter must be checked for
 * presence before they are accessed.
 *
 * @see DirectoryMsgWithPayloadImpl
 * @see DirectoryUpdate
 * @see DirectoryService
 */
final class DirectoryUpdateImpl extends DirectoryMsgWithPayloadImpl<UpdateMsg> implements DirectoryUpdate
{
    private final UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
    private final BitSet flags = new BitSet();
    private static final int HAS_SEQUENCE_NUMBER_FLAG = 0;
    private static final int HAS_FILTER_FLAG = 1;
    private static final int DO_NOT_CACHE_FLAG = 2;
    private static final int DO_NOT_CONFLATE_FLAG = 3;
    private long sequenceNumber;

    DirectoryUpdateImpl()
    {
        clear();
    }

    /**
     * Clears the current contents of this directory update and prepares the
     * instance for reuse.
     *
     * @return this directory update instance
     */
    @Override
    public DirectoryUpdate clear()
    {
        super.clear();
        flags.clear();
        sequenceNumber = 0;
        updateMsg.clear();
        return this;
    }

    /**
     * Indicates whether this update currently carries a sequence number.
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
     * Indicates whether this update currently carries an explicit filter.
     *
     * @return {@code true} if {@link #filter()} is available; otherwise
     *         {@code false}
     */
    @Override
    public boolean checkHasFilter()
    {
        return flags.get(HAS_FILTER_FLAG);
    }

    /**
     * Marks the optional filter member as present.
     */
    private void applyHasFilter()
    {
        flags.set(HAS_FILTER_FLAG);
    }

    /**
     * Indicates whether the do-not-cache flag is set on this update message.
     * <p>
     * When this method returns {@code true}, receivers should not cache this
     * update.
     *
     * @return {@code true} if this update should not be cached; otherwise
     *         {@code false}
     */
    @Override
    public boolean doNotCache()
    {
        return flags.get(DO_NOT_CACHE_FLAG);
    }

    /**
     * Sets or clears the do-not-cache flag on this update message.
     * <p>
     * When encoded with {@code true}, this instructs receivers that the update
     * should not be cached.
     *
     * @param value {@code true} to mark this update as non-cacheable;
     *              {@code false} to allow normal caching
     * @return this directory update instance
     */
    @Override
    public DirectoryUpdate doNotCache(boolean value)
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
     * Indicates whether the do-not-conflate flag is set on this update message.
     * <p>
     * When this method returns {@code true}, receivers should not conflate this
     * update with other updates.
     *
     * @return {@code true} if this update should not be conflated; otherwise
     *         {@code false}
     */
    @Override
    public boolean doNotConflate()
    {
        return flags.get(DO_NOT_CONFLATE_FLAG);
    }

    /**
     * Sets or clears the do-not-conflate flag on this update message.
     * <p>
     * When encoded with {@code true}, this instructs receivers not to conflate
     * the update with other updates.
     *
     * @param value {@code true} to mark this update as non-conflatable;
     *              {@code false} to allow normal conflation
     * @return this directory update instance
     */
    @Override
    public DirectoryUpdate doNotConflate(boolean value)
    {
        if (value)
        {
            flags.set(DO_NOT_CONFLATE_FLAG);
        }
        else
        {
            flags.clear(DO_NOT_CONFLATE_FLAG);
        }
        return this;
    }

    /**
     * Returns the optional sequence number carried by this update.
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
     * Sets the sequence number for this update.
     * <p>
     * Calling this method marks the optional sequence-number member as present.
     *
     * @param sequenceNumber the sequence number
     * @return this directory update instance
     */
    @Override
    public DirectoryUpdate sequenceNumber(long sequenceNumber)
    {
        this.sequenceNumber = sequenceNumber;

        applyHasSequenceNumber();
        return this;
    }

    /**
     * Returns the directory filter carried by this update.
     *
     * @return the filter value composed of {@link EmaRdm} directory filter bits
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
     * Sets the directory filter for this update.
     * <p>
     * The filter indicates which service filter sections may appear on the
     * stream and, where possible, should match the consumer's request.
     * Calling this method marks the optional filter member as present.
     *
     * @param filter the directory filter composed of {@link EmaRdm} bits
     * @return this directory update instance
     */
    @Override
    public DirectoryUpdate filter(long filter)
    {
        this.filter = filter;

        applyHasFilter();
        return this;
    }

    /**
     * Populates this directory update from the supplied {@link UpdateMsg}.
     * <p>
     * The message must not be {@code null}, must have the
     * {@link EmaRdm#MMT_DIRECTORY} domain type, and must carry a
     * {@link DataType.DataTypes#MAP} payload containing directory service
     * entries.
     *
     * @param msg the update message used to populate this directory update
     * @return this directory update instance
     * @throws OmmInvalidUsageException if {@code msg} is {@code null}, has an
     *                                  unexpected domain type, or does not carry
     *                                  a map payload
     */
    @Override
    public DirectoryUpdate message(UpdateMsg msg)
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
        if (msg.hasSeqNum())
        {
            sequenceNumber(msg.seqNum());
        }
        doNotCache(msg.doNotCache());
        doNotConflate(msg.doNotConflate());
        decodeServiceList(msg.payload().map());
                return this;
    }

    @Override
    public DirectoryUpdate streamId(int streamId)
    {
        super.streamId(streamId);
        return this;
    }

    @Override
    public DirectoryUpdate serviceList(java.util.List<DirectoryService> serviceList)
    {
        super.serviceList(serviceList);
        return this;
    }

    /**
     * Creates an {@link UpdateMsg} view of the current directory-update state.
     * <p>
     * The returned message reflects the current stream ID, optional filter and
     * sequence number, do-not-cache and do-not-conflate flags, and the encoded
     * service list payload.
     *
     * @return the encoded EMA update message representation of this directory update
     */
    @Override
    public UpdateMsg message()
    {
        updateMsg.clear();
        updateMsg.domainType(domainType());
        updateMsg.streamId(streamId());
        if(checkHasSequenceNumber())
        {
            updateMsg.seqNum(sequenceNumber);
        }
        if(checkHasFilter())
        {
            updateMsg.filter(filter);
        }
        updateMsg.doNotCache(doNotCache());
        updateMsg.doNotConflate(doNotConflate());
        encodeServiceList();
        updateMsg.payload(payload());

        return updateMsg;
    }

    /**
     * Replaces the contents of this object with a deep copy of the supplied
     * {@link DirectoryUpdate}.
     * <p>
     * Service entries are copied into newly created {@link DirectoryServiceImpl}
     * instances so that the copied message does not share mutable state with the
     * source.
     *
     * @param sourceUpdateMsg the source directory update to copy from; cannot be
     *                        {@code null}
     * @return this directory update instance
     * @throws OmmInvalidUsageException if {@code sourceUpdateMsg} is {@code null}
     */
    @Override
    public DirectoryUpdate copy(DirectoryUpdate sourceUpdateMsg)
    {
        if (sourceUpdateMsg == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceUpdateMsg can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceUpdateMsg == this)
        {
            return this;
        }

        clear();

        streamId(sourceUpdateMsg.streamId());
        if (sourceUpdateMsg.checkHasFilter())
        {
            filter(sourceUpdateMsg.filter());
        }
        if (sourceUpdateMsg.checkHasSequenceNumber())
        {
            sequenceNumber(sourceUpdateMsg.sequenceNumber());
        }
        doNotCache(sourceUpdateMsg.doNotCache());
        doNotConflate(sourceUpdateMsg.doNotConflate());

        for (DirectoryService service : sourceUpdateMsg.serviceList())
        {
            DirectoryServiceImpl sourceRDMService = new DirectoryServiceImpl();
            sourceRDMService.copy(service);
            serviceList().add(sourceRDMService);
        }
        return this;
    }

    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "DirectoryUpdate: " + EOL);

        if (checkHasSequenceNumber())
        {
            stringBuilder.append(TAB)
                    .append("sequenceNumber: ")
                    .append(sequenceNumber())
                    .append(EOL);
        }

        stringBuilder.append(TAB)
                .append("doNotCache: ")
                .append(doNotCache())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("doNotConflate: ")
                .append(doNotConflate())
                .append(EOL);

        if (checkHasFilter())
        {
            stringBuilder.append(filterAsString());
        }

        for(DirectoryService service : serviceList())
        {
            stringBuilder.append(service.toString());
        }
        stringBuilder.append(EOL);

        return stringBuilder.toString();
    }
}
