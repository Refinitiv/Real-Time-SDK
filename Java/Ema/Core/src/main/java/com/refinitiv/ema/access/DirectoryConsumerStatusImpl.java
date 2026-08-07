/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryConsumerStatus;
import com.refinitiv.ema.domain.directory.DirectoryConsumerStatusService;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.ArrayList;
import java.util.List;

/**
 * Default {@link DirectoryConsumerStatus} implementation.
 * <p>
 * An OMM consumer sends a Directory Consumer Status message to describe how one
 * or more services are being used for source mirroring and warm-standby
 * workflows. The message is carried as an EMA {@link GenericMsg} in the
 * Directory domain whose name is {@link EmaRdm#ENAME_CONS_STATUS}.
 * <p>
 * This implementation stores a live list of
 * {@link DirectoryConsumerStatusService} entries. The list returned by
 * {@link #consumerServiceStatusList()} is the internal list used by this
 * object, while {@link #copy(DirectoryConsumerStatus)} return deep copies
 * of service entries.
 * <p>
 * Instances may be cleared and re-used.
 *
 * @see DirectoryMsgImpl
 * @see DirectoryConsumerStatus
 * @see DirectoryConsumerStatusService
 */
final class DirectoryConsumerStatusImpl extends DirectoryMsgImpl<GenericMsg> implements DirectoryConsumerStatus
{
    private final List<DirectoryConsumerStatusService> consumerServiceStatusList = new ArrayList<>();
    private final GenericMsg genericMsg = EmaFactory.createGenericMsg();
    private final Map payload = EmaFactory.createMap();
    private long sequenceNumber;
    private boolean hasSequenceNumber;

    DirectoryConsumerStatusImpl()
    {
    }

    /**
     * Resets this message to its default state.
     * <p>
     * After calling this method, the stored stream id is reset by the base
     * class, the optional sequence number is removed, the consumer-service list
     * becomes empty, and any cached EMA container/message objects are cleared so
     * this instance may be populated again.
     *
     * @return this directory consumer-status message instance
     */
    @Override
    public DirectoryConsumerStatus clear()
    {
        super.clear();
        sequenceNumber = 0;
        hasSequenceNumber = false;
        consumerServiceStatusList.clear();
        payload.clear();
        genericMsg.clear();
        return this;
    }

    /**
     * Returns the fixed RDM name for this message type.
     *
     * @return {@link EmaRdm#ENAME_CONS_STATUS}
     */
    @Override
    public String name()
    {
        return EmaRdm.ENAME_CONS_STATUS;
    }

    /**
     * Returns the optional sequence number carried by this message.
     *
     * @return the sequence number
     * @throws OmmInvalidUsageException if {@link #checkHasSequenceNumber()} returns {@code false}
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
     * Sets the optional sequence number for this message.
     * <p>
     * Calling this method marks the sequence-number field as present.
     *
     * @param sequenceNumber the sequence number to store
     * @return this directory consumer-status message instance
     */
    @Override
    public DirectoryConsumerStatus sequenceNumber(long sequenceNumber)
    {
        this.sequenceNumber = sequenceNumber;

        applyHasSequenceNumber();
        return this;
    }

    /**
     * Indicates whether this message currently contains a sequence number.
     *
     * @return {@code true} if {@link #sequenceNumber()} may be called; otherwise {@code false}
     */
    @Override
    public boolean checkHasSequenceNumber()
    {
        return hasSequenceNumber;
    }

    /**
     * Applies the sequence number flag.
     *
     */
    private void applyHasSequenceNumber()
    {
        hasSequenceNumber = true;
    }

    /**
     * Returns the consumer-status service entries carried by this message.
     * <p>
     * The returned list is the live internal list used by this object. Changes
     * made to the list or to the contained
     * {@link DirectoryConsumerStatusService} objects are reflected immediately in
     * this message instance.
     *
     * @return the live list of consumer-status service entries
     */
    @Override
    public List<DirectoryConsumerStatusService> consumerServiceStatusList()
    {
        return consumerServiceStatusList;
    }

