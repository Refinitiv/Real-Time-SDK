/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;
import java.util.Arrays;

import com.refinitiv.eta.codec.AckMsgFlags;
import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.BufferImpl;
import com.refinitiv.eta.codec.CloseMsgFlags;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Decoders;
import com.refinitiv.eta.codec.DecodersToXML;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Encoders;
import com.refinitiv.eta.codec.GenericMsgFlags;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.PostMsgFlags;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.PostUserInfo;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.UpdateMsg;

/* This class provides the implementation for the superset of message classes
 * (e.g. RequestMsg, RefreshMsg, etc.).
 * Representing the superset of message types in a single class affords users the ability
 * to maintain a single pool of messages that can be re-used across message types.
 * Users cast to the appropriate message-class specific interface.
 * For example, an instance of this class may initially represent a
 * request message, and later, a response message.
 */
final class MsgImpl implements Msg, AckMsg, CloseMsg, GenericMsg, PostMsg, RefreshMsg, RequestMsg, StatusMsg, UpdateMsg
{
    private static final int GENERAL_PURPOSE_LONGS = 2;
    private static final int GENERAL_PURPOSE_INTS = 4;
    private static final int GENERAL_PURPOSE_BUFFERS = 2;
    private static final int GENERAL_PURPOSE_QOS = 2;
    
    /* To make efficient use of memory and let a single MsgImpl class
     * represent any message class defined by MsgClasses,
     * we "simulate a C++ union" by (re)using general-purpose fields that are
     * sufficient to provide storage for all the message classes.
     * 
     * This (private) class tells us where message-class-specific values will be stored.
     */
    private static class MsgFields
    {
        private MsgFields()
        {
        }
        
        private static class Common
        {
            private static class Long
            {
                private static final int SEQ_NUM = 0;
            }

            private static class Int
            {
                private static final int FLAGS = 0;
                private static final int PART_NUM = 1;
            }

            private static class Buffer
            {
                private static final int PERM_DATA = 0;
                private static final int GROUP_ID = 1;
            }

            private static class Qos
            {
                private static final int QOS = 0;
            }
        }
        
        private static class Ack
        {
            private static class Long
            {
                private static final int ACK_ID = 1;
            }

            private static class Int
            {
                private static final int NAK_CODE = 2;
            }

            private static class Buffer
            {
                private static final int TEXT = 0;
            }
        }
        
        private static class Generic
        {
            private static class Long
            {
                private static final int SECONDARY_SEQ_NUM = 1;
            }
        }
        
        private static class Post
        {
            private static class Long
            {
                private static final int POST_ID = 1;
            }

            private static class Int
            {
                private static final int POST_USER_RIGHTS = 3;
            }
        }
        
        private static class Request
        {
            private static class Qos
            {
                private static final int WORST_QOS = 1;
            }
        }
        
        private static class Update
        {
            private static class Int
            {
                private static final int UPDATE_TYPE = 1;
                private static final int CONFLATION_COUNT = 2;
                private static final int CONFLATION_TIME = 3;
            }
        }
    }
    
    // the following fields are common to all the message types
    private int         _msgClass;   // uint5
    private int         _domainType; // uint8
    private int         _containerType = DataTypes.CONTAINER_TYPE_MIN; // uint8
    private int         _streamId;   // int32
    private final MsgKey     _msgKey = new MsgKeyImpl();
    private final Buffer     _extendedHeader = CodecFactory.createBuffer();
    private final Buffer     _encodedDataBody = CodecFactory.createBuffer();
    private final Buffer     _encodedMsgBuffer = CodecFactory.createBuffer();
	
    // Through the use of the following general-purpose fields, we attempt
    // to "simulate" a C++ union so we can implement storage for the superset
    // of message types while using as little memory as possible.
    private long[] _generalLong = new long[GENERAL_PURPOSE_LONGS];
    private int[] _generalInt = new int[GENERAL_PURPOSE_INTS];
    private Buffer[] _generalBuffer = new Buffer[GENERAL_PURPOSE_BUFFERS];
    private final PostUserInfoImpl _generalPostUserInfo = new PostUserInfoImpl();
    private State _generalState;
    private Qos[] _generalQos = new Qos[GENERAL_PURPOSE_QOS];
    private Priority _generalPriority;
	
    // flags to track if key and extended header space to be reserved
    boolean _keyReserved;
    boolean _extendedHeaderReserved;
	
