/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;

import java.lang.Double;
import java.lang.Float;
import java.nio.ByteBuffer;

public class ItemEncoder {

    private final int TIM_TRK_1_FID = 3902; // Field TIM_TRK_1 is used to send update latency.
    private final int TIM_TRK_2_FID = 3903; // Field TIM_TRK_2 is used to send post latency.
    private final int TIM_TRK_3_FID = 3904; // Field TIM_TRK_3 is used to send generic msg latency.

    private final Date dateTmp = CodecFactory.createDate();
    private final Time timeTmp = CodecFactory.createTime();
    private final DateTime dateTimeTmp = CodecFactory.createDateTime();

    private FieldEntry _fieldEntry = EmaFactory.createFieldEntry();

    private XmlMsgData _xmlMsgData;

    public ItemEncoder() {}

    public ItemEncoder(XmlMsgData xmlMsgData) {
        _xmlMsgData = xmlMsgData;
    }

    /* Creates field entry data from file data. */
    public FieldEntry loadPrimitive(MarketField field) {
        final int fieldId = field.fieldId();
        final String value = field.value();
        final FieldEntry fieldEntry = EmaFactory.createFieldEntry();
        double doubleValue;
        float floatValue;
        long intValue;
        switch (field.loadType()) {
            case DataType.DataTypes.INT:
                if (!field.isBlank()) {
                    intValue = Long.parseLong(value);
                    fieldEntry.intValue(fieldId, intValue);
                } else {
                    fieldEntry.codeInt(fieldId);
                }
                break;
            case DataType.DataTypes.UINT:
                if (!field.isBlank()) {
                    intValue = Long.parseLong(value);
                    fieldEntry.uintValue(fieldId, intValue);
                    fieldEntry.load();
                } else {
                    fieldEntry.codeUInt(fieldId);
                }
                break;
            case DataType.DataTypes.FLOAT:
                if (!field.isBlank()) {
                    floatValue = Float.parseFloat(value);
                    fieldEntry.floatValue(fieldId, floatValue);
                } else {
                    fieldEntry.codeFloat(fieldId);
                }
                break;
            case DataType.DataTypes.DOUBLE:
                if (!field.isBlank()) {
                    doubleValue = Double.parseDouble(value);
                    fieldEntry.doubleValue(fieldId, doubleValue);
                } else {
                    fieldEntry.codeDouble(fieldId);
                }
                break;
            case DataType.DataTypes.REAL:
                if (!field.isBlank()) {
                    doubleValue = Double.parseDouble(value);
                    fieldEntry.realFromDouble(fieldId, doubleValue);
                } else {
                    fieldEntry.codeReal(fieldId);
                }
                break;
            case DataType.DataTypes.DATE:
                if (!field.isBlank()) {
                    dateTmp.clear();
                    dateTmp.value(value);
                    fieldEntry.date(fieldId, dateTmp.year(), dateTmp.month(), dateTmp.day());
                } else {
                    fieldEntry.codeDate(fieldId);
                }
                break;
            case DataType.DataTypes.TIME:
                if (!field.isBlank()) {
                    timeTmp.clear();
                    timeTmp.value(value);
                    fieldEntry.time(fieldId,
                            timeTmp.hour(), timeTmp.minute(), timeTmp.second(), timeTmp.millisecond(), timeTmp.microsecond(), timeTmp.nanosecond());
                } else {
                    fieldEntry.codeTime(fieldId);
                }
                break;
            case DataType.DataTypes.DATETIME:
                if (!field.isBlank()) {
                    dateTimeTmp.clear();
                    dateTimeTmp.value(value);
                    fieldEntry.dateTime(fieldId,
                            dateTimeTmp.year(), dateTimeTmp.month(), dateTimeTmp.day(), dateTimeTmp.hour(), dateTimeTmp.minute(), dateTimeTmp.second(),
                            dateTimeTmp.millisecond(), dateTimeTmp.microsecond(), dateTimeTmp.nanosecond());
                } else {
                    fieldEntry.codeDateTime(fieldId);
                }
                break;
            case DataType.DataTypes.ENUM:
                if (!field.isBlank()) {
                    intValue = Integer.parseInt(value);
                    fieldEntry.enumValue(fieldId, (int) intValue);
                } else {
                    fieldEntry.codeEnum(fieldId);
                }
                break;
            case DataType.DataTypes.BUFFER:
                if (!field.isBlank()) {
                    fieldEntry.buffer(fieldId, ByteBuffer.wrap(value.getBytes()));
                } else {
                    fieldEntry.codeBuffer(fieldId);
                }
                break;
            case DataType.DataTypes.ASCII:
                if (!field.isBlank()) {
                    fieldEntry.ascii(fieldId, value);
                } else {
                    fieldEntry.codeAscii(fieldId);
                }
                break;
            case DataType.DataTypes.UTF8:
                if (!field.isBlank()) {
                    fieldEntry.utf8(fieldId, value);
                } else {
                    fieldEntry.codeUtf8(fieldId);
                }
                break;
            case DataType.DataTypes.RMTES:
                if (!field.isBlank()) {
                    fieldEntry.rmtes(fieldId, ByteBuffer.wrap(value.getBytes()));
                } else {
                    fieldEntry.codeRmtes(fieldId);
                }
                break;
            case DataType.DataTypes.QOS:
                final QosWrapper qos = field.qos();
                fieldEntry.qos(fieldId, qos.timeliness(), qos.rate());
                break;
            case DataType.DataTypes.STATE:
                final StateWrapper state = field.state();
                fieldEntry.state(fieldId, state.streamState(), state.dataState(), state.statusCode(), state.statusText());
                break;
            default:
                break;
        }
        return fieldEntry;
    }