    /**
     * Replaces this message's consumer-status service entries with those from
     * the supplied list.
     * <p>
     * The list reference itself is not retained. Instead, this object clears its
     * current entries and copies the supplied entry references into its internal
     * live list. The individual {@link DirectoryConsumerStatusService} objects
     * are not deep-copied by this method.
     *
     * @param consumerServiceStatusList list of consumer-status service entries to assign
     * @return this directory consumer-status message instance
     * @throws OmmInvalidUsageException if {@code consumerServiceStatusList} is {@code null}
     */
    @Override
    public DirectoryConsumerStatus consumerServiceStatusList(List<DirectoryConsumerStatusService> consumerServiceStatusList)
    {
        if (consumerServiceStatusList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("consumerServiceStatusList must be non-null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.consumerServiceStatusList == consumerServiceStatusList)
        {
            return this;
        }

        this.consumerServiceStatusList.clear();
        this.consumerServiceStatusList.addAll(consumerServiceStatusList);
        return this;
    }

    /**
     * Populates this object from an EMA {@link GenericMsg}.
     * <p>
     * The supplied message must be a Directory-domain generic message named
     * {@link EmaRdm#ENAME_CONS_STATUS} with a {@link Map} payload. If present,
     * the message sequence number is copied. The map payload is then decoded
     * into this object's consumer-service list.
     *
     * @param msg encoded consumer-status generic message to decode
     * @return this directory consumer-status message instance
     * @throws OmmInvalidUsageException if {@code msg} is {@code null}, belongs
     * to the wrong domain, has an unexpected name, or carries a payload that
     * does not match the expected consumer-status map structure
     */
    @Override
    public DirectoryConsumerStatus message(GenericMsg msg)
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

        if (!msg.hasName())
        {
            throw new OmmInvalidUsageExceptionImpl().message("Message name is absent.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (!msg.name().equals(EmaRdm.ENAME_CONS_STATUS))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Message name is incorrect.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (msg.payload().dataType() != DataType.DataTypes.MAP)
        {
            throw new OmmInvalidUsageExceptionImpl().message("Payload data type should be Map.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        streamId(msg.streamId());
        if (msg.hasSeqNum())
        {
            sequenceNumber(msg.seqNum());
        }
        decodeServiceList(msg.payload().map());
        return this;
    }

    @Override
    public DirectoryConsumerStatus streamId(int streamId)
    {
        super.streamId(streamId);
        return this;
    }

    /**
     * Decodes the consumer-status service map payload.
     * <p>
     * Each map entry must use a {@code UINT} key representing the service id.
     * {@code ADD} and {@code UPDATE} entries are expected to contain an
     * {@link ElementList} payload decoded by
     * {@link DirectoryConsumerStatusService#decode(ElementList)}. {@code DELETE}
     * entries may omit the payload and therefore use {@code NO_DATA}. In all
     * cases, the service id and map-entry action are taken from the enclosing map
     * entry and stored on the decoded service object.
     *
     * @param map map payload containing consumer-status service entries
     * @throws OmmInvalidUsageException if a map entry uses an unexpected key or
     * payload type, or if a contained service payload cannot be decoded
     */
    private void decodeServiceList(Map map)
    {
        for (MapEntry mapEntry : map)
        {
            if (mapEntry.key().data().dataType() != DataType.DataTypes.UINT)
            {
                throw new OmmInvalidUsageExceptionImpl().message("Unexpected map entry key type: " +
                                DataType.asString(mapEntry.key().data().dataType()),
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
            }

            if (mapEntry.loadType() == DataType.DataTypes.ELEMENT_LIST ||
                    (mapEntry.action() == MapEntry.MapAction.DELETE && mapEntry.loadType() == DataType.DataTypes.NO_DATA))
            {
                int serviceId = (int) mapEntry.key().uintValue();
                DirectoryConsumerStatusService service = new DirectoryConsumerStatusServiceImpl();
                if (mapEntry.loadType() == DataType.DataTypes.ELEMENT_LIST)
                {
                    service.decode(mapEntry.elementList());
                }
                service.serviceId(serviceId);
                service.action(mapEntry.action());
                consumerServiceStatusList.add(service);
            }
            else
            {
                throw new OmmInvalidUsageExceptionImpl().message("Unexpected map entry payload: " +
                                DataType.asString(mapEntry.loadType()),
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
            }
        }
    }

    /**
     * Creates an EMA {@link GenericMsg} representing the current RDM state.
     * <p>
     * The returned message is a Directory-domain generic message named
     * {@link #name()}. If a sequence number is present it is applied to the
     * message. The current service list is encoded into the message payload as a
     * {@link Map} keyed by service id.
     *
     * @return encoded {@link GenericMsg} for this consumer-status object
     */
    @Override
    public GenericMsg message()
    {
        genericMsg.clear();
        genericMsg.domainType(domainType());
        genericMsg.streamId(streamId());
        genericMsg.name(name());
        if (checkHasSequenceNumber())
        {
            genericMsg.seqNum(sequenceNumber);
        }
        encodeServiceList();
        genericMsg.payload(payload);

        return genericMsg;
    }

    /**
     * Encodes the current consumer-status service entries into the cached map
     * payload.
     * <p>
     * Each service entry is encoded as a map entry whose key is the service id,
     * whose action is the service action, and whose payload is the encoded
     * {@link ElementList} returned by
     * {@link DirectoryConsumerStatusService#encode()}.
     */
    private void encodeServiceList()
    {
        payload.clear();
        payload.keyType(DataType.DataTypes.UINT);
        payload.totalCountHint(consumerServiceStatusList.size());

        for (DirectoryConsumerStatusService service : consumerServiceStatusList)
        {
            MapEntry mapEntry = EmaFactory.createMapEntry();
            mapEntry.keyUInt(service.serviceId(), service.action(), service.encode());

            payload.add(mapEntry);
        }
    }

    /**
     * Replaces this object with a deep copy of another consumer-status message.
     * <p>
     * The current state is cleared before copying the source stream id, optional
     * sequence number, and deep copies of all consumer-status service entries.
     * Passing this instance results in a no-op.
     *
     * @param sourceConsumerStatus source consumer-status message to copy from;
     * must not be {@code null}
     * @return this directory consumer-status message instance
     * @throws OmmInvalidUsageException if {@code sourceConsumerStatus} is {@code null}
     */
    @Override
    public DirectoryConsumerStatus copy(DirectoryConsumerStatus sourceConsumerStatus)
    {
        if (sourceConsumerStatus == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceConsumerStatus can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceConsumerStatus == this)
        {
            return this;
        }

        clear();

        streamId(sourceConsumerStatus.streamId());
        if (sourceConsumerStatus.checkHasSequenceNumber())
        {
            sequenceNumber(sourceConsumerStatus.sequenceNumber());
        }

        for (DirectoryConsumerStatusService service : sourceConsumerStatus.consumerServiceStatusList())
        {
            DirectoryConsumerStatusService sourceRDMService = new DirectoryConsumerStatusServiceImpl();
            sourceRDMService.copy(service);
            consumerServiceStatusList.add(sourceRDMService);
        }
        return this;
    }

    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "DirectoryConsumerStatus: " + EOL);

        if (checkHasSequenceNumber())
        {
            stringBuilder.append(TAB)
                    .append("sequenceNumber: ")
                    .append(sequenceNumber())
                    .append(EOL);
        }

        for (DirectoryConsumerStatusService service : consumerServiceStatusList())
        {
            stringBuilder.append(service);
        }

        return stringBuilder.toString();
    }
}
