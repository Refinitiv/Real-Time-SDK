/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using Xunit;
using Xunit.Categories;

namespace LSEG.Eta.Transports.Tests
{
    public class MessageTest
    {
        Msg msg = new Msg();
        /* Encode nested Msg */
        int g_dataFormat = DataTypes.FIELD_LIST;
        bool g_changeData = true;

        /* set/unset flag after encoding */
        bool g_setRefreshFlags = false;
        bool g_unsetRefreshFlags = false;
        bool g_setRequestFlags = false;
        bool g_unsetRequestFlags = false;

        int streamId = 2;
        System.String extendedHeader;
        int extendedHeaderLen;

        System.String exchangeId;
        int exchangeIdLen;

        System.String permissionData;
        int permissionDataLen;

        System.String text;
        int textLen;

        byte[] groupId = { 0, 3 };
        int groupIdLen;

        System.String stateText;
        int stateTextLen;

        long seqNum = 12345;

        int postId = 123456;

        /* Msg Key */
        System.String payloadName;
        int payloadNameLen;
        int opaqueLen;
        ByteBuffer opaque;

        private int repeatCount = 1;
        private bool commonSetupComplete = false, fullSetupComplete = false;
        //Using all primitives makes the top container too big to test nested structure to maximum depth 
        private const int FIELD_LIST_MAX_ENTRIES = 6;
        /* FIDs and primitives for field list */
        int[] fieldListFids =
        {
            1,
            2,
            245,
            255,
            256,
            32767
        };

        int[] fieldListDataTypes =
        {
            DataTypes.INT,
            DataTypes.DOUBLE,
            DataTypes.REAL,
            DataTypes.DATE,
            DataTypes.TIME,
            DataTypes.DATETIME
        };
        Int fieldListInt = new Int();
        Double fieldListFloat = new Double();
        Real fieldListReal = new Real();
        Date fieldListDate = new Date();
        Time fieldListTime = new Time();
        DateTime fieldListDateTime = new DateTime();

        /* Buffer sizes */
        private int dataBufferSize = 1024;
        private int msgBufferSize = 1024 + 512;

        /* Iterators and Buffers for writing/reading */
        Buffer encBuf = new Buffer();
        Buffer encDataBuf = new Buffer();

        EncodeIterator encIter = new EncodeIterator(),
                encDataIter = new EncodeIterator();
        DecodeIterator decIter = new DecodeIterator(),
                decDataIter = new DecodeIterator();

        private int masksSize, actionsSize, keyMasksSize;
        private int[] masks, actions, keyMasks;
        private int masksIter, actionsIter, keyMasksIter;
        private int[] actionsCommon =
        {
                ExtraTestAction.POST_PAYLOAD,
                ExtraTestAction.PRE_PAYLOAD,
                ExtraTestAction.FOUR_BYTE_SEQ,
                ExtraTestAction.POST_OPAQUE,
                ExtraTestAction.POST_EXTENDED
        };
        private int[] actionsAll =
        {
                ExtraTestAction.POST_PAYLOAD,
                ExtraTestAction.PRE_PAYLOAD,
                ExtraTestAction.FOUR_BYTE_SEQ,
                ExtraTestAction.TRIM_DATA_BUF,
                ExtraTestAction.POST_OPAQUE,
                ExtraTestAction.POST_EXTENDED
        };
        private int[] keyMasksCommon =
        {
                MsgKeyFlags.HAS_SERVICE_ID,
                MsgKeyFlags.HAS_NAME,
                MsgKeyFlags.HAS_NAME_TYPE,
                MsgKeyFlags.HAS_ATTRIB
        };
        private int[] keyMasksAll =
        {
                MsgKeyFlags.HAS_SERVICE_ID,
                MsgKeyFlags.HAS_NAME,
                MsgKeyFlags.HAS_NAME_TYPE,
                MsgKeyFlags.HAS_FILTER,
                MsgKeyFlags.HAS_IDENTIFIER,
                MsgKeyFlags.HAS_ATTRIB
        };
        private int[] updateMasksCommon =
        {
                UpdateMsgFlags.DO_NOT_RIPPLE,
                UpdateMsgFlags.DO_NOT_CACHE,
                UpdateMsgFlags.HAS_MSG_KEY,
                UpdateMsgFlags.HAS_PERM_DATA,
                UpdateMsgFlags.HAS_SEQ_NUM,
                UpdateMsgFlags.HAS_CONF_INFO
        };
        private int[] updateMasksAll =
        {
                UpdateMsgFlags.HAS_EXTENDED_HEADER,
                UpdateMsgFlags.HAS_PERM_DATA,
                UpdateMsgFlags.HAS_MSG_KEY,
                UpdateMsgFlags.HAS_SEQ_NUM,
                UpdateMsgFlags.HAS_CONF_INFO,
                UpdateMsgFlags.DO_NOT_CACHE,
                UpdateMsgFlags.DO_NOT_CONFLATE,
                UpdateMsgFlags.DO_NOT_RIPPLE,
                UpdateMsgFlags.HAS_POST_USER_INFO,
                UpdateMsgFlags.DISCARDABLE
        };

        private int[] responseMasksCommon =
        {
                RefreshMsgFlags.DO_NOT_CACHE,
                RefreshMsgFlags.CLEAR_CACHE,
                RefreshMsgFlags.REFRESH_COMPLETE,
                RefreshMsgFlags.HAS_MSG_KEY,
                RefreshMsgFlags.HAS_PERM_DATA,
                RefreshMsgFlags.HAS_SEQ_NUM,
                RefreshMsgFlags.HAS_QOS,
                RefreshMsgFlags.HAS_EXTENDED_HEADER
        };

        private int[] responseMasksAll =
        {
                RefreshMsgFlags.HAS_EXTENDED_HEADER,
                RefreshMsgFlags.HAS_PERM_DATA,
                RefreshMsgFlags.HAS_MSG_KEY,
                RefreshMsgFlags.HAS_SEQ_NUM,
                RefreshMsgFlags.SOLICITED,
                RefreshMsgFlags.REFRESH_COMPLETE,
                RefreshMsgFlags.HAS_QOS,
                RefreshMsgFlags.CLEAR_CACHE,
                RefreshMsgFlags.DO_NOT_CACHE,
                RefreshMsgFlags.PRIVATE_STREAM,
                RefreshMsgFlags.HAS_POST_USER_INFO,
                RefreshMsgFlags.HAS_PART_NUM
        };

        // make sure this list matches with MsgUtil.set/unset refresh masks
        private int[] responseSetUnSetMasksTests =
        {
                RefreshMsgFlags.NONE,
                RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE,
                RefreshMsgFlags.HAS_QOS,
                RefreshMsgFlags.HAS_QOS | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE,
                RefreshMsgFlags.HAS_PART_NUM,
                RefreshMsgFlags.HAS_PART_NUM | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE,
                RefreshMsgFlags.HAS_EXTENDED_HEADER,
                RefreshMsgFlags.HAS_EXTENDED_HEADER | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE,
                RefreshMsgFlags.HAS_EXTENDED_HEADER | RefreshMsgFlags.HAS_PERM_DATA | RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.HAS_SEQ_NUM | RefreshMsgFlags.SOLICITED
                        | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS | RefreshMsgFlags.CLEAR_CACHE | RefreshMsgFlags.DO_NOT_CACHE |
                        RefreshMsgFlags.PRIVATE_STREAM | RefreshMsgFlags.HAS_POST_USER_INFO | RefreshMsgFlags.HAS_PART_NUM
        };

        private int[] responseSetUnSetMasks =
        {
                RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE
        };

        private int[] requestMasksAll =
        {
                RequestMsgFlags.HAS_EXTENDED_HEADER,
                RequestMsgFlags.HAS_PRIORITY,
                RequestMsgFlags.STREAMING,
                RequestMsgFlags.MSG_KEY_IN_UPDATES,
                RequestMsgFlags.CONF_INFO_IN_UPDATES,
                RequestMsgFlags.NO_REFRESH,
                RequestMsgFlags.HAS_QOS,
                RequestMsgFlags.HAS_WORST_QOS,
                RequestMsgFlags.PRIVATE_STREAM
        };

        private int[] requestMasksCommon =
        {
                RequestMsgFlags.NO_REFRESH,
                RequestMsgFlags.STREAMING,
                RequestMsgFlags.HAS_PRIORITY,
                RequestMsgFlags.HAS_QOS,
                RequestMsgFlags.HAS_WORST_QOS,
                RequestMsgFlags.HAS_EXTENDED_HEADER,
                RequestMsgFlags.MSG_KEY_IN_UPDATES,
                RequestMsgFlags.CONF_INFO_IN_UPDATES
        };

        private int[] requestSetUnSetMasksTests =
        {
                RequestMsgFlags.NONE,
                RequestMsgFlags.NO_REFRESH | RequestMsgFlags.STREAMING | RequestMsgFlags.CONF_INFO_IN_UPDATES | RequestMsgFlags.MSG_KEY_IN_UPDATES,
                RequestMsgFlags.HAS_EXTENDED_HEADER,
                RequestMsgFlags.HAS_EXTENDED_HEADER | RequestMsgFlags.NO_REFRESH | RequestMsgFlags.STREAMING | RequestMsgFlags.CONF_INFO_IN_UPDATES | RequestMsgFlags.MSG_KEY_IN_UPDATES,
                RequestMsgFlags.HAS_WORST_QOS,
                RequestMsgFlags.HAS_WORST_QOS | RequestMsgFlags.NO_REFRESH | RequestMsgFlags.STREAMING | RequestMsgFlags.CONF_INFO_IN_UPDATES | RequestMsgFlags.MSG_KEY_IN_UPDATES,
                RequestMsgFlags.PRIVATE_STREAM,
                RequestMsgFlags.PRIVATE_STREAM | RequestMsgFlags.NO_REFRESH | RequestMsgFlags.STREAMING | RequestMsgFlags.CONF_INFO_IN_UPDATES | RequestMsgFlags.MSG_KEY_IN_UPDATES,
                RequestMsgFlags.HAS_EXTENDED_HEADER | RequestMsgFlags.HAS_PRIORITY | RequestMsgFlags.STREAMING | RequestMsgFlags.MSG_KEY_IN_UPDATES | RequestMsgFlags.CONF_INFO_IN_UPDATES |
                        RequestMsgFlags.NO_REFRESH | RequestMsgFlags.HAS_QOS | RequestMsgFlags.HAS_WORST_QOS | RequestMsgFlags.PRIVATE_STREAM
        };

        private int[] requestSetUnSetMasks =
        {
                RequestMsgFlags.NO_REFRESH | RequestMsgFlags.STREAMING | RequestMsgFlags.CONF_INFO_IN_UPDATES | RequestMsgFlags.MSG_KEY_IN_UPDATES
        };

        private int[] statusMasksAll =
        {
                StatusMsgFlags.HAS_EXTENDED_HEADER,
                StatusMsgFlags.HAS_PERM_DATA,
                StatusMsgFlags.HAS_MSG_KEY,
                StatusMsgFlags.HAS_GROUP_ID,
                StatusMsgFlags.HAS_STATE,
                StatusMsgFlags.CLEAR_CACHE,
                StatusMsgFlags.PRIVATE_STREAM,
                StatusMsgFlags.HAS_POST_USER_INFO
        };

        private int[] statusMasksCommon =
        {
                StatusMsgFlags.CLEAR_CACHE,
                StatusMsgFlags.HAS_STATE,
                StatusMsgFlags.HAS_PERM_DATA,
                StatusMsgFlags.HAS_EXTENDED_HEADER
        };

        private int[] closeMasksAll =
        {
                CloseMsgFlags.HAS_EXTENDED_HEADER,
                CloseMsgFlags.ACK
        };

        private int[] closeMasksCommon =
        {
                CloseMsgFlags.HAS_EXTENDED_HEADER
        };

        private int[] ackMasksAll =
        {
                AckMsgFlags.HAS_NAK_CODE,
                AckMsgFlags.HAS_TEXT,
                AckMsgFlags.HAS_EXTENDED_HEADER,
                AckMsgFlags.PRIVATE_STREAM,
                AckMsgFlags.HAS_SEQ_NUM,
                AckMsgFlags.HAS_MSG_KEY
        };

        private int[] ackMasksCommon =
        {
                AckMsgFlags.HAS_NAK_CODE,
                AckMsgFlags.HAS_TEXT,
                AckMsgFlags.HAS_MSG_KEY,
                AckMsgFlags.HAS_EXTENDED_HEADER
        };

        private int[] postMasksAll =
        {
                PostMsgFlags.HAS_EXTENDED_HEADER,
                PostMsgFlags.HAS_POST_ID,
                PostMsgFlags.HAS_MSG_KEY,
                PostMsgFlags.HAS_SEQ_NUM,
                PostMsgFlags.POST_COMPLETE,
                PostMsgFlags.ACK,
                PostMsgFlags.HAS_PART_NUM,
                PostMsgFlags.HAS_POST_USER_RIGHTS
        };

        private int[] postMasksCommon =
        {
                PostMsgFlags.HAS_POST_ID,
                PostMsgFlags.HAS_MSG_KEY,
                PostMsgFlags.ACK,
                PostMsgFlags.HAS_POST_USER_RIGHTS
        };

        private int[] genericMasksAll =
        {
                GenericMsgFlags.HAS_EXTENDED_HEADER,
                GenericMsgFlags.HAS_PERM_DATA,
                GenericMsgFlags.HAS_MSG_KEY,
                GenericMsgFlags.HAS_SEQ_NUM,
                GenericMsgFlags.MESSAGE_COMPLETE,
                GenericMsgFlags.HAS_SECONDARY_SEQ_NUM,
                GenericMsgFlags.HAS_PART_NUM
        };

