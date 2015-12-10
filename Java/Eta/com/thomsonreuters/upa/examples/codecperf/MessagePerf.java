package com.thomsonreuters.upa.examples.codecperf;

import java.nio.ByteBuffer;

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
import com.thomsonreuters.upa.codec.UpdateMsgFlags;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;

public class MessagePerf
{
    Msg msg = CodecFactory.createMsg();
    /* Encode nested Msg */
    int g_dataFormat = DataTypes.FIELD_LIST;;
    boolean g_changeData = true;

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

    private int repeatCount = 15;
    private boolean commonSetupComplete = false, fullSetupComplete = false;
    private final int FIELD_LIST_MAX_ENTRIES = 4; // TODO RRG 6; /* Using all
                                                  // primitives makes the top
                                                  // container too big to test
                                                  // nested structure to maximum
                                                  // depth */
    /* FIDs and primitives for field list */
    int[] fieldListFids = { 1,
            // TODO RRG 2,
            245, 255, 256,
    // TODO RRG 32767
    };

    int[] fieldListDataTypes = { DataTypes.INT,
            // TODO RRG DataTypes.DOUBLE,
            DataTypes.REAL, DataTypes.DATE, DataTypes.TIME,
    // TODO RRG DataTypes.DATETIME
    };
    Int fieldListInt = CodecFactory.createInt();
    // TODO RRG Double fieldListFloat = 0xFF;
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
    EncodeIterator encIter = CodecFactory.createEncodeIterator(), encDataIter = CodecFactory
            .createEncodeIterator();
    DecodeIterator decIter = CodecFactory.createDecodeIterator(), decDataIter = CodecFactory
            .createDecodeIterator();

    private int masksSize, actionsSize, keyMasksSize;
    private int[] masks, actions, keyMasks;
    private int masksIter, actionsIter, keyMasksIter;
    private int[] actionsCommon = { ExtraTestAction.POST_PAYLOAD, ExtraTestAction.PRE_PAYLOAD,
            ExtraTestAction.FOUR_BYTE_SEQ, ExtraTestAction.POST_OPAQUE,
            ExtraTestAction.POST_EXTENDED };
    private int[] actionsAll = { ExtraTestAction.POST_PAYLOAD, ExtraTestAction.PRE_PAYLOAD,
            ExtraTestAction.FOUR_BYTE_SEQ, ExtraTestAction.TRIM_DATA_BUF,
            ExtraTestAction.POST_OPAQUE, ExtraTestAction.POST_EXTENDED };
    private int[] keyMasksCommon = { MsgKeyFlags.HAS_SERVICE_ID, MsgKeyFlags.HAS_NAME,
            MsgKeyFlags.HAS_NAME_TYPE, MsgKeyFlags.HAS_ATTRIB };
    private int[] keyMasksAll = { MsgKeyFlags.HAS_SERVICE_ID, MsgKeyFlags.HAS_NAME,
            MsgKeyFlags.HAS_NAME_TYPE, MsgKeyFlags.HAS_FILTER, MsgKeyFlags.HAS_IDENTIFIER,
            MsgKeyFlags.HAS_ATTRIB };
    private int[] updateMasksCommon = { UpdateMsgFlags.DO_NOT_RIPPLE, UpdateMsgFlags.DO_NOT_CACHE,
            UpdateMsgFlags.HAS_MSG_KEY, UpdateMsgFlags.HAS_PERM_DATA, UpdateMsgFlags.HAS_SEQ_NUM,
            UpdateMsgFlags.HAS_CONF_INFO };
    private int[] updateMasksAll = { UpdateMsgFlags.HAS_EXTENDED_HEADER,
            UpdateMsgFlags.HAS_PERM_DATA, UpdateMsgFlags.HAS_MSG_KEY, UpdateMsgFlags.HAS_SEQ_NUM,
            UpdateMsgFlags.HAS_CONF_INFO, UpdateMsgFlags.DO_NOT_CACHE,
            UpdateMsgFlags.DO_NOT_CONFLATE, UpdateMsgFlags.DO_NOT_RIPPLE,
            UpdateMsgFlags.HAS_POST_USER_INFO, UpdateMsgFlags.DISCARDABLE };
    private int[] responseMasksAll = { RefreshMsgFlags.HAS_EXTENDED_HEADER,
            RefreshMsgFlags.HAS_PERM_DATA, RefreshMsgFlags.HAS_MSG_KEY,
            RefreshMsgFlags.HAS_SEQ_NUM, RefreshMsgFlags.SOLICITED,
            RefreshMsgFlags.REFRESH_COMPLETE, RefreshMsgFlags.HAS_QOS, RefreshMsgFlags.CLEAR_CACHE,
            RefreshMsgFlags.DO_NOT_CACHE, RefreshMsgFlags.PRIVATE_STREAM,
            RefreshMsgFlags.HAS_POST_USER_INFO, RefreshMsgFlags.HAS_PART_NUM };

    private int[] requestMasksAll = { RequestMsgFlags.HAS_EXTENDED_HEADER,
            RequestMsgFlags.HAS_PRIORITY, RequestMsgFlags.STREAMING,
            RequestMsgFlags.MSG_KEY_IN_UPDATES, RequestMsgFlags.CONF_INFO_IN_UPDATES,
            RequestMsgFlags.NO_REFRESH, RequestMsgFlags.HAS_QOS, RequestMsgFlags.HAS_WORST_QOS,
            RequestMsgFlags.PRIVATE_STREAM };

    private int[] requestMasksCommon = { RequestMsgFlags.NO_REFRESH, RequestMsgFlags.STREAMING,
            RequestMsgFlags.HAS_PRIORITY, RequestMsgFlags.HAS_QOS, RequestMsgFlags.HAS_WORST_QOS,
            RequestMsgFlags.HAS_EXTENDED_HEADER, RequestMsgFlags.MSG_KEY_IN_UPDATES,
            RequestMsgFlags.CONF_INFO_IN_UPDATES };

