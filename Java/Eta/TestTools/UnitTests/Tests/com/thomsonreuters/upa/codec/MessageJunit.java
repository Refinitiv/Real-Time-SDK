///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;
import org.junit.Test;

import com.thomsonreuters.upa.codec.AckMsgFlags;
import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsgFlags;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Encoders;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.FieldListFlags;
import com.thomsonreuters.upa.codec.GenericMsgFlags;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.PostMsgFlags;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;

public class MessageJunit
{
	int i = 0;
    Msg msg = CodecFactory.createMsg();
    /* Encode nested Msg */
    int g_dataFormat = DataTypes.FIELD_LIST;;
    boolean g_changeData = true;

    /* set/unset flag after encoding */
    boolean g_setRefreshFlags = false;
    boolean g_unsetRefreshFlags = false;
    boolean g_setRequestFlags = false;
    boolean g_unsetRequestFlags = false;

    int streamId = 2;
    String extendedHeader = "extendedHeader";
    int extendedHeaderLen = extendedHeader.length();

    String exchangeId = "NYSE";
    int exchangeIdLen = exchangeId.length();

    String permissionData = "permission";
    int permissionDataLen = permissionData.length();

    int permissionEntity = 42;

    String text = "Acknowledged";
    int textLen = text.length();

    byte[] groupId = { 0, 3 };
    int groupIdLen = groupId.length;

    String stateText = "Source Unavailable";
    int stateTextLen = stateText.length();

    long seqNum = 12345;

    int postId = 123456;

    /* Msg Key */
    String payloadName = "TRI.N";
    int payloadNameLen = payloadName.length();
    int opaqueLen = 1000;
    ByteBuffer opaque = ByteBuffer.allocate(opaqueLen);

    private int repeatCount = 1;
    private boolean commonSetupComplete = false, fullSetupComplete = false;
	//Using all primitives makes the top container too big to test nested structure to maximum depth 
    private final int FIELD_LIST_MAX_ENTRIES = 6; 
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
    Int fieldListInt = CodecFactory.createInt();
    Double fieldListFloat = CodecFactory.createDouble();
    Real fieldListReal = CodecFactory.createReal();
    Date fieldListDate = CodecFactory.createDate();
    Time fieldListTime = CodecFactory.createTime();
    DateTime fieldListDateTime = CodecFactory.createDateTime();

    /* Buffer sizes */
    private int dataBufferSize = 1024;
    private int msgBufferSize = 1024 + 512;

    /* Iterators and Buffers for writing/reading */
    Buffer encBuf = CodecFactory.createBuffer();
    Buffer encDataBuf = CodecFactory.createBuffer();
    {
        encBuf.data(ByteBuffer.allocate(msgBufferSize));
        encDataBuf.data(ByteBuffer.allocate(dataBufferSize));
    }
    Buffer encNestedBuf, encTempBuf;
    EncodeIterator encIter = CodecFactory.createEncodeIterator(),
            encDataIter = CodecFactory.createEncodeIterator();
    DecodeIterator decIter = CodecFactory.createDecodeIterator(),
            decDataIter = CodecFactory.createDecodeIterator();

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
        public static final int POST_PAYLOAD = 0x0001; //Add post-encoded payload to the message (if PRE_PAYLOAD is not set)
        public static final int PRE_PAYLOAD = 0x0002;  // Add a pre-encoded payload to the message
        public static final int FOUR_BYTE_SEQ = 0x0004; // Test a sequence number with 4 bytes instead of 2
        public static final int TRIM_DATA_BUF = 0x0008; // Test trimming the data buffer size to the actual encoded length
        public static final int POST_OPAQUE = 0x0010; // Test post encoding of opaque - only applies in EncodeMsgInit cases 
        public static final int POST_EXTENDED = 0x0020; // Test post encoding of extended header - only applies in EncodeMsgInit cases
                                                         
