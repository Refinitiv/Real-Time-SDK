package com.refinitiv.ema.perftools.common;

import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.Time;

import java.nio.ByteBuffer;

public class ItemEncoder {

    private final Date dateTmp = CodecFactory.createDate();
    private final Time timeTmp = CodecFactory.createTime();
    private final DateTime dateTimeTmp = CodecFactory.createDateTime();

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
}