    private int[] responseMasksCommon = { RefreshMsgFlags.DO_NOT_CACHE,
            RefreshMsgFlags.CLEAR_CACHE, RefreshMsgFlags.REFRESH_COMPLETE,
            RefreshMsgFlags.HAS_MSG_KEY, RefreshMsgFlags.HAS_PERM_DATA,
            RefreshMsgFlags.HAS_SEQ_NUM, RefreshMsgFlags.HAS_QOS,
            RefreshMsgFlags.HAS_EXTENDED_HEADER };

    private int[] statusMasksAll = { StatusMsgFlags.HAS_EXTENDED_HEADER,
            StatusMsgFlags.HAS_PERM_DATA, StatusMsgFlags.HAS_MSG_KEY, StatusMsgFlags.HAS_GROUP_ID,
            StatusMsgFlags.HAS_STATE, StatusMsgFlags.CLEAR_CACHE, StatusMsgFlags.PRIVATE_STREAM,
            StatusMsgFlags.HAS_POST_USER_INFO };

    private int[] statusMasksCommon = { StatusMsgFlags.CLEAR_CACHE, StatusMsgFlags.HAS_STATE,
            StatusMsgFlags.HAS_PERM_DATA, StatusMsgFlags.HAS_EXTENDED_HEADER };

    private int[] closeMasksAll = { CloseMsgFlags.HAS_EXTENDED_HEADER, CloseMsgFlags.ACK };

    private int[] closeMasksCommon = { CloseMsgFlags.HAS_EXTENDED_HEADER };

    private int[] ackMasksAll = { AckMsgFlags.HAS_NAK_CODE, AckMsgFlags.HAS_TEXT,
            AckMsgFlags.HAS_EXTENDED_HEADER, AckMsgFlags.PRIVATE_STREAM, AckMsgFlags.HAS_SEQ_NUM,
            AckMsgFlags.HAS_MSG_KEY };

    private int[] ackMasksCommon = { AckMsgFlags.HAS_NAK_CODE, AckMsgFlags.HAS_TEXT,
            AckMsgFlags.HAS_MSG_KEY, AckMsgFlags.HAS_EXTENDED_HEADER };

    private int[] postMasksAll = { PostMsgFlags.HAS_EXTENDED_HEADER, PostMsgFlags.HAS_POST_ID,
            PostMsgFlags.HAS_MSG_KEY, PostMsgFlags.HAS_SEQ_NUM, PostMsgFlags.POST_COMPLETE,
            PostMsgFlags.ACK, PostMsgFlags.HAS_PERM_DATA, PostMsgFlags.HAS_PART_NUM,
            PostMsgFlags.HAS_POST_USER_RIGHTS };

    private int[] postMasksCommon = { PostMsgFlags.HAS_POST_ID, PostMsgFlags.HAS_MSG_KEY,
            PostMsgFlags.ACK, PostMsgFlags.HAS_PERM_DATA, PostMsgFlags.HAS_POST_USER_RIGHTS };

    private int[] genericMasksAll = { GenericMsgFlags.HAS_EXTENDED_HEADER,
            GenericMsgFlags.HAS_PERM_DATA, GenericMsgFlags.HAS_MSG_KEY,
            GenericMsgFlags.HAS_SEQ_NUM, GenericMsgFlags.MESSAGE_COMPLETE,
            GenericMsgFlags.HAS_SECONDARY_SEQ_NUM, GenericMsgFlags.HAS_PART_NUM };

    private int[] genericMasksCommon = { GenericMsgFlags.MESSAGE_COMPLETE,
            GenericMsgFlags.HAS_MSG_KEY, GenericMsgFlags.HAS_PERM_DATA,
            GenericMsgFlags.HAS_SEQ_NUM, GenericMsgFlags.HAS_EXTENDED_HEADER };

    /*
     * ExtraTestActions - Additional flag set for testing situations
     * additionally not covered by flags
     */
    private class ExtraTestAction
    {
        public static final int POST_PAYLOAD = 0x0001; /*
                                                        * Add post-encoded
                                                        * payload to the message
                                                        * (if PRE_PAYLOAD is not
                                                        * set)
                                                        */
        public static final int PRE_PAYLOAD = 0x0002; /*
                                                       * Add a pre-encoded
                                                       * payload to the message
                                                       */
        public static final int FOUR_BYTE_SEQ = 0x0004; /*
                                                         * Test a sequence
                                                         * number with 4 bytes
                                                         * instead of 2
                                                         *//*
                                                            * TODO is this
                                                            * needed in the UPA
                                                            * version?
                                                            */
        public static final int TRIM_DATA_BUF = 0x0008; /*
                                                         * Test trimming the
                                                         * data buffer size to
                                                         * the actual encoded
                                                         * length
                                                         */
        public static final int POST_OPAQUE = 0x0010; /*
                                                       * Test post encoding of
                                                       * opaque - only applies
                                                       * in EncodeMsgInit cases
                                                       */
        public static final int POST_EXTENDED = 0x0020; /*
                                                         * Test post encoding of
                                                         * extended header -
                                                         * only applies in
                                                         * EncodeMsgInit cases
                                                         */
        public static final int PAYLOAD = POST_PAYLOAD | PRE_PAYLOAD; /*
                                                                       * For
                                                                       * Decoder
                                                                       * --
                                                                       * indicates
                                                                       * payload
                                                                       * whether
                                                                       * pre or
                                                                       * post
                                                                       * -encoded
                                                                       */
    }

    public MessagePerf()
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

        updMsg.msgKey().flags(MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME
                                      | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_FILTER
                                      | MsgKeyFlags.HAS_IDENTIFIER | MsgKeyFlags.HAS_ATTRIB);
        encodeMsgKey(updMsg.msgKey());

        updMsg.encodeInit(encIter, 0);
        encodeFieldList(encIter);

        updMsg.encodeComplete(encIter, true);
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
        fieldList.encodeInit(encIter, null, 0);

