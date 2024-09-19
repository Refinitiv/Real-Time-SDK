/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Text;
using System.Xml;
using LSEG.Ema.Access;
using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Manages message data information from an XML file
    /// </summary>
    public class XmlMsgData
    {
        private const int INT_SIZE = 8;
        private const int UINT_SIZE = 8;
        private const int FLOAT_SIZE = 4;
        private const int DOUBLE_SIZE = 8;
        private const int REAL_SIZE = 16;
        private const int DATE_SIZE = 4;
        private const int TIME_SIZE = 6;
        private const int DATETIME_SIZE = 10;
        private const int QOS_SIZE = 8;
        private const int STATE_SIZE = 12;
        private const int ENUM_SIZE = 2;
        private const int BUFFER_SIZE = 8;
        private const int MAX_UPDATE_MSGS = 10;
        private const int MAX_POST_MSGS = 10;
        private const int MAX_GEN_MSGS = 10;
        private const int MAX_FIELD_ENTRIES = 100;
        private int _fileUpdateCount;
        private int _filePostCount;
        private int _fileGenMsgCount;
        private const string SECTION_NAME = "marketPriceMsgList";
        private const string UPD_COUNT_NAME = "updateMsgCount";
        private const string POST_COUNT_NAME = "postMsgCount";
        private const string GEN_COUNT_NAME = "genMsgCount";
        private const string REF_MSG_NAME = "refreshMsg";
        private const string UPD_MSG_NAME = "updateMsg";
        private const string GEN_MSG_NAME = "genMsg";
        private const string POST_MSG_NAME = "postMsg";
        private string FIELD_LIST_NAME = "fieldList";
        private string FIELD_ENTRY_NAME = "fieldEntry";
        private string COUNT_ATTR_NAME = "entryCount";

        public MarketPriceMsg? MarketPriceRefreshMsg { get; internal set; }

        public MarketPriceMsg[]? MpUpdateMsgs { get; internal set; }

        public MarketPriceMsg[]? MpPostMsgs { get; internal set; }

        public MarketPriceMsg[]? MpGenMsgs { get; internal set; }

        public int UpdateCount { get; internal set; }

        public int PostCount { get; internal set; }

        public int GenMsgCount { get; internal set; }

        public bool HasMarketPrice { get; internal set; }

        public XmlMsgData()
        {
            _fileUpdateCount = MAX_UPDATE_MSGS;
            _filePostCount = MAX_POST_MSGS;
            _fileGenMsgCount = MAX_GEN_MSGS;
        }

        public PerfToolsReturnCode ParseFile(string fileName)
        {
            XmlDocument xmlDocument = new XmlDocument();
            try
            {
                xmlDocument.Load(fileName);
                XmlNodeList mpList = xmlDocument.GetElementsByTagName(SECTION_NAME);
                XmlNode? section;
                if (mpList.Count == 0 || (section = mpList.Item(0)) == null)
                {
                    Console.WriteLine($"Item file address: {fileName}. Item file contains no data...");
                    return PerfToolsReturnCode.FAILURE;
                }
                ParseSectionAttributes(section);
                foreach (XmlNode childNode in section.ChildNodes)
                {
                    XmlNode? xmlNode = childNode.SelectSingleNode(".//" + FIELD_LIST_NAME);
                    MarketPriceMsg marketPriceMsg = new MarketPriceMsg(GetFieldCount(xmlNode!));
                    PutMsg(marketPriceMsg, childNode.Name);
                    ParseFieldList(marketPriceMsg, xmlNode);
                    GetEstimatedFieldListContentLength(marketPriceMsg);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to parse XML file: {ex.ToString()}");
                return PerfToolsReturnCode.FAILURE;
            }
            return PerfToolsReturnCode.SUCCESS;
        }

        private void ParseSectionAttributes(XmlNode section)
        {
            if (section.Attributes != null)
            {
                XmlAttribute? updAttr = section.Attributes[UPD_COUNT_NAME];
                _fileUpdateCount = updAttr != null ? int.Parse(updAttr.Value) : 0;
                XmlAttribute? postAttr = section.Attributes[POST_COUNT_NAME];
                _filePostCount = postAttr != null ? int.Parse(postAttr.Value) : 0;
                XmlAttribute? genAttr = section.Attributes[GEN_COUNT_NAME];
                _fileGenMsgCount = genAttr != null ? int.Parse(genAttr.Value) : 0;
            }
            MpUpdateMsgs = new MarketPriceMsg[_fileUpdateCount];
            MpPostMsgs = new MarketPriceMsg[_filePostCount];
            MpGenMsgs = new MarketPriceMsg[_fileGenMsgCount];
            HasMarketPrice = true;
        }

        private int GetFieldCount(XmlNode msg)
        {
            XmlNode? attribute;
            return msg.Attributes != null && (attribute = msg.Attributes[COUNT_ATTR_NAME]) != null ? int.Parse(attribute.Value ?? "0") : 0;
        }

        private void PutMsg(MarketPriceMsg mpMsg, string nodeName)
        {
            if (nodeName == REF_MSG_NAME)
            {
                MarketPriceRefreshMsg = mpMsg;
            }
            else if (nodeName == UPD_MSG_NAME)
            {
                MpUpdateMsgs![UpdateCount++] = mpMsg;
            }
            else if (nodeName == GEN_MSG_NAME)
            {
                MpGenMsgs![GenMsgCount++] = mpMsg;
            }
            else if (nodeName == POST_MSG_NAME)
            {
                MpPostMsgs![PostCount++] = mpMsg;
            }                
        }

        private PerfToolsReturnCode ParseFieldList(MarketPriceMsg message, XmlNode? fieldListNode)
        {
            if (fieldListNode == null)
                return PerfToolsReturnCode.FAILURE;

            int count = 0;
            foreach (XmlNode fieldNode in fieldListNode.ChildNodes)
            {
                if (count >= message.FieldEntries.Length)
                {
                    break;
                }
                Debug.Assert(fieldNode.Name.Equals(FIELD_ENTRY_NAME));

                if (fieldNode.Attributes != null)
                {
                    XmlAttribute? fieldIdAttr = fieldNode.Attributes["fieldId"];

                    XmlAttribute? dataTypeAttr = fieldNode.Attributes["dataType"];
                    if (dataTypeAttr == null)
                        return PerfToolsReturnCode.FAILURE;
                    MarketField field = new MarketField()
                    {
                        FieldId = fieldIdAttr != null ? int.Parse(fieldIdAttr.Value) : 0,
                        LoadType = DataTypeValue(dataTypeAttr.Value)
                    };

                    message.FieldEntries[count++] = field;
                    ParseFieldEntry(field, fieldNode);
                }
            }
            message.FieldEntryCount = count;
            return PerfToolsReturnCode.SUCCESS;
        }

        private void ParseFieldEntry(MarketField field, XmlNode fieldNode)
        {
            if (fieldNode.Attributes == null)
                return;

            QosWrapper qosWrapper;
            StateWrapper stateWrapper;
            if (field.LoadType == DataTypes.QOS)
            {
                qosWrapper = new QosWrapper();
                XmlAttribute? rateAttr = fieldNode.Attributes["qosRate"];
                qosWrapper.Rate(rateAttr != null ?rateAttr.Value : null);

                XmlAttribute? rateInfoAttr = fieldNode.Attributes["qosRateInfo"];
                qosWrapper.RateInfo(rateInfoAttr != null ? rateInfoAttr.Value : null);

                XmlAttribute? timelinessAttr = fieldNode.Attributes["qosTimeliness"];
                qosWrapper.Timeliness(timelinessAttr != null ? timelinessAttr.Value : null);

                XmlAttribute? timeInfoAttr = fieldNode.Attributes["qosTimeInfo"];
                qosWrapper.TimeInfo(timeInfoAttr != null ? timeInfoAttr.Value : null);

                XmlAttribute? dynamicAttr = fieldNode.Attributes["qosDynamic"];
                qosWrapper.Dynamic(dynamicAttr?.Value);
                field.Qos = qosWrapper;
            }
            else if (field.LoadType == DataTypes.STATE)
            {
                stateWrapper = new StateWrapper();

                XmlAttribute? streamStateAttr = fieldNode.Attributes["streamState"];
                stateWrapper.StreamState(streamStateAttr != null ? streamStateAttr.Value : null);

                XmlAttribute? dataStateAttr = fieldNode.Attributes["dataState"];
                stateWrapper.DataState(dataStateAttr != null ? dataStateAttr.Value : null);

                XmlAttribute? codeAttr = fieldNode.Attributes["code"];
                stateWrapper.StatusCode(codeAttr != null ? codeAttr.Value : null);

                XmlAttribute? textAttr = fieldNode.Attributes["text"];
                stateWrapper.StatusText(textAttr != null ? textAttr.Value : "");
                field.State = stateWrapper;
            }
            else
            {
                XmlAttribute? dataAttr = fieldNode.Attributes["data"];
                field.Value = dataAttr!.Value;
                if (dataAttr is not null)
                {
                    switch (field.LoadType)
                    {
                        case DataTypes.INT:
                            Int intv = new Int();
                            intv.Value(dataAttr.Value);
                            field.FieldEntry = intv;
                            break;
                        case DataTypes.UINT:
                            UInt uintv = new UInt();
                            uintv.Value(dataAttr.Value);
                            field.FieldEntry = uintv;
                            break;
                        case DataTypes.FLOAT:
                            Float floatv = new Float();
                            floatv.Value(dataAttr.Value);
                            field.FieldEntry = floatv;
                            break;
                        case DataTypes.DOUBLE:
                            Double doublev = new Double();
                            doublev.Value(dataAttr.Value);
                            field.FieldEntry = doublev;
                            break;
                        case DataTypes.REAL:
                            Real real = new Real();
                            real.Value(dataAttr.Value);
                            field.FieldEntry = real;
                            break;
                        case DataTypes.DATE:
                            Date date = new Date();
                            date.Value(dataAttr.Value);
                            field.FieldEntry = date;
                            break;
                        case DataTypes.TIME:
                            Time time = new Time();
                            time.Value(dataAttr.Value);
                            field.FieldEntry = time;
                            break;
                        case DataTypes.DATETIME:
                            DateTime dateTime = new DateTime();
                            dateTime.Value(dataAttr.Value);
                            field.FieldEntry = dateTime;
                            break;
                        case DataTypes.ENUM:
                            Enum enumv = new Enum();
                            enumv.Value(int.Parse(dataAttr.Value));
                            field.FieldEntry = enumv;
                            break;
                        case DataTypes.BUFFER:
                        case DataTypes.RMTES_STRING:
                        case DataTypes.ASCII_STRING:
                            EmaBuffer buffer = new EmaBuffer(Encoding.ASCII.GetBytes(dataAttr.Value!));
                            field.FieldEntry = buffer;
                            break;
                    }
                }               
            }
        }

        private int CodeValue(string value)
        {
            switch (value)
            {
                case "APP_AUTHORIZATION_FAILED":
                    return StateCodes.APP_AUTHORIZATION_FAILED;
                case "GAP_FILL":
                    return StateCodes.GAP_FILL;
                case "RSSL_SC_ALREADY_OPEN":
                    return StateCodes.ALREADY_OPEN;
                case "RSSL_SC_DACS_DOWN":
                    return StateCodes.DACS_DOWN;
                case "RSSL_SC_DACS_MAX_LOGINS_REACHED":
                    return StateCodes.DACS_MAX_LOGINS_REACHED;
                case "RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED":
                    return StateCodes.DACS_USER_ACCESS_TO_APP_DENIED;
                case "RSSL_SC_ERROR":
                    return StateCodes.ERROR;
                case "RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER":
                    return StateCodes.EXCEEDED_MAX_MOUNTS_PER_USER;
                case "RSSL_SC_FAILOVER_COMPLETED":
                    return StateCodes.FAILOVER_COMPLETED;
                case "RSSL_SC_FAILOVER_STARTED":
                    return StateCodes.FAILOVER_STARTED;
                case "RSSL_SC_FULL_VIEW_PROVIDED":
                    return StateCodes.FULL_VIEW_PROVIDED;
                case "RSSL_SC_GAP_DETECTED":
                    return StateCodes.GAP_DETECTED;
                case "RSSL_SC_INVALID_ARGUMENT":
                    return StateCodes.INVALID_ARGUMENT;
                case "RSSL_SC_INVALID_VIEW":
                    return StateCodes.INVALID_VIEW;
                case "RSSL_SC_JIT_CONFLATION_STARTED":
                    return StateCodes.JIT_CONFLATION_STARTED;
                case "RSSL_SC_NONE":
                    return StateCodes.NONE;
                case "RSSL_SC_NON_UPDATING_ITEM":
                    return StateCodes.NON_UPDATING_ITEM;
                case "RSSL_SC_NOT_ENTITLED":
                    return StateCodes.NOT_ENTITLED;
                case "RSSL_SC_NOT_FOUND":
                    return StateCodes.NOT_FOUND;
                case "RSSL_SC_NOT_OPEN":
                    return StateCodes.NOT_OPEN;
                case "RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ":
                    return StateCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ;
                case "RSSL_SC_NO_RESOURCES":
                    return StateCodes.NO_RESOURCES;
                case "RSSL_SC_PREEMPTED":
                    return StateCodes.PREEMPTED;
                case "RSSL_SC_REALTIME_RESUMED":
                    return StateCodes.REALTIME_RESUMED;
                case "RSSL_SC_SOURCE_UNKNOWN":
                    return StateCodes.SOURCE_UNKNOWN;
                case "RSSL_SC_TIMEOUT":
                    return StateCodes.TIMEOUT;
                case "RSSL_SC_TOO_MANY_ITEMS":
                    return StateCodes.TOO_MANY_ITEMS;
                case "RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH":
                    return StateCodes.UNABLE_TO_REQUEST_AS_BATCH;
                case "RSSL_SC_UNSUPPORTED_VIEW_TYPE":
                    return StateCodes.UNSUPPORTED_VIEW_TYPE;
                case "RSSL_SC_USAGE_ERROR":
                    return StateCodes.USAGE_ERROR;
                case "RSSL_SC_USER_UNKNOWN_TO_PERM_SYS":
                    return StateCodes.USER_UNKNOWN_TO_PERM_SYS;
                default:
                    return StateCodes.NONE;
            }
        }

        private int DataStateValue(string value)
        {
            if (value.Equals("RSSL_DATA_NO_CHANGE"))
                return DataStates.NO_CHANGE;
            else if (value.Equals("RSSL_DATA_OK"))
                return DataStates.OK;
            else if (value.Equals("RSSL_DATA_SUSPECT"))
                return DataStates.SUSPECT;
            return DataStates.NO_CHANGE;
        }

        private int StreamStateValue(string value)
        {
            if (value.Equals("RSSL_STREAM_UNSPECIFIED"))
                return StreamStates.UNSPECIFIED;
            else if (value.Equals("RSSL_STREAM_OPEN"))
                return StreamStates.OPEN;
            else if (value.Equals("RSSL_STREAM_NON_STREAMING"))
                return StreamStates.NON_STREAMING;
            else if (value.Equals("RSSL_STREAM_CLOSED_RECOVER"))
                return StreamStates.CLOSED_RECOVER;
            else if (value.Equals("RSSL_STREAM_CLOSED"))
                return StreamStates.CLOSED;
            else if (value.Equals("RSSL_STREAM_REDIRECTED"))
                return StreamStates.REDIRECTED;
            return StreamStates.UNSPECIFIED;
        }

        private int QosTimelinessValue(string value)
        {
            if (value.Equals("RSSL_QOS_TIME_UNSPECIFIED"))
                return QosTimeliness.UNSPECIFIED;
            else if (value.Equals("RSSL_QOS_TIME_REALTIME"))
                return QosTimeliness.REALTIME;
            else if (value.Equals("RSSL_QOS_TIME_DELAYED_UNKNOWN"))
                return QosTimeliness.DELAYED_UNKNOWN;
            else if (value.Equals("RSSL_QOS_TIME_DELAYED"))
                return QosTimeliness.DELAYED;

            return QosTimeliness.UNSPECIFIED;
        }

        private int QosRateValue(string value)
        {
            if (value.Equals("RSSL_QOS_RATE_UNSPECIFIED"))
                return QosRates.UNSPECIFIED;
            else if (value.Equals("RSSL_QOS_RATE_TICK_BY_TICK"))
                return QosRates.TICK_BY_TICK;
            else if (value.Equals("RSSL_QOS_RATE_JIT_CONFLATED"))
                return QosRates.JIT_CONFLATED;
            else if (value.Equals("RSSL_QOS_RATE_TIME_CONFLATED"))
                return QosRates.TIME_CONFLATED;

            return QosRates.UNSPECIFIED;
        }

        private void GetEstimatedFieldListContentLength(MarketPriceMsg marketPriceMsg)
        {
            int estimatedContentLength = 1;
            for (int index = 0; index < marketPriceMsg.FieldEntryCount; ++index)
            {
                MarketField fieldEntry = marketPriceMsg.FieldEntries[index];
                estimatedContentLength += 2;

                switch (fieldEntry.LoadType)
                {
                    case DataTypes.INT:
                        estimatedContentLength += INT_SIZE;
                        break;
                    case DataTypes.UINT:
                        estimatedContentLength += UINT_SIZE;
                        break;
                    case DataTypes.FLOAT:
                        estimatedContentLength += FLOAT_SIZE;
                        break;
                    case DataTypes.DOUBLE:
                        estimatedContentLength += DOUBLE_SIZE;
                        break;
                    case DataTypes.REAL:
                        estimatedContentLength += REAL_SIZE;
                        break;
                    case DataTypes.DATE:
                        estimatedContentLength += DATE_SIZE;
                        break;
                    case DataTypes.TIME:
                        estimatedContentLength += TIME_SIZE;
                        break;
                    case DataTypes.DATETIME:
                        estimatedContentLength += DATETIME_SIZE;
                        break;
                    case DataTypes.QOS:
                        estimatedContentLength += QOS_SIZE;
                        break;
                    case DataTypes.STATE:
                        estimatedContentLength += STATE_SIZE;
                        estimatedContentLength += ((State)fieldEntry.FieldEntry!).Text().Length;
                        break;
                    case DataTypes.ENUM:
                        estimatedContentLength += ENUM_SIZE;
                        break;
                    case DataTypes.BUFFER:
                    case DataTypes.ASCII_STRING:
                    case DataTypes.UTF8_STRING:
                    case DataTypes.RMTES_STRING:
                        estimatedContentLength += BUFFER_SIZE;
                        estimatedContentLength += fieldEntry.Value!.Length;
                        break;
                    default: break;
                }
            }
            marketPriceMsg.EstimatedContentLength = estimatedContentLength;
        }

        private int DataTypeValue(string dataTypeString)
        {
            if (dataTypeString.Equals("RSSL_DT_INT"))
                return DataTypes.INT;
            else if (dataTypeString.Equals("RSSL_DT_UINT"))
                return DataTypes.UINT;
            else if (dataTypeString.Equals("RSSL_DT_FLOAT"))
                return DataTypes.FLOAT;
            else if (dataTypeString.Equals("RSSL_DT_DOUBLE"))
                return DataTypes.DOUBLE;
            else if (dataTypeString.Equals("RSSL_DT_REAL"))
                return DataTypes.REAL;
            else if (dataTypeString.Equals("RSSL_DT_DATE"))
                return DataTypes.DATE;
            else if (dataTypeString.Equals("RSSL_DT_TIME"))
                return DataTypes.TIME;
            else if (dataTypeString.Equals("RSSL_DT_DATETIME"))
                return DataTypes.DATETIME;
            else if (dataTypeString.Equals("RSSL_DT_QOS"))
                return DataTypes.QOS;
            else if (dataTypeString.Equals("RSSL_DT_STATE"))
                return DataTypes.STATE;
            else if (dataTypeString.Equals("RSSL_DT_ENUM"))
                return DataTypes.ENUM;
            else if (dataTypeString.Equals("RSSL_DT_BUFFER"))
                return DataTypes.BUFFER;
            else if (dataTypeString.Equals("RSSL_DT_ASCII_STRING"))
                return DataTypes.ASCII_STRING;
            else if (dataTypeString.Equals("RSSL_DT_UTF8_STRING"))
                return DataTypes.UTF8_STRING;
            else if (dataTypeString.Equals("RSSL_DT_RMTES_STRING"))
                return DataTypes.RMTES_STRING;
            return DataTypes.NO_DATA;
        }
    }
}
