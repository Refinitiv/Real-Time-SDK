/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
	sealed internal class Decoders
	{
		private static int[] real32LenHints = new int[] {2, 3, 4, 5};
		private static int[] real64LenHints = new int[] {3, 5, 7, 9};
		private static readonly int[] lens = new int[81];
		private const int POST_USER_INFO_SIZE = 8;
		static Decoders()
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

            for (int i = 0; i < 256; i++)
            {
                primitiveTypes[i] = i;
            }

            primitiveTypes[1] = 3;
            primitiveTypes[64] = 3;
            primitiveTypes[66] = 3;
            primitiveTypes[68] = 3;
            primitiveTypes[70] = 3;
            primitiveTypes[2] = 4;
            primitiveTypes[65] = 4;
            primitiveTypes[67] = 4;
            primitiveTypes[69] = 4;
            primitiveTypes[71] = 4;
            primitiveTypes[72] = 5;
            primitiveTypes[73] = 6;
            primitiveTypes[74] = 8;
            primitiveTypes[75] = 8;
            primitiveTypes[7] = 8;
            primitiveTypes[76] = 9;
            primitiveTypes[77] = 10;
            primitiveTypes[78] = 10;
            primitiveTypes[83] = 10;
            primitiveTypes[84] = 10;
            primitiveTypes[79] = 11;
            primitiveTypes[80] = 11;
            primitiveTypes[81] = 11;
            primitiveTypes[82] = 11;

            DecFunctions[DataTypes.INT_1] = Dec8;
            DecFunctions[DataTypes.UINT_1] = Dec8;
            DecFunctions[DataTypes.INT_2] = Dec16;
            DecFunctions[DataTypes.UINT_2] = Dec16;
            DecFunctions[DataTypes.TIME_3] = Dec24;
            DecFunctions[DataTypes.INT_4] = Dec32;
            DecFunctions[DataTypes.UINT_4] = Dec32;
            DecFunctions[DataTypes.FLOAT_4] = Dec32;
            DecFunctions[DataTypes.DATE_4] = Dec32;
            DecFunctions[DataTypes.TIME_5] = Dec40;
            DecFunctions[DataTypes.DATETIME_7] = Dec56;
            DecFunctions[DataTypes.TIME_7] = Dec56;
            DecFunctions[DataTypes.INT_8] = Dec64;
            DecFunctions[DataTypes.UINT_8] = Dec64;
            DecFunctions[DataTypes.DOUBLE_8] = Dec64;
            DecFunctions[DataTypes.TIME_8] = Dec64;
            DecFunctions[DataTypes.DATETIME_9] = Dec72;
            DecFunctions[DataTypes.DATETIME_11] = Dec88;
            DecFunctions[DataTypes.DATETIME_12] = Dec96;
            DecFunctions[DataTypes.ENUM] = DecBuf16;
            DecFunctions[DataTypes.ARRAY] = DecBuf16;
            DecFunctions[DataTypes.BUFFER] = DecBuf16;
            DecFunctions[DataTypes.ASCII_STRING] = DecBuf16;
            DecFunctions[DataTypes.UTF8_STRING] = DecBuf16;
            DecFunctions[DataTypes.RMTES_STRING] = DecBuf16;
            DecFunctions[DataTypes.OPAQUE] = DecBuf16;
            DecFunctions[DataTypes.XML] = DecBuf16;
            DecFunctions[DataTypes.FIELD_LIST] = DecBuf16;
            DecFunctions[DataTypes.ELEMENT_LIST] = DecBuf16;
            DecFunctions[DataTypes.ANSI_PAGE] = DecBuf16;
            DecFunctions[DataTypes.FILTER_LIST] = DecBuf16;
            DecFunctions[DataTypes.VECTOR] = DecBuf16;
            DecFunctions[DataTypes.MAP] = DecBuf16;
            DecFunctions[DataTypes.SERIES] = DecBuf16;
            DecFunctions[DataTypes.MSG] = DecBuf16;
            DecFunctions[DataTypes.INT] = DecBuf8_1;
            DecFunctions[DataTypes.UINT] = DecBuf8_1;
            DecFunctions[DataTypes.FLOAT] = DecBuf8_1;
            DecFunctions[DataTypes.DOUBLE] = DecBuf8_1;
            DecFunctions[DataTypes.REAL] = DecBuf8_1;
            DecFunctions[DataTypes.DATETIME] = DecBuf8_1;
            DecFunctions[DataTypes.TIME] = DecBuf8_1;
            DecFunctions[DataTypes.DATE] = DecBuf8_1;
            DecFunctions[DataTypes.QOS] = DecBuf8_1;
            DecFunctions[DataTypes.REAL_4RB] = DecReal_4rb;
            DecFunctions[DataTypes.REAL_8RB] = DecReal_8rb;

            PrimitiveDecFunctions[DataTypes.UINT] = (iter, data) => DecodeUInt(iter, (UInt)data);
            PrimitiveDecFunctions[DataTypes.INT] = (iter, data) => DecodeInt(iter, (Int)data);
            PrimitiveDecFunctions[DataTypes.FLOAT] = (iter, data) => DecodeFloat(iter, (Float)data);
            PrimitiveDecFunctions[DataTypes.DOUBLE] = (iter, data) => DecodeDouble(iter, (Double)data);
            PrimitiveDecFunctions[DataTypes.REAL] = (iter, data) => DecodeReal(iter, (Real)data);
            PrimitiveDecFunctions[DataTypes.DATE] = (iter, data) => DecodeDate(iter, (Date)data);
            PrimitiveDecFunctions[DataTypes.TIME] = (iter, data) => DecodeTime(iter, (Time)data);
            PrimitiveDecFunctions[DataTypes.DATETIME] = (iter, data) => DecodeDateTime(iter, (DateTime)data);
            PrimitiveDecFunctions[DataTypes.QOS] = (iter, data) => DecodeQos(iter, (Qos)data);
            PrimitiveDecFunctions[DataTypes.STATE] = (iter, data) => DecodeState(iter, (State)data);
            PrimitiveDecFunctions[DataTypes.ENUM] = (iter, data) => DecodeEnum(iter, (Enum)data);
            PrimitiveDecFunctions[DataTypes.BUFFER] = (iter, data) => ((Buffer)data).Data_internal(iter._reader._buffer, iter._curBufPos, iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos);
            PrimitiveDecFunctions[DataTypes.ASCII_STRING] = (iter, data) => ((Buffer)data).Data_internal(iter._reader._buffer, iter._curBufPos, iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos);
            PrimitiveDecFunctions[DataTypes.UTF8_STRING] = (iter, data) => ((Buffer)data).Data_internal(iter._reader._buffer, iter._curBufPos, iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos);
            PrimitiveDecFunctions[DataTypes.RMTES_STRING] = (iter, data) => ((Buffer)data).Data_internal(iter._reader._buffer, iter._curBufPos, iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos);

            sizeTimeDecFuncArray[8] = DecodeTime8;
            sizeTimeDecFuncArray[7] = DecodeTime7;
            sizeTimeDecFuncArray[5] = DecodeTime5;
            sizeTimeDecFuncArray[3] = DecodeTime3;
            sizeTimeDecFuncArray[2] = DecodeTime2;
            sizeTimeDecFuncArray[0] = DecodeTime0;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode DecodePrimitiveType(DecodeIterator iter, int type, object data)
        {
            PrimitiveDecFunctions[type](iter, data);
            return CodecReturnCode.SUCCESS;
        }

        private static Action<DecodeIterator, Buffer>[] DecFunctions = new Action<DecodeIterator, Buffer>[256];

        private static Action<DecodeIterator, object>[] PrimitiveDecFunctions = new Action<DecodeIterator, object>[256];

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode DecodeSet(DecodeIterator iterInt, int type, object data)
        {
            DecFunctions[type](iterInt, (Buffer)data);
            return CodecReturnCode.SUCCESS;
        }

        private static int[] primitiveTypes = new int[256];

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static int ConvertToPrimitiveType1(int type)
        {
            return primitiveTypes[type];
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeMsg(DecodeIterator iter, Msg msg)
        {
            int position;
            DecodingLevel _levelInfo;

            if (++iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            _levelInfo = iter._levelInfo[iter._decodingLevel];
            _levelInfo._containerType = DataTypes.MSG;

            try
            {
                DecodeMsgHeader(iter, msg);
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            /* Maintain this use of the position variable for now, in case there is public use of this method. */
            iter._curBufPos = iter._reader._position;
            position = iter._curBufPos;

            if (position > _levelInfo._endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            var dataBody = msg._encodedDataBody;
            if ((_levelInfo._endBufPos - position) > 0)
            {
                dataBody.Data_internal(iter._reader._buffer, position, _levelInfo._endBufPos - position);

                if (!CanDecodeContainerType(msg.ContainerType))
                {
                    /* ETA has no decoders for this format(e.g. Opaque). Move past it. */
                    iter._curBufPos += dataBody.GetLength();
                    
                    iter._reader.SkipBytes(dataBody.GetLength());
                    if (iter._reader._position > iter._reader._buffer.BufferLimit())
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                    
                    iter._curBufPos = iter._reader._position;
                    EndOfList(iter);
                    return CodecReturnCode.SUCCESS;
                }

                /* For now, _endBufPos for msg and data levels is the same. */
                iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._endBufPos;

                return CodecReturnCode.SUCCESS;
            }
            else
            {
                /* No payload. Reset iterator and return. */
                dataBody.Clear();
                EndOfList(iter);
                return CodecReturnCode.SUCCESS;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeMsgHeader(DecodeIterator iter, Msg msg)
        {
            int startPosition = iter._curBufPos;
            int headerSize;
            int keySize;
            byte* pointer = iter._reader._buffer._pointer;
            BufferReader reader = iter._reader;

            reader._position = startPosition;

            /* header size */
            headerSize = reader.ReadUnsignedShort();

            msg.MsgClass = pointer[iter._reader._position++];

            /* mask msgClass top three bits for reserved use later */
            /* Top three bits reserved for later use */
            msg.MsgClass = msg.MsgClass & 0x1F;

            msg.DomainType = pointer[iter._reader._position++];
            msg.StreamId = reader.ReadInt();
            iter._curBufPos = reader._position; // adjust the iterator's position to match the reader's position.

            msg.SetEncodedMsgBuffer(iter._buffer.Data(), startPosition, iter._buffer.GetLength());
            
            /* IMPORTANT: When new message classes are added, CopyMsg and ValidateMsg have to be modified as well */

            switch (msg.MsgClass)
            {
                case MsgClasses.UPDATE:
                    int updateFlags = iter._reader.ReadUShort15rb();
                    msg.Flags = updateFlags;

                    // need to scale containerType
                    msg.ContainerType = pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;

                    msg.UpdateType = pointer[reader._position++];

                    if ((updateFlags & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                    {
                        msg.SeqNum = reader.ReadUnsignedInt();
                    }

                    if ((updateFlags & UpdateMsgFlags.HAS_CONF_INFO) > 0)
                    {
                        msg.ConflationCount = reader.ReadUShort15rb();
                        msg.ConflationTime = reader.ReadUnsignedShort();
                    }

                    if ((updateFlags & UpdateMsgFlags.HAS_PERM_DATA) > 0)
                    {
                        DecodeBuffer15(iter, msg.PermData);
                    }

                    if ((updateFlags & UpdateMsgFlags.HAS_MSG_KEY) > 0)
                    {
                        keySize = reader.ReadUShort15rb();

                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = reader._position;
                        DecodeBaseKey(iter, msg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        reader._position = currentPosition;
                    }

                    if ((updateFlags & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
                    {
                        DecodeBuffer8(iter, msg._extendedHeader);
                    }

                    if ((updateFlags & UpdateMsgFlags.HAS_POST_USER_INFO) > 0)
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (reader._position - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                        {
                            /* get user address */
                            msg.PostUserInfo.UserAddr = iter._reader.ReadUnsignedInt();
                            /* get user ID */
                            msg.PostUserInfo.UserId = iter._reader.ReadUnsignedInt();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            msg.Flags &= ~UpdateMsgFlags.HAS_POST_USER_INFO;
                        }
                    }
                    break;
                case MsgClasses.REFRESH:
                    int refreshFlags = iter._reader.ReadUShort15rb();
                    msg.Flags = refreshFlags;
                    
                    // need to scale containerType
                    msg.ContainerType = pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;

                    if ((refreshFlags & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                    {
                        msg.SeqNum = iter._reader.ReadUnsignedInt();
                    }

                    DecodeStateInMsg(iter, msg.State);
                    DecodeBuffer8(iter, msg.GroupId);

                    if ((refreshFlags & RefreshMsgFlags.HAS_PERM_DATA) > 0)
                    {
                        DecodeBuffer15(iter, msg.PermData);
                    }

                    if ((refreshFlags & RefreshMsgFlags.HAS_QOS) > 0)
                    {
                        DecodeQosInMsg(iter, msg.Qos);
                    }

                    if ((refreshFlags & RefreshMsgFlags.HAS_MSG_KEY) > 0)
                    {
                        keySize = reader.ReadUShort15rb();
                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = reader._position;
                        DecodeBaseKey(iter, msg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        reader._position = currentPosition;
                    }

                    if ((refreshFlags & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
                    {
                        DecodeBuffer8(iter, msg._extendedHeader);
                    }

                    if ((refreshFlags & RefreshMsgFlags.HAS_POST_USER_INFO) > 0)
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (reader._position - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                        {
                            /* get user address */
                            msg.PostUserInfo.UserAddr = reader.ReadUnsignedInt();
                            /* get user ID */
                            msg.PostUserInfo.UserId = reader.ReadUnsignedInt();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            msg.Flags &= ~RefreshMsgFlags.HAS_POST_USER_INFO;
                        }
                    }

                    if ((refreshFlags & RefreshMsgFlags.HAS_PART_NUM) > 0)
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (reader._position - (startPosition + 2))) > 0)
                        {
                            msg.PartNum = reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            msg.Flags &= ~RefreshMsgFlags.HAS_PART_NUM;
                        }
                    }
                    break;
                case MsgClasses.REQUEST:
                    DecodeRequestMsgHeader(iter, msg);
                    break;
                case MsgClasses.STATUS:

                    msg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    msg.ContainerType = pointer[iter._reader._position++] + DataTypes.CONTAINER_TYPE_MIN;

                    if (msg.CheckHasState())
                    {
                        DecodeStateInMsg(iter, msg.State);
                    }

                    if (msg.CheckHasGroupId())
                    {
                        DecodeBuffer8(iter, msg.GroupId);
                    }

                    if (msg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, msg.PermData);
                    }

                    if (msg.CheckHasMsgKey())
                    {
                        keySize = reader.ReadUShort15rb();
                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = reader._position;
                        DecodeBaseKey(iter, msg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        reader._position = currentPosition;
                    }

                    if (msg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, msg.ExtendedHeader);
                    }

                    if (msg.CheckHasPostUserInfo())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (reader._position - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                        {
                            /* get user address */
                            msg.PostUserInfo.UserAddr = reader.ReadUnsignedInt();
                            /* get user ID */
                            msg.PostUserInfo.UserId = reader.ReadUnsignedInt();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            msg.Flags &= ~StatusMsgFlags.HAS_POST_USER_INFO;
                        }
                    }
                    break;
                case MsgClasses.CLOSE:
                    DecodeCloseMsgHeader(iter, msg);
                    break;
                case MsgClasses.ACK:
                    msg.Flags = reader.ReadUShort15rb();
                    // need to scale containerType
                    msg.ContainerType = pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;
                    msg.AckId = reader.ReadUnsignedInt();

                    if (msg.CheckHasNakCode())
                    {
                        msg.NakCode = pointer[reader._position++];
                    }

                    if (msg.CheckHasText())
                    {
                        DecodeBuffer16(iter, msg.Text);
                    }

                    if (msg.CheckHasSeqNum())
                    {
                        msg.SeqNum = reader.ReadUnsignedInt();
                    }

                    if (msg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();
                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = reader._position;
                        DecodeBaseKey(iter, msg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        reader._position = currentPosition;
                    }

                    if (msg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, msg.ExtendedHeader);
                    }
                    break;
                case MsgClasses.GENERIC:
                    msg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    msg.ContainerType = pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;

                    if (msg.CheckHasSeqNum())
                    {
                        msg.SeqNum = reader.ReadUnsignedInt();
                    }

                    if (msg.CheckHasSecondarySeqNum())
                    {
                        msg.SecondarySeqNum = reader.ReadUnsignedInt();
                    }

                    if (msg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, msg.PermData);
                    }

                    if (msg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();

                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = reader._position;
                        DecodeBaseKey(iter, msg.MsgKey);

                        /* add keySize to position */
                        currentPosition += keySize;
                        reader._position = currentPosition;
                    }

                    if (msg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, msg.ExtendedHeader);
                    }

                    if (msg.CheckHasPartNum())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (reader._position - (startPosition + 2))) > 0)
                        {
                            msg.PartNum = iter._reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int val = msg.Flags;
                            val &= ~GenericMsgFlags.HAS_PART_NUM;
                            msg.Flags = val;
                        }
                    }
                    break;
                case MsgClasses.POST:
                    msg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    msg.ContainerType = pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;

                    /* get user address */
                    msg.PostUserInfo.UserAddr = reader.ReadUnsignedInt();
                    /* get user ID */
                    msg.PostUserInfo.UserId = reader.ReadUnsignedInt();

                    if (msg.CheckHasSeqNum())
                    {
                        msg.SeqNum = reader.ReadUnsignedInt();
                    }

                    if (msg.CheckHasPostId())
                    {
                        msg.PostId = reader.ReadUnsignedInt();
                    }

                    if (msg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, msg.PermData);
                    }

                    if (msg.CheckHasMsgKey())
                    {
                        keySize = reader.ReadUShort15rb();

                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = reader._position;
                        DecodeBaseKey(iter, msg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        reader._position = currentPosition;
                    }

                    if (msg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, msg.ExtendedHeader);
                    }

                    if (msg.CheckHasPartNum())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (reader._position - (startPosition + 2))) > 0)
                        {
                            msg.PartNum = reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = msg.Flags;
                            flags &= ~PostMsgFlags.HAS_PART_NUM;
                            msg.Flags = flags;
                        }
                    }

                    if (msg.CheckHasPostUserRights())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (reader._position - (startPosition + 2))) > 0)
                        {
                            msg.PostUserRights = reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = msg.Flags;
                            flags &= ~PostMsgFlags.HAS_POST_USER_RIGHTS;
                            msg.Flags = flags;
                        }
                    }
                    break;
                default:
                    throw new System.NotSupportedException("Unknown msgClass: " + msg.MsgClass);
            }

            /* move to end of header */
            reader._position = headerSize + 2 + startPosition;
            iter._curBufPos = reader._position;
        }

        /* Decodes the RequestMsg header.
		 * Returns CodecReturnCode.SUCCESS if the message was successfully decoded.
		 * 
		 * iter is the iterator that decodes the message
		 * requestMsg is the message to decode
		 * 
		 * Returns CodecReturnCode.SUCCESS if the message was successfully decoded.
		 */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeRequestMsgHeader(DecodeIterator iter, IRequestMsg requestMsg)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            requestMsg.Flags = reader.ReadUShort15rb();
            // need to scale containerType
            requestMsg.ContainerType = pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;

            if (requestMsg.CheckHasPriority())
            {
                requestMsg.Priority.PriorityClass = pointer[iter._reader._position++];
                requestMsg.Priority.Count = reader.ReadUShort16ob();
            }

            // note: at present, the code that parses QOS will never return a non-success value
            DecodeRequestMsgHeaderQos(iter, requestMsg);

            int keySize = reader.ReadUShort15rb();
            // don't iterate position by this value anymore. We want to add the keySize to position
            int currentPosition = reader._position;
            DecodeBaseKey(iter, requestMsg.MsgKey);

            // add keySize to position
            currentPosition += keySize;
            reader._position = currentPosition;

            if (requestMsg.CheckHasExtendedHdr())
            {
                DecodeBuffer8(iter, requestMsg.ExtendedHeader);
            }
        }

        /* Decodes the (optional) "QOS" and "Worst QOS" entries in an RequestMsg header.
		 * Returns CodecReturnCode.SUCCESS if the (optional) "QOS"
		 * and "Worst QOS" entries in a RequestMsg header were successfully decoded.
		 * 
		 * iter is the iterator that decodes the message
		 * requestMsg is the message to decode
		 * 
		 * Returns CodecReturnCode.SUCCESS if the (optional) "QOS"
		 *         and "Worst QOS" entries in an RequestMsg header were successfully decoded.
		 */

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void DecodeRequestMsgHeaderQos(DecodeIterator iter, IRequestMsg requestMsg)
        {
            if (requestMsg.CheckHasQos())
            {
                // note: at present, the code that parses QOS will never return a non-success value
                DecodeQosInMsg(iter, requestMsg.Qos);
            }

            if (requestMsg.CheckHasWorstQos())
            {
                // note: at present, the code that parses QOS will never return a non-success value
                DecodeQosInMsg(iter, requestMsg.WorstQos);
            }
        }

        /* Decodes the CloseMsg header.
		 * Returns CodecReturnCode.SUCCESS if the message was successfully decoded.
		 * 
		 * iter is the iterator that decodes the message
		 * closeMsg is the message to decode
		 * 
		 * Returns CodecReturnCode.SUCCESS if the message was successfully decoded.
		 */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeCloseMsgHeader(DecodeIterator iter, ICloseMsg closeMsg)
        {
            BufferReader reader = iter._reader;
            closeMsg.Flags = reader.ReadUShort15rb();
            // need to scale containerType
            closeMsg.ContainerType = reader._buffer._pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;

            if (closeMsg.CheckHasExtendedHdr())
            {
                DecodeBuffer8(iter, closeMsg.ExtendedHeader);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeStateInMsg(DecodeIterator iter, State state)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            byte stateVal = pointer[reader._position++];

            /* only take lowest three bits */
            state.DataState(stateVal & 0x7);

            /* now shift and take top five bits */
            state.StreamState(stateVal >> 3);

            state.Code(pointer[reader._position++]);
            DecodeBuffer15(iter, state.Text());
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeState(DecodeIterator iter, State value)
        {
            byte* pointer;
            BufferReader reader = iter._reader;

            CodecReturnCode retVal = CodecReturnCode.SUCCESS;
            int stateVal;

            if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) >= 3)
            {
                int savePosition = reader._position;
                try
                {
                    pointer = reader._buffer._pointer;
                    reader._position = iter._curBufPos;
                    stateVal = pointer[reader._position++];
                    value.Code(pointer[reader._position++]);
                    DecodeBuffer15(iter, value.Text());
                    value.StreamState(stateVal >> 3);
                    value.DataState(stateVal & 0x7);
                    reader._position = savePosition;
                }
                catch (Exception)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }
            }
            else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
            {
                value.Blank();
                retVal = CodecReturnCode.BLANK_DATA;
            }
            else
            {
                retVal = CodecReturnCode.INCOMPLETE_DATA;
            }
            return retVal;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeBuffer(DecodeIterator iter, Buffer value)
		{
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;

			Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(null != value, "Invalid parameters or parameters passed in as NULL");

			int len = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;

			if (len == 0)
			{
				value.Blank();
				retVal = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				value.Data_internal(iter._buffer.Data(), iter._curBufPos, len);
			}

			return retVal;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeBuffer8(DecodeIterator iter, Buffer buf)
		{
            BufferReader reader = iter._reader;
            int len = reader._buffer._pointer[reader._position++];

			buf.Data_internal(reader._buffer, reader._position, len);
			reader.SkipBytes(len);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void DecodeBuffer15(DecodeIterator iter, Buffer buffer)
		{
            BufferReader reader = iter._reader;

            int len = reader.ReadUShort15rb();

			buffer.Data_internal(reader._buffer, reader._position, len);
			reader.SkipBytes(len);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeBuffer16(DecodeIterator iter, Buffer buf)
		{
            BufferReader reader = iter._reader;

            int temp = reader.ReadUShort16ob();

			buf.Data_internal(reader._buffer, reader._position, temp);
			reader.SkipBytes(temp);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeKeyEncAttrib(DecodeIterator iter, Buffer buffer)
		{
            BufferReader reader = iter._reader;

            int tlen = reader.ReadUShort15rb();
			buffer.Data_internal(reader._buffer, reader._position, tlen);
			reader.SkipBytes(tlen);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeQosInMsg(DecodeIterator iter, Qos qos)
		{
            BufferReader reader = iter._reader;

			sbyte qosValue = (sbyte)reader._buffer._pointer[reader._position++];

            qos.Timeliness(qosValue >> 5);
			qos.Rate((qosValue >> 1) & 0xF);
			qos.IsDynamic = (qosValue & 0x1) > 0 ? true : false;

			if (qos._timeliness > QosTimeliness.DELAYED_UNKNOWN)
			{
				qos.TimeInfo(reader.ReadUnsignedShort());
			}
			else
			{
				qos._timeInfo = 0;
			}

			if (qos._rate > QosRates.JIT_CONFLATED)
			{
				qos.RateInfo(reader.ReadUnsignedShort());
			}
			else
			{
				qos._rateInfo = 0;
			}
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeQos(DecodeIterator iter, Qos value)
		{
            byte* pointer;
            BufferReader reader = iter._reader;

            CodecReturnCode retVal = CodecReturnCode.SUCCESS;
			int qosValue;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) >= 1)
			{
				int savePosition = reader._position;
				try
				{
					pointer = reader._buffer._pointer;
					reader._position = iter._curBufPos;
					qosValue = pointer[reader._position++];
					value.Timeliness(qosValue >> 5);
					value.Rate((qosValue >> 1) & 0xF);
					value.IsDynamic = (qosValue & 0x1) > 0 ? true : false;
					if (value._timeliness > QosTimeliness.DELAYED_UNKNOWN)
					{
						value.TimeInfo(reader.ReadUnsignedShort());
					}
					else
					{
						value._timeInfo = 0;
					}
					if (value._rate > QosRates.JIT_CONFLATED)
					{
						value.RateInfo(reader.ReadUnsignedShort());
					}
					else
					{
						value._rateInfo = 0;
					}
					reader._position = savePosition;
				}
				catch (Exception)
				{
					return CodecReturnCode.INCOMPLETE_DATA;
				}
			}
			else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
			{
				value.Blank();
				retVal = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				retVal = CodecReturnCode.INCOMPLETE_DATA;
			}
			return retVal;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecodeBaseKey(DecodeIterator iter, MsgKey key)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            int keyFlags = reader.ReadUShort15rb();
            key.Flags = keyFlags;

            if ((keyFlags & MsgKeyFlags.HAS_SERVICE_ID) > 0)
            {
                key.ServiceId = reader.ReadUShort16ob();
            }

            if ((keyFlags & MsgKeyFlags.HAS_NAME) > 0)
            {
                /* take name off wire */
                DecodeBuffer8(iter, key.Name);

                /* name type is only present if name is there */
                if ((keyFlags & MsgKeyFlags.HAS_NAME_TYPE) > 0)
                {
                    key.NameType = pointer[reader._position++];
                }
            }

            if ((keyFlags & MsgKeyFlags.HAS_FILTER) > 0)
            {
                key.Filter = reader.ReadUnsignedInt();
            }

            if ((keyFlags & MsgKeyFlags.HAS_IDENTIFIER) > 0)
            {
                key.Identifier = reader._buffer.ReadIntAt(reader._position);
                reader._position += 4;
            }

            if ((keyFlags & MsgKeyFlags.HAS_ATTRIB) > 0)
            {
                /* container type needs to be scaled back up */
                key.AttribContainerType = pointer[reader._position++] + DataTypes.CONTAINER_TYPE_MIN;
                /* size is now an RB15 */
                if (key.AttribContainerType != DataTypes.NO_DATA)
                {
                    DecodeKeyEncAttrib(iter, key.EncodedAttrib);
                }
                else
                {
                    if (key.EncodedAttrib != null)
                    {
                        key.EncodedAttrib.Clear();
                    }
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static bool CanDecodeContainerType(int containerType)
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
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static void EndOfList(DecodeIterator iter)
		{
			/* Go back to previous level and check its type */
			DecodingLevel _levelInfo;

			while (--iter._decodingLevel >= 0)
			{
				_levelInfo = iter._levelInfo[iter._decodingLevel];

				switch (_levelInfo._containerType)
				{
					case DataTypes.MSG: // This was contained by a message
						break; // Keep unwinding (in case the message was contained by another message)

					case DataTypes.NO_DATA: // Finished decoding a 'temporary' container. Just undo the changes.
						iter._curBufPos = _levelInfo._nextEntryPos;
						--iter._decodingLevel;
						return; // STOP

					default: // Inside an RWF container
						return; // STOP
				}
			}
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeMsgKeyAttrib(DecodeIterator iter, IMsgKey key)
        {
            /* Create a level to save the current iterator position.
			 * Once the decode of the opaque is finished, the iterator will be returned to this position. */
            DecodingLevel _levelInfo;

            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(null != key, "Invalid parameters or parameters passed in as NULL");

            if (!key.CheckHasAttrib())
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }
            ++iter._decodingLevel;
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS - 1)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }

            _levelInfo = iter._levelInfo[iter._decodingLevel];

            /* Set type to NO_DATA. When an entry decoder finishes and sees this,
			 * they will know that this was only a 'temporary' container and reset
			 * curBufPos (actual rwf container decoders set this to their appropriate type). */
            _levelInfo._containerType = DataTypes.NO_DATA;

            /* Save iterator position */
            _levelInfo._nextEntryPos = iter._curBufPos;

            /* Setup iterator to decode opaque. */
            iter._curBufPos = key.EncodedAttrib.Position;
            try
            {
                iter._reader._position = iter._curBufPos;
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
            iter._levelInfo[iter._decodingLevel + 1]._endBufPos = key.EncodedAttrib.Position + key.EncodedAttrib.GetLength();

            /* Leave the current _levelInfo's _endBufPos where it is.
			 * It's not used in the opaque decoding, and it needs to be pointing to the correct position after the reset. */

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeElementList(DecodeIterator iter, ElementList elementList, LocalElementSetDefDb localSetDb)
        {
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");

            BufferReader reader = iter._reader;
            byte* pointer;

            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.ELEMENT_LIST, elementList);

            position = iter._curBufPos;
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = position;
                _endBufPos = _levelInfo._endBufPos;

                if (_endBufPos - position == 0)
                {
                    EndOfList(iter);
                    return CodecReturnCode.NO_DATA;
                }

                /* get flags */
                elementList.Flags = (ElementListFlags)pointer[reader._position++];

                /* get element list information */
                if (elementList.CheckHasInfo())
                {
                    int infoLen;
                    int startpos;

                    /* has 1 byte length */
                    infoLen = pointer[reader._position++];
                    startpos = reader._position;

                    if ((startpos + infoLen) > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    elementList.ElementListNum = reader.ReadUnsignedShort();

                    /* used info length to skip */
                    position = startpos + infoLen;
                    reader._position = position;
                }

                /* get set data */
                if (elementList.CheckHasSetData())
                {
                    /* get set id */
                    if (elementList.CheckHasSetId())
                    {
                        elementList.SetId = reader.ReadUShort15rb();
                    }
                    else
                    {
                        elementList.SetId = 0;
                    }
                    position = reader._position;

                    /* set definition from the local or global set list */
                    if (elementList.SetId <= LocalElementSetDefDb.MAX_LOCAL_ID)
                    {
                        if (localSetDb != null && (localSetDb.Definitions[elementList.SetId].SetId != LocalElementSetDefDb.BLANK_ID))
                        {
                            _levelInfo._elemListSetDef = localSetDb.Definitions[elementList.SetId];
                        }
                        else
                        {
                            _levelInfo._elemListSetDef = null;
                        }
                    }
                    else
                    {
                        if (iter._elementSetDefDb != null && (iter._elementSetDefDb.Definitions[elementList.SetId].SetId != GlobalFieldSetDefDb.BLANK_ID))
                        {
                            _levelInfo._elemListSetDef = (ElementSetDef)iter._elementSetDefDb.Definitions[elementList.SetId];
                        }
                        else
                        {
                            _levelInfo._elemListSetDef = null;
                        }
                    }

                    /* check for element list data */
                    if (elementList.CheckHasStandardData())
                    {
                        /* if hassetdata and HasFieldList then set data is length specified */
                        DecodeBuffer15(iter, elementList._encodedSetData);

                        /* get element list data */
                        _levelInfo._itemCount = reader.ReadUnsignedShort();
                        position = reader._position;
                        elementList._encodedEntries.Data_internal(reader._buffer, position, _endBufPos - position);
                        /* check for buffer overflow - post check ok since no copy */
                        if (position > _endBufPos)
                        {
                            return CodecReturnCode.INCOMPLETE_DATA;
                        }
                    }
                    else
                    {
                        /* get the element list set data - not length specified since no field list data exists */
                        elementList._encodedEntries.Clear();

                        elementList._encodedSetData.Data_internal(reader._buffer, position, _endBufPos - position);
                        if (position > _endBufPos)
                        {
                            return CodecReturnCode.INCOMPLETE_DATA;
                        }
                    }

                    /* Setup to decode the set if able. Otherwise skip to entries. */
                    if (_levelInfo._elemListSetDef != null)
                    {
                        _levelInfo._setCount = _levelInfo._elemListSetDef.Count;
                        _levelInfo._itemCount += _levelInfo._elemListSetDef.Count;

                        _levelInfo._nextEntryPos = (_levelInfo._setCount > 0) ? elementList._encodedSetData.Position : position; // iter._curBufPos =

                        return CodecReturnCode.SUCCESS;
                    }
                    else
                    {
                        _levelInfo._setCount = 0;
                        _levelInfo._nextEntryPos = position + elementList._encodedSetData.GetLength();
                        return CodecReturnCode.SET_SKIPPED;
                    }
                }
                else if (elementList.CheckHasStandardData())
                {
                    /* get element list data only */

                    _levelInfo._itemCount = reader.ReadUnsignedShort();

                    elementList._encodedEntries.Data_internal(reader._buffer, reader._position, _endBufPos - reader._position);
                    iter._curBufPos = reader._position;
                    _levelInfo._setCount = 0;
                }
                else
                {
                    elementList._encodedSetData.Clear();
                    elementList._encodedEntries.Clear();
                    _levelInfo._itemCount = 0;
                    iter._curBufPos = 0;
                    _levelInfo._setCount = 0;
                }

                _levelInfo._nextEntryPos = iter._curBufPos;
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeElementEntry(DecodeIterator iter, ElementEntry element)
        {
            int position;
            ElementList elementList;

            Debug.Assert(null != element && null != iter, "Invalid parameters or parameters passed in as NULL");

            BufferReader reader = iter._reader;
            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

            elementList = (ElementList)_levelInfo._listType;

            if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
            {
                EndOfList(iter);
                return CodecReturnCode.END_OF_CONTAINER;
            }

            /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
            position = iter._curBufPos = _levelInfo._nextEntryPos;

            try
            {
                reader._position = position;
                if (_levelInfo._nextSetPosition < _levelInfo._setCount)
                {
                    ElementSetDefEntry encoding = null;
                    Debug.Assert(null != _levelInfo._elemListSetDef, "Invalid parameters or parameters passed in as NULL");
                    Debug.Assert(_levelInfo._elemListSetDef.Count == _levelInfo._setCount, "Invalid parameters or parameters passed in as NULL");

                    encoding = _levelInfo._elemListSetDef.Entries[_levelInfo._nextSetPosition];

                    element._name.CopyReferences(encoding._name);
                    element.DataType = encoding.DataType;

                    /* get the set data and reset position */
                    DecFunctions[encoding.DataType](iter, element._encodedData);

                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._nextEntryPos;

                    /* handle legacy conversion */
                    element.DataType = primitiveTypes[element.DataType];

                    _levelInfo._nextItemPosition++;
                    _levelInfo._nextSetPosition++;

                    if (_levelInfo._nextSetPosition == _levelInfo._setCount && elementList._encodedEntries.GetLength() > 0)
                    {
                        _levelInfo._nextEntryPos = elementList._encodedEntries.Position;
                    }

                    return CodecReturnCode.SUCCESS;
                }

                /* get normal element list data */
                if ((elementList._encodedEntries).Position + elementList._encodedEntries.GetLength() - position < 3)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                DecodeBuffer15(iter, element._name);
                element.DataType = reader._buffer._pointer[reader._position++];

                if (element.DataType != DataTypes.NO_DATA)
                {
                    DecodeBuffer16(iter, element._encodedData);

                    if (reader._position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* handle legacy conversion */
                    element.DataType = primitiveTypes[element.DataType];

                    /* shift iterator */
                    iter._curBufPos = element._encodedData.Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = reader._position;
                    _levelInfo._nextItemPosition++;
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    element._encodedData.Clear();
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = reader._position;
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeUInt(DecodeIterator iter, UInt value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            BufferReader reader = iter._reader;

            if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
			{
				int savePosition = reader._position;
				try
				{
					reader._position = iter._curBufPos;
					int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
					value.Value(reader.ReadULong64ls(size));
					reader._position = savePosition;
				}
				catch (Exception)
				{
					return CodecReturnCode.INVALID_ARGUMENT;
				}
			}
			else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
			{
				value.Blank();
				ret = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				ret = CodecReturnCode.INCOMPLETE_DATA;
			}
			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeInt(DecodeIterator iter, Int value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            BufferReader reader = iter._reader;

            if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
			{
				int savePosition = reader._position;
				try
				{
					reader._position = iter._curBufPos;
					int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
                    value.Value(reader.ReadLong64ls(size));
					reader._position = savePosition;
				}
				catch (Exception)
				{
					return CodecReturnCode.INVALID_ARGUMENT;
				}
			}
			else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
			{
				value.Blank();
				ret = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				ret = CodecReturnCode.INCOMPLETE_DATA;
			}
			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeEnum(DecodeIterator iter, Enum value)
		{
            BufferReader reader = iter._reader;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            int endBufPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos;
            int curBufPos = iter._curBufPos;

            if ((endBufPos - curBufPos) > 0)
			{
				int savePosition = reader._position;
				try
				{
					reader._position = curBufPos;
					int size = endBufPos - curBufPos;
					value.Value(reader.ReadUInt16ls(size));
					reader._position = savePosition;
				}
				catch (Exception)
				{
					return CodecReturnCode.INVALID_ARGUMENT;
				}
			}
			else if ((endBufPos - curBufPos) == 0)
			{
				value.Blank();
				ret = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				ret = CodecReturnCode.INCOMPLETE_DATA;
			}

			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeDate(DecodeIterator iter, Date value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            int endBufPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos;
            int curBufPos = iter._curBufPos;

            BufferReader reader = iter._reader;
            byte* pointer;

            if ((endBufPos - curBufPos) == 4)
			{
				int savePosition = reader._position;
				try
				{
					pointer = reader._buffer._pointer;
					reader._position = curBufPos;
					value.Day(pointer[reader._position++]);
					value.Month(pointer[reader._position++]);
					value.Year(reader.ReadUnsignedShort());
					reader._position = savePosition;
					if (value.IsBlank)
					{
						ret = CodecReturnCode.BLANK_DATA;
					}
				}
				catch (Exception)
				{
					return CodecReturnCode.INCOMPLETE_DATA;
				}
			}
			else if ((endBufPos - curBufPos) == 0)
			{
				value.Blank();
				ret = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				ret = CodecReturnCode.INCOMPLETE_DATA;
			}

			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static CodecReturnCode DecodeTime8(DecodeIterator iter, Time value)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            int savePosition = reader._position;
            reader._position = iter._curBufPos;
            value.Hour(pointer[reader._position++]);
            value.Minute(pointer[reader._position++]);
            value.Second(pointer[reader._position++]);
            value.Millisecond(reader._buffer.ReadUShortAt(reader._position));
            reader._position += 2;
            int tempMicro = reader._buffer.ReadUShortAt(reader._position);
            reader._position += 2;
            int tempNano = pointer[reader._position++];
            value.Microsecond(tempMicro & 0x000007FF);
            value.Nanosecond(((tempMicro & 0x00003800) >> 3) + tempNano);

            reader._position = savePosition;

            if (value._hour == Time.BLANK_HOUR
                && value._minute == Time.BLANK_MINUTE
                && value._second == Time.BLANK_SECOND
                && value._millisecond == Time.BLANK_MILLI
                && value._microsecond == Time.BLANK_MICRO_NANO
                && value._nanosecond == Time.BLANK_MICRO_NANO)
            {
                return CodecReturnCode.BLANK_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static CodecReturnCode DecodeTime7(DecodeIterator iter, Time value)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            int savePosition = reader._position;
            reader._position = iter._curBufPos;
            value.Hour(pointer[reader._position++]);
            value.Minute(pointer[reader._position++]);
            value.Second(pointer[reader._position++]);            
            value.Millisecond(reader._buffer.ReadUShortAt(reader._position));
            reader._position += 2;
            value.Microsecond(reader._buffer.ReadUShortAt(reader._position));
            reader._position += 2;
            value._nanosecond = 0;
            reader._position = savePosition;

            if (value._hour == Time.BLANK_HOUR
                && value._minute == Time.BLANK_MINUTE
                && value._second == Time.BLANK_SECOND
                && value._millisecond == Time.BLANK_MILLI
                && value._microsecond == Time.BLANK_MICRO_NANO)
            {
                value._nanosecond = Time.BLANK_MICRO_NANO;
                return CodecReturnCode.BLANK_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static CodecReturnCode DecodeTime5(DecodeIterator iter, Time value)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            int savePosition = reader._position;
            reader._position = iter._curBufPos;
            value.Hour(pointer[reader._position++]);
            value.Minute(pointer[reader._position++]);
            value.Second(pointer[reader._position++]);
            value.Millisecond(reader.ReadUnsignedShort());
            value._microsecond = 0;
            value._nanosecond = 0;
            reader._position = savePosition;

            if (value._hour == Time.BLANK_HOUR
                && value._minute == Time.BLANK_MINUTE
                && value._second == Time.BLANK_SECOND
                && value._millisecond == Time.BLANK_MILLI)
            {
                value._microsecond = Time.BLANK_MICRO_NANO;
                value._nanosecond = Time.BLANK_MICRO_NANO;
                return CodecReturnCode.BLANK_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static CodecReturnCode DecodeTime3(DecodeIterator iter, Time value)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            int savePosition = reader._position;
            reader._position = iter._curBufPos;
            value.Clear();
            value.Hour(pointer[reader._position++]);
            value.Minute(pointer[reader._position++]);
            value.Second(pointer[reader._position++]);
            reader._position = savePosition;

            if (value._hour == Time.BLANK_HOUR
                && value._minute == Time.BLANK_MINUTE
                && value._second == Time.BLANK_SECOND)
            {
                value._millisecond = Time.BLANK_MILLI;
                value._microsecond = Time.BLANK_MICRO_NANO;
                value._nanosecond = Time.BLANK_MICRO_NANO;
                return CodecReturnCode.BLANK_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static CodecReturnCode DecodeTime2(DecodeIterator iter, Time value)
        {
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            int savePosition = reader._position;
            reader._position = iter._curBufPos;
            value.Clear();
            value.Hour(pointer[reader._position++]);
            value.Minute(pointer[reader._position++]);
            reader._position = savePosition;
            if (value._hour == Time.BLANK_HOUR && value._minute == Time.BLANK_MINUTE)
            {
                value._second = Time.BLANK_SECOND;
                value._millisecond = Time.BLANK_MILLI;
                value._microsecond = Time.BLANK_MICRO_NANO;
                value._nanosecond = Time.BLANK_MICRO_NANO;
                return CodecReturnCode.BLANK_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode DecodeTime0(DecodeIterator iter, Time value)
        {
            value.Blank();
            return CodecReturnCode.BLANK_DATA;
        }

        private static Func<DecodeIterator, Time, CodecReturnCode>[] sizeTimeDecFuncArray = new Func<DecodeIterator, Time, CodecReturnCode>[9];     

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeTime(DecodeIterator iter, Time value)
		{
            try
            {
                int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
                return sizeTimeDecFuncArray[size](iter, value);
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static bool MakeSecMillisBlankCheckDateBlank(DateTime value)
		{
			value.Time().Second(Time.BLANK_SECOND);
			value.Time().Millisecond(Time.BLANK_MILLI);
			value.Time().Microsecond(Time.BLANK_MICRO_NANO);
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode MakeSecMillisZeros(DateTime value)
		{
			value.Time().Second(0);
			value.Time().Millisecond(0);
			value.Time().Microsecond(0);
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static bool MakeMillisBlankCheckDateBlank(DateTime value)
		{
			value.Time().Millisecond(Time.BLANK_MILLI);
			value.Time().Microsecond(Time.BLANK_MICRO_NANO);
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode MakeMillisZeros(DateTime value)
		{
			value.Time().Millisecond(0);
			value.Time().Microsecond(0);
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static bool MakeMicroNanoBlankCheckDateBlank(DateTime value)
		{
			value.Time().Microsecond(Time.BLANK_MICRO_NANO);
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode MakeMicroNanoZeros(DateTime value)
		{
			value.Time().Microsecond(0);
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static bool MakeNanoBlankCheckDateBlank(DateTime value)
		{
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode MakeNanoZeros(DateTime value)
		{
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeDateTime(DecodeIterator iter, DateTime value)
		{
            BufferReader reader = iter._reader;
            byte* pointer;

            CodecReturnCode ret = CodecReturnCode.SUCCESS;
			int savePosition = reader._position;

			switch (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos)
			{
				case 0:
					value.Blank();
					ret = CodecReturnCode.BLANK_DATA;
					return ret;

				case 6:
					try
					{
						pointer = reader._buffer._pointer;
						reader._position = iter._curBufPos;
						value.Day(pointer[reader._position++]);
						value.Month(pointer[reader._position++]);
						value.Year(reader.ReadUnsignedShort());
						value.Time().Hour(pointer[reader._position++]);
						value.Time().Minute(pointer[reader._position++]);

						/* If the time we took from wire matches blank, fill in rest of time as blank,
						 * then ensure that date portion is also blank - if so, return blank data.
						 * If time portion was not blank, just return success. */
						ret = value.Time().Hour() == Time.BLANK_HOUR && value.Time().Minute() == Time.BLANK_MINUTE 
                            ? (MakeSecMillisBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeSecMillisZeros(value);

						reader._position = savePosition;

						return ret;
					}
					catch (Exception)
					{
						value.Clear();
						return CodecReturnCode.INCOMPLETE_DATA;
					}

				case 7:

					try
					{
						pointer = reader._buffer._pointer;
						reader._position = iter._curBufPos;
						value.Day(pointer[reader._position++]);
						value.Month(pointer[reader._position++]);
						value.Year(reader.ReadUnsignedShort());
						value.Time().Hour(pointer[reader._position++]);
						value.Time().Minute(pointer[reader._position++]);
						value.Time().Second(pointer[reader._position++]);

						/* need this to populate rest of time properly */
						/* If the time we took from wire matches blank, fill in rest of time as blank,
						 * then ensure that date portion is also blank - if so, return blank data. */
						/* If time portion was not blank, just return success */
						ret = value.Time().Hour() == Time.BLANK_HOUR && value.Time().Minute() == Time.BLANK_MINUTE && value.Time().Second() == Time.BLANK_SECOND ? (MakeMillisBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeMillisZeros(value);

						reader._position = savePosition;

						return ret;
					}
					catch (Exception)
					{
						value.Clear();
						return CodecReturnCode.INCOMPLETE_DATA;
					}

				case 9:
					try
					{
						pointer = reader._buffer._pointer;
						reader._position = iter._curBufPos;
						value.Day(pointer[reader._position++]);
						value.Month(pointer[reader._position++]);
						value.Year(reader.ReadUnsignedShort());
						value.Time().Hour(pointer[reader._position++]);
						value.Time().Minute(pointer[reader._position++]);
						value.Time().Second(pointer[reader._position++]);
						value.Time().Millisecond(reader.ReadUnsignedShort());

						ret = ((value.Time().Hour() == Time.BLANK_HOUR) && (value.Time().Minute() == Time.BLANK_MINUTE) && (value.Time().Second() == Time.BLANK_SECOND) && (value.Time().Millisecond() == Time.BLANK_MILLI)) ? (MakeMicroNanoBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeMicroNanoZeros(value);

						reader._position = savePosition;

						return ret;
					}
					catch (Exception)
					{
						value.Clear();
						return CodecReturnCode.INCOMPLETE_DATA;
					}

				case 11:
					try
					{
						pointer = reader._buffer._pointer;
						reader._position = iter._curBufPos;
						value.Day(pointer[reader._position++]);
						value.Month(pointer[reader._position++]);
						value.Year(reader.ReadUnsignedShort());
						value.Time().Hour(pointer[reader._position++]);
						value.Time().Minute(pointer[reader._position++]);
						value.Time().Second(pointer[reader._position++]);
						value.Time().Millisecond(reader.ReadUnsignedShort());
						value.Time().Microsecond(reader.ReadUnsignedShort());

						ret = (((value.Time().Hour() == Time.BLANK_HOUR) && (value.Time().Minute() == Time.BLANK_MINUTE) && (value.Time().Second() == Time.BLANK_SECOND) && (value.Time().Millisecond() == Time.BLANK_MILLI) && (value.Time().Microsecond() == Time.BLANK_MICRO_NANO)) ? (MakeNanoBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeNanoZeros(value));

						reader._position = savePosition;

						return ret;
					}
					catch (Exception)
					{
						value.Clear();
						return CodecReturnCode.INCOMPLETE_DATA;
					}

				case 12:
					try
					{
						int tempMicro;
						int tempNano;
						pointer = reader._buffer._pointer;
						reader._position = iter._curBufPos;
						value.Day(pointer[reader._position++]);
						value.Month(pointer[reader._position++]);
						value.Year(reader.ReadUnsignedShort());
						value.Time().Hour(pointer[reader._position++]);
						value.Time().Minute(pointer[reader._position++]);
						value.Time().Second(pointer[reader._position++]);
						value.Time().Millisecond(reader.ReadUnsignedShort());
						tempMicro = reader.ReadUnsignedShort();
						tempNano = pointer[reader._position++];
						value.Time().Microsecond(tempMicro & 0x000007FF);
						value.Time().Nanosecond(((tempMicro & 0x00003800) >> 3) + tempNano);

						reader._position = savePosition;

						if (value.IsBlank)
						{
							return CodecReturnCode.BLANK_DATA;
						}
						else
						{
							return ret; // set to success above
						}

					}
					catch (Exception)
					{
						value.Clear();
						return CodecReturnCode.INCOMPLETE_DATA;
					}

				default:
					return CodecReturnCode.INCOMPLETE_DATA;
			}
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeReal(DecodeIterator iter, Real value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            int endBufPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos;
            BufferReader reader = iter._reader;
            try
            {
                if (endBufPos - iter._curBufPos > 1)
                {
                    int savePosition = reader._position;
                    reader._position = iter._curBufPos;
                    int hint = reader._buffer._pointer[reader._position++];
                    switch (hint & 0x3F)
                    {
                        case Real.BLANK_REAL:
                            value.Blank();
                            ret = CodecReturnCode.BLANK_DATA;
                            break;
                        case RealHints.INFINITY:
                        case RealHints.NEG_INFINITY:
                        case RealHints.NOT_A_NUMBER:
                            value.Value(0, hint & 0x3F);
                            ret = CodecReturnCode.SUCCESS;
                            break;
                        default:
                            int length = endBufPos - iter._curBufPos - 1;
                            value.Value(reader.ReadLong64ls(length), hint & 0x1F);
                            break;
                    }
                    reader._position = savePosition;
                }
                else if (endBufPos - iter._curBufPos == 1)
                {
                    int savePosition = reader._position;
                    reader._position = iter._curBufPos;
                    int hint = reader._buffer._pointer[reader._position++];
                    switch (hint & 0x3F)
                    {
                        case RealHints.INFINITY:
                        case RealHints.NEG_INFINITY:
                        case RealHints.NOT_A_NUMBER:
                            value.Value(0, hint & 0x3F);
                            ret = CodecReturnCode.SUCCESS;
                            break;
                        default:
                            value.Blank();
                            ret = CodecReturnCode.BLANK_DATA;
                            break;
                    }
                    reader._position = savePosition;
                }
                else if (endBufPos - iter._curBufPos == 0)
                {
                    value.Blank();
                    ret = CodecReturnCode.BLANK_DATA;
                }
                else
                {
                    ret = CodecReturnCode.INCOMPLETE_DATA;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }       

			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeMap(DecodeIterator iter, Map map)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != map, "Invalid MapInt in as NULL");
            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;

            BufferReader reader = iter._reader;
            byte* pointer;

            Debug.Assert(null != iter._buffer, "");

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.MAP, map);

            _endBufPos = _levelInfo._endBufPos;
            position = iter._curBufPos;
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = position;

                if (_endBufPos - position == 0)
                {
                    EndOfList(iter);
                    return CodecReturnCode.NO_DATA;
                }
                else if (_endBufPos - position < 5)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* extract flags, keyDataFormat and _containerType */
                map.Flags = (MapFlags)pointer[reader._position++];
                map.KeyPrimitiveType = pointer[reader._position++];
                map.ContainerType = reader._buffer._pointer[reader._position++];
                /* container type needs to be scaled back */
                map.ContainerType += DataTypes.CONTAINER_TYPE_MIN;

                /* Handle legacy conversion */
                map.KeyPrimitiveType = primitiveTypes[map.KeyPrimitiveType];

                if (map.CheckHasKeyFieldId())
                {
                    map.KeyFieldId = reader.ReadShort();
                    position = reader._position;

                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }

                if (map.CheckHasSetDefs())
                {
                    DecodeBuffer15(iter, map._encodedSetDefs);
                    position = reader._position;

                    /* Check for buffer overflow. Post check ok since no copy */
                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }
                else
                {
                    map._encodedSetDefs.Clear();
                }

                if (map.CheckHasSummaryData())
                {
                    DecodeBuffer15(iter, map._encodedSummaryData);
                    position = reader._position;

                    /* Check for buffer overflow. Post check ok since no copy */
                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                }
                else
                {
                    map._encodedSummaryData.Clear();
                }

                /* extract total count hint */
                if (map.CheckHasTotalCountHint())
                {
                    map.TotalCountHint = reader.ReadUInt30rb();
                }

                _levelInfo._itemCount = reader.ReadUnsignedShort();

                position = reader._position;

                map._encodedEntries.Data_internal(reader._buffer, position, _endBufPos - position);

                /* Check for buffer overflow. Post check ok since no copy */
                if (position > _endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo._nextEntryPos = position; // set entry ptr to first entry
            iter._curBufPos = map.CheckHasSummaryData() ? map._encodedSummaryData.Position : position;
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeMapEntry(DecodeIterator iter, MapEntry mapEntry, object keyData)
        {
            int position;
            Map map;
            int flags;

            Debug.Assert(null != mapEntry && null != iter, "Invalid parameters or parameters passed in as NULL");

            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

            BufferReader reader = iter._reader;

            map = (Map)_levelInfo._listType;

            if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
            {
                EndOfList(iter);
                return CodecReturnCode.END_OF_CONTAINER;
            }

            /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
            position = iter._curBufPos = _levelInfo._nextEntryPos;
            try
            {
                reader._position = position;

                if ((position + 2) > _levelInfo._endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* take out action/flags */
                flags = reader._buffer._pointer[reader._position++];

                mapEntry.Action = (MapEntryActions)(flags & 0xF);
                mapEntry.Flags = (MapEntryFlags)(flags >> 4);

                /* get perm data */
                if (map.CheckHasPerEntryPermData() && mapEntry.CheckHasPermData())
                {
                    DecodeBuffer15(iter, mapEntry._permData);
                    position = reader._position;
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }
                else
                {
                    mapEntry._permData.Clear();
                }

                DecodeBuffer15(iter, mapEntry._encodedKey);
                position = reader._position;

                /* User provided storage for decoded key, so decode it for them */
                if (keyData != null)
                {
                    iter._levelInfo[iter._decodingLevel]._nextEntryPos = iter._curBufPos = mapEntry._encodedKey.GetLength() > 0 ? mapEntry._encodedKey.Position : position;
                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    PrimitiveDecFunctions[map.KeyPrimitiveType](iter, keyData);
                }

                if (position > _levelInfo._endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* parse MapEntry value */
                if ((mapEntry.Action == MapEntryActions.DELETE) || (map.ContainerType == DataTypes.NO_DATA))
                {
                    mapEntry._encodedData.Clear();
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    /* only have data if action is not delete */
                    DecodeBuffer16(iter, mapEntry._encodedData);
                    position = reader._position;
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* shift iterator */
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = mapEntry._encodedData.Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeFilterList(DecodeIterator iter, FilterList filterList)
        {
            Debug.Assert(null != filterList, "Invalid filterListInt passed in as NULL");
            Debug.Assert(null != iter, "Invalid iterInt passed in as NULL");
            int position;
            int _endBufPos;
            int count;
            DecodingLevel _levelInfo;

            BufferReader reader = iter._reader;
            byte* pointer;

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.FILTER_LIST, filterList);

            _endBufPos = _levelInfo._endBufPos;
            position = iter._curBufPos;
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = position;

                if (_endBufPos - position == 0)
                {
                    EndOfList(iter);
                    return CodecReturnCode.NO_DATA;
                }
                else if (_endBufPos - position < 3)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                filterList.Flags = (FilterListFlags)pointer[reader._position++];
                filterList.ContainerType = pointer[reader._position++];
                /* needs to be scaled back after decoding */
                filterList.ContainerType += DataTypes.CONTAINER_TYPE_MIN;

                if (filterList.CheckHasTotalCountHint())
                {
                    filterList.TotalCountHint = pointer[reader._position++];
                }
                else
                {
                    filterList.TotalCountHint = 0;
                }

                count = pointer[reader._position++];
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo._itemCount = count;

            position = reader._position;

            filterList._encodedEntries.Data_internal(reader._buffer, position, _endBufPos - position);

            /* Check for buffer overflow. Post check ok since no copy */
            if (position > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            iter._curBufPos = _levelInfo._nextEntryPos = position;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeFilterEntry(DecodeIterator iter, FilterEntry filterEntry)
        {
            int position;
            FilterList filterList;
            int flags;
            byte* pointer;

            Debug.Assert(null != filterEntry && null != iter, "Invalid parameters or parameters passed in as NULL");

            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            BufferReader reader = iter._reader;

            Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

            filterList = (FilterList)_levelInfo._listType;

            if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
            {
                EndOfList(iter);
                return CodecReturnCode.END_OF_CONTAINER;
            }

            /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
            position = iter._curBufPos = _levelInfo._nextEntryPos;
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = position;

                if ((position + 2) > _levelInfo._endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* take out flags */
                flags = pointer[reader._position++];

                filterEntry.Action = (FilterEntryActions)(flags & 0xF);
                filterEntry.Flags = (FilterEntryFlags)(flags >> 4);

                /* parse FilterEntry */
                filterEntry.Id = pointer[reader._position++];
                position = reader._position;

                if (filterEntry.CheckHasContainerType())
                {
                    if ((position + 1) > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                    filterEntry.ContainerType = pointer[reader._position++];
                    /* needs to be scaled back after decoding */
                    filterEntry.ContainerType += DataTypes.CONTAINER_TYPE_MIN;
                }
                else
                {
                    filterEntry.ContainerType = (filterList.ContainerType);
                }

                if (filterList.CheckHasPerEntryPermData() && filterEntry.CheckHasPermData())
                {
                    DecodeBuffer15(iter, filterEntry._permData);
                    position = reader._position;

                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }
                else
                {
                    filterEntry._permData.Clear();
                }

                if ((filterEntry.ContainerType != DataTypes.NO_DATA) && (filterEntry.Action != FilterEntryActions.CLEAR))
                {
                    DecodeBuffer16(iter, filterEntry._encodedData);
                    position = reader._position;
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* shift iterator */
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = filterEntry._encodedData.Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    position = reader._position;
                    filterEntry._encodedData.Clear();
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeArray(DecodeIterator iter, Array array)
        {
            DecodingLevel _levelInfo;
            int _endBufPos;

            Debug.Assert(null != array && null != iter, "Invalid parameters or parameters passed in as NULL");

            BufferReader reader = iter._reader;
            byte* pointer;

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.ARRAY, array);

            _endBufPos = _levelInfo._endBufPos;
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = iter._curBufPos;

                if (iter._curBufPos == _endBufPos)
                {
                    EndOfList(iter);
                    array.Blank();
                    return CodecReturnCode.BLANK_DATA;
                }
                else if (_endBufPos - iter._curBufPos < 3)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* extract data type */
                array._primitiveType = pointer[reader._position++];
                /* extract itemLength */
                array._itemLength = reader.ReadUShort16ob();
                /* extract count */
                _levelInfo._itemCount = reader.ReadUnsignedShort();
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            iter._curBufPos = reader._position;

            _levelInfo._nextEntryPos = iter._curBufPos;
            array._encodedData.Data_internal(reader._buffer, iter._curBufPos, _endBufPos - iter._curBufPos);

            /* handle legacy types */
            array._primitiveType = primitiveTypes[array._primitiveType];

            /* Check for buffer overflow. Post check ok since no copy */
            if (iter._curBufPos > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeArrayEntry(DecodeIterator iter, ArrayEntry arrayEntry)
        {
            Debug.Assert(null != arrayEntry, "Invalid arrayEntry passed in as NULL");
            Debug.Assert(null != iter, "Invalid iterInt passed in as NULL");

            BufferReader reader = iter._reader;

            Array array;
            DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];

            iter._curBufPos = levelInfo._nextEntryPos;
            try
            {
                reader._position = iter._curBufPos;
                /* For an array, since the entries are always primitives(not containers),
				 * no skip logic(_nextEntryPos) needs to be applied. */

                array = (Array)levelInfo._listType;

                if (levelInfo._nextItemPosition >= levelInfo._itemCount)
                {
                    EndOfList(iter);
                    return CodecReturnCode.END_OF_CONTAINER;
                }

                int len = array._itemLength;
                if (len == 0)
                {
                    len = DecodePrimitiveLength(iter, array._primitiveType);
                }

                arrayEntry.EncodedData.Data_internal(reader._buffer, iter._curBufPos, len);
                levelInfo._nextEntryPos = iter._curBufPos + len;

            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            if (levelInfo._nextEntryPos > levelInfo._endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            iter._levelInfo[iter._decodingLevel + 1]._endBufPos = levelInfo._nextEntryPos;

            /* shift iterator */
            levelInfo._nextItemPosition++;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static int DecodePrimitiveLength(DecodeIterator iter, int type)
		{
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            int len = lens[type];
			switch (len)
			{
				case 0:
					// variable len
					len = reader.ReadUShort16ob();
					iter._curBufPos = reader._position;
					break;
				case -1:
					// REAL_4RB
					len = ((pointer[reader._position++] & 0xC0) >> 6) + 1;
					reader._position = iter._curBufPos; // rewind one byte
					break;
				case -2:
					// REAL_8RB
					// shift by 5 is intentional. Each combo represents 2 bytes here
					// so to shift by 5 instead of 6 accounts for this.
					len = ((pointer[reader._position++] & 0xC0) >> 5) + 2;
					reader._position = iter._curBufPos; // rewind one byte
					break;
				default:
					// len is set to the value in the len array
			break;
			}

			return len;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeFieldList(DecodeIterator iter, FieldList fieldList, LocalFieldSetDefDb localFieldSetDeb)
		{
			int position;
			int _endBufPos;
			DecodingLevel _levelInfo;

 			Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");

			BufferReader reader = iter._reader;
			byte* pointer;

			_levelInfo = iter._levelInfo[++iter._decodingLevel];
			if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
			{
				return CodecReturnCode.ITERATOR_OVERRUN;
			}
			iter.Setup(_levelInfo, DataTypes.FIELD_LIST, fieldList);

			_endBufPos = _levelInfo._endBufPos;
			position = iter._curBufPos;
			try
			{
				pointer = reader._buffer._pointer;
				reader._position = position;

				if (_endBufPos - position == 0)
				{
					EndOfList(iter);
					return CodecReturnCode.NO_DATA;
				}

				/* Get Flags */
				fieldList.Flags = (FieldListFlags)pointer[reader._position++];

				/* Get the Field List Information */
				if ((fieldList.Flags & FieldListFlags.HAS_FIELD_LIST_INFO) > 0)
				{
					/* Has 1 byte length */
					int infoLen = pointer[reader._position++];
					int startpos = reader._position;

					fieldList.DictionaryId = reader.ReadUShort15rb();
					fieldList.FieldListNum = reader.ReadUnsignedShort();

					if (startpos + infoLen > _endBufPos)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}

					/* Used info Length to skip */
					position = startpos + infoLen;
					reader._position = position;
				}

				/* Get the Field List Set Data */
				if ((fieldList.Flags & FieldListFlags.HAS_SET_DATA) > 0)
				{
					/* Get the set identifier */
					if (fieldList.CheckHasSetId())
					{
						fieldList.SetId = reader.ReadUShort15rb();
					}
					else
					{
						fieldList.SetId = 0;
					}
					position = reader._position;

                    /* Set the set definition from the local or global set list */
                    if (localFieldSetDeb != null && fieldList.SetId <= localFieldSetDeb.maxLocalId)
                    {
                        if (localFieldSetDeb != null && (localFieldSetDeb._definitions[fieldList.SetId].SetId != LocalFieldSetDefDb.BLANK_ID))
                        {
                            _levelInfo._fieldListSetDef = localFieldSetDeb._definitions[fieldList.SetId];
                        }
                        else
                        {
                            _levelInfo._fieldListSetDef = null;
                        }
                    }
                    else
                    {
                        if (iter._fieldSetDefDb != null && (iter._fieldSetDefDb.Definitions[fieldList.SetId].SetId != GlobalFieldSetDefDb.BLANK_ID))
                        {
                            _levelInfo._fieldListSetDef = iter._fieldSetDefDb.Definitions[fieldList.SetId];
                        }
                        else
                        {
                            _levelInfo._fieldListSetDef = null;
                        }
                    }

                    /* Check for field list data */
                    if ((fieldList.Flags & FieldListFlags.HAS_STANDARD_DATA) > 0)
					{
						/* If HasSetData and HasFieldList, then set data is length specified. */
						DecodeBuffer15(iter, fieldList._encodedSetData);

						/* Get the Field List Data */
						_levelInfo._itemCount = reader.ReadUnsignedShort();
						position = reader._position;
						fieldList._encodedEntries.Data_internal(reader._buffer, position, _endBufPos - position);

						/* check for buffer overflow - post check ok since no copy */
						if (position > _endBufPos)
						{
							return CodecReturnCode.INCOMPLETE_DATA;
						}
					}
					else
					{
						/* Get the field list set data. Not length specified since no field list data exists. */
						fieldList._encodedEntries.Clear();

						fieldList._encodedSetData.Data_internal(reader._buffer, position, _endBufPos - position);

						if (position > _endBufPos)
						{
							return CodecReturnCode.INCOMPLETE_DATA;
						}
					}

					/* Setup to decode the set if able. Otherwise skip to entries. */
					if (_levelInfo._fieldListSetDef != null)
					{
						_levelInfo._setCount = _levelInfo._fieldListSetDef.Count;
						_levelInfo._itemCount += _levelInfo._fieldListSetDef.Count;

						_levelInfo._nextEntryPos = _levelInfo._setCount > 0 ? fieldList._encodedSetData.Position : position; 

						return CodecReturnCode.SUCCESS;
					}
					else
					{
						_levelInfo._setCount = 0;
						_levelInfo._nextEntryPos = position + fieldList._encodedSetData.GetLength();
						return CodecReturnCode.SET_SKIPPED;
					}
				}
				else if ((fieldList.Flags & FieldListFlags.HAS_STANDARD_DATA) > 0)
				{
					/* Get the field list data only */
					fieldList._encodedSetData.Clear();

					_levelInfo._itemCount = reader.ReadUnsignedShort();
					position = reader._position;
					fieldList._encodedEntries.Data_internal(reader._buffer, position, _endBufPos - position);

					iter._curBufPos = _levelInfo._nextEntryPos = position;
					_levelInfo._setCount = 0;
				}
				else
				{
					fieldList._encodedSetData.Clear();
					fieldList._encodedEntries.Clear();
					_levelInfo._itemCount = 0;
					iter._curBufPos = 0;
					_levelInfo._setCount = 0;
				}
			}
			catch (Exception)
			{
				return CodecReturnCode.INCOMPLETE_DATA;
			}
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeFieldEntry(DecodeIterator iter, FieldEntry field)
		{
			int position;
			FieldList fieldList;

			Debug.Assert(null != field && null != iter, "Invalid parameters or parameters passed in as NULL");

			DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

			BufferReader reader = iter._reader;

			Debug.Assert(null != _levelInfo._listType, "Invalid decoding attempted");

			fieldList = (FieldList)_levelInfo._listType;

			if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
			{
				EndOfList(iter);
				return CodecReturnCode.END_OF_CONTAINER;
			}

			/* Make sure we skip to the next entry if we didn't decode the previous entry payload */
			position = iter._curBufPos = _levelInfo._nextEntryPos;
			if (position < reader._buffer.BufferLimit()) // Make sure we are reading only if we can read something at all
			{
				try
				{
					reader._position = position;
                    int encEntriesLength = fieldList._encodedEntries.GetLength();

                    if (_levelInfo._nextSetPosition < _levelInfo._setCount)
					{
						FieldSetDefEntry encoding = null;

						Debug.Assert(null != _levelInfo._fieldListSetDef, "Invalid parameters or parameters passed in as NULL");
						Debug.Assert(_levelInfo._fieldListSetDef.Count == _levelInfo._setCount, "Invalid data");

						encoding = _levelInfo._fieldListSetDef.Entries[_levelInfo._nextSetPosition];

						field.FieldId = encoding.FieldId;
						field.DataType = encoding.DataType;

                        /* Get the set data and reset position */
                        DecFunctions[encoding.DataType](iter, field._encodedData);
						iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._nextEntryPos;

						/* handle legacy conversion */
						field.DataType = primitiveTypes[field.DataType];

                        _levelInfo._nextItemPosition++;
						_levelInfo._nextSetPosition++;

						if (_levelInfo._nextSetPosition == _levelInfo._setCount && encEntriesLength > 0)
						{
							_levelInfo._nextEntryPos = fieldList._encodedEntries.Position;
						}

						return CodecReturnCode.SUCCESS;
					}

					/* Get normal field list data */
					if (fieldList._encodedEntries.Position + encEntriesLength - position < 3)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}

					field.FieldId = reader.ReadShort();
					field.DataType = DataTypes.UNKNOWN;

					/* parse Field */
					DecodeBuffer16(iter, field._encodedData);
				}
				catch (Exception)
				{
					return CodecReturnCode.INCOMPLETE_DATA;
				}
			}
			position = reader._position;
			if (position > _levelInfo._endBufPos)
			{
				return CodecReturnCode.INCOMPLETE_DATA;
			}

			/* shift iterator */
			iter._curBufPos = field._encodedData._position;
			_levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
			_levelInfo._nextItemPosition++;
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeSeries(DecodeIterator iter, Series series)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != series, "Invalid seriesInt in as NULL");
            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;
            BufferReader reader = iter._reader;
            byte* pointer;

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.SERIES, series);

            _endBufPos = _levelInfo._endBufPos;
            position = iter._curBufPos;
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = position;

                if (_endBufPos - position == 0)
                {
                    EndOfList(iter);
                    return CodecReturnCode.NO_DATA;
                }
                else if (_endBufPos - position < 4)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                _levelInfo._endBufPos = _endBufPos;

                /* extract flags, data format */
                series.Flags = (SeriesFlags)pointer[reader._position++];
                series.ContainerType = pointer[reader._position++];
                /* container type needs to be scaled back after decoding */
                series.ContainerType += DataTypes.CONTAINER_TYPE_MIN;

                if (series.CheckHasSetDefs())
                {
                    DecodeBuffer15(iter, series._encodedSetDefs);
                    position = reader._position;
                    /* check for buffer overflow. Post check ok since no copy */
                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }
                else
                {
                    series._encodedSetDefs.Clear();
                }

                if (series.CheckHasSummaryData())
                {
                    DecodeBuffer15(iter, series._encodedSummaryData);
                    position = reader._position;

                    /* check for buffer overflow. Post check ok since no copy */
                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                }
                else
                {
                    series._encodedSummaryData.Clear();
                }

                /* extract total count hints */
                if (series.CheckHasTotalCountHint())
                {
                    series.TotalCountHint = reader.ReadUInt30rb();
                }
                else
                {
                    series.TotalCountHint = 0;
                }

                _levelInfo._itemCount = reader.ReadUnsignedShort();
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            position = reader._position;

            series._encodedEntries.Data_internal(reader._buffer, position, (_endBufPos - position));

            /* check for overflow */
            if (position > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo._nextEntryPos = position; // set entry ptr to first entry
            iter._curBufPos = series.CheckHasSummaryData() ? (series._encodedSummaryData).Position : position;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeLocalElementSetDefDb(DecodeIterator iter, LocalElementSetDefDb localSetDb)
        {
            int position;
            int _endBufPos;
            int flags;
            int _setCount;
            int curEntryCount = 0;

            Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");

            BufferReader reader = iter._reader;
            byte* pointer;

            try
            {
                pointer = reader._buffer._pointer;
                if (iter._decodingLevel >= 0)
                {
                    /* Get the encodedSetDefs pointer out of the current container. */
                    Buffer encodedSetDefs;

                    Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

                    switch (iter._levelInfo[iter._decodingLevel]._containerType)
                    {
                        case DataTypes.MAP:
                            encodedSetDefs = ((Map)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.SERIES:
                            encodedSetDefs = ((Series)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.VECTOR:
                            encodedSetDefs = ((Vector)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        default:
                            return CodecReturnCode.INVALID_ARGUMENT;
                    }

                    if (encodedSetDefs.GetLength() == 0)
                    {
                        return CodecReturnCode.INVALID_DATA;
                    }

                    position = encodedSetDefs.Position;
                    reader._position = position;
                    _endBufPos = encodedSetDefs.Position + encodedSetDefs.GetLength();
                }
                else
                {
                    /* Separate iterator. */
                    _endBufPos = iter._levelInfo[0]._endBufPos;
                    position = iter._curBufPos;
                }

                if ((_endBufPos - position) < 2)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                flags = pointer[reader._position++];
                _setCount = pointer[reader._position++];
                position = reader._position;

                if (_setCount == 0)
                {
                    return CodecReturnCode.SET_DEF_DB_EMPTY;
                }

                if (_setCount > LocalElementSetDefDb.MAX_LOCAL_ID)
                {
                    return CodecReturnCode.TOO_MANY_LOCAL_SET_DEFS;
                }

                for (int i = 0; i <= LocalElementSetDefDb.MAX_LOCAL_ID; i++)
                {
                    localSetDb.Definitions[i].SetId = LocalElementSetDefDb.BLANK_ID;
                }

                for (int i = 0; i < _setCount; i++)
                {
                    ElementSetDef curSetDef;
                    ElementSetDefEntry curEntry;
                    int setId;
                    int encCount;

                    if ((position + 2) > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* get the setId and the number of element encodings */
                    setId = reader.ReadUShort15rb();
                    encCount = pointer[reader._position++];

                    /* sanity checks */
                    if (setId > LocalElementSetDefDb.MAX_LOCAL_ID)
                    {
                        return CodecReturnCode.ILLEGAL_LOCAL_SET_DEF;
                    }
                    if (localSetDb.Definitions[setId].SetId != LocalElementSetDefDb.BLANK_ID)
                    {
                        return CodecReturnCode.DUPLICATE_LOCAL_SET_DEFS;
                    }

                    /* get memory for new set definition from working memory. */
                    curSetDef = localSetDb.Definitions[setId];

                    /* make sure we have space in our entry pool */
                    if (curEntryCount > localSetDb.Entries.Length)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    /* setup set def and put in database */
                    curSetDef.SetId = setId;
                    curSetDef.Count = encCount;
                    curSetDef.Entries = localSetDb.Entries[curEntryCount]; // Point to the entries from the pool that will be used for this def.

                    /* populate the entries */
                    for (int j = 0; j < encCount; j++)
                    {
                        /* make sure we have space in our entry pool */
                        if (encCount > localSetDb.Entries[curEntryCount].Length)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        curEntry = localSetDb.Entries[curEntryCount][j];
                        DecodeBuffer15(iter, curEntry.Name);
                        curEntry.DataType = pointer[reader._position++];
                        position = reader._position;
                    }
                    ++curEntryCount;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            return (position <= _endBufPos) ? CodecReturnCode.SUCCESS : CodecReturnCode.INCOMPLETE_DATA;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeLocalFieldSetDefDb(DecodeIterator iter, LocalFieldSetDefDb localSetDb)
        {
            int position;
            int _endBufPos;
            int _setCount;
            int flags;

            Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");

            BufferReader reader = iter._reader;
            byte* pointer;

            int curEntryCount = 0;

            try
            {
                pointer = reader._buffer._pointer;
                if (iter._decodingLevel >= 0)
                {
                    /* Get the encodedSetDefs pointer out of the current container. */
                    Buffer encodedSetDefs;

                    switch (iter._levelInfo[iter._decodingLevel]._containerType)
                    {
                        case DataTypes.MAP:
                            encodedSetDefs = ((Map)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.SERIES:
                            encodedSetDefs = ((Series)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.VECTOR:
                            encodedSetDefs = ((Vector)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        default:
                            return CodecReturnCode.INVALID_ARGUMENT;
                    }

                    if (encodedSetDefs.GetLength() == 0)
                    {
                        return CodecReturnCode.INVALID_DATA;
                    }

                    position = encodedSetDefs.Position;
                    reader._position = position;
                    _endBufPos = encodedSetDefs.Position + encodedSetDefs.GetLength();
                }
                else
                {
                    /* Separate iterator. */
                    _endBufPos = iter._levelInfo[0]._endBufPos;
                    position = iter._curBufPos;
                }

                if ((_endBufPos - position) < 2)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                flags = pointer[reader._position++];
                _setCount = pointer[reader._position++];
                position = reader._position;

                if (_setCount == 0)
                {
                    return CodecReturnCode.SET_DEF_DB_EMPTY;
                }

                if (_setCount > LocalFieldSetDefDb.MAX_LOCAL_ID)
                {
                    return CodecReturnCode.TOO_MANY_LOCAL_SET_DEFS;
                }

                for (int i = 0; i <= LocalFieldSetDefDb.MAX_LOCAL_ID; i++)
                {
                    localSetDb._definitions[i].SetId = LocalFieldSetDefDb.BLANK_ID;
                }

                for (int i = 0; i < _setCount; i++)
                {
                    FieldSetDef curSetDef;
                    FieldSetDefEntry curEntry;
                    int setId;
                    int encCount;

                    if ((position + 2) > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* Get the setId and the number of field encodings */
                    setId = reader.ReadUShort15rb();
                    encCount = pointer[reader._position++];

                    /* Basic sanity checks */
                    if (setId > LocalFieldSetDefDb.MAX_LOCAL_ID)
                    {
                        return CodecReturnCode.ILLEGAL_LOCAL_SET_DEF;
                    }
                    if (localSetDb._definitions[setId].SetId != LocalFieldSetDefDb.BLANK_ID)
                    {
                        return CodecReturnCode.DUPLICATE_LOCAL_SET_DEFS;
                    }

                    /* get memory for new set definition from working memory. */
                    curSetDef = localSetDb._definitions[setId];

                    /* make sure we have space in our entry pool */
                    if (curEntryCount > localSetDb.Entries.Length)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    /* Setup set definition and put in database */
                    curSetDef.SetId = setId;
                    curSetDef.Count = encCount;
                    curSetDef.Entries = localSetDb.Entries[curEntryCount]; // Point to the entries from the pool that will be used for this def.

                    /* Fill in the field list encodings */
                    for (int j = 0; j < encCount; j++)
                    {
                        /* make sure we have space in our entry pool */
                        if (encCount > localSetDb.Entries[curEntryCount].Length)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        curEntry = localSetDb.Entries[curEntryCount][j];
                        curEntry.FieldId = reader.ReadUnsignedShort();
                        curEntry.DataType = pointer[reader._position++];
                        position = reader._position;
                    }
                    ++curEntryCount;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
            return (position <= _endBufPos) ? CodecReturnCode.SUCCESS : CodecReturnCode.INCOMPLETE_DATA;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeSeriesEntry(DecodeIterator iter, SeriesEntry seriesEntry)
        {
            int position;
            Series series;

            Debug.Assert(null != seriesEntry && null != iter, "Invalid parameters or parameters passed in as NULL");

            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            BufferReader reader = iter._reader;

            Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

            series = (Series)_levelInfo._listType;

            if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
            {
                EndOfList(iter);
                return CodecReturnCode.END_OF_CONTAINER;
            }

            /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
            position = iter._curBufPos = _levelInfo._nextEntryPos;
            try
            {
                reader._position = position;

                if (series.ContainerType != DataTypes.NO_DATA)
                {
                    DecodeBuffer16(iter, seriesEntry._encodedData);
                    position = reader._position;
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* shift iterator */
                    iter._curBufPos = (seriesEntry._encodedData).Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    _levelInfo._nextItemPosition++;
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    seriesEntry._encodedData.Clear();
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    reader._position = position;
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static int ConvertToPrimitiveType(int type)
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode PrimitiveToString(object type, int dataType, Buffer @out)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
			switch (dataType)
			{
				case DataTypes.BUFFER:
				case DataTypes.ASCII_STRING:
				case DataTypes.UTF8_STRING:
				case DataTypes.RMTES_STRING:
					(@out).Data_internal(((Buffer)type).ToString());
					break;
				case DataTypes.UINT:
					string tempUInt = ((UInt)type).ToString();
					(@out).Data_internal(tempUInt);
					break;
				case DataTypes.INT:
					string tempInt = ((Int)type).ToString();
					(@out).Data_internal(tempInt);
					break;
				case DataTypes.FLOAT:
					string tempFloat = ((Float)type).ToString();
					(@out).Data_internal(tempFloat);
					break;
				case DataTypes.DOUBLE:
					string tempDouble = ((Double)type).ToString();
					(@out).Data_internal(tempDouble);
					break;
				case DataTypes.REAL:
					string tempReal = ((Real)type).ToString();
					(@out).Data_internal(tempReal);
					break;
				case DataTypes.DATE:
					string tempDate = ((Date)type).ToString();
					(@out).Data_internal(tempDate);
					break;
				case DataTypes.TIME:
					string tempTime = ((Time)type).ToString();
					(@out).Data_internal(tempTime);
					break;
				case DataTypes.DATETIME:
					string tempDateTime = ((DateTime)type).ToString();
					(@out).Data_internal(tempDateTime);
					break;
				case DataTypes.QOS:
					string tempQos = ((Qos)type).ToString();
					(@out).Data_internal(tempQos);
					break;
				case DataTypes.STATE:
					string tempState = ((State)type).ToString();
					(@out).Data_internal(tempState);
					break;
				case DataTypes.ENUM:
					string tempEnum = ((Enum)type).ToString();
					(@out).Data_internal(tempEnum);
					break;
				default:
					ret = CodecReturnCode.FAILURE;
					break;
			}

			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static int GetItemCount(DecodeIterator iter)
		{
			return iter._levelInfo[iter._decodingLevel]._itemCount;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeVector(DecodeIterator iter, Vector vector)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != vector, "Invalid vectorInt in as NULL");
            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;

            BufferReader reader = iter._reader;
            byte* pointer;

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.VECTOR, vector);

            _endBufPos = _levelInfo._endBufPos;
            position = iter._curBufPos;

            if (_endBufPos - position == 0)
            {
                EndOfList(iter);
                return CodecReturnCode.NO_DATA;
            }
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = position;

                vector.Flags = (VectorFlags)pointer[reader._position++];

                vector._containerType = pointer[reader._position++];
                /* container type needs to be scaled after its decoded */
                vector._containerType += DataTypes.CONTAINER_TYPE_MIN;

                if (vector.CheckHasSetDefs())
                {
                    DecodeBuffer15(iter, vector._encodedSetDefs);
                    position = reader._position;
                    /* check for buffer overflow. Post check ok since no copy */
                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }
                else
                {
                    vector._encodedSetDefs.Clear();
                }

                if (vector.CheckHasSummaryData())
                {
                    DecodeBuffer15(iter, vector._encodedSummaryData);
                    position = reader._position;
                    /* check for buffer overflow. Post check ok since no copy */
                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                }
                else
                {
                    vector._encodedSummaryData.Clear();
                }

                /* take out total count hint if present */
                if (vector.CheckHasTotalCountHint())
                {
                    vector._totalCountHint = reader.ReadUInt30rb();
                }
                else
                {
                    vector._totalCountHint = 0;
                }

                _levelInfo._itemCount = reader.ReadUnsignedShort();
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            position = reader._position;

            vector.EncodedEntries.Data_internal(reader._buffer, position, (_endBufPos - position));

            /* check for buffer overflow. Post check ok since no copy */
            if (position > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo._nextEntryPos = position; // set entry ptr to first entry
            iter._curBufPos = vector.CheckHasSummaryData() ? (vector._encodedSummaryData).Position : position;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeVectorEntry(DecodeIterator iter, VectorEntry vectorEntry)
        {
            int position;
            Vector vector;
            int flags;

            Debug.Assert(null != vectorEntry && null != iter, "Invalid parameters or parameters passed in as NULL)");

            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];
            BufferReader reader = iter._reader;
            byte* pointer;

            Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

            vector = (Vector)_levelInfo._listType;

            if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
            {
                EndOfList(iter);
                return CodecReturnCode.END_OF_CONTAINER;
            }

            /* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
            position = iter._curBufPos = _levelInfo._nextEntryPos;
            try
            {
                pointer = reader._buffer._pointer;
                reader._position = position;

                if ((position + 2) > _levelInfo._endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* take out action/flags */
                flags = pointer[reader._position++];

                vectorEntry.Action = (VectorEntryActions)(flags & 0xF);
                vectorEntry.Flags = (VectorEntryFlags)(flags >> 4);

                /* get index */
                vectorEntry._index = (uint)reader.ReadUInt30rb();

                /* get perm data */
                if ((vector.CheckHasPerEntryPermData()) && (vectorEntry.CheckHasPermData()))
                {
                    DecodeBuffer15(iter, vectorEntry._permData);
                    position = reader._position;
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }
                else
                {
                    vectorEntry._permData.Clear();
                }

                if ((vectorEntry.Action != VectorEntryActions.CLEAR) && (vectorEntry.Action != VectorEntryActions.DELETE) && (vector.ContainerType != DataTypes.NO_DATA))
                {
                    DecodeBuffer16(iter, vectorEntry._encodedData);
                    position = reader._position;
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* shift iterator */
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = (vectorEntry._encodedData).Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    position = reader._position;
                    vectorEntry._encodedData.Clear();
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static CodecReturnCode DecBuf8(DecodeIterator iter, Buffer data)
		{
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int length;

            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            Debug.Assert(iter._curBufPos == levelInfo._nextEntryPos, "Invalid decoding attempted");

			/* Move _curBufPos and _endBufPos around data. _nextEntryPos should point after it. */
			length = pointer[reader._position++];
			iter._curBufPos = reader._position;
			data.Data_internal(reader._buffer, iter._curBufPos, length);
			levelInfo._nextEntryPos = iter._curBufPos + length;

			return levelInfo._nextEntryPos <= levelInfo._endBufPos ? CodecReturnCode.SUCCESS : CodecReturnCode.INCOMPLETE_DATA;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecBuf8_1(DecodeIterator iter, Buffer data)
        {
            DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
            int length;
            BufferReader reader = iter._reader;
            byte* pointer = reader._buffer._pointer;

            Debug.Assert(iter._curBufPos == levelInfo._nextEntryPos, "Invalid decoding attempted");

            /* Move _curBufPos and _endBufPos around data. _nextEntryPos should point after it. */
            length = pointer[reader._position++];
            iter._curBufPos = reader._position;
            data.Data_internal(reader._buffer, iter._curBufPos, length);
            levelInfo._nextEntryPos = iter._curBufPos + length;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void DecBuf16(DecodeIterator iter, Buffer data)
		{
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int tlen;

			Debug.Assert(iter._curBufPos == levelInfo._nextEntryPos, "Invalid decoding attempted");

            BufferReader reader = iter._reader;

            tlen = reader.ReadUShort16ob();
			iter._curBufPos = reader._position;
			data.Data_internal(reader._buffer, iter._curBufPos, tlen);
			levelInfo._nextEntryPos = iter._curBufPos + tlen;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec8(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 1);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos++;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec16(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 2);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 2;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec24(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 3);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 3;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec32(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 4);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 4;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec40(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 5);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 5;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec56(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 7);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 7;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec64(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 8);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 8;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec72(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 9);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 9;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec88(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 11);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 11;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void Dec96(DecodeIterator iter, Buffer data)
		{
			data.Data_internal(iter._reader._buffer, iter._curBufPos, 12);
            iter._levelInfo[iter._decodingLevel]._nextEntryPos += 12;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecReal_4rb(DecodeIterator iter, Buffer data)
		{
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int format = iter._reader._buffer._pointer[iter._reader._position++];

			int len;
			if ((format & 0x20) == 0x20)
			{
				len = 1;
			}
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
			data.Data_internal(iter._reader._buffer, iter._curBufPos, len);
			levelInfo._nextEntryPos += data.GetLength();
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private unsafe static void DecReal_8rb(DecodeIterator iter, Buffer data)
		{
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int format = iter._reader._buffer._pointer[iter._reader._position++];

			int len;
			if ((format & 0x20) == 0x20)
			{
				len = 1;
			}
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
			data.Data_internal(iter._reader._buffer, iter._curBufPos, len);
			levelInfo._nextEntryPos += data.GetLength();
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeFloat(DecodeIterator iter, Float value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 4)
			{
				int savePosition = iter._reader._position;
				try
				{
					iter._reader._position = iter._curBufPos;
					value.Value(iter._reader.ReadFloat());
					iter._reader._position = savePosition;
				}
				catch (Exception)
				{
					return CodecReturnCode.INVALID_ARGUMENT;
				}
			}
			else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
			{
				value.Blank();
				ret = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				ret = CodecReturnCode.INCOMPLETE_DATA;
			}
			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode DecodeDouble(DecodeIterator iter, Double value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 8)
			{
				int savePosition = iter._reader._position;
				try
				{
					iter._reader._position = iter._curBufPos;
					value.Value(iter._reader.ReadDouble());
					iter._reader._position = savePosition;
				}
				catch (Exception)
				{
					return CodecReturnCode.INVALID_ARGUMENT;
				}
			}
			else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
			{
				value.Blank();
				ret = CodecReturnCode.BLANK_DATA;
			}
			else
			{
				ret = CodecReturnCode.INCOMPLETE_DATA;
			}
			return ret;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeFieldSetDefDb(DecodeIterator iter, FieldSetDefDb localSetDb)
        {
            int position;
            int _endBufPos;
            int _setCount;
            int flags;

            int curEntryCount = 0;

            Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");
            try
            {
                if (iter._decodingLevel >= 0)
                {
                    /* Get the encodedSetDefs pointer out of the current container. */
                    Buffer encodedSetDefs;

                    switch (iter._levelInfo[iter._decodingLevel]._containerType)
                    {
                        case DataTypes.MAP:
                            encodedSetDefs = ((Map)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.SERIES:
                            encodedSetDefs = ((Series)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.VECTOR:
                            encodedSetDefs = ((Vector)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        default:
                            return CodecReturnCode.INVALID_ARGUMENT;
                    }

                    if (encodedSetDefs.GetLength() == 0)
                    {
                        return CodecReturnCode.INVALID_DATA;
                    }

                    position = (encodedSetDefs).Position;
                    iter._reader._position = position;
                    _endBufPos = (encodedSetDefs).Position + encodedSetDefs.GetLength();
                }
                else
                {
                    /* Separate iterator. */
                    _endBufPos = iter._levelInfo[0]._endBufPos;
                    position = iter._curBufPos;
                }

                if ((_endBufPos - position) < 2)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                flags = iter._reader._buffer._pointer[iter._reader._position++];
                _setCount = iter._reader._buffer._pointer[iter._reader._position++];
                position = iter._reader._position;

                if (_setCount == 0)
                {
                    return CodecReturnCode.SET_DEF_DB_EMPTY;
                }

                if (_setCount > LocalFieldSetDefDb.MAX_LOCAL_ID)
                {
                    return CodecReturnCode.TOO_MANY_LOCAL_SET_DEFS;
                }

                for (int i = 0; i <= LocalFieldSetDefDb.MAX_LOCAL_ID; i++)
                {
                    localSetDb._definitions[i].SetId = LocalFieldSetDefDb.BLANK_ID;
                }

                for (int i = 0; i < _setCount; i++)
                {
                    FieldSetDef curSetDef;
                    FieldSetDefEntry curEntry;
                    int setId;
                    int encCount;

                    if ((position + 2) > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* Get the setId and the number of field encodings */
                    setId = iter._reader.ReadUShort15rb();
                    encCount = iter._reader._buffer._pointer[iter._reader._position++];

                    /* Basic sanity checks */
                    if (setId > localSetDb.maxLocalId)
                    {
                        return CodecReturnCode.ILLEGAL_LOCAL_SET_DEF;
                    }
                    if (localSetDb._definitions[setId].SetId != FieldSetDefDb.BLANK_ID)
                    {
                        return CodecReturnCode.DUPLICATE_LOCAL_SET_DEFS;
                    }

                    /* get memory for new set definition from working memory. */
                    curSetDef = localSetDb._definitions[setId];

                    /* make sure we have space in our entry pool */
                    if (curEntryCount > localSetDb.Definitions[curEntryCount].Entries.Length)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    /* Setup set definition and put in database */
                    curSetDef.SetId = setId;
                    curSetDef.Count = encCount;
                    curSetDef.Entries = localSetDb.Definitions[curEntryCount].Entries; // Point to the entries from the pool that will be used for this def.

                    /* Fill in the field list encodings */
                    for (int j = 0; j < encCount; j++)
                    {
                        /* make sure we have space in our entry pool */
                        if (encCount > localSetDb.Definitions[curEntryCount].Entries.Length)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        curEntry = localSetDb.Definitions[curEntryCount].Entries[j];
                        curEntry.FieldId = iter._reader.ReadUnsignedShort();
                        curEntry.DataType = iter._reader._buffer._pointer[iter._reader._position++];
                        position = iter._reader._position;
                    }
                    ++curEntryCount;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
            return (position <= _endBufPos) ? CodecReturnCode.SUCCESS : CodecReturnCode.INCOMPLETE_DATA;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal unsafe static CodecReturnCode DecodeElementSetDefDb(DecodeIterator iter, ElementSetDefDb localSetDb)
        {
            int position;
            int _endBufPos;
            int flags;
            int _setCount;
            int curEntryCount = 0;

            Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");

            BufferReader reader = iter._reader;
            byte* pointer;

            try
            {
                pointer = reader._buffer._pointer;
                if (iter._decodingLevel >= 0)
                {
                    /* Get the encodedSetDefs pointer out of the current container. */
                    Buffer encodedSetDefs;

                    Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

                    switch (iter._levelInfo[iter._decodingLevel]._containerType)
                    {
                        case DataTypes.MAP:
                            encodedSetDefs = ((Map)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.SERIES:
                            encodedSetDefs = ((Series)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        case DataTypes.VECTOR:
                            encodedSetDefs = ((Vector)iter._levelInfo[iter._decodingLevel]._listType)._encodedSetDefs;
                            break;
                        default:
                            return CodecReturnCode.INVALID_ARGUMENT;
                    }

                    if (encodedSetDefs.Length == 0)
                    {
                        return CodecReturnCode.INVALID_DATA;
                    }

                    position = encodedSetDefs.Position;
                    reader._position = position;
                    _endBufPos = encodedSetDefs.Position + encodedSetDefs.GetLength();
                }
                else
                {
                    /* Separate iterator. */
                    _endBufPos = iter._levelInfo[0]._endBufPos;
                    position = iter._curBufPos;
                }

                if ((_endBufPos - position) < 2)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                flags = pointer[reader._position++];
                _setCount = pointer[reader._position++];
                position = reader._position;

                if (_setCount == 0)
                {
                    return CodecReturnCode.SET_DEF_DB_EMPTY;
                }

                if (_setCount > localSetDb.MAX_LOCAL_ID)
                {
                    return CodecReturnCode.TOO_MANY_LOCAL_SET_DEFS;
                }

                for (int i = 0; i <= localSetDb.MAX_LOCAL_ID; i++)
                {
                    localSetDb.Definitions[i].SetId = ElementSetDefDb.BLANK_ID;
                }

                for (int i = 0; i < _setCount; i++)
                {
                    ElementSetDef curSetDef;
                    ElementSetDefEntry curEntry;
                    int setId;
                    int encCount;

                    if ((position + 2) > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* get the setId and the number of element encodings */
                    setId = reader.ReadUShort15rb();
                    encCount = pointer[reader._position++];

                    /* sanity checks */
                    if (setId > localSetDb.MAX_LOCAL_ID)
                    {
                        return CodecReturnCode.ILLEGAL_LOCAL_SET_DEF;
                    }
                    if (localSetDb.Definitions[setId].SetId != ElementSetDefDb.BLANK_ID)
                    {
                        return CodecReturnCode.DUPLICATE_LOCAL_SET_DEFS;
                    }

                    /* get memory for new set definition from working memory. */
                    curSetDef = localSetDb.Definitions[setId];

                    /* make sure we have space in our entry pool */
                    if (curEntryCount > localSetDb.Definitions[curEntryCount].Entries.Length)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    /* setup set def and put in database */
                    curSetDef.SetId = setId;
                    curSetDef.Count = encCount;
                    curSetDef.Entries = localSetDb.Definitions[curEntryCount].Entries;

                    /* populate the entries */
                    for (int j = 0; j < encCount; j++)
                    {
                        /* make sure we have space in our entry pool */
                        if (encCount > localSetDb.Definitions[curEntryCount].Entries.Length)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        curEntry = localSetDb.Definitions[curEntryCount].Entries[j];
                        DecodeBuffer15(iter, curEntry.Name);
                        curEntry.DataType = pointer[reader._position++];
                        position = reader._position;
                    }
                    ++curEntryCount;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            return (position <= _endBufPos) ? CodecReturnCode.SUCCESS : CodecReturnCode.INCOMPLETE_DATA;
        }
    }
}