        public static final int PAYLOAD = POST_PAYLOAD | PRE_PAYLOAD; //For Decoder -- indicates payload whether pre or post -encoded
    }

    public MessageJunit()
    {
        fieldListReal.value(0xFFFFFL, 1);
        fieldListDate.day(8);
        fieldListDate.month(3);
        fieldListDate.year(1892);
        fieldListTime.hour(23);
        fieldListTime.minute(59);
        fieldListTime.second(59);
        fieldListTime.millisecond(999);
        fieldListDateTime.day(8);
        fieldListDateTime.month(3);
        fieldListDateTime.year(1892);
        fieldListDateTime.hour(23);
        fieldListDateTime.minute(59);
        fieldListDateTime.second(59);
        fieldListDateTime.millisecond(999);

        /*
         * Encode Payload Data for pre-encoded data tests( a nested msg or
         * fieldList, depending on cmdline )
         */
        setupEncodeDataIterator();
        encodePayload(encDataIter);
    }

    private void setupEncodeIterator()
    {
        encIter.clear();
        encIter.setBufferAndRWFVersion(encBuf, Codec.majorVersion(), Codec.minorVersion());
    }

    private void setupDecodeIterator()
    {
        decIter.clear();
        decIter.setBufferAndRWFVersion(encBuf, Codec.majorVersion(), Codec.minorVersion());
    }

    private void setupEncodeDataIterator()
    {
        encDataIter.clear();
        encDataIter.setBufferAndRWFVersion(encDataBuf, Codec.majorVersion(), Codec.minorVersion());
    }

    /*
     * private void setupDecodeDataIterator() { decDataIter.clear(); }
     */

    private void encodePayload(EncodeIterator encIter)
    {
        if (g_dataFormat == DataTypes.MSG)
            encodeNestedMsg(encIter);
        else
            // FIELD_LIST
            encodeFieldList(encIter);
    }

    /* Encode a payload that is actually another message */
    void encodeNestedMsg(EncodeIterator encIter)
    {
        UpdateMsg updMsg = (UpdateMsg)CodecFactory.createMsg();

        updMsg.msgClass(MsgClasses.UPDATE);
        updMsg.containerType(DataTypes.FIELD_LIST);
        updMsg.streamId(streamId);
        updMsg.domainType(DomainTypes.MARKET_PRICE);
        updMsg.flags(UpdateMsgFlags.HAS_EXTENDED_HEADER | UpdateMsgFlags.HAS_MSG_KEY);
        updMsg.updateType(3);

        updMsg.extendedHeader().data(extendedHeader);

        updMsg.msgKey().flags(MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_FILTER | MsgKeyFlags.HAS_IDENTIFIER | MsgKeyFlags.HAS_ATTRIB);
        encodeMsgKey(updMsg.msgKey());

        assertEquals("EncodeMsgInit", CodecReturnCodes.ENCODE_CONTAINER, updMsg.encodeInit(encIter, 0));
        encodeFieldList(encIter);

        assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, updMsg.encodeComplete(encIter, true));
    }

    void encodeFieldList(EncodeIterator encIter)
    {
        /*
         * construct field list This field list will be used as every field list
         * in the structure
         */

        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();
        int iiEntry;

        fieldList.clear();

        fieldList.flags(FieldListFlags.HAS_STANDARD_DATA);

        /* init */
        assertEquals("EncodeFieldListInit", CodecReturnCodes.SUCCESS, fieldList.encodeInit(encIter, null, 0));

        /* add entries */
        for (iiEntry = 0; iiEntry < FIELD_LIST_MAX_ENTRIES; ++iiEntry)
        {
            entry.clear();
            entry.fieldId(fieldListFids[iiEntry]);
            entry.dataType(fieldListDataTypes[iiEntry]);

            switch (fieldListDataTypes[iiEntry])
            {
                case DataTypes.INT:
                    assertEquals("EncodeFieldEntry", CodecReturnCodes.SUCCESS, entry.encode(encIter, fieldListInt));
                    break;
                case DataTypes.DOUBLE:
                    assertEquals("EncodeFieldEntry", CodecReturnCodes.SUCCESS, entry.encode(encIter, fieldListFloat));
                    break;
                case DataTypes.REAL:
                    assertEquals("EncodeFieldEntry", CodecReturnCodes.SUCCESS, entry.encode(encIter, fieldListReal));
                    break;
                case DataTypes.DATE:
                    assertEquals("EncodeFieldEntry", CodecReturnCodes.SUCCESS, entry.encode(encIter, fieldListDate));
                    break;
                case DataTypes.TIME:
                    assertEquals("EncodeFieldEntry", CodecReturnCodes.SUCCESS, entry.encode(encIter, fieldListTime));
                    break;
                case DataTypes.DATETIME:
                    assertEquals("EncodeFieldEntry", CodecReturnCodes.SUCCESS, entry.encode(encIter, fieldListDateTime));
                    break;
                default:
                    assertTrue("Error in _encodeFieldList()", false);
                    break;
            }
        }

        /* finish encoding */
        assertEquals("EncodeFieldListComplete", CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encIter, true));
    }

    private void encodeMsgKey(MsgKey key)
    {
        int mask = key.flags();

        /* Add Service ID */
        if ((mask & MsgKeyFlags.HAS_SERVICE_ID) > 0)
            key.serviceId(7);

        /* Add Name */
        if ((mask & MsgKeyFlags.HAS_NAME) > 0)
        {
            key.name().data(payloadName);
        }

        /* Add Name Type */
        if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0)
            key.nameType(InstrumentNameTypes.RIC);

        /* Add Filter */
        if ((mask & MsgKeyFlags.HAS_FILTER) > 0)
            key.filter(4294967294L);

        /* Add ID */
        if ((mask & MsgKeyFlags.HAS_IDENTIFIER) > 0)
            key.identifier(9001);

        /* Add Attrib ContainerType/Data *//* Rssl calls this "opaque" */
        if ((mask & MsgKeyFlags.HAS_ATTRIB) > 0)
        {
            EncodeIterator encIter = CodecFactory.createEncodeIterator();
            opaque.position(0);
            key.encodedAttrib().data(opaque, 0, opaqueLen);
            encIter.setBufferAndRWFVersion(key.encodedAttrib(), Codec.majorVersion(), Codec.minorVersion());
            encodeFieldList(encIter);
            key.attribContainerType(DataTypes.FIELD_LIST);
        }
    }

    /*
     * does the work for encoding opaque and/or extended header after MsgInit is
     * called
     */
    private void postEncodeMsg(EncodeIterator encIter, Msg msg, int extraAction, Buffer extHdr, int msgFlags, int extHdrFlag)
    {

        int ret = CodecReturnCodes.ENCODE_CONTAINER;

        if ((extraAction & ExtraTestAction.POST_OPAQUE) > 0 && msg.msgKey() != null && msg.msgKey().checkHasAttrib())
        {
            /* if we are post encoding the opaque, make it a field list */
            msg.msgKey().encodedAttrib().clear();
            msg.msgKey().attribContainerType(DataTypes.FIELD_LIST);
			// if our opaque gets encoded after, this should be our ret
            ret = CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB; 

            if ((extraAction & ExtraTestAction.POST_EXTENDED) > 0 && extHdr != null)
            {
                /*
                 * if we are also post encoding our extended header, unset its
                 * values
                 */
                extHdr.clear();
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
            extHdr.clear();
            ret = CodecReturnCodes.ENCODE_EXTENDED_HEADER;
        }

        assertEquals("EncodeMsgInit", ret, msg.encodeInit(encIter, 0));

        if (ret == CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB)
        {
            /* we need to encode our opaque here, should be a field list */

            encodeFieldList(encIter);

            if ((extraAction & ExtraTestAction.POST_EXTENDED) > 0 && (msgFlags & extHdrFlag) > 0)
            {
                /* we should have to encode our extended header next */
                ret = CodecReturnCodes.ENCODE_EXTENDED_HEADER;
            }
            else
            {
                /*
                 * if our extended header is not being post encoded, it should
                 * be completed by the KeyOpaquecomplete call or is not present
                 */
                ret = CodecReturnCodes.ENCODE_CONTAINER;
            }

            assertEquals("EncodeMsgKeyAttribComplete", ret, msg.encodeKeyAttribComplete(encIter, true));
        }

        if ((ret == CodecReturnCodes.ENCODE_EXTENDED_HEADER) && extHdr != null)
        {
            Buffer buffer = CodecFactory.createBuffer();

            /* we must encode our extended header now */
            /* just hack copy it onto the wire */
            assertEquals("EncodeNonRWFDataTypeInit", CodecReturnCodes.SUCCESS, Encoders.encodeNonRWFInit(encIter, buffer));
            assertTrue("Length Failure", buffer.length() >= extendedHeaderLen);
            buffer.data().put(extendedHeader.getBytes());
            assertEquals("EncodeNonRWFDataTypeComplete", CodecReturnCodes.SUCCESS, Encoders.encodeNonRWFComplete(encIter, buffer, true));

            assertEquals("EncodeExtendedHeaderComplete", CodecReturnCodes.ENCODE_CONTAINER, msg.encodeExtendedHeaderComplete(encIter, true));
        }
    }

    /*
     * Remove flags that the encoder should have removed because the test
     * is(intentionally) trying to do something wrong
     */
    private int fixDecodeMsgKeyMask(int mask)
    {
		// Ensure that NameType is not set without a Name
        if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0 && !((mask & MsgKeyFlags.HAS_NAME) > 0)) 
            mask &= ~MsgKeyFlags.HAS_NAME_TYPE;

        return mask;
    }

    private boolean isMemEqual(Buffer buf1, Buffer buf2, int len)
    {
        boolean retVal = true;
        int pos1 = buf1.position(), pos2 = buf2.position();
        ByteBuffer data1 = buf1.data(), data2 = buf2.data();

        for (int i = 0; i < len; i++)
        {
            if (data1.get(pos1 + i) != data2.get(pos2 + i))
            {
                retVal = false;
                break;
            }
        }

        return retVal;
    }

    private boolean isMemEqual(Buffer buf1, String buf2, int len)
    {
        boolean retVal = true;
        int pos1 = buf1.position();
        ByteBuffer data1 = buf1.data();

        for (int i = 0; i < len; i++)
        {
            if (data1.get(pos1 + i) != buf2.charAt(i))
            {
                retVal = false;
                break;
            }
        }

        return retVal;
    }

    private void decodeMsgKey(DecodeIterator decIter, MsgKey key)
    {
        int mask = key.flags();

        /* Check Service ID */
        if ((mask & MsgKeyFlags.HAS_SERVICE_ID) > 0)
            assertEquals("Correct Service ID in MsgKey", 7, key.serviceId());

        /* Check Name */
        if ((mask & MsgKeyFlags.HAS_NAME) > 0)
        {
            assertEquals("Correct Name in MsgKey", payloadNameLen, key.name().length());
            assertEquals("Correct Name in MsgKey", true, isMemEqual(key.name(), payloadName, payloadNameLen));
        }

        /* Check Name Type */
        if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0)
            assertEquals("Correct Name Type in MsgKey", InstrumentNameTypes.RIC, key.nameType());

        /* Check Filter */
        if ((mask & MsgKeyFlags.HAS_FILTER) > 0)
            assertEquals("Correct Filter in MsgKey", 4294967294L, key.filter());

        /* Check ID */
        if ((mask & MsgKeyFlags.HAS_IDENTIFIER) > 0)
            assertEquals("Correct ID in MsgKey", 9001, key.identifier());

        /* check opaque */
        if ((mask & MsgKeyFlags.HAS_ATTRIB) > 0)
        {
            assertEquals("Correct attribContainerType", DataTypes.FIELD_LIST, key.attribContainerType());
            assertEquals("DecodeMsgKeyAttrib", CodecReturnCodes.SUCCESS, msg.decodeKeyAttrib(decIter, key));
            decodeFieldList(decIter);
        }
    }

    private void decodePayload(DecodeIterator decIter)
    {
        if (g_dataFormat == DataTypes.MSG)
            decodeNestedMsg(decIter);
        else
            decodeFieldList(decIter);
    }

    private void decodeNestedMsg(DecodeIterator decIter)
    {
        Msg msg = CodecFactory.createMsg();
        UpdateMsg updateMsg = (UpdateMsg)msg;

        assertEquals("nestedMsg trwfDecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
        assertEquals("nestedMsg containerType", DataTypes.FIELD_LIST, msg.containerType());

        assertEquals("nestedMsg msgClass", MsgClasses.UPDATE, msg.msgClass());
        assertEquals("nestedMsg containerType", DataTypes.FIELD_LIST, msg.containerType());
        assertEquals("nestedMsg streamId", streamId, msg.streamId());
        assertEquals("nestedMsg domainType", DomainTypes.MARKET_PRICE, msg.domainType());
        assertEquals("nestedMsg flags", (UpdateMsgFlags.HAS_EXTENDED_HEADER | UpdateMsgFlags.HAS_MSG_KEY), updateMsg.flags());
        assertEquals("extendedHeader", extendedHeaderLen, updateMsg.extendedHeader().length());
        assertEquals("extendedHeader", true, isMemEqual(updateMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
        assertEquals("nestedMsg updateType", 3, updateMsg.updateType());

        assertEquals("nestedMsg key flags", (MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_FILTER | MsgKeyFlags.HAS_IDENTIFIER | MsgKeyFlags.HAS_ATTRIB), msg.msgKey().flags());
        decodeMsgKey(decIter, msg.msgKey());

        decodeFieldList(decIter);
    }

    private void decodeFieldList(DecodeIterator decIter)
    {

        Int decInt = CodecFactory.createInt();
        Real decReal = CodecFactory.createReal();
        Date decDate = CodecFactory.createDate();
        Time decTime = CodecFactory.createTime();
        Double decFloat = CodecFactory.createDouble();
        DateTime decDateTime = CodecFactory.createDateTime();

        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();

        int iiEntry;

        // Setup container
        container.clear();
        entry.clear();

        // Begin container decoding
        assertEquals("DecodeFieldList", CodecReturnCodes.SUCCESS, container.decode(decIter, null));

        assertEquals("FieldList is correct", FieldListFlags.HAS_STANDARD_DATA, container.flags());

        // Decode entries
        for (iiEntry = 0; iiEntry < FIELD_LIST_MAX_ENTRIES; ++iiEntry)
        {
            assertEquals("DecodeFieldEntry", CodecReturnCodes.SUCCESS, entry.decode(decIter));

            assertEquals("Decoded entry is correct", fieldListFids[iiEntry], entry.fieldId());
            assertEquals("Decoded entry is correct", DataTypes.UNKNOWN, entry.dataType());

            switch (fieldListDataTypes[iiEntry])
            {
                case DataTypes.INT:
                    assertEquals("Integer correctly decoded", CodecReturnCodes.SUCCESS, decInt.decode(decIter));
                    assertEquals("Integer correctly decoded", fieldListInt.toLong(), decInt.toLong());
                    break;
                case DataTypes.DOUBLE:
                    assertEquals("Float correctly decoded",
                                 CodecReturnCodes.SUCCESS, decFloat.decode(decIter));
                    /*
                     * not rounded inside encoding/decoding, so this should
                     * match exactly
                     */
                    assertEquals("Float correctly decoded", 0, java.lang.Double.compare(fieldListFloat.toDouble(), decFloat.toDouble()));
                    break;
                case DataTypes.REAL:
                    assertEquals("Real correctly decoded", CodecReturnCodes.SUCCESS, decReal.decode(decIter));
                    assertEquals("Real correctly decoded", false, decReal.isBlank());
                    assertEquals("Real correctly decoded", fieldListReal.hint(), decReal.hint());
                    assertEquals("Real correctly decoded", fieldListReal.toLong(), decReal.toLong());
                    break;
                case DataTypes.DATE:
                    assertEquals("Date correctly decoded", CodecReturnCodes.SUCCESS, decDate.decode(decIter));
                    assertEquals("Date correctly decoded", fieldListDate.day(), decDate.day());
                    assertEquals("Date correctly decoded", fieldListDate.month(), decDate.month());
                    assertEquals("Date correctly decoded", fieldListDate.year(), decDate.year());
                    break;
                case DataTypes.TIME:
                    assertEquals("Time correctly decoded", CodecReturnCodes.SUCCESS, decTime.decode(decIter));
                    assertEquals("Time correctly decoded", fieldListTime.hour(), decTime.hour());
                    assertEquals("Time correctly decoded", fieldListTime.minute(), decTime.minute());
                    assertEquals("Time correctly decoded", fieldListTime.second(), decTime.second());
                    assertEquals("Time correctly decoded", fieldListTime.millisecond(), decTime.millisecond());

                    break;
                case DataTypes.DATETIME:
                    assertEquals("DateTime correctly decoded", CodecReturnCodes.SUCCESS, decDateTime.decode(decIter));
                    assertEquals("DateTime correctly decoded", fieldListDateTime.day(), decDateTime.day());
                    assertEquals("DateTime correctly decoded", fieldListDateTime.month(), decDateTime.month());
                    assertEquals("DateTime correctly decoded", fieldListDateTime.year(), decDateTime.year());
                    assertEquals("DateTime correctly decoded", fieldListDateTime.hour(), decDateTime.hour());
                    assertEquals("DateTime correctly decoded", fieldListDateTime.minute(), decDateTime.minute());
                    assertEquals("DateTime correctly decoded", fieldListDateTime.second(), decDateTime.second());
                    assertEquals("DateTime correctly decoded", fieldListDateTime.millisecond(), decDateTime.millisecond());
                    break;
                default:
                    assertTrue("Error in _decodeFieldList()", false);
                    break;
            }
        }
        assertEquals("DecodeFieldEntry(end of container)", CodecReturnCodes.END_OF_CONTAINER, entry.decode(decIter));
    }

    private void updateMsgTest(int repeat)
    {
        int updateMask;

        int extraAction;

        UpdateMsg updMsg = (UpdateMsg)msg;

        int msgKeyMask;
        int decodeMsgKeyMask;

        int domainType = DomainTypes.LOGIN;

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
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */
                        updMsg.clear();

                        updMsg.msgClass(MsgClasses.UPDATE);
                        updMsg.streamId(streamId);
                        updMsg.domainType(domainType);

                        updMsg.flags(updateMask);
                        updMsg.updateType(3);

                        /* Add msgKey */
                        if ((updateMask & UpdateMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            updMsg.msgKey().flags(msgKeyMask);
                            encodeMsgKey(updMsg.msgKey());
                        }

                        /* Add Permission Info */
                        if ((updateMask & UpdateMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            updMsg.permData().data(permissionData);
                        }

                        /* Add Item Sequence Number */
                        if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(updateMask + 0xFFFF) : (int)updateMask;
                            updMsg.seqNum(seqNum);
                        }

                        /* Add Conflation Info */
                        if ((updateMask & UpdateMsgFlags.HAS_CONF_INFO) > 0)
                        {
                            updMsg.conflationCount(updateMask + 2);
                            updMsg.conflationTime(updateMask + 3);
                        }

                        if ((updateMask & UpdateMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            /* Set up user information */
                            updMsg.postUserInfo().userAddr("204.170.85.29");
                            updMsg.postUserInfo().userId(0xCCAA551DL);
                        }

                        /* Add Extended Header */
                        if ((updateMask & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            updMsg.extendedHeader().data(extendedHeader);
                        }

                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.encodedDataBody(encDataBuf);
                            updMsg.containerType(g_dataFormat);

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                                msg.encodedDataBody(encDataBuf);
                            }
                            else
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                                msg.encodedDataBody(encDataBuf);
                            }

                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            updMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, updMsg.extendedHeader(), updMsg.flags(), UpdateMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            updMsg.containerType(DataTypes.NO_DATA);
                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }

                        /* Decode msg */
                        decodeMsgKeyMask = fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
						setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            assertEquals("ExtractMsgClass", MsgClasses.UPDATE, decIter.extractMsgClass());
                            setupDecodeIterator();

                            /* extract the domainType */
                            assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                            setupDecodeIterator();

                            /* extract streamId */
                            assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* replace the streamId */
                            assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
                            setupEncodeIterator();

                            /* extract the streamId */
                            assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* extract the seqNum */
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum", seqNum, decIter.extractSeqNum());
                            setupDecodeIterator();

                            /* replace the seqNum */
                            assertEquals("ReplaceSeqNum",
                                         ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE),
                                         encIter.replaceSeqNum(seqNum + 1));
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum2", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum2", seqNum + 1, decIter.extractSeqNum());
                            setupDecodeIterator();
                        }

                        assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
                        assertEquals("Correct MsgBase", MsgClasses.UPDATE, msg.msgClass());
                        assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                        assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                        assertEquals("Correct MsgBase", (g_changeData ? domainType : domainType), msg.domainType());

                        assertEquals("Correct Mask", updateMask, updMsg.flags());

                        assertEquals("Correct update Type", 3, updMsg.updateType());

                        /* Check msgKey */
                        if ((updateMask & UpdateMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            assertEquals("Correct Msg Key Mask", decodeMsgKeyMask, updMsg.msgKey().flags());
                            decodeMsgKey(decIter, updMsg.msgKey());
                        }

                        /* Check Permission Info */
                        if ((updateMask & UpdateMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            assertEquals("Correct Permission Info", permissionDataLen, updMsg.permData().length());
                            assertEquals("Correct Permission Info", true, isMemEqual(updMsg.permData(), permissionData, permissionDataLen));
                        }

                        /* Check Item Sequence Number */
                        if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            assertEquals("Correct Item Sequence Number", (g_changeData ? seqNum + 1 : seqNum), updMsg.seqNum());
                        }

                        /* Check Conflation Info */
                        if ((updateMask & UpdateMsgFlags.HAS_CONF_INFO) > 0)
                        {
                            assertEquals("Correct Conflation Info", updateMask + 2, updMsg.conflationCount());
                            assertEquals("Correct Conflation Info", updateMask + 3, updMsg.conflationTime());
                        }

                        if ((updateMask & UpdateMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            assertEquals("Correct Post User Addr", 0xCCAA551DL, updMsg.postUserInfo().userAddr());
                            assertEquals("Correct Post User ID", 0xCCAA551DL, updMsg.postUserInfo().userId());
                        }

                        /* Check Extended Header */
                        if ((updateMask & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            assertEquals("Correct Extended Header", extendedHeaderLen, updMsg.extendedHeader().length());
                            assertEquals("Correct Extended Header", true, isMemEqual(updMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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
    private void allocateFlagCombinations(int[] dstMasks, int[] srcMasks, int srcMasksSize, boolean skipZero)
    {
        int skip = skipZero ? 1 : 0;
        int dstMasksSize = (int)Math.pow(2, srcMasksSize) - skip;
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

    private void testSetupCommon()
    {
        if (!commonSetupComplete)
        {
            actionsSize = (int)Math.pow(2, actionsCommon.length);
            actions = new int[actionsSize];
            allocateFlagCombinations(actions, actionsCommon, actionsCommon.length, false);
            keyMasksSize = (int)Math.pow(2, keyMasksCommon.length) - 1;
            keyMasks = new int[keyMasksSize];
            allocateFlagCombinations(keyMasks, keyMasksCommon, keyMasksCommon.length, true);
            commonSetupComplete = true;
        }
    }

    private void testSetupFull()
    {
        if (!fullSetupComplete)
        {
            actionsSize = (int)Math.pow(2, actionsAll.length);
            actions = new int[actionsSize];
            allocateFlagCombinations(actions, actionsAll, actionsAll.length, false);
            keyMasksSize = (int)Math.pow(2, keyMasksAll.length) - 1;
            keyMasks = new int[keyMasksSize];
            allocateFlagCombinations(keyMasks, keyMasksAll, keyMasksAll.length, true);
            fullSetupComplete = true;
        }
    }
    
    @Test
    public void checkFlagsTest()
    {
    	// AckMsg
    	AckMsg ackMsg = (AckMsg)CodecFactory.createMsg();
    	ackMsg.msgClass(MsgClasses.ACK);
    	assertEquals(false, ackMsg.checkHasExtendedHdr());
    	assertEquals(null, ackMsg.extendedHeader());
    	ackMsg.applyHasExtendedHdr();
    	assertNotNull(ackMsg.extendedHeader());
    	assertEquals(false, ackMsg.checkHasMsgKey());
    	assertEquals(null, ackMsg.msgKey());
    	ackMsg.applyHasMsgKey();
    	assertNotNull(ackMsg.msgKey());
    	assertEquals(false, ackMsg.checkHasNakCode());
    	assertEquals(0, ackMsg.nakCode());
    	ackMsg.applyHasNakCode();
    	ackMsg.nakCode(11);
    	assertEquals(11, ackMsg.nakCode());
    	assertEquals(false, ackMsg.checkHasSeqNum());
    	assertEquals(0, ackMsg.seqNum());
    	ackMsg.applyHasSeqNum();
    	ackMsg.seqNum(22);
    	assertEquals(22, ackMsg.seqNum());
    	assertEquals(false, ackMsg.checkHasText());
    	assertEquals(null, ackMsg.text());
    	assertEquals(false, ackMsg.checkPrivateStream());
    	ackMsg.applyPrivateStream();
    	assertEquals(true, ackMsg.checkPrivateStream());
    	ackMsg.applyHasText();
    	assertNotNull(ackMsg.text());
    	ackMsg.flags(0);
    	assertEquals(false, ackMsg.checkHasExtendedHdr());
    	assertEquals(null, ackMsg.extendedHeader());
    	assertEquals(false, ackMsg.checkHasMsgKey());
    	assertEquals(null, ackMsg.msgKey());
    	assertEquals(false, ackMsg.checkHasNakCode());
    	assertEquals(0, ackMsg.nakCode());
    	assertEquals(false, ackMsg.checkHasSeqNum());
    	assertEquals(0, ackMsg.seqNum());
    	assertEquals(false, ackMsg.checkHasText());
    	assertEquals(null, ackMsg.text());
    	assertEquals(false, ackMsg.checkPrivateStream());
    	
    	// CloseMsg
    	CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
    	closeMsg.msgClass(MsgClasses.CLOSE);
    	assertEquals(false, closeMsg.checkHasExtendedHdr());
    	assertEquals(null, closeMsg.extendedHeader());
    	closeMsg.applyHasExtendedHdr();
    	assertNotNull(closeMsg.extendedHeader());
    	assertEquals(false, closeMsg.checkAck());
    	closeMsg.applyAck();
    	assertEquals(true, closeMsg.checkAck());
    	closeMsg.flags(0);
    	assertEquals(false, closeMsg.checkHasExtendedHdr());
    	assertEquals(null, closeMsg.extendedHeader());
    	assertEquals(false, closeMsg.checkAck());
    	
    	// GenericMsg
    	GenericMsg genericMsg = (GenericMsg)CodecFactory.createMsg();
    	genericMsg.msgClass(MsgClasses.GENERIC);
    	assertEquals(false, genericMsg.checkHasExtendedHdr());
    	assertEquals(null, genericMsg.extendedHeader());
    	genericMsg.applyHasExtendedHdr();
    	assertNotNull(genericMsg.extendedHeader());
    	assertEquals(false, genericMsg.checkHasMsgKey());
    	assertEquals(null, genericMsg.msgKey());
    	genericMsg.applyHasMsgKey();
    	assertNotNull(genericMsg.msgKey());
    	assertEquals(false, genericMsg.checkHasPartNum());
    	assertEquals(0, genericMsg.partNum());
    	genericMsg.applyHasPartNum();
    	genericMsg.partNum(11);
    	assertEquals(11, genericMsg.partNum());
    	assertEquals(false, genericMsg.checkHasPermData());
    	assertEquals(null, genericMsg.permData());
    	genericMsg.applyHasPermData();
    	assertNotNull(genericMsg.permData());
    	assertEquals(false, genericMsg.checkHasSecondarySeqNum());
    	assertEquals(0, genericMsg.secondarySeqNum());
    	genericMsg.applyHasSecondarySeqNum();
    	genericMsg.secondarySeqNum(22);
    	assertEquals(22, genericMsg.secondarySeqNum());    	
    	assertEquals(false, genericMsg.checkHasSeqNum());
    	assertEquals(0, genericMsg.seqNum());
    	genericMsg.applyHasSeqNum();
    	genericMsg.seqNum(33);
    	assertEquals(33, genericMsg.seqNum());
    	assertEquals(false, genericMsg.checkMessageComplete());
    	genericMsg.applyMessageComplete();
    	assertEquals(true, genericMsg.checkMessageComplete());
    	genericMsg.flags(0);
    	assertEquals(false, genericMsg.checkHasExtendedHdr());
    	assertEquals(null, genericMsg.extendedHeader());
    	assertEquals(false, genericMsg.checkHasMsgKey());
    	assertEquals(null, genericMsg.msgKey());
    	assertEquals(false, genericMsg.checkHasPartNum());
    	assertEquals(0, genericMsg.partNum());
    	assertEquals(false, genericMsg.checkHasPermData());
    	assertEquals(null, genericMsg.permData());
    	assertEquals(false, genericMsg.checkHasSecondarySeqNum());
    	assertEquals(0, genericMsg.secondarySeqNum());
    	assertEquals(false, genericMsg.checkHasSeqNum());
    	assertEquals(0, genericMsg.seqNum());
    	assertEquals(false, genericMsg.checkMessageComplete());
    	
    	// PostMsg
    	PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
    	postMsg.msgClass(MsgClasses.POST);
    	assertEquals(false, postMsg.checkHasExtendedHdr());
    	assertEquals(null, postMsg.extendedHeader());
    	postMsg.applyHasExtendedHdr();
    	assertNotNull(postMsg.extendedHeader());
    	assertEquals(false, postMsg.checkHasMsgKey());
    	assertEquals(null, postMsg.msgKey());
    	postMsg.applyHasMsgKey();
    	assertNotNull(postMsg.msgKey());
    	assertEquals(false, postMsg.checkHasPartNum());
    	assertEquals(0, postMsg.partNum());
    	postMsg.applyHasPartNum();
    	postMsg.partNum(11);
    	assertEquals(11, postMsg.partNum());
    	assertEquals(false, postMsg.checkHasPermData());
    	assertEquals(null, postMsg.permData());
    	postMsg.applyHasPermData();
    	assertNotNull(postMsg.permData());
    	assertEquals(false, postMsg.checkHasPostId());
    	assertEquals(0, postMsg.postId());
    	postMsg.applyHasPostId();
    	postMsg.postId(22);
    	assertEquals(22, postMsg.postId());
    	assertEquals(false, postMsg.checkHasPostUserRights());
    	assertEquals(0, postMsg.postUserRights());
    	postMsg.applyHasPostUserRights();
    	postMsg.postUserRights(33);
    	assertEquals(33, postMsg.postUserRights());
    	assertEquals(false, postMsg.checkHasSeqNum());
    	assertEquals(0, postMsg.seqNum());
    	postMsg.applyHasSeqNum();
    	postMsg.seqNum(44);
    	assertEquals(44, postMsg.seqNum());
    	assertEquals(false, postMsg.checkPostComplete());
    	postMsg.applyPostComplete();
    	assertEquals(true, postMsg.checkPostComplete());
    	assertEquals(false, postMsg.checkAck());
    	postMsg.applyAck();
    	assertEquals(true, postMsg.checkAck());
    	postMsg.flags(0);
    	assertEquals(false, postMsg.checkHasExtendedHdr());
    	assertEquals(null, postMsg.extendedHeader());
    	assertEquals(false, postMsg.checkHasMsgKey());
    	assertEquals(null, postMsg.msgKey());
    	assertEquals(false, postMsg.checkHasPartNum());
    	assertEquals(0, postMsg.partNum());
    	assertEquals(false, postMsg.checkHasPermData());
    	assertEquals(null, postMsg.permData());
    	assertEquals(false, postMsg.checkHasPostId());
    	assertEquals(0, postMsg.postId());
    	assertEquals(false, postMsg.checkHasPostUserRights());
    	assertEquals(0, postMsg.postUserRights());
    	assertEquals(false, postMsg.checkHasSeqNum());
    	assertEquals(0, postMsg.seqNum());
    	assertEquals(false, postMsg.checkPostComplete());
    	assertEquals(false, postMsg.checkAck());

    	// RefreshMsg
    	RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
    	refreshMsg.msgClass(MsgClasses.REFRESH);
    	assertEquals(false, refreshMsg.checkHasExtendedHdr());
    	assertEquals(null, refreshMsg.extendedHeader());
    	refreshMsg.applyHasExtendedHdr();
    	assertNotNull(refreshMsg.extendedHeader());
    	assertEquals(false, refreshMsg.checkHasMsgKey());
    	assertEquals(null, refreshMsg.msgKey());
    	refreshMsg.applyHasMsgKey();
    	assertNotNull(refreshMsg.msgKey());
    	assertEquals(false, refreshMsg.checkHasPartNum());
    	assertEquals(0, refreshMsg.partNum());
    	refreshMsg.applyHasPartNum();
    	refreshMsg.partNum(11);
    	assertEquals(11, refreshMsg.partNum());
    	assertEquals(false, refreshMsg.checkHasPermData());
    	assertEquals(null, refreshMsg.permData());
    	refreshMsg.applyHasPermData();
    	assertNotNull(refreshMsg.permData());
    	assertEquals(false, refreshMsg.checkHasPostUserInfo());
    	assertEquals(null, refreshMsg.postUserInfo());
    	refreshMsg.applyHasPostUserInfo();
    	assertNotNull(refreshMsg.postUserInfo());
    	assertEquals(false, refreshMsg.checkHasQos());
    	assertEquals(null, refreshMsg.qos());
    	refreshMsg.applyHasQos();
    	assertNotNull(refreshMsg.qos());
    	assertEquals(false, refreshMsg.checkHasSeqNum());
    	assertEquals(0, refreshMsg.seqNum());
    	refreshMsg.applyHasSeqNum();
    	refreshMsg.seqNum(22);
    	assertEquals(22, refreshMsg.seqNum());
    	assertEquals(false, refreshMsg.checkPrivateStream());
    	refreshMsg.applyPrivateStream();
    	assertEquals(true, refreshMsg.checkPrivateStream());
    	assertEquals(false, refreshMsg.checkRefreshComplete());
    	refreshMsg.applyRefreshComplete();
    	assertEquals(true, refreshMsg.checkRefreshComplete());
    	assertEquals(false, refreshMsg.checkSolicited());
    	refreshMsg.applySolicited();
    	assertEquals(true, refreshMsg.checkSolicited());
    	assertEquals(false, refreshMsg.checkClearCache());
    	refreshMsg.applyClearCache();
    	assertEquals(true, refreshMsg.checkClearCache());
    	assertEquals(false, refreshMsg.checkDoNotCache());
    	refreshMsg.applyDoNotCache();
    	assertEquals(true, refreshMsg.checkDoNotCache());
    	refreshMsg.flags(0);
    	assertEquals(false, refreshMsg.checkHasExtendedHdr());
    	assertEquals(null, refreshMsg.extendedHeader());
    	assertEquals(false, refreshMsg.checkHasMsgKey());
    	assertEquals(null, refreshMsg.msgKey());
    	assertEquals(false, refreshMsg.checkHasPartNum());
    	assertEquals(0, refreshMsg.partNum());
    	assertEquals(false, refreshMsg.checkHasPermData());
    	assertEquals(null, refreshMsg.permData());
    	assertEquals(false, refreshMsg.checkHasPostUserInfo());
    	assertEquals(null, refreshMsg.postUserInfo());
    	assertEquals(false, refreshMsg.checkHasQos());
    	assertEquals(null, refreshMsg.qos());
    	assertEquals(false, refreshMsg.checkHasSeqNum());
    	assertEquals(0, refreshMsg.seqNum());
    	assertEquals(false, refreshMsg.checkPrivateStream());
    	assertEquals(false, refreshMsg.checkRefreshComplete());
    	assertEquals(false, refreshMsg.checkSolicited());
    	assertEquals(false, refreshMsg.checkClearCache());
    	assertEquals(false, refreshMsg.checkDoNotCache());

    	// RequestMsg
    	RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
    	requestMsg.msgClass(MsgClasses.REQUEST);
    	assertEquals(false, requestMsg.checkHasExtendedHdr());
    	assertEquals(null, requestMsg.extendedHeader());
    	requestMsg.applyHasExtendedHdr();
    	assertNotNull(requestMsg.extendedHeader());
    	assertEquals(false, requestMsg.checkHasPriority());
    	assertEquals(null, requestMsg.priority());
    	requestMsg.applyHasPriority();
    	assertNotNull(requestMsg.priority());
    	assertEquals(false, requestMsg.checkHasQos());
    	assertEquals(null, requestMsg.qos());
    	requestMsg.applyHasQos();
    	assertNotNull(requestMsg.qos());
    	assertEquals(false, requestMsg.checkHasWorstQos());
    	assertEquals(null, requestMsg.worstQos());
    	requestMsg.applyHasWorstQos();
    	assertNotNull(requestMsg.worstQos());
    	assertEquals(false, requestMsg.checkConfInfoInUpdates());
    	requestMsg.applyConfInfoInUpdates();
    	assertEquals(true, requestMsg.checkConfInfoInUpdates());
    	assertEquals(false, requestMsg.checkHasBatch());
    	requestMsg.applyHasBatch();
    	assertEquals(true, requestMsg.checkHasBatch());
    	assertEquals(false, requestMsg.checkHasView());
    	requestMsg.applyHasView();
    	assertEquals(true, requestMsg.checkHasView());
    	assertEquals(false, requestMsg.checkMsgKeyInUpdates());
    	requestMsg.applyMsgKeyInUpdates();
    	assertEquals(true, requestMsg.checkMsgKeyInUpdates());
    	assertEquals(false, requestMsg.checkNoRefresh());
    	requestMsg.applyNoRefresh();
    	assertEquals(true, requestMsg.checkNoRefresh());
    	assertEquals(false, requestMsg.checkPause());
    	requestMsg.applyPause();
    	assertEquals(true, requestMsg.checkPause());
    	assertEquals(false, requestMsg.checkPrivateStream());
    	requestMsg.applyPrivateStream();
    	assertEquals(true, requestMsg.checkPrivateStream());
    	assertEquals(false, requestMsg.checkStreaming());
    	requestMsg.applyStreaming();
    	assertEquals(true, requestMsg.checkStreaming());
    	requestMsg.flags(0);
    	assertEquals(false, requestMsg.checkHasExtendedHdr());
    	assertEquals(null, requestMsg.extendedHeader());
    	assertEquals(false, requestMsg.checkHasPriority());
    	assertEquals(null, requestMsg.priority());
    	assertEquals(false, requestMsg.checkHasQos());
    	assertEquals(null, requestMsg.qos());
    	assertEquals(false, requestMsg.checkHasWorstQos());
    	assertEquals(null, requestMsg.worstQos());
    	assertEquals(false, requestMsg.checkConfInfoInUpdates());
    	assertEquals(false, requestMsg.checkHasBatch());
    	assertEquals(false, requestMsg.checkHasView());
    	assertEquals(false, requestMsg.checkMsgKeyInUpdates());
    	assertEquals(false, requestMsg.checkNoRefresh());
    	assertEquals(false, requestMsg.checkPause());
    	assertEquals(false, requestMsg.checkPrivateStream());
    	assertEquals(false, requestMsg.checkStreaming());
    	
    	// StatusMsg
    	StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
    	statusMsg.msgClass(MsgClasses.STATUS);
    	assertEquals(false, statusMsg.checkHasExtendedHdr());
    	assertEquals(null, statusMsg.extendedHeader());
    	statusMsg.applyHasExtendedHdr();
    	assertNotNull(statusMsg.extendedHeader());
    	assertEquals(false, statusMsg.checkHasGroupId());
    	assertEquals(null, statusMsg.groupId());
    	statusMsg.applyHasGroupId();
    	assertNotNull(statusMsg.groupId());
    	assertEquals(false, statusMsg.checkHasMsgKey());
    	assertEquals(null, statusMsg.msgKey());
    	statusMsg.applyHasMsgKey();
    	assertNotNull(statusMsg.msgKey());
    	assertEquals(false, statusMsg.checkHasPermData());
    	assertEquals(null, statusMsg.permData());
    	statusMsg.applyHasPermData();
    	assertNotNull(statusMsg.permData());
    	assertEquals(false, statusMsg.checkHasPostUserInfo());
    	assertEquals(null, statusMsg.postUserInfo());
    	statusMsg.applyHasPostUserInfo();
    	assertNotNull(statusMsg.postUserInfo());
    	assertEquals(false, statusMsg.checkHasState());
    	assertEquals(null, statusMsg.state());
    	statusMsg.applyHasState();
    	assertNotNull(statusMsg.state());
    	assertEquals(false, statusMsg.checkPrivateStream());
    	statusMsg.applyPrivateStream();
    	assertEquals(true, statusMsg.checkPrivateStream());
    	assertEquals(false, statusMsg.checkClearCache());
    	statusMsg.applyClearCache();
    	assertEquals(true, statusMsg.checkClearCache());
    	statusMsg.flags(0);
    	assertEquals(false, statusMsg.checkHasExtendedHdr());
    	assertEquals(null, statusMsg.extendedHeader());
    	assertEquals(false, statusMsg.checkHasGroupId());
    	assertEquals(null, statusMsg.groupId());
    	assertEquals(false, statusMsg.checkHasMsgKey());
    	assertEquals(null, statusMsg.msgKey());
    	assertEquals(false, statusMsg.checkHasPermData());
    	assertEquals(null, statusMsg.permData());
    	assertEquals(false, statusMsg.checkHasPostUserInfo());
    	assertEquals(null, statusMsg.postUserInfo());
    	assertEquals(false, statusMsg.checkHasState());
    	assertEquals(null, statusMsg.state());
    	assertEquals(false, statusMsg.checkPrivateStream());
    	assertEquals(false, statusMsg.checkClearCache());
    	
    	// UpdateMsg
    	UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();    	
    	updateMsg.msgClass(MsgClasses.UPDATE);
    	assertEquals(false, updateMsg.checkHasExtendedHdr());
    	assertEquals(null, updateMsg.extendedHeader());
    	updateMsg.applyHasExtendedHdr();
    	assertNotNull(updateMsg.extendedHeader());
    	assertEquals(false, updateMsg.checkHasMsgKey());
    	assertEquals(null, updateMsg.msgKey());
    	updateMsg.applyHasMsgKey();
    	assertNotNull(updateMsg.msgKey());
    	assertEquals(false, updateMsg.checkHasPermData());
    	assertEquals(null, updateMsg.permData());
    	updateMsg.applyHasPermData();
    	assertNotNull(updateMsg.permData());
    	assertEquals(false, updateMsg.checkHasPostUserInfo());
    	assertEquals(null, updateMsg.postUserInfo());
    	updateMsg.applyHasPostUserInfo();
    	assertNotNull(updateMsg.postUserInfo());
    	assertEquals(false, updateMsg.checkHasConfInfo());
    	assertEquals(0, updateMsg.conflationCount());
    	assertEquals(0, updateMsg.conflationTime());
    	updateMsg.applyHasConfInfo();
    	updateMsg.conflationCount(11);
    	assertEquals(11, updateMsg.conflationCount());
    	updateMsg.conflationTime(22);
    	assertEquals(22, updateMsg.conflationTime());
    	assertEquals(false, updateMsg.checkHasSeqNum());
    	assertEquals(0, updateMsg.seqNum());
    	updateMsg.applyHasSeqNum();
    	updateMsg.seqNum(33);
    	assertEquals(33, updateMsg.seqNum());
    	assertEquals(false, updateMsg.checkDiscardable());
    	updateMsg.applyDiscardable();
    	assertEquals(true, updateMsg.checkDiscardable());
    	assertEquals(false, updateMsg.checkDoNotCache());
    	updateMsg.applyDoNotCache();
    	assertEquals(true, updateMsg.checkDoNotCache());
    	assertEquals(false, updateMsg.checkDoNotConflate());
    	updateMsg.applyDoNotConflate();
    	assertEquals(true, updateMsg.checkDoNotConflate());
    	assertEquals(false, updateMsg.checkDoNotRipple());
    	updateMsg.applyDoNotRipple();
    	assertEquals(true, updateMsg.checkDoNotRipple());
    	updateMsg.flags(0);
    	assertEquals(false, updateMsg.checkHasExtendedHdr());
    	assertEquals(null, updateMsg.extendedHeader());
    	assertEquals(false, updateMsg.checkHasMsgKey());
    	assertEquals(null, updateMsg.msgKey());
    	assertEquals(false, updateMsg.checkHasPermData());
    	assertEquals(null, updateMsg.permData());
    	assertEquals(false, updateMsg.checkHasPostUserInfo());
    	assertEquals(null, updateMsg.postUserInfo());
    	assertEquals(false, updateMsg.checkHasConfInfo());
    	assertEquals(false, updateMsg.checkHasSeqNum());
    	assertEquals(false, updateMsg.checkDiscardable());
    	assertEquals(false, updateMsg.checkDoNotCache());
    	assertEquals(false, updateMsg.checkDoNotConflate());
    	assertEquals(false, updateMsg.checkDoNotRipple());
    }
    
    @Test
    public void updateMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, updateMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, updateMasksCommon, updateMasksCommon.length, false);
        updateMsgTest(1);
    }

    @Test
    public void updateMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, updateMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, updateMasksAll, updateMasksAll.length, false);
        updateMsgTest(1);
    }

    private void refreshMsgTest(int repeat)
    {
        int extraAction;

        int responseMask;

        RefreshMsg respMsg = (RefreshMsg)msg;

        int msgKeyMask;
        int decodeMsgKeyMask;

        int domainType = DomainTypes.LOGIN;

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
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */

                        respMsg.clear();

                        respMsg.msgClass(MsgClasses.REFRESH);
                        respMsg.streamId(streamId);
                        respMsg.domainType(domainType);

                        respMsg.flags(responseMask);

                        respMsg.state().streamState(StreamStates.OPEN);
                        respMsg.state().dataState(DataStates.SUSPECT);
                        respMsg.state().code(StateCodes.NO_RESOURCES);
                        respMsg.state().text().data(stateText);

                        respMsg.groupId().data(ByteBuffer.wrap(groupId));

                        /* Add msgKey */
                        if ((responseMask & RefreshMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            respMsg.msgKey().flags(msgKeyMask);
                            encodeMsgKey(respMsg.msgKey());
                        }

                        /* Add Permission Info */
                        if ((responseMask & RefreshMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            respMsg.permData().data(permissionData);
                        }

                        /* Add Item Sequence Number */
                        if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(responseMask + 0xFFFF) : (int)responseMask;
                            respMsg.seqNum(seqNum);
                        }

                        if ((responseMask & RefreshMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            /* Set up user information */
                            respMsg.postUserInfo().userAddr("204.170.85.29");
                            respMsg.postUserInfo().userId(0xCCAA551DL);
                        }

                        if ((responseMask & RefreshMsgFlags.HAS_PART_NUM) > 0)
                        {
                            respMsg.partNum(0x100f);
                        }

                        /* Add QoS */
                        if ((responseMask & RefreshMsgFlags.HAS_QOS) > 0)
                        {
                            respMsg.qos().rate(QosRates.TIME_CONFLATED);
                            respMsg.qos().rateInfo(65535);
                            respMsg.qos().timeliness(QosTimeliness.DELAYED);
                            respMsg.qos().timeInfo(65534);
                        }

                        /* Add Extended Header */
                        if ((responseMask & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            respMsg.extendedHeader().data(extendedHeader);
                        }

                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.encodedDataBody(encDataBuf);
                            respMsg.containerType(g_dataFormat);

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                                msg.encodedDataBody(encDataBuf);
                            }
                            else
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                                msg.encodedDataBody(encDataBuf);
                            }

                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            respMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, respMsg.extendedHeader(), respMsg.flags(), RefreshMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            respMsg.containerType(DataTypes.NO_DATA);
                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }

                        /* Decode msg */
                        decodeMsgKeyMask = fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
						setupEncodeIterator();

                        if (g_changeData)
                        {
                            Buffer newGroupId = CodecFactory.createBuffer();
                            newGroupId.data(ByteBuffer.wrap(groupId));
                            byte[] groupIdData = new byte[32];
                            Buffer extractGroupId = CodecFactory.createBuffer();
                            extractGroupId.data(ByteBuffer.wrap(groupIdData));

                            /* extract the msgClass */
                            assertEquals("ExtractMsgClass", MsgClasses.REFRESH, decIter.extractMsgClass());
                            setupDecodeIterator();

                            /* extract the domainType */
                            assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                            setupDecodeIterator();

                            /* extract streamId */
                            assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* replace the streamId */
                            assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
                            setupEncodeIterator();

                            /* extract the streamId */
                            assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* extract the seqNum */
                            if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                            	assertEquals("ExtractSeqNum", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum", seqNum, decIter.extractSeqNum());
                            setupDecodeIterator();

                            /* replace the seqNum */
                            assertEquals("ReplaceSeqNum",
                                         ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE),
                                         encIter.replaceSeqNum(seqNum + 1));

                            /* extract the new seqNum */
                            if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                            	assertEquals("ExtractSeqNum2", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum2", seqNum + 1, decIter.extractSeqNum());
                            setupDecodeIterator();

                            /* extract the groupId */
                            assertEquals("ExtractGroupId", CodecReturnCodes.SUCCESS, decIter.extractGroupId(extractGroupId));
                            assertEquals("extractGroupId", groupIdLen, extractGroupId.length());
                            assertEquals("extractGroupId", true, isMemEqual(extractGroupId, newGroupId, newGroupId.length()));
                            setupDecodeIterator();

                            /* replace the groupId */
                            groupId[0] = '4';
                            assertEquals("ReplaceGroupId", CodecReturnCodes.SUCCESS, encIter.replaceGroupId(newGroupId));

                            /* extract the new groupId */
                            assertEquals("ExtractGroupId2", CodecReturnCodes.SUCCESS, decIter.extractGroupId(extractGroupId));
                            assertEquals("extractGroupId2", newGroupId.length(), extractGroupId.length());
                            assertEquals("extractGroupId2", true, isMemEqual(extractGroupId, newGroupId, newGroupId.length()));
                            setupDecodeIterator();

                            /* replace the streamState */
                            assertEquals("ReplaceStreamState", CodecReturnCodes.SUCCESS, encIter.replaceStreamState(StreamStates.REDIRECTED));

                            /* replace the dataState */
                            assertEquals("ReplaceDataState", CodecReturnCodes.SUCCESS, encIter.replaceDataState(DataStates.OK));

                            /* replace the stateCode */
                            assertEquals("ReplaceStateCode", CodecReturnCodes.SUCCESS, encIter.replaceStateCode(StateCodes.INVALID_VIEW));
                        }

                        if (g_setRefreshFlags)
                        {
                            assertEquals("SetRefreshComplete", CodecReturnCodes.SUCCESS, encIter.setRefreshCompleteFlag());
                            assertEquals("SetSolicited", CodecReturnCodes.SUCCESS, encIter.setSolicitedFlag());

                            for (int maskToSet : responseSetUnSetMasks)
                            {
                                responseMask |= maskToSet;
                            }
                        }

                        if (g_unsetRefreshFlags)
                        {
                            assertEquals("UnSetRefreshComplete", CodecReturnCodes.SUCCESS, encIter.unsetRefreshCompleteFlag());
                            assertEquals("UnSetSolicited", CodecReturnCodes.SUCCESS, encIter.unsetSolicitedFlag());
                            setupEncodeIterator();

                            for (int maskToUnSet : responseSetUnSetMasks)
                            {
                                responseMask &= ~maskToUnSet;
                            }
                        }

                        assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
                        assertEquals("Correct MsgBase", MsgClasses.REFRESH, msg.msgClass());
                        assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                        assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                        assertEquals("Correct MsgBase", (g_changeData ? domainType : domainType), msg.domainType());

                        assertEquals("Correct Mask", responseMask, respMsg.flags());

                        /* Check msgKey */
                        if ((responseMask & RefreshMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            assertEquals("Correct Msg Key Mask", decodeMsgKeyMask, respMsg.msgKey().flags());
                            decodeMsgKey(decIter, respMsg.msgKey());
                        }

                        /* Check State */
                        assertEquals("Correct Stream State",
                                     (g_changeData ? StreamStates.REDIRECTED : StreamStates.OPEN),
                                     respMsg.state().streamState());
                        assertEquals("Correct Item State", (g_changeData ? DataStates.OK : DataStates.SUSPECT), respMsg.state().dataState());
                        assertEquals("Correct Item State", (g_changeData ? StateCodes.INVALID_VIEW : StateCodes.NO_RESOURCES), respMsg.state().code());
                        assertEquals("Correct Item State", stateTextLen, respMsg.state().text().length());
                        assertEquals("Correct Item State", true, isMemEqual(respMsg.state().text(), stateText, stateTextLen));

                        assertEquals("Correct Group Id", groupIdLen, respMsg.groupId().length());
                        Buffer gid = CodecFactory.createBuffer();
                        gid.data(ByteBuffer.wrap(groupId));
                        assertEquals("Correct Group Id", true, isMemEqual(respMsg.groupId(), gid, groupIdLen));

                        /* Check Permission Info */
                        if ((responseMask & RefreshMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            assertEquals("Correct Permission Info", permissionDataLen, respMsg.permData().length());
                            assertEquals("Correct Permission Info", true, isMemEqual(respMsg.permData(), permissionData, permissionDataLen));
                        }

                        /* Check Item Sequence Number */
                        if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            assertEquals("Correct Item Sequence Number", (g_changeData ? seqNum + 1 : seqNum), respMsg.seqNum());
                        }

                        if ((responseMask & RefreshMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            assertEquals("Correct Post User Addr", 0xCCAA551DL, respMsg.postUserInfo().userAddr());
                            assertEquals("Correct Post User ID", 0xCCAA551DL, respMsg.postUserInfo().userId());
                        }

                        if ((responseMask & RefreshMsgFlags.HAS_PART_NUM) > 0)
                        {
                            assertEquals("Correct PartNum", 0x100f, respMsg.partNum());
                        }

                        /* Check QoS */
                        if ((responseMask & RefreshMsgFlags.HAS_QOS) > 0)
                        {
                            assertEquals("Correct QoS", QosRates.TIME_CONFLATED, respMsg.qos().rate());
                            assertEquals("Correct QoS", 65535, respMsg.qos().rateInfo());
                            assertEquals("Correct QoS", QosTimeliness.DELAYED, respMsg.qos().timeliness());
                            assertEquals("Correct QoS", 65534, respMsg.qos().timeInfo());
                        }

                        /* Check Extended Header */
                        if ((responseMask & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            assertEquals("Correct Extended Header", extendedHeaderLen, respMsg.extendedHeader().length());
                            assertEquals("Correct Extended Header", true, isMemEqual(respMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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

    @Test
    public void refreshMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, responseMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, responseMasksCommon, responseMasksCommon.length, false);
        refreshMsgTest(repeatCount);
    }

    @Test
    public void refreshMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, responseMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, responseMasksAll, responseMasksAll.length, false);
        refreshMsgTest(1);
    }

    @Test
    public void refreshMessageSetMsgFlagsTest()
    {
        testSetupFull();
        masks = responseSetUnSetMasksTests;
        masksSize = masks.length;

        g_setRefreshFlags = true;
        refreshMsgTest(1);
        g_setRefreshFlags = false;
    }

    @Test
    public void refreshMessageUnSetMsgFlagsTest()
    {
        testSetupFull();
        masks = responseSetUnSetMasksTests;
        masksSize = masks.length;
        g_unsetRefreshFlags = true;
        refreshMsgTest(1);
        g_unsetRefreshFlags = false;
    }

    private void requestMsgTest(int repeat)
    {
        int extraAction;

        int requestMask;
        RequestMsg reqMsg = (RequestMsg)msg;

        int msgKeyMask;
        int decodeMsgKeyMask;

        int domainType = DomainTypes.DICTIONARY;

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
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */
                        reqMsg.clear();

                        reqMsg.msgClass(MsgClasses.REQUEST);
                        reqMsg.streamId(streamId);
                        reqMsg.domainType(domainType);

                        reqMsg.flags(requestMask);

                        /* Add msg key */
                        reqMsg.msgKey().flags(msgKeyMask);
                        encodeMsgKey(reqMsg.msgKey());

                        /* Add priority info */
                        if ((requestMask & RequestMsgFlags.HAS_PRIORITY) > 0)
                        {
                            reqMsg.priority().priorityClass(3);
                            reqMsg.priority().count(4);
                        }

                        /* Add best QoS */
                        if ((requestMask & RequestMsgFlags.HAS_QOS) > 0)
                        {
                            reqMsg.qos().rate(QosRates.TIME_CONFLATED);
                            reqMsg.qos().rateInfo(65535);
                            reqMsg.qos().timeliness(QosTimeliness.DELAYED);
                            reqMsg.qos().timeInfo(65534);
                        }

                        /* Add worst QoS */
                        if ((requestMask & RequestMsgFlags.HAS_WORST_QOS) > 0)
                        {
                            reqMsg.worstQos().rate(QosRates.TIME_CONFLATED);
                            reqMsg.worstQos().rateInfo(65533);
                            reqMsg.worstQos().timeliness(QosTimeliness.DELAYED);
                            reqMsg.worstQos().timeInfo(65532);
                        }

                        /* Add extended header */
                        if ((requestMask & RequestMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            reqMsg.extendedHeader().data(extendedHeader);
                        }

                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.encodedDataBody(encDataBuf);
                            reqMsg.containerType(g_dataFormat);

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                                msg.encodedDataBody(encDataBuf);
                            }
                            else
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                                msg.encodedDataBody(encDataBuf);
                            }

                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            reqMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, reqMsg.extendedHeader(), reqMsg.flags(), RequestMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            reqMsg.containerType(DataTypes.NO_DATA);
                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }

                        /* Decode msg */
                        decodeMsgKeyMask = fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
						setupEncodeIterator();

                        if (g_changeData)
                        {
                        	setupEncodeIterator();
                        	
                            /* extract the msgClass */
                            assertEquals("ExtractMsgClass", MsgClasses.REQUEST, decIter.extractMsgClass());
                            setupDecodeIterator();

                            /* extract the domainType */
                            assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                            setupDecodeIterator();

                            /* extract streamId */
                            assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* replace the streamId */
                            assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
							setupEncodeIterator();

                            /* extract the streamId */
                            assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                            setupDecodeIterator();
                        }

                        if (g_setRequestFlags)
                        {
                            assertEquals("SetNoRefresh", CodecReturnCodes.SUCCESS, encIter.setNoRefreshFlag());
                            assertEquals("SetStreaming", CodecReturnCodes.SUCCESS, encIter.setStreamingFlag());
                            assertEquals("ConfInfoInUpdate", CodecReturnCodes.SUCCESS, encIter.setConfInfoInUpdatesFlag());
                            assertEquals("MsgKeyInUpdate", CodecReturnCodes.SUCCESS, encIter.setMsgKeyInUpdatesFlag());
                            setupEncodeIterator();

                            for (int maskToSet : requestSetUnSetMasks)
                            {
                                requestMask |= maskToSet;
                            }
                        }

                        if (g_unsetRequestFlags)
                        {
                            assertEquals("UnSetNoRefresh", CodecReturnCodes.SUCCESS, encIter.unsetNoRefreshFlag());
                            assertEquals("UnSetStreaming", CodecReturnCodes.SUCCESS, encIter.unsetStreamingFlag());
                            assertEquals("UnSetConfInfoInUpdate", CodecReturnCodes.SUCCESS, encIter.unsetConfInfoInUpdatesFlag());
                            assertEquals("UnSetMsgKeyInUpdate", CodecReturnCodes.SUCCESS, encIter.unsetMsgKeyInUpdatesFlag());
                            setupEncodeIterator();

                            for (int maskToUnSet : requestSetUnSetMasks)
                            {
                                requestMask &= ~maskToUnSet;
                            }
                        }

                        assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));

                        /* Check mask and msgBase */
                        assertEquals("Correct MsgBase", MsgClasses.REQUEST, msg.msgClass());
                        assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                        assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                        assertEquals("Correct MsgBase", (g_changeData ? domainType : domainType), msg.domainType());

                        assertEquals("Correct Mask", requestMask, reqMsg.flags());

                        /* Check msg key */
                        assertEquals("Correct Msg Key Mask", decodeMsgKeyMask, reqMsg.msgKey().flags());
                        decodeMsgKey(decIter, reqMsg.msgKey());

                        /* Check priority info */
                        if ((requestMask & RequestMsgFlags.HAS_PRIORITY) > 0)
                        {
                            assertEquals("Correct Priority", 3, reqMsg.priority().priorityClass());
                            assertEquals("Correct Priority", 4, reqMsg.priority().count());
                        }

                        /* Check best QoS */
                        if ((requestMask & RequestMsgFlags.HAS_QOS) > 0)
                        {
                            assertEquals("Correct Best QoS", QosRates.TIME_CONFLATED, reqMsg.qos().rate());
                            assertEquals("Correct Best QoS", 65535, reqMsg.qos().rateInfo());
                            assertEquals("Correct Best QoS", QosTimeliness.DELAYED, reqMsg.qos().timeliness());
                            assertEquals("Correct Best QoS", 65534, reqMsg.qos().timeInfo());
                        }

                        /* Check worst QoS */
                        if ((requestMask & RequestMsgFlags.HAS_WORST_QOS) > 0)
                        {
                            assertEquals("Correct Worst QoS", QosRates.TIME_CONFLATED, reqMsg.worstQos().rate());
                            assertEquals("Correct Worst QoS", 65533, reqMsg.worstQos().rateInfo());
                            assertEquals("Correct Worst QoS", QosTimeliness.DELAYED, reqMsg.worstQos().timeliness());
                            assertEquals("Correct Worst QoS", 65532, reqMsg.worstQos().timeInfo());
                        }

                        /* Check extended header */
                        if ((requestMask & RequestMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            assertEquals("Correct Extended Header", extendedHeaderLen, reqMsg.extendedHeader().length());
                            assertEquals("Correct Extended Header", true, isMemEqual(reqMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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

    @Test
    public void requestMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, requestMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, requestMasksCommon, requestMasksCommon.length, false);
        requestMsgTest(repeatCount);
    }

    @Test
    public void requestMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, requestMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, requestMasksAll, requestMasksAll.length, false);
        requestMsgTest(1);
    }

    @Test
    public void requestMessageFullTestSetFlags()
    {
        testSetupFull();
        masks = requestSetUnSetMasksTests;
        masksSize = masks.length;
        g_setRequestFlags = true;
        requestMsgTest(1);
        g_setRequestFlags = false;
    }

    @Test
    public void requestMessageFullTestUnSetFlags()
    {
        testSetupFull();
        masks = requestSetUnSetMasksTests;
        masksSize = masks.length;
        g_unsetRequestFlags = true;
        requestMsgTest(1);
        g_unsetRequestFlags = false;
    }

    private void statusMsgTest(int repeat)
    {
        int extraAction;

        int statusMask;

        StatusMsg statMsg = (StatusMsg)msg;

        int msgKeyMask;
        int decodeMsgKeyMask;

        int domainType = DomainTypes.MARKET_PRICE;

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
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */
                        statMsg.clear();

                        statMsg.msgClass(MsgClasses.STATUS);
                        statMsg.streamId(streamId);
                        statMsg.domainType(domainType);

                        statMsg.flags(statusMask);

                        /* Add Item State */
                        if ((statusMask & StatusMsgFlags.HAS_STATE) > 0)
                        {
                            statMsg.state().streamState(StreamStates.OPEN);
                            statMsg.state().dataState(DataStates.SUSPECT);
                            statMsg.state().code(StateCodes.NO_RESOURCES);
                            statMsg.state().text().data(stateText);
                        }

                        /* Add Group ID */
                        if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                        {
                            statMsg.groupId().data(ByteBuffer.wrap(groupId));
                        }

                        /* Add msgKey */
                        if ((statusMask & StatusMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            statMsg.msgKey().flags(msgKeyMask);
                            encodeMsgKey(statMsg.msgKey());
                        }

                        /* Add Permission Info */
                        if ((statusMask & StatusMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            statMsg.permData().data(permissionData);
                        }

                        if ((statusMask & StatusMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            /* Set up user information */
                            statMsg.postUserInfo().userAddr("204.170.85.29");
                            statMsg.postUserInfo().userId(0xCCAA551DL);
                        }

                        /* Add Extended Header */
                        if ((statusMask & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            statMsg.extendedHeader().data(extendedHeader);
                        }

                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.encodedDataBody(encDataBuf);
                            statMsg.containerType(g_dataFormat);

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                                msg.encodedDataBody(encDataBuf);
                            }
                            else
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                                msg.encodedDataBody(encDataBuf);
                            }

                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            statMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, statMsg.extendedHeader(), statMsg.flags(), StatusMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            statMsg.containerType(DataTypes.NO_DATA);
                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }

                        /* Decode msg */
                        decodeMsgKeyMask = fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
						setupEncodeIterator();

                        if (g_changeData)
                        {
                            Buffer newGroupId = CodecFactory.createBuffer();
                            newGroupId.data(ByteBuffer.wrap(groupId));
                            byte[] groupIdData = new byte[32];
                            Buffer extractGroupId = CodecFactory.createBuffer();
                            extractGroupId.data(ByteBuffer.wrap(groupIdData));

                            /* extract the msgClass */
                            assertEquals("ExtractMsgClass", MsgClasses.STATUS, decIter.extractMsgClass());
                            setupDecodeIterator();

                            /* extract the domainType */
                            assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                            setupDecodeIterator();

                            /* extract streamId */
                            assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* replace the streamId */
                            assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
							setupEncodeIterator();

                            /* extract the streamId */
                            assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* extract the groupId */
                            assertEquals("ExtractGroupId", ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE), decIter.extractGroupId(extractGroupId));
                            if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                            {
                                assertEquals("extractGroupId", groupIdLen, extractGroupId.length());
                                assertEquals("extractGroupId", true, isMemEqual(extractGroupId, newGroupId, newGroupId.length()));
                            }
                            setupDecodeIterator();

                            /* replace the groupId */
                            groupId[0] = '4';
                            assertEquals("ReplaceGroupId", ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE), encIter.replaceGroupId(newGroupId));

                            /* extract the new groupId */
                            assertEquals("ExtractGroupId2", ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE), decIter.extractGroupId(extractGroupId));
                            if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                            {
                                assertEquals("extractGroupId2", newGroupId.length(), extractGroupId.length());
                                assertEquals("extractGroupId2", true, isMemEqual(extractGroupId, newGroupId, newGroupId.length()));
                            }
                            setupDecodeIterator();

                            /* replace the streamState */
                            assertEquals("ReplaceStreamState", ((statusMask & StatusMsgFlags.HAS_STATE) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE), encIter.replaceStreamState(StreamStates.REDIRECTED));

                            /* replace the dataState */
                            assertEquals("ReplaceDataState", ((statusMask & StatusMsgFlags.HAS_STATE) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE), encIter.replaceDataState(DataStates.OK));

                            /* replace the stateCode */
                            assertEquals("ReplaceStateCode", ((statusMask & StatusMsgFlags.HAS_STATE) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE), encIter.replaceStateCode(StateCodes.INVALID_VIEW));
                            setupEncodeIterator();
                        }

                        assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
                        assertEquals("Correct MsgBase", MsgClasses.STATUS, msg.msgClass());
                        assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                        assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                        assertEquals("Correct MsgBase", (g_changeData ? domainType: domainType), msg.domainType());

                        assertEquals("Correct Mask", statusMask, statMsg.flags());

                        /* Check Item State */
                        if ((statusMask & StatusMsgFlags.HAS_STATE) > 0)
                        {
                            assertEquals("Correct Stream State",
                                         (g_changeData ? StreamStates.REDIRECTED : StreamStates.OPEN),
                                         statMsg.state().streamState());
                            assertEquals("Correct Item State", (g_changeData ? DataStates.OK : DataStates.SUSPECT), statMsg.state().dataState());
                            assertEquals("Correct Item State", (g_changeData ? StateCodes.INVALID_VIEW : StateCodes.NO_RESOURCES), statMsg.state().code());
                            assertEquals("Correct Item State", stateTextLen, statMsg.state().text().length());
                            assertEquals("Correct Item State", true, isMemEqual(statMsg.state().text(), stateText, stateTextLen));
                        }

                        if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                        {
                            assertEquals("Correct Group Id", groupIdLen, statMsg.groupId().length());
                            Buffer gid = CodecFactory.createBuffer();
                            gid.data(ByteBuffer.wrap(groupId));
                            assertEquals("Correct Group Id", true, isMemEqual(statMsg.groupId(), gid, groupIdLen));
                        }

                        /* Check msgKey */
                        if ((statusMask & StatusMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            assertEquals("Correct Msg Key Mask", decodeMsgKeyMask, statMsg.msgKey().flags());
                            decodeMsgKey(decIter, statMsg.msgKey());
                        }

                        /* Check Permission Info */
                        if ((statusMask & StatusMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            assertEquals("Correct Permission Info", permissionDataLen, statMsg.permData().length());
                            assertEquals("Correct Permission Info", true, isMemEqual(statMsg.permData(), permissionData, permissionDataLen));
                        }

                        if ((statusMask & StatusMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            assertEquals("Correct Post User Addr", 0xCCAA551DL, statMsg.postUserInfo().userAddr());
                            assertEquals("Correct Post User ID", 0xCCAA551DL, statMsg.postUserInfo().userId());
                        }

                        /* Check Extended Header */
                        if ((statusMask & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            assertEquals("Correct Extended Header", extendedHeaderLen, statMsg.extendedHeader().length());
                            assertEquals("Correct Extended Header", true, isMemEqual(statMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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

    @Test
    public void statusMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, statusMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, statusMasksCommon, statusMasksCommon.length, false);
        statusMsgTest(repeatCount);
    }

    @Test
    public void statusMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, statusMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, statusMasksAll, statusMasksAll.length, false);
        statusMsgTest(1);
    }

    private void closeMsgTest(int repeat)
    {
        int extraAction;

        int closeMask;

        CloseMsg closeMsg = (CloseMsg)msg;

        int domainType = DomainTypes.DICTIONARY;

        for (; repeat > 0; --repeat)
        {
            for (masksIter = 0; masksIter < masksSize; ++masksIter)
            {
                closeMask = masks[masksIter];

                for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                {
                    extraAction = actions[actionsIter];
                    encBuf.data().position(0);
                    setupEncodeIterator();

                    /* Encode msg */

                    closeMsg.clear();
                    closeMsg.msgClass(MsgClasses.CLOSE);
                    closeMsg.streamId(streamId);
                    closeMsg.domainType(domainType);

                    closeMsg.flags(closeMask);

                    /* Add Extended Header */
                    if ((closeMask & CloseMsgFlags.HAS_EXTENDED_HEADER) > 0)
                    {
                        closeMsg.extendedHeader().data(extendedHeader);
                    }

                    /* Add Payload */
                    if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                    {
                        msg.encodedDataBody(encDataBuf);
                        closeMsg.containerType(g_dataFormat);

                        /* Trim payload length if desired */
                        if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                        {
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                            msg.encodedDataBody(encDataBuf);
                        }
                        else
                        {
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                            msg.encodedDataBody(encDataBuf);
                        }

                        assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                    }
                    else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                    {
                        closeMsg.containerType(g_dataFormat);

                        postEncodeMsg(encIter, msg, extraAction, closeMsg.extendedHeader(), closeMsg.flags(), CloseMsgFlags.HAS_EXTENDED_HEADER);

                        encodePayload(encIter);
                        encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                        assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                    }
                    else
                    {
                        msg.encodedDataBody().clear();
                        closeMsg.containerType(DataTypes.NO_DATA);
                        assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                    }

                    setupDecodeIterator();
					setupEncodeIterator();

                    if (g_changeData)
                    {
                        /* extract the msgClass */
                        assertEquals("ExtractMsgClass", MsgClasses.CLOSE, decIter.extractMsgClass());
                        setupDecodeIterator();

                        /* extract the domainType */
                        assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                        setupDecodeIterator();

                        /* extract streamId */
                        assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                        setupDecodeIterator();

                        /* replace the streamId */
                        assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
						setupEncodeIterator();

                        /* extract the streamId */
                        assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                        setupDecodeIterator();
                    }

                    assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
                    assertEquals("Correct MsgBase", MsgClasses.CLOSE, msg.msgClass());
                    assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                    assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                    assertEquals("Correct MsgBase", (g_changeData ? domainType : domainType), msg.domainType());

                    assertEquals("Correct Mask", closeMask, closeMsg.flags());

                    /* Check Extended Header */
                    if ((closeMask & CloseMsgFlags.HAS_EXTENDED_HEADER) > 0)
                    {
                        assertEquals("Correct Extended Header", extendedHeaderLen, closeMsg.extendedHeader().length());
                        assertEquals("Correct Extended Header", true, isMemEqual(closeMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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

    @Test
    public void closeMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, closeMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, closeMasksCommon, closeMasksCommon.length, false);
        closeMsgTest(repeatCount);
    }

    @Test
    public void closeMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, closeMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, closeMasksAll, closeMasksAll.length, false);
        closeMsgTest(1);
    }

    private void ackMsgTest(int repeat)
    {
        int extraAction;

        int ackMask;

        int msgKeyMask;
        int decodeMsgKeyMask;

        AckMsg ackMsg = (AckMsg)msg;

        int domainType = DomainTypes.TRANSACTION;

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
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */
                        ackMsg.ackId(11); // make sure clear method clears ackId
                        ackMsg.clear(); // make sure clear method clears ackId
                        assertEquals(0, ackMsg.ackId()); // make sure clear method clears ackId

                        ackMsg.msgClass(MsgClasses.ACK);
                        ackMsg.streamId(streamId);
                        ackMsg.domainType(domainType);

                        ackMsg.flags(ackMask);
                        ackMsg.ackId(ackMask);

                        /* Add nakCode */
                        if ((ackMask & AckMsgFlags.HAS_NAK_CODE) > 0)
                        {
                            ackMsg.nakCode(ackMask + 1);
                        }

                        if ((ackMask & AckMsgFlags.HAS_TEXT) > 0)
                        {
                            ackMsg.text().data(text);
                        }

                        if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                            ackMsg.seqNum(seqNum);

                        /* Add msgKey */
                        if ((ackMask & AckMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            ackMsg.msgKey().flags(msgKeyMask);
                            encodeMsgKey(ackMsg.msgKey());
                        }

                        /* Add Extended Header */
                        /*
                         * always set it here to cover our pre-encoded
                         * data/EncodeMsg cases.
                         */
                        if ((ackMask & AckMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            ackMsg.extendedHeader().data(extendedHeader);
                        }

                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.encodedDataBody(encDataBuf);
                            ackMsg.containerType(g_dataFormat);

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                                msg.encodedDataBody(encDataBuf);
                            }
                            else
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                                msg.encodedDataBody(encDataBuf);
                            }

                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            ackMsg.containerType(g_dataFormat);

                            /*
                             * if our key opaque and our extended header are
                             * pre-encoded, EncodeMsgInit should tell us to
                             * encode our payload/container
                             */
                            postEncodeMsg(encIter, msg, extraAction, ackMsg.extendedHeader(), ackMsg.flags(), AckMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            ackMsg.containerType(DataTypes.NO_DATA);
                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }

                        /* Decode msg */
                        decodeMsgKeyMask = fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
						setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            assertEquals("ExtractMsgClass", MsgClasses.ACK, decIter.extractMsgClass());
                            setupDecodeIterator();

                            /* extract the domainType */
                            assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                            setupDecodeIterator();

                            /* extract streamId */
                            assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* replace the streamId */
                            assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
                            setupEncodeIterator();

                            /* extract the streamId */
                            assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* extract the seqNum */
                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum", seqNum, decIter.extractSeqNum());

                            setupDecodeIterator();

                            /* replace the seqNum */
                            assertEquals("ReplaceSeqNum",
                                         ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE),
                                         encIter.replaceSeqNum(seqNum + 1));
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum2", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum2", seqNum + 1, decIter.extractSeqNum());
                            setupDecodeIterator();
                        }

                        assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
                        assertEquals("Correct MsgBase", MsgClasses.ACK, msg.msgClass());
                        assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                        assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                        assertEquals("Correct MsgBase", (g_changeData ? domainType : domainType), msg.domainType());

                        assertEquals("Correct Mask", ackMask, ackMsg.flags());

                        /* Check status */
                        if ((ackMask & AckMsgFlags.HAS_TEXT) > 0)
                        {
                            assertEquals("Text is correct", textLen, ackMsg.text().length());
                            assertEquals("Text is correct", true, isMemEqual(ackMsg.text(), text, textLen));
                        }

                        if ((ackMask & AckMsgFlags.HAS_NAK_CODE) > 0)
                        {
                            assertEquals("Correct NakCode", ackMask + 1, ackMsg.nakCode());
                        }

                        if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            assertEquals("SeqNum is correct", (g_changeData ? seqNum + 1 : seqNum), ackMsg.seqNum());
                        }

                        /* Check msgKey */
                        if ((ackMask & AckMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            assertEquals("Correct Msg Key Mask", decodeMsgKeyMask, ackMsg.msgKey().flags());
                            decodeMsgKey(decIter, ackMsg.msgKey());
                        }

                        /* Check Extended Header */
                        if ((ackMask & AckMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            assertEquals("Correct Extended Header", extendedHeaderLen, ackMsg.extendedHeader().length());
                            assertEquals("Correct Extended Header", true, isMemEqual(ackMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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

    @Test
    public void ackMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, ackMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, ackMasksCommon, ackMasksCommon.length, false);
        ackMsgTest(repeatCount);
    }

    @Test
    public void ackMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, ackMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, ackMasksAll, ackMasksAll.length, false);
        ackMsgTest(1);
    }

    private void postMsgTest(int repeat)
    {
        int extraAction;

        int postMask;

        int msgKeyMask;
        int decodeMsgKeyMask;

        PostMsg postMsg = (PostMsg)msg;

        int domainType = DomainTypes.MARKET_PRICE;

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
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */

                        postMsg.clear();

                        postMsg.msgClass(MsgClasses.POST);
                        postMsg.streamId(streamId);
                        postMsg.domainType(domainType);
                        postMsg.flags(postMask);

                        /* Add msgKey */
                        if ((postMask & PostMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            postMsg.msgKey().flags(msgKeyMask);
                            encodeMsgKey(postMsg.msgKey());
                        }

                        /* Add Post ID */
                        if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                        {
                            postMsg.postId(postId);

                        }

                        if ((postMask & PostMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            assertEquals("Correct Permission Info", permissionDataLen, postMsg.permData().length());
                            assertEquals("Correct Permission Info", true, isMemEqual(postMsg.permData(), permissionData, permissionDataLen));
                        }

                        /* Set up user information */
                        postMsg.postUserInfo().userAddr("204.170.85.29");
                        postMsg.postUserInfo().userId(0xCCAA551DL);

                        /* Add Post Sequence Number */
                        if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(postMask + 0xFFFF) : (int)postMask;
                            postMsg.seqNum(seqNum);
                        }

                        if ((postMask & PostMsgFlags.HAS_PART_NUM) > 0)
                        {
                            postMsg.partNum(0x100f);
                        }

                        if ((postMask & PostMsgFlags.HAS_POST_USER_RIGHTS) > 0)
                        {
                            postMsg.postUserRights(0x0001);
                        }

                        /* Add Extended Header */
                        if ((postMask & PostMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            postMsg.extendedHeader().data(extendedHeader);
                        }

                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.encodedDataBody(encDataBuf);
                            postMsg.containerType(g_dataFormat);

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                                msg.encodedDataBody(encDataBuf);
                            }
                            else
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                                msg.encodedDataBody(encDataBuf);
                            }

                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            postMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, postMsg.extendedHeader(), postMsg.flags(), UpdateMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            postMsg.containerType(DataTypes.NO_DATA);
                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                        }

                        /* Decode msg */
                        decodeMsgKeyMask = fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
						setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            assertEquals("ExtractMsgClass", MsgClasses.POST, decIter.extractMsgClass());
                            setupDecodeIterator();

                            /* extract the domainType */
                            assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                            setupDecodeIterator();

                            /* extract streamId */
                            assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* replace the streamId */
                            assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
                            setupEncodeIterator();

                            /* extract the streamId */
                            assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* extract the seqNum */
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum", seqNum, decIter.extractSeqNum());
                            setupDecodeIterator();

                            /* replace the seqNum */
                            assertEquals("ReplaceSeqNum",
                                         ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE),
                                         encIter.replaceSeqNum(seqNum + 1));
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum2", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum2", seqNum + 1, decIter.extractSeqNum());
                            setupDecodeIterator();

                            /* extract the postId */
                            if ((postMask & PostMsgFlags.HAS_POST_ID) == 0)
                                assertEquals("extractPostId", CodecReturnCodes.FAILURE, decIter.extractPostId());
                            if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                                assertEquals("extractPostId", postId, decIter.extractPostId());
                            setupDecodeIterator();

                            /* replace the postId */
                            assertEquals("ReplacePostId",
                                         ((postMask & PostMsgFlags.HAS_POST_ID) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE),
                                         encIter.replacePostId(postId + 1));
                            setupEncodeIterator();

                            /* extract the new postId */
                            if ((postMask & PostMsgFlags.HAS_POST_ID) == 0)
                                assertEquals("extractPostId2", CodecReturnCodes.FAILURE, decIter.extractPostId());
                            if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                                assertEquals("extractPostId2", postId + 1, decIter.extractPostId());
                            setupDecodeIterator();
                        }

                        assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
                        assertEquals("Correct MsgBase", MsgClasses.POST, msg.msgClass());
                        assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                        assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                        assertEquals("Correct MsgBase", (g_changeData ? domainType : domainType), msg.domainType());

                        assertEquals("Correct Mask", postMask, postMsg.flags());

                        /* Check msgKey */
                        if ((postMask & PostMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            assertEquals("Correct Msg Key Mask", decodeMsgKeyMask, postMsg.msgKey().flags());
                            decodeMsgKey(decIter, postMsg.msgKey());
                        }

                        /* Check Post ID */
                        if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                        {
                            assertEquals("Correct Permission Info", (g_changeData ? postId + 1 : postId), postMsg.postId());
                        }

                        /* Check Post User Info */
                        assertEquals("Correct Post User Address", 0xCCAA551DL, postMsg.postUserInfo().userAddr());

                        assertEquals("Correct Post User ID", 0xCCAA551DL, postMsg.postUserInfo().userId());

                        /* Check Item Sequence Number */
                        if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            assertEquals("Correct Item Sequence Number", (g_changeData ? seqNum + 1 : seqNum), postMsg.seqNum());
                        }

                        if ((postMask & PostMsgFlags.HAS_PART_NUM) > 0)
                        {
                            assertEquals("Correct PartNum", 0x100f, postMsg.partNum());
                        }

                        /* Check Post User Rights */
                        if ((postMask & PostMsgFlags.HAS_POST_USER_RIGHTS) > 0)
                        {
                            assertEquals("Correct Post User Rights", 0x0001, postMsg.postUserRights());
                        }

                        /* Check Permission Info */
                        if ((postMask & PostMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            assertEquals("Correct Permission Info", permissionDataLen, postMsg.permData().length());
                            assertEquals("Correct Permission Info", true, isMemEqual(postMsg.permData(), permissionData, permissionDataLen));
                        }

                        /* Check Extended Header */
                        if ((postMask & PostMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            assertEquals("Correct Extended Header", extendedHeaderLen, postMsg.extendedHeader().length());
                            assertEquals("Correct Extended Header", true, isMemEqual(postMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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

    @Test
    public void postMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, postMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, postMasksCommon, postMasksCommon.length, false);
        postMsgTest(repeatCount);
    }

    @Test
    public void postMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, postMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, postMasksAll, postMasksAll.length, false);
        postMsgTest(1);
    }

    private void genericMsgTest(int repeat)
    {
        int extraAction;

        int genericMask;

        int msgKeyMask;
        int decodeMsgKeyMask;

        GenericMsg genMsg = (GenericMsg)msg;

        int domainType = DomainTypes.STORY;

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
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */

                        genMsg.clear();

                        genMsg.msgClass(MsgClasses.GENERIC);
                        genMsg.streamId(streamId);
                        genMsg.domainType(domainType);
                        genMsg.flags(genericMask);

                        /* Add msgKey */
                        if ((genericMask & GenericMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            genMsg.msgKey().flags(msgKeyMask);
                            encodeMsgKey(genMsg.msgKey());
                        }

                        /* Add Permission Info */
                        if ((genericMask & GenericMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            genMsg.permData().data(permissionData);
                        }

                        /* Add Item Sequence Number */
                        if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF) : (int)genericMask;
                            genMsg.seqNum(seqNum);
                        }

                        if ((genericMask & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
                        {
                            genMsg.secondarySeqNum((extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF) : (int)genericMask);
                        }

                        if ((genericMask & GenericMsgFlags.HAS_PART_NUM) > 0)
                        {
                            genMsg.partNum(0x100f);
                        }

                        /* Add Extended Header */
                        if ((genericMask & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            genMsg.extendedHeader().data(extendedHeader);
                        }
                        
                        /* test setGenericCompleteFlag()/unsetGenericCompleteFlag(), remove/add it here first, then add/remove later*/
                        int genMsgFlags = genMsg.flags();
    					if ( (genericMask & GenericMsgFlags.MESSAGE_COMPLETE )  > 0)
    						genMsg.flags( genMsgFlags & ~GenericMsgFlags.MESSAGE_COMPLETE);
    					else
    						genMsg.flags(genMsgFlags | GenericMsgFlags.MESSAGE_COMPLETE);
    						
                        /* Add Payload */
                        if ((extraAction & ExtraTestAction.PRE_PAYLOAD) > 0)
                        {
                            msg.encodedDataBody(encDataBuf);
                            genMsg.containerType(g_dataFormat);

                            /* Trim payload length if desired */
                            if ((extraAction & ExtraTestAction.TRIM_DATA_BUF) > 0)
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());
                                msg.encodedDataBody(encDataBuf);
                            }
                            else
                            {
                                encDataBuf.data(encDataBuf.data(), 0, encDataBuf.length());
                                msg.encodedDataBody(encDataBuf);
                            }

                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                            
                            /* Call setGenericCompleteFlag()/unsetGenericCompleteFlag()  on EncodeIterator to reset back to the original genericMask*/
                            if ( (genericMask & GenericMsgFlags.MESSAGE_COMPLETE )  > 0)
                            	assertEquals( "setGenericCompleteFlag", CodecReturnCodes.SUCCESS, encIter.setGenericCompleteFlag());
                            else
    							assertEquals( "unsetGenericCompleteFlag", CodecReturnCodes.SUCCESS, encIter.unsetGenericCompleteFlag());
    						
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            genMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, genMsg.extendedHeader(), genMsg.flags(), GenericMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            /* Call setGenericCompleteFlag()/unsetGenericCompleteFlag()  on EncodeIterator to reset back to the original genericMask*/
                            if ( (genericMask & GenericMsgFlags.MESSAGE_COMPLETE )  > 0)
                            	assertEquals( "setGenericCompleteFlag", CodecReturnCodes.SUCCESS, encIter.setGenericCompleteFlag());
                            else
    							assertEquals( "unsetGenericCompleteFlag", CodecReturnCodes.SUCCESS, encIter.unsetGenericCompleteFlag());
                            
                            assertEquals("EncodeMsgComplete", CodecReturnCodes.SUCCESS, msg.encodeComplete(encIter, true));
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            genMsg.containerType(DataTypes.NO_DATA);
                            assertEquals("EncodeMsg", CodecReturnCodes.SUCCESS, msg.encode(encIter));
                            
                            /* Call setGenericCompleteFlag()/unsetGenericCompleteFlag()  on EncodeIterator to reset back to the original genericMask*/
                            if ( (genericMask & GenericMsgFlags.MESSAGE_COMPLETE )  > 0)
                            	assertEquals( "setGenericCompleteFlag", CodecReturnCodes.SUCCESS, encIter.setGenericCompleteFlag());
                            else
    							assertEquals( "unsetGenericCompleteFlag", CodecReturnCodes.SUCCESS, encIter.unsetGenericCompleteFlag());
                        }

                        /* Decode msg */
                        decodeMsgKeyMask = fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
						setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            assertEquals("ExtractMsgClass", MsgClasses.GENERIC, decIter.extractMsgClass());
                            setupDecodeIterator();

                            /* extract the domainType */
                            assertEquals("ExtractDomainType", domainType, decIter.extractDomainType());
                            setupDecodeIterator();

                            /* extract streamId */
                            assertEquals("ExtractStreamId", streamId, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* replace the streamId */
                            assertEquals("ReplaceStreamId", CodecReturnCodes.SUCCESS, encIter.replaceStreamId(streamId + 1));
                            setupEncodeIterator();

                            /* extract the streamId */
                            assertEquals("ExtractStreamId2", streamId + 1, decIter.extractStreamId());
                            setupDecodeIterator();

                            /* extract the seqNum */
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum", seqNum, decIter.extractSeqNum());
                            setupDecodeIterator();

                            /* replace the seqNum */
                            assertEquals("ReplaceSeqNum",
                                         ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE),
                                         encIter.replaceSeqNum(seqNum + 1));
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                                assertEquals("extractSeqNum2", CodecReturnCodes.FAILURE, decIter.extractSeqNum());
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                                assertEquals("extractSeqNum2", seqNum + 1, decIter.extractSeqNum());
                            setupDecodeIterator();
                        }

                        assertEquals("DecodeMsg", CodecReturnCodes.SUCCESS, msg.decode(decIter));
                        assertEquals("Correct MsgBase", MsgClasses.GENERIC, msg.msgClass());
                        assertEquals("Correct MsgBase", ((extraAction & ExtraTestAction.PAYLOAD) > 0 ? g_dataFormat : DataTypes.NO_DATA), msg.containerType());
                        assertEquals("Correct MsgBase", (g_changeData ? streamId + 1 : streamId), msg.streamId());
                        assertEquals("Correct MsgBase", (g_changeData ? domainType : domainType), msg.domainType());

                        assertEquals("Correct Mask", genericMask, genMsg.flags());

                        /* Check msgKey */
                        if ((genericMask & GenericMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            assertEquals("Correct Msg Key Mask", decodeMsgKeyMask, genMsg.msgKey().flags());
                            decodeMsgKey(decIter, genMsg.msgKey());
                        }

                        /* Check Permission Info */
                        if ((genericMask & GenericMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            assertEquals("Correct Permission Info", permissionDataLen, genMsg.permData().length());
                            assertEquals("Correct Permission Info", true, isMemEqual(genMsg.permData(), permissionData, permissionDataLen));
                        }

                        /* Check Item Sequence Number */
                        if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            assertEquals("Correct Item Sequence Number", (g_changeData ? seqNum + 1 : seqNum), genMsg.seqNum());
                        }

                        /* Check Item Secondary Sequence Number */
                        if ((genericMask & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
                        {
                            assertEquals("Correct Item Secondary Sequence Number",
                                         (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF) : (int)genericMask,
                                         genMsg.secondarySeqNum());
                        }

                        if ((genericMask & GenericMsgFlags.HAS_PART_NUM) > 0)
                        {
                            assertEquals("Correct PartNum", 0x100f, genMsg.partNum());
                        }

                        /* Check Extended Header */
                        if ((genericMask & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            assertEquals("Correct Extended Header", extendedHeaderLen, genMsg.extendedHeader().length());
                            assertEquals("Correct Extended Header", true, isMemEqual(genMsg.extendedHeader(), extendedHeader, extendedHeaderLen));
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

    @Test
    public void genericMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, genericMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, genericMasksCommon, genericMasksCommon.length, false);
        genericMsgTest(repeatCount);
    }

    @Test
    public void genericMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, genericMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, genericMasksAll, genericMasksAll.length, false);
        genericMsgTest(1);
    }

}