    private FieldList createPayload(MarketPriceMsg mpMsg, long encodeStartTime, int timeFieldId) {
        FieldList fieldList = EmaFactory.createFieldList();
        for (int i = 0; i < mpMsg.fieldEntryCount(); i++) {
            FieldEntry fieldEntry = loadPrimitive(mpMsg.fieldEntries()[i]);
            fieldList.add(fieldEntry);
        }

        if (timeFieldId == 0) {
            fieldList.add(EmaFactory.createFieldEntry().uintValue(TIM_TRK_1_FID, 0L));
            fieldList.add(EmaFactory.createFieldEntry().uintValue(TIM_TRK_2_FID, 0L));
            fieldList.add(EmaFactory.createFieldEntry().uintValue(TIM_TRK_3_FID, 0L));
        } else if (encodeStartTime > 0) {
            fieldList.add(EmaFactory.createFieldEntry().uintValue(timeFieldId, encodeStartTime));
        }

        return fieldList;
    }

    public MarketPriceMsg nextGenMsg(MarketPriceItem mpItem)
    {
        int mpItemIndex = mpItem.iMsg();
        MarketPriceMsg mpMsg = _xmlMsgData.marketPriceGenMsgs()[mpItemIndex++];

        if (mpItemIndex == _xmlMsgData.marketPriceGenMsgCount())
            mpItemIndex = 0;

        mpItem.iMsg(mpItemIndex);

        return mpMsg;
    }

    public MarketPriceMsg nextPostMsg(MarketPriceItem mpItem)
    {
        int mpItemIndex = mpItem.iMsg();
        MarketPriceMsg mpMsg = _xmlMsgData.marketPricePostMsgs()[mpItemIndex++];

        if (mpItemIndex == _xmlMsgData.marketPricePostMsgCount())
            mpItemIndex = 0;

        mpItem.iMsg(mpItemIndex);

        return mpMsg;
    }

    public int populateGenericMsg(GenericMsg genMsg, ItemInfo itemInfo, long encodeStartTime) {

        genMsg.streamId(itemInfo.itemId());
        genMsg.domainType(itemInfo.attributes().domainType());
        switch(itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                genMsg.payload(createPayload(nextGenMsg(itemInfo.marketPriceItem()), encodeStartTime, TIM_TRK_3_FID));
                break;
            default:
                return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    public int populatePostMsg(PostMsg postMsg, ItemInfo itemInfo, PostUserInfo postUserInfo, long encodeStartTime) {

        postMsg.streamId(itemInfo.itemId());
        switch(itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
                nestedUpdateMsg.payload(createPayload(nextPostMsg(itemInfo.marketPriceItem()), encodeStartTime, TIM_TRK_2_FID))
                        .publisherId(postUserInfo.userId, postUserInfo.userAddr);
                postMsg.publisherId(postUserInfo.userId, postUserInfo.userAddr).payload(nestedUpdateMsg).complete(true);
                break;
            default:
                return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }
}