        private int[] genericMasksCommon =
        {
                GenericMsgFlags.MESSAGE_COMPLETE,
                GenericMsgFlags.HAS_MSG_KEY,
                GenericMsgFlags.HAS_PERM_DATA,
                GenericMsgFlags.HAS_SEQ_NUM,
                GenericMsgFlags.HAS_EXTENDED_HEADER
        };

        /*
         * ExtraTestActions - Additional flag set for testing situations
         * additionally not covered by flags
         */
        private class ExtraTestAction
        {
            public const int POST_PAYLOAD = 0x0001; //Add post-encoded payload to the message (if PRE_PAYLOAD is not set)
            public const int PRE_PAYLOAD = 0x0002;  // Add a pre-encoded payload to the message
            public const int FOUR_BYTE_SEQ = 0x0004; // Test a sequence number with 4 bytes instead of 2
            public const int TRIM_DATA_BUF = 0x0008; // Test trimming the data buffer size to the actual encoded length
            public const int POST_OPAQUE = 0x0010; // Test post encoding of opaque - only applies in EncodeMsgInit cases 
            public const int POST_EXTENDED = 0x0020; // Test post encoding of extended header - only applies in EncodeMsgInit cases

            public const int PAYLOAD = POST_PAYLOAD | PRE_PAYLOAD; //For Decoder -- indicates payload whether pre or post -encoded
        }

        public MessageTest()
        {

            extendedHeader = "extendedHeader";
            extendedHeaderLen = extendedHeader.Length;

            exchangeId = "NYSE";
            exchangeIdLen = exchangeId.Length;

            permissionData = "permission";
            permissionDataLen = permissionData.Length;

            text = "Acknowledged";
            textLen = text.Length;

            groupIdLen = groupId.Length;

            stateText = "Source Unavailable";
            stateTextLen = stateText.Length;

            payloadName = "TRI.N";
            payloadNameLen = payloadName.Length;
            opaqueLen = 1000;
            opaque = new ByteBuffer(opaqueLen);

            encBuf.Data(new ByteBuffer(msgBufferSize));
            encDataBuf.Data(new ByteBuffer(dataBufferSize));


            fieldListReal.Value(0xFFFFFL, 1);
            fieldListDate.Day(8);
            fieldListDate.Month(3);
            fieldListDate.Year(1892);
            fieldListTime.Hour(23);
            fieldListTime.Minute(59);
            fieldListTime.Second(59);
            fieldListTime.Millisecond(999);
            fieldListDateTime.Day(8);
            fieldListDateTime.Month(3);
            fieldListDateTime.Year(1892);
            fieldListDateTime.Hour(23);
            fieldListDateTime.Minute(59);
            fieldListDateTime.Second(59);
            fieldListDateTime.Millisecond(999);

            /*
             * Encode Payload Data for pre-encoded data tests( a nested msg or
             * fieldList, depending on cmdline )
             */
            SetupEncodeDataIterator();
            EncodePayload(encDataIter);
        }