        /* add entries */
        for (iiEntry = 0; iiEntry < FIELD_LIST_MAX_ENTRIES; ++iiEntry)
        {
            entry.clear();
            entry.fieldId(fieldListFids[iiEntry]);
            entry.dataType(fieldListDataTypes[iiEntry]);

            switch (fieldListDataTypes[iiEntry])
            {
                case DataTypes.INT:
                    entry.encode(encIter, fieldListInt);
                    break;
                case DataTypes.DOUBLE:
                    // entry.encode(encIter, fieldListFloat);
                    break;
                case DataTypes.REAL:
                    entry.encode(encIter, fieldListReal);
                    break;
                case DataTypes.DATE:
                    entry.encode(encIter, fieldListDate);
                    break;
                case DataTypes.TIME:
                    entry.encode(encIter, fieldListTime);
                    break;
                case DataTypes.DATETIME:
                    entry.encode(encIter, fieldListDateTime);
                    break;
                default:
                    break;
            }
        }

        /* finish encoding */
        fieldList.encodeComplete(encIter, true);
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

        /* Add Attrib ContainerType/Data *//* UPA calls this "opaque" */
        if ((mask & MsgKeyFlags.HAS_ATTRIB) > 0)
        {
            EncodeIterator encIter = CodecFactory.createEncodeIterator();
            opaque.position(0);
            key.encodedAttrib().data(opaque, 0, opaqueLen);
            encIter.setBufferAndRWFVersion(key.encodedAttrib(), Codec.majorVersion(),
                                           Codec.minorVersion());
            encodeFieldList(encIter);
            key.attribContainerType(DataTypes.FIELD_LIST);
        }
    }

    /*
     * does the work for encoding opaque and/or extended header after MsgInit is
     * called
     */
    private void postEncodeMsg(EncodeIterator encIter, Msg msg, int extraAction, Buffer extHdr,
            int msgFlags, int extHdrFlag)
    {

        int ret = CodecReturnCodes.ENCODE_CONTAINER;

        if ((extraAction & ExtraTestAction.POST_OPAQUE) > 0 && msg.msgKey().checkHasAttrib())
        {
            /* if we are post encoding the opaque, make it a field list */
            msg.msgKey().encodedAttrib().clear();
            msg.msgKey().attribContainerType(DataTypes.FIELD_LIST);
            ret = CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB; // if our opaque gets
                                                          // encoded after, this
                                                          // shoudl be our ret
                                                          // */

            if ((extraAction & ExtraTestAction.POST_EXTENDED) > 0 && extHdr != null)
            {
                /*
                 * if we are also post encoding our extended header, unset its
                 * values
                 */
                extHdr.clear();
            }
        }
        else if ((extraAction & ExtraTestAction.POST_EXTENDED) > 0 && (msgFlags & extHdrFlag) > 0
                && extHdr != null)
        {
            /*
             * if we are also post encoding our extended header, unset its
             * values
             */
            extHdr.clear();
            ret = CodecReturnCodes.ENCODE_EXTENDED_HEADER;
        }

        msg.encodeInit(encIter, 0);

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

            msg.encodeKeyAttribComplete(encIter, true);
        }

        if ((ret == CodecReturnCodes.ENCODE_EXTENDED_HEADER) && extHdr != null)
        {
            Buffer buffer = CodecFactory.createBuffer();

            /* we must encode our extended header now */
            /* just hack copy it onto the wire */
            encIter.encodeNonRWFInit(buffer);

            buffer.data().put(extendedHeader.getBytes());

            encIter.encodeNonRWFComplete(buffer, true);

            msg.encodeExtendedHeaderComplete(encIter, true);
        }
    }

    /*
     * Remove flags that the encoder should have removed because the test
     * is(intentionally) trying to do something wrong
     */
    private int fixDecodeMsgKeyMask(int mask)
    {
        if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0 && !((mask & MsgKeyFlags.HAS_NAME) > 0)) /*
                                                                                             * NameType
                                                                                             * without
                                                                                             * Name
                                                                                             *//*
                                                                                                * UPA
                                                                                                * DOES
                                                                                                * do
                                                                                                * this
                                                                                                * optimization
                                                                                                */
            mask &= ~MsgKeyFlags.HAS_NAME_TYPE;

        return mask;
    }

    /*
     * private boolean isMemEqual(Buffer buf1, Buffer buf2, int len) { boolean
     * retVal = true; int pos1 = buf1.position(), pos2 = buf2.position();
     * ByteBuffer data1 = buf1.data(), data2 = buf2.data();
     * 
     * for (int i = 0; i < len; i++) { if (data1.get(pos1 + i) != data2.get(pos2
     * + i)) { retVal = false; break; } }
     * 
     * return retVal; }
     * 
     * private boolean isMemEqual(Buffer buf1, String buf2, int len) { boolean
     * retVal = true; int pos1 = buf1.position(); ByteBuffer data1 =
     * buf1.data();
     * 
     * for (int i = 0; i < len; i++) { if (data1.get(pos1 + i) !=
     * buf2.charAt(i)) { retVal = false; break; } }
     * 
     * return retVal; }
     */

    private void decodeMsgKey(DecodeIterator decIter, MsgKey key)
    {
        int mask = key.flags();

        /* Check Service ID */
        if ((mask & MsgKeyFlags.HAS_SERVICE_ID) > 0)
        {
            //
        }

        /* Check Name */
        if ((mask & MsgKeyFlags.HAS_NAME) > 0)
        {
            //
        }

        /* Check Name Type */
        if ((mask & MsgKeyFlags.HAS_NAME_TYPE) > 0)
        {
            //
        }

        /* Check Filter */
        if ((mask & MsgKeyFlags.HAS_FILTER) > 0)
        {
            //
        }

        /* Check ID */
        if ((mask & MsgKeyFlags.HAS_IDENTIFIER) > 0)
        {
            //
        }

        /* check opaque */
        if ((mask & MsgKeyFlags.HAS_ATTRIB) > 0)
        {
            msg.decodeKeyAttrib(decIter, key);
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
        // UpdateMsg updateMsg = (UpdateMsg)msg;

        msg.decode(decIter);

        decodeMsgKey(decIter, msg.msgKey());

        decodeFieldList(decIter);
    }

    private void decodeFieldList(DecodeIterator decIter)
    {

        Int decInt = CodecFactory.createInt();
        // TODO RRG Double decFloat;
        Real decReal = CodecFactory.createReal();
        Date decDate = CodecFactory.createDate();
        Time decTime = CodecFactory.createTime();
        DateTime decDateTime = CodecFactory.createDateTime();

        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();

        int iiEntry;

        // Setup container
        container.clear();
        entry.clear();

        // Begin container decoding
        container.decode(decIter, null);

        // Decode entries
        for (iiEntry = 0; iiEntry < FIELD_LIST_MAX_ENTRIES; ++iiEntry)
        {
            entry.decode(decIter);

            switch (fieldListDataTypes[iiEntry])
            {
                case DataTypes.INT:
                    decInt.decode(decIter);
                    break;
                case DataTypes.DOUBLE:
                    // TODO RRG
                    // decFloat.decode(decIter);
                    break;
                case DataTypes.REAL:
                    decReal.decode(decIter);
                    break;
                case DataTypes.DATE:
                    decDate.decode(decIter);
                    break;
                case DataTypes.TIME:
                    decTime.decode(decIter);
                    break;
                case DataTypes.DATETIME:
                    decDateTime.decode(decIter);
                    break;
                default:
                    break;
            }
        }

        entry.decode(decIter);
    }

    private void updateMsgTest(int repeat)
    {
        int updateMask;

        int extraAction;

        UpdateMsg updMsg = (UpdateMsg)msg;

        int msgKeyMask;
        // int decodeMsgKeyMask;

        int domainType = DomainTypes.LOGIN;

        for (; repeat > 0; --repeat)
        {
            for (masksIter = 0; masksIter < masksSize; ++masksIter)
            {
                updateMask = masks[masksIter];

                for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                {
                    extraAction = actions[actionsIter];
                    for (keyMasksIter = 0; keyMasksIter < ((updateMask & UpdateMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize
                            : 1); ++keyMasksIter)
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
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(updateMask + 0xFFFF)
                                    : (int)updateMask;
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
                            updMsg.postUserInfo().userAddr(0xCCAA551DL);
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

                            msg.encode(encIter);
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            updMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, updMsg.extendedHeader(),
                                          updMsg.flags(), UpdateMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            msg.encodeComplete(encIter, true);
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            updMsg.containerType(DataTypes.NO_DATA);
                            msg.encode(encIter);
                        }

                        /* Decode msg */
                        fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
                        setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            decIter.extractMsgClass();
                            setupDecodeIterator();

                            /* extract the domainType */
                            decIter.extractDomainType();
                            setupDecodeIterator();

                            /* extract streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* replace the streamId */
                            encIter.replaceStreamId(streamId + 1);
                            setupEncodeIterator();

                            /* extract the streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* extract the seqNum */
                            decIter.extractSeqNum();
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the seqNum */
                            encIter.replaceSeqNum(seqNum + 1);
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            decIter.extractSeqNum();
                            if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();
                        }

                        msg.decode(decIter);

                        /* copy test */
                        /*
                         * TODO RRG { Msg copyMsg =
                         * CodecFactory.createMsg(); Buffer copyBuffer =
                         * CodecFactory.createBuffer(); int msgSize =
                         * msg.sizeOfMsg(CopyMsgFlags.ALL_FLAGS); ByteBuffer
                         * copyBufData = ByteBuffer.allocate(msgSize);
                         * copyBuffer.data(copyBufData, 0, msgSize); copyMsg =
                         * msg.copy(CopyMsgFlags.ALL_FLAGS, 0, copyMsg,
                         * copyBuffer); UpdateMsg copyUpdateMsg =
                         * (UpdateMsg)copyMsg; }
                         */

                        /* Check msgKey */
                        if ((updateMask & UpdateMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            decodeMsgKey(decIter, updMsg.msgKey());
                        }

                        /* Check Permission Info */
                        if ((updateMask & UpdateMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            //
                        }

                        /* Check Item Sequence Number */
                        if ((updateMask & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            //
                        }

                        /* Check Conflation Info */
                        if ((updateMask & UpdateMsgFlags.HAS_CONF_INFO) > 0)
                        {
                            //
                        }

                        if ((updateMask & UpdateMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            //
                        }

                        /* Check Extended Header */
                        if ((updateMask & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            //
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
    private void allocateFlagCombinations(int[] dstMasks, int[] srcMasks, int srcMasksSize,
            boolean skipZero)
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
            fullSetupComplete = false;
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
            commonSetupComplete = false;
        }
    }

    public void updateMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, updateMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, updateMasksCommon, updateMasksCommon.length, false);
        updateMsgTest(3 * repeatCount);
    }

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

        int domainType = DomainTypes.LOGIN;

        for (; repeat > 0; --repeat)
        {
            for (masksIter = 0; masksIter < masksSize; ++masksIter)
            {
                responseMask = masks[masksIter];

                for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                {
                    extraAction = actions[actionsIter];
                    for (keyMasksIter = 0; keyMasksIter < ((responseMask & RefreshMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize
                            : 1); ++keyMasksIter)
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
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(responseMask + 0xFFFF)
                                    : (int)responseMask;
                            respMsg.seqNum(seqNum);
                        }

                        if ((responseMask & RefreshMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            respMsg.postUserInfo().userAddr(0xCCAA551DL);
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

                            msg.encode(encIter);
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            respMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, respMsg.extendedHeader(),
                                          respMsg.flags(), RefreshMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            msg.encodeComplete(encIter, true);
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            respMsg.containerType(DataTypes.NO_DATA);
                            msg.encode(encIter);
                        }

                        /* Decode msg */
                        fixDecodeMsgKeyMask(msgKeyMask);

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
                            decIter.extractMsgClass();
                            setupDecodeIterator();

                            /* extract the domainType */
                            decIter.extractDomainType();
                            setupDecodeIterator();

                            /* extract streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* replace the streamId */
                            encIter.replaceStreamId(streamId + 1);
                            setupEncodeIterator();

                            /* extract the streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* extract the seqNum */
                            decIter.extractSeqNum();
                            if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the seqNum */
                            encIter.replaceSeqNum(seqNum + 1);
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            decIter.extractSeqNum();
                            setupDecodeIterator();

                            /* extract the groupId */
                            decIter.extractGroupId(extractGroupId);
                            setupDecodeIterator();

                            /* replace the groupId */
                            groupId[0] = '4';
                            encIter.replaceGroupId(newGroupId);
                            setupEncodeIterator();

                            /* extract the new groupId */
                            decIter.extractGroupId(extractGroupId);
                            setupDecodeIterator();

                            /* replace the streamState */
                            encIter.replaceStreamState(StreamStates.REDIRECTED);
                            setupEncodeIterator();

                            /* replace the dataState */
                            encIter.replaceDataState(DataStates.OK);
                            setupEncodeIterator();

                            /* replace the stateCode */
                            encIter.replaceStateCode(StateCodes.INVALID_VIEW);
                            setupEncodeIterator();
                        }

                        msg.decode(decIter);

                        /* copy test */
                        /*
                         * TODO RRG { Msg copyMsg =
                         * CodecFactory.createMsg(); Buffer copyBuffer =
                         * CodecFactory.createBuffer(); int msgSize =
                         * msg.sizeOfMsg(CopyMsgFlags.ALL_FLAGS); ByteBuffer
                         * copyBufData = ByteBuffer.allocate(msgSize);
                         * copyBuffer.data(copyBufData, 0, msgSize); copyMsg =
                         * msg.copy(CopyMsgFlags.ALL_FLAGS, 0, copyMsg,
                         * copyBuffer); RefreshMsg copyRefreshMsg =
                         * (RefreshMsg)copyMsg; }
                         */

                        /* Check msgKey */
                        if ((responseMask & RefreshMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            //
                        }

                        /* Check State */
                        {
                            //
                        }

                        /* Check Group Id */
                        Buffer gid = CodecFactory.createBuffer();
                        gid.data(ByteBuffer.wrap(groupId));

                        /* Check Permission Info */
                        if ((responseMask & RefreshMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            //
                        }

                        /* Check Item Sequence Number */
                        if ((responseMask & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            //
                        }

                        if ((responseMask & RefreshMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            //
                        }

                        if ((responseMask & RefreshMsgFlags.HAS_PART_NUM) > 0)
                        {
                            //
                        }

                        /* Check QoS */
                        if ((responseMask & RefreshMsgFlags.HAS_QOS) > 0)
                        {
                            //
                        }

                        /* Check Extended Header */
                        if ((responseMask & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            //
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

    public void refreshMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, responseMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, responseMasksCommon, responseMasksCommon.length, false);
        refreshMsgTest(repeatCount);
    }

    public void refreshMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, responseMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, responseMasksAll, responseMasksAll.length, false);
        refreshMsgTest(1);
    }

    private void requestMsgTest(int repeat)
    {
        int extraAction;

        int requestMask;
        RequestMsg reqMsg = (RequestMsg)msg;

        int msgKeyMask;

        int domainType = DomainTypes.DICTIONARY;

        for (; repeat > 0; --repeat)
        {
            for (masksIter = 0; masksIter < masksSize; ++masksIter)
            {
                requestMask = masks[masksIter];

                for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                {
                    extraAction = actions[actionsIter];
                    for (keyMasksIter = 0; keyMasksIter < keyMasksSize /*
                                                                        * reqMsg
                                                                        * always
                                                                        * has
                                                                        * key
                                                                        */; ++keyMasksIter)
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

                            msg.encode(encIter);
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            reqMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, reqMsg.extendedHeader(),
                                          reqMsg.flags(), RequestMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            msg.encodeComplete(encIter, true);
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            reqMsg.containerType(DataTypes.NO_DATA);
                            msg.encode(encIter);
                        }

                        /* Decode msg */
                        fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
                        setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                        	decIter.extractMsgClass();
                            setupDecodeIterator();

                            /* extract the domainType */
                            decIter.extractDomainType();
                            setupDecodeIterator();

                            /* extract streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* replace the streamId */
                            encIter.replaceStreamId(streamId + 1);
                            setupEncodeIterator();

                            /* extract the streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();
                        }

                        msg.decode(decIter);

                        /* Check mask and msgBase */
                        {
                            //
                        }

                        /* copy test */
                        /*
                         * TODO RRG { Msg copyMsg =
                         * CodecFactory.createMsg(); Buffer copyBuffer =
                         * CodecFactory.createBuffer(); int msgSize =
                         * msg.sizeOfMsg(CopyMsgFlags.ALL_FLAGS); ByteBuffer
                         * copyBufData = ByteBuffer.allocate(msgSize);
                         * copyBuffer.data(copyBufData, 0, msgSize); copyMsg =
                         * msg.copy(CopyMsgFlags.ALL_FLAGS, 0, copyMsg,
                         * copyBuffer); RequestMsg copyRequestMsg =
                         * (RequestMsg)copyMsg; }
                         */

                        /* Check msg key */
                        decodeMsgKey(decIter, reqMsg.msgKey());

                        /* Check priority info */
                        if ((requestMask & RequestMsgFlags.HAS_PRIORITY) > 0)
                        {
                            //
                        }

                        /* Check best QoS */
                        if ((requestMask & RequestMsgFlags.HAS_QOS) > 0)
                        {
                            //
                        }

                        /* Check worst QoS */
                        if ((requestMask & RequestMsgFlags.HAS_WORST_QOS) > 0)
                        {
                            //
                        }

                        /* Check extended header */
                        if ((requestMask & RequestMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            //
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

    public void requestMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, requestMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, requestMasksCommon, requestMasksCommon.length, false);
        requestMsgTest(repeatCount);
    }

    public void requestMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, requestMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, requestMasksAll, requestMasksAll.length, false);
        requestMsgTest(1);
    }

    private void statusMsgTest(int repeat)
    {
        int extraAction;

        int statusMask;

        StatusMsg statMsg = (StatusMsg)msg;

        int msgKeyMask;

        int domainType = DomainTypes.MARKET_PRICE;

        for (; repeat > 0; --repeat)
        {
            for (masksIter = 0; masksIter < masksSize; ++masksIter)
            {
                statusMask = masks[masksIter];

                for (actionsIter = 0; actionsIter < actionsSize; ++actionsIter)
                {
                    extraAction = actions[actionsIter];
                    for (keyMasksIter = 0; keyMasksIter < ((statusMask & StatusMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize
                            : 1); ++keyMasksIter)
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
                            statMsg.postUserInfo().userAddr(0xCCAA551DL);
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

                            msg.encode(encIter);
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            statMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, statMsg.extendedHeader(),
                                          statMsg.flags(), StatusMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            msg.encodeComplete(encIter, true);
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            statMsg.containerType(DataTypes.NO_DATA);
                            msg.encode(encIter);
                        }

                        /* Decode msg */
                        fixDecodeMsgKeyMask(msgKeyMask);

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
                            decIter.extractMsgClass();
                            setupDecodeIterator();

                            /* extract the domainType */
                            decIter.extractDomainType();
                            setupDecodeIterator();

                            /* extract streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* replace the streamId */
                            encIter.replaceStreamId(streamId + 1);
                            setupEncodeIterator();

                            /* extract the streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* extract the groupId */
                            decIter.extractGroupId(extractGroupId);
                            if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the groupId */
                            groupId[0] = '4';
                            encIter.replaceGroupId(newGroupId);
                            setupEncodeIterator();

                            /* extract the new groupId */
                            decIter.extractGroupId(extractGroupId);
                            if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the streamState */
                            encIter.replaceStreamState(StreamStates.REDIRECTED);
                            setupEncodeIterator();

                            /* replace the dataState */
                            encIter.replaceDataState(DataStates.OK);
                            setupEncodeIterator();

                            /* replace the stateCode */
                            encIter.replaceStateCode(StateCodes.INVALID_VIEW);
                            setupEncodeIterator();
                        }

                        msg.decode(decIter);

                        /* copy test */
                        /*
                         * TODO RRG { Msg copyMsg =
                         * CodecFactory.createMsg(); Buffer copyBuffer =
                         * CodecFactory.createBuffer(); int msgSize =
                         * msg.sizeOfMsg(CopyMsgFlags.ALL_FLAGS); ByteBuffer
                         * copyBufData = ByteBuffer.allocate(msgSize);
                         * copyBuffer.data(copyBufData, 0, msgSize); copyMsg =
                         * msg.copy(CopyMsgFlags.ALL_FLAGS, 0, copyMsg,
                         * copyBuffer); RefreshMsg copyRefreshMsg =
                         * (RefreshMsg)copyMsg; }
                         */

                        /* Check Item State */
                        if ((statusMask & StatusMsgFlags.HAS_STATE) > 0)
                        {
                            //
                        }

                        if ((statusMask & StatusMsgFlags.HAS_GROUP_ID) > 0)
                        {
                            Buffer gid = CodecFactory.createBuffer();
                            gid.data(ByteBuffer.wrap(groupId));
                        }

                        /* Check msgKey */
                        if ((statusMask & StatusMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            decodeMsgKey(decIter, statMsg.msgKey());
                        }

                        /* Check Permission Info */
                        if ((statusMask & StatusMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            //
                        }

                        if ((statusMask & StatusMsgFlags.HAS_POST_USER_INFO) > 0)
                        {
                            //
                        }

                        /* Check Extended Header */
                        if ((statusMask & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            //
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

    public void statusMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, statusMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, statusMasksCommon, statusMasksCommon.length, false);
        statusMsgTest(repeatCount);
    }

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

                        msg.encode(encIter);
                    }
                    else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                    {
                        closeMsg.containerType(g_dataFormat);

                        postEncodeMsg(encIter, msg, extraAction, closeMsg.extendedHeader(),
                                      closeMsg.flags(), CloseMsgFlags.HAS_EXTENDED_HEADER);

                        encodePayload(encIter);
                        encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                        msg.encodeComplete(encIter, true);
                    }
                    else
                    {
                        msg.encodedDataBody().clear();
                        closeMsg.containerType(DataTypes.NO_DATA);
                        msg.encode(encIter);
                    }

                    setupDecodeIterator();
                    setupEncodeIterator();

                    if (g_changeData)
                    {
                        /* extract the msgClass */
                    	decIter.extractMsgClass();
                        setupDecodeIterator();

                        /* extract the domainType */
                        decIter.extractDomainType();
                        setupDecodeIterator();

                        /* extract streamId */
                        decIter.extractStreamId();
                        setupDecodeIterator();

                        /* replace the streamId */
                        encIter.replaceStreamId(streamId + 1);
                        setupEncodeIterator();

                        /* extract the streamId */
                        decIter.extractStreamId();
                        setupDecodeIterator();
                    }

                    msg.decode(decIter);

                    /* copy test */
                    /*
                     * TODO RRG { Msg copyMsg =
                     * CodecFactory.createMsg(); Buffer copyBuffer =
                     * CodecFactory.createBuffer(); int msgSize =
                     * msg.sizeOfMsg(CopyMsgFlags.ALL_FLAGS); ByteBuffer
                     * copyBufData = ByteBuffer.allocate(msgSize);
                     * copyBuffer.data(copyBufData, 0, msgSize); copyMsg =
                     * msg.copy(CopyMsgFlags.ALL_FLAGS, 0, copyMsg,
                     * copyBuffer); CloseMsg copyCloseMsg =
                     * (CloseMsg)copyMsg; }
                     */

                    /* Check Extended Header */
                    if ((closeMask & CloseMsgFlags.HAS_EXTENDED_HEADER) > 0)
                    {
                        //
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

    public void closeMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, closeMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, closeMasksCommon, closeMasksCommon.length, false);
        closeMsgTest(repeatCount);
    }

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
        // int decodeMsgKeyMask;

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
                    for (keyMasksIter = 0; keyMasksIter < ((ackMask & AckMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize
                            : 1); ++keyMasksIter)
                    {
                        msgKeyMask = keyMasks[keyMasksIter];
                        encBuf.data().position(0);
                        setupEncodeIterator();

                        /* Encode msg */

                        ackMsg.clear();

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
                            ackMsg.flags(msgKeyMask);
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

                            msg.encode(encIter);
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            ackMsg.containerType(g_dataFormat);

                            /*
                             * if our key opaque and our extended header are
                             * pre-encoded, EncodeMsgInit should tell us to
                             * encode our payload/container
                             */
                            postEncodeMsg(encIter, msg, extraAction, ackMsg.extendedHeader(),
                                          ackMsg.flags(), AckMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            msg.encodeComplete(encIter, true);
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            ackMsg.containerType(DataTypes.NO_DATA);
                            msg.encode(encIter);
                        }

                        /* Decode msg */
                        fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
                        setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            decIter.extractMsgClass();
                            setupDecodeIterator();

                            /* extract the domainType */
                            decIter.extractDomainType();
                            setupDecodeIterator();

                            /* extract streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* replace the streamId */
                            encIter.replaceStreamId(streamId + 1);
                            setupEncodeIterator();

                            /* extract the streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* extract the seqNum */
                            decIter.extractSeqNum();
                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the seqNum */
                            encIter.replaceSeqNum(seqNum + 1);
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            decIter.extractSeqNum();
                            if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();
                        }

                        msg.decode(decIter);

                        /* Check status */
                        if ((ackMask & AckMsgFlags.HAS_TEXT) > 0)
                        {
                            //
                        }

                        if ((ackMask & AckMsgFlags.HAS_NAK_CODE) > 0)
                        {
                            //
                        }

                        if ((ackMask & AckMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            //
                        }

                        /* Check msgKey */
                        if ((ackMask & AckMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            decodeMsgKey(decIter, ackMsg.msgKey());
                        }

                        /* Check Extended Header */
                        if ((ackMask & AckMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            //
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

    public void ackMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, ackMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, ackMasksCommon, ackMasksCommon.length, false);
        ackMsgTest(repeatCount);
    }

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
        // int decodeMsgKeyMask;

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
                    for (keyMasksIter = 0; keyMasksIter < ((postMask & PostMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize
                            : 1); ++keyMasksIter)
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
                            //
                        }

                        /* Set up user information */
                        postMsg.postUserInfo().userAddr(0xCCAA551DL);
                        postMsg.postUserInfo().userId(0xCCAA551DL);

                        /* Add Post Sequence Number */
                        if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(postMask + 0xFFFF)
                                    : (int)postMask;
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

                            msg.encode(encIter);
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            postMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, postMsg.extendedHeader(),
                                          postMsg.flags(), UpdateMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            msg.encodeComplete(encIter, true);
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            postMsg.containerType(DataTypes.NO_DATA);
                            msg.encode(encIter);
                        }

                        /* Decode msg */
                        fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
                        setupEncodeIterator();

                        if (g_changeData)
                        {

                            /* extract the msgClass */
                            decIter.extractMsgClass();
                            setupDecodeIterator();

                            /* extract the domainType */
                            decIter.extractDomainType();
                            setupDecodeIterator();

                            /* extract streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* replace the streamId */
                            encIter.replaceStreamId(streamId + 1);
                            setupEncodeIterator();

                            /* extract the streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* extract the seqNum */
                            decIter.extractSeqNum();
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the seqNum */
                            encIter.replaceSeqNum(seqNum + 1);
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            decIter.extractSeqNum();
                            if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* extract the postId */
                            decIter.extractPostId();
                            if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the postId */
                            encIter.replaceSeqNum(postId + 1);
                            setupEncodeIterator();

                            /* extract the new postId */
                            decIter.extractPostId();
                            if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();
                        }

                        msg.decode(decIter);

                        /* Check msgKey */
                        if ((postMask & PostMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            decodeMsgKey(decIter, postMsg.msgKey());
                        }

                        /* Check Post ID */
                        if ((postMask & PostMsgFlags.HAS_POST_ID) > 0)
                        {
                            //
                        }

                        /* Check Post User Info */
                        {
                            //
                        }

                        /* Check Item Sequence Number */
                        if ((postMask & PostMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            //
                        }

                        if ((postMask & PostMsgFlags.HAS_PART_NUM) > 0)
                        {
                            //
                        }

                        /* Check Post User Rights */
                        if ((postMask & PostMsgFlags.HAS_POST_USER_RIGHTS) > 0)
                        {
                            //
                        }

                        /* Check Permission Info */
                        if ((postMask & PostMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            //
                        }

                        /* Check Extended Header */
                        if ((postMask & PostMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            //
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

    public void postMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, postMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, postMasksCommon, postMasksCommon.length, false);
        postMsgTest(repeatCount);
    }

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
        // int decodeMsgKeyMask;

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
                    for (keyMasksIter = 0; keyMasksIter < ((genericMask & GenericMsgFlags.HAS_MSG_KEY) > 0 ? keyMasksSize
                            : 1); ++keyMasksIter)
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
                            seqNum = (extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF)
                                    : (int)genericMask;
                            genMsg.seqNum(seqNum);
                        }

                        if ((genericMask & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
                        {
                            genMsg.secondarySeqNum((extraAction & ExtraTestAction.FOUR_BYTE_SEQ) > 0 ? (int)(genericMask + 0xFFFF)
                                    : (int)genericMask);
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

                            msg.encode(encIter);
                        }
                        else if ((extraAction & ExtraTestAction.POST_PAYLOAD) > 0)
                        {
                            genMsg.containerType(g_dataFormat);

                            postEncodeMsg(encIter, msg, extraAction, genMsg.extendedHeader(),
                                          genMsg.flags(), GenericMsgFlags.HAS_EXTENDED_HEADER);

                            encodePayload(encIter);
                            encDataBuf.data(encDataBuf.data(), 0, encDataBuf.data().position());

                            msg.encodeComplete(encIter, true);
                        }
                        else
                        {
                            msg.encodedDataBody().clear();
                            genMsg.containerType(DataTypes.NO_DATA);
                            msg.encode(encIter);
                        }

                        /* Decode msg */
                        fixDecodeMsgKeyMask(msgKeyMask);

                        setupDecodeIterator();
                        setupEncodeIterator();

                        if (g_changeData)
                        {
                            /* extract the msgClass */
                            decIter.extractMsgClass();
                            setupDecodeIterator();

                            /* extract the domainType */
                            decIter.extractDomainType();
                            setupDecodeIterator();

                           /* extract streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* replace the streamId */
                            encIter.replaceStreamId(streamId + 1);
                            setupEncodeIterator();

                            /* extract the streamId */
                            decIter.extractStreamId();
                            setupDecodeIterator();

                            /* extract the seqNum */
                            decIter.extractSeqNum();
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();

                            /* replace the seqNum */
                            encIter.replaceSeqNum(seqNum + 1);
                            setupEncodeIterator();

                            /* extract the new seqNum */
                            decIter.extractSeqNum();
                            if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                            {
                                //
                            }
                            setupDecodeIterator();
                        }

                        msg.decode(decIter);

                        /* copy test */
                        /*
                         * TODO RRG { Msg copyMsg = CodecFactory.createMsg();
                         * Buffer copyBuffer = CodecFactory.createBuffer(); int
                         * msgSize = msg.sizeOfMsg(CopyMsgFlags.ALL_FLAGS);
                         * ByteBuffer copyBufData =
                         * ByteBuffer.allocate(msgSize);
                         * copyBuffer.data(copyBufData, 0, msgSize); copyMsg =
                         * msg.copy(CopyMsgFlags.ALL_FLAGS, 0, copyMsg,
                         * copyBuffer); GenericMsg copyGenericMsg =
                         * (GenericMsg)copyMsg; }
                         */

                        /* Check msgKey */
                        if ((genericMask & GenericMsgFlags.HAS_MSG_KEY) > 0)
                        {
                            decodeMsgKey(decIter, genMsg.msgKey());
                        }

                        /* Check Permission Info */
                        if ((genericMask & GenericMsgFlags.HAS_PERM_DATA) > 0)
                        {
                            //
                        }

                        /* Check Item Sequence Number */
                        if ((genericMask & GenericMsgFlags.HAS_SEQ_NUM) > 0)
                        {
                            //
                        }

                        /* Check Item Secondary Sequence Number */
                        if ((genericMask & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
                        {
                            //
                        }

                        if ((genericMask & GenericMsgFlags.HAS_PART_NUM) > 0)
                        {
                            //
                        }

                        /* Check Extended Header */
                        if ((genericMask & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0)
                        {
                            //
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

    public void genericMessageCommonTest()
    {
        testSetupCommon();
        masksSize = (int)Math.pow(2, genericMasksCommon.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, genericMasksCommon, genericMasksCommon.length, false);
        genericMsgTest(repeatCount);
    }

    public void genericMessageFullTest()
    {
        testSetupFull();
        masksSize = (int)Math.pow(2, genericMasksAll.length);
        masks = new int[masksSize];
        allocateFlagCombinations(masks, genericMasksAll, genericMasksAll.length, false);
        genericMsgTest(1);
    }

    public void run(String[] args)
    {
        long startAllTestsTime = System.currentTimeMillis();
        long startTime = System.currentTimeMillis();

        updateMessageCommonTest();
        System.out.println("finished updateMessageCommonTest ");

        long stopTime = System.currentTimeMillis();
        long diff = stopTime - startTime;
        System.out.println("updateMessageCommonTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        updateMessageFullTest();
        System.out.println("finished updateMessageFullTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("updateMessageFullTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        refreshMessageCommonTest();
        System.out.println("finished refreshMessageCommonTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("refreshMessageCommonTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        refreshMessageFullTest();
        System.out.println("finished refreshMessageFullTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("refreshMessageFullTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        requestMessageCommonTest();
        System.out.println("finished requestMessageCommonTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("requestMessageCommonTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        requestMessageFullTest();
        System.out.println("finished requestMessageFullTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("requestMessageFullTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        statusMessageCommonTest();
        System.out.println("finished statusMessageCommonTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("statusMessageCommonTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        statusMessageFullTest();
        System.out.println("finished statusMessageFullTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("statusMessageFullTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        closeMessageCommonTest();
        System.out.println("finished closeMessageCommonTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("closeMessageCommonTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        closeMessageFullTest();
        System.out.println("finished closeMessageFullTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("closeMessageFullTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        genericMessageCommonTest();
        System.out.println("finished genericMessageCommonTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("genericMessageCommonTest total time " + diff + " milliseconds.");
        startTime = System.currentTimeMillis();

        genericMessageFullTest();
        System.out.println("finished genericMessageFullTest ");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startTime;
        System.out.println("genericMessageFullTest total time " + diff + " milliseconds.");

        stopTime = System.currentTimeMillis();
        diff = stopTime - startAllTestsTime;
        System.out.println("various messages encoding/decoding total time " + diff
                + " milliseconds.");
    }

    public static void main(String[] args)
    {
        MessagePerf messagePerf = new MessagePerf();
        messagePerf.run(args);
    }
}