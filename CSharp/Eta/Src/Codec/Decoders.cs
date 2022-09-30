/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;

namespace Refinitiv.Eta.Codec
{
	internal class Decoders
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
		}


        internal static CodecReturnCode DecodeMsg(DecodeIterator iterInt, IMsg msg)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
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
            iter._curBufPos = iter._reader.Position();
            position = iter._curBufPos;

            if (position > _levelInfo._endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            if ((_levelInfo._endBufPos - position) > 0)
            {
                msg.EncodedDataBody.Data_internal(iter._reader.Buffer(), position, (_levelInfo._endBufPos - position));

                if (!CanDecodeContainerType(msg.ContainerType))
                {
                    /* ETA has no decoders for this format(e.g. Opaque). Move past it. */
                    iter._curBufPos += msg.EncodedDataBody.Length;
                    try
                    {
                        iter._reader.SkipBytes(msg.EncodedDataBody.Length);
                    }
                    catch (Exception)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                    iter._curBufPos = iter._reader.Position();
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
                msg.EncodedDataBody.Clear();
                EndOfList(iter);
                return CodecReturnCode.SUCCESS;
            }
        }

        private static void DecodeMsgHeader(DecodeIterator iterInt, IMsg msg)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            int startPosition = iter._curBufPos;
            int headerSize = 0;
            int keySize = 0;

            iter._reader.Position(startPosition);

            /* header size */
            headerSize = iter._reader.ReadUnsignedShort();

            msg.MsgClass = iter._reader.ReadByte();

            /* mask msgClass top three bits for reserved use later */
            /* Top three bits reserved for later use */
            msg.MsgClass = msg.MsgClass & 0x1F;

            msg.DomainType = iter._reader.ReadUnsignedByte();
            msg.StreamId = iter._reader.ReadInt();
            iter._curBufPos = iter._reader._position; // adjust the iterator's position to match the reader's position.

            ((Msg)msg).SetEncodedMsgBuffer(iter._buffer.Data(), startPosition, iter._buffer.Length);

            /* IMPORTANT: When new message classes are added, CopyMsg and ValidateMsg have to be modified as well */

            switch (msg.MsgClass)
            {
                case MsgClasses.UPDATE:
                    IUpdateMsg updateMsg = (IUpdateMsg)msg;

                    updateMsg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    updateMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;

                    updateMsg.UpdateType = iter._reader.ReadUnsignedByte();

                    if (updateMsg.CheckHasSeqNum())
                    {
                        updateMsg.SeqNum = iter._reader.ReadUnsignedInt();
                    }

                    if (updateMsg.CheckHasConfInfo())
                    {
                        updateMsg.ConflationCount = iter._reader.ReadUShort15rb();
                        updateMsg.ConflationTime = iter._reader.ReadUnsignedShort();
                    }

                    if (updateMsg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, updateMsg.PermData);
                    }

                    if (updateMsg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();

                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = iter._reader.Position();
                        DecodeBaseKey(iter, updateMsg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        iter._reader.Position(currentPosition);
                    }

                    if (updateMsg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, updateMsg.ExtendedHeader);
                    }

                    if (updateMsg.CheckHasPostUserInfo())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (iter._reader.Position() - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                        {
                            /* get user address */
                            updateMsg.PostUserInfo.UserAddr = iter._reader.ReadUnsignedInt();
                            /* get user ID */
                            updateMsg.PostUserInfo.UserId = iter._reader.ReadUnsignedInt();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = updateMsg.Flags;
                            flags &= ~UpdateMsgFlags.HAS_POST_USER_INFO;
                            updateMsg.Flags = flags;
                        }
                    }
                    break;
                case MsgClasses.REFRESH:
                    IRefreshMsg refreshMsg = (IRefreshMsg)msg;

                    refreshMsg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    refreshMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;

                    if (refreshMsg.CheckHasSeqNum())
                    {
                        refreshMsg.SeqNum = iter._reader.ReadUnsignedInt();
                    }

                    DecodeStateInMsg(iter, refreshMsg.State);
                    DecodeBuffer8(iter, refreshMsg.GroupId);

                    if (refreshMsg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, refreshMsg.PermData);
                    }

                    if (refreshMsg.CheckHasQos())
                    {
                        DecodeQosInMsg(iter, refreshMsg.Qos);
                    }

                    if (refreshMsg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();
                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = iter._reader.Position();
                        DecodeBaseKey(iter, refreshMsg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        iter._reader.Position(currentPosition);
                    }

                    if (refreshMsg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, refreshMsg.ExtendedHeader);
                    }

                    if (refreshMsg.CheckHasPostUserInfo())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (iter._reader.Position() - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                        {
                            /* get user address */
                            refreshMsg.PostUserInfo.UserAddr = iter._reader.ReadUnsignedInt();
                            /* get user ID */
                            refreshMsg.PostUserInfo.UserId = iter._reader.ReadUnsignedInt();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = refreshMsg.Flags;
                            flags &= ~RefreshMsgFlags.HAS_POST_USER_INFO;
                            refreshMsg.Flags = flags;
                        }
                    }

                    if (refreshMsg.CheckHasPartNum())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (iter._reader.Position() - (startPosition + 2))) > 0)
                        {
                            refreshMsg.PartNum = iter._reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = refreshMsg.Flags;
                            flags &= ~RefreshMsgFlags.HAS_PART_NUM;
                            refreshMsg.Flags = flags;
                        }
                    }
                    break;
                case MsgClasses.REQUEST:
                    IRequestMsg requestMsg = (IRequestMsg)msg;
                    DecodeRequestMsgHeader(iter, requestMsg);
                    break;
                case MsgClasses.STATUS:
                    IStatusMsg statusMsg = (IStatusMsg)msg;

                    statusMsg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    statusMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;

                    if (statusMsg.CheckHasState())
                    {
                        DecodeStateInMsg(iter, statusMsg.State);
                    }

                    if (statusMsg.CheckHasGroupId())
                    {
                        DecodeBuffer8(iter, statusMsg.GroupId);
                    }

                    if (statusMsg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, statusMsg.PermData);
                    }

                    if (statusMsg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();
                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = iter._reader.Position();
                        DecodeBaseKey(iter, statusMsg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        iter._reader.Position(currentPosition);
                    }

                    if (statusMsg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, statusMsg.ExtendedHeader);
                    }

                    if (statusMsg.CheckHasPostUserInfo())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (iter._reader.Position() - (startPosition + 2))) >= POST_USER_INFO_SIZE)
                        {
                            /* get user address */
                            statusMsg.PostUserInfo.UserAddr = iter._reader.ReadUnsignedInt();
                            /* get user ID */
                            statusMsg.PostUserInfo.UserId = iter._reader.ReadUnsignedInt();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = statusMsg.Flags;
                            flags &= ~StatusMsgFlags.HAS_POST_USER_INFO;
                            statusMsg.Flags = flags;
                        }
                    }
                    break;
                case MsgClasses.CLOSE:
                    ICloseMsg closeMsg = (ICloseMsg)msg;
                    DecodeCloseMsgHeader(iter, closeMsg);
                    break;
                case MsgClasses.ACK:
                    IAckMsg ackMsg = (IAckMsg)msg;

                    ackMsg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    ackMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;
                    ackMsg.AckId = iter._reader.ReadUnsignedInt();

                    if (ackMsg.CheckHasNakCode())
                    {
                        ackMsg.NakCode = iter._reader.ReadUnsignedByte();
                    }

                    if (ackMsg.CheckHasText())
                    {
                        DecodeBuffer16(iter, ackMsg.Text);
                    }

                    if (ackMsg.CheckHasSeqNum())
                    {
                        ackMsg.SeqNum = iter._reader.ReadUnsignedInt();
                    }

                    if (ackMsg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();
                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = iter._reader.Position();
                        DecodeBaseKey(iter, ackMsg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        iter._reader.Position(currentPosition);
                    }

                    if (ackMsg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, ackMsg.ExtendedHeader);
                    }
                    break;
                case MsgClasses.GENERIC:
                    IGenericMsg genericMsg = (IGenericMsg)msg;

                    genericMsg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    genericMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;

                    if (genericMsg.CheckHasSeqNum())
                    {
                        genericMsg.SeqNum = iter._reader.ReadUnsignedInt();
                    }

                    if (genericMsg.CheckHasSecondarySeqNum())
                    {
                        genericMsg.SecondarySeqNum = iter._reader.ReadUnsignedInt();
                    }

                    if (genericMsg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, genericMsg.PermData);
                    }

                    if (genericMsg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();

                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = iter._reader.Position();
                        DecodeBaseKey(iter, genericMsg.MsgKey);

                        /* add keySize to position */
                        currentPosition += keySize;
                        iter._reader.Position(currentPosition);
                    }

                    if (genericMsg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, genericMsg.ExtendedHeader);
                    }

                    if (genericMsg.CheckHasPartNum())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (iter._reader.Position() - (startPosition + 2))) > 0)
                        {
                            genericMsg.PartNum = iter._reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = genericMsg.Flags;
                            flags &= ~GenericMsgFlags.HAS_PART_NUM;
                            genericMsg.Flags = flags;
                        }
                    }
                    break;
                case MsgClasses.POST:
                    IPostMsg postMsg = (IPostMsg)msg;

                    postMsg.Flags = iter._reader.ReadUShort15rb();
                    // need to scale containerType
                    postMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;

                    /* get user address */
                    postMsg.PostUserInfo.UserAddr = iter._reader.ReadUnsignedInt();
                    /* get user ID */
                    postMsg.PostUserInfo.UserId = iter._reader.ReadUnsignedInt();

                    if (postMsg.CheckHasSeqNum())
                    {
                        postMsg.SeqNum = iter._reader.ReadUnsignedInt();
                    }

                    if (postMsg.CheckHasPostId())
                    {
                        postMsg.PostId = iter._reader.ReadUnsignedInt();
                    }

                    if (postMsg.CheckHasPermData())
                    {
                        DecodeBuffer15(iter, postMsg.PermData);
                    }

                    if (postMsg.CheckHasMsgKey())
                    {
                        keySize = iter._reader.ReadUShort15rb();

                        /* don't iterate position by this value anymore. We want to add the keySize to position */
                        int currentPosition = iter._reader.Position();
                        DecodeBaseKey(iter, postMsg.MsgKey);
                        /* add keySize to position */
                        currentPosition += keySize;
                        iter._reader.Position(currentPosition);
                    }

                    if (postMsg.CheckHasExtendedHdr())
                    {
                        DecodeBuffer8(iter, postMsg.ExtendedHeader);
                    }

                    if (postMsg.CheckHasPartNum())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (iter._reader.Position() - (startPosition + 2))) > 0)
                        {
                            postMsg.PartNum = iter._reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = postMsg.Flags;
                            flags &= ~PostMsgFlags.HAS_PART_NUM;
                            postMsg.Flags = flags;
                        }
                    }

                    if (postMsg.CheckHasPostUserRights())
                    {
                        /* decode only if actually in the header */
                        if ((headerSize - (iter._reader.Position() - (startPosition + 2))) > 0)
                        {
                            postMsg.PostUserRights = iter._reader.ReadUShort15rb();
                        }
                        else
                        /* not really there, unset flag */
                        {
                            int flags = postMsg.Flags;
                            flags &= ~PostMsgFlags.HAS_POST_USER_RIGHTS;
                            postMsg.Flags = flags;
                        }
                    }
                    break;
                default:
                    throw new System.NotSupportedException("Unknown msgClass: " + msg.MsgClass);
            }

            /* move to end of header */
            iter._reader.Position(headerSize + 2 + startPosition);
            iter._curBufPos = headerSize + 2 + startPosition;
        }

        /* Decodes the RequestMsg header.
		 * Returns CodecReturnCode.SUCCESS if the message was successfully decoded.
		 * 
		 * iter is the iterator that decodes the message
		 * requestMsg is the message to decode
		 * 
		 * Returns CodecReturnCode.SUCCESS if the message was successfully decoded.
		 */

        private static void DecodeRequestMsgHeader(DecodeIterator iter, IRequestMsg requestMsg)
        {
            requestMsg.Flags = iter._reader.ReadUShort15rb();
            // need to scale containerType
            requestMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;

            if (requestMsg.CheckHasPriority())
            {
                requestMsg.Priority.PriorityClass = iter._reader.ReadUnsignedByte();
                requestMsg.Priority.Count = iter._reader.ReadUShort16ob();
            }

            // note: at present, the code that parses QOS will never return a non-success value
            DecodeRequestMsgHeaderQos(iter, requestMsg);

            int keySize = iter._reader.ReadUShort15rb();
            // don't iterate position by this value anymore. We want to add the keySize to position
            int currentPosition = iter._reader.Position();
            DecodeBaseKey(iter, requestMsg.MsgKey);

            // add keySize to position
            currentPosition += keySize;
            iter._reader.Position(currentPosition);

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
        private static void DecodeCloseMsgHeader(DecodeIterator iter, ICloseMsg closeMsg)
        {
            closeMsg.Flags = iter._reader.ReadUShort15rb();
            // need to scale containerType
            closeMsg.ContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;

            if (closeMsg.CheckHasExtendedHdr())
            {
                DecodeBuffer8(iter, closeMsg.ExtendedHeader);
            }
        }

        private static void DecodeStateInMsg(DecodeIterator iterInt, State stateInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            State state = stateInt;
            sbyte stateVal = 0;

            stateVal = iter._reader.ReadByte();

            /* only take lowest three bits */
            state.DataState(stateVal & 0x7);

            /* now shift and take top five bits */
            state.StreamState(stateVal >> 3);

            state.Code(iter._reader.ReadByte());
            DecodeBuffer15(iter, state.Text());
        }


        internal static CodecReturnCode DecodeState(DecodeIterator iterInt, State valueInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            State value = valueInt;
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;
            int stateVal;

            if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) >= 3)
            {
                int savePosition = iter._reader.Position();
                try
                {
                    iter._reader.Position(iter._curBufPos);
                    stateVal = iter._reader.ReadByte();
                    value.Code(iter._reader.ReadByte());
                    DecodeBuffer15(iter, value.Text());
                    value.StreamState(stateVal >> 3);
                    value.DataState(stateVal & 0x7);
                    iter._reader.Position(savePosition);
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

        internal static CodecReturnCode DecodeBuffer(DecodeIterator iterInt, Buffer value)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
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
				(value).Data_internal(iter._buffer.Data(), iter._curBufPos, len);
			}

			return retVal;
		}

		private static void DecodeBuffer8(DecodeIterator iter, Buffer buf)
		{
			int len = iter._reader.ReadUnsignedByte();

			(buf).Data_internal(iter._reader.Buffer(), iter._reader.Position(), len);
			iter._reader.SkipBytes(len);
		}

		private static void DecodeBuffer15(DecodeIterator iter, Buffer buffer)
		{
			int len = iter._reader.ReadUShort15rb();

			buffer.Data_internal(iter._reader.Buffer(), iter._reader.Position(), len);
			iter._reader.SkipBytes(len);
		}

		private static void DecodeBuffer16(DecodeIterator iter, Buffer buf)
		{
			int temp = iter._reader.ReadUShort16ob();

			(buf).Data_internal(iter._reader.Buffer(), iter._reader.Position(), temp);
			iter._reader.SkipBytes(temp);
		}

    	private static void DecodeKeyEncAttrib(DecodeIterator iterInt, Buffer buffer)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			int tlen = 0;

			tlen = iter._reader.ReadUShort15rb();
			(buffer).Data_internal(iter._reader.Buffer(), iter._reader.Position(), tlen);
			iter._reader.SkipBytes(tlen);
		}

        private static void DecodeQosInMsg(DecodeIterator iterInt, Qos qosInt)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			Qos qos = qosInt;
			sbyte qosValue = 0;

			qosValue = iter._reader.ReadByte();

			qos._timeliness = (qosValue >> 5);
			qos._rate = ((qosValue >> 1) & 0xF);
			qos._dynamic = ((qosValue & 0x1) > 0 ? true : false);

			if (qos._timeliness > QosTimeliness.DELAYED_UNKNOWN)
			{
				qos._timeInfo = (iter._reader.ReadUnsignedShort());
			}
			else
			{
				qos._timeInfo = 0;
			}

			if (qos._rate > QosRates.JIT_CONFLATED)
			{
				qos._rateInfo = (iter._reader.ReadUnsignedShort());
			}
			else
			{
				qos._rateInfo = 0;
			}
		}

		internal static CodecReturnCode DecodeQos(DecodeIterator iterInt, Qos valueInt)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			Qos value = valueInt;
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;
			int qosValue;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) >= 1)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					qosValue = iter._reader.ReadByte();
					value._timeliness = (qosValue >> 5);
					value._rate = ((qosValue >> 1) & 0xF);
					value._dynamic = ((qosValue & 0x1) > 0 ? true : false);
					if (value._timeliness > QosTimeliness.DELAYED_UNKNOWN)
					{
						value._timeInfo = (iter._reader.ReadUnsignedShort());
					}
					else
					{
						value._timeInfo = 0;
					}
					if (value._rate > QosRates.JIT_CONFLATED)
					{
						value._rateInfo = (iter._reader.ReadUnsignedShort());
					}
					else
					{
						value._rateInfo = 0;
					}
					iter._reader.Position(savePosition);
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


        private static void DecodeBaseKey(DecodeIterator iterInt, IMsgKey key)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;

            ((MsgKey)key).Flags = iter._reader.ReadUShort15rb();

            if (key.CheckHasServiceId())
            {
                key.ServiceId = iter._reader.ReadUShort16ob();
            }

            if (key.CheckHasName())
            {
                /* take name off wire */
                DecodeBuffer8(iter, key.Name);

                /* name type is only present if name is there */
                if (key.CheckHasNameType())
                {
                    key.NameType = iter._reader.ReadUnsignedByte();
                }
            }

            if (key.CheckHasFilter())
            {
                key.Filter = iter._reader.ReadUnsignedInt();
            }

            if (key.CheckHasIdentifier())
            {
                key.Identifier = iter._reader.ReadInt();
            }

            if (key.CheckHasAttrib())
            {
                /* container type needs to be scaled back up */
                key.AttribContainerType = iter._reader.ReadUnsignedByte() + DataTypes.CONTAINER_TYPE_MIN;
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
        internal static void EndOfList(DecodeIterator iterInt)
		{
			/* Go back to previous level and check its type */
			DecodeIterator iter = (DecodeIterator)iterInt;
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

        internal static CodecReturnCode DecodeMsgKeyAttrib(DecodeIterator iterInt, IMsgKey key)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            /* Create a level to save the current iterator position.
			 * Once the decode of the opaque is finished, the iterator will be returned to this position. */
            DecodingLevel _levelInfo = null;

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
            iter._curBufPos = (key.EncodedAttrib).Position;
            try
            {
                iter._reader.Position(iter._curBufPos);
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
            iter._levelInfo[iter._decodingLevel + 1]._endBufPos = (key.EncodedAttrib).Position + key.EncodedAttrib.Length;

            /* Leave the current _levelInfo's _endBufPos where it is.
			 * It's not used in the opaque decoding, and it needs to be pointing to the correct position after the reset. */

            return CodecReturnCode.SUCCESS;
        }


        internal static CodecReturnCode DecodeElementList(DecodeIterator iterInt, ElementList elementListInt, LocalElementSetDefDb localSetDbInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;
            LocalElementSetDefDb localSetDb = localSetDbInt;
            ElementList elementList = elementListInt;

            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.ELEMENT_LIST, elementList);

            position = iter._curBufPos;
            try
            {
                iter._reader.Position(position);
                _endBufPos = _levelInfo._endBufPos;

                if (_endBufPos - position == 0)
                {
                    EndOfList(iter);
                    return CodecReturnCode.NO_DATA;
                }

                /* get flags */
                elementList.Flags = (ElementListFlags)iter._reader.ReadByte();

                /* get element list information */
                if (elementList.CheckHasInfo())
                {
                    int infoLen;
                    int startpos;

                    /* has 1 byte length */
                    infoLen = iter._reader.ReadUnsignedByte();
                    startpos = iter._reader.Position();

                    if ((startpos + infoLen) > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    elementList.ElementListNum = iter._reader.ReadUnsignedShort();

                    /* used info length to skip */
                    position = startpos + infoLen;
                    iter._reader.Position(position);
                }

                /* get set data */
                if (elementList.CheckHasSetData())
                {
                    /* get set id */
                    if (elementList.CheckHasSetId())
                    {
                        elementList.SetId = iter._reader.ReadUShort15rb();
                    }
                    else
                    {
                        elementList.SetId = 0;
                    }
                    position = iter._reader.Position();

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
                        _levelInfo._itemCount = iter._reader.ReadUnsignedShort();
                        position = iter._reader.Position();
                        (elementList._encodedEntries).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));
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

                        (elementList._encodedSetData).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));
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

                        _levelInfo._nextEntryPos = (_levelInfo._setCount > 0) ? (elementList._encodedSetData).Position : position; // iter._curBufPos =

                        return CodecReturnCode.SUCCESS;
                    }
                    else
                    {
                        _levelInfo._setCount = 0;
                        _levelInfo._nextEntryPos = position + elementList._encodedSetData.Length;
                        return CodecReturnCode.SET_SKIPPED;
                    }
                }
                else if (elementList.CheckHasStandardData())
                {
                    /* get element list data only */

                    _levelInfo._itemCount = iter._reader.ReadUnsignedShort();

                    (elementList._encodedEntries).Data_internal(iter._reader.Buffer(), iter._reader.Position(), (_endBufPos - iter._reader.Position()));
                    iter._curBufPos = iter._reader.Position();
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

        internal static CodecReturnCode DecodeElementEntry(DecodeIterator iterInt, ElementEntry elementInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            CodecReturnCode ret;
            int position;
            ElementList elementList;
            ElementEntry element = elementInt;

            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            Debug.Assert(null != element && null != iter, "Invalid parameters or parameters passed in as NULL");
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
                iter._reader.Position(position);
                if (_levelInfo._nextSetPosition < _levelInfo._setCount)
                {
                    ElementSetDefEntry encoding = null;
                    Debug.Assert(null != _levelInfo._elemListSetDef, "Invalid parameters or parameters passed in as NULL");
                    Debug.Assert(_levelInfo._elemListSetDef.Count == _levelInfo._setCount, "Invalid parameters or parameters passed in as NULL");

                    encoding = (ElementSetDefEntry)_levelInfo._elemListSetDef.Entries[_levelInfo._nextSetPosition];

                    (element._name).CopyReferences(encoding._name);
                    element.DataType = encoding.DataType;

                    /* get the set data and reset position */
                    if ((ret = DecodeSet(iter, encoding.DataType, element._encodedData)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._nextEntryPos;

                    /* handle legacy conversion */
                    element.DataType = ConvertToPrimitiveType(element.DataType);

                    _levelInfo._nextItemPosition++;
                    _levelInfo._nextSetPosition++;

                    if (_levelInfo._nextSetPosition == _levelInfo._setCount && elementList._encodedEntries.Length > 0)
                    {
                        _levelInfo._nextEntryPos = (elementList._encodedEntries).Position;
                    }

                    return CodecReturnCode.SUCCESS;
                }

                /* get normal element list data */
                if ((elementList._encodedEntries).Position + elementList._encodedEntries.Length - position < 3)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                DecodeBuffer15(iter, element._name);
                element.DataType = iter._reader.ReadUnsignedByte();

                if (element.DataType != DataTypes.NO_DATA)
                {
                    DecodeBuffer16(iter, element._encodedData);

                    if (iter._reader.Position() > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* handle legacy conversion */
                    element.DataType = ConvertToPrimitiveType(element.DataType);

                    /* shift iterator */
                    iter._curBufPos = (element._encodedData).Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = iter._reader.Position();
                    _levelInfo._nextItemPosition++;
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    element._encodedData.Clear();
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = iter._reader.Position();
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }

        internal static CodecReturnCode DecodeUInt(DecodeIterator iterInt, UInt value)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
					value.Value(iter._reader.ReadULong64ls(size));
					iter._reader.Position(savePosition);
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

		internal static CodecReturnCode DecodeInt(DecodeIterator iterInt, Int value)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
					value.Value(iter._reader.ReadLong64ls(size));
					iter._reader.Position(savePosition);
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

		internal static CodecReturnCode DecodeEnum(DecodeIterator iterInt, Enum value)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 0)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					int size = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos;
					value.Value(iter._reader.ReadUInt16ls(size));
					iter._reader.Position(savePosition);
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

		internal static CodecReturnCode DecodeDate(DecodeIterator iterInt, Date value)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 4)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					value.Day(iter._reader.ReadUnsignedByte());
					value.Month(iter._reader.ReadUnsignedByte());
					value.Year(iter._reader.ReadUnsignedShort());
					iter._reader.Position(savePosition);
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

		internal static CodecReturnCode DecodeTime(DecodeIterator iterInt, Time valueInt)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			Time value = valueInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
			int savePosition = iter._reader.Position();

			switch (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos)
			{
				case 8:
					try
					{
						int tempMicro;
						int tempNano;
						iter._reader.Position(iter._curBufPos);
						value._hour = (iter._reader.ReadUnsignedByte());
						value._minute = (iter._reader.ReadUnsignedByte());
						value._second = (iter._reader.ReadUnsignedByte());
						value._millisecond = (iter._reader.ReadUnsignedShort());
						tempMicro = (iter._reader.ReadUnsignedShort());
						tempNano = (iter._reader.ReadUnsignedByte());
						value._microsecond = (tempMicro & 0x000007FF);
						value._nanosecond = (((tempMicro & 0x00003800) >> 3) + tempNano);

						iter._reader.Position(savePosition);
					}
					catch (Exception)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}
					if (value.IsBlank)
					{
						ret = CodecReturnCode.BLANK_DATA;
					}
					break;
				case 7:
					try
					{
						iter._reader.Position(iter._curBufPos);
						value._hour = (iter._reader.ReadUnsignedByte());
						value._minute = (iter._reader.ReadUnsignedByte());
						value._second = (iter._reader.ReadUnsignedByte());
						value._millisecond = (iter._reader.ReadUnsignedShort());
						value._microsecond = (iter._reader.ReadUnsignedShort());
						value._nanosecond = 0;
						iter._reader.Position(savePosition);
					}
					catch (Exception)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}
					if ((value._hour == Time.BLANK_HOUR) && (value._minute == Time.BLANK_MINUTE) && (value._second == Time.BLANK_SECOND) && (value._millisecond == Time.BLANK_MILLI) && (value._microsecond == Time.BLANK_MICRO_NANO))
					{
						value._nanosecond = (Time.BLANK_MICRO_NANO);
						ret = CodecReturnCode.BLANK_DATA;
					}
					break;
				case 5:
					try
					{
						iter._reader.Position(iter._curBufPos);
						value._hour = (iter._reader.ReadUnsignedByte());
						value._minute = (iter._reader.ReadUnsignedByte());
						value._second = (iter._reader.ReadUnsignedByte());
						value._millisecond = (iter._reader.ReadUnsignedShort());
						value._microsecond = 0;
						value._nanosecond = 0;
						iter._reader.Position(savePosition);
					}
					catch (Exception)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}
					if ((value._hour == Time.BLANK_HOUR) && (value._minute == Time.BLANK_MINUTE) && (value._second == Time.BLANK_SECOND) && (value._millisecond == Time.BLANK_MILLI))
					{
						value._microsecond = (Time.BLANK_MICRO_NANO);
						value._nanosecond = (Time.BLANK_MICRO_NANO);
						ret = CodecReturnCode.BLANK_DATA;
					}
					break;
				case 3:
					try
					{
						iter._reader.Position(iter._curBufPos);
						value.Clear();
						value._hour = (iter._reader.ReadUnsignedByte());
						value._minute = (iter._reader.ReadUnsignedByte());
						value._second = (iter._reader.ReadUnsignedByte());
						iter._reader.Position(savePosition);
					}
					catch (Exception)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}
					if ((value._hour == Time.BLANK_HOUR) && (value._minute == Time.BLANK_MINUTE) && (value._second == Time.BLANK_SECOND))
					{
						value._millisecond = (Time.BLANK_MILLI);
						value._microsecond = (Time.BLANK_MICRO_NANO);
						value._nanosecond = (Time.BLANK_MICRO_NANO);
						ret = CodecReturnCode.BLANK_DATA;
					}
					break;
				case 2:
					try
					{
						iter._reader.Position(iter._curBufPos);
						value.Clear();
						value._hour = (iter._reader.ReadUnsignedByte());
						value._minute = (iter._reader.ReadUnsignedByte());
						iter._reader.Position(savePosition);
					}
					catch (Exception)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}
					if ((value._hour == Time.BLANK_HOUR) && (value._minute == Time.BLANK_MINUTE))
					{
						value._second = (Time.BLANK_SECOND);
						value._millisecond = (Time.BLANK_MILLI);
						value._microsecond = (Time.BLANK_MICRO_NANO);
						value._nanosecond = (Time.BLANK_MICRO_NANO);
						ret = CodecReturnCode.BLANK_DATA;
					}
					break;
				case 0:
					value.Blank();
					ret = CodecReturnCode.BLANK_DATA;
					break;
				default:
					ret = CodecReturnCode.INCOMPLETE_DATA;
					break;
			}
			return ret;
		}

		private static bool MakeSecMillisBlankCheckDateBlank(DateTime value)
		{
			value.Time().Second(Time.BLANK_SECOND);
			value.Time().Millisecond(Time.BLANK_MILLI);
			value.Time().Microsecond(Time.BLANK_MICRO_NANO);
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

		private static CodecReturnCode MakeSecMillisZeros(DateTime value)
		{
			value.Time().Second(0);
			value.Time().Millisecond(0);
			value.Time().Microsecond(0);
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

		private static bool MakeMillisBlankCheckDateBlank(DateTime value)
		{
			value.Time().Millisecond(Time.BLANK_MILLI);
			value.Time().Microsecond(Time.BLANK_MICRO_NANO);
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

		private static CodecReturnCode MakeMillisZeros(DateTime value)
		{
			value.Time().Millisecond(0);
			value.Time().Microsecond(0);
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

		private static bool MakeMicroNanoBlankCheckDateBlank(DateTime value)
		{
			value.Time().Microsecond(Time.BLANK_MICRO_NANO);
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

		private static CodecReturnCode MakeMicroNanoZeros(DateTime value)
		{
			value.Time().Microsecond(0);
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

		private static bool MakeNanoBlankCheckDateBlank(DateTime value)
		{
			value.Time().Nanosecond(Time.BLANK_MICRO_NANO);
			return (value.Date().Day() == 0) && (value.Date().Month() == 0) && (value.Date().Year() == 0);
		}

		private static CodecReturnCode MakeNanoZeros(DateTime value)
		{
			value.Time().Nanosecond(0);
			return CodecReturnCode.SUCCESS;
		}

		internal static CodecReturnCode DecodeDateTime(DecodeIterator iterInt, DateTime valueInt)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DateTime value = valueInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
			int savePosition = iter._reader.Position();

			switch (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos)
			{
				case 0:
					value.Blank();
					ret = CodecReturnCode.BLANK_DATA;
					return ret;

				case 6:
					try
					{
						iter._reader.Position(iter._curBufPos);
						value.Day(iter._reader.ReadUnsignedByte());
						value.Month(iter._reader.ReadUnsignedByte());
						value.Year(iter._reader.ReadUnsignedShort());
						value.Time().Hour(iter._reader.ReadUnsignedByte());
						value.Time().Minute(iter._reader.ReadUnsignedByte());

						/* If the time we took from wire matches blank, fill in rest of time as blank,
						 * then ensure that date portion is also blank - if so, return blank data.
						 * If time portion was not blank, just return success. */
						ret = (((value.Time().Hour() == Time.BLANK_HOUR && value.Time().Minute() == Time.BLANK_MINUTE)) ? (MakeSecMillisBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeSecMillisZeros(value));

						iter._reader.Position(savePosition);

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
						iter._reader.Position(iter._curBufPos);
						value.Day(iter._reader.ReadUnsignedByte());
						value.Month(iter._reader.ReadUnsignedByte());
						value.Year(iter._reader.ReadUnsignedShort());
						value.Time().Hour(iter._reader.ReadUnsignedByte());
						value.Time().Minute(iter._reader.ReadUnsignedByte());
						value.Time().Second(iter._reader.ReadUnsignedByte());

						/* need this to populate rest of time properly */
						/* If the time we took from wire matches blank, fill in rest of time as blank,
						 * then ensure that date portion is also blank - if so, return blank data. */
						/* If time portion was not blank, just return success */
						ret = (((value.Time().Hour() == Time.BLANK_HOUR && value.Time().Minute() == Time.BLANK_MINUTE && value.Time().Second() == Time.BLANK_SECOND)) ? (MakeMillisBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeMillisZeros(value));

						iter._reader.Position(savePosition);

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
						iter._reader.Position(iter._curBufPos);
						value.Day(iter._reader.ReadUnsignedByte());
						value.Month(iter._reader.ReadUnsignedByte());
						value.Year(iter._reader.ReadUnsignedShort());
						value.Time().Hour(iter._reader.ReadUnsignedByte());
						value.Time().Minute(iter._reader.ReadUnsignedByte());
						value.Time().Second(iter._reader.ReadUnsignedByte());
						value.Time().Millisecond(iter._reader.ReadUnsignedShort());

						ret = (((value.Time().Hour() == Time.BLANK_HOUR) && (value.Time().Minute() == Time.BLANK_MINUTE) && (value.Time().Second() == Time.BLANK_SECOND) && (value.Time().Millisecond() == Time.BLANK_MILLI)) ? (MakeMicroNanoBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeMicroNanoZeros(value));

						iter._reader.Position(savePosition);

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
						iter._reader.Position(iter._curBufPos);
						value.Day(iter._reader.ReadUnsignedByte());
						value.Month(iter._reader.ReadUnsignedByte());
						value.Year(iter._reader.ReadUnsignedShort());
						value.Time().Hour(iter._reader.ReadUnsignedByte());
						value.Time().Minute(iter._reader.ReadUnsignedByte());
						value.Time().Second(iter._reader.ReadUnsignedByte());
						value.Time().Millisecond(iter._reader.ReadUnsignedShort());
						value.Time().Microsecond(iter._reader.ReadUnsignedShort());

						ret = (((value.Time().Hour() == Time.BLANK_HOUR) && (value.Time().Minute() == Time.BLANK_MINUTE) && (value.Time().Second() == Time.BLANK_SECOND) && (value.Time().Millisecond() == Time.BLANK_MILLI) && (value.Time().Microsecond() == Time.BLANK_MICRO_NANO)) ? (MakeNanoBlankCheckDateBlank(value) ? CodecReturnCode.BLANK_DATA : CodecReturnCode.SUCCESS) : MakeNanoZeros(value));

						iter._reader.Position(savePosition);

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
						iter._reader.Position(iter._curBufPos);
						value.Day(iter._reader.ReadUnsignedByte());
						value.Month(iter._reader.ReadUnsignedByte());
						value.Year(iter._reader.ReadUnsignedShort());
						value.Time().Hour(iter._reader.ReadUnsignedByte());
						value.Time().Minute(iter._reader.ReadUnsignedByte());
						value.Time().Second(iter._reader.ReadUnsignedByte());
						value.Time().Millisecond(iter._reader.ReadUnsignedShort());
						tempMicro = (iter._reader.ReadUnsignedShort());
						tempNano = (iter._reader.ReadUnsignedByte());
						value.Time().Microsecond(tempMicro & 0x000007FF);
						value.Time().Nanosecond(((tempMicro & 0x00003800) >> 3) + tempNano);

						iter._reader.Position(savePosition);

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

		internal static CodecReturnCode DecodeReal(DecodeIterator iterInt, Real valueInt)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			Real value = valueInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 1)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					int hint = iter._reader.ReadByte();
					switch (hint & 0x3F)
					{
						case Real.BLANK_REAL:
							value.Blank();
							ret = CodecReturnCode.BLANK_DATA;
							break;
						case RealHints.INFINITY:
						case RealHints.NEG_INFINITY:
						case RealHints.NOT_A_NUMBER:
							value.Value(0, (hint & 0x3F));
							ret = CodecReturnCode.SUCCESS;
							break;
						default:
							int length = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos - 1;
							value.Value(iter._reader.ReadLong64ls(length), ((sbyte)(hint & 0x1F)));
						break;
					}
					iter._reader.Position(savePosition);
				}
				catch (Exception)
				{
					return CodecReturnCode.INVALID_ARGUMENT;
				}
			}
			else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 1)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					int hint = iter._reader.ReadByte();
					switch (hint & 0x3F)
					{
						case RealHints.INFINITY:
						case RealHints.NEG_INFINITY:
						case RealHints.NOT_A_NUMBER:
							value.Value(0, (hint & 0x3F));
							ret = CodecReturnCode.SUCCESS;
							break;
						default:
							value.Blank();
							ret = CodecReturnCode.BLANK_DATA;
						break;
					}
					iter._reader.Position(savePosition);
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

        internal static CodecReturnCode DecodeMap(DecodeIterator iterInt, Map mapInt)
        {
            Debug.Assert(null != iterInt, "Invalid IterInt in as NULL");
            Debug.Assert(null != mapInt, "Invalid MapInt in as NULL");
            DecodeIterator iter = (DecodeIterator)iterInt;
            Map map = mapInt;
            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;

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
                iter._reader.Position(position);

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
                map.Flags = (MapFlags)(iter._reader.ReadByte());
                map.KeyPrimitiveType = (iter._reader.ReadUnsignedByte());
                map.ContainerType = (iter._reader.ReadUnsignedByte());
                /* container type needs to be scaled back */
                map.ContainerType += DataTypes.CONTAINER_TYPE_MIN;

                /* Handle legacy conversion */
                map.KeyPrimitiveType = ConvertToPrimitiveType(map.KeyPrimitiveType);

                if (map.CheckHasKeyFieldId())
                {
                    map.KeyFieldId = (iter._reader.ReadShort());
                    position = iter._reader.Position();

                    if (position > _endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                }

                if (map.CheckHasSetDefs())
                {
                    DecodeBuffer15(iter, map._encodedSetDefs);
                    position = iter._reader.Position();

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
                    position = iter._reader.Position();

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
                    map.TotalCountHint = (iter._reader.ReadUInt30rb());
                }

                _levelInfo._itemCount = iter._reader.ReadUnsignedShort();

                position = iter._reader.Position();

                (map._encodedEntries).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));

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
            iter._curBufPos = map.CheckHasSummaryData() ? (map._encodedSummaryData).Position : position;
            return CodecReturnCode.SUCCESS;
        }

        internal static CodecReturnCode DecodeMapEntry(DecodeIterator iterInt, MapEntry mapEntryInt, object keyData)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            MapEntry mapEntry = mapEntryInt;
            int position;
            Map map;
            int flags;
            CodecReturnCode ret;
            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            Debug.Assert(null != mapEntry && null != iter, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(null != iter._levelInfo[iter._decodingLevel]._listType, "Invalid decoding attempted");

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
                iter._reader.Position(position);

                if ((position + 2) > _levelInfo._endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* take out action/flags */
                flags = iter._reader.ReadByte();

                mapEntry.Action = (MapEntryActions)((flags & 0xF));
                mapEntry.Flags = (MapEntryFlags)(flags >> 4);

                /* get perm data */
                if ((map.CheckHasPerEntryPermData()) && (mapEntry.CheckHasPermData()))
                {
                    DecodeBuffer15(iter, mapEntry._permData);
                    position = iter._reader.Position();
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
                position = iter._reader.Position();

                /* User provided storage for decoded key, so decode it for them */
                if (keyData != null)
                {
                    iter._levelInfo[iter._decodingLevel]._nextEntryPos = iter._curBufPos = mapEntry._encodedKey.Length > 0 ? (mapEntry._encodedKey).Position : position;
                    iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    if ((ret = DecodePrimitiveType(iter, map.KeyPrimitiveType, keyData)) < 0)
                    {
                        return ret;
                    }
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
                    position = iter._reader.Position();
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* shift iterator */
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = (mapEntry._encodedData).Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }

        private static CodecReturnCode DecodePrimitiveType(DecodeIterator iter, int type, object data)
		{
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;

			switch (type)
			{
				case DataTypes.UINT:
					DecodeUInt(iter, (UInt)data);
					break;
				case DataTypes.INT:
					DecodeInt(iter, (Int)data);
					break;
				case DataTypes.FLOAT:
					DecodeFloat(iter, (Float)data);
					break;
				case DataTypes.DOUBLE:
					DecodeDouble(iter, (Double)data);
					break;
				case DataTypes.REAL:
					DecodeReal(iter, (Real)data);
					break;
				case DataTypes.DATE:
					DecodeDate(iter, (Date)data);
					break;
				case DataTypes.TIME:
					DecodeTime(iter, (Time)data);
					break;
				case DataTypes.DATETIME:
					DecodeDateTime(iter, (DateTime)data);
					break;
				case DataTypes.QOS:
					DecodeQos(iter, (Qos)data);
					break;
				case DataTypes.STATE:
					DecodeState(iter, (State)data);
					break;
				case DataTypes.ENUM:
					DecodeEnum(iter, (Enum)data);
					break;
				case DataTypes.BUFFER:
				case DataTypes.ASCII_STRING:
				case DataTypes.UTF8_STRING:
				case DataTypes.RMTES_STRING:
					((Buffer)data).Data_internal(iter._reader.Buffer(), iter._curBufPos, (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos));
					break;
				default:
					retVal = CodecReturnCode.UNSUPPORTED_DATA_TYPE;
					break;
			}

			return retVal;
		}

		private static CodecReturnCode DecodeSet(DecodeIterator iterInt, int type, object data)
		{
			switch (type)
			{
				case DataTypes.INT_1:
				case DataTypes.UINT_1:
					Dec8(iterInt, (Buffer)data);
					break;
				case DataTypes.INT_2:
				case DataTypes.UINT_2:
					Dec16(iterInt, (Buffer)data);
					break;
				case DataTypes.TIME_3:
					Dec24(iterInt, (Buffer)data);
					break;
				case DataTypes.INT_4:
				case DataTypes.UINT_4:
				case DataTypes.FLOAT_4:
				case DataTypes.DATE_4:
					Dec32(iterInt, (Buffer)data);
					break;
				case DataTypes.TIME_5:
					Dec40(iterInt, (Buffer)data);
					break;
				case DataTypes.DATETIME_7:
				case DataTypes.TIME_7:
					Dec56(iterInt, (Buffer)data);
					break;
				case DataTypes.INT_8:
				case DataTypes.UINT_8:
				case DataTypes.DOUBLE_8:
				case DataTypes.TIME_8:
					Dec64(iterInt, (Buffer)data);
					break;
				case DataTypes.DATETIME_9:
					Dec72(iterInt, (Buffer)data);
					break;
				case DataTypes.DATETIME_11:
					Dec88(iterInt, (Buffer)data);
					break;
				case DataTypes.DATETIME_12:
					Dec96(iterInt, (Buffer)data);
					break;
				case DataTypes.ENUM:
				case DataTypes.ARRAY:
					DecBuf16(iterInt, (Buffer)data);
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
					DecBuf16(iterInt, (Buffer)data);
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
					DecBuf8(iterInt, (Buffer)data);
					break;
				case DataTypes.REAL_4RB:
					DecReal_4rb(iterInt, (Buffer)data);
					break;
				case DataTypes.REAL_8RB:
					DecReal_8rb(iterInt, (Buffer)data);
					break;
				default:
					return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
			}
			return CodecReturnCode.SUCCESS;
		}


        internal static CodecReturnCode DecodeFilterList(DecodeIterator iterInt, FilterList filterListInt)
        {
            Debug.Assert(null != filterListInt, "Invalid filterListInt passed in as NULL");
            Debug.Assert(null != iterInt, "Invalid iterInt passed in as NULL");
            DecodeIterator iter = (DecodeIterator)iterInt;
            FilterList filterList = (FilterList)filterListInt;
            int position;
            int _endBufPos;
            int count;
            DecodingLevel _levelInfo;

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
                iter._reader.Position(position);

                if (_endBufPos - position == 0)
                {
                    EndOfList(iter);
                    return CodecReturnCode.NO_DATA;
                }
                else if (_endBufPos - position < 3)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                filterList.Flags = (FilterListFlags)(iter._reader.ReadByte());
                filterList.ContainerType = (iter._reader.ReadUnsignedByte());
                /* needs to be scaled back after decoding */
                filterList.ContainerType += DataTypes.CONTAINER_TYPE_MIN;

                if (filterList.CheckHasTotalCountHint())
                {
                    filterList.TotalCountHint = iter._reader.ReadUnsignedByte();
                }
                else
                {
                    filterList.TotalCountHint = 0;
                }

                count = iter._reader.ReadUnsignedByte();
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo._itemCount = count;

            position = iter._reader.Position();

            (filterList._encodedEntries).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));

            /* Check for buffer overflow. Post check ok since no copy */
            if (position > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            iter._curBufPos = _levelInfo._nextEntryPos = position;

            return CodecReturnCode.SUCCESS;
        }

        internal static CodecReturnCode DecodeFilterEntry(DecodeIterator iterInt, FilterEntry filterEntryInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            FilterEntry filterEntry = (FilterEntry)filterEntryInt;
            int position;
            FilterList filterList;
            int flags;
            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            Debug.Assert(null != filterEntry && null != iter, "Invalid parameters or parameters passed in as NULL");
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
                iter._reader.Position(position);

                if ((position + 2) > _levelInfo._endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* take out flags */
                flags = iter._reader.ReadByte();

                filterEntry.Action = (FilterEntryActions)(flags & 0xF);
                filterEntry.Flags = (FilterEntryFlags)(flags >> 4);

                /* parse FilterEntry */
                filterEntry.Id = (iter._reader.ReadUnsignedByte());
                position = iter._reader.Position();

                if (filterEntry.CheckHasContainerType())
                {
                    if ((position + 1) > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }
                    filterEntry.ContainerType = (iter._reader.ReadUnsignedByte());
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
                    position = iter._reader.Position();

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
                    position = iter._reader.Position();
                    if (position > _levelInfo._endBufPos)
                    {
                        return CodecReturnCode.INCOMPLETE_DATA;
                    }

                    /* shift iterator */
                    _levelInfo._nextItemPosition++;
                    iter._curBufPos = (filterEntry._encodedData).Position;
                    _levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    position = iter._reader.Position();
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

        internal static CodecReturnCode DecodeArray(DecodeIterator iterInt, Array arrayInt)
        {
            Array array = arrayInt;
            DecodeIterator iter = (DecodeIterator)iterInt;
            DecodingLevel _levelInfo;
            int _endBufPos;

            Debug.Assert(null != array && null != iter, "Invalid parameters or parameters passed in as NULL");

            _levelInfo = iter._levelInfo[++iter._decodingLevel];
            if (iter._decodingLevel >= DecodeIterator.DEC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            iter.Setup(_levelInfo, DataTypes.ARRAY, array);

            _endBufPos = _levelInfo._endBufPos;
            try
            {
                iter._reader.Position(iter._curBufPos);

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
                array._primitiveType = iter._reader.ReadUnsignedByte();
                /* extract itemLength */
                array._itemLength = iter._reader.ReadUShort16ob();
                /* extract count */
                _levelInfo._itemCount = iter._reader.ReadUnsignedShort();
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            iter._curBufPos = iter._reader.Position();

            _levelInfo._nextEntryPos = iter._curBufPos;
            (array._encodedData).Data_internal(iter._reader.Buffer(), iter._curBufPos, (_endBufPos - iter._curBufPos));

            /* handle legacy types */
            array._primitiveType = ConvertToPrimitiveType(array._primitiveType);

            /* Check for buffer overflow. Post check ok since no copy */
            if (iter._curBufPos > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        internal static CodecReturnCode DecodeArrayEntry(DecodeIterator iterInt, ArrayEntry arrayEntry)
        {
            Debug.Assert(null != arrayEntry, "Invalid arrayEntry passed in as NULL");
            Debug.Assert(null != iterInt, "Invalid iterInt passed in as NULL");

            DecodeIterator iter = (DecodeIterator)iterInt;
            Array array;
            DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];

            iter._curBufPos = levelInfo._nextEntryPos;
            try
            {
                iter._reader.Position(iter._curBufPos);
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

                (arrayEntry.EncodedData).Data_internal(iter._reader.Buffer(), iter._curBufPos, len);
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


        private static int DecodePrimitiveLength(DecodeIterator iter, int type)
		{
			int len = lens[type];
			switch (len)
			{
				case 0:
					// variable len
					len = iter._reader.ReadUShort16ob();
					iter._curBufPos = iter._reader.Position();
					break;
				case -1:
					// REAL_4RB
					len = ((iter._reader.ReadByte() & 0xC0) >> 6) + 1;
					iter._reader.Position(iter._curBufPos); // rewind one byte
					break;
				case -2:
					// REAL_8RB
					// shift by 5 is intentional. Each combo represents 2 bytes here
					// so to shift by 5 instead of 6 accounts for this.
					len = ((iter._reader.ReadByte() & 0xC0) >> 5) + 2;
					iter._reader.Position(iter._curBufPos); // rewind one byte
					break;
				default:
					// len is set to the value in the len array
			break;
			}

			return len;
		}

		internal static CodecReturnCode DecodeFieldList(DecodeIterator iterInt, FieldList fieldListInt, LocalFieldSetDefDb localFieldSetDeb)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			int position;
			int _endBufPos;
			DecodingLevel _levelInfo;
			FieldList fieldList = (FieldList)fieldListInt;

			Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");

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
				iter._reader.Position(position);

				if (_endBufPos - position == 0)
				{
					EndOfList(iter);
					return CodecReturnCode.NO_DATA;
				}

				/* Get Flags */
				fieldList.Flags = (FieldListFlags)iter._reader.ReadByte();

				/* Get the Field List Information */
				if (fieldList.CheckHasInfo())
				{
					int infoLen;
					int startpos;

					/* Has 1 byte length */
					infoLen = iter._reader.ReadUnsignedByte();
					startpos = iter._reader.Position();

					fieldList.DictionaryId = iter._reader.ReadUShort15rb();
					fieldList.FieldListNum = iter._reader.ReadUnsignedShort();

					if ((startpos + infoLen) > _endBufPos)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}

					/* Used info Length to skip */
					position = startpos + infoLen;
					iter._reader.Position(position);
				}

				/* Get the Field List Set Data */
				if (fieldList.CheckHasSetData())
				{
					/* Get the set identifier */
					if (fieldList.CheckHasSetId())
					{
						fieldList.SetId = iter._reader.ReadUShort15rb();
					}
					else
					{
						fieldList.SetId = 0;
					}
					position = iter._reader.Position();

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
                            _levelInfo._fieldListSetDef = (FieldSetDef)iter._fieldSetDefDb.Definitions[fieldList.SetId];
                        }
                        else
                        {
                            _levelInfo._fieldListSetDef = null;
                        }
                    }

                    /* Check for field list data */
                    if (fieldList.CheckHasStandardData())
					{
						/* If HasSetData and HasFieldList, then set data is length specified. */
						DecodeBuffer15(iter, fieldList._encodedSetData);

						/* Get the Field List Data */
						_levelInfo._itemCount = iter._reader.ReadUnsignedShort();
						position = iter._reader.Position();
						(fieldList._encodedEntries).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));

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

						(fieldList._encodedSetData).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));

						if (position > _endBufPos)
						{
							return CodecReturnCode.INCOMPLETE_DATA;
						}
					}

					/* Setup to decode the set if able. Otherwise skip to entries. */
					if (_levelInfo._fieldListSetDef != null)
					{
						_levelInfo._setCount = (int)_levelInfo._fieldListSetDef.Count;
						_levelInfo._itemCount += _levelInfo._fieldListSetDef.Count;

						_levelInfo._nextEntryPos = (_levelInfo._setCount > 0) ? (fieldList._encodedSetData).Position : position; // iter._curBufPos =

						return CodecReturnCode.SUCCESS;
					}
					else
					{
						_levelInfo._setCount = 0;
						_levelInfo._nextEntryPos = position + fieldList._encodedSetData.Length;
						return CodecReturnCode.SET_SKIPPED;
					}
				}
				else if (fieldList.CheckHasStandardData())
				{
					/* Get the field list data only */
					fieldList._encodedSetData.Clear();

					_levelInfo._itemCount = iter._reader.ReadUnsignedShort();
					position = iter._reader.Position();
					(fieldList._encodedEntries).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));

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

		internal static CodecReturnCode DecodeFieldEntry(DecodeIterator iterInt, FieldEntry fieldInt)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			FieldEntry field = (FieldEntry)fieldInt;
            CodecReturnCode ret;
			int position;
			FieldList fieldList;
			DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

			Debug.Assert(null != field && null != iter, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(null != _levelInfo._listType, "Invalid decoding attempted");

			fieldList = (FieldList)_levelInfo._listType;

			if (_levelInfo._nextItemPosition >= _levelInfo._itemCount)
			{
				EndOfList(iter);
				return CodecReturnCode.END_OF_CONTAINER;
			}

			/* Make sure we skip to the next entry if we didn't decode the previous entry payload */
			position = iter._curBufPos = _levelInfo._nextEntryPos;
			if (position < iter._reader.Buffer().Limit) // Make sure we are reading only if we can read something at all
			{
				try
				{
					iter._reader.Position(position);

					if (_levelInfo._nextSetPosition < _levelInfo._setCount)
					{
						FieldSetDefEntry encoding = null;

						Debug.Assert(null != _levelInfo._fieldListSetDef, "Invalid parameters or parameters passed in as NULL");
						Debug.Assert(_levelInfo._fieldListSetDef.Count == _levelInfo._setCount, "Invalid data");

						encoding = _levelInfo._fieldListSetDef.Entries[_levelInfo._nextSetPosition];

						field.FieldId = (int)encoding.FieldId;
						field.DataType = (int)encoding.DataType;

						/* Get the set data and reset position */
						if ((ret = DecodeSet(iter, (int)encoding.DataType, field._encodedData)) != CodecReturnCode.SUCCESS)
						{
							return (ret);
						}

						iter._levelInfo[iter._decodingLevel + 1]._endBufPos = _levelInfo._nextEntryPos;

						/* handle legacy conversion */
						int dataType = ConvertToPrimitiveType(field.DataType);
						field.DataType = dataType;

						_levelInfo._nextItemPosition++;
						_levelInfo._nextSetPosition++;

						if (_levelInfo._nextSetPosition == _levelInfo._setCount && fieldList._encodedEntries.Length > 0)
						{
							_levelInfo._nextEntryPos = (fieldList._encodedEntries).Position;
						}

						return CodecReturnCode.SUCCESS;
					}

					/* Get normal field list data */
					if ((fieldList._encodedEntries).Position + fieldList._encodedEntries.Length - position < 3)
					{
						return CodecReturnCode.INCOMPLETE_DATA;
					}

					field.FieldId = iter._reader.ReadShort();
					field.DataType = DataTypes.UNKNOWN;

					/* parse Field */
					DecodeBuffer16(iter, field._encodedData);
				}
				catch (Exception)
				{
					return CodecReturnCode.INCOMPLETE_DATA;
				}
			}
			position = iter._reader.Position();
			if (position > _levelInfo._endBufPos)
			{
				return CodecReturnCode.INCOMPLETE_DATA;
			}

			/* shift iterator */
			iter._curBufPos = (field._encodedData).Position;
			_levelInfo._nextEntryPos = iter._levelInfo[iter._decodingLevel + 1]._endBufPos = position;
			_levelInfo._nextItemPosition++;
			return CodecReturnCode.SUCCESS;
		}


        internal static CodecReturnCode DecodeSeries(DecodeIterator iterInt, Series seriesInt)
        {
            Debug.Assert(null != iterInt, "Invalid IterInt in as NULL");
            Debug.Assert(null != seriesInt, "Invalid seriesInt in as NULL");
            DecodeIterator iter = (DecodeIterator)iterInt;
            Series series = (Series)seriesInt;
            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;

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
                iter._reader.Position(position);

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
                series.Flags = (SeriesFlags)(iter._reader.ReadByte());
                series.ContainerType = (iter._reader.ReadUnsignedByte());
                /* container type needs to be scaled back after decoding */
                series.ContainerType += DataTypes.CONTAINER_TYPE_MIN;

                if (series.CheckHasSetDefs())
                {
                    DecodeBuffer15(iter, series._encodedSetDefs);
                    position = iter._reader.Position();
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
                    position = iter._reader.Position();

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
                    series.TotalCountHint = (iter._reader.ReadUInt30rb());
                }
                else
                {
                    series.TotalCountHint = 0;
                }

                _levelInfo._itemCount = iter._reader.ReadUnsignedShort();
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            position = iter._reader.Position();

            (series._encodedEntries).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));

            /* check for overflow */
            if (position > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo._nextEntryPos = position; // set entry ptr to first entry
            iter._curBufPos = series.CheckHasSummaryData() ? (series._encodedSummaryData).Position : position;

            return CodecReturnCode.SUCCESS;
        }

        internal static CodecReturnCode DecodeLocalElementSetDefDb(DecodeIterator iterInt, LocalElementSetDefDb localSetDbInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            int position;
            int _endBufPos;
            int flags;
            int _setCount;
            int curEntryCount = 0;
            LocalElementSetDefDb localSetDb = localSetDbInt;

            Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");

            try
            {
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

                    position = (encodedSetDefs).Position;
                    iter._reader.Position(position);
                    _endBufPos = (encodedSetDefs).Position + encodedSetDefs.Length;
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

                flags = iter._reader.ReadByte();
                _setCount = iter._reader.ReadUnsignedByte();
                position = iter._reader.Position();

                /* System.out.println("ElementSetDefDb: flags " + flags + " count " + _setCount); */

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
                    setId = iter._reader.ReadUShort15rb();
                    encCount = iter._reader.ReadUnsignedByte();

                    /* System.out.println("  ElementSetDef: setId " + setId + " count " + encCount); */

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
                        curEntry.DataType = iter._reader.ReadUnsignedByte();
                        position = iter._reader.Position();
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

        internal static CodecReturnCode DecodeLocalFieldSetDefDb(DecodeIterator iterInt, LocalFieldSetDefDb localSetDbInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            LocalFieldSetDefDb localSetDb = localSetDbInt;
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

                    Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");

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

                    position = (encodedSetDefs).Position;
                    iter._reader.Position(position);
                    _endBufPos = (encodedSetDefs).Position + encodedSetDefs.Length;
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

                flags = iter._reader.ReadByte();
                _setCount = iter._reader.ReadUnsignedByte();
                position = iter._reader.Position();

                /* System.out.println("FieldSetDefDb: flags " + flags + " count " + _setCount); */

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
                    encCount = iter._reader.ReadUnsignedByte();

                    /* System.out.println("  FieldSetDef: setId " + setId + " count " + encCount); */

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
                        curEntry.FieldId = iter._reader.ReadUnsignedShort();
                        curEntry.DataType = iter._reader.ReadUnsignedByte();
                        position = iter._reader.Position();
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

        internal static CodecReturnCode DecodeSeriesEntry(DecodeIterator iterInt, SeriesEntry seriesEntryInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            SeriesEntry seriesEntry = seriesEntryInt;
            int position;
            Series series;
            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            Debug.Assert(null != seriesEntry && null != iter, "Invalid parameters or parameters passed in as NULL");
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
                iter._reader.Position(position);

                if (series.ContainerType != DataTypes.NO_DATA)
                {
                    DecodeBuffer16(iter, seriesEntry._encodedData);
                    position = iter._reader.Position();
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
                    iter._reader.Position(position);
                    return CodecReturnCode.SUCCESS;
                }
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }
        }


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

		internal static int GetItemCount(DecodeIterator iter)
		{
			return iter._levelInfo[iter._decodingLevel]._itemCount;
		}


        internal static CodecReturnCode DecodeVector(DecodeIterator iterInt, Vector vectorInt)
        {
            Debug.Assert(null != iterInt, "Invalid IterInt in as NULL");
            Debug.Assert(null != vectorInt, "Invalid vectorInt in as NULL");
            DecodeIterator iter = (DecodeIterator)iterInt;
            Vector vector = vectorInt;
            int position;
            int _endBufPos;
            DecodingLevel _levelInfo;

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
                iter._reader.Position(position);

                vector.Flags = (VectorFlags)(iter._reader.ReadByte());

                vector._containerType = (iter._reader.ReadUnsignedByte());
                /* container type needs to be scaled after its decoded */
                vector._containerType += DataTypes.CONTAINER_TYPE_MIN;

                if (vector.CheckHasSetDefs())
                {
                    DecodeBuffer15(iter, vector._encodedSetDefs);
                    position = iter._reader.Position();
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
                    position = iter._reader.Position();
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
                    vector._totalCountHint = (iter._reader.ReadUInt30rb());
                }
                else
                {
                    vector._totalCountHint = 0;
                }

                _levelInfo._itemCount = iter._reader.ReadUnsignedShort();
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            position = iter._reader.Position();

            (vector.EncodedEntries).Data_internal(iter._reader.Buffer(), position, (_endBufPos - position));

            /* check for buffer overflow. Post check ok since no copy */
            if (position > _endBufPos)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo._nextEntryPos = position; // set entry ptr to first entry
            iter._curBufPos = vector.CheckHasSummaryData() ? (vector._encodedSummaryData).Position : position;

            return CodecReturnCode.SUCCESS;
        }

        internal static CodecReturnCode DecodeVectorEntry(DecodeIterator iterInt, VectorEntry vectorEntryInt)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            VectorEntry vectorEntry = vectorEntryInt;
            int position;
            Vector vector;
            int flags;
            DecodingLevel _levelInfo = iter._levelInfo[iter._decodingLevel];

            Debug.Assert(null != vectorEntry && null != iter, "Invalid parameters or parameters passed in as NULL)");
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
                iter._reader.Position(position);

                if ((position + 2) > _levelInfo._endBufPos)
                {
                    return CodecReturnCode.INCOMPLETE_DATA;
                }

                /* take out action/flags */
                flags = iter._reader.ReadByte();

                vectorEntry.Action = (VectorEntryActions)(flags & 0xF);
                vectorEntry.Flags = (VectorEntryFlags)(flags >> 4);

                /* get index */
                vectorEntry._index = (uint)(iter._reader.ReadUInt30rb());

                /* get perm data */
                if ((vector.CheckHasPerEntryPermData()) && (vectorEntry.CheckHasPermData()))
                {
                    DecodeBuffer15(iter, vectorEntry._permData);
                    position = iter._reader.Position();
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
                    position = iter._reader.Position();
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
                    position = iter._reader.Position();
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


        private static CodecReturnCode DecBuf8(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int length;

			Debug.Assert(iter._curBufPos == levelInfo._nextEntryPos, "Invalid decoding attempted");

			/* Move _curBufPos and _endBufPos around data. _nextEntryPos should point after it. */
			length = iter._reader.ReadUnsignedByte();
			iter._curBufPos = iter._reader.Position();
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, length);
			levelInfo._nextEntryPos = iter._curBufPos + length;

			return (levelInfo._nextEntryPos <= levelInfo._endBufPos ? CodecReturnCode.SUCCESS : CodecReturnCode.INCOMPLETE_DATA);
		}

		private static void DecBuf16(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int tlen;

			Debug.Assert(iter._curBufPos == levelInfo._nextEntryPos, "Invalid decoding attempted");

			tlen = iter._reader.ReadUShort16ob();
			iter._curBufPos = iter._reader.Position();
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, tlen);
			levelInfo._nextEntryPos = iter._curBufPos + tlen;
		}

		private static void Dec8(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 1);
			levelInfo._nextEntryPos++;
		}

		private static void Dec16(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 2);
			levelInfo._nextEntryPos += 2;
		}

		private static void Dec24(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 3);
			levelInfo._nextEntryPos += 3;
		}

		private static void Dec32(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 4);
			levelInfo._nextEntryPos += 4;
		}

		private static void Dec40(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 5);
			levelInfo._nextEntryPos += 5;
		}

		private static void Dec56(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 7);
			levelInfo._nextEntryPos += 7;
		}

		private static void Dec64(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 8);
			levelInfo._nextEntryPos += 8;
		}

		private static void Dec72(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 9);
			levelInfo._nextEntryPos += 9;
		}

		private static void Dec88(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 11);
			levelInfo._nextEntryPos += 11;
		}

		private static void Dec96(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, 12);
			levelInfo._nextEntryPos += 12;
		}

		private static void DecReal_4rb(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int format = 0;

			format = iter._reader.ReadByte();

			int len;
			if ((format & 0x20) == 1)
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
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, len);
			levelInfo._nextEntryPos += data.Length;
		}

		private static void DecReal_8rb(DecodeIterator iterInt, Buffer data)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
			DecodingLevel levelInfo = iter._levelInfo[iter._decodingLevel];
			int format = 0;

			format = iter._reader.ReadByte();

			int len;
			if ((format & 0x20) == 1)
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
			(data).Data_internal(iter._reader.Buffer(), iter._curBufPos, len);
			levelInfo._nextEntryPos += data.Length;
		}

		internal static CodecReturnCode DecodeFloat(DecodeIterator iterInt, Float value)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 4)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					value.Value(iter._reader.ReadFloat());
					iter._reader.Position(savePosition);
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

		internal static CodecReturnCode DecodeDouble(DecodeIterator iterInt, Double value)
		{
			DecodeIterator iter = (DecodeIterator)iterInt;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

			if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 8)
			{
				int savePosition = iter._reader.Position();
				try
				{
					iter._reader.Position(iter._curBufPos);
					value.Value(iter._reader.ReadDouble());
					iter._reader.Position(savePosition);
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

        internal static CodecReturnCode DecodeFieldSetDefDb(DecodeIterator iterInt, FieldSetDefDb fieldSetDefDbImpl)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            FieldSetDefDb localSetDb = fieldSetDefDbImpl;
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

                    Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");

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

                    position = (encodedSetDefs).Position;
                    iter._reader.Position(position);
                    _endBufPos = (encodedSetDefs).Position + encodedSetDefs.Length;
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

                flags = iter._reader.ReadByte();
                _setCount = iter._reader.ReadUnsignedByte();
                position = iter._reader.Position();

                /* System.out.println("FieldSetDefDb: flags " + flags + " count " + _setCount); */

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
                    encCount = iter._reader.ReadUnsignedByte();

                    /* System.out.println("  FieldSetDef: setId " + setId + " count " + encCount); */

                    /* Basic sanity checks */
                    if (setId > fieldSetDefDbImpl.maxLocalId)
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
                        curEntry.FieldId = (iter._reader.ReadUnsignedShort());
                        curEntry.DataType = (iter._reader.ReadUnsignedByte());
                        position = iter._reader.Position();
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

        internal static CodecReturnCode DecodeElementSetDefDb(DecodeIterator iterInt, ElementSetDefDb elementSetDefDbImpl)
        {
            DecodeIterator iter = (DecodeIterator)iterInt;
            int position;
            int _endBufPos;
            int flags;
            int _setCount;
            int curEntryCount = 0;
            ElementSetDefDb localSetDb = elementSetDefDbImpl;

            Debug.Assert(null != iter && null != localSetDb, "Invalid parameters or parameters passed in as NULL");

            try
            {
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

                    position = (encodedSetDefs).Position;
                    iter._reader.Position(position);
                    _endBufPos = (encodedSetDefs).Position + encodedSetDefs.Length;
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

                flags = iter._reader.ReadByte();
                _setCount = iter._reader.ReadUnsignedByte();
                position = iter._reader.Position();

                /* System.out.println("ElementSetDefDb: flags " + flags + " count " + _setCount); */

                if (_setCount == 0)
                {
                    return CodecReturnCode.SET_DEF_DB_EMPTY;
                }

                if (_setCount > elementSetDefDbImpl.MAX_LOCAL_ID)
                {
                    return CodecReturnCode.TOO_MANY_LOCAL_SET_DEFS;
                }

                for (int i = 0; i <= elementSetDefDbImpl.MAX_LOCAL_ID; i++)
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
                    setId = iter._reader.ReadUShort15rb();
                    encCount = iter._reader.ReadUnsignedByte();

                    /* System.out.println("  ElementSetDef: setId " + setId + " count " + encCount); */

                    /* sanity checks */
                    if (setId > elementSetDefDbImpl.MAX_LOCAL_ID)
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
                        curEntry = (ElementSetDefEntry)localSetDb.Definitions[curEntryCount].Entries[j];
                        DecodeBuffer15(iter, curEntry.Name);
                        curEntry.DataType = iter._reader.ReadUnsignedByte();
                        position = iter._reader.Position();
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