        private void SetupEncodeIterator()
        {
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(encBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        }

        private void SetupDecodeIterator()
        {
            decIter.Clear();
            decIter.SetBufferAndRWFVersion(encBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        }

        private void SetupEncodeDataIterator()
        {
            encDataIter.Clear();
            encDataIter.SetBufferAndRWFVersion(encDataBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        }

        private void EncodePayload(EncodeIterator encIter)
        {
            if (g_dataFormat == DataTypes.MSG)
                EncodeNestedMsg(encIter);
            else
                // FIELD_LIST
                EncodeFieldList(encIter);
        }

        void EncodeNestedMsg(EncodeIterator encIter)
        {
            IUpdateMsg updMsg = new Msg();

            updMsg.MsgClass = MsgClasses.UPDATE;
            updMsg.ContainerType = DataTypes.FIELD_LIST;
            updMsg.StreamId = streamId;
            updMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updMsg.Flags = UpdateMsgFlags.HAS_EXTENDED_HEADER | UpdateMsgFlags.HAS_MSG_KEY;
            updMsg.UpdateType = 3;

            updMsg.ExtendedHeader.Data(extendedHeader);

            updMsg.MsgKey.Flags = MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_FILTER | MsgKeyFlags.HAS_IDENTIFIER | MsgKeyFlags.HAS_ATTRIB;
            EncodeMsgKey(updMsg.MsgKey);

            Assert.Equal(CodecReturnCode.ENCODE_CONTAINER, updMsg.EncodeInit(encIter, 0));
            EncodeFieldList(encIter);

            Assert.Equal(CodecReturnCode.SUCCESS, updMsg.EncodeComplete(encIter, true));
        }

        void EncodeFieldList(EncodeIterator encIter)
        {
            /*
             * construct field list This field list will be used as every field list
             * in the structure
             */

            FieldList fieldList = new FieldList();
            FieldEntry entry = new FieldEntry();
            int iiEntry;

            fieldList.Clear();

            fieldList.Flags = FieldListFlags.HAS_STANDARD_DATA;

            /* init */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(encIter, null, 0));

            /* add entries */
            for (iiEntry = 0; iiEntry < FIELD_LIST_MAX_ENTRIES; ++iiEntry)
            {
                entry.Clear();
                entry.FieldId = fieldListFids[iiEntry];
                entry.DataType = fieldListDataTypes[iiEntry];

                switch (fieldListDataTypes[iiEntry])
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(encIter, fieldListInt));
                        break;
                    case DataTypes.DOUBLE:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(encIter, fieldListFloat));
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(encIter, fieldListReal));
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(encIter, fieldListDate));
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(encIter, fieldListTime));
                        break;
                    case DataTypes.DATETIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(encIter, fieldListDateTime));
                        break;
                    default:
                        Assert.True(false);
                        break;
                }
            }

            /* finish encoding */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(encIter, true));
        }

        private void EncodeMsgKey(IMsgKey key)
        {
            int mask = key.Flags;

            /* Add Service ID */
            if ((mask & MsgKeyFlags.HAS_SERVICE_ID) > 0)
                key.ServiceId = 7;

            /* Add Name */
            if ((mask & MsgKeyFlags.HAS_NAME) > 0)
            {
                key.Name.Data(payloadName);
            }

            /* Add Name Type */
            if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0)
                key.NameType = InstrumentNameTypes.RIC;

            /* Add Filter */
            if ((mask & MsgKeyFlags.HAS_FILTER) > 0)
                key.Filter = 4294967294L;

            /* Add ID */
            if ((mask & MsgKeyFlags.HAS_IDENTIFIER) > 0)
                key.Identifier = 9001;

            /* Add Attrib ContainerType/Data *//* Rssl calls this "opaque" */
            if ((mask & MsgKeyFlags.HAS_ATTRIB) > 0)
            {
                EncodeIterator encIter = new EncodeIterator();
                opaque.WritePosition = 0;
                key.EncodedAttrib.Data(opaque, 0, opaqueLen);
                encIter.SetBufferAndRWFVersion(key.EncodedAttrib, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                EncodeFieldList(encIter);
                key.AttribContainerType = DataTypes.FIELD_LIST;
            }
        }

        /*
    * does the work for encoding opaque and/or extended header after MsgInit is
    * called
    */
        private void PostEncodeMsg(EncodeIterator encIter, Msg msg, int extraAction, Buffer extHdr, int msgFlags, int extHdrFlag)
        {

            CodecReturnCode ret = CodecReturnCode.ENCODE_CONTAINER;

            if ((extraAction & ExtraTestAction.POST_OPAQUE) > 0 && msg.MsgKey != null && msg.MsgKey.CheckHasAttrib())
            {
                /* if we are post encoding the opaque, make it a field list */
                msg.MsgKey.EncodedAttrib.Clear();
                msg.MsgKey.AttribContainerType = DataTypes.FIELD_LIST;
                // if our opaque gets encoded after, this should be our ret
                ret = CodecReturnCode.ENCODE_MSG_KEY_ATTRIB;

                if ((extraAction & ExtraTestAction.POST_EXTENDED) > 0 && extHdr != null)
                {
                    /*
                     * if we are also post encoding our extended header, unset its
                     * values
                     */
                    extHdr.Clear();
                }
            }
            else if ((extraAction & ExtraTestAction.POST_EXTENDED) > 0 &&
                    (msgFlags & extHdrFlag) > 0 &&
                    extHdr != null)
            {
                /*
                 * if we are also post encoding our extended header, unset its
                 * values
                 */
                extHdr.Clear();
                ret = CodecReturnCode.ENCODE_EXTENDED_HEADER;
            }

            Assert.Equal(ret, msg.EncodeInit(encIter, 0));

            if (ret == CodecReturnCode.ENCODE_MSG_KEY_ATTRIB)
            {
                /* we need to encode our opaque here, should be a field list */

                EncodeFieldList(encIter);

                if ((extraAction & ExtraTestAction.POST_EXTENDED) > 0 && (msgFlags & extHdrFlag) > 0)
                {
                    /* we should have to encode our extended header next */
                    ret = CodecReturnCode.ENCODE_EXTENDED_HEADER;
                }
                else
                {
                    /*
                     * if our extended header is not being post encoded, it should
                     * be completed by the KeyOpaquecomplete call or is not present
                     */
                    ret = CodecReturnCode.ENCODE_CONTAINER;
                }

                Assert.Equal(ret, msg.EncodeKeyAttribComplete(encIter, true));
            }

            if ((ret == CodecReturnCode.ENCODE_EXTENDED_HEADER) && extHdr != null)
            {
                Buffer buffer = new Buffer();

                /* we must encode our extended header now */
                /* just hack copy it onto the wire */
                Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeNonRWFInit(encIter, buffer));
                Assert.True(buffer.Length >= extendedHeaderLen);
                buffer.Data().Put(System.Text.ASCIIEncoding.ASCII.GetBytes(extendedHeader));
                Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeNonRWFComplete(encIter, buffer, true));

                Assert.Equal(CodecReturnCode.ENCODE_CONTAINER, msg.EncodeExtendedHeaderComplete(encIter, true));
            }
        }

        /*
        * Remove flags that the encoder should have removed because the test
        * is(intentionally) trying to do something wrong
        */
        private int FixDecodeMsgKeyMask(int mask)
        {
            // Ensure that NameType is not set without a Name
            if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0 && !((mask & MsgKeyFlags.HAS_NAME) > 0))
                mask &= ~MsgKeyFlags.HAS_NAME_TYPE;

            return mask;
        }


        private bool IsEqual(Buffer buf1, Buffer buf2, int len)
        {
            bool retVal = true;
            int pos1 = buf1.Position, pos2 = buf2.Position;
            ByteBuffer data1 = buf1.Data(), data2 = buf2.Data();

            for (int i = 0; i < len; i++)
            {
                if (data1.Contents[i + pos1] != data2.Contents[i + pos2])
                {
                    retVal = false;
                    break;
                }
            }

            return retVal;
        }

        private bool IsEqual(Buffer buf1, System.String buf2, int len)
        {
            bool retVal = true;
            int pos1 = buf1.Position;
            ByteBuffer data1 = buf1.Data();
            byte[] stringByteArray = System.Text.ASCIIEncoding.ASCII.GetBytes(buf2);

            for (int i = 0; i < len; i++)
            {
                if (data1.Contents[i + pos1] != stringByteArray[i])
                {
                    retVal = false;
                    break;
                }
            }

            return retVal;
        }

        private void DecodeMsgKey(DecodeIterator decIter, IMsgKey key)
        {
            int mask = key.Flags;

            /* Check Service ID */
            if ((mask & MsgKeyFlags.HAS_SERVICE_ID) > 0)
                Assert.Equal(7, key.ServiceId);

            /* Check Name */
            if ((mask & MsgKeyFlags.HAS_NAME) > 0)
            {
                Assert.Equal(payloadNameLen, key.Name.Length);
                Assert.True(IsEqual(key.Name, payloadName, payloadNameLen));
            }

            /* Check Name Type */
            if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0)
                Assert.Equal(InstrumentNameTypes.RIC, key.NameType);

            /* Check Filter */
            if ((mask & MsgKeyFlags.HAS_FILTER) > 0)
                Assert.Equal(4294967294L, key.Filter);

            /* Check ID */
            if ((mask & MsgKeyFlags.HAS_IDENTIFIER) > 0)
                Assert.Equal(9001, key.Identifier);

            /* check opaque */
            if ((mask & MsgKeyFlags.HAS_ATTRIB) > 0)
            {
                Assert.Equal(DataTypes.FIELD_LIST, key.AttribContainerType);
                Assert.Equal(CodecReturnCode.SUCCESS, msg.DecodeKeyAttrib(decIter, key));
                DecodeFieldList(decIter);
            }
        }

        private void decodePayload(DecodeIterator decIter)
        {
            if (g_dataFormat == DataTypes.MSG)
                DecodeNestedMsg(decIter);
            else
                DecodeFieldList(decIter);
        }

        private void DecodeNestedMsg(DecodeIterator decIter)
        {
            Msg msg = new Msg();
            IUpdateMsg updateMsg = msg;

            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
            Assert.Equal(DataTypes.FIELD_LIST, msg.ContainerType);

            Assert.Equal(MsgClasses.UPDATE, msg.MsgClass);
            Assert.Equal(DataTypes.FIELD_LIST, msg.ContainerType);
            Assert.Equal(streamId, msg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, msg.DomainType);
            Assert.Equal((UpdateMsgFlags.HAS_EXTENDED_HEADER | UpdateMsgFlags.HAS_MSG_KEY), updateMsg.Flags);
            Assert.Equal(extendedHeaderLen, updateMsg.ExtendedHeader.Length);
            Assert.True(IsEqual(updateMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
            Assert.Equal(3, updateMsg.UpdateType);

            Assert.Equal((MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_FILTER | MsgKeyFlags.HAS_IDENTIFIER | MsgKeyFlags.HAS_ATTRIB), msg.MsgKey.Flags);
            DecodeMsgKey(decIter, msg.MsgKey);

            DecodeFieldList(decIter);
        }

        private void DecodeFieldList(DecodeIterator decIter)
        {
            Int decInt = new Int();
            Real decReal = new Real();
            Date decDate = new Date();
            Time decTime = new Time();
            Double decFloat = new Double();
            DateTime decDateTime = new DateTime();

            FieldList container = new FieldList();
            FieldEntry entry = new FieldEntry();

            int iiEntry;

            // Setup container
            container.Clear();
            entry.Clear();

            // Begin container decoding
            Assert.Equal(CodecReturnCode.SUCCESS, container.Decode(decIter, null));

            Assert.Equal(FieldListFlags.HAS_STANDARD_DATA, container.Flags);

            // Decode entries
            for (iiEntry = 0; iiEntry < FIELD_LIST_MAX_ENTRIES; ++iiEntry)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, entry.Decode(decIter));

                Assert.Equal(fieldListFids[iiEntry], entry.FieldId);
                Assert.Equal(DataTypes.UNKNOWN, entry.DataType);

                switch (fieldListDataTypes[iiEntry])
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decInt.Decode(decIter));
                        Assert.Equal(fieldListInt.ToLong(), decInt.ToLong());
                        break;
                    case DataTypes.DOUBLE:
                        Assert.Equal(CodecReturnCode.SUCCESS, decFloat.Decode(decIter));
                        /*
                         * not rounded inside encoding/decoding, so this should
                         * match exactly
                         */
                        Assert.True(fieldListFloat.ToDouble().Equals(decFloat.ToDouble()));
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, decReal.Decode(decIter));
                        Assert.False(decReal.IsBlank);
                        Assert.Equal(fieldListReal.Hint, decReal.Hint);
                        Assert.True(fieldListReal.ToLong().Equals(decReal.ToLong()));
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDate.Decode(decIter));
                        Assert.Equal(fieldListDate.Day(), decDate.Day());
                        Assert.Equal(fieldListDate.Month(), decDate.Month());
                        Assert.Equal(fieldListDate.Year(), decDate.Year());
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decTime.Decode(decIter));
                        Assert.Equal(fieldListTime.Hour(), decTime.Hour());
                        Assert.Equal(fieldListTime.Minute(), decTime.Minute());
                        Assert.Equal(fieldListTime.Second(), decTime.Second());
                        Assert.Equal(fieldListTime.Millisecond(), decTime.Millisecond());

                        break;
                    case DataTypes.DATETIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDateTime.Decode(decIter));
                        Assert.Equal(fieldListDateTime.Day(), decDateTime.Day());
                        Assert.Equal(fieldListDateTime.Month(), decDateTime.Month());
                        Assert.Equal(fieldListDateTime.Year(), decDateTime.Year());
                        Assert.Equal(fieldListDateTime.Hour(), decDateTime.Hour());
                        Assert.Equal(fieldListDateTime.Minute(), decDateTime.Minute());
                        Assert.Equal(fieldListDateTime.Second(), decDateTime.Second());
                        Assert.Equal(fieldListDateTime.Millisecond(), decDateTime.Millisecond());
                        break;
                    default:
                        Assert.True(false);
                        break;
                }
            }
            Assert.Equal(CodecReturnCode.END_OF_CONTAINER, entry.Decode(decIter));
        }

        private void UpdateMsgTest(int repeat)
        {
            int updateMask;

            int extraAction;

            IUpdateMsg updMsg = msg;

            int msgKeyMask;
            int decodeMsgKeyMask;

            int domainType = (int)DomainType.LOGIN;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    updateMask = masks[masksIter];

                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        for (keyMasksIter = 0; keyMasksIter < ((updateMask & UpdateMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize : 1); ++keyMasksIter)
                        {
                            msgKeyMask = keyMasks[keyMasksIter];
                            encBuf.Data().WritePosition = 0;
                            SetupEncodeIterator();

                            /* Encode msg */
                            updMsg.Clear();

                            updMsg.MsgClass = MsgClasses.UPDATE;
                            updMsg.StreamId = streamId;
                            updMsg.DomainType = domainType;

                            updMsg.Flags = updateMask;
                            updMsg.UpdateType = 3;

                            /* Add msgKey */
                            if ((updateMask & UpdateMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                updMsg.MsgKey.Flags = msgKeyMask;
                                EncodeMsgKey(updMsg.MsgKey);
                            }

                            /* Add Permission Info */
                            if ((updateMask & UpdateMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                updMsg.PermData.Data(permissionData);
                            }

                            /* Add Item Sequence Number */
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(updateMask + 0xFFFF) : (int)updateMask;
                                updMsg.SeqNum = seqNum;
                            }

                            /* Add Conflation Info */
                            if ((updateMask & UpdateMsgFlags.HAS_CONF_INFO) > 0)
                            {
                                updMsg.ConflationCount = updateMask + 2;
                                updMsg.ConflationTime = updateMask + 3;
                            }

                            if ((updateMask & UpdateMsgFlags.HAS_POST_USER_INFO) > 0)
                            {
                                /* Set up user information */
                                updMsg.PostUserInfo.UserAddrFromString("204.170.85.29");
                                updMsg.PostUserInfo.UserId = 0xCCAA551DL;
                            }

                            /* Add Extended Header */
                            if ((updateMask & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                updMsg.ExtendedHeader.Data(extendedHeader);
                            }

                            /* Add Payload */
                            if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                            {
                                msg.EncodedDataBody = encDataBuf;
                                updMsg.ContainerType = g_dataFormat;

                                /* Trim payload length if desired */
                                if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                    msg.EncodedDataBody = encDataBuf;
                                }
                                else
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                    msg.EncodedDataBody = encDataBuf;
                                }

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }
                            else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                            {
                                updMsg.ContainerType = g_dataFormat;

                                PostEncodeMsg(encIter, msg, extraAction, updMsg.ExtendedHeader, updMsg.Flags, UpdateMsgFlags.HAS_EXTENDED_HEADER);

                                EncodePayload(encIter);
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                            }
                            else
                            {
                                msg.EncodedDataBody.Clear();
                                updMsg.ContainerType = DataTypes.NO_DATA;
                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }

                            /* Decode msg */
                            decodeMsgKeyMask = FixDecodeMsgKeyMask(msgKeyMask);

                            SetupDecodeIterator();

                            if (g_changeData)
                            {
                                /* extract the msgClass */
                                Assert.Equal(MsgClasses.UPDATE, decIter.ExtractMsgClass());

                                /* extract the domainType */
                                Assert.Equal(domainType, decIter.ExtractDomainType());

                                /* extract streamId */
                                Assert.Equal(streamId, decIter.ExtractStreamId());

                                /* replace the streamId */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));

                                /* extract the streamId */
                                SetupDecodeIterator();
                                Assert.Equal(streamId + 1, decIter.ExtractStreamId());

                                /* extract the seqNum */
                                if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum, decIter.ExtractSeqNum());

                                /* replace the seqNum */
                                Assert.Equal(((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE),
                                             encIter.ReplaceSeqNum(seqNum + 1));

                                SetupDecodeIterator();
                                /* extract the new seqNum */
                                if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum + 1, decIter.ExtractSeqNum());
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
                            Assert.Equal(MsgClasses.UPDATE, msg.MsgClass);
                            Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                            Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                            Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                            Assert.Equal(updateMask, updMsg.Flags);

                            Assert.Equal(3, updMsg.UpdateType);

                            /* Check msgKey */
                            if ((updateMask & UpdateMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                Assert.Equal(decodeMsgKeyMask, updMsg.MsgKey.Flags);
                                DecodeMsgKey(decIter, updMsg.MsgKey);
                            }

                            /* Check Permission Info */
                            if ((updateMask & UpdateMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                Assert.Equal(permissionDataLen, updMsg.PermData.Length);
                                Assert.True(IsEqual(updMsg.PermData, permissionData, permissionDataLen));
                            }

                            /* Check Item Sequence Number */
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                Assert.Equal((g_changeData ? seqNum + 1 : seqNum), updMsg.SeqNum);
                            }

                            /* Check Conflation Info */
                            if ((updateMask & UpdateMsgFlags.HAS_CONF_INFO) > 0)
                            {
                                Assert.Equal(updateMask + 2, updMsg.ConflationCount);
                                Assert.Equal(updateMask + 3, updMsg.ConflationTime);
                            }

                            if ((updateMask & UpdateMsgFlags.HAS_POST_USER_INFO) > 0)
                            {
                                Assert.Equal(0xCCAA551DL, updMsg.PostUserInfo.UserAddr);
                                Assert.Equal(0xCCAA551DL, updMsg.PostUserInfo.UserId);
                            }

                            /* Check Extended Header */
                            if ((updateMask & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                Assert.Equal(extendedHeaderLen, updMsg.ExtendedHeader.Length);
                                Assert.True(IsEqual(updMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                            }

                            /* Check Payload */
                            if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                            {
                                decodePayload(decIter);
                            }
                        }
                    }
                }
            }
        }

        /*
         * allocateFlagCombinations() Pass in an array of masks(masksBase), this
         * will allocate 'masks' with a block containing all combinations of those
         * flags (including all & none).
         */
        private void AllocateFlagCombinations(int[] dstMasks, int[] srcMasks, int srcMasksSize, bool skipZero)
        {
            int skip = skipZero ? 1 : 0;
            int dstMasksSize = (int)System.Math.Pow(2, srcMasksSize) - skip;
            int srcMasksIter, dstMasksIter;

            for (dstMasksIter = skip; dstMasksIter < dstMasksSize + skip; ++dstMasksIter)
            {
                dstMasks[dstMasksIter - skip] = 0;
                for (srcMasksIter = 0; srcMasksIter < srcMasksSize; ++srcMasksIter)
                {
                    if (((dstMasksIter >> srcMasksIter) & 0x1) > 0)
                        dstMasks[dstMasksIter - skip] |= srcMasks[srcMasksIter];
                }
            }
        }

        private void TestSetupCommon()
        {
            if (!commonSetupComplete)
            {
                actionsSize = (int)System.Math.Pow(2, actionsCommon.Length);
                actions = new int[actionsSize];
                AllocateFlagCombinations(actions, actionsCommon, actionsCommon.Length, false);
                keyMasksSize = (int)System.Math.Pow(2, keyMasksCommon.Length) - 1;
                keyMasks = new int[keyMasksSize];
                AllocateFlagCombinations(keyMasks, keyMasksCommon, keyMasksCommon.Length, true);
                commonSetupComplete = true;
            }
        }

        private void TestSetupFull()
        {
            if (!fullSetupComplete)
            {
                actionsSize = (int)System.Math.Pow(2, actionsAll.Length);
                actions = new int[actionsSize];
                AllocateFlagCombinations(actions, actionsAll, actionsAll.Length, false);
                keyMasksSize = (int)System.Math.Pow(2, keyMasksAll.Length) - 1;
                keyMasks = new int[keyMasksSize];
                AllocateFlagCombinations(keyMasks, keyMasksAll, keyMasksAll.Length, true);
                fullSetupComplete = true;
            }
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void CheckFlagsTest()
        {
            // AckMsg
            Msg msg = new Msg();
            IAckMsg ackMsg = msg as IAckMsg;
            ackMsg.MsgClass = MsgClasses.ACK;
            Assert.False(ackMsg.CheckHasExtendedHdr());
            Assert.Null(ackMsg.ExtendedHeader);
            ackMsg.ApplyHasExtendedHdr();
            Assert.NotNull(ackMsg.ExtendedHeader);
            Assert.False(ackMsg.CheckHasMsgKey());
            Assert.Null(ackMsg.MsgKey);
            ackMsg.ApplyHasMsgKey();
            Assert.NotNull(ackMsg.MsgKey);
            Assert.False(ackMsg.CheckHasNakCode());
            Assert.Equal(0, ackMsg.NakCode);
            ackMsg.ApplyHasNakCode();
            ackMsg.NakCode = 11;
            Assert.Equal(11, ackMsg.NakCode);
            Assert.False(ackMsg.CheckHasSeqNum());
            Assert.Equal(0, ackMsg.SeqNum);
            ackMsg.ApplyHasSeqNum();
            ackMsg.SeqNum = 22;
            Assert.Equal(22, ackMsg.SeqNum);
            Assert.False(ackMsg.CheckHasText());
            Assert.Null(ackMsg.Text);
            Assert.False(ackMsg.CheckPrivateStream());
            ackMsg.ApplyPrivateStream();
            Assert.True(ackMsg.CheckPrivateStream());
            ackMsg.ApplyHasText();
            Assert.NotNull(ackMsg.Text);
            ackMsg.Flags = 0;
            Assert.False(ackMsg.CheckHasExtendedHdr());
            Assert.Null(ackMsg.ExtendedHeader);
            Assert.False(ackMsg.CheckHasMsgKey());
            Assert.Null(ackMsg.MsgKey);
            Assert.False(ackMsg.CheckHasNakCode());
            Assert.Equal(0, ackMsg.NakCode);
            Assert.False(ackMsg.CheckHasSeqNum());
            Assert.Equal(0, ackMsg.SeqNum);
            Assert.False(ackMsg.CheckHasText());
            Assert.Null(ackMsg.Text);
            Assert.False(ackMsg.CheckPrivateStream());

            msg.Clear();
            // CloseMsg
            ICloseMsg closeMsg = msg as ICloseMsg;
            closeMsg.MsgClass = MsgClasses.CLOSE;
            Assert.False(closeMsg.CheckHasExtendedHdr());
            Assert.Null(closeMsg.ExtendedHeader);
            closeMsg.ApplyHasExtendedHdr();
            Assert.NotNull(closeMsg.ExtendedHeader);
            Assert.False(closeMsg.CheckAck());
            closeMsg.ApplyAck();
            Assert.True(closeMsg.CheckAck());
            closeMsg.Flags = 0;
            Assert.False(closeMsg.CheckHasExtendedHdr());
            Assert.Null(closeMsg.ExtendedHeader);
            Assert.False(closeMsg.CheckAck());

            msg.Clear();
            // GenericMsg
            IGenericMsg genericMsg = msg as IGenericMsg;
            genericMsg.MsgClass = MsgClasses.GENERIC;
            Assert.False(genericMsg.CheckHasExtendedHdr());
            Assert.Null(genericMsg.ExtendedHeader);
            genericMsg.ApplyHasExtendedHdr();
            Assert.NotNull(genericMsg.ExtendedHeader);
            Assert.False(genericMsg.CheckHasMsgKey());
            Assert.Null(genericMsg.MsgKey);
            genericMsg.ApplyHasMsgKey();
            Assert.NotNull(genericMsg.MsgKey);
            Assert.False(genericMsg.CheckHasPartNum());
            Assert.Equal(0, genericMsg.PartNum);
            genericMsg.ApplyHasPartNum();
            genericMsg.PartNum = 11;
            Assert.Equal(11, genericMsg.PartNum);
            Assert.False(genericMsg.CheckHasPermData());
            Assert.Null(genericMsg.PermData);
            genericMsg.ApplyHasPermData();
            Assert.NotNull(genericMsg.PermData);
            Assert.False(genericMsg.CheckHasSecondarySeqNum());
            Assert.Equal(0, genericMsg.SecondarySeqNum);
            genericMsg.ApplyHasSecondarySeqNum();
            genericMsg.SecondarySeqNum = 22;
            Assert.Equal(22, genericMsg.SecondarySeqNum);
            Assert.False(genericMsg.CheckHasSeqNum());
            Assert.Equal(0, genericMsg.SeqNum);
            genericMsg.ApplyHasSeqNum();
            genericMsg.SeqNum = 33;
            Assert.Equal(33, genericMsg.SeqNum);
            Assert.False(genericMsg.CheckMessageComplete());
            genericMsg.ApplyMessageComplete();
            Assert.True(genericMsg.CheckMessageComplete());
            genericMsg.Flags = 0;
            Assert.False(genericMsg.CheckHasExtendedHdr());
            Assert.Null(genericMsg.ExtendedHeader);
            Assert.False(genericMsg.CheckHasMsgKey());
            Assert.Null(genericMsg.MsgKey);
            Assert.False(genericMsg.CheckHasPartNum());
            Assert.Equal(0, genericMsg.PartNum);
            Assert.False(genericMsg.CheckHasPermData());
            Assert.Null(genericMsg.PermData);
            Assert.False(genericMsg.CheckHasSecondarySeqNum());
            Assert.Equal(0, genericMsg.SecondarySeqNum);
            Assert.False(genericMsg.CheckHasSeqNum());
            Assert.Equal(0, genericMsg.SeqNum);
            Assert.False(genericMsg.CheckMessageComplete());

            msg.Clear();
            // PostMsg
            IPostMsg postMsg = msg as IPostMsg;
            postMsg.MsgClass = MsgClasses.POST;
            Assert.False(postMsg.CheckHasExtendedHdr());
            Assert.Null(postMsg.ExtendedHeader);
            postMsg.ApplyHasExtendedHdr();
            Assert.NotNull(postMsg.ExtendedHeader);
            Assert.False(postMsg.CheckHasMsgKey());
            Assert.Null(postMsg.MsgKey);
            postMsg.ApplyHasMsgKey();
            Assert.NotNull(postMsg.MsgKey);
            Assert.False(postMsg.CheckHasPartNum());
            Assert.Equal(0, postMsg.PartNum);
            postMsg.ApplyHasPartNum();
            postMsg.PartNum = 11;
            Assert.Equal(11, postMsg.PartNum);
            Assert.False(postMsg.CheckHasPermData());
            Assert.Null(postMsg.PermData);
            postMsg.ApplyHasPermData();
            Assert.NotNull(postMsg.PermData);
            Assert.False(postMsg.CheckHasPostId());
            Assert.Equal(0, postMsg.PostId);
            postMsg.ApplyHasPostId();
            postMsg.PostId = 22;
            Assert.Equal(22, postMsg.PostId);
            Assert.False(postMsg.CheckHasPostUserRights());
            Assert.Equal(0, postMsg.PostUserRights);
            postMsg.ApplyHasPostUserRights();
            postMsg.PostUserRights = 33;
            Assert.Equal(33, postMsg.PostUserRights);
            Assert.False(postMsg.CheckHasSeqNum());
            Assert.Equal(0, postMsg.SeqNum);
            postMsg.ApplyHasSeqNum();
            postMsg.SeqNum = 44;
            Assert.Equal(44, postMsg.SeqNum);
            Assert.False(postMsg.CheckPostComplete());
            postMsg.ApplyPostComplete();
            Assert.True(postMsg.CheckPostComplete());
            Assert.False(postMsg.CheckAck());
            postMsg.ApplyAck();
            Assert.True(postMsg.CheckAck());
            postMsg.Flags = 0;
            Assert.False(postMsg.CheckHasExtendedHdr());
            Assert.Null(postMsg.ExtendedHeader);
            Assert.False(postMsg.CheckHasMsgKey());
            Assert.Null(postMsg.MsgKey);
            Assert.False(postMsg.CheckHasPartNum());
            Assert.Equal(0, postMsg.PartNum);
            Assert.False(postMsg.CheckHasPermData());
            Assert.Null(postMsg.PermData);
            Assert.False(postMsg.CheckHasPostId());
            Assert.Equal(0, postMsg.PostId);
            Assert.False(postMsg.CheckHasPostUserRights());
            Assert.Equal(0, postMsg.PostUserRights);
            Assert.False(postMsg.CheckHasSeqNum());
            Assert.Equal(0, postMsg.SeqNum);
            Assert.False(postMsg.CheckPostComplete());
            Assert.False(postMsg.CheckAck());

            msg.Clear();
            // RefreshMsg
            IRefreshMsg refreshMsg = msg as IRefreshMsg;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            Assert.False(refreshMsg.CheckHasExtendedHdr());
            Assert.Null(refreshMsg.ExtendedHeader);
            refreshMsg.ApplyHasExtendedHdr();
            Assert.NotNull(refreshMsg.ExtendedHeader);
            Assert.False(refreshMsg.CheckHasMsgKey());
            Assert.Null(refreshMsg.MsgKey);
            refreshMsg.ApplyHasMsgKey();
            Assert.NotNull(refreshMsg.MsgKey);
            Assert.False(refreshMsg.CheckHasPartNum());
            Assert.Equal(0, refreshMsg.PartNum);
            refreshMsg.ApplyHasPartNum();
            refreshMsg.PartNum = 11;
            Assert.Equal(11, refreshMsg.PartNum);
            Assert.False(refreshMsg.CheckHasPermData());
            Assert.Null(refreshMsg.PermData);
            refreshMsg.ApplyHasPermData();
            Assert.NotNull(refreshMsg.PermData);
            Assert.False(refreshMsg.CheckHasPostUserInfo());
            Assert.Null(refreshMsg.PostUserInfo);
            refreshMsg.ApplyHasPostUserInfo();
            Assert.NotNull(refreshMsg.PostUserInfo);
            Assert.False(refreshMsg.CheckHasQos());
            Assert.Null(refreshMsg.Qos);
            refreshMsg.ApplyHasQos();
            Assert.NotNull(refreshMsg.Qos);
            Assert.False(refreshMsg.CheckHasSeqNum());
            Assert.Equal(0, refreshMsg.SeqNum);
            refreshMsg.ApplyHasSeqNum();
            refreshMsg.SeqNum = 22;
            Assert.Equal(22, refreshMsg.SeqNum);
            Assert.False(refreshMsg.CheckPrivateStream());
            refreshMsg.ApplyPrivateStream();
            Assert.True(refreshMsg.CheckPrivateStream());
            Assert.False(refreshMsg.CheckRefreshComplete());
            refreshMsg.ApplyRefreshComplete();
            Assert.True(refreshMsg.CheckRefreshComplete());
            Assert.False(refreshMsg.CheckSolicited());
            refreshMsg.ApplySolicited();
            Assert.True(refreshMsg.CheckSolicited());
            Assert.False(refreshMsg.CheckClearCache());
            refreshMsg.ApplyClearCache();
            Assert.True(refreshMsg.CheckClearCache());
            Assert.False(refreshMsg.CheckDoNotCache());
            refreshMsg.ApplyDoNotCache();
            Assert.True(refreshMsg.CheckDoNotCache());
            refreshMsg.Flags = 0;
            Assert.False(refreshMsg.CheckHasExtendedHdr());
            Assert.Null(refreshMsg.ExtendedHeader);
            Assert.False(refreshMsg.CheckHasMsgKey());
            Assert.Null(refreshMsg.MsgKey);
            Assert.False(refreshMsg.CheckHasPartNum());
            Assert.Equal(0, refreshMsg.PartNum);
            Assert.False(refreshMsg.CheckHasPermData());
            Assert.Null(refreshMsg.PermData);
            Assert.False(refreshMsg.CheckHasPostUserInfo());
            Assert.Null(refreshMsg.PostUserInfo);
            Assert.False(refreshMsg.CheckHasQos());
            Assert.Null(refreshMsg.Qos);
            Assert.False(refreshMsg.CheckHasSeqNum());
            Assert.Equal(0, refreshMsg.SeqNum);
            Assert.False(refreshMsg.CheckPrivateStream());
            Assert.False(refreshMsg.CheckRefreshComplete());
            Assert.False(refreshMsg.CheckSolicited());
            Assert.False(refreshMsg.CheckClearCache());
            Assert.False(refreshMsg.CheckDoNotCache());

            msg.Clear();
            // RequestMsg
            IRequestMsg requestMsg = msg as IRequestMsg;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            Assert.False(requestMsg.CheckHasExtendedHdr());
            Assert.Null(requestMsg.ExtendedHeader);
            requestMsg.ApplyHasExtendedHdr();
            Assert.NotNull(requestMsg.ExtendedHeader);
            Assert.False(requestMsg.CheckHasPriority());
            Assert.Null(requestMsg.Priority);
            requestMsg.ApplyHasPriority();
            Assert.NotNull(requestMsg.Priority);
            Assert.False(requestMsg.CheckHasQos());
            Assert.Null(requestMsg.Qos);
            requestMsg.ApplyHasQos();
            Assert.NotNull(requestMsg.Qos);
            Assert.False(requestMsg.CheckHasWorstQos());
            Assert.Null(requestMsg.WorstQos);
            requestMsg.ApplyHasWorstQos();
            Assert.NotNull(requestMsg.WorstQos);
            Assert.False(requestMsg.CheckConfInfoInUpdates());
            requestMsg.ApplyConfInfoInUpdates();
            Assert.True(requestMsg.CheckConfInfoInUpdates());
            Assert.False(requestMsg.CheckHasBatch());
            requestMsg.ApplyHasBatch();
            Assert.True(requestMsg.CheckHasBatch());
            Assert.False(requestMsg.CheckHasView());
            requestMsg.ApplyHasView();
            Assert.True(requestMsg.CheckHasView());
            Assert.False(requestMsg.CheckMsgKeyInUpdates());
            requestMsg.ApplyMsgKeyInUpdates();
            Assert.True(requestMsg.CheckMsgKeyInUpdates());
            Assert.False(requestMsg.CheckNoRefresh());
            requestMsg.ApplyNoRefresh();
            Assert.True(requestMsg.CheckNoRefresh());
            Assert.False(requestMsg.CheckPause());
            requestMsg.ApplyPause();
            Assert.True(requestMsg.CheckPause());
            Assert.False(requestMsg.CheckPrivateStream());
            requestMsg.ApplyPrivateStream();
            Assert.True(requestMsg.CheckPrivateStream());
            Assert.False(requestMsg.CheckStreaming());
            requestMsg.ApplyStreaming();
            Assert.True(requestMsg.CheckStreaming());
            requestMsg.Flags = 0;
            Assert.False(requestMsg.CheckHasExtendedHdr());
            Assert.Null(requestMsg.ExtendedHeader);
            Assert.False(requestMsg.CheckHasPriority());
            Assert.Null(requestMsg.Priority);
            Assert.False(requestMsg.CheckHasQos());
            Assert.Null(requestMsg.Qos);
            Assert.False(requestMsg.CheckHasWorstQos());
            Assert.Null(requestMsg.WorstQos);
            Assert.False(requestMsg.CheckConfInfoInUpdates());
            Assert.False(requestMsg.CheckHasBatch());
            Assert.False(requestMsg.CheckHasView());
            Assert.False(requestMsg.CheckMsgKeyInUpdates());
            Assert.False(requestMsg.CheckNoRefresh());
            Assert.False(requestMsg.CheckPause());
            Assert.False(requestMsg.CheckPrivateStream());
            Assert.False(requestMsg.CheckStreaming());

            msg.Clear();
            // StatusMsg
            IStatusMsg statusMsg = msg as IStatusMsg;
            statusMsg.MsgClass = MsgClasses.STATUS;
            Assert.False(statusMsg.CheckHasExtendedHdr());
            Assert.Null(statusMsg.ExtendedHeader);
            statusMsg.ApplyHasExtendedHdr();
            Assert.NotNull(statusMsg.ExtendedHeader);
            Assert.False(statusMsg.CheckHasGroupId());
            Assert.Null(statusMsg.GroupId);
            statusMsg.ApplyHasGroupId();
            Assert.NotNull(statusMsg.GroupId);
            Assert.False(statusMsg.CheckHasMsgKey());
            Assert.Null(statusMsg.MsgKey);
            statusMsg.ApplyHasMsgKey();
            Assert.NotNull(statusMsg.MsgKey);
            Assert.False(statusMsg.CheckHasPermData());
            Assert.Null(statusMsg.PermData);
            statusMsg.ApplyHasPermData();
            Assert.NotNull(statusMsg.PermData);
            Assert.False(statusMsg.CheckHasPostUserInfo());
            Assert.Null(statusMsg.PostUserInfo);
            statusMsg.ApplyHasPostUserInfo();
            Assert.NotNull(statusMsg.PostUserInfo);
            Assert.False(statusMsg.CheckHasState());
            Assert.Null(statusMsg.State);
            statusMsg.ApplyHasState();
            Assert.NotNull(statusMsg.State);
            Assert.False(statusMsg.CheckPrivateStream());
            statusMsg.ApplyPrivateStream();
            Assert.True(statusMsg.CheckPrivateStream());
            Assert.False(statusMsg.CheckClearCache());
            statusMsg.ApplyClearCache();
            Assert.True(statusMsg.CheckClearCache());
            statusMsg.Flags = 0;
            Assert.False(statusMsg.CheckHasExtendedHdr());
            Assert.Null(statusMsg.ExtendedHeader);
            Assert.False(statusMsg.CheckHasGroupId());
            Assert.Null(statusMsg.GroupId);
            Assert.False(statusMsg.CheckHasMsgKey());
            Assert.Null(statusMsg.MsgKey);
            Assert.False(statusMsg.CheckHasPermData());
            Assert.Null(statusMsg.PermData);
            Assert.False(statusMsg.CheckHasPostUserInfo());
            Assert.Null(statusMsg.PostUserInfo);
            Assert.False(statusMsg.CheckHasState());
            Assert.Null(statusMsg.State);
            Assert.False(statusMsg.CheckPrivateStream());
            Assert.False(statusMsg.CheckClearCache());

            msg.Clear();
            // UpdateMsg
            IUpdateMsg updateMsg = msg as IUpdateMsg;
            updateMsg.MsgClass = MsgClasses.UPDATE;
            Assert.False(updateMsg.CheckHasExtendedHdr());
            Assert.Null(updateMsg.ExtendedHeader);
            updateMsg.ApplyHasExtendedHdr();
            Assert.NotNull(updateMsg.ExtendedHeader);
            Assert.False(updateMsg.CheckHasMsgKey());
            Assert.Null(updateMsg.MsgKey);
            updateMsg.ApplyHasMsgKey();
            Assert.NotNull(updateMsg.MsgKey);
            Assert.False(updateMsg.CheckHasPermData());
            Assert.Null(updateMsg.PermData);
            updateMsg.ApplyHasPermData();
            Assert.NotNull(updateMsg.PermData);
            Assert.False(updateMsg.CheckHasPostUserInfo());
            Assert.Null(updateMsg.PostUserInfo);
            updateMsg.ApplyHasPostUserInfo();
            Assert.NotNull(updateMsg.PostUserInfo);
            Assert.False(updateMsg.CheckHasConfInfo());
            Assert.Equal(0, updateMsg.ConflationCount);
            Assert.Equal(0, updateMsg.ConflationTime);
            updateMsg.ApplyHasConfInfo();
            updateMsg.ConflationCount = 11;
            Assert.Equal(11, updateMsg.ConflationCount);
            updateMsg.ConflationTime = 22;
            Assert.Equal(22, updateMsg.ConflationTime);
            Assert.False(updateMsg.CheckHasSeqNum());
            Assert.Equal(0, updateMsg.SeqNum);
            updateMsg.ApplyHasSeqNum();
            updateMsg.SeqNum = 33;
            Assert.Equal(33, updateMsg.SeqNum);
            Assert.False(updateMsg.CheckDiscardable());
            updateMsg.ApplyDiscardable();
            Assert.True(updateMsg.CheckDiscardable());
            Assert.False(updateMsg.CheckDoNotCache());
            updateMsg.ApplyDoNotCache();
            Assert.True(updateMsg.CheckDoNotCache());
            Assert.False(updateMsg.CheckDoNotConflate());
            updateMsg.ApplyDoNotConflate();
            Assert.True(updateMsg.CheckDoNotConflate());
            Assert.False(updateMsg.CheckDoNotRipple());
            updateMsg.ApplyDoNotRipple();
            Assert.True(updateMsg.CheckDoNotRipple());
            updateMsg.Flags = 0;
            Assert.False(updateMsg.CheckHasExtendedHdr());
            Assert.Null(updateMsg.ExtendedHeader);
            Assert.False(updateMsg.CheckHasMsgKey());
            Assert.Null(updateMsg.MsgKey);
            Assert.False(updateMsg.CheckHasPermData());
            Assert.Null(updateMsg.PermData);
            Assert.False(updateMsg.CheckHasPostUserInfo());
            Assert.Null(updateMsg.PostUserInfo);
            Assert.False(updateMsg.CheckHasConfInfo());
            Assert.False(updateMsg.CheckHasSeqNum());
            Assert.False(updateMsg.CheckDiscardable());
            Assert.False(updateMsg.CheckDoNotCache());
            Assert.False(updateMsg.CheckDoNotConflate());
            Assert.False(updateMsg.CheckDoNotRipple());
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void UpdateMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, updateMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, updateMasksCommon, updateMasksCommon.Length, false);
            UpdateMsgTest(repeatCount);
        }

        [Fact]
        [Category("Message")]
        public void UpdateMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, updateMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, updateMasksAll, updateMasksAll.Length, false);
            UpdateMsgTest(repeatCount);
        }

        private void RefreshMsgTest(int repeat)
        {
            int extraAction;

            int responseMask;

            IRefreshMsg respMsg = msg as IRefreshMsg;

            int msgKeyMask;
            int decodeMsgKeyMask;

            int domainType = (int)DomainType.LOGIN;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    responseMask = masks[masksIter];

                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        for (keyMasksIter = 0; keyMasksIter < ((responseMask & RefreshMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize : 1); ++keyMasksIter)
                        {
                            msgKeyMask = keyMasks[keyMasksIter];
                            encBuf.Data().WritePosition = 0;
                            SetupEncodeIterator();

                            /* Encode msg */

                            respMsg.Clear();

                            respMsg.MsgClass = MsgClasses.REFRESH;
                            respMsg.StreamId = streamId;
                            respMsg.DomainType = domainType;

                            respMsg.Flags = responseMask;

                            respMsg.State.StreamState(StreamStates.OPEN);
                            respMsg.State.DataState(DataStates.SUSPECT);
                            respMsg.State.Code(StateCodes.NO_RESOURCES);
                            respMsg.State.Text().Data(stateText);

                            respMsg.GroupId.Data(ByteBuffer.Wrap(groupId));

                            /* Add msgKey */
                            if ((responseMask & RefreshMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                respMsg.MsgKey.Flags = msgKeyMask;
                                EncodeMsgKey(respMsg.MsgKey);
                            }

                            /* Add Permission Info */
                            if ((responseMask & RefreshMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                respMsg.PermData.Data(permissionData);
                            }

                            /* Add Item Sequence Number */
                            if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(responseMask + 0xFFFF) : (int)responseMask;
                                respMsg.SeqNum = seqNum;
                            }

                            if ((responseMask & RefreshMsgFlags.HAS_POST_USER_INFO) > 0)
                            {
                                /* Set up user information */
                                respMsg.PostUserInfo.UserAddrFromString("204.170.85.29");
                                respMsg.PostUserInfo.UserId = 0xCCAA551DL;
                            }

                            if ((responseMask & RefreshMsgFlags.HAS_PART_NUM) > 0)
                            {
                                respMsg.PartNum = 0x100f;
                            }

                            /* Add QoS */
                            if ((responseMask & RefreshMsgFlags.HAS_QOS) > 0)
                            {
                                respMsg.Qos.Rate(QosRates.TIME_CONFLATED);
                                respMsg.Qos.RateInfo(65535);
                                respMsg.Qos.Timeliness(QosTimeliness.DELAYED);
                                respMsg.Qos.TimeInfo(65534);
                            }

                            /* Add Extended Header */
                            if ((responseMask & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                respMsg.ExtendedHeader.Data(extendedHeader);
                            }

                            /* Add Payload */
                            if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                            {
                                msg.EncodedDataBody = encDataBuf;
                                respMsg.ContainerType = g_dataFormat;

                                /* Trim payload length if desired */
                                if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                    msg.EncodedDataBody = encDataBuf;
                                }
                                else
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                    msg.EncodedDataBody = encDataBuf;
                                }

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }
                            else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                            {
                                respMsg.ContainerType = g_dataFormat;

                                PostEncodeMsg(encIter, msg, extraAction, respMsg.ExtendedHeader, respMsg.Flags, RefreshMsgFlags.HAS_EXTENDED_HEADER);

                                EncodePayload(encIter);
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                            }
                            else
                            {
                                msg.EncodedDataBody.Clear();
                                respMsg.ContainerType = DataTypes.NO_DATA;
                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }

                            /* Decode msg */
                            decodeMsgKeyMask = FixDecodeMsgKeyMask(msgKeyMask);

                            SetupDecodeIterator();
                            SetupEncodeIterator();

                            if (g_changeData)
                            {
                                Buffer newGroupId = new Buffer();
                                newGroupId.Data(ByteBuffer.Wrap(groupId));
                                byte[] groupIdData;
                                Buffer extractGroupId = new Buffer();
                                extractGroupId.Data(new ByteBuffer(32));
                                groupIdData = extractGroupId.Data().Contents;

                                /* extract the msgClass */
                                Assert.Equal(MsgClasses.REFRESH, decIter.ExtractMsgClass());
                                SetupDecodeIterator();

                                /* extract the domainType */
                                Assert.Equal(domainType, decIter.ExtractDomainType());
                                SetupDecodeIterator();

                                /* extract streamId */
                                Assert.Equal(streamId, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* replace the streamId */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));
                                SetupEncodeIterator();

                                /* extract the streamId */
                                Assert.Equal(streamId + 1, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* extract the seqNum */
                                if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum, decIter.ExtractSeqNum());
                                SetupDecodeIterator();

                                /* replace the seqNum */
                                Assert.Equal(((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE),
                                             encIter.ReplaceSeqNum(seqNum + 1));

                                /* extract the new seqNum */
                                if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum + 1, decIter.ExtractSeqNum());
                                SetupDecodeIterator();

                                /* extract the groupId */
                                Assert.Equal(CodecReturnCode.SUCCESS, decIter.ExtractGroupId(extractGroupId));
                                Assert.Equal(groupIdLen, extractGroupId.Length);
                                Assert.True(IsEqual(extractGroupId, newGroupId, newGroupId.Length));
                                SetupDecodeIterator();

                                /* replace the groupId */
                                groupId[0] = (byte)'4';
                                newGroupId.Data(ByteBuffer.Wrap(groupId));
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceGroupId(newGroupId));

                                extractGroupId.Data().Clear();

                                /* extract the new groupId */
                                Assert.Equal(CodecReturnCode.SUCCESS, decIter.ExtractGroupId(extractGroupId));
                                Assert.Equal(newGroupId.Length, extractGroupId.Length);
                                Assert.True(IsEqual(extractGroupId, newGroupId, newGroupId.Length));
                                SetupDecodeIterator();

                                /* replace the streamState */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamState(StreamStates.REDIRECTED));

                                /* replace the dataState */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceDataState(DataStates.OK));

                                /* replace the stateCode */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStateCode(StateCodes.INVALID_VIEW));
                            }

                            if (g_setRefreshFlags)
                            {
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetRefreshCompleteFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetSolicitedFlag());

                                foreach (int maskToSet in responseSetUnSetMasks)
                                {
                                    responseMask |= maskToSet;
                                }
                            }

                            if (g_unsetRefreshFlags)
                            {
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetRefreshCompleteFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetSolicitedFlag());
                                SetupEncodeIterator();

                                foreach (int maskToUnSet in responseSetUnSetMasks)
                                {
                                    responseMask &= ~maskToUnSet;
                                }
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
                            Assert.Equal(MsgClasses.REFRESH, msg.MsgClass);
                            Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                            Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                            Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                            Assert.Equal(responseMask, respMsg.Flags);

                            /* Check msgKey */
                            if ((responseMask & RefreshMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                Assert.Equal(decodeMsgKeyMask, respMsg.MsgKey.Flags);
                                DecodeMsgKey(decIter, respMsg.MsgKey);
                            }

                            /* Check State */
                            Assert.Equal((g_changeData ? StreamStates.REDIRECTED : StreamStates.OPEN),
                                         respMsg.State.StreamState());
                            Assert.Equal((g_changeData ? DataStates.OK : DataStates.SUSPECT), respMsg.State.DataState());
                            Assert.Equal((g_changeData ? StateCodes.INVALID_VIEW : StateCodes.NO_RESOURCES), respMsg.State.Code());
                            Assert.Equal(stateTextLen, respMsg.State.Text().Length);
                            Assert.True(IsEqual(respMsg.State.Text(), stateText, stateTextLen));

                            Assert.Equal(groupIdLen, respMsg.GroupId.Length);
                            Buffer gid = new Buffer();
                            gid.Data(ByteBuffer.Wrap(groupId));
                            Assert.True(IsEqual(respMsg.GroupId, gid, groupIdLen));

                            /* Check Permission Info */
                            if ((responseMask & RefreshMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                Assert.Equal(permissionDataLen, respMsg.PermData.Length);
                                Assert.True(IsEqual(respMsg.PermData, permissionData, permissionDataLen));
                            }

                            /* Check Item Sequence Number */
                            if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                Assert.Equal((g_changeData ? seqNum + 1 : seqNum), respMsg.SeqNum);
                            }

                            if ((responseMask & RefreshMsgFlags.HAS_POST_USER_INFO) > 0)
                            {
                                Assert.Equal(0xCCAA551DL, respMsg.PostUserInfo.UserAddr);
                                Assert.Equal(0xCCAA551DL, respMsg.PostUserInfo.UserId);
                            }

                            if ((responseMask & RefreshMsgFlags.HAS_PART_NUM) > 0)
                            {
                                Assert.Equal(0x100f, respMsg.PartNum);
                            }

                            /* Check QoS */
                            if ((responseMask & RefreshMsgFlags.HAS_QOS) > 0)
                            {
                                Assert.Equal(QosRates.TIME_CONFLATED, respMsg.Qos.Rate());
                                Assert.Equal(65535, respMsg.Qos.RateInfo());
                                Assert.Equal(QosTimeliness.DELAYED, respMsg.Qos.Timeliness());
                                Assert.Equal(65534, respMsg.Qos.TimeInfo());
                            }

                            /* Check Extended Header */
                            if ((responseMask & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                Assert.Equal(extendedHeaderLen, respMsg.ExtendedHeader.Length);
                                Assert.True(IsEqual(respMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                            }

                            /* Check Payload */
                            if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                            {
                                decodePayload(decIter);
                            }
                        }
                    }
                }
            }
        }

        [Fact][Category("Unit")]
        [Category("Message")]
        public void RefreshMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, responseMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, responseMasksCommon, responseMasksCommon.Length, false);
            RefreshMsgTest(repeatCount);
        }


        [Fact(Skip = "Take 9 minute to run")]
        [Category("Message")]
        public void RefreshMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, responseMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, responseMasksAll, responseMasksAll.Length, false);
            RefreshMsgTest(repeatCount);
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void RefreshMessageSetMsgFlagsTest()
        {
            TestSetupFull();
            masks = responseSetUnSetMasksTests;
            masksSize = masks.Length;

            g_setRefreshFlags = true;
            RefreshMsgTest(repeatCount);
            g_setRefreshFlags = false;
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void RefreshMessageUnSetMsgFlagsTest()
        {
            TestSetupFull();
            masks = responseSetUnSetMasksTests;
            masksSize = masks.Length;
            g_unsetRefreshFlags = true;
            RefreshMsgTest(repeatCount);
            g_unsetRefreshFlags = false;
        }

        private void RequestMsgTest(int repeat)
        {
            int extraAction;

            int requestMask;
            IRequestMsg reqMsg = msg as IRequestMsg;

            int msgKeyMask;
            int decodeMsgKeyMask;

            int domainType = (int)DomainType.DICTIONARY;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    requestMask = masks[masksIter];

                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        //reqMsg always has key
                        for (keyMasksIter = 0; keyMasksIter < keyMasksSize; ++keyMasksIter)
                        {
                            msgKeyMask = keyMasks[keyMasksIter];
                            encBuf.Data().WritePosition = 0;
                            SetupEncodeIterator();

                            /* Encode msg */
                            reqMsg.Clear();

                            reqMsg.MsgClass = MsgClasses.REQUEST;
                            reqMsg.StreamId = streamId;
                            reqMsg.DomainType = domainType;

                            reqMsg.Flags = requestMask;

                            /* Add msg key */
                            reqMsg.MsgKey.Flags = msgKeyMask;
                            EncodeMsgKey(reqMsg.MsgKey);

                            /* Add priority info */
                            if ((requestMask & RequestMsgFlags.HAS_PRIORITY) > 0)
                            {
                                reqMsg.Priority.PriorityClass = 3;
                                reqMsg.Priority.Count = 4;
                            }

                            /* Add best QoS */
                            if ((requestMask & RequestMsgFlags.HAS_QOS) > 0)
                            {
                                reqMsg.Qos.Rate(QosRates.TIME_CONFLATED);
                                reqMsg.Qos.RateInfo(65535);
                                reqMsg.Qos.Timeliness(QosTimeliness.DELAYED);
                                reqMsg.Qos.TimeInfo(65534);
                            }

                            /* Add worst QoS */
                            if ((requestMask & RequestMsgFlags.HAS_WORST_QOS) > 0)
                            {
                                reqMsg.WorstQos.Rate(QosRates.TIME_CONFLATED);
                                reqMsg.WorstQos.RateInfo(65533);
                                reqMsg.WorstQos.Timeliness(QosTimeliness.DELAYED);
                                reqMsg.WorstQos.TimeInfo(65532);
                            }

                            /* Add extended header */
                            if ((requestMask & RequestMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                reqMsg.ExtendedHeader.Data(extendedHeader);
                            }

                            /* Add Payload */
                            if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                            {
                                msg.EncodedDataBody = encDataBuf;
                                reqMsg.ContainerType = g_dataFormat;

                                /* Trim payload length if desired */
                                if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                    msg.EncodedDataBody = encDataBuf;
                                }
                                else
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                    msg.EncodedDataBody = encDataBuf;
                                }

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }
                            else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                            {
                                reqMsg.ContainerType = g_dataFormat;

                                PostEncodeMsg(encIter, msg, extraAction, reqMsg.ExtendedHeader, reqMsg.Flags, RequestMsgFlags.HAS_EXTENDED_HEADER);

                                EncodePayload(encIter);
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                            }
                            else
                            {
                                msg.EncodedDataBody.Clear();
                                reqMsg.ContainerType = DataTypes.NO_DATA;
                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }

                            /* Decode msg */
                            decodeMsgKeyMask = FixDecodeMsgKeyMask(msgKeyMask);

                            SetupDecodeIterator();
                            SetupEncodeIterator();

                            if (g_changeData)
                            {
                                SetupEncodeIterator();

                                /* extract the msgClass */
                                Assert.Equal(MsgClasses.REQUEST, decIter.ExtractMsgClass());
                                SetupDecodeIterator();

                                /* extract the domainType */
                                Assert.Equal(domainType, decIter.ExtractDomainType());
                                SetupDecodeIterator();

                                /* extract streamId */
                                Assert.Equal(streamId, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* replace the streamId */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));
                                SetupEncodeIterator();

                                /* extract the streamId */
                                Assert.Equal(streamId + 1, decIter.ExtractStreamId());
                                SetupDecodeIterator();
                            }

                            if (g_setRequestFlags)
                            {
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetNoRefreshFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetStreamingFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetConfInfoInUpdatesFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetMsgKeyInUpdatesFlag());
                                SetupEncodeIterator();

                                foreach (int maskToSet in requestSetUnSetMasks)
                                {
                                    requestMask |= maskToSet;
                                }
                            }

                            if (g_unsetRequestFlags)
                            {
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetNoRefreshFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetStreamingFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetConfInfoInUpdatesFlag());
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetMsgKeyInUpdatesFlag());
                                SetupEncodeIterator();

                                foreach (int maskToUnSet in requestSetUnSetMasks)
                                {
                                    requestMask &= ~maskToUnSet;
                                }
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));

                            /* Check mask and msgBase */
                            Assert.Equal(MsgClasses.REQUEST, msg.MsgClass);
                            Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                            Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                            Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                            Assert.Equal(requestMask, reqMsg.Flags);

                            /* Check msg key */
                            Assert.Equal(decodeMsgKeyMask, reqMsg.MsgKey.Flags);
                            // decodeMsgKey(decIter, reqMsg.MsgKey);

                            /* Check priority info */
                            if ((requestMask & RequestMsgFlags.HAS_PRIORITY) > 0)
                            {
                                Assert.Equal(3, reqMsg.Priority.PriorityClass);
                                Assert.Equal(4, reqMsg.Priority.Count);
                            }

                            /* Check best QoS */
                            if ((requestMask & RequestMsgFlags.HAS_QOS) > 0)
                            {
                                Assert.Equal(QosRates.TIME_CONFLATED, reqMsg.Qos.Rate());
                                Assert.Equal(65535, reqMsg.Qos.RateInfo());
                                Assert.Equal(QosTimeliness.DELAYED, reqMsg.Qos.Timeliness());
                                Assert.Equal(65534, reqMsg.Qos.TimeInfo());
                            }

                            /* Check worst QoS */
                            if ((requestMask & RequestMsgFlags.HAS_WORST_QOS) > 0)
                            {
                                Assert.Equal(QosRates.TIME_CONFLATED, reqMsg.WorstQos.Rate());
                                Assert.Equal(65533, reqMsg.WorstQos.RateInfo());
                                Assert.Equal(QosTimeliness.DELAYED, reqMsg.WorstQos.Timeliness());
                                Assert.Equal(65532, reqMsg.WorstQos.TimeInfo());
                            }

                            /* Check extended header */
                            if ((requestMask & RequestMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                Assert.Equal(extendedHeaderLen, reqMsg.ExtendedHeader.Length);
                                Assert.True(IsEqual(reqMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                            }

                            /* Check Payload */
                            if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                            {
                                decodePayload(decIter);
                            }
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Message")]
        public void RequestMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, requestMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, requestMasksCommon, requestMasksCommon.Length, false);
            RequestMsgTest(repeatCount);
        }

        [Fact]
        [Category("Message")]
        public void RequestMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, requestMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, requestMasksAll, requestMasksAll.Length, false);
            RequestMsgTest(repeatCount);
        }

        [Fact]
        [Category("Message")]
        public void RequestMessageFullTestSetFlags()
        {
            TestSetupFull();
            masks = requestSetUnSetMasksTests;
            masksSize = masks.Length;
            g_setRequestFlags = true;
            RequestMsgTest(repeatCount);
            g_setRequestFlags = false;
        }

        [Fact]
        [Category("Message")]
        public void RequestMessageFullTestUnSetFlags()
        {
            TestSetupFull();
            masks = requestSetUnSetMasksTests;
            masksSize = masks.Length;
            g_unsetRequestFlags = true;
            RequestMsgTest(repeatCount);
            g_unsetRequestFlags = false;
        }

        private void StatusMsgTest(int repeat)
        {
            int extraAction;

            int statusMask;

            IStatusMsg statMsg = msg as IStatusMsg;

            int msgKeyMask;
            int decodeMsgKeyMask;

            int domainType = (int)DomainType.MARKET_PRICE;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    statusMask = masks[masksIter];

                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        for (keyMasksIter = 0; keyMasksIter < ((statusMask & StatusMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize : 1); ++keyMasksIter)
                        {
                            msgKeyMask = keyMasks[keyMasksIter];
                            encBuf.Data().WritePosition = 0;
                            SetupEncodeIterator();

                            /* Encode msg */
                            statMsg.Clear();

                            statMsg.MsgClass = MsgClasses.STATUS;
                            statMsg.StreamId = streamId;
                            statMsg.DomainType = domainType;

                            statMsg.Flags = statusMask;

                            /* Add Item State */
                            if ((statusMask & StatusMsgFlags.HAS_STATE) > 0)
                            {
                                statMsg.State.StreamState(StreamStates.OPEN);
                                statMsg.State.DataState(DataStates.SUSPECT);
                                statMsg.State.Code(StateCodes.NO_RESOURCES);
                                statMsg.State.Text().Data(stateText);
                            }

                            /* Add Group ID */
                            if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                            {
                                statMsg.GroupId.Data(ByteBuffer.Wrap(groupId));
                            }

                            /* Add msgKey */
                            if ((statusMask & StatusMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                statMsg.MsgKey.Flags = msgKeyMask;
                                EncodeMsgKey(statMsg.MsgKey);
                            }

                            /* Add Permission Info */
                            if ((statusMask & StatusMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                statMsg.PermData.Data(permissionData);
                            }

                            if ((statusMask & StatusMsgFlags.HAS_POST_USER_INFO) > 0)
                            {
                                /* Set up user information */
                                statMsg.PostUserInfo.UserAddrFromString("204.170.85.29");
                                statMsg.PostUserInfo.UserId = 0xCCAA551DL;
                            }

                            /* Add Extended Header */
                            if ((statusMask & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                statMsg.ExtendedHeader.Data(extendedHeader);
                            }

                            /* Add Payload */
                            if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                            {
                                msg.EncodedDataBody = encDataBuf;
                                statMsg.ContainerType = g_dataFormat;

                                /* Trim payload length if desired */
                                if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                    msg.EncodedDataBody = encDataBuf;
                                }
                                else
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                    msg.EncodedDataBody = encDataBuf;
                                }

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }
                            else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                            {
                                statMsg.ContainerType = g_dataFormat;

                                PostEncodeMsg(encIter, msg, extraAction, statMsg.ExtendedHeader, statMsg.Flags, StatusMsgFlags.HAS_EXTENDED_HEADER);

                                EncodePayload(encIter);
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                            }
                            else
                            {
                                msg.EncodedDataBody.Clear();
                                statMsg.ContainerType = DataTypes.NO_DATA;
                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }

                            /* Decode msg */
                            decodeMsgKeyMask = FixDecodeMsgKeyMask(msgKeyMask);

                            SetupDecodeIterator();
                            SetupEncodeIterator();

                            if (g_changeData)
                            {
                                Buffer newGroupId = new Buffer();
                                newGroupId.Data(ByteBuffer.Wrap(groupId));
                                byte[] groupIdData;
                                Buffer extractGroupId = new Buffer();
                                extractGroupId.Data(new ByteBuffer(32));
                                groupIdData = extractGroupId.Data().Contents;

                                /* extract the msgClass */
                                Assert.Equal(MsgClasses.STATUS, decIter.ExtractMsgClass());
                                SetupDecodeIterator();

                                /* extract the domainType */
                                Assert.Equal(domainType, decIter.ExtractDomainType());
                                SetupDecodeIterator();

                                /* extract streamId */
                                Assert.Equal(streamId, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* replace the streamId */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));
                                SetupEncodeIterator();

                                /* extract the streamId */
                                Assert.Equal(streamId + 1, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* extract the groupId */

                                if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0 )
                                    Assert.Equal(CodecReturnCode.SUCCESS, decIter.ExtractGroupId(extractGroupId));
                                else
                                    Assert.Equal(CodecReturnCode.FAILURE, decIter.ExtractGroupId(extractGroupId));

                                if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                                {
                                    Assert.Equal(groupIdLen, extractGroupId.Length);
                                    Assert.True(IsEqual(extractGroupId, newGroupId, newGroupId.Length));
                                }
                                SetupDecodeIterator();

                                /* replace the groupId */
                                groupId[0] = (byte)'4';
                                Assert.Equal(((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE), encIter.ReplaceGroupId(newGroupId));

                                /* extract the new groupId */
                                Assert.Equal(((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE), decIter.ExtractGroupId(extractGroupId));
                                if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                                {
                                    Assert.Equal(newGroupId.Length, extractGroupId.Length);
                                    Assert.True(IsEqual(extractGroupId, newGroupId, newGroupId.Length));
                                }
                                SetupDecodeIterator();

                                /* replace the streamState */
                                Assert.Equal(((statusMask & StatusMsgFlags.HAS_STATE) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE), encIter.ReplaceStreamState(StreamStates.REDIRECTED));

                                /* replace the dataState */
                                Assert.Equal(((statusMask & StatusMsgFlags.HAS_STATE) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE), encIter.ReplaceDataState(DataStates.OK));

                                /* replace the stateCode */
                                Assert.Equal(((statusMask & StatusMsgFlags.HAS_STATE) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE), encIter.ReplaceStateCode(StateCodes.INVALID_VIEW));
                                SetupEncodeIterator();
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
                            Assert.Equal(MsgClasses.STATUS, msg.MsgClass);
                            Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                            Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                            Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                            Assert.Equal(statusMask, statMsg.Flags);

                            /* Check Item State */
                            if ((statusMask & StatusMsgFlags.HAS_STATE) > 0)
                            {
                                Assert.Equal((g_changeData ? StreamStates.REDIRECTED : StreamStates.OPEN),
                                             statMsg.State.StreamState());
                                Assert.Equal((g_changeData ? DataStates.OK : DataStates.SUSPECT), statMsg.State.DataState());
                                Assert.Equal((g_changeData ? StateCodes.INVALID_VIEW : StateCodes.NO_RESOURCES), statMsg.State.Code());
                                Assert.Equal(stateTextLen, statMsg.State.Text().Length);
                                Assert.True(IsEqual(statMsg.State.Text(), stateText, stateTextLen));
                            }

                            if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                            {
                                Assert.Equal(groupIdLen, statMsg.GroupId.Length);
                                Buffer gid = new Buffer();
                                gid.Data(ByteBuffer.Wrap(groupId));
                                Assert.True(IsEqual(statMsg.GroupId, gid, groupIdLen));
                            }

                            /* Check msgKey */
                            if ((statusMask & StatusMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                Assert.Equal(decodeMsgKeyMask, statMsg.MsgKey.Flags);
                                DecodeMsgKey(decIter, statMsg.MsgKey);
                            }

                            /* Check Permission Info */
                            if ((statusMask & StatusMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                Assert.Equal(permissionDataLen, statMsg.PermData.Length);
                                Assert.True(IsEqual(statMsg.PermData, permissionData, permissionDataLen));
                            }

                            if ((statusMask & StatusMsgFlags.HAS_POST_USER_INFO) > 0)
                            {
                                Assert.Equal(0xCCAA551DL, statMsg.PostUserInfo.UserAddr);
                                Assert.Equal(0xCCAA551DL, statMsg.PostUserInfo.UserId);
                            }

                            /* Check Extended Header */
                            if ((statusMask & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                Assert.Equal(extendedHeaderLen, statMsg.ExtendedHeader.Length);
                                Assert.True(IsEqual(statMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                            }

                            /* Check Payload */
                            if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                            {
                                decodePayload(decIter);
                            }
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void StatusMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, statusMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, statusMasksCommon, statusMasksCommon.Length, false);
            StatusMsgTest(repeatCount);
        }

        [Fact]
        [Category("Message")]
        public void StatusMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, statusMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, statusMasksAll, statusMasksAll.Length, false);
            StatusMsgTest(repeatCount);
        }


        private void CloseMsgTest(int repeat)
        {
            int extraAction;

            int closeMask;

            ICloseMsg closeMsg = msg as ICloseMsg;

            int domainType = (int)DomainType.DICTIONARY;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    closeMask = masks[masksIter];

                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        encBuf.Data().WritePosition = 0;
                        SetupEncodeIterator();

                        /* Encode msg */

                        closeMsg.Clear();
                        closeMsg.MsgClass = MsgClasses.CLOSE;
                        closeMsg.StreamId = streamId;
                        closeMsg.DomainType = domainType;

                        closeMsg.Flags = closeMask;

                        /* Add Extended Header */
                        if ((closeMask & CloseMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            closeMsg.ExtendedHeader.Data(extendedHeader);
                        }

                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.EncodedDataBody = encDataBuf;
                            closeMsg.ContainerType = g_dataFormat;

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                msg.EncodedDataBody = encDataBuf;
                            }
                            else
                            {
                                encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                msg.EncodedDataBody = encDataBuf;
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            closeMsg.ContainerType = g_dataFormat;

                            PostEncodeMsg(encIter, msg, extraAction, closeMsg.ExtendedHeader, closeMsg.Flags, CloseMsgFlags.HAS_EXTENDED_HEADER);

                            EncodePayload(encIter);
                            encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.EncodedDataBody.Clear();
                            closeMsg.ContainerType = DataTypes.NO_DATA;
                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                        }

                        SetupDecodeIterator();
                        SetupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            Assert.Equal(MsgClasses.CLOSE, decIter.ExtractMsgClass());
                            SetupDecodeIterator();

                            /* extract the domainType */
                            Assert.Equal(domainType, decIter.ExtractDomainType());
                            SetupDecodeIterator();

                            /* extract streamId */
                            Assert.Equal(streamId, decIter.ExtractStreamId());

                            /* replace the streamId */
                            Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));
                            SetupEncodeIterator();

                            /* extract the streamId */
                            Assert.Equal(streamId + 1, decIter.ExtractStreamId());
                            SetupDecodeIterator();
                        }

                        Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
                        Assert.Equal(MsgClasses.CLOSE, msg.MsgClass);
                        Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                        Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                        Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                        Assert.Equal(closeMask, closeMsg.Flags);

                        /* Check Extended Header */
                        if ((closeMask & CloseMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            Assert.Equal(extendedHeaderLen, closeMsg.ExtendedHeader.Length);
                            Assert.True(IsEqual(closeMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                        }

                        /* Check Payload */
                        if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                        {
                            decodePayload(decIter);
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void CloseMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, closeMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, closeMasksCommon, closeMasksCommon.Length, false);
            CloseMsgTest(repeatCount);
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void CloseMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, closeMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, closeMasksAll, closeMasksAll.Length, false);
            CloseMsgTest(repeatCount);
        }

        private void ackMsgTest(int repeat)
        {
            int extraAction;

            int ackMask;

            int msgKeyMask;
            int decodeMsgKeyMask;

            IAckMsg ackMsg = msg as IAckMsg;

            int domainType = (int)DomainType.TRANSACTION;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    ackMask = masks[masksIter];
                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        for (keyMasksIter = 0; keyMasksIter < ((ackMask & AckMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize : 1); ++keyMasksIter)
                        {
                            msgKeyMask = keyMasks[keyMasksIter];
                            encBuf.Data().WritePosition = 0;
                            SetupEncodeIterator();

                            /* Encode msg */
                            ackMsg.AckId = 11; // make sure clear method clears ackId
                            ackMsg.Clear(); // make sure clear method clears ackId
                            Assert.Equal(0, ackMsg.AckId); // make sure clear method clears ackId

                            ackMsg.MsgClass = MsgClasses.ACK;
                            ackMsg.StreamId = streamId;
                            ackMsg.DomainType = domainType;

                            ackMsg.Flags = ackMask;
                            ackMsg.AckId = ackMask;

                            /* Add nakCode */
                            if ((ackMask & AckMsgFlags.HAS_NAK_CODE) > 0)
                            {
                                ackMsg.NakCode = ackMask + 1;
                            }

                            if ((ackMask & AckMsgFlags.HAS_TEXT) > 0)
                            {
                                ackMsg.Text.Data(text);
                            }

                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                                ackMsg.SeqNum = seqNum;

                            /* Add msgKey */
                            if ((ackMask & AckMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                ackMsg.MsgKey.Flags = msgKeyMask;
                                EncodeMsgKey(ackMsg.MsgKey);
                            }

                            /* Add Extended Header */
                            /*
                             * always set it here to cover our pre-encoded
                             * data/EncodeMsg cases.
                             */
                            if ((ackMask & AckMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                ackMsg.ExtendedHeader.Data(extendedHeader);
                            }

                            /* Add Payload */
                            if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                            {
                                msg.EncodedDataBody = encDataBuf;
                                ackMsg.ContainerType = g_dataFormat;

                                /* Trim payload length if desired */
                                if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                    msg.EncodedDataBody = encDataBuf;
                                }
                                else
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                    msg.EncodedDataBody = encDataBuf;
                                }

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }
                            else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                            {
                                ackMsg.ContainerType = g_dataFormat;

                                /*
                                 * if our key opaque and our extended header are
                                 * pre-encoded, EncodeMsgInit should tell us to
                                 * encode our payload/container
                                 */
                                PostEncodeMsg(encIter, msg, extraAction, ackMsg.ExtendedHeader, ackMsg.Flags, AckMsgFlags.HAS_EXTENDED_HEADER);

                                EncodePayload(encIter);
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                            }
                            else
                            {
                                msg.EncodedDataBody.Clear();
                                ackMsg.ContainerType = DataTypes.NO_DATA;
                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }

                            /* Decode msg */
                            decodeMsgKeyMask = FixDecodeMsgKeyMask(msgKeyMask);

                            SetupDecodeIterator();
                            SetupEncodeIterator();

                            if (g_changeData)
                            {
                                /* extract the msgClass */
                                Assert.Equal(MsgClasses.ACK, decIter.ExtractMsgClass());
                                SetupDecodeIterator();

                                /* extract the domainType */
                                Assert.Equal(domainType, decIter.ExtractDomainType());
                                SetupDecodeIterator();

                                /* extract streamId */
                                Assert.Equal(streamId, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* replace the streamId */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));
                                SetupEncodeIterator();

                                /* extract the streamId */
                                Assert.Equal(streamId + 1, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* extract the seqNum */
                                if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum, decIter.ExtractSeqNum());

                                SetupDecodeIterator();

                                /* replace the seqNum */
                                Assert.Equal(((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE),
                                             encIter.ReplaceSeqNum(seqNum + 1));
                                SetupEncodeIterator();

                                /* extract the new seqNum */
                                if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum + 1, decIter.ExtractSeqNum());
                                SetupDecodeIterator();
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
                            Assert.Equal(MsgClasses.ACK, msg.MsgClass);
                            Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                            Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                            Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                            Assert.Equal(ackMask, ackMsg.Flags);

                            /* Check status */
                            if ((ackMask & AckMsgFlags.HAS_TEXT) > 0)
                            {
                                Assert.Equal(textLen, ackMsg.Text.Length);
                                Assert.True(IsEqual(ackMsg.Text, text, textLen));
                            }

                            if ((ackMask & AckMsgFlags.HAS_NAK_CODE) > 0)
                            {
                                Assert.Equal(ackMask + 1, ackMsg.NakCode);
                            }

                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                Assert.Equal((g_changeData ? seqNum + 1 : seqNum), ackMsg.SeqNum);
                            }

                            /* Check msgKey */
                            if ((ackMask & AckMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                Assert.Equal(decodeMsgKeyMask, ackMsg.MsgKey.Flags);
                                DecodeMsgKey(decIter, ackMsg.MsgKey);
                            }

                            /* Check Extended Header */
                            if ((ackMask & AckMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                Assert.Equal(extendedHeaderLen, ackMsg.ExtendedHeader.Length);
                                Assert.True(IsEqual(ackMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                            }

                            /* Check Payload */
                            if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                            {
                                decodePayload(decIter);
                            }
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void AckMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, ackMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, ackMasksCommon, ackMasksCommon.Length, false);
            ackMsgTest(repeatCount);
        }

        [Fact]
        [Category("Message")]
        public void AckMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, ackMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, ackMasksAll, ackMasksAll.Length, false);
            ackMsgTest(1);
        }

        private void PostMsgTest(int repeat)
        {
            int extraAction;

            int postMask;

            int msgKeyMask;
            int decodeMsgKeyMask;

            IPostMsg postMsg = msg as IPostMsg;

            int domainType = (int)DomainType.MARKET_PRICE;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    postMask = masks[masksIter];

                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        for (keyMasksIter = 0; keyMasksIter < ((postMask & PostMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize : 1); ++keyMasksIter)
                        {
                            msgKeyMask = keyMasks[keyMasksIter];
                            encBuf.Data().WritePosition = 0;
                            SetupEncodeIterator();

                            /* Encode msg */

                            postMsg.Clear();

                            postMsg.MsgClass = MsgClasses.POST;
                            postMsg.StreamId = streamId;
                            postMsg.DomainType = domainType;
                            postMsg.Flags = postMask;

                            /* Add msgKey */
                            if ((postMask & PostMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                postMsg.MsgKey.Flags = msgKeyMask;
                                EncodeMsgKey(postMsg.MsgKey);
                            }

                            /* Add Post ID */
                            if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                            {
                                postMsg.PostId = postId;

                            }

                            if ((postMask & PostMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                Assert.Equal(permissionDataLen, postMsg.PermData.Length);
                                Assert.True(IsEqual(postMsg.PermData, permissionData, permissionDataLen));
                            }

                            /* Set up user information */
                            postMsg.PostUserInfo.UserAddrFromString("204.170.85.29");
                            postMsg.PostUserInfo.UserId = 0xCCAA551DL;

                            /* Add Post Sequence Number */
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(postMask + 0xFFFF) : (int)postMask;
                                postMsg.SeqNum = seqNum;
                            }

                            if ((postMask & PostMsgFlags.HAS_PART_NUM) > 0)
                            {
                                postMsg.PartNum = 0x100f;
                            }

                            if ((postMask & PostMsgFlags.HAS_POST_USER_RIGHTS) > 0)
                            {
                                postMsg.PostUserRights = 0x0001;
                            }

                            /* Add Extended Header */
                            if ((postMask & PostMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                postMsg.ExtendedHeader.Data(extendedHeader);
                            }

                            /* Add Payload */
                            if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                            {
                                msg.EncodedDataBody = encDataBuf;
                                postMsg.ContainerType = g_dataFormat;

                                /* Trim payload length if desired */
                                if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                    msg.EncodedDataBody = encDataBuf;
                                }
                                else
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                    msg.EncodedDataBody = encDataBuf;
                                }

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }
                            else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                            {
                                postMsg.ContainerType = g_dataFormat;

                                PostEncodeMsg(encIter, msg, extraAction, postMsg.ExtendedHeader, postMsg.Flags, UpdateMsgFlags.HAS_EXTENDED_HEADER);

                                EncodePayload(encIter);
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                            }
                            else
                            {
                                msg.EncodedDataBody.Clear();
                                postMsg.ContainerType = DataTypes.NO_DATA;
                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
                            }

                            /* Decode msg */
                            decodeMsgKeyMask = FixDecodeMsgKeyMask(msgKeyMask);

                            SetupDecodeIterator();
                            SetupEncodeIterator();

                            if (g_changeData)
                            {
                                /* extract the msgClass */
                                Assert.Equal(MsgClasses.POST, decIter.ExtractMsgClass());
                                SetupDecodeIterator();

                                /* extract the domainType */
                                Assert.Equal(domainType, decIter.ExtractDomainType());
                                SetupDecodeIterator();

                                /* extract streamId */
                                Assert.Equal(streamId, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* replace the streamId */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));
                                SetupEncodeIterator();

                                /* extract the streamId */
                                Assert.Equal(streamId + 1, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* extract the seqNum */
                                if ((postMask & PostMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum, decIter.ExtractSeqNum());
                                SetupDecodeIterator();

                                /* replace the seqNum */
                                Assert.Equal(((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE),
                                             encIter.ReplaceSeqNum(seqNum + 1));
                                SetupEncodeIterator();

                                /* extract the new seqNum */
                                if ((postMask & PostMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum + 1, decIter.ExtractSeqNum());
                                SetupDecodeIterator();

                                /* extract the postId */
                                if ((postMask & PostMsgFlags.HAS_POST_ID) == 0)
                                    Assert.Equal(-1, decIter.ExtractPostId());
                                if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                                    Assert.Equal(postId, decIter.ExtractPostId());
                                SetupDecodeIterator();

                                /* replace the postId */
                                Assert.Equal(((postMask & PostMsgFlags.HAS_POST_ID) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE),
                                             encIter.ReplacePostId(postId + 1));
                                SetupEncodeIterator();

                                /* extract the new postId */
                                if ((postMask & PostMsgFlags.HAS_POST_ID) == 0)
                                    Assert.Equal(-1, decIter.ExtractPostId());
                                if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                                    Assert.Equal(postId + 1, decIter.ExtractPostId());
                                SetupDecodeIterator();
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
                            Assert.Equal(MsgClasses.POST, msg.MsgClass);
                            Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                            Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                            Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                            Assert.Equal(postMask, postMsg.Flags);

                            /* Check msgKey */
                            if ((postMask & PostMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                Assert.Equal(decodeMsgKeyMask, postMsg.MsgKey.Flags);
                                DecodeMsgKey(decIter, postMsg.MsgKey);
                            }

                            /* Check Post ID */
                            if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                            {
                                Assert.Equal((g_changeData ? postId + 1 : postId), postMsg.PostId);
                            }

                            /* Check Post User Info */
                            Assert.Equal(0xCCAA551DL, postMsg.PostUserInfo.UserAddr);

                            Assert.Equal(0xCCAA551DL, postMsg.PostUserInfo.UserId);

                            /* Check Item Sequence Number */
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                Assert.Equal((g_changeData ? seqNum + 1 : seqNum), postMsg.SeqNum);
                            }

                            if ((postMask & PostMsgFlags.HAS_PART_NUM) > 0)
                            {
                                Assert.Equal(0x100f, postMsg.PartNum);
                            }

                            /* Check Post User Rights */
                            if ((postMask & PostMsgFlags.HAS_POST_USER_RIGHTS) > 0)
                            {
                                Assert.Equal(0x0001, postMsg.PostUserRights);
                            }

                            /* Check Permission Info */
                            if ((postMask & PostMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                Assert.Equal(permissionDataLen, postMsg.PermData.Length);
                                Assert.True(IsEqual(postMsg.PermData, permissionData, permissionDataLen));
                            }

                            /* Check Extended Header */
                            if ((postMask & PostMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                Assert.Equal(extendedHeaderLen, postMsg.ExtendedHeader.Length);
                                Assert.True(IsEqual(postMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                            }

                            /* Check Payload */
                            if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                            {
                                decodePayload(decIter);
                            }
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void PostMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, postMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, postMasksCommon, postMasksCommon.Length, false);
            PostMsgTest(repeatCount);
        }

        [Fact]
        [Category("Message")]
        public void PostMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, postMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, postMasksAll, postMasksAll.Length, false);
            PostMsgTest(1);
        }

        private void GenericMsgTest(int repeat)
        {
            int extraAction;

            int genericMask;

            int msgKeyMask;
            int decodeMsgKeyMask;

            IGenericMsg genMsg = msg as IGenericMsg;

            int domainType = (int)DomainType.STORY;

            for (; repeat > 0; --repeat)
            {
                for (masksIter = 0; masksIter < masksSize; ++masksIter)
                {
                    genericMask = masks[masksIter];

                    for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                    {
                        extraAction = actions[actionsIter];
                        for (keyMasksIter = 0; keyMasksIter < ((genericMask & GenericMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize : 1); ++keyMasksIter)
                        {
                            msgKeyMask = keyMasks[keyMasksIter];
                            encBuf.Data().WritePosition = 0;
                            SetupEncodeIterator();

                            /* Encode msg */

                            genMsg.Clear();

                            genMsg.MsgClass = MsgClasses.GENERIC;
                            genMsg.StreamId = streamId;
                            genMsg.DomainType = domainType;
                            genMsg.Flags = genericMask;

                            /* Add msgKey */
                            if ((genericMask & GenericMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                genMsg.MsgKey.Flags = msgKeyMask;
                                EncodeMsgKey(genMsg.MsgKey);
                            }

                            /* Add Permission Info */
                            if ((genericMask & GenericMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                genMsg.PermData.Data(permissionData);
                            }

                            /* Add Item Sequence Number */
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF) : (int)genericMask;
                                genMsg.SeqNum = seqNum;
                            }

                            if ((genericMask & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
                            {
                                genMsg.SecondarySeqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF) : (int)genericMask;
                            }

                            if ((genericMask & GenericMsgFlags.HAS_PART_NUM) > 0)
                            {
                                genMsg.PartNum = 0x100f;
                            }

                            /* Add Extended Header */
                            if ((genericMask & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                genMsg.ExtendedHeader.Data(extendedHeader);
                            }

                            /* test setGenericCompleteFlag()/unsetGenericCompleteFlag(), remove/add it here first, then add/remove later*/
                            int genMsgFlags = genMsg.Flags;
                            if ((genericMask & GenericMsgFlags.MESSAGE_COMPLETE) > 0)
                                genMsg.Flags = genMsgFlags & ~GenericMsgFlags.MESSAGE_COMPLETE;
                            else
                                genMsg.Flags = genMsgFlags | GenericMsgFlags.MESSAGE_COMPLETE;

                            /* Add Payload */
                            if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                            {
                                msg.EncodedDataBody = encDataBuf;
                                genMsg.ContainerType = g_dataFormat;

                                /* Trim payload length if desired */
                                if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);
                                    msg.EncodedDataBody = encDataBuf;
                                }
                                else
                                {
                                    encDataBuf.Data(encDataBuf.Data(), 0, encDataBuf.Length);
                                    msg.EncodedDataBody = encDataBuf;
                                }

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));

                                /* Call SetGenericCompleteFlag()/UnsetGenericCompleteFlag()  on EncodeIterator to reset back to the original genericMask*/
                                if ((genericMask & GenericMsgFlags.MESSAGE_COMPLETE) > 0)
                                    Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetGenericCompleteFlag());
                                else
                                    Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetGenericCompleteFlag());

                            }
                            else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                            {
                                genMsg.ContainerType = g_dataFormat;

                                PostEncodeMsg(encIter, msg, extraAction, genMsg.ExtendedHeader, genMsg.Flags, GenericMsgFlags.HAS_EXTENDED_HEADER);

                                EncodePayload(encIter);
                                encDataBuf.Data(encDataBuf.Data(), 0, (int)encDataBuf.Data().Position);

                                /* Call setGenericCompleteFlag()/unsetGenericCompleteFlag()  on EncodeIterator to reset back to the original genericMask*/
                                if ((genericMask & GenericMsgFlags.MESSAGE_COMPLETE) > 0)
                                    Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetGenericCompleteFlag());
                                else
                                    Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetGenericCompleteFlag());

                                Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encIter, true));
                            }
                            else
                            {
                                msg.EncodedDataBody.Clear();
                                genMsg.ContainerType = DataTypes.NO_DATA;
                                Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));

                                /* Call setGenericCompleteFlag()/unsetGenericCompleteFlag()  on EncodeIterator to reset back to the original genericMask*/
                                if ((genericMask & GenericMsgFlags.MESSAGE_COMPLETE) > 0)
                                    Assert.Equal(CodecReturnCode.SUCCESS, encIter.SetGenericCompleteFlag());
                                else
                                    Assert.Equal(CodecReturnCode.SUCCESS, encIter.UnsetGenericCompleteFlag());
                            }

                            /* Decode msg */
                            decodeMsgKeyMask = FixDecodeMsgKeyMask(msgKeyMask);

                            SetupDecodeIterator();
                            SetupEncodeIterator();

                            if (g_changeData)
                            {
                                /* extract the msgClass */
                                Assert.Equal(MsgClasses.GENERIC, decIter.ExtractMsgClass());
                                SetupDecodeIterator();

                                /* extract the domainType */
                                Assert.Equal(domainType, decIter.ExtractDomainType());
                                SetupDecodeIterator();

                                /* extract streamId */
                                Assert.Equal(streamId, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* replace the streamId */
                                Assert.Equal(CodecReturnCode.SUCCESS, encIter.ReplaceStreamId(streamId + 1));
                                SetupEncodeIterator();

                                /* extract the streamId */
                                Assert.Equal(streamId + 1, decIter.ExtractStreamId());
                                SetupDecodeIterator();

                                /* extract the seqNum */
                                if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum, decIter.ExtractSeqNum());
                                SetupDecodeIterator();

                                /* replace the seqNum */
                                Assert.Equal(((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCode.SUCCESS : CodecReturnCode.FAILURE),
                                             encIter.ReplaceSeqNum(seqNum + 1));
                                SetupEncodeIterator();

                                /* extract the new seqNum */
                                if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                                    Assert.Equal(-1, decIter.ExtractSeqNum());
                                if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                                    Assert.Equal(seqNum + 1, decIter.ExtractSeqNum());
                                SetupDecodeIterator();
                            }

                            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(decIter));
                            Assert.Equal(MsgClasses.GENERIC, msg.MsgClass);
                            Assert.Equal(((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.ContainerType);
                            Assert.Equal((g_changeData ? streamId + 1 : streamId), msg.StreamId);
                            Assert.Equal((g_changeData ? domainType : domainType), msg.DomainType);

                            Assert.Equal(genericMask, genMsg.Flags);

                            /* Check msgKey */
                            if ((genericMask & GenericMsgFlags.HAS_MSG_KEY) > 0)
                            {
                                Assert.Equal(decodeMsgKeyMask, genMsg.MsgKey.Flags);
                                DecodeMsgKey(decIter, genMsg.MsgKey);
                            }

                            /* Check Permission Info */
                            if ((genericMask & GenericMsgFlags.HAS_PERM_DATA) > 0)
                            {
                                Assert.Equal(permissionDataLen, genMsg.PermData.Length);
                                Assert.True(IsEqual(genMsg.PermData, permissionData, permissionDataLen));
                            }

                            /* Check Item Sequence Number */
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                Assert.Equal((g_changeData ? seqNum + 1 : seqNum), genMsg.SeqNum);
                            }

                            /* Check Item Secondary Sequence Number */
                            if ((genericMask & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
                            {
                                Assert.Equal((extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF) : (int)genericMask,
                                             genMsg.SecondarySeqNum);
                            }

                            if ((genericMask & GenericMsgFlags.HAS_PART_NUM) > 0)
                            {
                                Assert.Equal(0x100f, genMsg.PartNum);
                            }

                            /* Check Extended Header */
                            if ((genericMask & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0)
                            {
                                Assert.Equal(extendedHeaderLen, genMsg.ExtendedHeader.Length);
                                Assert.True(IsEqual(genMsg.ExtendedHeader, extendedHeader, extendedHeaderLen));
                            }

                            /* Check Payload */
                            if ((extraAction & ExtraTestAction.PAYLOAD) > 0)
                            {
                                decodePayload(decIter);
                            }
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        [Category("Message")]
        public void GenericMessageCommonTest()
        {
            TestSetupCommon();
            masksSize = (int)System.Math.Pow(2, genericMasksCommon.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, genericMasksCommon, genericMasksCommon.Length, false);
            GenericMsgTest(repeatCount);
        }

        [Fact]
        [Category("Message")]
        public void GenericMessageFullTest()
        {
            TestSetupFull();
            masksSize = (int)System.Math.Pow(2, genericMasksAll.Length);
            masks = new int[masksSize];
            AllocateFlagCombinations(masks, genericMasksAll, genericMasksAll.Length, false);
            GenericMsgTest(1);

        }
    }
}