    @Override
    public MsgKey msgKey()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_MSG_KEY) > 0 ? _msgKey : null);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_MSG_KEY) > 0 ? _msgKey : null);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_MSG_KEY) > 0 ? _msgKey : null);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_MSG_KEY) > 0 ? _msgKey : null);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_MSG_KEY) > 0 ? _msgKey : null);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_MSG_KEY) > 0 ? _msgKey : null);
            case MsgClasses.REQUEST: // request message always has message key
                return _msgKey;
            default: // all other messages don't have message key
                return null;
        }
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeMsg(iter, this);
    }

    @Override
    public int encodeInit(EncodeIterator iter, int dataMaxSize)
    {
        return Encoders.encodeMsgInit(iter, this, dataMaxSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeMsgComplete(iter, success);
    }

    @Override
    public int encodeKeyAttribComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeMsgKeyAttribComplete(iter, success);
    }

    @Override
    public int encodeExtendedHeaderComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeExtendedHeaderComplete(iter, success);
    }

    @Override
    public void msgClass(int msgClass)
    {
        assert (msgClass > 0 && msgClass <= 31) : "msgClass is out of range. Refer to MsgClasses"; // uint5

        _msgClass = msgClass;
    }

    @Override
    public int msgClass()
    {
        return _msgClass;
    }

    @Override
    public void domainType(int domainType)
    {
        assert (domainType >= 0 && domainType <= 255) : "domainType is out of range. Refer to DomainTypes"; // uint8

        _domainType = domainType;
    }

    @Override
    public int domainType()
    {
        return _domainType;
    }

    @Override
    public void containerType(int containerType)
    {
        assert (containerType >= DataTypes.CONTAINER_TYPE_MIN && containerType <= DataTypes.LAST) : 
            "containerType must be from the DataTypes enumeration in the range CONTAINER_TYPE_MIN to LAST.";

        _containerType = containerType;
    }

    @Override
    public int containerType()
    {
        return _containerType;
    }

    @Override
    public void streamId(int streamId)
    {
        assert (streamId >= -2147483648 && streamId <= 2147483647) : "streamId is out of range ((-2147483648)-2147483647)"; // int32

        _streamId = streamId;
    }

    @Override
    public int streamId()
    {
        return _streamId;
    }

    @Override
    public void extendedHeader(Buffer extendedHeader)
    {
        assert (extendedHeader != null) : "extendedHeader must be non-null";

        ((BufferImpl)_extendedHeader).copyReferences(extendedHeader);
    }

    @Override
    public Buffer extendedHeader()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            case MsgClasses.CLOSE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & CloseMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            case MsgClasses.REQUEST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_EXTENDED_HEADER) > 0 ? _extendedHeader : null);
            default:
                return null;
        }
    }

    @Override
    public void encodedDataBody(Buffer encodedDataBody)
    {
        assert (encodedDataBody != null) : "encodedDataBody must be non-null";

        ((BufferImpl)_encodedDataBody).copyReferences(encodedDataBody);
    }

    @Override
    public Buffer encodedDataBody()
    {
        return _encodedDataBody;
    }

    void encodedMsgBuffer(ByteBuffer encodedMsgBuffer, int position, int length)
    {
        assert (encodedMsgBuffer != null) : "encodedMsgBuffer must be non-null";

        ((BufferImpl)_encodedMsgBuffer).data_internal(encodedMsgBuffer, position, length);
    }

    @Override
    public Buffer encodedMsgBuffer()
    {
        return _encodedMsgBuffer;
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeMsg(iter, this);
    }

    @Override
    public int decodeKeyAttrib(DecodeIterator iter, MsgKey key)
    {
        return Decoders.decodeMsgKeyAttrib(iter, key);
    }

    private int copyUpdateMsg(UpdateMsg msg, int copyMsgFlags)
    {
        int ret;
        msg.updateType(updateType());
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~UpdateMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if (checkHasPermData())
        {
            if ((copyMsgFlags & CopyMsgFlags.PERM_DATA) > 0)
            {
                if ((ret = ((BufferImpl)permData()).copyWithOrWithoutByteBuffer(msg.permData())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~UpdateMsgFlags.HAS_PERM_DATA);
            }
        }
        if (checkHasMsgKey())
        {
            if ((ret = ((MsgKeyImpl)msgKey()).copy(msg.msgKey(), copyMsgFlags)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (checkHasPostUserInfo())
        {
            msg.postUserInfo().userAddr(postUserInfo().userAddr());
            msg.postUserInfo().userId(postUserInfo().userId());
            msg.applyHasPostUserInfo();
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    private int copyRefreshMsg(RefreshMsg msg, int copyMsgFlags)
    {
        int ret;
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~RefreshMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if (checkHasPermData())
        {
            if ((copyMsgFlags & CopyMsgFlags.PERM_DATA) > 0)
            {
                if ((ret = ((BufferImpl)permData()).copyWithOrWithoutByteBuffer(msg.permData())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~RefreshMsgFlags.HAS_PERM_DATA);
            }
        }
        if (checkHasMsgKey())
        {
            if ((ret = ((MsgKeyImpl)msgKey()).copy(msg.msgKey(), copyMsgFlags)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if ((copyMsgFlags & CopyMsgFlags.GROUP_ID) > 0)
        {
            if ((ret = ((BufferImpl)groupId()).copyWithOrWithoutByteBuffer(msg.groupId())) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        msg.state().dataState(state().dataState());
        msg.state().streamState(state().streamState());
        msg.state().code(state().code());
        if ((copyMsgFlags & CopyMsgFlags.STATE_TEXT) > 0)
        {
            if ((ret = ((BufferImpl)state().text()).copyWithOrWithoutByteBuffer(msg.state().text())) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (checkHasQos())
        {
            msg.qos().rate(qos().rate());
            msg.qos().rateInfo(qos().rateInfo());
            msg.qos().timeliness(qos().timeliness());
            msg.qos().timeInfo(qos().timeInfo());
            msg.qos().dynamic(qos().isDynamic());
            msg.applyHasQos();
        }
        if (checkHasPostUserInfo())
        {
            msg.postUserInfo().userAddr(postUserInfo().userAddr());
            msg.postUserInfo().userId(postUserInfo().userId());
            msg.applyHasPostUserInfo();
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    private int copyRequestMsg(RequestMsg msg, int copyMsgFlags)
    {
        int ret;
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~RequestMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if ((ret = ((MsgKeyImpl)msgKey()).copy(msg.msgKey(), copyMsgFlags)) != CodecReturnCodes.SUCCESS)
            return ret;
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (checkHasPriority())
        {
            msg.priority().count(priority().count());
            msg.priority().priorityClass(priority().priorityClass());
            msg.applyHasPriority();
        }
        if (checkHasQos())
        {
            msg.qos().rate(qos().rate());
            msg.qos().rateInfo(qos().rateInfo());
            msg.qos().timeliness(qos().timeliness());
            msg.qos().timeInfo(qos().timeInfo());
            msg.qos().dynamic(qos().isDynamic());
            msg.applyHasQos();
        }
        if (checkHasWorstQos())
        {
            msg.worstQos().rate(worstQos().rate());
            msg.worstQos().rateInfo(worstQos().rateInfo());
            msg.worstQos().timeliness(worstQos().timeliness());
            msg.worstQos().timeInfo(worstQos().timeInfo());
            msg.worstQos().dynamic(worstQos().isDynamic());
            msg.applyHasWorstQos();
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    private int copyPostMsg(PostMsg msg, int copyMsgFlags)
    {
        int ret;
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~PostMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if (checkHasPermData())
        {
            if ((copyMsgFlags & CopyMsgFlags.PERM_DATA) > 0)
            {
                if ((ret = ((BufferImpl)permData()).copyWithOrWithoutByteBuffer(msg.permData())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~PostMsgFlags.HAS_PERM_DATA);
            }
        }
        if (checkHasMsgKey())
        {
            if ((ret = ((MsgKeyImpl)msgKey()).copy(msg.msgKey(), copyMsgFlags)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        msg.postUserInfo().userAddr(postUserInfo().userAddr());
        msg.postUserInfo().userId(postUserInfo().userId());
        return CodecReturnCodes.SUCCESS;
    }
    
    private int copyGenericMsg(GenericMsg msg, int copyMsgFlags)
    {
        int ret;
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~GenericMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if (checkHasPermData())
        {
            if ((copyMsgFlags & CopyMsgFlags.PERM_DATA) > 0)
            {
                if ((ret = ((BufferImpl)permData()).copyWithOrWithoutByteBuffer(msg.permData())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~GenericMsgFlags.HAS_PERM_DATA);
            }
        }
        if (checkHasMsgKey())
        {
            if ((ret = ((MsgKeyImpl)msgKey()).copy(msg.msgKey(), copyMsgFlags)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    private int copyCloseMsg(CloseMsg msg, int copyMsgFlags)
    {
        int ret;
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~CloseMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    private int copyStatusMsg(StatusMsg msg, int copyMsgFlags)
    {
        int ret;
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~StatusMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if (checkHasPermData())
        {
            if ((copyMsgFlags & CopyMsgFlags.PERM_DATA) > 0)
            {
                if ((ret = ((BufferImpl)permData()).copyWithOrWithoutByteBuffer(msg.permData())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~StatusMsgFlags.HAS_PERM_DATA);
            }
        }
        if (checkHasMsgKey())
        {
            if ((ret = ((MsgKeyImpl)msgKey()).copy(msg.msgKey(), copyMsgFlags)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (checkHasGroupId())
        {
            if ((copyMsgFlags & CopyMsgFlags.GROUP_ID) > 0)
            {
                if ((ret = ((BufferImpl)groupId()).copyWithOrWithoutByteBuffer(msg.groupId())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~StatusMsgFlags.HAS_GROUP_ID);
            }
        }
        if (checkHasState())
        {
            msg.state().dataState(state().dataState());
            msg.state().streamState(state().streamState());
            msg.state().code(state().code());
            if ((copyMsgFlags & CopyMsgFlags.STATE_TEXT) > 0)
            {
                if ((ret = ((BufferImpl)state().text()).copyWithOrWithoutByteBuffer(msg.state().text())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (checkHasPostUserInfo())
        {
            msg.postUserInfo().userAddr(postUserInfo().userAddr());
            msg.postUserInfo().userId(postUserInfo().userId());
            msg.applyHasPostUserInfo();
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    private int copyAckMsg(AckMsg msg, int copyMsgFlags)
    {
        int ret;
        if (checkHasExtendedHdr())
        {
            if ((copyMsgFlags & CopyMsgFlags.EXTENDED_HEADER) > 0)
            {
                if ((ret = ((BufferImpl)_extendedHeader).copyWithOrWithoutByteBuffer(msg.extendedHeader())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~AckMsgFlags.HAS_EXTENDED_HEADER);
            }
        }
        if (checkHasMsgKey())
        {
            if ((ret = ((MsgKeyImpl)msgKey()).copy(msg.msgKey(), copyMsgFlags)) != CodecReturnCodes.SUCCESS)
                return ret;
        }
        if (encodedDataBody().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.DATA_BODY) > 0)
            {
                if ((ret = ((BufferImpl)encodedDataBody()).copyWithOrWithoutByteBuffer(msg.encodedDataBody())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (encodedMsgBuffer().length() > 0)
        {
            if ((copyMsgFlags & CopyMsgFlags.MSG_BUFFER) > 0)
            {
                if ((ret = ((BufferImpl)encodedMsgBuffer()).copyWithOrWithoutByteBuffer(msg.encodedMsgBuffer())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        if (checkHasText())
        {
            if ((copyMsgFlags & CopyMsgFlags.NAK_TEXT) > 0)
            {
                if ((ret = ((BufferImpl)text()).copyWithOrWithoutByteBuffer(msg.text())) != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                msg.flags(msg.flags() & ~AckMsgFlags.HAS_TEXT);
            }
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
    public int copy(Msg destMsg, int copyMsgFlags)
    {
        if (!validateMsg())
            return CodecReturnCodes.FAILURE;

        destMsg.clear();
        destMsg.msgClass(_msgClass);
        destMsg.domainType(_domainType);
        destMsg.containerType(_containerType);
        destMsg.streamId(_streamId);

        for (int i = 0; i < GENERAL_PURPOSE_LONGS; i++)
        {
            ((MsgImpl)destMsg)._generalLong[i] = _generalLong[i];
        }
        for (int i = 0; i < GENERAL_PURPOSE_INTS; i++)
        {
            ((MsgImpl)destMsg)._generalInt[i] = _generalInt[i];
        }

        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                UpdateMsg updMsg = (UpdateMsg)destMsg;
                return copyUpdateMsg(updMsg, copyMsgFlags);
            case MsgClasses.REFRESH:
                RefreshMsg rfMsg = (RefreshMsg)destMsg;
                return copyRefreshMsg(rfMsg, copyMsgFlags);
            case MsgClasses.REQUEST:
                RequestMsg rqMsg = (RequestMsg)destMsg;
                return copyRequestMsg(rqMsg, copyMsgFlags);
            case MsgClasses.POST:
                PostMsg postMsg = (PostMsg)destMsg;
                return copyPostMsg(postMsg, copyMsgFlags);
            case MsgClasses.GENERIC:
                GenericMsg genMsg = (GenericMsg)destMsg;
                return copyGenericMsg(genMsg, copyMsgFlags);
            case MsgClasses.CLOSE:
                CloseMsg clMsg = (CloseMsg)destMsg;
                return copyCloseMsg(clMsg, copyMsgFlags);
            case MsgClasses.STATUS:
                StatusMsg stMsg = (StatusMsg)destMsg;
                return copyStatusMsg(stMsg, copyMsgFlags);
            case MsgClasses.ACK:
                AckMsg ackMsg = (AckMsg)destMsg;
                return copyAckMsg(ackMsg, copyMsgFlags);
            default:
                return CodecReturnCodes.FAILURE;
        }
    }

    @Override
    public boolean isFinalMsg()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                if (state().isFinal())
                    return true;
                if (((flags() & RefreshMsgFlags.REFRESH_COMPLETE) > 0) &&
                        (state().streamState() == StreamStates.NON_STREAMING))
                    return true;
                break;
            case MsgClasses.STATUS:
                if (checkHasState() && state().isFinal())
                    return true;
                break;
            case MsgClasses.CLOSE:
                return true;
            default:
                return false;
        }

        return false;
    }

    @Override
    public boolean validateMsg()
    {
        if (checkHasExtendedHdr() && ((extendedHeader() == null) || (extendedHeader().length() == 0)))
            return false;

        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
            case MsgClasses.REFRESH:
            case MsgClasses.POST:
            case MsgClasses.GENERIC:
                if (checkHasPermData() && ((permData() == null) || (permData().length() == 0)))
                    return false;
                return true;
            case MsgClasses.REQUEST:
            case MsgClasses.CLOSE:
                return true;
            case MsgClasses.STATUS:
                if (checkHasPermData() && ((permData() == null) || (permData().length() == 0)))
                    return false;
                if (checkHasGroupId() && (groupId().length() == 0))
                    return false;
                return true;
            case MsgClasses.ACK:
                if (checkHasText() && (text().length() == 0))
                    return false;
                return true;
            default:
                return false;
        }
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.MSG, null, null, null, iter);
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.MSG, null, dictionary, null, iter);
    }

    @Override
    public boolean checkHasExtendedHdr()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            case MsgClasses.CLOSE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & CloseMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            case MsgClasses.REQUEST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_EXTENDED_HEADER) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public void applyHasExtendedHdr()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.HAS_EXTENDED_HEADER;
                break;
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.HAS_EXTENDED_HEADER;
                break;
            case MsgClasses.STATUS:
                _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.HAS_EXTENDED_HEADER;
                break;
            case MsgClasses.GENERIC:
                _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.HAS_EXTENDED_HEADER;
                break;
            case MsgClasses.CLOSE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= CloseMsgFlags.HAS_EXTENDED_HEADER;
                break;
            case MsgClasses.REQUEST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.HAS_EXTENDED_HEADER;
                break;
            case MsgClasses.POST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.HAS_EXTENDED_HEADER;
                break;
            case MsgClasses.ACK:
                _generalInt[MsgFields.Common.Int.FLAGS] |= AckMsgFlags.HAS_EXTENDED_HEADER;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }

    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= 32767) : "flags is out of range (0-32767)"; // uint15-rb

        _generalInt[MsgFields.Common.Int.FLAGS] = flags;
    }

    @Override
    public int flags()
    {
        return _generalInt[MsgFields.Common.Int.FLAGS];
    }
    
    @Override
    public boolean checkHasMsgKey()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
   
    @Override
    public void applyHasMsgKey()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.HAS_MSG_KEY;
                break;
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.HAS_MSG_KEY;
                break;
            case MsgClasses.STATUS:
                _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.HAS_MSG_KEY;
                break;
            case MsgClasses.GENERIC:
                _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.HAS_MSG_KEY;
                break;
            case MsgClasses.POST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.HAS_MSG_KEY;
                break;
            case MsgClasses.ACK:
                _generalInt[MsgFields.Common.Int.FLAGS] |= AckMsgFlags.HAS_MSG_KEY;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }

    @Override
    public boolean checkHasSeqNum()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_SEQ_NUM) > 0 ? true : false);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_SEQ_NUM) > 0 ? true : false);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_SEQ_NUM) > 0 ? true : false);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_SEQ_NUM) > 0 ? true : false);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_SEQ_NUM) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public void applyHasSeqNum()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.HAS_SEQ_NUM;
                break;
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.HAS_SEQ_NUM;
                break;
            case MsgClasses.STATUS:
                break;
            case MsgClasses.GENERIC:
                _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.HAS_SEQ_NUM;
                break;
            case MsgClasses.POST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.HAS_SEQ_NUM;
                break;
            case MsgClasses.ACK:
                _generalInt[MsgFields.Common.Int.FLAGS] |= AckMsgFlags.HAS_SEQ_NUM;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
    
    @Override
    public void seqNum(long seqNum)
    {
        assert (seqNum >= 0 && seqNum <= 4294967295L) : "seqNum is out of range (0-4294967296)"; // uint32

        _generalLong[MsgFields.Common.Long.SEQ_NUM] = seqNum;
    }
    
    @Override
    public long seqNum()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_SEQ_NUM) > 0 ? _generalLong[MsgFields.Common.Long.SEQ_NUM] : 0);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_SEQ_NUM) > 0 ? _generalLong[MsgFields.Common.Long.SEQ_NUM] : 0);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_SEQ_NUM) > 0 ? _generalLong[MsgFields.Common.Long.SEQ_NUM] : 0);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_SEQ_NUM) > 0 ? _generalLong[MsgFields.Common.Long.SEQ_NUM] : 0);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_SEQ_NUM) > 0 ? _generalLong[MsgFields.Common.Long.SEQ_NUM] : 0);
            default: // all other messages don't have sequence number
                return 0;
        }
    }
    
    @Override
    public boolean checkAck()
    {
        switch (_msgClass)
        {
            case MsgClasses.CLOSE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & CloseMsgFlags.ACK) > 0 ? true : false);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.ACK) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public boolean checkHasPermData()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_PERM_DATA) > 0 ? true : false);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_PERM_DATA) > 0 ? true : false);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_PERM_DATA) > 0 ? true : false);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_PERM_DATA) > 0 ? true : false);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_PERM_DATA) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public void applyAck()
    {
        switch (_msgClass)
        {
            case MsgClasses.CLOSE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= CloseMsgFlags.ACK;
                break;
            case MsgClasses.REQUEST:
                break;
            case MsgClasses.POST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.ACK;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
    
    @Override
    public void applyHasPermData()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.HAS_PERM_DATA;
                break;
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.HAS_PERM_DATA;
                break;
            case MsgClasses.STATUS:
                _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.HAS_PERM_DATA;
                break;
            case MsgClasses.GENERIC:
                _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.HAS_PERM_DATA;
                break;
            case MsgClasses.POST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.HAS_PERM_DATA;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }

    @Override
    public void partNum(int partNum)
    {
        assert (partNum >= 0 && partNum <= 32767) : "partNum is out of range (0-32767)"; // uint15-rb

        _generalInt[MsgFields.Common.Int.PART_NUM] = partNum;
    }
    
    @Override
    public int partNum()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_PART_NUM) > 0 ? _generalInt[MsgFields.Common.Int.PART_NUM] : 0);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_PART_NUM) > 0 ? _generalInt[MsgFields.Common.Int.PART_NUM] : 0);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_PART_NUM) > 0 ? _generalInt[MsgFields.Common.Int.PART_NUM] : 0);
            default: // all other messages don't have partNum
                return 0;
        }
    }
    
    @Override
    public void permData(Buffer permData)
    {
        assert (permData != null) : "permData must be non-null";

        if (_generalBuffer[MsgFields.Common.Buffer.PERM_DATA] == null)
        {
            _generalBuffer[MsgFields.Common.Buffer.PERM_DATA] = CodecFactory.createBuffer();
        }

        ((BufferImpl)_generalBuffer[MsgFields.Common.Buffer.PERM_DATA]).copyReferences(permData);
    }
    
    @Override
    public Buffer permData()
    {
        if (_generalBuffer[MsgFields.Common.Buffer.PERM_DATA] == null)
        {
            _generalBuffer[MsgFields.Common.Buffer.PERM_DATA] = CodecFactory.createBuffer();
        }

        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_PERM_DATA) > 0 ?
                        _generalBuffer[MsgFields.Common.Buffer.PERM_DATA] : null);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_PERM_DATA) > 0 ?
                        _generalBuffer[MsgFields.Common.Buffer.PERM_DATA] : null);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_PERM_DATA) > 0 ?
                        _generalBuffer[MsgFields.Common.Buffer.PERM_DATA] : null);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_PERM_DATA) > 0 ?
                        _generalBuffer[MsgFields.Common.Buffer.PERM_DATA] : null);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_PERM_DATA) > 0 ?
                        _generalBuffer[MsgFields.Common.Buffer.PERM_DATA] : null);
            default: // all other messages don't have permData
                return null;
        }
    }
    
    @Override
    public void applyHasPartNum()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.HAS_PART_NUM;
                break;
            case MsgClasses.GENERIC:
                _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.HAS_PART_NUM;
                break;
            case MsgClasses.POST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.HAS_PART_NUM;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
    
    @Override
    public boolean checkPrivateStream()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.PRIVATE_STREAM) > 0 ? true : false);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.PRIVATE_STREAM) > 0 ? true : false);
            case MsgClasses.REQUEST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.PRIVATE_STREAM) > 0 ? true : false);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.PRIVATE_STREAM) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public boolean checkQualifiedStream()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.QUALIFIED_STREAM) > 0 ? true : false);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.QUALIFIED_STREAM) > 0 ? true : false);
            case MsgClasses.REQUEST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.QUALIFIED_STREAM) > 0 ? true : false);
            case MsgClasses.ACK:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.QUALIFIED_STREAM) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }

    @Override
    public void groupId(Buffer groupId)
    {
        assert (groupId != null) : "groupId must be non-null";

        if (_generalBuffer[MsgFields.Common.Buffer.GROUP_ID] == null)
        {
            _generalBuffer[MsgFields.Common.Buffer.GROUP_ID] = CodecFactory.createBuffer();
        }

        ((BufferImpl)_generalBuffer[MsgFields.Common.Buffer.GROUP_ID]).copyReferences(groupId);
    }
    
    @Override
    public Buffer groupId()
    {
        if (_generalBuffer[MsgFields.Common.Buffer.GROUP_ID] == null)
        {
            _generalBuffer[MsgFields.Common.Buffer.GROUP_ID] = CodecFactory.createBuffer();
        }

        switch (_msgClass)
        {
            case MsgClasses.REFRESH: // refresh message always has group id
                return _generalBuffer[MsgFields.Common.Buffer.GROUP_ID];
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_GROUP_ID) > 0 ?
                        _generalBuffer[MsgFields.Common.Buffer.GROUP_ID] : null);
            default: // all other messages don't have group id
                return null;
        }
    }
    
    @Override
    public void applyPrivateStream()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.PRIVATE_STREAM;
                break;
            case MsgClasses.STATUS:
                _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.PRIVATE_STREAM;
                break;
            case MsgClasses.REQUEST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.PRIVATE_STREAM;
                break;
            case MsgClasses.ACK:
                _generalInt[MsgFields.Common.Int.FLAGS] |= AckMsgFlags.PRIVATE_STREAM;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
    
    @Override
    public void applyQualifiedStream()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.QUALIFIED_STREAM;
                break;
            case MsgClasses.STATUS:
                _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.QUALIFIED_STREAM;
                break;
            case MsgClasses.REQUEST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.QUALIFIED_STREAM;
                break;
            case MsgClasses.ACK:
                _generalInt[MsgFields.Common.Int.FLAGS] |= AckMsgFlags.QUALIFIED_STREAM;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
    
    @Override
    public boolean checkHasPartNum()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_PART_NUM) > 0 ? true : false);
            case MsgClasses.GENERIC:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_PART_NUM) > 0 ? true : false);
            case MsgClasses.POST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_PART_NUM) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public PostUserInfo postUserInfo()
    {
        switch (_msgClass)
        {
            case MsgClasses.POST: // post message always has postUserInfo
                return _generalPostUserInfo;
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_POST_USER_INFO) > 0 ? _generalPostUserInfo : null);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_POST_USER_INFO) > 0 ? _generalPostUserInfo : null);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_POST_USER_INFO) > 0 ? _generalPostUserInfo : null);
            default: // all other messages don't have postUserInfo
                return null;
        }
    }
    
    @Override
    public boolean checkHasQos()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_QOS) > 0 ? true : false);
            case MsgClasses.REQUEST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_QOS) > 0 ? true : false);
            default:
                assert (false); // message class was not set on this class (or a new class was not implemented)
                return false;
        }
    }
    
    @Override
    public boolean checkClearCache()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.CLEAR_CACHE) > 0 ? true : false);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.CLEAR_CACHE) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public boolean checkDoNotCache()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.DO_NOT_CACHE) > 0 ? true : false);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.DO_NOT_CACHE) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public boolean checkHasPostUserInfo()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_POST_USER_INFO) > 0 ? true : false);
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_POST_USER_INFO) > 0 ? true : false);
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_POST_USER_INFO) > 0 ? true : false);
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    @Override
    public void applyClearCache()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.CLEAR_CACHE;
                break;
            case MsgClasses.STATUS:
                _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.CLEAR_CACHE;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
    
    @Override
    public void applyDoNotCache()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.DO_NOT_CACHE;
                break;
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.DO_NOT_CACHE;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
    
    @Override
    public void applyHasPostUserInfo()
    {
        switch (_msgClass)
        {
            case MsgClasses.UPDATE:
                _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.HAS_POST_USER_INFO;
                break;
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.HAS_POST_USER_INFO;
                break;
            case MsgClasses.STATUS:
                _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.HAS_POST_USER_INFO;
                break;
            default:
                assert (false); // not supported on this message class
                break;
        }
    }
   
    @Override
    public void state(State state)
    {
        assert (state != null) : "state must be non-null";

        if (_generalState == null)
        {
            _generalState = CodecFactory.createState();
        }

        state.copy(_generalState);
    }
    
    @Override
    public State state()
    {
        if (_generalState == null)
        {
            _generalState = CodecFactory.createState();
        }

        switch (_msgClass)
        {
            case MsgClasses.REFRESH: // refresh message always has state
                return _generalState;
            case MsgClasses.STATUS:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_STATE) > 0 ? _generalState : null);
            default: // all other messages don't have state
                return null;
        }
    }
    
    @Override
    public Qos qos()
    {
        if (_generalQos[MsgFields.Common.Qos.QOS] == null)
        {
            _generalQos[MsgFields.Common.Qos.QOS] = CodecFactory.createQos();
        }

        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.HAS_QOS) > 0 ? _generalQos[MsgFields.Common.Qos.QOS] : null);
            case MsgClasses.REQUEST:
                return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_QOS) > 0 ? _generalQos[MsgFields.Common.Qos.QOS] : null);
            default: // all other messages don't have qos
                return null;
        }
    }
    
    ////////// begin AckMsg-specific methods //////////
    
    @Override
    public boolean checkHasText()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_TEXT) > 0 ? true : false);
    }

    @Override
    public boolean checkHasNakCode()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_NAK_CODE) > 0 ? true : false);
    }

    @Override
    public void applyHasText()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= AckMsgFlags.HAS_TEXT;
    }

    @Override
    public void applyHasNakCode()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= AckMsgFlags.HAS_NAK_CODE;
    }
    
    @Override
    public void ackId(long ackId)
    {
        assert (ackId >= 0 && ackId <= 4294967295L) : "ackId is out of range (0-4294967296)"; // uint32

        _generalLong[MsgFields.Ack.Long.ACK_ID] = ackId;
    }

    @Override
    public long ackId()
    {
        return _generalLong[MsgFields.Ack.Long.ACK_ID];
    }

    @Override
    public void nakCode(int nakCode)
    {
        assert (nakCode >= 0 && nakCode <= 255) : "nakCode is out of range (0-255)"; // uint8

        _generalInt[MsgFields.Ack.Int.NAK_CODE] = nakCode;
    }
    
    @Override
    public int nakCode()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_NAK_CODE) > 0 ? _generalInt[MsgFields.Ack.Int.NAK_CODE] : 0);
    }
    
    @Override
    public void text(Buffer text)
    {
        assert (text != null) : "text must be non-null";

        if (_generalBuffer[MsgFields.Ack.Buffer.TEXT] == null)
        {
            _generalBuffer[MsgFields.Ack.Buffer.TEXT] = CodecFactory.createBuffer();
        }

        ((BufferImpl)_generalBuffer[MsgFields.Ack.Buffer.TEXT]).copyReferences(text);
    }
    
    @Override
    public Buffer text()
    {
        if (_generalBuffer[MsgFields.Ack.Buffer.TEXT] == null)
        {
            _generalBuffer[MsgFields.Ack.Buffer.TEXT] = CodecFactory.createBuffer();
        }

        return ((_generalInt[MsgFields.Common.Int.FLAGS] & AckMsgFlags.HAS_TEXT) > 0 ? _generalBuffer[MsgFields.Ack.Buffer.TEXT] : null);
    }
    
    ////////// end AckMsg-specific methods //////////
        
    ////////// begin GenericMsg-specific methods //////////

    @Override
    public boolean checkMessageComplete()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.MESSAGE_COMPLETE) > 0 ? true : false);
    }

    @Override
    public boolean checkHasSecondarySeqNum()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0 ? true : false);
    }

    @Override
    public boolean checkIsProviderDriven() {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.PROVIDER_DRIVEN) > 0 ? true : false);
    }

    @Override
    public void applyMessageComplete()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.MESSAGE_COMPLETE;
    }

    @Override
    public void applyHasSecondarySeqNum()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.HAS_SECONDARY_SEQ_NUM;
    }

    @Override
    public void applyProviderDriven() {
        _generalInt[MsgFields.Common.Int.FLAGS] |= GenericMsgFlags.PROVIDER_DRIVEN;
    }

    @Override
    public void secondarySeqNum(long secondarySeqNum)
    {
        assert (secondarySeqNum >= 0 && secondarySeqNum <= 4294967295L) : "secondarySeqNum is out of range (0-4294967296)"; // uint32

        _generalLong[MsgFields.Generic.Long.SECONDARY_SEQ_NUM] = secondarySeqNum;
    }
    
    @Override
    public long secondarySeqNum()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0 ?
                _generalLong[MsgFields.Generic.Long.SECONDARY_SEQ_NUM] : 0);
    }
        
    ////////// end GenericMsg-specific methods //////////

    ////////// begin PostMsg-specific methods //////////

    @Override
    public boolean checkHasPostId()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_POST_ID) > 0 ? true : false);
    }

    @Override
    public boolean checkPostComplete()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.POST_COMPLETE) > 0 ? true : false);
    }

    @Override
    public boolean checkHasPostUserRights()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_POST_USER_RIGHTS) > 0 ? true : false);
    }

    @Override
    public void applyHasPostId()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.HAS_POST_ID;
    }

    @Override
    public void applyPostComplete()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.POST_COMPLETE;
    }

    @Override
    public void applyHasPostUserRights()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= PostMsgFlags.HAS_POST_USER_RIGHTS;
    }

    @Override
    public void postId(long postId)
    {
        assert (postId >= 0 && postId <= 4294967295L) : "postId is out of range (0-4294967296)"; // uint32

        _generalLong[MsgFields.Post.Long.POST_ID] = postId;
    }
    
    @Override
    public long postId()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_POST_ID) > 0 ? _generalLong[MsgFields.Post.Long.POST_ID] : 0);
    }
       
    @Override
    public void postUserRights(int postUserRights)
    {
        assert (postUserRights >= 0 && postUserRights <= 32767) : "postUserRights is out of range (0-32767)"; // uint15-rb

        _generalInt[MsgFields.Post.Int.POST_USER_RIGHTS] = postUserRights;
    }
    
    @Override
    public int postUserRights()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & PostMsgFlags.HAS_POST_USER_RIGHTS) > 0 ?
                _generalInt[MsgFields.Post.Int.POST_USER_RIGHTS] : 0);
    }
    
    ////////// end PostMsg-specific methods //////////

    ////////// begin RefreshMsg-specific methods //////////
    
    @Override
    public boolean checkSolicited()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.SOLICITED) > 0 ? true : false);
    }

    @Override
    public boolean checkRefreshComplete()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RefreshMsgFlags.REFRESH_COMPLETE) > 0 ? true : false);
    }

    @Override
    public void applySolicited()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.SOLICITED;
    }

    @Override
    public void applyRefreshComplete()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.REFRESH_COMPLETE;
    }
    
    @Override
    public void applyHasQos()
    {
        switch (_msgClass)
        {
            case MsgClasses.REFRESH:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RefreshMsgFlags.HAS_QOS;
                break;
            case MsgClasses.REQUEST:
                _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.HAS_QOS;
                break;
            default:
                assert (false); // message class was not set on this class (or a new class was not implemented)
        }
    }
    
    ////////// end RefreshMsg-specific methods //////////
    
    ////////// begin RequestMsg-specific methods //////////
    
    @Override
    public boolean checkHasPriority()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_PRIORITY) > 0 ? true : false);
    }

    @Override
    public boolean checkStreaming()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.STREAMING) > 0 ? true : false);
    }

    @Override
    public boolean checkMsgKeyInUpdates()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.MSG_KEY_IN_UPDATES) > 0 ? true : false);
    }

    @Override
    public boolean checkConfInfoInUpdates()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.CONF_INFO_IN_UPDATES) > 0 ? true : false);
    }

    @Override
    public boolean checkNoRefresh()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.NO_REFRESH) > 0 ? true : false);
    }
    
    @Override
    public boolean checkHasWorstQos()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_WORST_QOS) > 0 ? true : false);
    }

    @Override
    public boolean checkPause()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.PAUSE) > 0 ? true : false);
    }

    @Override
    public boolean checkHasView()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_VIEW) > 0 ? true : false);
    }

    @Override
    public boolean checkHasBatch()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_BATCH) > 0 ? true : false);
    }
    
    @Override
    public void applyHasPriority()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.HAS_PRIORITY;
    }

    @Override
    public void applyStreaming()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.STREAMING;
    }

    @Override
    public void applyMsgKeyInUpdates()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.MSG_KEY_IN_UPDATES;
    }

    @Override
    public void applyConfInfoInUpdates()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.CONF_INFO_IN_UPDATES;
    }

    @Override
    public void applyNoRefresh()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.NO_REFRESH;
    }
    
    @Override
    public void applyHasWorstQos()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.HAS_WORST_QOS;
    }

    @Override
    public void applyPause()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.PAUSE;
    }

    @Override
    public void applyHasView()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.HAS_VIEW;
    }

    @Override
    public void applyHasBatch()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= RequestMsgFlags.HAS_BATCH;
    }
    
    @Override
    public Priority priority()
    {
        if (_generalPriority == null)
        {
            _generalPriority = new PriorityImpl();
        }

        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_PRIORITY) > 0 ? _generalPriority : null);
    }
    
    @Override
    public Qos worstQos()
    {
        if (_generalQos[MsgFields.Request.Qos.WORST_QOS] == null)
        {
            _generalQos[MsgFields.Request.Qos.WORST_QOS] = CodecFactory.createQos();
        }

        return ((_generalInt[MsgFields.Common.Int.FLAGS] & RequestMsgFlags.HAS_WORST_QOS) > 0 ? _generalQos[MsgFields.Request.Qos.WORST_QOS] : null);
    }
    
    ////////// end RequestMsg-specific methods //////////
    
    ////////// begin StatusMsg-specific methods //////////
    
    @Override
    public boolean checkHasGroupId()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_GROUP_ID) > 0 ? true : false);
    }

    @Override
    public boolean checkHasState()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & StatusMsgFlags.HAS_STATE) > 0 ? true : false);
    }

    @Override
    public void applyHasGroupId()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.HAS_GROUP_ID;
    }

    @Override
    public void applyHasState()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= StatusMsgFlags.HAS_STATE;
    }
    
    ////////// end StatusMsg-specific methods //////////
    
    ////////// begin UpdateMsg-specific methods //////////
    
    @Override
    public boolean checkHasConfInfo()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_CONF_INFO) > 0 ? true : false);
    }

    @Override
    public boolean checkDoNotConflate()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.DO_NOT_CONFLATE) > 0 ? true : false);
    }

    @Override
    public boolean checkDoNotRipple()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.DO_NOT_RIPPLE) > 0 ? true : false);
    }

    @Override
    public boolean checkDiscardable()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.DISCARDABLE) > 0 ? true : false);
    }

    @Override
    public void applyHasConfInfo()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.HAS_CONF_INFO;
    }
    
    @Override
    public void applyDoNotConflate()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.DO_NOT_CONFLATE;
    }

    @Override
    public void applyDoNotRipple()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.DO_NOT_RIPPLE;
    }

    @Override
    public void applyDiscardable()
    {
        _generalInt[MsgFields.Common.Int.FLAGS] |= UpdateMsgFlags.DISCARDABLE;
    }

    @Override
    public void updateType(int updateType)
    {
        assert (updateType >= 0 && updateType <= 255) : "updateType is out of range (0-255)"; // uint8

        _generalInt[MsgFields.Update.Int.UPDATE_TYPE] = updateType;
    }

    @Override
    public int updateType()
    {
        return _generalInt[MsgFields.Update.Int.UPDATE_TYPE];
    }

    @Override
    public void conflationCount(int conflationCount)
    {
        assert (conflationCount >= 0 && conflationCount <= 32767) : "conflationCount is out of range (0-32767)"; // uint15-rb

        _generalInt[MsgFields.Update.Int.CONFLATION_COUNT] = conflationCount;
    }
    
    @Override
    public int conflationCount()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_CONF_INFO) > 0 ? _generalInt[MsgFields.Update.Int.CONFLATION_COUNT] : 0);
    }

    @Override
    public void conflationTime(int conflationTime)
    {
        assert (conflationTime >= 0 && conflationTime <= 65535) : "conflationTime is out of range (0-65535)"; // uint16

        _generalInt[MsgFields.Update.Int.CONFLATION_TIME] = conflationTime;
    }

    @Override
    public int conflationTime()
    {
        return ((_generalInt[MsgFields.Common.Int.FLAGS] & UpdateMsgFlags.HAS_CONF_INFO) > 0 ? _generalInt[MsgFields.Update.Int.CONFLATION_TIME] : 0);
    }
   
    ////////// end UpdateMsg-specific methods //////////

    @Override
    public void clear()
    {
        _msgClass = 0;
        _domainType = 0;
        _containerType = DataTypes.CONTAINER_TYPE_MIN;
        _streamId = 0;
        _msgKey.clear();
        _extendedHeader.clear();
        _encodedDataBody.clear();
        _encodedMsgBuffer.clear();

        _keyReserved = false;
        _extendedHeaderReserved = false;

        // clear the general-purpose fields:
        Arrays.fill(_generalInt, 0);
        Arrays.fill(_generalLong, 0);

        for (Buffer buf : _generalBuffer)
        {
            if (buf != null)
            {
                buf.clear();
            }
        }

        _generalPostUserInfo.clear();

        if (_generalState != null)
        {
            _generalState.clear();
        }

        for (Qos qos : _generalQos)
        {
            if (qos != null)
            {
                qos.clear();
            }
        }

        if (_generalPriority != null)
        {
            _generalPriority.clear();
        }
    }
}
