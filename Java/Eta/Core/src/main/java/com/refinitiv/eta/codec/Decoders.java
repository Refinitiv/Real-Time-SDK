package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.BufferImpl;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.FilterEntry;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.FilterList;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.LocalElementSetDefDb;
import com.refinitiv.eta.codec.LocalFieldSetDefDb;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyImpl;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.Series;
import com.refinitiv.eta.codec.SeriesEntry;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.codec.Vector;
import com.refinitiv.eta.codec.VectorEntry;
import com.refinitiv.eta.codec.VectorEntryActions;

class Decoders
{
    private static int real32LenHints[] = { 2, 3, 4, 5 };
    private static int real64LenHints[] = { 3, 5, 7, 9 };
    private static final int[] lens = new int[81];
    private static final int POST_USER_INFO_SIZE = 8;
    static
    {
        lens[64] = lens[65] = 1;
        lens[66] = lens[67] = 2;
        lens[77] = 3;
        lens[68] = lens[69] = lens[72] = lens[76] = 4;
        lens[78] = 5;
        lens[79] = 7;
        lens[70] = lens[71] = lens[73] = 8;
        lens[80] = 9;
        lens[74] = -1;
        lens[75] = -2;
    }

    static int decodeMsg(DecodeIterator iterInt, Msg msg)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int position;
        DecodingLevel _levelInfo;

        if (++iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._decodingLevel];
        _levelInfo._containerType = DataTypes.MSG;

        try
        {
            decodeMsgHeader(iter, msg);
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        /* Maintain this use of the position variable for now, in case there is public use of this method. */
        iter._curBufPos = iter._reader.position();
        position = iter._curBufPos;

        if (position > _levelInfo._endBufPos)
            return CodecReturnCodes.INCOMPLETE_DATA;

        if ((_levelInfo._endBufPos - position) > 0)
        {
            ((BufferImpl)msg.encodedDataBody()).data_internal(iter._reader.buffer(), position,
                                                              (_levelInfo._endBufPos - position));

            if (!canDecodeContainerType(msg.containerType()))
            {
                /* UPA has no decoders for this format(e.g. Opaque). Move past it. */
                iter._curBufPos += msg.encodedDataBody().length();
                try
                {
                    iter._reader.skipBytes(msg.encodedDataBody().length());
                }
                catch (Exception e)
                {
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }
                iter._curBufPos = iter._reader.position();
                endOfList(iter);
                return CodecReturnCodes.SUCCESS;
            }

            /* For now, _endBufPos for msg and data levels is the same. */
            iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._endBufPos;

            return CodecReturnCodes.SUCCESS;
        }
        else
        {
            /* No payload. Reset iterator and return. */
            msg.encodedDataBody().clear();
            endOfList(iter);
            return CodecReturnCodes.SUCCESS;
        }
    }

    private static void decodeMsgHeader(DecodeIterator iterInt, Msg msg) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int startPosition = iter._curBufPos;
        int headerSize = 0;
        int keySize = 0;

        iter._reader.position(startPosition);

        /* header size */
        headerSize = iter._reader.readUnsignedShort();

        msg.msgClass(iter._reader.readByte());

        /* mask msgClass top three bits for reserved use later */
        /* Top three bits reserved for later use */
        msg.msgClass(msg.msgClass() & 0x1F);

        msg.domainType(iter._reader.readUnsignedByte());
        msg.streamId(iter._reader.readInt());
        iter._curBufPos = iter._reader._position; // adjust the iterator's position to match the reader's position.

        ((MsgImpl)msg).encodedMsgBuffer(iter._buffer.data(), startPosition, iter._buffer.length());

        /* IMPORTANT: When new message classes are added, CopyMsg and ValidateMsg have to be modified as well */

        switch (msg.msgClass())
        {
            case MsgClasses.UPDATE:
                UpdateMsg updateMsg = (UpdateMsg)msg;

                updateMsg.flags(iter._reader.readUShort15rb());
                // need to scale containerType
                updateMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);

                updateMsg.updateType(iter._reader.readUnsignedByte());

                if (updateMsg.checkHasSeqNum())
                    updateMsg.seqNum(iter._reader.readUnsignedInt());

                if (updateMsg.checkHasConfInfo())
                {
                    updateMsg.conflationCount(iter._reader.readUShort15rb());
                    updateMsg.conflationTime(iter._reader.readUnsignedShort());
                }

                if (updateMsg.checkHasPermData())
                {
                    decodeBuffer15(iter, updateMsg.permData());
                }

                if (updateMsg.checkHasMsgKey())
                {
                    keySize = iter._reader.readUShort15rb();

                    /* don't iterate position by this value anymore. We want to add the keySize to position */
                    int currentPosition = iter._reader.position();
                    decodeBaseKey(iter, updateMsg.msgKey());
                    /* add keySize to position */
                    currentPosition += keySize;
                    iter._reader.position(currentPosition);
                }

                if (updateMsg.checkHasExtendedHdr())
                {
                    decodeBuffer8(iter, updateMsg.extendedHeader());
                }

                if (updateMsg.checkHasPostUserInfo())
                {
                    /* decode only if actually in the header */
                    if ((headerSize - (iter._reader.position() - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                    {
                        /* get user address */
                        updateMsg.postUserInfo().userAddr(iter._reader.readUnsignedInt());
                        /* get user ID */
                        updateMsg.postUserInfo().userId(iter._reader.readUnsignedInt());
                    }
                    else
                    /* not really there, unset flag */
                    {
                        int flags = updateMsg.flags();
                        flags &= ~UpdateMsgFlags.HAS_POST_USER_INFO;
                        updateMsg.flags(flags);
                    }
                }
                break;
            case MsgClasses.REFRESH:
                RefreshMsg refreshMsg = (RefreshMsg)msg;

                refreshMsg.flags(iter._reader.readUShort15rb());
                // need to scale containerType
                refreshMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);

                if (refreshMsg.checkHasSeqNum())
                    refreshMsg.seqNum(iter._reader.readUnsignedInt());

                decodeStateInMsg(iter, refreshMsg.state());
                decodeBuffer8(iter, refreshMsg.groupId());

                if (refreshMsg.checkHasPermData())
                {
                    decodeBuffer15(iter, refreshMsg.permData());
                }

                if (refreshMsg.checkHasQos())
                {
                    decodeQosInMsg(iter, refreshMsg.qos());
                }

                if (refreshMsg.checkHasMsgKey())
                {
                    keySize = iter._reader.readUShort15rb();
                    /* don't iterate position by this value anymore. We want to add the keySize to position */
                    int currentPosition = iter._reader.position();
                    decodeBaseKey(iter, refreshMsg.msgKey());
                    /* add keySize to position */
                    currentPosition += keySize;
                    iter._reader.position(currentPosition);
                }

                if (refreshMsg.checkHasExtendedHdr())
                {
                    decodeBuffer8(iter, refreshMsg.extendedHeader());
                }

                if (refreshMsg.checkHasPostUserInfo())
                {
                    /* decode only if actually in the header */
                    if ((headerSize - (iter._reader.position() - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                    {
                        /* get user address */
                        refreshMsg.postUserInfo().userAddr(iter._reader.readUnsignedInt());
                        /* get user ID */
                        refreshMsg.postUserInfo().userId(iter._reader.readUnsignedInt());
                    }
                    else
                    /* not really there, unset flag */
                    {
                        int flags = refreshMsg.flags();
                        flags &= ~RefreshMsgFlags.HAS_POST_USER_INFO;
                        refreshMsg.flags(flags);
                    }
                }

                if (refreshMsg.checkHasPartNum())
                {
                    /* decode only if actually in the header */
                    if ((headerSize - (iter._reader.position() - (startPosition + 2))) > 0)
                    {
                        refreshMsg.partNum(iter._reader.readUShort15rb());
                    }
                    else
                    /* not really there, unset flag */
                    {
                        int flags = refreshMsg.flags();
                        flags &= ~RefreshMsgFlags.HAS_PART_NUM;
                        refreshMsg.flags(flags);
                    }
                }
                break;
            case MsgClasses.REQUEST:
                RequestMsg requestMsg = (RequestMsg)msg;
                decodeRequestMsgHeader(iter, requestMsg);
                break;
            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg;

                statusMsg.flags(iter._reader.readUShort15rb());
                // need to scale containerType
                statusMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);

                if (statusMsg.checkHasState())
                {
                    decodeStateInMsg(iter, statusMsg.state());
                }

                if (statusMsg.checkHasGroupId())
                {
                    decodeBuffer8(iter, statusMsg.groupId());
                }

                if (statusMsg.checkHasPermData())
                {
                    decodeBuffer15(iter, statusMsg.permData());
                }

                if (statusMsg.checkHasMsgKey())
                {
                    keySize = iter._reader.readUShort15rb();
                    /* don't iterate position by this value anymore. We want to add the keySize to position */
                    int currentPosition = iter._reader.position();
                    decodeBaseKey(iter, statusMsg.msgKey());
                    /* add keySize to position */
                    currentPosition += keySize;
                    iter._reader.position(currentPosition);
                }

                if (statusMsg.checkHasExtendedHdr())
                {
                    decodeBuffer8(iter, statusMsg.extendedHeader());
                }

                if (statusMsg.checkHasPostUserInfo())
                {
                    /* decode only if actually in the header */
                    if ((headerSize - (iter._reader.position() - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                    {
                        /* get user address */
                        statusMsg.postUserInfo().userAddr(iter._reader.readUnsignedInt());
                        /* get user ID */
                        statusMsg.postUserInfo().userId(iter._reader.readUnsignedInt());
                    }
                    else
                    /* not really there, unset flag */
                    {
                        int flags = statusMsg.flags();
                        flags &= ~StatusMsgFlags.HAS_POST_USER_INFO;
                        statusMsg.flags(flags);
                    }
                }
                break;
            case MsgClasses.CLOSE:
                CloseMsg closeMsg = (CloseMsg)msg;
                decodeCloseMsgHeader(iter, closeMsg);
                break;
            case MsgClasses.ACK:
                AckMsg ackMsg = (AckMsg)msg;

                ackMsg.flags(iter._reader.readUShort15rb());
                // need to scale containerType
                ackMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);
                ackMsg.ackId(iter._reader.readUnsignedInt());

                if (ackMsg.checkHasNakCode())
                {
                    ackMsg.nakCode(iter._reader.readUnsignedByte());
                }

                if (ackMsg.checkHasText())
                {
                    decodeBuffer16(iter, ackMsg.text());
                }

                if (ackMsg.checkHasSeqNum())
                    ackMsg.seqNum(iter._reader.readUnsignedInt());

                if (ackMsg.checkHasMsgKey())
                {
                    keySize = iter._reader.readUShort15rb();
                    /* don't iterate position by this value anymore. We want to add the keySize to position */
                    int currentPosition = iter._reader.position();
                    decodeBaseKey(iter, ackMsg.msgKey());
                    /* add keySize to position */
                    currentPosition += keySize;
                    iter._reader.position(currentPosition);
                }

                if (ackMsg.checkHasExtendedHdr())
                {
                    decodeBuffer8(iter, ackMsg.extendedHeader());
                }
                break;
            case MsgClasses.GENERIC:
                GenericMsg genericMsg = (GenericMsg)msg;

                genericMsg.flags(iter._reader.readUShort15rb());
                // need to scale containerType
                genericMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);

                if (genericMsg.checkHasSeqNum())
                    genericMsg.seqNum(iter._reader.readUnsignedInt());

                if (genericMsg.checkHasSecondarySeqNum())
                    genericMsg.secondarySeqNum(iter._reader.readUnsignedInt());

                if (genericMsg.checkHasPermData())
                {
                    decodeBuffer15(iter, genericMsg.permData());
                }

                if (genericMsg.checkHasMsgKey())
                {
                    keySize = iter._reader.readUShort15rb();

                    /* don't iterate position by this value anymore. We want to add the keySize to position */
                    int currentPosition = iter._reader.position();
                    decodeBaseKey(iter, genericMsg.msgKey());

                    /* add keySize to position */
                    currentPosition += keySize;
                    iter._reader.position(currentPosition);
                }

                if (genericMsg.checkHasExtendedHdr())
                {
                    decodeBuffer8(iter, genericMsg.extendedHeader());
                }

                if (genericMsg.checkHasPartNum())
                {
                    /* decode only if actually in the header */
                    if ((headerSize - (iter._reader.position() - (startPosition + 2))) > 0)
                    {
                        genericMsg.partNum(iter._reader.readUShort15rb());
                    }
                    else
                    /* not really there, unset flag */
                    {
                        int flags = genericMsg.flags();
                        flags &= ~GenericMsgFlags.HAS_PART_NUM;
                        genericMsg.flags(flags);
                    }
                }
                break;
            case MsgClasses.POST:
                PostMsg postMsg = (PostMsg)msg;

                postMsg.flags(iter._reader.readUShort15rb());
                // need to scale containerType
                postMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);

                /* get user address */
                postMsg.postUserInfo().userAddr(iter._reader.readUnsignedInt());
                /* get user ID */
                postMsg.postUserInfo().userId(iter._reader.readUnsignedInt());

                if (postMsg.checkHasSeqNum())
                    postMsg.seqNum(iter._reader.readUnsignedInt());

                if (postMsg.checkHasPostId())
                    postMsg.postId(iter._reader.readUnsignedInt());

                if (postMsg.checkHasPermData())
                {
                    decodeBuffer15(iter, postMsg.permData());
                }

                if (postMsg.checkHasMsgKey())
                {
                    keySize = iter._reader.readUShort15rb();

                    /* don't iterate position by this value anymore. We want to add the keySize to position */
                    int currentPosition = iter._reader.position();
                    decodeBaseKey(iter, postMsg.msgKey());
                    /* add keySize to position */
                    currentPosition += keySize;
                    iter._reader.position(currentPosition);
                }

                if (postMsg.checkHasExtendedHdr())
                {
                    decodeBuffer8(iter, postMsg.extendedHeader());
                }

                if (postMsg.checkHasPartNum())
                {
                    /* decode only if actually in the header */
                    if ((headerSize - (iter._reader.position() - (startPosition + 2))) > 0)
                    {
                        postMsg.partNum(iter._reader.readUShort15rb());
                    }
                    else
                    /* not really there, unset flag */
                    {
                        int flags = postMsg.flags();
                        flags &= ~PostMsgFlags.HAS_PART_NUM;
                        postMsg.flags(flags);
                    }
                }

                if (postMsg.checkHasPostUserRights())
                {
                    /* decode only if actually in the header */
                    if ((headerSize - (iter._reader.position() - (startPosition + 2))) > 0)
                    {
                        postMsg.postUserRights(iter._reader.readUShort15rb());
                    }
                    else
                    /* not really there, unset flag */
                    {
                        int flags = postMsg.flags();
                        flags &= ~PostMsgFlags.HAS_POST_USER_RIGHTS;
                        postMsg.flags(flags);
                    }
                }
                break;
            default:
                System.out.println("Unknown msgClass: " + msg.msgClass());
                assert (false);
        }

        /* move to end of header */
        iter._reader.position(headerSize + 2 + startPosition);
        iter._curBufPos = headerSize + 2 + startPosition;
    }

    /* Decodes the RequestMsg header.
     * Returns CodecReturnCodes.SUCCESS if the message was successfully decoded.
     * 
     * iter is the iterator that decodes the message
     * requestMsg is the message to decode
     * 
     * Returns CodecReturnCodes.SUCCESS if the message was successfully decoded.
     */
    private static void decodeRequestMsgHeader(DecodeIteratorImpl iter, RequestMsg requestMsg) throws Exception
    {
        assert (iter != null && requestMsg != null);

        requestMsg.flags(iter._reader.readUShort15rb());
        // need to scale containerType
        requestMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);

        if (requestMsg.checkHasPriority())
        {
            requestMsg.priority().priorityClass(iter._reader.readUnsignedByte());
            requestMsg.priority().count(iter._reader.readUShort16ob());
        }

        // note: at present, the code that parses QOS will never return a non-success value
        decodeRequestMsgHeaderQos(iter, requestMsg);

        final int keySize = iter._reader.readUShort15rb();
        // don't iterate position by this value anymore. We want to add the keySize to position
        int currentPosition = iter._reader.position();
        decodeBaseKey(iter, requestMsg.msgKey());

        // add keySize to position
        currentPosition += keySize;
        iter._reader.position(currentPosition);

        if (requestMsg.checkHasExtendedHdr())
        {
            decodeBuffer8(iter, requestMsg.extendedHeader());
        }
    }

    /* Decodes the (optional) "QOS" and "Worst QOS" entries in an RequestMsg header.
     * Returns CodecReturnCodes.SUCCESS if the (optional) "QOS"
     * and "Worst QOS" entries in a RequestMsg header were successfully decoded.
     * 
     * iter is the iterator that decodes the message
     * requestMsg is the message to decode
     * 
     * Returns CodecReturnCodes.SUCCESS if the (optional) "QOS"
     *         and "Worst QOS" entries in an RequestMsg header were successfully decoded.
     */
    private static void decodeRequestMsgHeaderQos(DecodeIteratorImpl iter, RequestMsg requestMsg) throws Exception
    {
        assert (iter != null && requestMsg != null);

        if (requestMsg.checkHasQos())
        {
            // note: at present, the code that parses QOS will never return a non-success value
            decodeQosInMsg(iter, requestMsg.qos());
        }

        if (requestMsg.checkHasWorstQos())
        {
            // note: at present, the code that parses QOS will never return a non-success value
            decodeQosInMsg(iter, requestMsg.worstQos());
        }
    }

    /* Decodes the CloseMsg header.
     * Returns CodecReturnCodes.SUCCESS if the message was successfully decoded.
     * 
     * iter is the iterator that decodes the message
     * closeMsg is the message to decode
     * 
     * Returns CodecReturnCodes.SUCCESS if the message was successfully decoded.
     */
    private static void decodeCloseMsgHeader(DecodeIteratorImpl iter, CloseMsg closeMsg) throws Exception
    {
        assert (iter != null && closeMsg != null);

        closeMsg.flags(iter._reader.readUShort15rb());
        // need to scale containerType
        closeMsg.containerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);

        if (closeMsg.checkHasExtendedHdr())
        {
            decodeBuffer8(iter, closeMsg.extendedHeader());
        }
    }

