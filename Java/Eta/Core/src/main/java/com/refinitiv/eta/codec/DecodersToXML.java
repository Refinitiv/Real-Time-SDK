/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.ElementSetDef;
import com.refinitiv.eta.codec.ElementSetDefEntry;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.FieldSetDef;
import com.refinitiv.eta.codec.FieldSetDefEntry;
import com.refinitiv.eta.codec.FilterEntry;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.FilterList;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.LocalElementSetDefDb;
import com.refinitiv.eta.codec.LocalFieldSetDefDb;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.NakCodes;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.Qos;
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
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;

class DecodersToXML
{
    private static int indents;

    private static String encodeindents()
    {
        StringBuilder xmlString = new StringBuilder();

        for (int i = 0; i < indents; i++)
        {
            xmlString.append("    ");
        }

        return xmlString.toString();
    }

    static String decodeDataTypeToXML(int dataType, Buffer buffer, DataDictionary dictionary, Object setDb, DecodeIterator iter)
    {
        StringBuilder xmlString = new StringBuilder();
        int ret;

        if (indents == 0)
        {
            xmlString.append("\n<!-- rwfMajorVer=\"" + iter.majorVersion() + "\" rwfMinorVer=\"" + iter.minorVersion() + "\" -->\n");
        }

        switch (dataType)
        {
            case DataTypes.INT:
                Int i64 = CodecFactory.createInt();
                ret = Decoders.decodeInt(iter, i64);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpInt(i64));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.UINT:
                UInt u64 = CodecFactory.createUInt();
                ret = Decoders.decodeUInt(iter, u64);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpUInt(u64));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.FLOAT:
                Float r32 = CodecFactory.createFloat();
                ret = Decoders.decodeFloat(iter, r32);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpDouble(r32));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.DOUBLE:
                Double r64 = CodecFactory.createDouble();
                ret = Decoders.decodeDouble(iter, r64);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpDouble(r64));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.ENUM:
                Enum enumVal = CodecFactory.createEnum();
                Int iTemp = CodecFactory.createInt();
                ret = Decoders.decodeEnum(iter, enumVal);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    iTemp.value(enumVal.toInt());
                    xmlString.append(xmlDumpInt(iTemp));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.QOS:
                Qos qos = CodecFactory.createQos();
                ret = Decoders.decodeQos(iter, qos);
                if (ret == CodecReturnCodes.BLANK_DATA) {
                    xmlString.append(" data=\"\"");
                } else if (ret >= CodecReturnCodes.SUCCESS) {
                    xmlString.append(xmlDumpQos(qos));
                }
                break;
            case DataTypes.STATE:
                State state = CodecFactory.createState();
                ret = Decoders.decodeState(iter, state);
                if (ret == CodecReturnCodes.BLANK_DATA) {
                    xmlString.append(" data=\"\"");
                } else if (ret >= CodecReturnCodes.SUCCESS) {
                    xmlString.append(xmlDumpState(state));
                }
                break;
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                Buffer out = CodecFactory.createBuffer();
                ret = Decoders.decodeBuffer(iter, out);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpString(out, true));
                    xmlString.append("\"");
                }
                else
                {
                    xmlString.append("Error occurred while decoding dataType " + dataType + ", CodecReturnCode=" + ret);
                    return xmlString.toString();
                }
                break;
            case DataTypes.DATE:
                DateTime dtDate = CodecFactory.createDateTime();
                ret = Decoders.decodeDate(iter, dtDate.date());
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpDate(dtDate.date()));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.TIME:
                DateTime dtTime = CodecFactory.createDateTime();
                ret = Decoders.decodeTime(iter, dtTime.time());
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpTime(dtTime.time()));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.DATETIME:
                DateTime dtDatetime = CodecFactory.createDateTime();
                ret = Decoders.decodeDateTime(iter, dtDatetime);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpDateTime(dtDatetime));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.REAL:
                Real oReal64 = CodecFactory.createReal();
                ret = Decoders.decodeReal(iter, oReal64);
                if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    xmlString.append(" data=\"\"");
                }
                else if (ret >= CodecReturnCodes.SUCCESS)
                {
                    xmlString.append(" data=\"");
                    xmlString.append(xmlDumpReal(oReal64));
                    xmlString.append("\"");
                }
                break;
            case DataTypes.ELEMENT_LIST:
                xmlString.append(decodeElementListToXML(iter, dictionary, (LocalElementSetDefDb)setDb, true));
                break;
            case DataTypes.ARRAY:
                xmlString.append(decodeArrayToXML(iter, dictionary));
                break;
            case DataTypes.FIELD_LIST:
                xmlString.append(decodeFieldListToXML(iter, dictionary, (LocalFieldSetDefDb)setDb, true));
                break;
            case DataTypes.FILTER_LIST:
                xmlString.append(decodeFilterListToXML(iter, dictionary, true));
                break;
            case DataTypes.VECTOR:
                xmlString.append(decodeVectorToXML(iter, dictionary, true));
                break;
            case DataTypes.MAP:
                xmlString.append(decodeMapToXML(iter, dictionary, true));
                break;
            case DataTypes.SERIES:
                xmlString.append(decodeSeriesToXML(iter, dictionary, true));
                break;
            case DataTypes.ANSI_PAGE:
                xmlString.append(decodeAnsiPageToXML(buffer, dictionary));
                break;
            case DataTypes.MSG:
                xmlString.append(decodeRwfMsgToXML(iter, dictionary));
                break;
            case DataTypes.NO_DATA:
                xmlString.append("");
                break;
            case DataTypes.OPAQUE:
                xmlString.append(dumpOpaqueToXML(buffer, dictionary));
                break;
            case DataTypes.JSON:
                xmlString.append(dumpJSONToXML(buffer, dictionary));
                break;
            default:
                xmlString.append(dumpOpaqueToXML(buffer, dictionary));
        }

        return xmlString.toString();
    }

    private static String dumpOpaqueToXML(Buffer buffer, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        indents++;

        xmlString.append("<opaque data=\"");
        xmlString.append(xmlDumpHexBuffer(buffer));
        xmlString.append("\" />\n");

        indents--;

        return xmlString.toString();
    }

    private static String dumpJSONToXML(Buffer buffer, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        indents++;

        xmlString.append("<json data=\"");
        xmlString.append(xmlDumpString(buffer, true));
        xmlString.append("\" />\n");

        indents--;

        return xmlString.toString();
    }

    private static String decodeRwfMsgToXML(DecodeIterator iter, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();
        int ret;
        Msg msg = CodecFactory.createMsg();
        DecodeIteratorImpl iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

        // copy iterator contents
        copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);

        ret = msg.decode(iterCopy);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            xmlString.append("Error occurred while decoding Msg, CodecReturnCode=" + ret);
            return xmlString.toString();
        }

        String tagName = MsgClasses.toString(msg.msgClass());

        xmlString.append(xmlDumpMsgBegin(msg, tagName));

        xmlString.append(decodeMsgClassToXML(msg, iterCopy, dictionary));

        xmlString.append(xmlDumpDataBodyBegin());

        xmlString.append(decodeDataTypeToXML(msg.containerType(), msg.encodedDataBody(), dictionary, null, iterCopy));

        xmlString.append(xmlDumpDataBodyEnd());

        xmlString.append(xmlDumpMsgEnd(tagName));

        return xmlString.toString();
    }

    private static String xmlDumpMsgEnd(String tagName)
    {
        StringBuilder xmlString = new StringBuilder();

        indents--;
        xmlString.append(encodeindents());
        xmlString.append("</" + tagName + ">\n");

        return xmlString.toString();
    }

    private static String xmlDumpDataBodyEnd()
    {
        indents--;

        return (encodeindents() + "</dataBody>\n");
    }

    private static String xmlDumpDataBodyBegin()
    {
        String ret = encodeindents() + "<dataBody>\n";

        indents++;

        return ret;
    }

    private static String decodeMsgClassToXML(Msg msg, DecodeIterator iter, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();

        switch (msg.msgClass())
        {
            case MsgClasses.UPDATE:
                UpdateMsg updateMsg = (UpdateMsg)msg;

                if (updateMsg.checkHasMsgKey())
                {
                    xmlString.append(decodeKeysToXML(updateMsg, iter, dictionary));
                }
                if (updateMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(updateMsg.extendedHeader()));
                }
                break;
            case MsgClasses.REFRESH:
                RefreshMsg refreshMsg = (RefreshMsg)msg;

                if (refreshMsg.checkHasMsgKey())
                {
                    xmlString.append(decodeKeysToXML(refreshMsg, iter, dictionary));
                }
                if (refreshMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(refreshMsg.extendedHeader()));
                }
                break;
            case MsgClasses.REQUEST:
                RequestMsg requestMsg = (RequestMsg)msg;

                xmlString.append(decodeKeysToXML(requestMsg, iter, dictionary));

                if (requestMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(requestMsg.extendedHeader()));
                }
                break;
            case MsgClasses.GENERIC:
                GenericMsg genericMsg = (GenericMsg)msg;

                if (genericMsg.checkHasMsgKey())
                    xmlString.append(decodeKeysToXML(genericMsg, iter, dictionary));

                if (genericMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(genericMsg.extendedHeader()));
                }
                break;
            case MsgClasses.POST:
                PostMsg postMsg = (PostMsg)msg;

                if (postMsg.checkHasMsgKey())
                {
                    xmlString.append(decodeKeysToXML(postMsg, iter, dictionary));
                }

                if (postMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(postMsg.extendedHeader()));
                }
                break;
            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg;

                if (statusMsg.checkHasMsgKey())
                {
                    xmlString.append(decodeKeysToXML(statusMsg, iter, dictionary));
                }

                if (statusMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(statusMsg.extendedHeader()));
                }
                break;
            case MsgClasses.CLOSE:
                CloseMsg closeMsg = (CloseMsg)msg;

                if (closeMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(closeMsg.extendedHeader()));
                }
                break;
            case MsgClasses.ACK:
                AckMsg ackMsg = (AckMsg)msg;

                if (ackMsg.checkHasMsgKey())
                {
                    xmlString.append(decodeKeysToXML(ackMsg, iter, dictionary));
                }

                if (ackMsg.checkHasExtendedHdr())
                {
                    xmlString.append(xmlDumpExtendedHeader(ackMsg.extendedHeader()));
                }
                break;
            default:
                xmlString.append("decodeMsgClassToXML() failed to decode unknown message class");
                break;
        }

        return xmlString.toString();
    }

    private static String xmlDumpExtendedHeader(Buffer extendedHeader)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        xmlString.append("<extendedHeader data=\"");
        xmlString.append(xmlDumpHexBuffer(extendedHeader));
        xmlString.append("\"/>\n");

        return xmlString.toString();
    }

    private static String decodeKeysToXML(Msg msg, DecodeIterator iter, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(xmlDumpKeyBegin(msg.msgKey()));
        if (msg.msgKey().checkHasAttrib())
        {
            xmlString.append(decodeKeyOpaque(msg, iter, dictionary));
            xmlString.append(xmlDumpKeyEnd());
        }

        return xmlString.toString();
    }

    private static String xmlDumpKeyEnd()
    {
        indents--;

        return (encodeindents() + "</key>\n");
    }

    private static String decodeKeyOpaque(Msg msg, DecodeIterator iter, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();
        int ret;
        int attribContainerType = msg.msgKey().attribContainerType();

        xmlString.append("<attrib>\n");

        indents++;

        ret = msg.decodeKeyAttrib(iter, msg.msgKey());
        if (ret < CodecReturnCodes.SUCCESS)
        {
            xmlString.append("Error occurred while decoding MsgKey, CodecReturnCode=" + ret);
            return xmlString.toString();
        }

        switch (attribContainerType)
        {
            case DataTypes.OPAQUE:
                Buffer opaqueBufferValue = CodecFactory.createBuffer();
                Decoders.decodeBuffer(iter, opaqueBufferValue);
                xmlString.append(dumpOpaqueToXML(opaqueBufferValue, dictionary));
                break;
            case DataTypes.FILTER_LIST:
                xmlString.append(decodeFilterListToXML(iter, dictionary, false));
                break;
            case DataTypes.ELEMENT_LIST:
                xmlString.append(decodeElementListToXML(iter, dictionary, null, false));
                break;
            case DataTypes.FIELD_LIST:
                xmlString.append(decodeFieldListToXML(iter, dictionary, null, false));
                break;
            case DataTypes.SERIES:
                xmlString.append(decodeSeriesToXML(iter, dictionary, false));
                break;
            case DataTypes.VECTOR:
                xmlString.append(decodeVectorToXML(iter, dictionary, false));
                break;
            case DataTypes.MAP:
                xmlString.append(decodeMapToXML(iter, dictionary, false));
                break;
            case DataTypes.XML:
                xmlString.append(DataTypes.toString(msg.msgKey().attribContainerType()) + "\n");
                xmlString.append(msg.msgKey().encodedAttrib().toString());
                xmlString.append("\n");
                break;
            case DataTypes.JSON:
                xmlString.append(dumpJSONToXML(msg.msgKey().encodedAttrib(), dictionary));
                break;
            case DataTypes.NO_DATA:
                xmlString.append("");
                break;
            default:
                xmlString.append(encodeindents());
                xmlString.append("Unknown data\n");
                break;
        }

        indents--;
        xmlString.append(encodeindents());
        xmlString.append("</attrib>\n");

        return xmlString.toString();
    }

    private static String xmlDumpKeyBegin(MsgKey msgKey)
    {
        boolean firstFlag = true;
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        xmlString.append("<key");

        // print out flags
        xmlString.append(" flags=\"0x" + String.format("%02X", msgKey.flags()));

        if (msgKey.flags() != 0)
            xmlString.append(" (");
        if (msgKey.checkHasServiceId())
        {
            xmlString.append("HAS_SERVICE_ID");
            firstFlag = false;
        }
        if (msgKey.checkHasName())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_NAME");
        }
        if (msgKey.checkHasNameType())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_NAME_TYPE");
        }
        if (msgKey.checkHasFilter())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_FILTER");
        }
        if (msgKey.checkHasIdentifier())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_IDENTIFIER");
        }
        if (msgKey.checkHasAttrib())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_ATTRIB");
        }

        if (msgKey.flags() != 0)
            xmlString.append(")");
        xmlString.append("\"");

        if (msgKey.checkHasServiceId())
            xmlString.append(" serviceId=\"" + msgKey.serviceId() + "\"");

        if (msgKey.checkHasName())
        {
            xmlString.append(" name=\"");
            xmlString.append(xmlDumpBuffer(msgKey.name()));
            xmlString.append("\"");
        }

        if (msgKey.checkHasNameType())
        {
            xmlString.append(" nameType=\"" + msgKey.nameType() + "\"");
        }

        if (msgKey.checkHasFilter())
        {
            xmlString.append(" filter=\"" + msgKey.filter() + "\"");
        }

        if (msgKey.checkHasIdentifier())
            xmlString.append(" identifier=\"" + msgKey.identifier() + "\"");

        if (msgKey.checkHasAttrib())
        {
            xmlString.append(" attribContainerType=\"");
            xmlString.append(xmlDumpDataType(msgKey.attribContainerType()));
            xmlString.append("\">\n");
            indents++;
            xmlString.append(encodeindents());
        }
        else
            xmlString.append("/>\n");

        return xmlString.toString();
    }

    private static String decodeAnsiPageToXML(Buffer buffer, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        indents++;

        xmlString.append("<ansiPage data=\"");
        xmlString.append(xmlDumpHexBuffer(buffer));
        xmlString.append("\"/>\n");

        indents--;

        return xmlString.toString();
    }

    private static String decodeSeriesToXML(DecodeIterator iter, DataDictionary dictionary, boolean copyIterator)
    {
        StringBuilder xmlString = new StringBuilder();
        Series series = CodecFactory.createSeries();
        SeriesEntry row = CodecFactory.createSeriesEntry();
        int ret = 0;
        Object setDb = null;

        DecodeIteratorImpl iterCopy;
        if (copyIterator)
        {
            iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

            // copy iterator contents
            copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);
        }
        else
        {
            iterCopy = (DecodeIteratorImpl)iter;
        }

        ret = series.decode(iterCopy);
        if (ret == CodecReturnCodes.NO_DATA || ret < CodecReturnCodes.SUCCESS)
            return "";

        xmlString.append(xmlDumpSeriesBegin(series));

        if (series.checkHasSetDefs())
        {
            if (series.containerType() == DataTypes.FIELD_LIST)
            {
                LocalFieldSetDefDb flListSetDb = CodecFactory.createLocalFieldSetDefDb();
                flListSetDb.clear();
                ret = Decoders.decodeLocalFieldSetDefDb(iterCopy, flListSetDb);
                if (ret >= CodecReturnCodes.SUCCESS)
                {
                    setDb = flListSetDb;
                    xmlString.append(xmlDumpLocalFieldSetDefDb(flListSetDb));
                }
                else
                {
                    xmlString.append("Error occurred while decoding FieldList SetDef contained in a Series, CodecReturnCode=" + ret);
                    return xmlString.toString();
                }
            }
            else
            {
                LocalElementSetDefDb elListSetDb = CodecFactory.createLocalElementSetDefDb();
                elListSetDb.clear();
                ret = Decoders.decodeLocalElementSetDefDb(iterCopy, elListSetDb);
                if (ret >= CodecReturnCodes.SUCCESS)
                {
                    setDb = elListSetDb;
                    xmlString.append(xmlDumpLocalElementSetDefDb(elListSetDb));
                }
                else
                {
                    xmlString.append("Error occurred while decoding a SetDef contained in a Series, CodecReturnCode=" + ret);
                    return xmlString.toString();
                }
            }
        }

        /* dump summary data */
        if (series.checkHasSummaryData())
            xmlString.append(decodeSummaryData(iterCopy, series.containerType(), series.encodedSummaryData(),
                    iterCopy.majorVersion(), iterCopy.minorVersion(), dictionary, setDb));

        if (Decoders.getItemCount(iterCopy) == 0)
        {
            xmlString.append(xmlDumpSeriesEnd());
            return xmlString.toString();
        }

        while ((ret = row.decode(iterCopy)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
            {
                xmlString.append("Error occurred while decoding SeriesEntry, CodecReturnCode=" + ret);
                return xmlString.toString();
            }

            xmlString.append(xmlDumpSeriesRowBegin(row));
            xmlString.append(decodeDataTypeToXML(series.containerType(), row.encodedData(), dictionary, setDb, iterCopy));
            xmlString.append(xmlDumpSeriesRowEnd());
        }

        xmlString.append(xmlDumpSeriesEnd());

        return xmlString.toString();
    }

    private static String decodeMapToXML(DecodeIterator iter, DataDictionary dictionary, boolean copyIterator)
    {
        StringBuilder xmlString = new StringBuilder();
        Map map = CodecFactory.createMap();
        int ret = 0;
        Object setDb = null;
        MapEntry mapEntry = CodecFactory.createMapEntry();
        Object mapKeyData = null;

        DecodeIteratorImpl iterCopy;
        if (copyIterator)
        {
            iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

            // copy iterator contents
            copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);
        }
        else
        {
            iterCopy = (DecodeIteratorImpl)iter;
        }

        ret = map.decode(iterCopy);
        if (ret == CodecReturnCodes.NO_DATA || ret < CodecReturnCodes.SUCCESS)
            return "";

        xmlString.append(xmlDumpMapBegin(map));

        if (map.checkHasSetDefs())
        {
            if (map.containerType() == DataTypes.FIELD_LIST)
            {
                LocalFieldSetDefDb flListSetDb = CodecFactory.createLocalFieldSetDefDb();
                flListSetDb.clear();
                ret = Decoders.decodeLocalFieldSetDefDb(iterCopy, flListSetDb);
                if (ret >= CodecReturnCodes.SUCCESS)
                {
                    setDb = flListSetDb;
                    xmlString.append(xmlDumpLocalFieldSetDefDb(flListSetDb));
                }
                else
                {
                    xmlString.append("Error occurred while decoding FieldList SetDef contained in a Map, CodecReturnCode=" + ret);
                    return xmlString.toString();
                }
            }
            else
            {
                LocalElementSetDefDb elListSetDb = CodecFactory.createLocalElementSetDefDb();
                elListSetDb.clear();
                ret = Decoders.decodeLocalElementSetDefDb(iterCopy, elListSetDb);
                if (ret >= CodecReturnCodes.SUCCESS)
                {
                    setDb = elListSetDb;
                    xmlString.append(xmlDumpLocalElementSetDefDb(elListSetDb));
                }
                else
                {
                    xmlString.append("Error occurred while decoding a SetDef contained in a Map, CodecReturnCode=" + ret);
                    return xmlString.toString();
                }
            }
        }

        /* dump summary data */
        if (map.checkHasSummaryData())
            xmlString.append(decodeSummaryData(iterCopy, map.containerType(), map.encodedSummaryData(),
                    iterCopy.majorVersion(), iterCopy.minorVersion(), dictionary, setDb));

        mapEntry.clear();
        mapKeyData = createKeyData(map.keyPrimitiveType());
        while ((ret = mapEntry.decode(iterCopy, mapKeyData)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
            {
                xmlString.append("Error occurred while decoding MapEntry, CodecReturnCode=" + ret);
                return xmlString.toString();
            }

            xmlString.append(xmlDumpMapEntryBegin(map.keyPrimitiveType(), mapEntry, mapKeyData));
            xmlString.append(decodeDataTypeToXML(map.containerType(), mapEntry.encodedData(), dictionary, setDb, iterCopy));
            xmlString.append(xmlDumpMapEntryEnd());

            mapEntry.clear();
        }

        xmlString.append(xmlDumpMapEnd());

        return xmlString.toString();
    }

    private static String decodeVectorToXML(DecodeIterator iter, DataDictionary dictionary, boolean copyIterator)
    {
        StringBuilder xmlString = new StringBuilder();
        Vector vec = CodecFactory.createVector();
        int ret = 0;
        Object setDb = null;
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();

        DecodeIteratorImpl iterCopy;
        if (copyIterator)
        {
            iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

            // copy iterator contents
            copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);
        }
        else
        {
            iterCopy = (DecodeIteratorImpl)iter;
        }

        ret = vec.decode(iterCopy);
        if (ret == CodecReturnCodes.NO_DATA || ret < CodecReturnCodes.SUCCESS)
            return "";

        xmlString.append(xmlDumpVectorBegin(vec));

        if (vec.checkHasSetDefs())
        {
            if (vec.containerType() == DataTypes.FIELD_LIST)
            {
                LocalFieldSetDefDb flListSetDb = CodecFactory.createLocalFieldSetDefDb();
                flListSetDb.clear();
                ret = Decoders.decodeLocalFieldSetDefDb(iterCopy, flListSetDb);
                if (ret >= CodecReturnCodes.SUCCESS)
                {
                    setDb = flListSetDb;
                    xmlString.append(xmlDumpLocalFieldSetDefDb(flListSetDb));
                }
                else
                {
                    xmlString.append("Error occurred while decoding Vector SetDef contained in a Series, CodecReturnCode=" + ret);
                    return xmlString.toString();
                }
            }
            else
            {
                LocalElementSetDefDb elListSetDb = CodecFactory.createLocalElementSetDefDb();
                elListSetDb.clear();
                ret = Decoders.decodeLocalElementSetDefDb(iterCopy, elListSetDb);
                if (ret >= CodecReturnCodes.SUCCESS)
                {
                    setDb = elListSetDb;
                    xmlString.append(xmlDumpLocalElementSetDefDb(elListSetDb));
                }
                else
                {
                    xmlString.append("Error occurred while decoding a SetDef contained in a Series, CodecReturnCode=" + ret);
                    return xmlString.toString();
                }
            }
        }

        /* dump summary data */
        if (vec.checkHasSummaryData())
            xmlString.append(decodeSummaryData(iterCopy, vec.containerType(), vec.encodedSummaryData(),
                    iterCopy.majorVersion(), iterCopy.minorVersion(), dictionary, setDb));

        vectorEntry.clear();
        while ((ret = vectorEntry.decode(iterCopy)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
            {
                xmlString.append("Error occurred while decoding VectorEntry, CodecReturnCode=" + ret);
                return xmlString.toString();
            }

            xmlString.append(xmlDumpVectorEntryBegin(vectorEntry));
            xmlString.append(decodeDataTypeToXML(vec.containerType(), vectorEntry.encodedData(), dictionary, setDb, iterCopy));
            xmlString.append(xmlDumpVectorEntryEnd());

            vectorEntry.clear();
        }

        xmlString.append(xmlDumpVectorEnd());

        return xmlString.toString();
    }

    private static String decodeFilterListToXML(DecodeIterator iter, DataDictionary dictionary, boolean copyIterator)
    {
        StringBuilder xmlString = new StringBuilder();
        FilterList fList = CodecFactory.createFilterList();
        FilterEntry filterItem = CodecFactory.createFilterEntry();
        int ret = 0;

        DecodeIteratorImpl iterCopy;
        if (copyIterator)
        {
            iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

            // copy iterator contents
            copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);
        }
        else
        {
            iterCopy = (DecodeIteratorImpl)iter;
        }

        ret = fList.decode(iterCopy);
        if (ret == CodecReturnCodes.NO_DATA || ret < CodecReturnCodes.SUCCESS)
            return "";

        xmlString.append(xmlDumpFilterListBegin(fList));

        while ((ret = filterItem.decode(iterCopy)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
            {
                xmlString.append("Error occurred while decoding FilterEntry, CodecReturnCode=" + ret);
                return xmlString.toString();
            }

            xmlString.append(xmlDumpFilterItemBegin(filterItem));
            xmlString.append(decodeDataTypeToXML(filterItem.containerType(), filterItem.encodedData(), dictionary, null, iterCopy));
            xmlString.append(xmlDumpFilterItemEnd());
        }

        xmlString.append(xmlDumpFilterListEnd());

        return xmlString.toString();
    }

    private static String decodeFieldListToXML(DecodeIterator iter, DataDictionary dictionary,
                                               LocalFieldSetDefDb setDb, boolean copyIterator)
    {
        StringBuilder xmlString = new StringBuilder();
        int ret = 0;
        int dataType = DataTypes.UNKNOWN;
        FieldList fList = CodecFactory.createFieldList();
        FieldEntry field = CodecFactory.createFieldEntry();

        DecodeIteratorImpl iterCopy;
        if (copyIterator)
        {
            iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

            // copy iterator contents
            copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);
        }
        else
        {
            iterCopy = (DecodeIteratorImpl)iter;
        }

        ret = fList.decode(iterCopy, setDb);
        if (ret == CodecReturnCodes.NO_DATA || ret < CodecReturnCodes.SUCCESS)
            return "";

        xmlString.append(xmlDumpFieldListBegin(fList));

        while ((ret = field.decode(iterCopy)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
            {
                xmlString.append("Error occurred while decoding FieldEntry, CodecReturnCode=" + ret);
                return xmlString.toString();
            }

            if (dictionary == null || dictionary.entry(field.fieldId()) == null)
                dataType = field.dataType();
            else
                dataType = dictionary.entry(field.fieldId()).rwfType();

            xmlString.append(xmlDumpFieldBegin(field, dataType));

            if (Encoders.validAggregateDataType(dataType) || dataType == DataTypes.ARRAY)
                xmlString.append(">\n");

            if (dataType != DataTypes.UNKNOWN)
            {
                xmlString.append(decodeDataTypeToXML(dataType, field.encodedData(), dictionary, setDb, iterCopy));
            }
            else
            {
                xmlString.append(" data=\"");
                xmlString.append(xmlDumpHexBuffer(field.encodedData()));
                xmlString.append("\"");
            }
            if (Encoders.validAggregateDataType(dataType) || dataType == DataTypes.ARRAY)
                xmlString.append(xmlDumpFieldEnd());
            else
                xmlString.append(xmlDumpEndNoTag());
        }

        xmlString.append(xmlDumpFieldListEnd());

        return xmlString.toString();
    }

    private static String decodeArrayToXML(DecodeIterator iter, DataDictionary dictionary)
    {
        StringBuilder xmlString = new StringBuilder();
        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
        int ret = 0;
        DecodeIteratorImpl iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

        // copy iterator contents
        copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);

        ret = array.decode(iterCopy);
        if (ret == CodecReturnCodes.NO_DATA || ret < CodecReturnCodes.SUCCESS || ret == CodecReturnCodes.BLANK_DATA) {
            return "";
        }

        xmlString.append(xmlDumpArrayBegin(array));

        while ((ret = arrayEntry.decode(iterCopy)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
            {
                xmlString.append("Error occurred while decoding ArrayEntry, CodecReturnCode=" + ret);
                return xmlString.toString();
            }

            xmlString.append(xmlDumpArrayItemBegin());

            Buffer arrayItem = CodecFactory.createBuffer();
            xmlString.append(decodeDataTypeToXML(array.primitiveType(), arrayItem, dictionary, null, iterCopy));

            xmlString.append(xmlDumpArrayItemEnd());
        }

        xmlString.append(xmlDumpArrayEnd());

        return xmlString.toString();
    }

    private static String decodeElementListToXML(DecodeIterator iter, DataDictionary dictionary,
                                                 LocalElementSetDefDb setDb, boolean copyIterator)
    {
        StringBuilder xmlString = new StringBuilder();
        int ret = 0;
        ElementList eList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();

        DecodeIteratorImpl iterCopy;
        if (copyIterator)
        {
            iterCopy = (DecodeIteratorImpl)CodecFactory.createDecodeIterator();

            // copy iterator contents
            copyIteratorInfo(iterCopy, (DecodeIteratorImpl)iter);
        }
        else
        {
            iterCopy = (DecodeIteratorImpl)iter;
        }

        ret = Decoders.decodeElementList(iterCopy, eList, setDb);
        if (ret == CodecReturnCodes.NO_DATA || ret < CodecReturnCodes.SUCCESS)
            return "";

        xmlString.append(xmlDumpElementListBegin(eList));

        while ((ret = Decoders.decodeElementEntry(iterCopy, element)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCodes.SUCCESS)
            {
                xmlString.append("Error occurred while decoding ElementEntry, CodecReturnCode=" + ret);
                return xmlString.toString();
            }

            xmlString.append(xmlDumpElementBegin(element));
            if ((Encoders.validAggregateDataType(element.dataType())) || (element.dataType() == DataTypes.ARRAY))
                xmlString.append(">\n");
            if (element.name().equals(ElementNames.GROUP))
            {
                xmlString.append(" data=\"");
                xmlString.append(xmlDumpGroupId(element.encodedData()));
                xmlString.append("\"");
            }
            else if (element.name().equals(ElementNames.MERG_TO_GRP))
            {
                xmlString.append(" data=\"");
                xmlString.append(xmlDumpGroupId(element.encodedData()));
                xmlString.append("\"");
            }
            else
            {
                xmlString.append(decodeDataTypeToXML(element.dataType(), element.encodedData(), dictionary, setDb, iterCopy));
            }

            if ((Encoders.validAggregateDataType(element.dataType())) || (element.dataType() == DataTypes.ARRAY))
                xmlString.append(xmlDumpElementEnd());
            else
                xmlString.append(xmlDumpEndNoTag());
        }

        xmlString.append(xmlDumpElementListEnd());

        return xmlString.toString();
    }

    private static String xmlDumpElementListEnd()
    {
        indents--;

        return (encodeindents() + "</elementList>\n");
    }

    private static String xmlDumpEndNoTag()
    {
        indents--;

        return "/>\n";
    }

    private static String xmlDumpElementEnd()
    {
        indents--;

        return (encodeindents() + "</elementEntry>\n");
    }

    private static String xmlDumpGroupId(Buffer buffer)
    {
        StringBuilder xmlString = new StringBuilder();
        short tempVal = 0;
        int index = ((BufferImpl)buffer).position();
        boolean printPeriod = false;
        byte[] shortBytes = new byte[2];

        for (int i = 0; i < buffer.length(); i++)
        {
            if (printPeriod)
                xmlString.append(".");
            if (index < buffer.data().limit() - 2)
            {
                shortBytes[0] = (buffer.data().get(index++));
                shortBytes[1] = (buffer.data().get(index++));
            }
            tempVal = shortBytes[0];
            tempVal <<= 8;
            tempVal |= shortBytes[1];
            xmlString.append(tempVal);
            printPeriod = true;
            i++;
        }

        return xmlString.toString();
    }

    private static String xmlDumpElementBegin(ElementEntry element)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        xmlString.append("<elementEntry name=\"");
        xmlString.append(xmlDumpBuffer(element.name()));
        xmlString.append("\" dataType=\"");
        xmlString.append(xmlDumpDataType(element.dataType()));
        xmlString.append("\"");

        indents++;

        return xmlString.toString();
    }

    private static String xmlDumpDataType(int dataType)
    {
        StringBuilder xmlString = new StringBuilder();
        String str = DataTypes.toString(dataType);

        if (str.length() == 0)
            xmlString.append(dataType);
        else
            xmlString.append(str);

        return xmlString.toString();
    }

    private static String xmlDumpDomainType(int domainType)
    {
        StringBuilder xmlString = new StringBuilder();
        String str = DomainTypes.toString(domainType);

        if (str.length() == 0)
            xmlString.append(domainType);
        else
            xmlString.append(str);

        return xmlString.toString();
    }

    private static String xmlDumpBuffer(Buffer buffer)
    {
        String ret = "";

        if (buffer.length() > 0)
            ret = buffer.toString();

        return ret;
    }

    private static String xmlDumpElementListBegin(ElementList eList)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<elementList flags=\"0x" + String.format("%02X", eList.flags()));

        if (eList.flags() != 0)
            xmlString.append(" (");
        if (eList.checkHasInfo())
        {
            xmlString.append("HAS_ELEMENT_LIST_INFO");
            firstFlag = false;
        }

        if (eList.checkHasSetData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_SET_DATA");
        }

        if (eList.checkHasSetId())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_SET_ID");
        }

        if (eList.checkHasStandardData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_STANDARD_DATA");
        }
        if (eList.flags() != 0)
            xmlString.append(")");
        xmlString.append("\"");

        if (eList.checkHasInfo())
        {
            xmlString.append(" elementListNum=\"" + eList.elementListNum() + "\"");
        }
        if (eList.checkHasSetData())
        {
            if (eList.checkHasSetId())
                xmlString.append(" setId=\"" + eList.setId() + "\"");

        }
        xmlString.append(">\n");

        return xmlString.toString();
    }

    private static String xmlDumpReal(Real oReal64)
    {
        return oReal64.toString();
    }

    private static String xmlDumpTime(Time time)
    {
        return time.toString();
    }

    private static String xmlDumpDate(Date date)
    {
        return date.toString();
    }

    private static String xmlDumpDateTime(DateTime datetime)
    {
        return datetime.toString();
    }

    private static String xmlDumpState(State state)
    {
        return (" " + state.toString());
    }

    private static String xmlDumpQos(Qos qos)
    {
        return (" " + qos.toString());
    }

    private static String xmlDumpWorstQos(Qos qos)
    {
        return (" " + qos.toString());
    }

    private static String xmlDumpUInt(UInt u64)
    {
        return u64.toString();
    }

    private static String xmlDumpInt(Int i64)
    {
        return i64.toString();
    }

    private static String xmlDumpDouble(Float f32)
    {
        return f32.toString();
    }

    private static String xmlDumpDouble(Double d64)
    {
        return d64.toString();
    }

    private static String xmlDumpMsgBegin(Msg msg, String tagName)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<" + tagName);
        xmlString.append(" domainType=\"");
        xmlString.append(xmlDumpDomainType(msg.domainType()));
        xmlString.append("\" streamId=\"" + msg.streamId() + "\" containerType=\"");
        xmlString.append(xmlDumpDataType(msg.containerType()));
        switch (msg.msgClass())
        {
            case MsgClasses.UPDATE:
                UpdateMsg updateMsg = (UpdateMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", updateMsg.flags()));

                if (updateMsg.flags() != 0)
                    xmlString.append(" (");

                if (updateMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (updateMsg.checkHasPermData())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PERM_DATA");
                }
                if (updateMsg.checkHasMsgKey())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_MSG_KEY");
                }
                if (updateMsg.checkHasSeqNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_SEQ_NUM");
                }
                if (updateMsg.checkHasConfInfo())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_CONF_INFO");
                }
                if (updateMsg.checkDoNotCache())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("DO_NOT_CACHE");
                }
                if (updateMsg.checkDoNotConflate())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("DO_NOT_CONFLATE");
                }
                if (updateMsg.checkDoNotRipple())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("DO_NOT_RIPPLE");
                }
                if (updateMsg.checkHasPostUserInfo())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_POST_USER_INFO");
                }
                if (updateMsg.checkDiscardable())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("DISCARDABLE");
                }

                if (updateMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");

                xmlString.append(" updateType=\"" + updateMsg.updateType() + "\"");
                if (updateMsg.checkHasSeqNum())
                {
                    xmlString.append(" seqNum=\"" + updateMsg.seqNum() + "\"");
                }

                if (updateMsg.checkHasPermData())
                {
                    xmlString.append(" permData=\"");
                    xmlString.append(xmlDumpHexBuffer(updateMsg.permData()));
                    xmlString.append("\"");
                }

                if (updateMsg.checkHasConfInfo())
                {
                    xmlString.append(" conflationCount=\"" + updateMsg.conflationCount() + "\" conflationTime=\""
                                     + updateMsg.conflationTime() + "\"");
                }

                if (updateMsg.checkHasPostUserInfo())
                {
                    xmlString.append("postUserId=\"" + updateMsg.postUserInfo().userId() + "\" postUserAddr=\""
                                     + updateMsg.postUserInfo().userAddrToString(updateMsg.postUserInfo().userAddr()) + "\"");
                }
                break;
            case MsgClasses.GENERIC:
                GenericMsg genericMsg = (GenericMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", genericMsg.flags()));

                if (genericMsg.flags() != 0)
                    xmlString.append(" (");
                if (genericMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (genericMsg.checkHasPermData())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PERM_DATA");
                }
                if (genericMsg.checkHasMsgKey())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_MSG_KEY");
                }
                if (genericMsg.checkHasSeqNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_SEQ_NUM");
                }
                if (genericMsg.checkMessageComplete())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("MESSAGE_COMPLETE");
                }
                if (genericMsg.checkIsProviderDriven())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("PROVIDER_DRIVEN");
                }
                if (genericMsg.checkHasSecondarySeqNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_SECONDARY_SEQ_NUM");
                }
                if (genericMsg.checkHasPartNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PART_NUM");
                }
                if (genericMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");

                if (genericMsg.checkHasSeqNum())
                {
                    xmlString.append(" seqNum=\"" + genericMsg.seqNum() + "\"");
                }

                if (genericMsg.checkHasSecondarySeqNum())
                {
                    xmlString.append(" secondarySeqNum=\"" + genericMsg.secondarySeqNum() + "\"");
                }

                if (genericMsg.checkHasPartNum())
                {
                    xmlString.append(" partNum=\"" + genericMsg.partNum() + "\"");
                }

                if (genericMsg.checkHasPermData())
                {
                    xmlString.append(" permData=\"");
                    xmlString.append(xmlDumpHexBuffer(genericMsg.permData()));
                    xmlString.append("\"");
                }
                break;
            case MsgClasses.REFRESH:
                RefreshMsg refreshMsg = (RefreshMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", refreshMsg.flags()));

                if (refreshMsg.flags() != 0)
                    xmlString.append(" (");
                if (refreshMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (refreshMsg.checkHasPermData())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PERM_DATA");
                }
                if (refreshMsg.checkHasMsgKey())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_MSG_KEY");
                }
                if (refreshMsg.checkHasSeqNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_SEQ_NUM");
                }
                if (refreshMsg.checkSolicited())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("SOLICITED");
                }
                if (refreshMsg.checkRefreshComplete())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("REFRESH_COMPLETE");
                }
                if (refreshMsg.checkHasQos())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_QOS");
                }
                if (refreshMsg.checkClearCache())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("CLEAR_CACHE");
                }
                if (refreshMsg.checkDoNotCache())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("DO_NOT_CACHE");
                }
                if (refreshMsg.checkPrivateStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("PRIVATE_STREAM");
                }
                if (refreshMsg.checkQualifiedStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("QUALIFIED_STREAM");
                }
                if (refreshMsg.checkHasPostUserInfo())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_POST_USER_INFO");
                }
                if (refreshMsg.checkHasPartNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PART_NUM");
                }
                if (refreshMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");

                xmlString.append(" groupId=\"");
                xmlString.append(xmlDumpGroupId(refreshMsg.groupId()));
                xmlString.append("\"");

                if (refreshMsg.checkHasSeqNum())
                {
                    xmlString.append(" seqNum=\"" + refreshMsg.seqNum() + "\"");
                }

                if (refreshMsg.checkHasPartNum())
                {
                    xmlString.append(" partNum=\"" + refreshMsg.partNum() + "\"");
                }

                if (refreshMsg.checkHasPermData())
                {
                    xmlString.append(" permData=\"");
                    xmlString.append(xmlDumpHexBuffer(refreshMsg.permData()));
                    xmlString.append("\"");
                }

                if (refreshMsg.checkHasQos())
                {
                    xmlString.append(xmlDumpQos(refreshMsg.qos()));
                }

                xmlString.append(xmlDumpState(refreshMsg.state()));

                if (refreshMsg.checkHasPostUserInfo())
                {
                    xmlString.append(" postUserId=\"" + refreshMsg.postUserInfo().userId() + "\" postUserAddr=\""
                                     + refreshMsg.postUserInfo().userAddrToString(refreshMsg.postUserInfo().userAddr()) + "\"");
                }
                break;
            case MsgClasses.POST:
                PostMsg postMsg = (PostMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", postMsg.flags()));

                if (postMsg.flags() != 0)
                    xmlString.append(" (");
                if (postMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (postMsg.checkHasPostId())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_POST_ID");
                }
                if (postMsg.checkHasMsgKey())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_MSG_KEY");
                }
                if (postMsg.checkHasSeqNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_SEQ_NUM");
                }
                if (postMsg.checkPostComplete())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("POST_COMPLETE");
                }
                if (postMsg.checkAck())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("ACK");
                }
                if (postMsg.checkHasPermData())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PERM_DATA");
                }
                if (postMsg.checkHasPartNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PART_NUM");
                }
                if (postMsg.checkHasPostUserRights())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_POST_USER_RIGHTS");
                }
                if (postMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");

                if (postMsg.checkHasSeqNum())
                {
                    xmlString.append(" seqNum=\"" + postMsg.seqNum() + "\"");
                }

                if (postMsg.checkHasPostId())
                {
                    xmlString.append(" postId=\"" + postMsg.postId() + "\"");
                }

                if (postMsg.checkHasPermData())
                {
                    xmlString.append(" permData=\"");
                    xmlString.append(xmlDumpHexBuffer(postMsg.permData()));
                    xmlString.append("\"");
                }

                if (postMsg.checkHasPartNum())
                {
                    xmlString.append(" partNum=\"" + postMsg.partNum() + "\"");
                }

                if (postMsg.checkHasPostUserRights())
                {
                    xmlString.append(" postUserRights=\"" + postMsg.postUserRights() + "\"");
                }

                /* print user info */
                xmlString.append(" postUserId=\"" + postMsg.postUserInfo().userId() + "\" postUserAddr=\""
                                 + postMsg.postUserInfo().userAddrToString(postMsg.postUserInfo().userAddr()) + "\"");
                break;
            case MsgClasses.REQUEST:
                RequestMsg requestMsg = (RequestMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", requestMsg.flags()));

                if (requestMsg.flags() != 0)
                    xmlString.append(" (");
                if (requestMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (requestMsg.checkHasPriority())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PRIORITY");
                }
                if (requestMsg.checkStreaming())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("STREAMING");
                }
                if (requestMsg.checkMsgKeyInUpdates())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("MSG_KEY_IN_UPDATES");
                }
                if (requestMsg.checkConfInfoInUpdates())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("CONF_INFO_IN_UPDATES");
                }
                if (requestMsg.checkNoRefresh())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("NO_REFRESH");
                }
                if (requestMsg.checkHasQos())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_QOS");
                }
                if (requestMsg.checkHasWorstQos())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_WORST_QOS");
                }
                if (requestMsg.checkPrivateStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("PRIVATE_STREAM");
                }
                if (requestMsg.checkQualifiedStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("QUALIFIED_STREAM");
                }
                if (requestMsg.checkPause())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("PAUSE");
                }
                if (requestMsg.checkHasView())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_VIEW");
                }
                if (requestMsg.checkHasBatch())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_BATCH");
                }
                if (requestMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");

                if (requestMsg.checkHasQos())
                {
                    xmlString.append(xmlDumpQos(requestMsg.qos()));
                }
                if (requestMsg.checkHasWorstQos())
                {
                    xmlString.append(xmlDumpWorstQos(requestMsg.worstQos()));
                }

                if (requestMsg.checkHasPriority())
                {
                    xmlString.append(" priorityClass=\"" + requestMsg.priority().priorityClass() + "\" priorityCount=\""
                                     + requestMsg.priority().count() + "\"");
                }
                break;
            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", statusMsg.flags()));

                if (statusMsg.flags() != 0)
                    xmlString.append(" (");
                if (statusMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (statusMsg.checkHasPermData())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_PERM_DATA");
                }
                if (statusMsg.checkHasMsgKey())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_MSG_KEY");
                }
                if (statusMsg.checkHasGroupId())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_GROUP_ID");
                }
                if (statusMsg.checkHasState())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_STATE");
                }
                if (statusMsg.checkClearCache())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("CLEAR_CACHE");
                }
                if (statusMsg.checkPrivateStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("PRIVATE_STREAM");
                }
                if (statusMsg.checkQualifiedStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("QUALIFIED_STREAM");
                }
                if (statusMsg.checkHasPostUserInfo())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_POST_USER_INFO");
                }
                if (statusMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");

                if (statusMsg.checkHasGroupId())
                {
                    xmlString.append(" groupId=\"");
                    xmlString.append(xmlDumpGroupId(statusMsg.groupId()));
                    xmlString.append("\"");
                }

                if (statusMsg.checkHasPermData())
                {
                    xmlString.append(" permData=\"");
                    xmlString.append(xmlDumpHexBuffer(statusMsg.permData()));
                    xmlString.append("\"");
                }

                if (statusMsg.checkHasState())
                {
                    xmlString.append(xmlDumpState(statusMsg.state()));
                }

                if (statusMsg.checkHasPostUserInfo())
                {
                    xmlString.append("postUserId=\"" + statusMsg.postUserInfo().userId() + "\" postUserAddr=\""
                                     + statusMsg.postUserInfo().userAddrToString(statusMsg.postUserInfo().userAddr()) + "\"");
                }
                break;
            case MsgClasses.CLOSE:
                CloseMsg closeMsg = (CloseMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", closeMsg.flags()));
                if (closeMsg.flags() != 0)
                    xmlString.append(" (");
                if (closeMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (closeMsg.checkAck())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("ACK");
                }
                if (closeMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");
                break;
            case MsgClasses.ACK:
                AckMsg ackMsg = (AckMsg)msg;
                xmlString.append("\" flags=\"0x" + String.format("%02X", ackMsg.flags()));

                if (ackMsg.flags() != 0)
                    xmlString.append(" (");
                if (ackMsg.checkHasExtendedHdr())
                {
                    xmlString.append("HAS_EXTENDED_HEADER");
                    firstFlag = false;
                }
                if (ackMsg.checkHasText())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_TEXT");
                }
                if (ackMsg.checkPrivateStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("PRIVATE_STREAM");
                }
                if (ackMsg.checkQualifiedStream())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("QUALIFIED_STREAM");
                }
                if (ackMsg.checkHasSeqNum())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_SEQ_NUM");
                }
                if (ackMsg.checkHasMsgKey())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_MSG_KEY");
                }
                if (ackMsg.checkHasNakCode())
                {
                    if (!firstFlag)
                        xmlString.append("|");
                    else
                        firstFlag = false;
                    xmlString.append("HAS_NAK_CODE");
                }
                if (ackMsg.flags() != 0)
                    xmlString.append(")");
                xmlString.append("\"");

                xmlString.append(" ackId=\"" + ackMsg.ackId() + "\"");
                if (ackMsg.checkHasNakCode())
                {
                    xmlString.append(" nakCode=\"" + NakCodes.toString(ackMsg.nakCode()) + "\"");
                }

                if (ackMsg.checkHasText())
                {
                    xmlString.append(" text=\"");
                    xmlString.append(xmlDumpBuffer(ackMsg.text()));
                    xmlString.append("\"");
                }

                if (ackMsg.checkHasSeqNum())
                {
                    xmlString.append(" seqNum=\"" + ackMsg.seqNum() + "\"");
                }
                break;
            default:
                xmlString.append("\"");
        }
        xmlString.append(" dataSize=\"" + msg.encodedDataBody().length() + "\">\n");

        return xmlString.toString();
    }

    private static String xmlDumpHexBuffer(Buffer buffer)
    {
        StringBuilder xmlString = new StringBuilder();
        int bufPosition = ((BufferImpl)buffer).position();
        byte bufByte;

        for (int i = 0; i < buffer.length(); i++)
        {
            bufByte = buffer.data().get(i + bufPosition);
            if (i % 32 == 0)
            {
                if (i != 0)
                {
                    xmlString.append("\n");
                    xmlString.append(encodeindents());
                }
            }
            else if ((i != 0) && (i % 2 == 0))
            {
                xmlString.append(" ");
            }
            xmlString.append(String.format("%02X", bufByte));
        }

        return xmlString.toString();
    }

    private static String xmlDumpFieldListEnd()
    {
        indents--;

        return (encodeindents() + "</fieldList>\n");
    }

    private static String xmlDumpFieldEnd()
    {
        indents--;

        return (encodeindents() + "</fieldEntry>\n");
    }

    private static String xmlDumpFieldBegin(FieldEntry field, int dataType)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        xmlString.append("<fieldEntry fieldId=\"" + field.fieldId());
        if (dataType != DataTypes.UNKNOWN)
        {
            xmlString.append("\" dataType=\"");
            xmlString.append(xmlDumpDataType(dataType));
        }
        xmlString.append("\"");
        indents++;

        return xmlString.toString();
    }

    private static String xmlDumpFieldListBegin(FieldList fList)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<fieldList flags=\"0x" + String.format("%02X", fList.flags()));

        if (fList.flags() != 0)
            xmlString.append(" (");
        if (fList.checkHasInfo())
        {
            xmlString.append("HAS_FIELD_LIST_INFO");
            firstFlag = false;
        }

        if (fList.checkHasSetData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_SET_DATA");
        }

        if (fList.checkHasSetId())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_SET_ID");
        }

        if (fList.checkHasStandardData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_STANDARD_DATA");
        }
        if (fList.flags() != 0)
            xmlString.append(")");
        xmlString.append("\"");

        if (fList.checkHasInfo())
        {
            xmlString.append(" fieldListNum=\"" + fList.fieldListNum() + "\" dictionaryId=\"" + fList.dictionaryId() + "\"");
        }
        if (fList.checkHasSetData())
        {
            if (fList.checkHasSetId())
                xmlString.append(" setId=\"" + fList.setId() + "\"");
        }
        xmlString.append(">\n");

        return xmlString.toString();
    }

    private static String decodeSummaryData(DecodeIterator iter, int containerType, Buffer input,
                                            int majorVer, int minorVer, DataDictionary dictionary, Object setDb)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(xmlDumpSummaryDataBegin());

        xmlString.append(decodeDataTypeToXML(containerType, input, dictionary, setDb, iter));

        xmlString.append(xmlDumpSummaryDataEnd());

        return xmlString.toString();
    }

    private static String xmlDumpMapEnd()
    {
        indents--;

        return (encodeindents() + "</map>\n");
    }

    private static String xmlDumpMapEntryEnd()
    {
        indents--;

        return (encodeindents() + "</mapEntry>\n");
    }

    private static String xmlDumpMapEntryBegin(int keyPrimitiveType, MapEntry mapEntry, Object mapKeyData)
    {
        StringBuilder xmlString = new StringBuilder();
        String actionString;
        Buffer stringBuf = CodecFactory.createBuffer();

        xmlString.append(encodeindents());
        indents++;

        switch (mapEntry.action())
        {
            case MapEntryActions.UPDATE:
                actionString = "UPDATE";
                break;
            case MapEntryActions.ADD:
                actionString = "ADD";
                break;
            case MapEntryActions.DELETE:
                actionString = "DELETE";
                break;
            default:
                actionString = "Unknown";

        }
        /* Don't print the data element for deleted rows, there should not be any. */
        xmlString.append("<mapEntry flags=\"0x" + String.format("%02X", mapEntry.flags()));

        if (mapEntry.checkHasPermData())
        {
            xmlString.append(" (HAS_PERM_DATA)");
        }

        xmlString.append("\" action=\"" + actionString + "\" key=\"");
        if (Decoders.primitiveToString(mapKeyData, keyPrimitiveType, stringBuf) < 0)
        {
            ((BufferImpl)stringBuf).data_internal("<Unknown>");
        }
        xmlString.append(stringBuf.toString() + "\" ");

        if (mapEntry.checkHasPermData())
        {
            xmlString.append("permData=\"");
            xmlString.append(xmlDumpHexBuffer(mapEntry.permData()));
            xmlString.append("\">\n");
        }
        else
            xmlString.append(">\n");

        return xmlString.toString();
    }

    private static String xmlDumpLocalElementSetDefDb(LocalElementSetDefDb elListSetDb)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents() + "<elementSetDefs>\n");
        indents++;

        for (int i = 0; i <= LocalElementSetDefDbImpl.MAX_LOCAL_ID; ++i)
        {
            if (elListSetDb.definitions()[i].setId() != LocalElementSetDefDbImpl.BLANK_ID)
            {
                ElementSetDef setDef = elListSetDb.definitions()[i];
                xmlString.append(encodeindents());
                xmlString.append("<elementSetDef setId=\"" + elListSetDb.definitions()[i].setId() + "\">\n");

                ++indents;
                for (int j = 0; j < setDef.count(); ++j)
                {
                    ElementSetDefEntry entry = setDef.entries()[j];
                    xmlString.append(encodeindents());
                    xmlString.append("<elementSetDefEntry name=\"" + entry.name().toString() + "\" dataType=\"");
                    xmlString.append(xmlDumpDataType(entry.dataType()));
                    xmlString.append("\" />\n");
                }
                --indents;

                xmlString.append(encodeindents());
                xmlString.append("</elementSetDef>\n");
            }
        }

        indents--;
        xmlString.append(encodeindents());
        xmlString.append("</elementSetDefs>\n");

        return xmlString.toString();
    }

    private static String xmlDumpLocalFieldSetDefDb(LocalFieldSetDefDb flListSetDb)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        xmlString.append("<fieldSetDefs>\n");

        indents++;

        for (int i = 0; i <= LocalFieldSetDefDbImpl.MAX_LOCAL_ID; ++i)
        {
            if (flListSetDb.definitions()[i].setId() != FieldSetDefDbImpl.BLANK_ID)
            {
                FieldSetDef setDef = flListSetDb.definitions()[i];
                xmlString.append(encodeindents());
                xmlString.append("<fieldSetDef setId=\"" + flListSetDb.definitions()[i].setId() + "\">\n");

                ++indents;
                for (int j = 0; j < setDef.count(); ++j)
                {
                    FieldSetDefEntry entry = setDef.entries()[j];
                    xmlString.append(encodeindents());
                    xmlString.append("<fieldSetDefEntry fieldId=\"" + entry.fieldId() + "\" dataType=\"");
                    xmlString.append(xmlDumpDataType(entry.dataType()));
                    xmlString.append("\" />\n");
                }
                --indents;

                xmlString.append(encodeindents());
                xmlString.append("</fieldSetDef>\n");
            }
        }

        indents--;
        xmlString.append(encodeindents());
        xmlString.append("</fieldSetDefs>\n");

        return xmlString.toString();
    }

    private static String xmlDumpMapBegin(Map map)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<map flags=\"0x" + String.format("%02X", map.flags()));

        if (map.flags() != 0)
            xmlString.append(" (");
        if (map.checkHasSetDefs())
        {
            xmlString.append("HAS_SET_DEFS");
            firstFlag = false;
        }
        if (map.checkHasSummaryData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_SUMMARY_DATA");
        }
        if (map.checkHasPerEntryPermData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_PER_ENTRY_PERM_DATA");
        }
        if (map.checkHasTotalCountHint())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_TOTAL_COUNT_HINT");
        }
        if (map.checkHasKeyFieldId())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_KEY_FIELD_ID");
        }
        if (map.flags() != 0)
            xmlString.append(")");
        xmlString.append("\"");

        xmlString.append(" countHint=\"" + map.totalCountHint() + "\" keyPrimitiveType=\"");
        xmlString.append(xmlDumpDataType(map.keyPrimitiveType()));
        xmlString.append("\" containerType=\"");
        xmlString.append(xmlDumpDataType(map.containerType()));
        xmlString.append("\" ");
        if (map.checkHasKeyFieldId())
        {
            xmlString.append("keyFieldId=\"" + map.keyFieldId() + "\" ");
        }
        xmlString.append(">\n");

        return xmlString.toString();
    }

    private static String xmlDumpSummaryDataEnd()
    {
        indents--;

        return (encodeindents() + "</summaryData>\n");
    }

    private static String xmlDumpSummaryDataBegin()
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<summaryData>\n");

        return xmlString.toString();
    }

    private static String xmlDumpFilterListEnd()
    {
        indents--;

        return (encodeindents() + "</filterList>\n");
    }

    private static String xmlDumpFilterItemEnd()
    {
        indents--;

        return (encodeindents() + "</filterEntry>\n");
    }

    private static String xmlDumpFilterItemBegin(FilterEntry filterItem)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;
        String actionString;

        xmlString.append(encodeindents());
        indents++;
        switch (filterItem.action())
        {
            case FilterEntryActions.UPDATE:
                actionString = "UPDATE";
                break;
            case FilterEntryActions.SET:
                actionString = "SET";
                break;
            case FilterEntryActions.CLEAR:
                actionString = "CLEAR";
                break;
            default:
                actionString = "Unknown";
        }
        /* Don't print the data element for deleted rows, there should not be any. */
        xmlString.append("<filterEntry id=\"" + filterItem.id() + "\" action=\"" + actionString
                         + "\" flags=\"0x" + String.format("%02X", filterItem.flags()));

        if (filterItem.flags() != 0)
            xmlString.append(" (");
        if (filterItem.checkHasPermData())
        {
            xmlString.append("HAS_PERM_DATA");
            firstFlag = false;
        }
        if (filterItem.checkHasContainerType())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_CONTAINER_TYPE");
        }
        if (filterItem.flags() != 0)
            xmlString.append(")");

        xmlString.append("\" containerType=\"");
        xmlString.append(xmlDumpDataType(filterItem.containerType()));
        if (filterItem.checkHasPermData())
        {
            xmlString.append("\" permData=\"");
            xmlString.append(xmlDumpHexBuffer(filterItem.permData()));
            xmlString.append("\">\n");
        }
        else
            xmlString.append("\">\n");

        return xmlString.toString();
    }

    private static String xmlDumpFilterListBegin(FilterList fList)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<filterList containerType=\"");
        xmlString.append(xmlDumpDataType(fList.containerType()));
        xmlString.append("\" countHint=\"" + fList.totalCountHint() + "\" flags=\"0x" + String.format("%02X", fList.flags()));

        if (fList.flags() != 0)
            xmlString.append(" (");
        if (fList.checkHasPerEntryPermData())
        {
            xmlString.append("HAS_PER_ENTRY_PERM_DATA");
            firstFlag = false;
        }
        if (fList.checkHasTotalCountHint())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_TOTAL_COUNT_HINT");
        }
        if (fList.flags() != 0)
            xmlString.append(")");
        xmlString.append("\">\n");

        return xmlString.toString();
    }

    private static Object createKeyData(int dataType)
    {
        Object data = null;

        switch (dataType)
        {
            case DataTypes.INT:
                data = CodecFactory.createInt();
                break;
            case DataTypes.UINT:
                data = CodecFactory.createUInt();
                break;
            case DataTypes.FLOAT:
                data = CodecFactory.createFloat();
                break;
            case DataTypes.DOUBLE:
                data = CodecFactory.createDouble();
                break;
            case DataTypes.REAL:
                data = CodecFactory.createReal();
                break;
            case DataTypes.DATE:
                data = CodecFactory.createDate();
                break;
            case DataTypes.TIME:
                data = CodecFactory.createTime();
                break;
            case DataTypes.DATETIME:
                data = CodecFactory.createDateTime();
                break;
            case DataTypes.QOS:
                data = CodecFactory.createQos();
                break;
            case DataTypes.STATE:
                data = CodecFactory.createState();
                break;
            case DataTypes.ENUM:
                data = CodecFactory.createEnum();
                break;
            case DataTypes.ASCII_STRING:
            case DataTypes.RMTES_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.BUFFER:
                data = CodecFactory.createBuffer();
                break;
            default:
                break;
        }

        return data;
    }

    private static String xmlDumpArrayItemEnd()
    {
        return "/>\n";
    }

    private static String xmlDumpArrayItemBegin()
    {
        return (encodeindents() + "<arrayEntry");
    }

    private static String xmlDumpArrayEnd()
    {
        indents--;

        return (encodeindents() + "</array>\n");
    }

    private static String xmlDumpArrayBegin(Array array)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<array itemLength=\"" + array.itemLength() + "\" primitiveType=\"");
        xmlString.append(xmlDumpDataType(array.primitiveType()));
        xmlString.append("\">\n");

        return xmlString.toString();
    }

    private static void copyIteratorInfo(DecodeIteratorImpl destIter, DecodeIteratorImpl sourceIter)
    {
        destIter.clear();
        destIter.setBufferAndRWFVersion(sourceIter._buffer, sourceIter.majorVersion(), sourceIter.minorVersion());
        destIter._curBufPos = sourceIter._curBufPos;
        if (destIter._curBufPos == 3 &&
            sourceIter._buffer.data().get(2) == MsgClasses.REFRESH &&
            sourceIter._buffer.data().get(3) == DomainTypes.DICTIONARY)
        {
            destIter._curBufPos = 0;
        }
        destIter._decodingLevel = sourceIter._decodingLevel;
        for (int i = 0; i < destIter._levelInfo.length; i++)
        {
            destIter._levelInfo[i]._containerType = sourceIter._levelInfo[i]._containerType;
            destIter._levelInfo[i]._endBufPos = sourceIter._levelInfo[i]._endBufPos;
            destIter._levelInfo[i]._itemCount = sourceIter._levelInfo[i]._itemCount;
            destIter._levelInfo[i]._listType = sourceIter._levelInfo[i]._listType;
            destIter._levelInfo[i]._nextEntryPos = sourceIter._levelInfo[i]._nextEntryPos;
            destIter._levelInfo[i]._nextItemPosition = sourceIter._levelInfo[i]._nextItemPosition;
            destIter._levelInfo[i]._nextSetPosition = sourceIter._levelInfo[i]._nextSetPosition;
            destIter._levelInfo[i]._setCount = sourceIter._levelInfo[i]._setCount;
            if (destIter._levelInfo[i]._elemListSetDef != null && sourceIter._levelInfo[i]._elemListSetDef != null)
            {
                destIter._levelInfo[i]._elemListSetDef._count = sourceIter._levelInfo[i]._elemListSetDef._count;
                destIter._levelInfo[i]._elemListSetDef._setId = sourceIter._levelInfo[i]._elemListSetDef._setId;
                destIter._levelInfo[i]._elemListSetDef._entries = sourceIter._levelInfo[i]._elemListSetDef._entries;
            }
            if (destIter._levelInfo[i]._fieldListSetDef != null && sourceIter._levelInfo[i]._fieldListSetDef != null)
            {
                destIter._levelInfo[i]._fieldListSetDef._count = sourceIter._levelInfo[i]._fieldListSetDef._count;
                destIter._levelInfo[i]._fieldListSetDef._setId = sourceIter._levelInfo[i]._fieldListSetDef._setId;
                destIter._levelInfo[i]._fieldListSetDef._entries = sourceIter._levelInfo[i]._fieldListSetDef._entries;
            }
        }
    }

    private static String xmlDumpSeriesRowEnd()
    {
        indents--;

        return (encodeindents() + "</seriesEntry>\n");
    }

    private static String xmlDumpSeriesRowBegin(SeriesEntry row)
    {
        StringBuilder xmlString = new StringBuilder();

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<seriesEntry>\n");

        return xmlString.toString();
    }

    private static String xmlDumpSeriesEnd()
    {
        indents--;

        return (encodeindents() + "</series>\n");
    }

    private static String xmlDumpSeriesBegin(Series series)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;

        xmlString.append(encodeindents());
        indents++;

        xmlString.append("<series  flags=\"0x" + String.format("%02X", series.flags()));
        if (series.flags() != 0)
            xmlString.append(" (");
        if (series.checkHasSetDefs())
        {
            xmlString.append("HAS_SET_DEFS");
            firstFlag = false;
        }
        if (series.checkHasSummaryData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_SUMMARY_DATA");
        }
        if (series.checkHasTotalCountHint())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_TOTAL_COUNT_HINT");
        }
        if (series.flags() != 0)
            xmlString.append(")");

        xmlString.append("\" countHint=\"" + series.totalCountHint() + "\" containerType=\"");
        xmlString.append(xmlDumpDataType(series.containerType()));
        xmlString.append("\">\n");

        return xmlString.toString();
    }

    private static String xmlDumpVectorEnd()
    {
        indents--;

        return (encodeindents() + "</vector>\n");
    }

    private static String xmlDumpVectorEntryEnd()
    {
        indents--;

        return (encodeindents() + "</vectorEntry>\n");
    }

    private static String xmlDumpVectorEntryBegin(VectorEntry vectorEntry)
    {
        StringBuilder xmlString = new StringBuilder();
        String actionString;

        xmlString.append(encodeindents());
        indents++;
        switch (vectorEntry.action())
        {
            case VectorEntryActions.UPDATE:
                actionString = "UPDATE";
                break;
            case VectorEntryActions.SET:
                actionString = "SET";
                break;
            case VectorEntryActions.INSERT:
                actionString = "INSERT";
                break;
            case VectorEntryActions.DELETE:
                actionString = "DELETE";
                break;
            case VectorEntryActions.CLEAR:
                actionString = "CLEAR";
                break;
            default:
                actionString = "Unknown";
        }
        /* Don't print the data element for deleted rows, there should not be
         * any. */
        xmlString.append("<vectorEntry index=\"" + vectorEntry.index() + "\" action=\"" + actionString
                         + "\" flags=\"0x" + String.format("%02X", vectorEntry.flags()));
        if (vectorEntry.checkHasPermData())
        {
            xmlString.append(" (HAS_PERM_DATA)\"");
            xmlString.append("permData=\"");
            xmlString.append(xmlDumpHexBuffer(vectorEntry.permData()));
            xmlString.append("\">\n");
        }
        else
            xmlString.append("\">\n");

        return xmlString.toString();
    }

    private static String xmlDumpVectorBegin(Vector vec)
    {
        StringBuilder xmlString = new StringBuilder();
        boolean firstFlag = true;

        xmlString.append(encodeindents());
        indents++;
        xmlString.append("<vector flags=\"0x" + String.format("%02X", vec.flags()));

        if (vec.flags() != 0)
            xmlString.append(" (");
        if (vec.checkHasSetDefs())
        {
            xmlString.append("HAS_SET_DEFS");
            firstFlag = false;
        }
        if (vec.checkHasSummaryData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_SUMMARY_DATA");
        }
        if (vec.checkHasPerEntryPermData())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_PER_ENTRY_PERM_DATA");
        }
        if (vec.checkHasTotalCountHint())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("HAS_TOTAL_COUNT_HINT");
        }
        if (vec.checkSupportsSorting())
        {
            if (!firstFlag)
                xmlString.append("|");
            else
                firstFlag = false;
            xmlString.append("SUPPORTS_SORTING");
        }
        if (vec.flags() != 0)
            xmlString.append(")");
        xmlString.append("\"");

        xmlString.append(" countHint=\"" + vec.totalCountHint() + "\" containerType=\"");
        xmlString.append(xmlDumpDataType(vec.containerType()));
        xmlString.append("\">\n");

        return xmlString.toString();
    }

    public static String xmlDumpString(Buffer buf, boolean escape) {
        final StringBuilder lineBuilder = new StringBuilder();
        if (buf.data() != null) {
            int pos = buf.position();
            byte c = buf.data().get(pos++);

            if (c != 0x00 || buf.length() > 1) {
                for (int i = 0; i < buf.length(); i++) {
                    if (escape) {
                        if (c == '<') {
                            lineBuilder.append("&lt;");
                        } else if (c == '>') {
                            lineBuilder.append("&gt;");
                        } else if (c == 0x26) {
                            // ampersand
                            lineBuilder.append("&amp;");
                        } else if (c == 0x22) {
                            // quotation mark
                            lineBuilder.append("&quot;");
                        } else if (c == 0x27) {
                            // apostrophe
                            lineBuilder.append("&apos;");
                        } else {
                            lineBuilder.append((char) c);
                        }
                    } else if (c < 0x20 || c > 0x7e) {
                        lineBuilder.append("(0x").append(String.format("%02X", c)).append(")");
                    } else {
                        lineBuilder.append((char) c);
                    }

                    if (pos < buf.data().limit()) {
                        c = buf.data().get(pos++);
                    }
                }
            }
        }
        return lineBuilder.toString();
    }
}