    private static void decodeStateInMsg(DecodeIterator iterInt, State stateInt) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        StateImpl state = (StateImpl)stateInt;
        byte stateVal = 0;

        stateVal = iter._reader.readByte();

        /* only take lowest three bits */
        state.dataState(stateVal & 0x7);

        /* now shift and take top five bits */
        state.streamState(stateVal >> 3);

        state.code(iter._reader.readByte());
        decodeBuffer15(iter, state.text());
    }

    static int decodeState(DecodeIterator iterInt, State valueInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        StateImpl value = (StateImpl)valueInt;
        int retVal = CodecReturnCodes.SUCCESS;
        int stateVal;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) >= 3)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                stateVal = iter._reader.readByte();
                value.code(iter._reader.readByte());
                decodeBuffer15(iter, value.text());
                value.streamState(stateVal >> 3);
                value.dataState(stateVal & 0x7);
                iter._reader.position(savePosition);
            }
            catch (Exception e)
            {
                return CodecReturnCodes.INCOMPLETE_DATA;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            value.blank();
            retVal = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            retVal = CodecReturnCodes.INCOMPLETE_DATA;
        }
        return retVal;
    }

    static int decodeBuffer(DecodeIterator iterInt, Buffer value)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int retVal = CodecReturnCodes.SUCCESS;

        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != value : "Invalid parameters or parameters passed in as NULL";

        int len = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;

        if (len == 0)
        {
            ((BufferImpl)value).blank();
            retVal = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ((BufferImpl)value).data_internal(iter._buffer.data(), iter._curBufPos, len);
        }

        return retVal;
    }

    private static void decodeBuffer8(DecodeIteratorImpl iter, Buffer buf) throws Exception
    {
        int len = iter._reader.readUnsignedByte();

        ((BufferImpl)buf).data_internal(iter._reader.buffer(), iter._reader.position(), len);
        iter._reader.skipBytes(len);
    }

    private static void decodeBuffer15(DecodeIteratorImpl iter, Buffer buffer) throws Exception
    {
        int len = iter._reader.readUShort15rb();

        ((BufferImpl)buffer).data_internal(iter._reader.buffer(), iter._reader.position(), len);
        iter._reader.skipBytes(len);
    }

    private static void decodeBuffer16(DecodeIteratorImpl iter, Buffer buf) throws Exception
    {
        int temp = iter._reader.readUShort16ob();

        ((BufferImpl)buf).data_internal(iter._reader.buffer(), iter._reader.position(), temp);
        iter._reader.skipBytes(temp);
    }

    private static void decodeKeyEncAttrib(DecodeIterator iterInt, Buffer buffer) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int tlen = 0;

        tlen = iter._reader.readUShort15rb();
        ((BufferImpl)buffer).data_internal(iter._reader.buffer(), iter._reader.position(), tlen);
        iter._reader.skipBytes(tlen);
    }

    private static void decodeQosInMsg(DecodeIterator iterInt, Qos qosInt) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        QosImpl qos = (QosImpl)qosInt;
        byte qosValue = 0;

        qosValue = iter._reader.readByte();

        qos._timeliness = (qosValue >> 5);
        qos._rate = ((qosValue >> 1) & 0xF);
        qos._dynamic = ((qosValue & 0x1) > 0 ? true : false);

        if (qos._timeliness > QosTimeliness.DELAYED_UNKNOWN)
            qos._timeInfo = (iter._reader.readUnsignedShort());
        else
            qos._timeInfo = 0;

        if (qos._rate > QosRates.JIT_CONFLATED)
            qos._rateInfo = (iter._reader.readUnsignedShort());
        else
            qos._rateInfo = 0;
    }

    static int decodeQos(DecodeIterator iterInt, Qos valueInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        QosImpl value = (QosImpl)valueInt;
        int retVal = CodecReturnCodes.SUCCESS;
        int qosValue;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) >= 1)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                qosValue = iter._reader.readByte();
                value._timeliness = (qosValue >> 5);
                value._rate = ((qosValue >> 1) & 0xF);
                value._dynamic = ((qosValue & 0x1) > 0 ? true : false);
                if (value._timeliness > QosTimeliness.DELAYED_UNKNOWN)
                {
                    value._timeInfo = (iter._reader.readUnsignedShort());
                }
                else
                {
                    value._timeInfo = 0;
                }
                if (value._rate > QosRates.JIT_CONFLATED)
                {
                    value._rateInfo = (iter._reader.readUnsignedShort());
                }
                else
                {
                    value._rateInfo = 0;
                }
                iter._reader.position(savePosition);
            }
            catch (Exception e)
            {
                return CodecReturnCodes.INCOMPLETE_DATA;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            value.blank();
            retVal = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            retVal = CodecReturnCodes.INCOMPLETE_DATA;
        }
        return retVal;
    }

    private static void decodeBaseKey(DecodeIterator iterInt, MsgKey key) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;

        ((MsgKeyImpl)key).flags(iter._reader.readUShort15rb());

        if (key.checkHasServiceId())
            key.serviceId(iter._reader.readUShort16ob());

        if (key.checkHasName())
        {
            /* take name off wire */
            decodeBuffer8(iter, key.name());

            /* name type is only present if name is there */
            if (key.checkHasNameType())
                key.nameType(iter._reader.readUnsignedByte());
        }

        if (key.checkHasFilter())
            key.filter(iter._reader.readUnsignedInt());

        if (key.checkHasIdentifier())
            key.identifier(iter._reader.readInt());

        if (key.checkHasAttrib())
        {
            /* container type needs to be scaled back up */
            key.attribContainerType(iter._reader.readUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN);
            /* size is now an RB15 */
            if (key.attribContainerType() != DataTypes.NO_DATA)
            {
                decodeKeyEncAttrib(iter, key.encodedAttrib());
            }
            else
            {
                if (key.encodedAttrib() != null)
                {
                    key.encodedAttrib().clear();
                }
            }
        }
    }

    private static boolean canDecodeContainerType(int containerType)
    {
        switch (containerType)
        {
            case DataTypes.FIELD_LIST:
            case DataTypes.FILTER_LIST:
            case DataTypes.ELEMENT_LIST:
            case DataTypes.MAP:
            case DataTypes.VECTOR:
            case DataTypes.SERIES:
            case DataTypes.MSG:
                return true;
            default:
                return false;
        }
    }

    /* Called when decoding a container and the end of the container's entries is reached. */
    static void endOfList(DecodeIterator iterInt)
    {
        /* Go back to previous level and check its type */
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel _levelInfo;

        while (--iter._decodingLevel >= 0)
        {
            _levelInfo = iter._levelInfo[iter._decodingLevel];

            switch (_levelInfo._containerType)
            {
                case DataTypes.MSG: /* This was contained by a message */
                    break; /* Keep unwinding (in case the message was contained by another message) */

                case DataTypes.NO_DATA: /* Finished decoding a 'temporary' container. Just undo the changes. */
                    iter._curBufPos = _levelInfo._nextEntryPos;
                    --iter._decodingLevel;
                    return; /* STOP */

                default: /* Inside an RWF container */
                    return; /* STOP */
            }
        }
    }

    static int decodeMsgKeyAttrib(DecodeIterator iterInt, MsgKey key)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        /* Create a level to save the current iterator position.
         * Once the decode of the opaque is finished, the iterator will be returned to this position. */
        DecodingLevel _levelInfo = null;

        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != key : "Invalid parameters or parameters passed in as NULL";

        if (!key.checkHasAttrib())
            return CodecReturnCodes.INVALID_ARGUMENT;
        ++iter._decodingLevel;
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS - 1)
            return CodecReturnCodes.ITERATOR_OVERRUN;

        _levelInfo = iter._levelInfo[iter._decodingLevel];

        /* Set type to NO_DATA. When an entry decoder finishes and sees this,
         * they will know that this was only a 'temporary' container and reset
         * curBufPos (actual rwf container decoders set this to their appropriate type). */
        _levelInfo._containerType = DataTypes.NO_DATA;

        /* Save iterator position */
        _levelInfo._nextEntryPos = iter._curBufPos;

        /* Setup iterator to decode opaque. */
        iter._curBufPos = ((BufferImpl)key.encodedAttrib()).position();
        try
        {
            iter._reader.position(iter._curBufPos);
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
        iter._levelInfo[iter._decodingLevel + 1]._endBufPos = ((BufferImpl)key.encodedAttrib()).position() + key.encodedAttrib().length();

        /* Leave the current _levelInfo's _endBufPos where it is.
         * It's not used in the opaque decoding, and it needs to be pointing to the correct position after the reset. */

        return CodecReturnCodes.SUCCESS;
    }

    static int decodeElementList(DecodeIterator iterInt, ElementList elementListInt, LocalElementSetDefDb localSetDbInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int position;
        int _endBufPos;
        DecodingLevel _levelInfo;
        LocalElementSetDefDbImpl localSetDb = (LocalElementSetDefDbImpl)localSetDbInt;
        ElementListImpl elementList = (ElementListImpl)elementListInt;

        assert null != iter : "Invalid parameters or parameters passed in as NULL";

        _levelInfo = iter._levelInfo[++iter._decodingLevel];
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        iter.setup(_levelInfo, DataTypes.ELEMENT_LIST, elementList);

        position = iter._curBufPos;
        try
        {
            iter._reader.position(position);
            _endBufPos = _levelInfo._endBufPos;

            if (_endBufPos - position == 0)
            {
                endOfList(iter);
                return CodecReturnCodes.NO_DATA;
            }

            /* get flags */
            elementList._flags = iter._reader.readByte();

            /* get element list information */
            if (elementList.checkHasInfo())
            {
                int infoLen;
                int startpos;

                /* has 1 byte length */
                infoLen = iter._reader.readUnsignedByte();
                startpos = iter._reader.position();

                if ((startpos + infoLen) > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                elementList._elementListNum = iter._reader.readUnsignedShort();

                /* used info length to skip */
                position = startpos + infoLen;
                iter._reader.position(position);
            }

            /* get set data */
            if (elementList.checkHasSetData())
            {
                /* get set id */
                if (elementList.checkHasSetId())
                    elementList._setId = iter._reader.readUShort15rb();
                else
                    elementList._setId = 0;
                position = iter._reader.position();

                /* set definition from the local or global set list */
                if (elementList._setId <= LocalElementSetDefDbImpl.MAX_LOCAL_ID)
                {
                    if (localSetDb != null && (localSetDb._definitions[elementList._setId]._setId != LocalElementSetDefDbImpl.BLANK_ID))
                        _levelInfo._elemListSetDef = localSetDb._definitions[elementList._setId];
                    else
                        _levelInfo._elemListSetDef = null;
                }
                else
                {
                    if (iter._elementSetDefDb != null
                            && (iter._elementSetDefDb.definitions()[elementList._setId].setId() != GlobalFieldSetDefDbImpl.BLANK_ID))
                        _levelInfo._elemListSetDef = (ElementSetDefImpl)iter._elementSetDefDb.definitions()[elementList._setId];
                    else
                        _levelInfo._elemListSetDef = null;
                }

                /* check for element list data */
                if (elementList.checkHasStandardData())
                {
                    /* if hassetdata and HasFieldList then set data is length specified */
                    decodeBuffer15(iter, elementList._encodedSetData);

                    /* get element list data */
                    _levelInfo._itemCount = iter._reader.readUnsignedShort();
                    position = iter._reader.position();
                    ((BufferImpl)elementList._encodedEntries).data_internal(iter._reader.buffer(), position, (_endBufPos - position));
                    /* check for buffer overflow - post check ok since no copy */
                    if (position > _endBufPos)
                        return CodecReturnCodes.INCOMPLETE_DATA;
                }
                else
                {
                    /* get the element list set data - not length specified since no field list data exists */
                    elementList._encodedEntries.clear();

                    ((BufferImpl)elementList._encodedSetData).data_internal(iter._reader.buffer(), position, (_endBufPos - position));
                    if (position > _endBufPos)
                        return CodecReturnCodes.INCOMPLETE_DATA;
                }

                /* Setup to decode the set if able. Otherwise skip to entries. */
                if (_levelInfo._elemListSetDef != null)
                {
                    _levelInfo._setCount = _levelInfo._elemListSetDef._count;
                    _levelInfo._itemCount += _levelInfo._elemListSetDef._count;

                    _levelInfo._nextEntryPos = /* iter._curBufPos = */
                    (_levelInfo._setCount > 0) ? ((BufferImpl)elementList._encodedSetData).position() : position;

                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    _levelInfo._setCount = 0;
                    _levelInfo._nextEntryPos = /* iter._curBufPos = */position + elementList._encodedSetData.length();
                    return CodecReturnCodes.SET_SKIPPED;
                }
            }
            else if (elementList.checkHasStandardData())
            {
                /* get element list data only */

                _levelInfo._itemCount = iter._reader.readUnsignedShort();

                ((BufferImpl)elementList._encodedEntries).data_internal(iter._reader.buffer(), iter._reader.position(),
                                                                        (_endBufPos - iter._reader.position()));
                iter._curBufPos = iter._reader.position();
                _levelInfo._setCount = 0;
            }
            else
            {
                elementList._encodedSetData.clear();
                elementList._encodedEntries.clear();
                _levelInfo._itemCount = 0;
                iter._curBufPos = 0;
                _levelInfo._setCount = 0;
            }

            _levelInfo._nextEntryPos = iter._curBufPos;
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
        return CodecReturnCodes.SUCCESS;
    }

    static int decodeElementEntry(DecodeIterator iterInt, ElementEntry elementInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret;
        int position;
        ElementListImpl elementList;
        ElementEntryImpl element = (ElementEntryImpl)elementInt;

        DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

        assert null != element && null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != iter._levelInfo[iter._decodingLevel]._listType : "Invalid decoding attempted";

        elementList = (ElementListImpl)_levelInfo._listType;

        if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
        {
            endOfList(iter);
            return CodecReturnCodes.END_OF_CONTAINER;
        }

        /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
        position = iter._curBufPos = _levelInfo._nextEntryPos;

        try
        {
            iter._reader.position(position);
            if (_levelInfo._nextSetPosition < _levelInfo._setCount)
            {
                ElementSetDefEntryImpl encoding = null;
                assert null != _levelInfo._elemListSetDef : "Invalid parameters or parameters passed in as NULL";
                assert _levelInfo._elemListSetDef._count == _levelInfo._setCount : "Invalid parameters or parameters passed in as NULL";

                encoding = (ElementSetDefEntryImpl)_levelInfo._elemListSetDef._entries[_levelInfo._nextSetPosition];

                ((BufferImpl)element._name).copyReferences(encoding._name);
                element._dataType = encoding._dataType;

                /* get the set data and reset position */
                if ((ret = decodeSet(iter, encoding._dataType, element._encodedData)) != CodecReturnCodes.SUCCESS)
                    return ret;

                iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._nextEntryPos;

                /* handle legacy conversion */
                element._dataType = convertToPrimitiveType(element._dataType);

                _levelInfo._nextItemPosition++;
                _levelInfo._nextSetPosition++;

                if (_levelInfo._nextSetPosition == _levelInfo._setCount && elementList._encodedEntries.length() > 0)
                    _levelInfo._nextEntryPos = ((BufferImpl)elementList._encodedEntries).position();

                return CodecReturnCodes.SUCCESS;
            }

            /* get normal element list data */
            if (((BufferImpl)elementList._encodedEntries).position() + elementList._encodedEntries.length() - position < 3)
                return CodecReturnCodes.INCOMPLETE_DATA;

            decodeBuffer15(iter, element._name);
            element._dataType = iter._reader.readUnsignedByte();

            if (element._dataType != DataTypes.NO_DATA)
            {
                decodeBuffer16(iter, element._encodedData);

                if (iter._reader.position() > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* handle legacy conversion */
                element._dataType = convertToPrimitiveType(element._dataType);

                /* shift iterator */
                iter._curBufPos = ((BufferImpl)element._encodedData).position();
                _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = iter._reader.position();
                _levelInfo._nextItemPosition++;
                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                element._encodedData.clear();
                _levelInfo._nextItemPosition++;
                iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = iter._reader.position();
                return CodecReturnCodes.SUCCESS;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
    }

    static int decodeUInt(DecodeIterator iterInt, UInt value)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret = CodecReturnCodes.SUCCESS;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
                value.value(iter._reader.readULong64ls(size));
                iter._reader.position(savePosition);
            }
            catch (Exception e)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            ((UIntImpl)value).blank();
            ret = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ret = CodecReturnCodes.INCOMPLETE_DATA;
        }
        return ret;
    }

    static int decodeInt(DecodeIterator iterInt, Int value)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret = CodecReturnCodes.SUCCESS;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
                value.value(iter._reader.readLong64ls(size));
                iter._reader.position(savePosition);
            }
            catch (Exception e1)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            ((IntImpl)value).blank();
            ret = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ret = CodecReturnCodes.INCOMPLETE_DATA;
        }
        return ret;
    }

    static int decodeEnum(DecodeIterator iterInt, Enum value)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret = CodecReturnCodes.SUCCESS;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
                value.value(iter._reader.readUInt16ls(size));
                iter._reader.position(savePosition);
            }
            catch (Exception e1)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            ((EnumImpl)value).blank();
            ret = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ret = CodecReturnCodes.INCOMPLETE_DATA;
        }

        return ret;
    }

    static int decodeDate(DecodeIterator iterInt, Date value)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret = CodecReturnCodes.SUCCESS;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 4)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                value.day(iter._reader.readUnsignedByte());
                value.month(iter._reader.readUnsignedByte());
                value.year(iter._reader.readUnsignedShort());
                iter._reader.position(savePosition);
                if (value.isBlank())
                {
                    ret = CodecReturnCodes.BLANK_DATA;
                }
            }
            catch (Exception e)
            {
                return CodecReturnCodes.INCOMPLETE_DATA;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            value.blank();
            ret = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ret = CodecReturnCodes.INCOMPLETE_DATA;
        }

        return ret;
    }

    static int decodeTime(DecodeIterator iterInt, Time valueInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        TimeImpl value = (TimeImpl)valueInt;
        int ret = CodecReturnCodes.SUCCESS;
        int savePosition = iter._reader.position();

        switch (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos)
        {
            case 8:
                try
                {
                    int tempMicro;
                    int tempNano;
                    iter._reader.position(iter._curBufPos);
                    value._hour = (iter._reader.readUnsignedByte());
                    value._minute = (iter._reader.readUnsignedByte());
                    value._second = (iter._reader.readUnsignedByte());
                    value._millisecond = (iter._reader.readUnsignedShort());
                    tempMicro = (iter._reader.readUnsignedShort());
                    tempNano = (iter._reader.readUnsignedByte());
                    value._microsecond = (tempMicro & 0x000007FF);
                    value._nanosecond = (((tempMicro & 0x00003800) >> 3) + tempNano);

                    iter._reader.position(savePosition);
                }
                catch (Exception e)
                {
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }
                if (value.isBlank())
                {
                    ret = CodecReturnCodes.BLANK_DATA;
                }
                break;
            case 7:
                try
                {
                    iter._reader.position(iter._curBufPos);
                    value._hour = (iter._reader.readUnsignedByte());
                    value._minute = (iter._reader.readUnsignedByte());
                    value._second = (iter._reader.readUnsignedByte());
                    value._millisecond = (iter._reader.readUnsignedShort());
                    value._microsecond = (iter._reader.readUnsignedShort());
                    value._nanosecond = 0;
                    iter._reader.position(savePosition);
                }
                catch (Exception e)
                {
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }
                if ((value._hour == TimeImpl.BLANK_HOUR) &&
                        (value._minute == TimeImpl.BLANK_MINUTE) &&
                        (value._second == TimeImpl.BLANK_SECOND) &&
                        (value._millisecond == TimeImpl.BLANK_MILLI) &&
                        (value._microsecond == TimeImpl.BLANK_MICRO_NANO))
                {
                    value._nanosecond = (TimeImpl.BLANK_MICRO_NANO);
                    ret = CodecReturnCodes.BLANK_DATA;
                }
                break;
            case 5:
                try
                {
                    iter._reader.position(iter._curBufPos);
                    value._hour = (iter._reader.readUnsignedByte());
                    value._minute = (iter._reader.readUnsignedByte());
                    value._second = (iter._reader.readUnsignedByte());
                    value._millisecond = (iter._reader.readUnsignedShort());
                    value._microsecond = 0;
                    value._nanosecond = 0;
                    iter._reader.position(savePosition);
                }
                catch (Exception e)
                {
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }
                if ((value._hour == TimeImpl.BLANK_HOUR) &&
                        (value._minute == TimeImpl.BLANK_MINUTE) &&
                        (value._second == TimeImpl.BLANK_SECOND) &&
                        (value._millisecond == TimeImpl.BLANK_MILLI))
                {
                    value._microsecond = (TimeImpl.BLANK_MICRO_NANO);
                    value._nanosecond = (TimeImpl.BLANK_MICRO_NANO);
                    ret = CodecReturnCodes.BLANK_DATA;
                }
                break;
            case 3:
                try
                {
                    iter._reader.position(iter._curBufPos);
                    value.clear();
                    value._hour = (iter._reader.readUnsignedByte());
                    value._minute = (iter._reader.readUnsignedByte());
                    value._second = (iter._reader.readUnsignedByte());
                    iter._reader.position(savePosition);
                }
                catch (Exception e)
                {
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }
                if ((value._hour == TimeImpl.BLANK_HOUR) &&
                        (value._minute == TimeImpl.BLANK_MINUTE) &&
                        (value._second == TimeImpl.BLANK_SECOND))
                {
                    value._millisecond = (TimeImpl.BLANK_MILLI);
                    value._microsecond = (TimeImpl.BLANK_MICRO_NANO);
                    value._nanosecond = (TimeImpl.BLANK_MICRO_NANO);
                    ret = CodecReturnCodes.BLANK_DATA;
                }
                break;
            case 2:
                try
                {
                    iter._reader.position(iter._curBufPos);
                    value.clear();
                    value._hour = (iter._reader.readUnsignedByte());
                    value._minute = (iter._reader.readUnsignedByte());
                    iter._reader.position(savePosition);
                }
                catch (Exception e)
                {
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }
                if ((value._hour == TimeImpl.BLANK_HOUR) &&
                        (value._minute == TimeImpl.BLANK_MINUTE))
                {
                    value._second = (TimeImpl.BLANK_SECOND);
                    value._millisecond = (TimeImpl.BLANK_MILLI);
                    value._microsecond = (TimeImpl.BLANK_MICRO_NANO);
                    value._nanosecond = (TimeImpl.BLANK_MICRO_NANO);
                    ret = CodecReturnCodes.BLANK_DATA;
                }
                break;
            case 0:
                value.blank();
                ret = CodecReturnCodes.BLANK_DATA;
                break;
            default:
                ret = CodecReturnCodes.INCOMPLETE_DATA;
                break;
        }
        return ret;
    }

    private static boolean makeSecMillisBlankCheckDateBlank(DateTimeImpl value)
    {
        value.time().second(TimeImpl.BLANK_SECOND);
        value.time().millisecond(TimeImpl.BLANK_MILLI);
        value.time().microsecond(TimeImpl.BLANK_MICRO_NANO);
        value.time().nanosecond(TimeImpl.BLANK_MICRO_NANO);
        return (value.date().day() == 0) && (value.date().month() == 0) && (value.date().year() == 0);
    }

    private static int makeSecMillisZeros(DateTimeImpl value)
    {
        value.time().second(0);
        value.time().millisecond(0);
        value.time().microsecond(0);
        value.time().nanosecond(0);
        return CodecReturnCodes.SUCCESS;
    }
    
    private static boolean makeMillisBlankCheckDateBlank(DateTimeImpl value)
    {
        value.time().millisecond(TimeImpl.BLANK_MILLI);
        value.time().microsecond(TimeImpl.BLANK_MICRO_NANO);
        value.time().nanosecond(TimeImpl.BLANK_MICRO_NANO);
        return (value.date().day() == 0) && (value.date().month() == 0) && (value.date().year() == 0);
    }

    private static int makeMillisZeros(DateTimeImpl value)
    {
        value.time().millisecond(0);
        value.time().microsecond(0);
        value.time().nanosecond(0);
        return CodecReturnCodes.SUCCESS;
    }
    
    private static boolean makeMicroNanoBlankCheckDateBlank(DateTimeImpl value)
    {
        value.time().microsecond(TimeImpl.BLANK_MICRO_NANO);
        value.time().nanosecond(TimeImpl.BLANK_MICRO_NANO);
        return (value.date().day() == 0) && (value.date().month() == 0) && (value.date().year() == 0);
    }

    private static int makeMicroNanoZeros(DateTimeImpl value)
    {
        value.time().microsecond(0);
        value.time().nanosecond(0);
        return CodecReturnCodes.SUCCESS;
    }
    
    private static boolean makeNanoBlankCheckDateBlank(DateTimeImpl value)
    {
        value.time().nanosecond(TimeImpl.BLANK_MICRO_NANO);
        return (value.date().day() == 0) && (value.date().month() == 0) && (value.date().year() == 0);
    }

    private static int makeNanoZeros(DateTimeImpl value)
    {
        value.time().nanosecond(0);
        return CodecReturnCodes.SUCCESS;
    }    

    static int decodeDateTime(DecodeIterator iterInt, DateTime valueInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DateTimeImpl value = (DateTimeImpl)valueInt;
        int ret = CodecReturnCodes.SUCCESS;
        int savePosition = iter._reader.position();

        switch (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos)
        {
            case 0:
                value.blank();
                ret = CodecReturnCodes.BLANK_DATA;
                return ret;

            case 6:
                try
                {
                    iter._reader.position(iter._curBufPos);
                    value.day(iter._reader.readUnsignedByte());
                    value.month(iter._reader.readUnsignedByte());
                    value.year(iter._reader.readUnsignedShort());
                    value.time().hour(iter._reader.readUnsignedByte());
                    value.time().minute(iter._reader.readUnsignedByte());

                    /* If the time we took from wire matches blank, fill in rest of time as blank,
                     * then ensure that date portion is also blank - if so, return blank data.
                     * If time portion was not blank, just return success. */
                    ret = (((value.time().hour() == TimeImpl.BLANK_HOUR && value.time().minute() == TimeImpl.BLANK_MINUTE)) ?
                            (makeSecMillisBlankCheckDateBlank(value) ?
                                    CodecReturnCodes.BLANK_DATA : CodecReturnCodes.SUCCESS) : makeSecMillisZeros(value));

                    iter._reader.position(savePosition);

                    return ret;
                }
                catch (Exception e)
                {
                    value.clear();
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }

            case 7:

                try
                {
                    iter._reader.position(iter._curBufPos);
                    value.day(iter._reader.readUnsignedByte());
                    value.month(iter._reader.readUnsignedByte());
                    value.year(iter._reader.readUnsignedShort());
                    value.time().hour(iter._reader.readUnsignedByte());
                    value.time().minute(iter._reader.readUnsignedByte());
                    value.time().second(iter._reader.readUnsignedByte());

                    /* need this to populate rest of time properly */
                    /* If the time we took from wire matches blank, fill in rest of time as blank,
                     * then ensure that date portion is also blank - if so, return blank data. */
                    /* If time portion was not blank, just return success */
                    ret = (((value.time().hour() == TimeImpl.BLANK_HOUR && value.time().minute() ==
                            TimeImpl.BLANK_MINUTE && value.time().second() == TimeImpl.BLANK_SECOND)) ?
                                   (makeMillisBlankCheckDateBlank(value) ? CodecReturnCodes.BLANK_DATA : CodecReturnCodes.SUCCESS) : makeMillisZeros(value));

                    iter._reader.position(savePosition);

                    return ret;
                }
                catch (Exception e)
                {
                    value.clear();
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }

            case 9:
                try
                {
                    iter._reader.position(iter._curBufPos);
                    value.day(iter._reader.readUnsignedByte());
                    value.month(iter._reader.readUnsignedByte());
                    value.year(iter._reader.readUnsignedShort());
                    value.time().hour(iter._reader.readUnsignedByte());
                    value.time().minute(iter._reader.readUnsignedByte());
                    value.time().second(iter._reader.readUnsignedByte());
                    value.time().millisecond(iter._reader.readUnsignedShort());

                    ret = (((value.time().hour() == TimeImpl.BLANK_HOUR) && (value.time().minute() == TimeImpl.BLANK_MINUTE)
                            && (value.time().second() == TimeImpl.BLANK_SECOND) && (value.time().millisecond() == TimeImpl.BLANK_MILLI)) ?
                                    (makeMicroNanoBlankCheckDateBlank(value) ?
                                            CodecReturnCodes.BLANK_DATA : CodecReturnCodes.SUCCESS) : makeMicroNanoZeros(value));

                    iter._reader.position(savePosition);

                    return ret;
                }
                catch (Exception e)
                {
                    value.clear();
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }

            case 11:
                try
                {
                    iter._reader.position(iter._curBufPos);
                    value.day(iter._reader.readUnsignedByte());
                    value.month(iter._reader.readUnsignedByte());
                    value.year(iter._reader.readUnsignedShort());
                    value.time().hour(iter._reader.readUnsignedByte());
                    value.time().minute(iter._reader.readUnsignedByte());
                    value.time().second(iter._reader.readUnsignedByte());
                    value.time().millisecond(iter._reader.readUnsignedShort());
                    value.time().microsecond(iter._reader.readUnsignedShort());

                    ret = (((value.time().hour() == TimeImpl.BLANK_HOUR) && (value.time().minute() == TimeImpl.BLANK_MINUTE)
                            && (value.time().second() == TimeImpl.BLANK_SECOND) && (value.time().millisecond() == TimeImpl.BLANK_MILLI)
                            && (value.time().microsecond() == TimeImpl.BLANK_MICRO_NANO)) ?
                                    (makeNanoBlankCheckDateBlank(value) ? CodecReturnCodes.BLANK_DATA : CodecReturnCodes.SUCCESS) : makeNanoZeros(value));

                    iter._reader.position(savePosition);

                    return ret;
                }
                catch (Exception e)
                {
                    value.clear();
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }

            case 12:
                try
                {
                    int tempMicro;
                    int tempNano;
                    iter._reader.position(iter._curBufPos);
                    value.day(iter._reader.readUnsignedByte());
                    value.month(iter._reader.readUnsignedByte());
                    value.year(iter._reader.readUnsignedShort());
                    value.time().hour(iter._reader.readUnsignedByte());
                    value.time().minute(iter._reader.readUnsignedByte());
                    value.time().second(iter._reader.readUnsignedByte());
                    value.time().millisecond(iter._reader.readUnsignedShort());
                    tempMicro = (iter._reader.readUnsignedShort());
                    tempNano = (iter._reader.readUnsignedByte());
                    value.time().microsecond(tempMicro & 0x000007FF);
                    value.time().nanosecond(((tempMicro & 0x00003800) >> 3) + tempNano);

                    iter._reader.position(savePosition);

                    if (value.isBlank())
                        return CodecReturnCodes.BLANK_DATA;
                    else
                        return ret; // set to success above

                }
                catch (Exception e)
                {
                    value.clear();
                    return CodecReturnCodes.INCOMPLETE_DATA;
                }

            default:
                return CodecReturnCodes.INCOMPLETE_DATA;
        }
    }

    static int decodeReal(DecodeIterator iterInt, Real valueInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        RealImpl value = (RealImpl)valueInt;
        int ret = CodecReturnCodes.SUCCESS;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 1)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                int hint = iter._reader.readByte();
                switch (hint & 0x3F)
                {
                    case RealImpl.BLANK_REAL:
                        value.blank();
                        ret = CodecReturnCodes.BLANK_DATA;
                        break;
                    case RealHints.INFINITY:
                    case RealHints.NEG_INFINITY:
                    case RealHints.NOT_A_NUMBER:
                        value.value(0, (hint & 0x3F));
                        ret = CodecReturnCodes.SUCCESS;
                        break;
                    default:
                        int length = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos - 1;
                        value.value(iter._reader.readLong64ls(length), ((byte)(hint & 0x1F)));
                }
                iter._reader.position(savePosition);
            }
            catch (Exception e1)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 1)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                int hint = iter._reader.readByte();
                switch (hint & 0x3F)
                {
                    case RealHints.INFINITY:
                    case RealHints.NEG_INFINITY:
                    case RealHints.NOT_A_NUMBER:
                        value.value(0, (hint & 0x3F));
                        ret = CodecReturnCodes.SUCCESS;
                        break;
                    default:
                        value.blank();
                        ret = CodecReturnCodes.BLANK_DATA;
                }
                iter._reader.position(savePosition);
            }
            catch (Exception e1)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }

        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            value.blank();
            ret = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ret = CodecReturnCodes.INCOMPLETE_DATA;
        }

        return ret;
    }

    static int decodeMap(DecodeIterator iterInt, Map mapInt)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != mapInt : "Invalid MapInt in as NULL";
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        MapImpl map = (MapImpl)mapInt;
        int position;
        int _endBufPos;
        DecodingLevel _levelInfo;

        assert null != iter._buffer : "";

        _levelInfo = iter._levelInfo[++iter._decodingLevel];
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        iter.setup(_levelInfo, DataTypes.MAP, map);

        _endBufPos = _levelInfo._endBufPos;
        position = iter._curBufPos;
        try
        {
            iter._reader.position(position);

            if (_endBufPos - position == 0)
            {
                endOfList(iter);
                return CodecReturnCodes.NO_DATA;
            }
            else if (_endBufPos - position < 5)
                return CodecReturnCodes.INCOMPLETE_DATA;

            /* extract flags, keyDataFormat and _containerType */
            map._flags = (iter._reader.readByte());
            map._keyPrimitiveType = (iter._reader.readUnsignedByte());
            map._containerType = (iter._reader.readUnsignedByte());
            /* container type needs to be scaled back */
            map._containerType += DataTypes.CONTAINER_TYPE_MIN;

            /* Handle legacy conversion */
            map._keyPrimitiveType = convertToPrimitiveType(map._keyPrimitiveType);

            if (map.checkHasKeyFieldId())
            {
                map._keyFieldId = (iter._reader.readShort());
                position = iter._reader.position();

                if (position > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
            }

            if (map.checkHasSetDefs())
            {
                decodeBuffer15(iter, map._encodedSetDefs);
                position = iter._reader.position();

                /* Check for buffer overflow. Post check ok since no copy */
                if (position > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
            }
            else
            {
                map._encodedSetDefs.clear();
            }

            if (map.checkHasSummaryData())
            {
                decodeBuffer15(iter, map._encodedSummaryData);
                position = iter._reader.position();

                /* Check for buffer overflow. Post check ok since no copy */
                if (position > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
            }
            else
            {
                map._encodedSummaryData.clear();
            }

            /* extract total count hint */
            if (map.checkHasTotalCountHint())
                map._totalCountHint = (iter._reader.readUInt30rb());

            _levelInfo._itemCount = iter._reader.readUnsignedShort();

            position = iter._reader.position();

            ((BufferImpl)map._encodedEntries).data_internal(iter._reader.buffer(), position, (_endBufPos - position));

            /* Check for buffer overflow. Post check ok since no copy */
            if (position > _endBufPos)
                return CodecReturnCodes.INCOMPLETE_DATA;
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        _levelInfo._nextEntryPos = position; /* set entry ptr to first entry */
        iter._curBufPos = map.checkHasSummaryData() ? ((BufferImpl)map._encodedSummaryData).position() : position;
        return CodecReturnCodes.SUCCESS;
    }

    static int decodeMapEntry(DecodeIterator iterInt, MapEntry mapEntryInt, Object keyData)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        MapEntryImpl mapEntry = (MapEntryImpl)mapEntryInt;
        int position;
        MapImpl map;
        int flags;
        int ret;
        DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

        assert null != mapEntry && null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != iter._levelInfo[iter._decodingLevel]._listType : "Invalid decoding attempted";

        map = (MapImpl)_levelInfo._listType;

        if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
        {
            endOfList(iter);
            return CodecReturnCodes.END_OF_CONTAINER;
        }

        /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
        position = iter._curBufPos = _levelInfo._nextEntryPos;
        try
        {
            iter._reader.position(position);

            if ((position + 2) > _levelInfo._endBufPos)
                return CodecReturnCodes.INCOMPLETE_DATA;

            /* take out action/flags */
            flags = iter._reader.readByte();

            mapEntry._action = ((flags & 0xF));
            mapEntry._flags = (flags >> 4);

            /* get perm data */
            if ((map.checkHasPerEntryPermData()) && (mapEntry.checkHasPermData()))
            {
                decodeBuffer15(iter, mapEntry._permData);
                position = iter._reader.position();
                if (position > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
            }
            else
            {
                mapEntry._permData.clear();
            }

            decodeBuffer15(iter, mapEntry._encodedKey);
            position = iter._reader.position();

            /* User provided storage for decoded key, so decode it for them */
            if (keyData != null)
            {
                iter._levelInfo[iter._decodingLevel]._nextEntryPos = iter._curBufPos = mapEntry._encodedKey.length() > 0 ?
                        ((BufferImpl)mapEntry._encodedKey).position() : position;
                iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                if ((ret = decodePrimitiveType(iter, map._keyPrimitiveType, keyData)) < 0)
                    return ret;
            }

            if (position > _levelInfo._endBufPos)
                return CodecReturnCodes.INCOMPLETE_DATA;

            /* parse MapEntry value */
            if ((mapEntry._action == MapEntryActions.DELETE) || (map._containerType == DataTypes.NO_DATA))
            {
                mapEntry._encodedData.clear();
                _levelInfo._nextItemPosition++;
                iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                /* only have data if action is not delete */
                decodeBuffer16(iter, mapEntry._encodedData);
                position = iter._reader.position();
                if (position > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* shift iterator */
                _levelInfo._nextItemPosition++;
                iter._curBufPos = ((BufferImpl)mapEntry._encodedData).position();
                _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                return CodecReturnCodes.SUCCESS;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
    }

    private static int decodePrimitiveType(DecodeIteratorImpl iter, int type, Object data) throws Exception
    {
        int retVal = CodecReturnCodes.SUCCESS;

        switch (type)
        {
            case DataTypes.UINT:
                decodeUInt(iter, (UInt)data);
                break;
            case DataTypes.INT:
                decodeInt(iter, (Int)data);
                break;
            case DataTypes.FLOAT:
                decodeFloat(iter, (Float)data);
                break;
            case DataTypes.DOUBLE:
                decodeDouble(iter, (Double)data);
                break;
            case DataTypes.REAL:
                decodeReal(iter, (Real)data);
                break;
            case DataTypes.DATE:
                decodeDate(iter, (Date)data);
                break;
            case DataTypes.TIME:
                decodeTime(iter, (Time)data);
                break;
            case DataTypes.DATETIME:
                decodeDateTime(iter, (DateTime)data);
                break;
            case DataTypes.QOS:
                decodeQos(iter, (Qos)data);
                break;
            case DataTypes.STATE:
                decodeState(iter, (State)data);
                break;
            case DataTypes.ENUM:
                decodeEnum(iter, (Enum)data);
                break;
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos,
                                                 (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos));
                break;
            default:
                retVal = CodecReturnCodes.UNSUPPORTED_DATA_TYPE;
                break;
        }

        return retVal;
    }

    private static int decodeSet(DecodeIterator iterInt, int type, Object data) throws Exception
    {
        switch (type)
        {
            case DataTypes.INT_1:
            case DataTypes.UINT_1:
                dec8(iterInt, (Buffer)data);
                break;
            case DataTypes.INT_2:
            case DataTypes.UINT_2:
                dec16(iterInt, (Buffer)data);
                break;
            case DataTypes.TIME_3:
                dec24(iterInt, (Buffer)data);
                break;
            case DataTypes.INT_4:
            case DataTypes.UINT_4:
            case DataTypes.FLOAT_4:
            case DataTypes.DATE_4:
                dec32(iterInt, (Buffer)data);
                break;
            case DataTypes.TIME_5:
                dec40(iterInt, (Buffer)data);
                break;
            case DataTypes.DATETIME_7:
            case DataTypes.TIME_7:
                dec56(iterInt, (Buffer)data);
                break;
            case DataTypes.INT_8:
            case DataTypes.UINT_8:
            case DataTypes.DOUBLE_8:
            case DataTypes.TIME_8:
                dec64(iterInt, (Buffer)data);
                break;
            case DataTypes.DATETIME_9:
                dec72(iterInt, (Buffer)data);
                break;
            case DataTypes.DATETIME_11:
                dec88(iterInt, (Buffer)data);
                break;
            case DataTypes.DATETIME_12:
                dec96(iterInt, (Buffer)data);
                break;
            case DataTypes.ENUM:
            case DataTypes.ARRAY:
                decBuf16(iterInt, (Buffer)data);
                break;
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
            case DataTypes.OPAQUE:
            case DataTypes.XML:
            case DataTypes.FIELD_LIST:
            case DataTypes.ELEMENT_LIST:
            case DataTypes.ANSI_PAGE:
            case DataTypes.FILTER_LIST:
            case DataTypes.VECTOR:
            case DataTypes.MAP:
            case DataTypes.SERIES:
            case DataTypes.MSG:
                decBuf16(iterInt, (Buffer)data);
                break;
            case DataTypes.INT:
            case DataTypes.UINT:
            case DataTypes.FLOAT:
            case DataTypes.DOUBLE:
            case DataTypes.REAL:
            case DataTypes.DATETIME:
            case DataTypes.TIME:
            case DataTypes.DATE:
            case DataTypes.QOS:
                decBuf8(iterInt, (Buffer)data);
                break;
            case DataTypes.REAL_4RB:
                decReal_4rb(iterInt, (Buffer)data);
                break;
            case DataTypes.REAL_8RB:
                decReal_8rb(iterInt, (Buffer)data);
                break;
            default:
                return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;
        }
        return CodecReturnCodes.SUCCESS;
    }

    static int decodeFilterList(DecodeIterator iterInt, FilterList filterListInt)
    {
        assert null != filterListInt : "Invalid filterListInt passed in as NULL";
        assert null != iterInt : "Invalid iterInt passed in as NULL";
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        FilterListImpl filterList = (FilterListImpl)filterListInt;
        int position;
        int _endBufPos;
        int count;
        DecodingLevel _levelInfo;

        _levelInfo = iter._levelInfo[++iter._decodingLevel];
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        iter.setup(_levelInfo, DataTypes.FILTER_LIST, filterList);

        _endBufPos = _levelInfo._endBufPos;
        position = iter._curBufPos;
        try
        {
            iter._reader.position(position);

            if (_endBufPos - position == 0)
            {
                endOfList(iter);
                return CodecReturnCodes.NO_DATA;
            }
            else if (_endBufPos - position < 3)
                return CodecReturnCodes.INCOMPLETE_DATA;

            filterList._flags = (iter._reader.readByte());
            filterList._containerType = (iter._reader.readUnsignedByte());
            /* needs to be scaled back after decoding */
            filterList._containerType += DataTypes.CONTAINER_TYPE_MIN;

            if (filterList.checkHasTotalCountHint())
            {
                filterList._totalCountHint = iter._reader.readUnsignedByte();
            }
            else
                filterList._totalCountHint = 0;

            count = iter._reader.readUnsignedByte();
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        _levelInfo._itemCount = count;

        position = iter._reader.position();

        ((BufferImpl)filterList._encodedEntries).data_internal(iter._reader.buffer(), position, (_endBufPos - position));

        /* Check for buffer overflow. Post check ok since no copy */
        if (position > _endBufPos)
            return CodecReturnCodes.INCOMPLETE_DATA;

        iter._curBufPos = _levelInfo._nextEntryPos = position;

        return CodecReturnCodes.SUCCESS;
    }

    static int decodeFilterEntry(DecodeIterator iterInt, FilterEntry filterEntryInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        FilterEntryImpl filterEntry = (FilterEntryImpl)filterEntryInt;
        int position;
        FilterListImpl filterList;
        int flags;
        DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

        assert null != filterEntry && null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != iter._levelInfo[iter._decodingLevel]._listType : "Invalid decoding attempted";

        filterList = (FilterListImpl)_levelInfo._listType;

        if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
        {
            endOfList(iter);
            return CodecReturnCodes.END_OF_CONTAINER;
        }

        /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
        position = iter._curBufPos = _levelInfo._nextEntryPos;
        try
        {
            iter._reader.position(position);

            if ((position + 2) > _levelInfo._endBufPos)
                return CodecReturnCodes.INCOMPLETE_DATA;

            /* take out flags */
            flags = iter._reader.readByte();

            filterEntry._action = (flags & 0xF);
            filterEntry._flags = (flags >> 4);

            /* parse FilterEntry */
            filterEntry._id = (iter._reader.readUnsignedByte());
            position = iter._reader.position();

            if (filterEntry.checkHasContainerType())
            {
                if ((position + 1) > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
                filterEntry._containerType = (iter._reader.readUnsignedByte());
                /* needs to be scaled back after decoding */
                filterEntry._containerType += DataTypes.CONTAINER_TYPE_MIN;
            }
            else
                filterEntry._containerType = (filterList._containerType);

            if (filterList.checkHasPerEntryPermData() && filterEntry.checkHasPermData())
            {
                decodeBuffer15(iter, filterEntry._permData);
                position = iter._reader.position();

                if (position > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
            }
            else
            {
                filterEntry._permData.clear();
            }

            if ((filterEntry._containerType != DataTypes.NO_DATA) && (filterEntry._action != FilterEntryActions.CLEAR))
            {
                decodeBuffer16(iter, filterEntry._encodedData);
                position = iter._reader.position();
                if (position > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* shift iterator */
                _levelInfo._nextItemPosition++;
                iter._curBufPos = ((BufferImpl)filterEntry._encodedData).position();
                _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                position = iter._reader.position();
                filterEntry._encodedData.clear();
                _levelInfo._nextItemPosition++;
                iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                return CodecReturnCodes.SUCCESS;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
    }

    static int decodeArray(DecodeIterator iterInt, Array arrayInt)
    {
        ArrayImpl array = (ArrayImpl)arrayInt;
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel _levelInfo;
        int _endBufPos;

        assert null != array && null != iter : "Invalid parameters or parameters passed in as NULL";

        _levelInfo = iter._levelInfo[++iter._decodingLevel];
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        iter.setup(_levelInfo, DataTypes.ARRAY, array);

        _endBufPos = _levelInfo._endBufPos;
        try
        {
            iter._reader.position(iter._curBufPos);

            if (iter._curBufPos == _endBufPos)
            {
                endOfList(iter);
                array.blank();
                return CodecReturnCodes.BLANK_DATA;
            }
            else if (_endBufPos - iter._curBufPos < 3)
                return CodecReturnCodes.INCOMPLETE_DATA;

            /* extract data type */
            array._primitiveType = iter._reader.readUnsignedByte();
            /* extract itemLength */
            array._itemLength = iter._reader.readUShort16ob();
            /* extract count */
            _levelInfo._itemCount = iter._reader.readUnsignedShort();
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        iter._curBufPos = iter._reader.position();

        _levelInfo._nextEntryPos = iter._curBufPos;
        ((BufferImpl)array._encodedData).data_internal(iter._reader.buffer(), iter._curBufPos,
                                                       (_endBufPos - iter._curBufPos));

        /* handle legacy types */
        array._primitiveType = convertToPrimitiveType(array._primitiveType);

        /* Check for buffer overflow. Post check ok since no copy */
        if (iter._curBufPos > _endBufPos)
            return CodecReturnCodes.INCOMPLETE_DATA;
        return CodecReturnCodes.SUCCESS;
    }

    static int decodeArrayEntry(DecodeIterator iterInt, ArrayEntry arrayEntry)
    {
        assert null != arrayEntry : "Invalid arrayEntry passed in as NULL";
        assert null != iterInt : "Invalid iterInt passed in as NULL";

        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        ArrayImpl array;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];

        iter._curBufPos = levelInfo._nextEntryPos;
        try
        {
            iter._reader.position(iter._curBufPos);
            /* For an array, since the entries are always primitives(not containers),
             * no skip logic(_nextEntryPos) needs to be applied. */

            array = (ArrayImpl)levelInfo._listType;

            if (levelInfo._nextItemPosition >= levelInfo._itemCount)
            {
                endOfList(iter);
                return CodecReturnCodes.END_OF_CONTAINER;
            }

            int len = array._itemLength;
            if (len == 0)
                len = decodePrimitiveLength(iter, array._primitiveType);

            ((BufferImpl)arrayEntry.encodedData()).data_internal(iter._reader.buffer(), iter._curBufPos, len);
            levelInfo._nextEntryPos = iter._curBufPos + len;

        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        if (levelInfo._nextEntryPos > levelInfo._endBufPos)
            return CodecReturnCodes.INCOMPLETE_DATA;

        iter._levelInfo[iter._decodingLevel + 1]._endBufPos = levelInfo._nextEntryPos;

        /* shift iterator */
        levelInfo._nextItemPosition++;

        return CodecReturnCodes.SUCCESS;
    }

    private static int decodePrimitiveLength(DecodeIteratorImpl iter, int type) throws Exception
    {
        int len = lens[type];
        switch (len)
        {
            case 0:
                // variable len
                len = iter._reader.readUShort16ob();
                iter._curBufPos = iter._reader.position();
                break;
            case -1:
                // REAL_4RB
                len = ((iter._reader.readByte() & 0xC0) >> 6) + 1;
                iter._reader.position(iter._curBufPos); // rewind one byte
                break;
            case -2:
                // REAL_8RB
                // shift by 5 is intentional. Each combo represents 2 bytes here
                // so to shift by 5 instead of 6 accounts for this.
                len = ((iter._reader.readByte() & 0xC0) >> 5) + 2;
                iter._reader.position(iter._curBufPos); // rewind one byte
                break;
            default:
                // len is set to the value in the len array
        }

        return len;
    }

    static int decodeFieldList(DecodeIterator iterInt, FieldList fieldListInt, LocalFieldSetDefDb localSetDbInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        LocalFieldSetDefDbImpl localSetDb = (LocalFieldSetDefDbImpl)localSetDbInt;
        int position;
        int _endBufPos;
        DecodingLevel _levelInfo;
        FieldListImpl fieldList = (FieldListImpl)fieldListInt;

        assert null != iter : "Invalid parameters or parameters passed in as NULL";

        _levelInfo = iter._levelInfo[++iter._decodingLevel];
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        iter.setup(_levelInfo, DataTypes.FIELD_LIST, fieldList);

        _endBufPos = _levelInfo._endBufPos;
        position = iter._curBufPos;
        try
        {
            iter._reader.position(position);

            if (_endBufPos - position == 0)
            {
                endOfList(iter);
                return CodecReturnCodes.NO_DATA;
            }

            /* Get Flags */
            fieldList._flags = iter._reader.readByte();

            /* Get the Field List Information */
            if (fieldList.checkHasInfo())
            {
                int infoLen;
                int startpos;

                /* Has 1 byte length */
                infoLen = iter._reader.readUnsignedByte();
                startpos = iter._reader.position();

                fieldList._dictionaryId = iter._reader.readUShort15rb();
                fieldList._fieldListNum = iter._reader.readUnsignedShort();

                if ((startpos + infoLen) > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* Used info Length to skip */
                position = startpos + infoLen;
                iter._reader.position(position);
            }

            /* Get the Field List Set Data */
            if (fieldList.checkHasSetData())
            {
                /* Get the set identifier */
                if (fieldList.checkHasSetId())
                    fieldList._setId = iter._reader.readUShort15rb();
                else
                    fieldList._setId = 0;
                position = iter._reader.position();

                /* Set the set definition from the local or global set list */
                if (localSetDb != null && fieldList._setId <= localSetDb.maxLocalId)
                {
                    if (localSetDb != null && (localSetDb._definitions[fieldList._setId]._setId != LocalFieldSetDefDbImpl.BLANK_ID))
                        _levelInfo._fieldListSetDef = localSetDb._definitions[fieldList._setId];
                    else
                        _levelInfo._fieldListSetDef = null;
                }
                else
                {
                    if (iter._fieldSetDefDb != null
                            && (iter._fieldSetDefDb.definitions()[fieldList._setId].setId() != GlobalFieldSetDefDbImpl.BLANK_ID))
                        _levelInfo._fieldListSetDef = (FieldSetDefImpl)iter._fieldSetDefDb.definitions()[fieldList._setId];
                    else
                        _levelInfo._fieldListSetDef = null;
                }

                /* Check for field list data */
                if (fieldList.checkHasStandardData())
                {
                    /* If HasSetData and HasFieldList, then set data is length specified. */
                    decodeBuffer15(iter, fieldList._encodedSetData);

                    /* Get the Field List Data */
                    _levelInfo._itemCount = iter._reader.readUnsignedShort();
                    position = iter._reader.position();
                    ((BufferImpl)fieldList._encodedEntries).data_internal(iter._reader.buffer(),
                                                                          position, (_endBufPos - position));

                    /* check for buffer overflow - post check ok since no copy */
                    if (position > _endBufPos)
                        return CodecReturnCodes.INCOMPLETE_DATA;
                }
                else
                {
                    /* Get the field list set data. Not length specified since no field list data exists. */
                    fieldList._encodedEntries.clear();

                    ((BufferImpl)fieldList._encodedSetData).data_internal(iter._reader.buffer(),
                                                                          position, (_endBufPos - position));

                    if (position > _endBufPos)
                        return CodecReturnCodes.INCOMPLETE_DATA;
                }

                /* Setup to decode the set if able. Otherwise skip to entries. */
                if (_levelInfo._fieldListSetDef != null)
                {
                    _levelInfo._setCount = _levelInfo._fieldListSetDef._count;
                    _levelInfo._itemCount += _levelInfo._fieldListSetDef._count;

                    _levelInfo._nextEntryPos = /* iter._curBufPos = */
                    (_levelInfo._setCount > 0) ? ((BufferImpl)fieldList._encodedSetData).position() : position;

                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    _levelInfo._setCount = 0;
                    _levelInfo._nextEntryPos = /* iter._curBufPos = */position + fieldList._encodedSetData.length();
                    return CodecReturnCodes.SET_SKIPPED;
                }
            }
            else if (fieldList.checkHasStandardData())
            {
                /* Get the field list data only */
                fieldList._encodedSetData.clear();

                _levelInfo._itemCount = iter._reader.readUnsignedShort();
                position = iter._reader.position();
                ((BufferImpl)fieldList._encodedEntries).data_internal(iter._reader.buffer(),
                                                                      position, (_endBufPos - position));

                iter._curBufPos = _levelInfo._nextEntryPos = position;
                _levelInfo._setCount = 0;
            }
            else
            {
                fieldList._encodedSetData.clear();
                fieldList._encodedEntries.clear();
                _levelInfo._itemCount = 0;
                iter._curBufPos = 0;
                _levelInfo._setCount = 0;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
        return CodecReturnCodes.SUCCESS;
    }

    static int decodeFieldEntry(DecodeIterator iterInt, FieldEntry fieldInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        FieldEntryImpl field = (FieldEntryImpl)fieldInt;
        int ret;
        int position;
        FieldListImpl fieldList;
        DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

        assert null != field && null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != _levelInfo._listType : "Invalid decoding attempted";

        fieldList = (FieldListImpl)_levelInfo._listType;

        if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
        {
            endOfList(iter);
            return CodecReturnCodes.END_OF_CONTAINER;
        }

        /* Make sure we skip to the next entry if we didn't decode the previous entry payload */
        position = iter._curBufPos = _levelInfo._nextEntryPos;
        if (position < iter._reader.buffer().limit()) // Make sure we are reading only if we can read something at all
            try
            {
                iter._reader.position(position);

                if (_levelInfo._nextSetPosition < _levelInfo._setCount)
                {
                    FieldSetDefEntryImpl encoding = null;

                    assert null != _levelInfo._fieldListSetDef : "Invalid parameters or parameters passed in as NULL";
                    assert _levelInfo._fieldListSetDef.count() == _levelInfo._setCount : "Invalid data";

                    encoding = (FieldSetDefEntryImpl)(_levelInfo._fieldListSetDef._entries[_levelInfo._nextSetPosition]);

                    field._fieldId = encoding._fieldId;
                    field._dataType = encoding._dataType;

                    /* Get the set data and reset position */
                    if ((ret = decodeSet(iter, encoding._dataType, field._encodedData)) != CodecReturnCodes.SUCCESS)
                        return (ret);

                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._nextEntryPos;

                    /* handle legacy conversion */
                    int dataType = convertToPrimitiveType(field._dataType);
                    field._dataType = dataType;

                    _levelInfo._nextItemPosition++;
                    _levelInfo._nextSetPosition++;

                    if (_levelInfo._nextSetPosition == _levelInfo._setCount && fieldList._encodedEntries.length() > 0)
                        _levelInfo._nextEntryPos = ((BufferImpl)fieldList._encodedEntries).position();

                    return CodecReturnCodes.SUCCESS;
                }

                /* Get normal field list data */
                if (((BufferImpl)fieldList._encodedEntries).position() + fieldList._encodedEntries.length() - position < 3)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                field._fieldId = iter._reader.readShort();
                field._dataType = DataTypes.UNKNOWN;

                /* parse Field */
                decodeBuffer16(iter, field._encodedData);
            }
            catch (Exception e)
            {
                return CodecReturnCodes.INCOMPLETE_DATA;
            }
        position = iter._reader.position();
        if (position > _levelInfo._endBufPos)
            return CodecReturnCodes.INCOMPLETE_DATA;

        /* shift iterator */
        iter._curBufPos = ((BufferImpl)field._encodedData).position();
        _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
        _levelInfo._nextItemPosition++;
        return CodecReturnCodes.SUCCESS;
    }

    static int decodeSeries(DecodeIterator iterInt, Series seriesInt)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != seriesInt : "Invalid seriesInt in as NULL";
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        SeriesImpl series = (SeriesImpl)seriesInt;
        int position;
        int _endBufPos;
        DecodingLevel _levelInfo;

        _levelInfo = iter._levelInfo[++iter._decodingLevel];
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        iter.setup(_levelInfo, DataTypes.SERIES, series);

        _endBufPos = _levelInfo._endBufPos;
        position = iter._curBufPos;
        try
        {
            iter._reader.position(position);

            if (_endBufPos - position == 0)
            {
                endOfList(iter);
                return CodecReturnCodes.NO_DATA;
            }
            else if (_endBufPos - position < 4)
                return CodecReturnCodes.INCOMPLETE_DATA;

            _levelInfo._endBufPos = _endBufPos;

            /* extract flags, data format */
            series._flags = (iter._reader.readByte());
            series._containerType = (iter._reader.readUnsignedByte());
            /* container type needs to be scaled back after decoding */
            series._containerType += DataTypes.CONTAINER_TYPE_MIN;

            if (series.checkHasSetDefs())
            {
                decodeBuffer15(iter, series._encodedSetDefs);
                position = iter._reader.position();
                /* check for buffer overflow. Post check ok since no copy */
                if (position > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
            }
            else
            {
                series._encodedSetDefs.clear();
            }

            if (series.checkHasSummaryData())
            {
                decodeBuffer15(iter, series._encodedSummaryData);
                position = iter._reader.position();

                /* check for buffer overflow. Post check ok since no copy */
                if (position > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
            }
            else
            {
                series._encodedSummaryData.clear();
            }

            /* extract total count hints */
            if (series.checkHasTotalCountHint())
                series._totalCountHint = (iter._reader.readUInt30rb());
            else
                series._totalCountHint = 0;

            _levelInfo._itemCount = iter._reader.readUnsignedShort();
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        position = iter._reader.position();

        ((BufferImpl)series._encodedEntries).data_internal(iter._reader.buffer(), position, (_endBufPos - position));

        /* check for overflow */
        if (position > _endBufPos)
            return CodecReturnCodes.INCOMPLETE_DATA;

        _levelInfo._nextEntryPos = position; /* set entry ptr to first entry */
        iter._curBufPos = series.checkHasSummaryData() ? ((BufferImpl)series._encodedSummaryData).position() : position;

        return CodecReturnCodes.SUCCESS;
    }

    static int decodeLocalElementSetDefDb(DecodeIterator iterInt, LocalElementSetDefDb localSetDbInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int position;
        int _endBufPos;
        @SuppressWarnings("unused")
        int flags;
        int _setCount;
        int curEntryCount = 0;
        LocalElementSetDefDbImpl localSetDb = (LocalElementSetDefDbImpl)localSetDbInt;

        assert null != iter && null != localSetDb : "Invalid parameters or parameters passed in as NULL";

        try
        {
            if (iter._decodingLevel >= 0)
            {
                /* Get the encodedSetDefs pointer out of the current container. */
                Buffer encodedSetDefs;

                assert null != iter._levelInfo[iter._decodingLevel]._listType : "Invalid decoding attempted";

                switch (iter._levelInfo[iter._decodingLevel]._containerType)
                {
                    case DataTypes.MAP:
                        encodedSetDefs = ((MapImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.SERIES:
                        encodedSetDefs = ((SeriesImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.VECTOR:
                        encodedSetDefs = ((VectorImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    default:
                        return CodecReturnCodes.INVALID_ARGUMENT;
                }

                if (encodedSetDefs.length() == 0)
                    return CodecReturnCodes.INVALID_DATA;

                position = ((BufferImpl)encodedSetDefs).position();
                iter._reader.position(position);
                _endBufPos = ((BufferImpl)encodedSetDefs).position() + encodedSetDefs.length();
            }
            else
            {
                /* Separate iterator. */
                _endBufPos = iter._levelInfo[0]._endBufPos;
                position = iter._curBufPos;
            }

            if ((_endBufPos - position) < 2)
                return CodecReturnCodes.INCOMPLETE_DATA;

            flags = iter._reader.readByte();
            _setCount = iter._reader.readUnsignedByte();
            position = iter._reader.position();

            /* System.out.println("ElementSetDefDb: flags " + flags + " count " + _setCount); */

            if (_setCount == 0)
                return CodecReturnCodes.SET_DEF_DB_EMPTY;

            if (_setCount > LocalElementSetDefDbImpl.MAX_LOCAL_ID)
                return CodecReturnCodes.TOO_MANY_LOCAL_SET_DEFS;

            for (int i = 0; i <= LocalElementSetDefDbImpl.MAX_LOCAL_ID; i++)
                localSetDb._definitions[i]._setId = LocalElementSetDefDbImpl.BLANK_ID;

            for (int i = 0; i < _setCount; i++)
            {
                ElementSetDefImpl curSetDef;
                ElementSetDefEntryImpl curEntry;
                int setId;
                int encCount;

                if ((position + 2) > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* get the setId and the number of element encodings */
                setId = iter._reader.readUShort15rb();
                encCount = iter._reader.readUnsignedByte();

                /* System.out.println("  ElementSetDef: setId " + setId + " count " + encCount); */

                /* sanity checks */
                if (setId > LocalElementSetDefDbImpl.MAX_LOCAL_ID)
                    return CodecReturnCodes.ILLEGAL_LOCAL_SET_DEF;
                if (localSetDb._definitions[setId]._setId != LocalElementSetDefDbImpl.BLANK_ID)
                    return CodecReturnCodes.DUPLICATE_LOCAL_SET_DEFS;

                /* get memory for new set definition from working memory. */
                curSetDef = localSetDb._definitions[setId];

                /* make sure we have space in our entry pool */
                if (curEntryCount > localSetDb._entries.length)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                /* setup set def and put in database */
                curSetDef._setId = setId;
                curSetDef._count = encCount;
                curSetDef._entries = localSetDb._entries[curEntryCount]; /* Point to the entries from the pool that will be used for this def. */

                /* populate the entries */
                for (int j = 0; j < encCount; j++)
                {
                    /* make sure we have space in our entry pool */
                    if (encCount > localSetDb._entries[curEntryCount].length)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    curEntry = localSetDb._entries[curEntryCount][j];
                    decodeBuffer15(iter, curEntry.name());
                    curEntry._dataType = iter._reader.readUnsignedByte();
                    position = iter._reader.position();
                }
                ++curEntryCount;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        return (position <= _endBufPos) ? CodecReturnCodes.SUCCESS : CodecReturnCodes.INCOMPLETE_DATA;
    }

    static int decodeLocalFieldSetDefDb(DecodeIterator iterInt, LocalFieldSetDefDb localSetDbInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        LocalFieldSetDefDbImpl localSetDb = (LocalFieldSetDefDbImpl)localSetDbInt;
        int position;
        int _endBufPos;
        int _setCount;
        @SuppressWarnings("unused")
        int flags;

        int curEntryCount = 0;

        assert null != iter && null != localSetDb : "Invalid parameters or parameters passed in as NULL";
        try
        {
            if (iter._decodingLevel >= 0)
            {
                /* Get the encodedSetDefs pointer out of the current container. */
                Buffer encodedSetDefs;

                assert null != iter && null != localSetDb : "Invalid parameters or parameters passed in as NULL";

                switch (iter._levelInfo[iter._decodingLevel]._containerType)
                {
                    case DataTypes.MAP:
                        encodedSetDefs = ((MapImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.SERIES:
                        encodedSetDefs = ((SeriesImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.VECTOR:
                        encodedSetDefs = ((VectorImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    default:
                        return CodecReturnCodes.INVALID_ARGUMENT;
                }

                if (encodedSetDefs.length() == 0)
                    return CodecReturnCodes.INVALID_DATA;

                position = ((BufferImpl)encodedSetDefs).position();
                iter._reader.position(position);
                _endBufPos = ((BufferImpl)encodedSetDefs).position() + encodedSetDefs.length();
            }
            else
            {
                /* Separate iterator. */
                _endBufPos = iter._levelInfo[0]._endBufPos;
                position = iter._curBufPos;
            }

            if ((_endBufPos - position) < 2)
                return CodecReturnCodes.INCOMPLETE_DATA;

            flags = iter._reader.readByte();
            _setCount = iter._reader.readUnsignedByte();
            position = iter._reader.position();

            /* System.out.println("FieldSetDefDb: flags " + flags + " count " + _setCount); */

            if (_setCount == 0)
                return CodecReturnCodes.SET_DEF_DB_EMPTY;

            if (_setCount > LocalFieldSetDefDbImpl.MAX_LOCAL_ID)
                return CodecReturnCodes.TOO_MANY_LOCAL_SET_DEFS;

            for (int i = 0; i <= LocalFieldSetDefDbImpl.MAX_LOCAL_ID; i++)
                localSetDb._definitions[i]._setId = LocalFieldSetDefDbImpl.BLANK_ID;

            for (int i = 0; i < _setCount; i++)
            {
                FieldSetDefImpl curSetDef;
                FieldSetDefEntryImpl curEntry;
                int setId;
                int encCount;

                if ((position + 2) > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* Get the setId and the number of field encodings */
                setId = iter._reader.readUShort15rb();
                encCount = iter._reader.readUnsignedByte();

                /* System.out.println("  FieldSetDef: setId " + setId + " count " + encCount); */

                /* Basic sanity checks */
                if (setId > LocalFieldSetDefDbImpl.MAX_LOCAL_ID)
                    return CodecReturnCodes.ILLEGAL_LOCAL_SET_DEF;
                if (localSetDb._definitions[setId]._setId != LocalFieldSetDefDbImpl.BLANK_ID)
                    return CodecReturnCodes.DUPLICATE_LOCAL_SET_DEFS;

                /* get memory for new set definition from working memory. */
                curSetDef = localSetDb._definitions[setId];

                /* make sure we have space in our entry pool */
                if (curEntryCount > localSetDb._entries.length)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                /* Setup set definition and put in database */
                curSetDef._setId = setId;
                curSetDef._count = encCount;
                curSetDef.entries(localSetDb._entries[curEntryCount]); /* Point to the entries from the pool that will be used for this def. */

                /* Fill in the field list encodings */
                for (int j = 0; j < encCount; j++)
                {
                    /* make sure we have space in our entry pool */
                    if (encCount > localSetDb._entries[curEntryCount].length)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    curEntry = localSetDb._entries[curEntryCount][j];
                    curEntry._fieldId = iter._reader.readUnsignedShort();
                    curEntry._dataType = iter._reader.readUnsignedByte();
                    position = iter._reader.position();
                }
                ++curEntryCount;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
        return (position <= _endBufPos) ? CodecReturnCodes.SUCCESS : CodecReturnCodes.INCOMPLETE_DATA;
    }

    static int decodeSeriesEntry(DecodeIterator iterInt, SeriesEntry seriesEntryInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        SeriesEntryImpl seriesEntry = (SeriesEntryImpl)seriesEntryInt;
        int position;
        SeriesImpl series;
        DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

        assert null != seriesEntry && null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != iter._levelInfo[iter._decodingLevel]._listType : "Invalid decoding attempted";

        series = (SeriesImpl)_levelInfo._listType;

        if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
        {
            endOfList(iter);
            return CodecReturnCodes.END_OF_CONTAINER;
        }

        /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
        position = iter._curBufPos = _levelInfo._nextEntryPos;
        try
        {
            iter._reader.position(position);

            if (series._containerType != DataTypes.NO_DATA)
            {
                decodeBuffer16(iter, seriesEntry._encodedData);
                position = iter._reader.position();
                if (position > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* shift iterator */
                iter._curBufPos = ((BufferImpl)seriesEntry._encodedData).position();
                _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                _levelInfo._nextItemPosition++;
                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                seriesEntry._encodedData.clear();
                _levelInfo._nextItemPosition++;
                iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                iter._reader.position(position);
                return CodecReturnCodes.SUCCESS;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
    }

    static int convertToPrimitiveType(int type)
    {
        int retVal = type;

        switch (type)
        {
            case DataTypes.RESERVED1:
            case DataTypes.INT_1:
            case DataTypes.INT_2:
            case DataTypes.INT_4:
            case DataTypes.INT_8:
                retVal = DataTypes.INT;
                break;
            case DataTypes.RESERVED2:
            case DataTypes.UINT_1:
            case DataTypes.UINT_2:
            case DataTypes.UINT_4:
            case DataTypes.UINT_8:
                retVal = DataTypes.UINT;
                break;
            case DataTypes.FLOAT_4:
                retVal = DataTypes.FLOAT;
                break;
            case DataTypes.DOUBLE_8:
                retVal = DataTypes.DOUBLE;
                break;
            case DataTypes.REAL_4RB:
            case DataTypes.REAL_8RB:
                retVal = DataTypes.REAL;
                break;
            case DataTypes.RESERVED7:
                retVal = DataTypes.REAL;
                break;
            case DataTypes.DATE_4:
                retVal = DataTypes.DATE;
                break;
            case DataTypes.TIME_3:
            case DataTypes.TIME_5:
            case DataTypes.TIME_7:
            case DataTypes.TIME_8:
                retVal = DataTypes.TIME;
                break;
            case DataTypes.DATETIME_7:
            case DataTypes.DATETIME_9:
            case DataTypes.DATETIME_11:
            case DataTypes.DATETIME_12:
                retVal = DataTypes.DATETIME;
                break;
            default:
                break;
        }

        return retVal;
    }

    static int primitiveToString(Object type, int dataType, Buffer out)
    {
        int ret = CodecReturnCodes.SUCCESS;
        switch (dataType)
        {
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                ((BufferImpl)out).data_internal(((Buffer)type).toString());
                break;
            case DataTypes.UINT:
                String tempUInt = ((UInt)type).toString();
                ((BufferImpl)out).data_internal(tempUInt);
                break;
            case DataTypes.INT:
                String tempInt = ((Int)type).toString();
                ((BufferImpl)out).data_internal(tempInt);
                break;
            case DataTypes.FLOAT:
                String tempFloat = ((Float)type).toString();
                ((BufferImpl)out).data_internal(tempFloat);
                break;
            case DataTypes.DOUBLE:
                String tempDouble = ((Double)type).toString();
                ((BufferImpl)out).data_internal(tempDouble);
                break;
            case DataTypes.REAL:
                String tempReal = ((Real)type).toString();
                ((BufferImpl)out).data_internal(tempReal);
                break;
            case DataTypes.DATE:
                String tempDate = ((Date)type).toString();
                ((BufferImpl)out).data_internal(tempDate);
                break;
            case DataTypes.TIME:
                String tempTime = ((Time)type).toString();
                ((BufferImpl)out).data_internal(tempTime);
                break;
            case DataTypes.DATETIME:
                String tempDateTime = ((DateTime)type).toString();
                ((BufferImpl)out).data_internal(tempDateTime);
                break;
            case DataTypes.QOS:
                String tempQos = ((Qos)type).toString();
                ((BufferImpl)out).data_internal(tempQos);
                break;
            case DataTypes.STATE:
                String tempState = ((State)type).toString();
                ((BufferImpl)out).data_internal(tempState);
                break;
            case DataTypes.ENUM:
                String tempEnum = ((Enum)type).toString();
                ((BufferImpl)out).data_internal(tempEnum);
                break;
            default:
                ret = CodecReturnCodes.FAILURE;
                break;
        }

        return ret;
    }

    static int getItemCount(DecodeIteratorImpl iter)
    {
        return iter._levelInfo[iter._decodingLevel]._itemCount;
    }

    static int decodeVector(DecodeIterator iterInt, Vector vectorInt)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != vectorInt : "Invalid vectorInt in as NULL";
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        VectorImpl vector = (VectorImpl)vectorInt;
        int position;
        int _endBufPos;
        DecodingLevel _levelInfo;

        _levelInfo = iter._levelInfo[++iter._decodingLevel];
        if (iter._decodingLevel >= DecodeIteratorImpl.DEC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        iter.setup(_levelInfo, DataTypes.VECTOR, vector);

        _endBufPos = _levelInfo._endBufPos;
        position = iter._curBufPos;

        if (_endBufPos - position == 0)
        {
            endOfList(iter);
            return CodecReturnCodes.NO_DATA;
        }
        try
        {
            iter._reader.position(position);

            vector._flags = (iter._reader.readByte());

            vector._containerType = (iter._reader.readUnsignedByte());
            /* container type needs to be scaled after its decoded */
            vector._containerType += DataTypes.CONTAINER_TYPE_MIN;

            if (vector.checkHasSetDefs())
            {
                decodeBuffer15(iter, vector._encodedSetDefs);
                position = iter._reader.position();
                /* check for buffer overflow. Post check ok since no copy */
                if (position > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
            }
            else
            {
                vector._encodedSetDefs.clear();
            }

            if (vector.checkHasSummaryData())
            {
                decodeBuffer15(iter, vector._encodedSummaryData);
                position = iter._reader.position();
                /* check for buffer overflow. Post check ok since no copy */
                if (position > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
            }
            else
            {
                vector._encodedSummaryData.clear();
            }

            /* take out total count hint if present */
            if (vector.checkHasTotalCountHint())
                vector._totalCountHint = (iter._reader.readUInt30rb());
            else
                vector._totalCountHint = 0;

            _levelInfo._itemCount = iter._reader.readUnsignedShort();
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        position = iter._reader.position();

        ((BufferImpl)vector._encodedEntries).data_internal(iter._reader.buffer(), position, (_endBufPos - position));

        /* check for buffer overflow. Post check ok since no copy */
        if (position > _endBufPos)
            return CodecReturnCodes.INCOMPLETE_DATA;

        _levelInfo._nextEntryPos = position; /* set entry ptr to first entry */
        iter._curBufPos = vector.checkHasSummaryData() ? ((BufferImpl)vector._encodedSummaryData).position() : position;

        return CodecReturnCodes.SUCCESS;
    }

    static int decodeVectorEntry(DecodeIterator iterInt, VectorEntry vectorEntryInt)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        VectorEntryImpl vectorEntry = (VectorEntryImpl)vectorEntryInt;
        int position;
        Vector vector;
        int flags;
        DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

        assert null != vectorEntry && null != iter : "Invalid parameters or parameters passed in as NULL)";
        assert null != iter._levelInfo[iter._decodingLevel]._listType : "Invalid decoding attempted";

        vector = (Vector)_levelInfo._listType;

        if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
        {
            endOfList(iter);
            return CodecReturnCodes.END_OF_CONTAINER;
        }

        /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
        position = iter._curBufPos = _levelInfo._nextEntryPos;
        try
        {
            iter._reader.position(position);

            if ((position + 2) > _levelInfo._endBufPos)
                return CodecReturnCodes.INCOMPLETE_DATA;

            /* take out action/flags */
            flags = iter._reader.readByte();

            vectorEntry._action = (flags & 0xF);
            vectorEntry._flags = (flags >> 4);

            /* get index */
            vectorEntry._index = (iter._reader.readUInt30rb());

            /* get perm data */
            if ((vector.checkHasPerEntryPermData()) && (vectorEntry.checkHasPermData()))
            {
                decodeBuffer15(iter, vectorEntry._permData);
                position = iter._reader.position();
                if (position > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;
            }
            else
            {
                vectorEntry._permData.clear();
            }

            if ((vectorEntry.action() != VectorEntryActions.CLEAR)
                    && (vectorEntry.action() != VectorEntryActions.DELETE)
                    && (vector.containerType() != DataTypes.NO_DATA))
            {
                decodeBuffer16(iter, vectorEntry._encodedData);
                position = iter._reader.position();
                if (position > _levelInfo._endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* shift iterator */
                _levelInfo._nextItemPosition++;
                iter._curBufPos = ((BufferImpl)vectorEntry._encodedData).position();
                _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                position = iter._reader.position();
                vectorEntry._encodedData.clear();
                _levelInfo._nextItemPosition++;
                iter._curBufPos = _levelInfo._nextEntryPos
                        = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                return CodecReturnCodes.SUCCESS;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
    }

    private static int decBuf8(DecodeIterator iterInt, Buffer data) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        int length;

        assert iter._curBufPos == levelInfo._nextEntryPos : "Invalid decoding attempted";

        /* Move _curBufPos and _endBufPos around data. _nextEntryPos should point after it. */
        length = iter._reader.readUnsignedByte();
        iter._curBufPos = iter._reader.position();
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, length);
        levelInfo._nextEntryPos = iter._curBufPos + length;

        return (levelInfo._nextEntryPos <= levelInfo._endBufPos ? CodecReturnCodes.SUCCESS : CodecReturnCodes.INCOMPLETE_DATA);
    }

    private static void decBuf16(DecodeIterator iterInt, Buffer data) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        int tlen;

        assert iter._curBufPos == levelInfo._nextEntryPos : "Invalid decoding attempted";

        tlen = iter._reader.readUShort16ob();
        iter._curBufPos = iter._reader.position();
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, tlen);
        levelInfo._nextEntryPos = iter._curBufPos + tlen;
    }

    private static void dec8(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 1);
        levelInfo._nextEntryPos++;
    }

    private static void dec16(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 2);
        levelInfo._nextEntryPos += 2;
    }
    
    private static void dec24(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 3);
        levelInfo._nextEntryPos += 3;
    }

    private static void dec32(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 4);
        levelInfo._nextEntryPos += 4;
    }
    
    private static void dec40(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 5);
        levelInfo._nextEntryPos += 5;
    }
    
    private static void dec56(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 7);
        levelInfo._nextEntryPos += 7;
    }
    
    private static void dec64(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 8);
        levelInfo._nextEntryPos += 8;
    }
    
    private static void dec72(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 9);
        levelInfo._nextEntryPos += 9;
    }
    
    private static void dec88(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 11);
        levelInfo._nextEntryPos += 11;
    }
    
    private static void dec96(DecodeIterator iterInt, Buffer data)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, 12);
        levelInfo._nextEntryPos += 12;
    }

    private static void decReal_4rb(DecodeIterator iterInt, Buffer data) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        int format = 0;

        format = iter._reader.readByte();

        int len;
        if ((format & 0x20) == 1)
            len = 1;
        else
        {
            /* mask out all but first two bits */
            switch (format & 0xC0)
            {
                case 0:
                    format = 0;
                    break;
                case 64:
                    format = 1;
                    break;
                case 128:
                    format = 2;
                    break;
                case 192:
                    format = 3;
                    break;
                default:
                    format = 0;
                    break;
            }

            len = real32LenHints[format];
        }
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, len);
        levelInfo._nextEntryPos += data.length();
    }
    
    private static void decReal_8rb(DecodeIterator iterInt, Buffer data) throws Exception
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
        int format = 0;

        format = iter._reader.readByte();

        int len;
        if ((format & 0x20) == 1)
            len = 1;
        else
        {
            /* mask out all but first two bits */
            switch (format & 0xC0)
            {
                case 0:
                    format = 0;
                    break;
                case 64:
                    format = 1;
                    break;
                case 128:
                    format = 2;
                    break;
                case 192:
                    format = 3;
                    break;
                default:
                    format = 0;
                    break;
            }

            len = real64LenHints[format];
        }
        ((BufferImpl)data).data_internal(iter._reader.buffer(), iter._curBufPos, len);
        levelInfo._nextEntryPos += data.length();
    }
    
    static int decodeFloat(DecodeIterator iterInt, Float value)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret = CodecReturnCodes.SUCCESS;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 4)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                value.value(iter._reader.readFloat());
                iter._reader.position(savePosition);
            }
            catch (Exception e)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            ((FloatImpl)value).blank();
            ret = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ret = CodecReturnCodes.INCOMPLETE_DATA;
        }
        return ret;
    }

    static int decodeDouble(DecodeIterator iterInt, Double value)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret = CodecReturnCodes.SUCCESS;

        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 8)
        {
            int savePosition = iter._reader.position();
            try
            {
                iter._reader.position(iter._curBufPos);
                value.value(iter._reader.readDouble());
                iter._reader.position(savePosition);
            }
            catch (Exception e)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
        {
            ((DoubleImpl)value).blank();
            ret = CodecReturnCodes.BLANK_DATA;
        }
        else
        {
            ret = CodecReturnCodes.INCOMPLETE_DATA;
        }
        return ret;
    }

    static int decodeFieldSetDefDb(DecodeIterator iterInt, FieldSetDefDbImpl fieldSetDefDbImpl)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        FieldSetDefDbImpl localSetDb = fieldSetDefDbImpl;
        int position;
        int _endBufPos;
        int _setCount;
        @SuppressWarnings("unused")
        int flags;

        int curEntryCount = 0;

        assert null != iter && null != localSetDb : "Invalid parameters or parameters passed in as NULL";
        try
        {
            if (iter._decodingLevel >= 0)
            {
                /* Get the encodedSetDefs pointer out of the current container. */
                Buffer encodedSetDefs;

                assert null != iter && null != localSetDb : "Invalid parameters or parameters passed in as NULL";

                switch (iter._levelInfo[iter._decodingLevel]._containerType)
                {
                    case DataTypes.MAP:
                        encodedSetDefs = ((MapImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.SERIES:
                        encodedSetDefs = ((SeriesImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.VECTOR:
                        encodedSetDefs = ((VectorImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    default:
                        return CodecReturnCodes.INVALID_ARGUMENT;
                }

                if (encodedSetDefs.length() == 0)
                    return CodecReturnCodes.INVALID_DATA;

                position = ((BufferImpl)encodedSetDefs).position();
                iter._reader.position(position);
                _endBufPos = ((BufferImpl)encodedSetDefs).position() + encodedSetDefs.length();
            }
            else
            {
                /* Separate iterator. */
                _endBufPos = iter._levelInfo[0]._endBufPos;
                position = iter._curBufPos;
            }

            if ((_endBufPos - position) < 2)
                return CodecReturnCodes.INCOMPLETE_DATA;

            flags = iter._reader.readByte();
            _setCount = iter._reader.readUnsignedByte();
            position = iter._reader.position();

            /* System.out.println("FieldSetDefDb: flags " + flags + " count " + _setCount); */

            if (_setCount == 0)
                return CodecReturnCodes.SET_DEF_DB_EMPTY;

            if (_setCount > LocalFieldSetDefDbImpl.MAX_LOCAL_ID)
                return CodecReturnCodes.TOO_MANY_LOCAL_SET_DEFS;

            for (int i = 0; i <= LocalFieldSetDefDbImpl.MAX_LOCAL_ID; i++)
                localSetDb._definitions[i]._setId = LocalFieldSetDefDbImpl.BLANK_ID;

            for (int i = 0; i < _setCount; i++)
            {
                FieldSetDefImpl curSetDef;
                FieldSetDefEntryImpl curEntry;
                int setId;
                int encCount;

                if ((position + 2) > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* Get the setId and the number of field encodings */
                setId = iter._reader.readUShort15rb();
                encCount = iter._reader.readUnsignedByte();

                /* System.out.println("  FieldSetDef: setId " + setId + " count " + encCount); */

                /* Basic sanity checks */
                if (setId > fieldSetDefDbImpl.maxLocalId)
                    return CodecReturnCodes.ILLEGAL_LOCAL_SET_DEF;
                if (localSetDb._definitions[setId]._setId != FieldSetDefDbImpl.BLANK_ID)
                    return CodecReturnCodes.DUPLICATE_LOCAL_SET_DEFS;

                /* get memory for new set definition from working memory. */
                curSetDef = localSetDb._definitions[setId];

                /* make sure we have space in our entry pool */
                if (curEntryCount > localSetDb.definitions()[curEntryCount].entries().length)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                /* Setup set definition and put in database */
                curSetDef._setId = setId;
                curSetDef._count = encCount;
                curSetDef.entries(localSetDb.definitions()[curEntryCount].entries()); /* Point to the entries from the pool that will be used for this def. */

                /* Fill in the field list encodings */
                for (int j = 0; j < encCount; j++)
                {
                    /* make sure we have space in our entry pool */
                    if (encCount > localSetDb.definitions()[curEntryCount].entries().length)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    curEntry = (FieldSetDefEntryImpl)localSetDb.definitions()[curEntryCount].entries()[j];
                    curEntry._fieldId = (iter._reader.readUnsignedShort());
                    curEntry._dataType = (iter._reader.readUnsignedByte());
                    position = iter._reader.position();
                }
                ++curEntryCount;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }
        return (position <= _endBufPos) ? CodecReturnCodes.SUCCESS : CodecReturnCodes.INCOMPLETE_DATA;
    }

    static int decodeElementSetDefDb(DecodeIterator iterInt, ElementSetDefDbImpl elementSetDefDbImpl)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int position;
        int _endBufPos;
        @SuppressWarnings("unused")
        int flags;
        int _setCount;
        int curEntryCount = 0;
        ElementSetDefDbImpl localSetDb = elementSetDefDbImpl;

        assert null != iter && null != localSetDb : "Invalid parameters or parameters passed in as NULL";

        try
        {
            if (iter._decodingLevel >= 0)
            {
                /* Get the encodedSetDefs pointer out of the current container. */
                Buffer encodedSetDefs;

                assert null != iter._levelInfo[iter._decodingLevel]._listType : "Invalid decoding attempted";

                switch (iter._levelInfo[iter._decodingLevel]._containerType)
                {
                    case DataTypes.MAP:
                        encodedSetDefs = ((MapImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.SERIES:
                        encodedSetDefs = ((SeriesImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    case DataTypes.VECTOR:
                        encodedSetDefs = ((VectorImpl)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                        break;
                    default:
                        return CodecReturnCodes.INVALID_ARGUMENT;
                }

                if (encodedSetDefs.length() == 0)
                    return CodecReturnCodes.INVALID_DATA;

                position = ((BufferImpl)encodedSetDefs).position();
                iter._reader.position(position);
                _endBufPos = ((BufferImpl)encodedSetDefs).position() + encodedSetDefs.length();
            }
            else
            {
                /* Separate iterator. */
                _endBufPos = iter._levelInfo[0]._endBufPos;
                position = iter._curBufPos;
            }

            if ((_endBufPos - position) < 2)
                return CodecReturnCodes.INCOMPLETE_DATA;

            flags = iter._reader.readByte();
            _setCount = iter._reader.readUnsignedByte();
            position = iter._reader.position();

            /* System.out.println("ElementSetDefDb: flags " + flags + " count " + _setCount); */

            if (_setCount == 0)
                return CodecReturnCodes.SET_DEF_DB_EMPTY;

            if (_setCount > elementSetDefDbImpl.MAX_LOCAL_ID)
                return CodecReturnCodes.TOO_MANY_LOCAL_SET_DEFS;

            for (int i = 0; i <= elementSetDefDbImpl.MAX_LOCAL_ID; i++)
                localSetDb._definitions[i]._setId = ElementSetDefDbImpl.BLANK_ID;

            for (int i = 0; i < _setCount; i++)
            {
                ElementSetDefImpl curSetDef;
                ElementSetDefEntryImpl curEntry;
                int setId;
                int encCount;

                if ((position + 2) > _endBufPos)
                    return CodecReturnCodes.INCOMPLETE_DATA;

                /* get the setId and the number of element encodings */
                setId = iter._reader.readUShort15rb();
                encCount = iter._reader.readUnsignedByte();

                /* System.out.println("  ElementSetDef: setId " + setId + " count " + encCount); */

                /* sanity checks */
                if (setId > elementSetDefDbImpl.MAX_LOCAL_ID)
                    return CodecReturnCodes.ILLEGAL_LOCAL_SET_DEF;
                if (localSetDb._definitions[setId]._setId != ElementSetDefDbImpl.BLANK_ID)
                    return CodecReturnCodes.DUPLICATE_LOCAL_SET_DEFS;

                /* get memory for new set definition from working memory. */
                curSetDef = localSetDb._definitions[setId];

                /* make sure we have space in our entry pool */
                if (curEntryCount > localSetDb._definitions[curEntryCount]._entries.length)
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                /* setup set def and put in database */
                curSetDef._setId = setId;
                curSetDef._count = encCount;
                curSetDef._entries = localSetDb._definitions[curEntryCount]._entries;
                
                /* populate the entries */
                for (int j = 0; j < encCount; j++)
                {
                    /* make sure we have space in our entry pool */
                    if (encCount > localSetDb._definitions[curEntryCount]._entries.length)
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    curEntry = (ElementSetDefEntryImpl)localSetDb._definitions[curEntryCount]._entries[j];
                    decodeBuffer15(iter, curEntry.name());
                    curEntry._dataType = iter._reader.readUnsignedByte();
                    position = iter._reader.position();
                }
                ++curEntryCount;
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        return (position <= _endBufPos) ? CodecReturnCodes.SUCCESS : CodecReturnCodes.INCOMPLETE_DATA;
    }
}
