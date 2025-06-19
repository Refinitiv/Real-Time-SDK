/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text;
using LSEG.Eta.Rdm;


namespace LSEG.Eta.Codec
{
	internal class DecodersToXML
	{
		private static int indents;
        private static int INIT_STRING_SIZE = 2048;

		private static string Encodeindents()
		{
			StringBuilder xmlString = new StringBuilder();

			for (int i = 0; i < indents; i++)
			{
				xmlString.Append("    ");
				xmlString.Append("    ");
			}

			return xmlString.ToString();
		}

		internal static string DecodeDataTypeToXML(int dataType, Buffer buffer, DataDictionary dictionary, object setDb, DecodeIterator iter)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
			CodecReturnCode ret;

			if (indents == 0)
			{
				xmlString.Append($"{NewLine}<!-- rwfMajorVer=\"").Append(iter.MajorVersion()).Append("\" rwfMinorVer=\"").Append(iter.MinorVersion()).Append("\" -->").AppendLine();
			}

			switch (dataType)
			{
				case DataTypes.INT:
					Int i64 = new Int();
					ret = Decoders.DecodeInt(iter, i64);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpInt(i64));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.UINT:
					UInt u64 = new UInt();
					ret = Decoders.DecodeUInt(iter, u64);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpUInt(u64));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.FLOAT:
					Float r32 = new Float();
					ret = Decoders.DecodeFloat(iter, r32);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpDouble(r32));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.DOUBLE:
					Double r64 = new Double();
					ret = Decoders.DecodeDouble(iter, r64);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpDouble(r64));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.ENUM:
					Enum enumVal = new Enum();
					Int iTemp = new Int();
					ret = Decoders.DecodeEnum(iter, enumVal);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						iTemp.Value(enumVal.ToInt());
						xmlString.Append(XmlDumpInt(iTemp));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.QOS:
					Qos qos = new Qos();
					xmlString.Append(" data=\"");
					ret = Decoders.DecodeQos(iter, qos);
					if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(XmlDumpQos(qos));
					}
					xmlString.Append("\"");
					break;
				case DataTypes.STATE:
					State state = new State();
					ret = Decoders.DecodeState(iter, state);
					xmlString.Append(" data=\"");
					if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(XmlDumpState(state));
					}
					xmlString.Append("\"");
					break;
				case DataTypes.BUFFER:
				case DataTypes.ASCII_STRING:
				case DataTypes.UTF8_STRING:
				case DataTypes.RMTES_STRING:
					Buffer @out = new Buffer();
					ret = Decoders.DecodeBuffer(iter, @out);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpString(@out));
						xmlString.Append("\"");
					}
					else
					{
						xmlString.Append("Error occurred while decoding dataType ").Append(dataType).Append(", CodecReturnCode=").Append(ret);
						return xmlString.ToString();
					}
					break;
				case DataTypes.DATE:
					Date dtDate = new Date();
					ret = Decoders.DecodeDate(iter, dtDate);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpDate(dtDate));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.TIME:
					DateTime dtTime = new DateTime();
					ret = Decoders.DecodeTime(iter, dtTime.Time());
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpTime(dtTime.Time()));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.DATETIME:
					DateTime dtDatetime = new DateTime();
					ret = Decoders.DecodeDateTime(iter, dtDatetime);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpDateTime(dtDatetime));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.REAL:
					Real oReal64 = new Real();
					ret = Decoders.DecodeReal(iter, oReal64);
					if (ret == CodecReturnCode.BLANK_DATA)
					{
						xmlString.Append(" data=\"\"");
					}
					else if (ret >= CodecReturnCode.SUCCESS)
					{
						xmlString.Append(" data=\"");
						xmlString.Append(XmlDumpReal(oReal64));
						xmlString.Append("\"");
					}
					break;
				case DataTypes.ELEMENT_LIST:
					xmlString.Append(DecodeElementListToXML(iter, dictionary, (LocalElementSetDefDb)setDb, true));
					break;
				case DataTypes.ARRAY:
					xmlString.Append(DecodeArrayToXML(iter, dictionary));
					break;
				case DataTypes.FIELD_LIST:
					xmlString.Append(DecodeFieldListToXML(iter, dictionary, (LocalFieldSetDefDb)setDb, true));
					break;
				case DataTypes.FILTER_LIST:
					xmlString.Append(DecodeFilterListToXML(iter, dictionary, true));
					break;
				case DataTypes.VECTOR:
					xmlString.Append(DecodeVectorToXML(iter, dictionary, true));
					break;
				case DataTypes.MAP:
					xmlString.Append(DecodeMapToXML(iter, dictionary, true));
					break;
				case DataTypes.SERIES:
					xmlString.Append(DecodeSeriesToXML(iter, dictionary, true));
					break;
				case DataTypes.ANSI_PAGE:
					xmlString.Append(DecodeAnsiPageToXML(buffer, dictionary));
					break;
				case DataTypes.MSG:
					xmlString.Append(DecodeRwfMsgToXML(iter, dictionary));
					break;
				case DataTypes.NO_DATA:
					xmlString.Append("");
					break;
				case DataTypes.OPAQUE:
					xmlString.Append(DumpOpaqueToXML(buffer, dictionary));
					break;
				case DataTypes.JSON:
					xmlString.Append(DumpJSONToXML(buffer, dictionary));
					break;
				default:
					xmlString.Append(DumpOpaqueToXML(buffer, dictionary));
				break;
			}

			return xmlString.ToString();
		}

		private static string XmlDumpString(Buffer buf)
		{
			if (buf.Data() != null)
			{
				StringBuilder xmlString = new StringBuilder();
				int pos = buf.Position;
				byte c = buf.Data().Contents[pos++];

				if (c != 0x00 || buf.Length > 1) // skip stuff below for empty string
				{
					for (int i = 0; i < buf.GetLength(); i++)
					{
						if (c < 0x20 || c > 0x7e)
						{
							xmlString.Append("(");
							xmlString.Append("0x").AppendFormat(string.Format("{0:X2}", c));
							xmlString.Append(")");
						}
						else if (c == '<')
						{
							xmlString.Append("&lt;");
						}
						else if (c == '>')
						{
							xmlString.Append("&gt;");
						}
						else if (c == 0x26) // ampersand
						{
							xmlString.Append("&amp;");
						}
						else if (c == 0x22) // quotation mark
						{
							xmlString.Append("&quot;");
						}
						else if (c == 0x27) // apostrophe
						{
							xmlString.Append("&apos;");
						}
						else
						{
							// printable
							xmlString.Append((char)c);
						}

						if (pos < buf.Data().Limit)
						{
							c = buf.Data().Contents[pos++];
						}
					}
				}

				return xmlString.ToString();
			}
			else
			{
				return "null";
			}
		}

		private static string DumpOpaqueToXML(Buffer buffer, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			indents++;

			xmlString.Append("<opaque data=\"");
			xmlString.Append(XmlDumpHexBuffer(buffer));
			xmlString.Append("\" />").AppendLine();

			indents--;

			return xmlString.ToString();
		}

		private static string DumpJSONToXML(Buffer buffer, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			indents++;

			xmlString.Append("<json data=\"");
			xmlString.Append(XmlDumpString(buffer));
			xmlString.Append("\" />").AppendLine();

			indents--;

			return xmlString.ToString();
		}

		private static string DecodeRwfMsgToXML(DecodeIterator iter, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
            CodecReturnCode ret;
            IMsg msg = new Msg();
			DecodeIterator iterCopy = new DecodeIterator();

			// copy iterator contents
			CopyIteratorInfo(iterCopy, (DecodeIterator)iter);

			ret = msg.Decode(iterCopy);
			if (ret < CodecReturnCode.SUCCESS)
			{
				xmlString.Append("Error occurred while decoding Msg, CodecReturnCode=").Append(ret);
				return xmlString.ToString();
			}

			string tagName = MsgClasses.ToString(msg.MsgClass);

			xmlString.Append(XmlDumpMsgBegin(msg, tagName));

			xmlString.Append(DecodeMsgClassToXML(msg, iterCopy, dictionary));

			xmlString.Append(XmlDumpDataBodyBegin());

			xmlString.Append(DecodeDataTypeToXML(msg.ContainerType, msg.EncodedDataBody, dictionary, null, iterCopy));

			xmlString.Append(XmlDumpDataBodyEnd());

			xmlString.Append(XmlDumpMsgEnd(tagName));

			return xmlString.ToString();
		}

		private static string XmlDumpMsgEnd(string tagName)
		{
			StringBuilder xmlString = new StringBuilder();

			indents--;
			xmlString.Append(Encodeindents());
			xmlString.Append("</").Append(tagName).Append(">").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpDataBodyEnd()
		{
			indents--;

			return Encodeindents() + $"</dataBody>{NewLine}";
		}

		private static string XmlDumpDataBodyBegin()
		{
			string ret = Encodeindents() + $"<dataBody>{NewLine}";

			indents++;

			return ret;
		}

		private static string DecodeMsgClassToXML(IMsg msg, DecodeIterator iter, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);

			switch (msg.MsgClass)
			{
				case MsgClasses.UPDATE:
					IUpdateMsg updateMsg = (IUpdateMsg)msg;

					if (updateMsg.CheckHasMsgKey())
					{
						xmlString.Append(DecodeKeysToXML(updateMsg, iter, dictionary));
					}
					if (updateMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(updateMsg.ExtendedHeader));
					}
					break;
				case MsgClasses.REFRESH:
					IRefreshMsg refreshMsg = (IRefreshMsg)msg;

					if (refreshMsg.CheckHasMsgKey())
					{
						xmlString.Append(DecodeKeysToXML(refreshMsg, iter, dictionary));
					}
					if (refreshMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(refreshMsg.ExtendedHeader));
					}
					break;
				case MsgClasses.REQUEST:
					IRequestMsg requestMsg = (IRequestMsg)msg;

					xmlString.Append(DecodeKeysToXML(requestMsg, iter, dictionary));

					if (requestMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(requestMsg.ExtendedHeader));
					}
					break;
				case MsgClasses.GENERIC:
					IGenericMsg genericMsg = (IGenericMsg)msg;

					if (genericMsg.CheckHasMsgKey())
					{
						xmlString.Append(DecodeKeysToXML(genericMsg, iter, dictionary));
					}

					if (genericMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(genericMsg.ExtendedHeader));
					}
					break;
				case MsgClasses.POST:
					IPostMsg postMsg = (IPostMsg)msg;

					if (postMsg.CheckHasMsgKey())
					{
						xmlString.Append(DecodeKeysToXML(postMsg, iter, dictionary));
					}

					if (postMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(postMsg.ExtendedHeader));
					}
					break;
				case MsgClasses.STATUS:
					IStatusMsg statusMsg = (IStatusMsg)msg;

					if (statusMsg.CheckHasMsgKey())
					{
						xmlString.Append(DecodeKeysToXML(statusMsg, iter, dictionary));
					}

					if (statusMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(statusMsg.ExtendedHeader));
					}
					break;
				case MsgClasses.CLOSE:
					ICloseMsg closeMsg = (ICloseMsg)msg;

					if (closeMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(closeMsg.ExtendedHeader));
					}
					break;
				case MsgClasses.ACK:
					IAckMsg ackMsg = (IAckMsg)msg;

					if (ackMsg.CheckHasMsgKey())
					{
						xmlString.Append(DecodeKeysToXML(ackMsg, iter, dictionary));
					}

					if (ackMsg.CheckHasExtendedHdr())
					{
						xmlString.Append(XmlDumpExtendedHeader(ackMsg.ExtendedHeader));
					}
					break;
			}

			return xmlString.ToString();
		}

		private static string XmlDumpExtendedHeader(Buffer extendedHeader)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			xmlString.Append("<extendedHeader data=\"");
			xmlString.Append(XmlDumpHexBuffer(extendedHeader));
			xmlString.Append("\"/>").AppendLine();

			return xmlString.ToString();
		}

		private static string DecodeKeysToXML(IMsg msg, DecodeIterator iter, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(XmlDumpKeyBegin(msg.MsgKey));
			if (msg.MsgKey.CheckHasAttrib())
			{
				xmlString.Append(DecodeKeyOpaque(msg, iter, dictionary));
				xmlString.Append(XmlDumpKeyEnd());
			}

			return xmlString.ToString();
		}

		private static string XmlDumpKeyEnd()
		{
			indents--;

			return (Encodeindents() + $"</key>{NewLine}");
        }

		private static string DecodeKeyOpaque(IMsg msg, DecodeIterator iter, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
            CodecReturnCode ret;
			int attribContainerType = msg.MsgKey.AttribContainerType;

			xmlString.Append("<attrib>").AppendLine();

			indents++;

			ret = msg.DecodeKeyAttrib(iter, msg.MsgKey);
			if (ret < CodecReturnCode.SUCCESS)
			{
				xmlString.Append("Error occurred while decoding MsgKey, CodecReturnCode=").Append(ret);
				return xmlString.ToString();
			}

			switch (attribContainerType)
			{
				case DataTypes.OPAQUE:
					Buffer opaqueBufferValue = new Buffer();
					Decoders.DecodeBuffer(iter, opaqueBufferValue);
					xmlString.Append(DumpOpaqueToXML(opaqueBufferValue, dictionary));
					break;
				case DataTypes.FILTER_LIST:
					xmlString.Append(DecodeFilterListToXML(iter, dictionary, false));
					break;
				case DataTypes.ELEMENT_LIST:
					xmlString.Append(DecodeElementListToXML(iter, dictionary, null, false));
					break;
				case DataTypes.FIELD_LIST:
					xmlString.Append(DecodeFieldListToXML(iter, dictionary, null, false));
					break;
				case DataTypes.SERIES:
					xmlString.Append(DecodeSeriesToXML(iter, dictionary, false));
					break;
				case DataTypes.VECTOR:
					xmlString.Append(DecodeVectorToXML(iter, dictionary, false));
					break;
				case DataTypes.MAP:
					xmlString.Append(DecodeMapToXML(iter, dictionary, false));
					break;
				case DataTypes.XML:
					xmlString.Append(DataTypes.ToString(msg.MsgKey.AttribContainerType)).AppendLine();
					xmlString.Append(msg.MsgKey.EncodedAttrib.ToString());
					xmlString.AppendLine();
					break;
				case DataTypes.JSON:
					xmlString.Append(DumpJSONToXML(msg.MsgKey.EncodedAttrib, dictionary));
					break;
				case DataTypes.NO_DATA:
					xmlString.Append("");
					break;
				default:
					xmlString.Append(Encodeindents());
					xmlString.Append("Unknown data").AppendLine();
					break;
			}

			indents--;
			xmlString.Append(Encodeindents());
			xmlString.Append("</attrib>").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpKeyBegin(IMsgKey msgKey)
		{
			bool firstFlag = true;
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			xmlString.Append("<key");

			// print out flags
			xmlString.Append(" flags=\"0x").AppendFormat(string.Format("{0:X2}", msgKey.Flags));

			if (msgKey.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (msgKey.CheckHasServiceId())
			{
				xmlString.Append("HAS_SERVICE_ID");
				firstFlag = false;
			}
			if (msgKey.CheckHasName())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_NAME");
			}
			if (msgKey.CheckHasNameType())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_NAME_TYPE");
			}
			if (msgKey.CheckHasFilter())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_FILTER");
			}
			if (msgKey.CheckHasIdentifier())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_IDENTIFIER");
			}
			if (msgKey.CheckHasAttrib())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_ATTRIB");
			}

			if (msgKey.Flags != 0)
			{
				xmlString.Append(")");
			}
			xmlString.Append("\"");

			if (msgKey.CheckHasServiceId())
			{
				xmlString.Append(" serviceId=\"").Append(msgKey.ServiceId).Append("\"");
			}

			if (msgKey.CheckHasName())
			{
				xmlString.Append(" name=\"");
				xmlString.Append(XmlDumpBuffer(msgKey.Name));
				xmlString.Append("\"");
			}

			if (msgKey.CheckHasNameType())
			{
				xmlString.Append(" nameType=\"").Append(msgKey.NameType).Append("\"");
			}

			if (msgKey.CheckHasFilter())
			{
				xmlString.Append(" filter=\"").Append(msgKey.Filter).Append("\"");
			}

			if (msgKey.CheckHasIdentifier())
			{
				xmlString.Append(" identifier=\"").Append(msgKey.Identifier).Append("\"");
			}

			if (msgKey.CheckHasAttrib())
			{
				xmlString.Append(" attribContainerType=\"");
				xmlString.Append(XmlDumpDataType(msgKey.AttribContainerType));
				xmlString.Append("\">").AppendLine();
				indents++;
				xmlString.Append(Encodeindents());
			}
			else
			{
				xmlString.Append("/>").AppendLine();
			}

			return xmlString.ToString();
		}

		private static string DecodeAnsiPageToXML(Buffer buffer, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			indents++;

			xmlString.Append("<ansiPage data=\"");
			xmlString.Append(XmlDumpHexBuffer(buffer));
			xmlString.Append("\"/>").AppendLine();

			indents--;

			return xmlString.ToString();
		}

		private static string DecodeSeriesToXML(DecodeIterator iter, DataDictionary dictionary, bool copyIterator)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
			Series series = new Series();
			SeriesEntry row = new SeriesEntry();
            CodecReturnCode ret = 0;
			object setDb = null;

			DecodeIterator iterCopy;
			if (copyIterator)
			{
				iterCopy = new DecodeIterator();

                // copy iterator contents
                CopyIteratorInfo(iterCopy, (DecodeIterator)iter);
			}
			else
			{
				iterCopy = (DecodeIterator)iter;
			}

			ret = series.Decode(iterCopy);
			if (ret == CodecReturnCode.NO_DATA || ret < CodecReturnCode.SUCCESS)
			{
				return "";
			}

			xmlString.Append(XmlDumpSeriesBegin(series));

			if (series.CheckHasSetDefs())
			{
				if (series.ContainerType== DataTypes.FIELD_LIST)
				{
					LocalFieldSetDefDb flListSetDb = new LocalFieldSetDefDb();
					flListSetDb.Clear();
					ret = Decoders.DecodeLocalFieldSetDefDb(iterCopy, flListSetDb);
					if (ret >= CodecReturnCode.SUCCESS)
					{
						setDb = flListSetDb;
						xmlString.Append(XmlDumpLocalFieldSetDefDb(flListSetDb));
					}
					else
					{
						xmlString.Append("Error occurred while decoding FieldList SetDef contained in a Series, CodecReturnCode=").Append(ret);
						return xmlString.ToString();
					}
				}
				else
				{
					LocalElementSetDefDb elListSetDb = new LocalElementSetDefDb();
					elListSetDb.Clear();
					ret = Decoders.DecodeLocalElementSetDefDb(iterCopy, elListSetDb);
					if (ret >= CodecReturnCode.SUCCESS)
					{
						setDb = elListSetDb;
						xmlString.Append(XmlDumpLocalElementSetDefDb(elListSetDb));
					}
					else
					{
						xmlString.Append("Error occurred while decoding a SetDef contained in a Series, CodecReturnCode=").Append(ret);
						return xmlString.ToString();
					}
				}
			}

			/* dump summary data */
			if (series.CheckHasSummaryData())
			{
				xmlString.Append(DecodeSummaryData(iterCopy, series.ContainerType, series.EncodedSummaryData, iterCopy.MajorVersion(), iterCopy.MinorVersion(), dictionary, setDb));
			}

			if (Decoders.GetItemCount(iterCopy) == 0)
			{
				xmlString.Append(XmlDumpSeriesEnd());
				return xmlString.ToString();
			}

			while ((ret = row.Decode(iterCopy)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					xmlString.Append("Error occurred while decoding SeriesEntry, CodecReturnCode=").Append(ret);
					return xmlString.ToString();
				}

				xmlString.Append(XmlDumpSeriesRowBegin(row));
				xmlString.Append(DecodeDataTypeToXML(series.ContainerType, row.EncodedData, dictionary, setDb, iterCopy));
				xmlString.Append(XmlDumpSeriesRowEnd());
			}

			xmlString.Append(XmlDumpSeriesEnd());

			return xmlString.ToString();
		}

		private static string DecodeMapToXML(DecodeIterator iter, DataDictionary dictionary, bool copyIterator)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
			Map map = new Map();
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
			object setDb = null;
			MapEntry mapEntry = new MapEntry();
			object mapKeyData = null;

			DecodeIterator iterCopy;
			if (copyIterator)
			{
				iterCopy = new DecodeIterator();

                // copy iterator contents
                CopyIteratorInfo(iterCopy, (DecodeIterator)iter);
			}
			else
			{
				iterCopy = (DecodeIterator)iter;
			}

			ret = map.Decode(iterCopy);
			if (ret == CodecReturnCode.NO_DATA || ret < CodecReturnCode.SUCCESS)
			{
				return "";
			}

			xmlString.Append(XmlDumpMapBegin(map));

			if (map.CheckHasSetDefs())
			{
				if (map.ContainerType== DataTypes.FIELD_LIST)
				{
					LocalFieldSetDefDb flListSetDb = new LocalFieldSetDefDb();
					flListSetDb.Clear();
					ret = Decoders.DecodeLocalFieldSetDefDb(iterCopy, flListSetDb);
					if (ret >= CodecReturnCode.SUCCESS)
					{
						setDb = flListSetDb;
						xmlString.Append(XmlDumpLocalFieldSetDefDb(flListSetDb));
					}
					else
					{
						xmlString.Append("Error occurred while decoding FieldList SetDef contained in a Map, CodecReturnCode=").Append(ret);
						return xmlString.ToString();
					}
				}
				else
				{
					LocalElementSetDefDb elListSetDb = new LocalElementSetDefDb();
					elListSetDb.Clear();
					ret = Decoders.DecodeLocalElementSetDefDb(iterCopy, elListSetDb);
					if (ret >= CodecReturnCode.SUCCESS)
					{
						setDb = elListSetDb;
						xmlString.Append(XmlDumpLocalElementSetDefDb(elListSetDb));
					}
					else
					{
						xmlString.Append("Error occurred while decoding a SetDef contained in a Map, CodecReturnCode=").Append(ret);
						return xmlString.ToString();
					}
				}
			}

			/* dump summary data */
			if (map.CheckHasSummaryData())
			{
				xmlString.Append(DecodeSummaryData(iterCopy, map.ContainerType, map.EncodedSummaryData, iterCopy.MajorVersion(), iterCopy.MinorVersion(), dictionary, setDb));
			}

			mapEntry.Clear();
			mapKeyData = CreateKeyData(map.KeyPrimitiveType);
			while ((ret = mapEntry.Decode(iterCopy, mapKeyData)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					xmlString.Append("Error occurred while decoding MapEntry, CodecReturnCode=").Append(ret);
					return xmlString.ToString();
				}

				xmlString.Append(XmlDumpMapEntryBegin(map.KeyPrimitiveType, mapEntry, mapKeyData));
				if (mapEntry.Action != MapEntryActions.DELETE)
                {
					xmlString.Append(DecodeDataTypeToXML(map.ContainerType, mapEntry.EncodedData, dictionary, setDb, iterCopy));
				}
				xmlString.Append(XmlDumpMapEntryEnd());

				mapEntry.Clear();
			}

			xmlString.Append(XmlDumpMapEnd());

			return xmlString.ToString();
		}

		private static string DecodeVectorToXML(DecodeIterator iter, DataDictionary dictionary, bool copyIterator)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
			Vector vec = new Vector();
            CodecReturnCode ret = 0;
			object setDb = null;
			VectorEntry vectorEntry = new VectorEntry();

			DecodeIterator iterCopy;
			if (copyIterator)
			{
				iterCopy = new DecodeIterator();

                // copy iterator contents
                CopyIteratorInfo(iterCopy, (DecodeIterator)iter);
			}
			else
			{
				iterCopy = (DecodeIterator)iter;
			}

			ret = vec.Decode(iterCopy);
			if (ret == CodecReturnCode.NO_DATA || ret < CodecReturnCode.SUCCESS)
			{
				return "";
			}

			xmlString.Append(XmlDumpVectorBegin(vec));

			if (vec.CheckHasSetDefs())
			{
				if (vec.ContainerType== DataTypes.FIELD_LIST)
				{
					LocalFieldSetDefDb flListSetDb =new LocalFieldSetDefDb();
					flListSetDb.Clear();
					ret = Decoders.DecodeLocalFieldSetDefDb(iterCopy, flListSetDb);
					if (ret >= CodecReturnCode.SUCCESS)
					{
						setDb = flListSetDb;
						xmlString.Append(XmlDumpLocalFieldSetDefDb(flListSetDb));
					}
					else
					{
						xmlString.Append("Error occurred while decoding Vector SetDef contained in a Series, CodecReturnCode=").Append(ret);
						return xmlString.ToString();
					}
				}
				else
				{
					LocalElementSetDefDb elListSetDb = new LocalElementSetDefDb();
					elListSetDb.Clear();
					ret = Decoders.DecodeLocalElementSetDefDb(iterCopy, elListSetDb);
					if (ret >= CodecReturnCode.SUCCESS)
					{
						setDb = elListSetDb;
						xmlString.Append(XmlDumpLocalElementSetDefDb(elListSetDb));
					}
					else
					{
						xmlString.Append("Error occurred while decoding a SetDef contained in a Series, CodecReturnCode=").Append(ret);
						return xmlString.ToString();
					}
				}
			}

			/* dump summary data */
			if (vec.CheckHasSummaryData())
			{
				xmlString.Append(DecodeSummaryData(iterCopy, vec.ContainerType, vec.EncodedSummaryData, iterCopy.MajorVersion(), iterCopy.MinorVersion(), dictionary, setDb));
			}

			vectorEntry.Clear();
			while ((ret = vectorEntry.Decode(iterCopy)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					xmlString.Append("Error occurred while decoding VectorEntry, CodecReturnCode=").Append(ret);
					return xmlString.ToString();
				}

				xmlString.Append(XmlDumpVectorEntryBegin(vectorEntry));
				if (vectorEntry.Action != VectorEntryActions.CLEAR && vectorEntry.Action != VectorEntryActions.DELETE)
                {
					xmlString.Append(DecodeDataTypeToXML(vec.ContainerType, vectorEntry.EncodedData, dictionary, setDb, iterCopy));
				}
				xmlString.Append(XmlDumpVectorEntryEnd());

				vectorEntry.Clear();
			}

			xmlString.Append(XmlDumpVectorEnd());

			return xmlString.ToString();
		}

		private static string DecodeFilterListToXML(DecodeIterator iter, DataDictionary dictionary, bool copyIterator)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
			FilterList fList = new FilterList();
			FilterEntry filterItem = new FilterEntry();
            CodecReturnCode ret = 0;

			DecodeIterator iterCopy;
			if (copyIterator)
			{
				iterCopy = new DecodeIterator();

                // copy iterator contents
                CopyIteratorInfo(iterCopy, iter);
			}
			else
			{
				iterCopy = iter;
			}

			ret = fList.Decode(iterCopy);
			if (ret == CodecReturnCode.NO_DATA || ret < CodecReturnCode.SUCCESS)
			{
				return "";
			}

			xmlString.Append(XmlDumpFilterListBegin(fList));

			while ((ret = filterItem.Decode(iterCopy)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					xmlString.Append("Error occurred while decoding FilterEntry, CodecReturnCode=").Append(ret);
					return xmlString.ToString();
				}

				xmlString.Append(XmlDumpFilterItemBegin(filterItem));
				if (filterItem.Action != FilterEntryActions.CLEAR)
                {
					xmlString.Append(DecodeDataTypeToXML(filterItem.ContainerType, filterItem.EncodedData, dictionary, null, iterCopy));
				}				
				xmlString.Append(XmlDumpFilterItemEnd());
			}

			xmlString.Append(XmlDumpFilterListEnd());

			return xmlString.ToString();
		}

		private static string DecodeFieldListToXML(DecodeIterator iter, DataDictionary dictionary, LocalFieldSetDefDb setDb, bool copyIterator)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
            CodecReturnCode ret = 0;
			int dataType = DataTypes.UNKNOWN;
            FieldList fList = new FieldList();
			FieldEntry field = new FieldEntry();

			DecodeIterator iterCopy;
			if (copyIterator)
			{
				iterCopy = new DecodeIterator();

                // copy iterator contents
                CopyIteratorInfo(iterCopy, iter);
			}
			else
			{
				iterCopy = iter;
			}

			ret = fList.Decode(iterCopy, setDb);
			if (ret == CodecReturnCode.NO_DATA || ret < CodecReturnCode.SUCCESS)
			{
				return "";
			}

			xmlString.Append(XmlDumpFieldListBegin(fList));

			while ((ret = field.Decode(iterCopy)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					xmlString.Append("Error occurred while decoding FieldEntry, CodecReturnCode=").Append(ret);
					return xmlString.ToString();
				}

				if (dictionary == null || dictionary.Entry(field.FieldId) == null)
				{
					dataType = field.DataType;
				}
				else
				{
					dataType = dictionary.Entry(field.FieldId).GetRwfType();
				}

				xmlString.Append(XmlDumpFieldBegin(field, dataType));

				if (Encoders.ValidAggregateDataType(dataType) || dataType == DataTypes.ARRAY)
				{
					xmlString.Append(">").AppendLine();
				}

				if (dataType != DataTypes.UNKNOWN)
				{
					xmlString.Append(DecodeDataTypeToXML(dataType, field.EncodedData, dictionary, setDb, iterCopy));
				}
				else
				{
					xmlString.Append(" data=\"");
					xmlString.Append(XmlDumpHexBuffer(field.EncodedData));
					xmlString.Append("\"");
				}
				if (Encoders.ValidAggregateDataType(dataType) || dataType == DataTypes.ARRAY)
				{
					xmlString.Append(XmlDumpFieldEnd());
				}
				else
				{
					xmlString.Append(XmlDumpEndNoTag());
				}
			}

			xmlString.Append(XmlDumpFieldListEnd());

			return xmlString.ToString();
		}

		private static string DecodeArrayToXML(DecodeIterator iter, DataDictionary dictionary)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
			Array array = new Array();
			ArrayEntry arrayEntry = new ArrayEntry();
            CodecReturnCode ret = 0;
			DecodeIterator iterCopy = new DecodeIterator();

            // copy iterator contents
            CopyIteratorInfo(iterCopy, (DecodeIterator)iter);

			ret = array.Decode(iterCopy);
			if (ret == CodecReturnCode.NO_DATA || ret < CodecReturnCode.SUCCESS)
			{
				return "";
			}

			xmlString.Append(XmlDumpArrayBegin(array));

			while ((ret = arrayEntry.Decode(iterCopy)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					xmlString.Append("Error occurred while decoding ArrayEntry, CodecReturnCode=").Append(ret);
					return xmlString.ToString();
				}

				xmlString.Append(XmlDumpArrayItemBegin());

				Buffer arrayItem = new Buffer();
				xmlString.Append(DecodeDataTypeToXML(array.PrimitiveType, arrayItem, dictionary, null, iterCopy));

				xmlString.Append(XmlDumpArrayItemEnd());
			}

			xmlString.Append(XmlDumpArrayEnd());

			return xmlString.ToString();
		}

		private static string DecodeElementListToXML(DecodeIterator iter, DataDictionary dictionary, LocalElementSetDefDb setDb, bool copyIterator)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
            CodecReturnCode ret = 0;
			ElementList eList = new ElementList();
			ElementEntry element = new ElementEntry();

			DecodeIterator iterCopy;
			if (copyIterator)
			{
				iterCopy = new DecodeIterator();

                // copy iterator contents
                CopyIteratorInfo(iterCopy, (DecodeIterator)iter);
			}
			else
			{
				iterCopy = (DecodeIterator)iter;
			}

			ret = Decoders.DecodeElementList(iterCopy, eList, setDb);
			if (ret == CodecReturnCode.NO_DATA || ret < CodecReturnCode.SUCCESS)
			{
				return "";
			}

			xmlString.Append(XmlDumpElementListBegin(eList));

			while ((ret = Decoders.DecodeElementEntry(iterCopy, element)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < CodecReturnCode.SUCCESS)
				{
					xmlString.Append("Error occurred while decoding ElementEntry, CodecReturnCode=").Append(ret);
					return xmlString.ToString();
				}

				xmlString.Append(XmlDumpElementBegin(element));
				if ((Encoders.ValidAggregateDataType(element.DataType)) || (element.DataType == DataTypes.ARRAY))
				{
					xmlString.Append(">").AppendLine();
				}
				if (element.Name.Equals(ElementNames.GROUP))
				{
					xmlString.Append(" data=\"");
					xmlString.Append(XmlDumpGroupId(element.EncodedData));
					xmlString.Append("\"");
				}
				else if (element.Name.Equals(ElementNames.MERG_TO_GRP))
				{
					xmlString.Append(" data=\"");
					xmlString.Append(XmlDumpGroupId(element.EncodedData));
					xmlString.Append("\"");
				}
				else
				{
					xmlString.Append(DecodeDataTypeToXML(element.DataType, element.EncodedData, dictionary, setDb, iterCopy));
				}

				if ((Encoders.ValidAggregateDataType(element.DataType)) || (element.DataType == DataTypes.ARRAY))
				{
					xmlString.Append(XmlDumpElementEnd());
				}
				else
				{
					xmlString.Append(XmlDumpEndNoTag());
				}
			}

			xmlString.Append(XmlDumpElementListEnd());

			return xmlString.ToString();
		}

		private static string XmlDumpElementListEnd()
		{
			indents--;

			return (Encodeindents() + $"</elementList>{NewLine}");
        }

		private static string XmlDumpEndNoTag()
		{
			indents--;

			return $"/>{NewLine}";
		}

		private static string XmlDumpElementEnd()
		{
			indents--;

			return (Encodeindents() + $"</elementEntry>{NewLine}");
        }

		private static string XmlDumpGroupId(Buffer buffer)
		{
			StringBuilder xmlString = new StringBuilder();
			short tempVal = 0;
			int index = buffer.Position;
			bool printPeriod = false;
			byte[] shortBytes = new byte[2];

			for (int i = 0; i < buffer.GetLength(); i++)
			{
				if (printPeriod)
				{
					xmlString.Append(".");
				}
				if (index < buffer.Data().Limit - 2)
				{
					shortBytes[0] = (byte)(buffer.Data().Contents[index++]);
					shortBytes[1] = (byte)(buffer.Data().Contents[index++]);
				}
				tempVal = shortBytes[0];
				tempVal <<= 8;
				tempVal |= (short)shortBytes[1];
				xmlString.Append(tempVal);
				printPeriod = true;
				i++;
			}

			return xmlString.ToString();
		}

		private static string XmlDumpElementBegin(ElementEntry element)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			xmlString.Append("<elementEntry name=\"");
			xmlString.Append(XmlDumpBuffer(element.Name));
			xmlString.Append("\" dataType=\"");
			xmlString.Append(XmlDumpDataType(element.DataType));
			xmlString.Append("\"");

			indents++;

			return xmlString.ToString();
		}

		private static string XmlDumpDataType(int dataType)
		{
			StringBuilder xmlString = new StringBuilder();
			string str = DataTypes.ToString(dataType);

			if (str.Length == 0)
			{
				xmlString.Append(dataType);
			}
			else
			{
				xmlString.Append(str);
			}

			return xmlString.ToString();
		}

		private static string XmlDumpDomainType(int domainType)
		{
			StringBuilder xmlString = new StringBuilder();
			string str = DomainTypes.ToString(domainType);

			if (str.Length == 0)
			{
				xmlString.Append(domainType);
			}
			else
			{
				xmlString.Append(str);
			}

			return xmlString.ToString();
		}

		private static string XmlDumpBuffer(Buffer buffer)
		{
			string ret = "";

			if (buffer.Length > 0)
			{
				ret = buffer.ToString();
			}

			return ret;
		}

		private static string XmlDumpElementListBegin(ElementList eList)
		{
			StringBuilder xmlString = new StringBuilder();
			bool firstFlag = true;

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<elementList flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)eList.Flags));

			if (eList.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (eList.CheckHasInfo())
			{
				xmlString.Append("HAS_ELEMENT_LIST_INFO");
				firstFlag = false;
			}

			if (eList.CheckHasSetData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_SET_DATA");
			}

			if (eList.CheckHasSetId())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_SET_ID");
			}

			if (eList.CheckHasStandardData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_STANDARD_DATA");
			}
			if (eList.Flags != 0)
			{
				xmlString.Append(")");
			}
			xmlString.Append("\"");

			if (eList.CheckHasInfo())
			{
				xmlString.Append(" elementListNum=\"").Append(eList.ElementListNum).Append("\"");
			}
			if (eList.CheckHasSetData())
			{
				if (eList.CheckHasSetId())
				{
					xmlString.Append(" setId=\"").Append(eList.SetId).Append("\"");
				}

			}
			xmlString.Append(">").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpReal(Real oReal64)
		{
			return oReal64.ToString();
		}

		private static string XmlDumpTime(Time time)
		{
			return time.ToString();
		}

		private static string XmlDumpDate(Date date)
		{
			return date.ToString();
		}

		private static string XmlDumpDateTime(DateTime datetime)
		{
			return datetime.ToString();
		}

		private static string XmlDumpState(State state)
		{
			return (state.ToString().Replace("\"", "\'"));
		}

		private static string XmlDumpQos(Qos qos)
		{
			return (qos.ToString());
		}

		private static string XmlDumpWorstQos(Qos qos)
		{
			return (qos.ToString());
		}

		private static string XmlDumpUInt(UInt u64)
		{
			return u64.ToString();
		}

		private static string XmlDumpInt(Int i64)
		{
			return i64.ToString();
		}

		private static string XmlDumpDouble(Float f32)
		{
			return f32.ToString();
		}

		private static string XmlDumpDouble(Double d64)
		{
			return d64.ToString();
		}

		private static string XmlDumpMsgBegin(IMsg msg, string tagName)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);
			bool firstFlag = true;

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<").Append(tagName);
			xmlString.Append(" domainType=\"");
			xmlString.Append(XmlDumpDomainType(msg.DomainType));
			xmlString.Append("\" streamId=\"").Append(msg.StreamId).Append("\" containerType=\"");
			xmlString.Append(XmlDumpDataType(msg.ContainerType));
			switch (msg.MsgClass)
			{
				case MsgClasses.UPDATE:
					IUpdateMsg updateMsg = (IUpdateMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", updateMsg.Flags));

					if (updateMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}

					if (updateMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (updateMsg.CheckHasPermData())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PERM_DATA");
					}
					if (updateMsg.CheckHasMsgKey())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_MSG_KEY");
					}
					if (updateMsg.CheckHasSeqNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_SEQ_NUM");
					}
					if (updateMsg.CheckHasConfInfo())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_CONF_INFO");
					}
					if (updateMsg.CheckDoNotCache())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("DO_NOT_CACHE");
					}
					if (updateMsg.CheckDoNotConflate())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("DO_NOT_CONFLATE");
					}
					if (updateMsg.CheckDoNotRipple())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("DO_NOT_RIPPLE");
					}
					if (updateMsg.CheckHasPostUserInfo())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_POST_USER_INFO");
					}
					if (updateMsg.CheckDiscardable())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("DISCARDABLE");
					}

					if (updateMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");

					xmlString.Append(" updateType=\"").Append(updateMsg.UpdateType).Append("\"");
					if (updateMsg.CheckHasSeqNum())
					{
						xmlString.Append(" seqNum=\"").Append(updateMsg.SeqNum).Append("\"");
					}

					if (updateMsg.CheckHasPermData())
					{
						xmlString.Append(" permData=\"");
						xmlString.Append(XmlDumpHexBuffer(updateMsg.PermData));
						xmlString.Append("\"");
					}

					if (updateMsg.CheckHasConfInfo())
					{
						xmlString.Append(" conflationCount=\"").Append(updateMsg.ConflationCount).Append("\" conflationTime=\"").Append(updateMsg.ConflationTime).Append("\"");
					}

					if (updateMsg.CheckHasPostUserInfo())
					{
						xmlString.Append(" postUserId=\"").Append(updateMsg.PostUserInfo.UserId).Append("\" postUserAddr=\"").Append(updateMsg.PostUserInfo.UserAddrToString(updateMsg.PostUserInfo.UserAddr)).Append("\"");
					}
					break;
				case MsgClasses.GENERIC:
					IGenericMsg genericMsg = (IGenericMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", genericMsg.Flags));

					if (genericMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}
					if (genericMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (genericMsg.CheckHasPermData())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PERM_DATA");
					}
					if (genericMsg.CheckHasMsgKey())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_MSG_KEY");
					}
					if (genericMsg.CheckHasSeqNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_SEQ_NUM");
					}
					if (genericMsg.CheckMessageComplete())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("MESSAGE_COMPLETE");
					}
					if (genericMsg.CheckHasSecondarySeqNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_SECONDARY_SEQ_NUM");
					}
					if (genericMsg.CheckHasPartNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PART_NUM");
					}
					if (genericMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");

					if (genericMsg.CheckHasSeqNum())
					{
						xmlString.Append(" seqNum=\"").Append(genericMsg.SeqNum).Append("\"");
					}

					if (genericMsg.CheckHasSecondarySeqNum())
					{
						xmlString.Append(" secondarySeqNum=\"").Append(genericMsg.SecondarySeqNum).Append("\"");
					}

					if (genericMsg.CheckHasPartNum())
					{
						xmlString.Append(" partNum=\"").Append(genericMsg.PartNum).Append("\"");
					}

					if (genericMsg.CheckHasPermData())
					{
						xmlString.Append(" permData=\"");
						xmlString.Append(XmlDumpHexBuffer(genericMsg.PermData));
						xmlString.Append("\"");
					}
					break;
				case MsgClasses.REFRESH:
					IRefreshMsg refreshMsg = (IRefreshMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", refreshMsg.Flags));

					if (refreshMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}
					if (refreshMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (refreshMsg.CheckHasPermData())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PERM_DATA");
					}
					if (refreshMsg.CheckHasMsgKey())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_MSG_KEY");
					}
					if (refreshMsg.CheckHasSeqNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_SEQ_NUM");
					}
					if (refreshMsg.CheckSolicited())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("SOLICITED");
					}
					if (refreshMsg.CheckRefreshComplete())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("REFRESH_COMPLETE");
					}
					if (refreshMsg.CheckHasQos())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_QOS");
					}
					if (refreshMsg.CheckClearCache())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("CLEAR_CACHE");
					}
					if (refreshMsg.CheckDoNotCache())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("DO_NOT_CACHE");
					}
					if (refreshMsg.CheckPrivateStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("PRIVATE_STREAM");
					}
					if (refreshMsg.CheckQualifiedStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("QUALIFIED_STREAM");
					}
					if (refreshMsg.CheckHasPostUserInfo())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_POST_USER_INFO");
					}
					if (refreshMsg.CheckHasPartNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PART_NUM");
					}
					if (refreshMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");

					xmlString.Append(" groupId=\"");
					xmlString.Append(XmlDumpGroupId(refreshMsg.GroupId));
					xmlString.Append("\"");

					if (refreshMsg.CheckHasSeqNum())
					{
						xmlString.Append(" seqNum=\"").Append(refreshMsg.SeqNum).Append("\"");
					}

					if (refreshMsg.CheckHasPartNum())
					{
						xmlString.Append(" partNum=\"").Append(refreshMsg.PartNum).Append("\"");
					}

					if (refreshMsg.CheckHasPermData())
					{
						xmlString.Append(" permData=\"");
						xmlString.Append(XmlDumpHexBuffer(refreshMsg.PermData));
						xmlString.Append("\"");
					}

					if (refreshMsg.CheckHasQos())
					{
						xmlString.Append(" qos=\"");
						xmlString.Append(XmlDumpQos(refreshMsg.Qos));
						xmlString.Append("\"");
					}

					xmlString.Append(" state=\"");
					xmlString.Append(XmlDumpState(refreshMsg.State));
					xmlString.Append("\"");

					if (refreshMsg.CheckHasPostUserInfo())
					{
						xmlString.Append(" postUserId=\"").Append(refreshMsg.PostUserInfo.UserId).Append("\" postUserAddr=\"").Append(refreshMsg.PostUserInfo.UserAddrToString(refreshMsg.PostUserInfo.UserAddr)).Append("\"");
					}
					break;
				case MsgClasses.POST:
					IPostMsg postMsg = (IPostMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", postMsg.Flags));

					if (postMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}
					if (postMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (postMsg.CheckHasPostId())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_POST_ID");
					}
					if (postMsg.CheckHasMsgKey())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_MSG_KEY");
					}
					if (postMsg.CheckHasSeqNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_SEQ_NUM");
					}
					if (postMsg.CheckPostComplete())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("POST_COMPLETE");
					}
					if (postMsg.CheckAck())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("ACK");
					}
					if (postMsg.CheckHasPermData())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PERM_DATA");
					}
					if (postMsg.CheckHasPartNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PART_NUM");
					}
					if (postMsg.CheckHasPostUserRights())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_POST_USER_RIGHTS");
					}
					if (postMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");

					if (postMsg.CheckHasSeqNum())
					{
						xmlString.Append(" seqNum=\"").Append(postMsg.SeqNum).Append("\"");
					}

					if (postMsg.CheckHasPostId())
					{
						xmlString.Append(" postId=\"").Append(postMsg.PostId).Append("\"");
					}

					if (postMsg.CheckHasPermData())
					{
						xmlString.Append(" permData=\"");
						xmlString.Append(XmlDumpHexBuffer(postMsg.PermData));
						xmlString.Append("\"");
					}

					if (postMsg.CheckHasPartNum())
					{
						xmlString.Append(" partNum=\"").Append(postMsg.PartNum).Append("\"");
					}

					if (postMsg.CheckHasPostUserRights())
					{
						xmlString.Append(" postUserRights=\"").Append(postMsg.PostUserRights).Append("\"");
					}

					/* print user info */
					xmlString.Append(" postUserId=\"").Append(postMsg.PostUserInfo.UserId).Append("\" postUserAddr=\"").Append(postMsg.PostUserInfo.UserAddrToString(postMsg.PostUserInfo.UserAddr)).Append("\"");
					break;
				case MsgClasses.REQUEST:
					IRequestMsg requestMsg = (IRequestMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", requestMsg.Flags));

					if (requestMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}
					if (requestMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (requestMsg.CheckHasPriority())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PRIORITY");
					}
					if (requestMsg.CheckStreaming())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("STREAMING");
					}
					if (requestMsg.CheckMsgKeyInUpdates())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("MSG_KEY_IN_UPDATES");
					}
					if (requestMsg.CheckConfInfoInUpdates())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("CONF_INFO_IN_UPDATES");
					}
					if (requestMsg.CheckNoRefresh())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("NO_REFRESH");
					}
					if (requestMsg.CheckHasQos())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_QOS");
					}
					if (requestMsg.CheckHasWorstQos())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_WORST_QOS");
					}
					if (requestMsg.CheckPrivateStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("PRIVATE_STREAM");
					}
					if (requestMsg.CheckQualifiedStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("QUALIFIED_STREAM");
					}
					if (requestMsg.CheckPause())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("PAUSE");
					}
					if (requestMsg.CheckHasView())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_VIEW");
					}
					if (requestMsg.CheckHasBatch())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_BATCH");
					}
					if (requestMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");

					if (requestMsg.CheckHasQos())
					{
						xmlString.Append(" qos=\"");
						xmlString.Append(XmlDumpQos(requestMsg.Qos));
						xmlString.Append("\"");
					}
					if (requestMsg.CheckHasWorstQos())
					{
						xmlString.Append(" worstQos=\"");
						xmlString.Append(XmlDumpWorstQos(requestMsg.WorstQos));
						xmlString.Append("\"");
					}

					if (requestMsg.CheckHasPriority())
					{
						xmlString.Append(" priorityClass=\"").Append(requestMsg.Priority.PriorityClass).Append("\" priorityCount=\"").Append(requestMsg.Priority.Count).Append("\"");
					}
					break;
				case MsgClasses.STATUS:
					IStatusMsg statusMsg = (IStatusMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", statusMsg.Flags));

					if (statusMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}
					if (statusMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (statusMsg.CheckHasPermData())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_PERM_DATA");
					}
					if (statusMsg.CheckHasMsgKey())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_MSG_KEY");
					}
					if (statusMsg.CheckHasGroupId())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_GROUP_ID");
					}
					if (statusMsg.CheckHasState())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_STATE");
					}
					if (statusMsg.CheckClearCache())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("CLEAR_CACHE");
					}
					if (statusMsg.CheckPrivateStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("PRIVATE_STREAM");
					}
					if (statusMsg.CheckQualifiedStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("QUALIFIED_STREAM");
					}
					if (statusMsg.CheckHasPostUserInfo())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_POST_USER_INFO");
					}
					if (statusMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");

					if (statusMsg.CheckHasGroupId())
					{
						xmlString.Append(" groupId=\"");
						xmlString.Append(XmlDumpGroupId(statusMsg.GroupId));
						xmlString.Append("\"");
					}

					if (statusMsg.CheckHasPermData())
					{
						xmlString.Append(" permData=\"");
						xmlString.Append(XmlDumpHexBuffer(statusMsg.PermData));
						xmlString.Append("\"");
					}

					if (statusMsg.CheckHasState())
					{
						xmlString.Append(" state=\"");
						xmlString.Append(XmlDumpState(statusMsg.State));
						xmlString.Append("\"");
					}

					if (statusMsg.CheckHasPostUserInfo())
					{
						xmlString.Append(" postUserId=\"").Append(statusMsg.PostUserInfo.UserId).Append("\" postUserAddr=\"").Append(statusMsg.PostUserInfo.UserAddrToString(statusMsg.PostUserInfo.UserAddr)).Append("\"");
					}
					break;
				case MsgClasses.CLOSE:
					ICloseMsg closeMsg = (ICloseMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", closeMsg.Flags));
					if (closeMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}
					if (closeMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (closeMsg.CheckAck())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("ACK");
					}
					if (closeMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");
					break;
				case MsgClasses.ACK:
					IAckMsg ackMsg = (IAckMsg)msg;
					xmlString.Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", ackMsg.Flags));

					if (ackMsg.Flags != 0)
					{
						xmlString.Append(" (");
					}
					if (ackMsg.CheckHasExtendedHdr())
					{
						xmlString.Append("HAS_EXTENDED_HEADER");
						firstFlag = false;
					}
					if (ackMsg.CheckHasText())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_TEXT");
					}
					if (ackMsg.CheckPrivateStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("PRIVATE_STREAM");
					}
					if (ackMsg.CheckQualifiedStream())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("QUALIFIED_STREAM");
					}
					if (ackMsg.CheckHasSeqNum())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_SEQ_NUM");
					}
					if (ackMsg.CheckHasMsgKey())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_MSG_KEY");
					}
					if (ackMsg.CheckHasNakCode())
					{
						if (!firstFlag)
						{
							xmlString.Append("|");
						}
						else
						{
							firstFlag = false;
						}
						xmlString.Append("HAS_NAK_CODE");
					}
					if (ackMsg.Flags != 0)
					{
						xmlString.Append(")");
					}
					xmlString.Append("\"");

					xmlString.Append(" ackId=\"").Append(ackMsg.AckId).Append("\"");
					if (ackMsg.CheckHasNakCode())
					{
						xmlString.Append(" nakCode=\"").Append(NakCodes.ToString(ackMsg.NakCode)).Append("\"");
					}

					if (ackMsg.CheckHasText())
					{
						xmlString.Append(" text=\"");
						xmlString.Append(XmlDumpBuffer(ackMsg.Text));
						xmlString.Append("\"");
					}

					if (ackMsg.CheckHasSeqNum())
					{
						xmlString.Append(" seqNum=\"").Append(ackMsg.SeqNum).Append("\"");
					}
					break;
				default:
					xmlString.Append("\"");
				break;
			}
			xmlString.Append(" dataSize=\"").Append(msg.EncodedDataBody.GetLength()).Append("\">").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpHexBuffer(Buffer buffer)
		{
			StringBuilder xmlString = new StringBuilder();
			int bufPosition = buffer.Position;
			byte bufByte;

			for (int i = 0; i < buffer.GetLength(); i++)
			{
				bufByte = buffer.Data().Contents[i + bufPosition];
				if (i % 32 == 0)
				{
					if (i != 0)
					{
						xmlString.AppendLine();
						xmlString.Append(Encodeindents());
					}
				}
				else if ((i != 0) && (i % 2 == 0))
				{
					xmlString.Append(" ");
				}
				xmlString.Append(string.Format("{0:X2}", bufByte));
			}

			return xmlString.ToString();
		}

		private static string XmlDumpFieldListEnd()
		{
			indents--;

			return (Encodeindents() + $"</fieldList>{NewLine}");
        }

		private static string XmlDumpFieldEnd()
		{
			indents--;

			return (Encodeindents() + $"</fieldEntry>{NewLine}");
        }

		private static string XmlDumpFieldBegin(FieldEntry field, int dataType)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			xmlString.Append("<fieldEntry fieldId=\"").Append(field.FieldId);
			if (dataType != DataTypes.UNKNOWN)
			{
				xmlString.Append("\" dataType=\"");
				xmlString.Append(XmlDumpDataType(dataType));
			}
			xmlString.Append("\"");
			indents++;

			return xmlString.ToString();
		}

		private static string XmlDumpFieldListBegin(FieldList fList)
		{
			StringBuilder xmlString = new StringBuilder();
			bool firstFlag = true;

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<fieldList flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)fList.Flags));

			if (fList.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (fList.CheckHasInfo())
			{
				xmlString.Append("HAS_FIELD_LIST_INFO");
				firstFlag = false;
			}

			if (fList.CheckHasSetData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_SET_DATA");
			}

			if (fList.CheckHasSetId())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_SET_ID");
			}

			if (fList.CheckHasStandardData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_STANDARD_DATA");
			}
			if (fList.Flags != 0)
			{
				xmlString.Append(")");
			}
			xmlString.Append("\"");

			if (fList.CheckHasInfo())
			{
				xmlString.Append(" fieldListNum=\"").Append(fList.FieldListNum).Append("\" dictionaryId=\"").Append(fList.DictionaryId).Append("\"");
			}
			if (fList.CheckHasSetData())
			{
				if (fList.CheckHasSetId())
				{
					xmlString.Append(" setId=\"").Append(fList.SetId).Append("\"");
				}
			}
			xmlString.Append(">").AppendLine();

			return xmlString.ToString();
		}

		private static string DecodeSummaryData(DecodeIterator iter, int containerType, Buffer input, int majorVer, int minorVer, DataDictionary dictionary, object setDb)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);

			xmlString.Append(XmlDumpSummaryDataBegin());

			xmlString.Append(DecodeDataTypeToXML(containerType, input, dictionary, setDb, iter));

			xmlString.Append(XmlDumpSummaryDataEnd());

			return xmlString.ToString();
		}

		private static string XmlDumpMapEnd()
		{
			indents--;

			return Encodeindents() + $"</map>{NewLine}";
        }

		private static string XmlDumpMapEntryEnd()
		{
			indents--;

			return Encodeindents() + $"</mapEntry>{NewLine}";
        }

		private static string XmlDumpMapEntryBegin(int keyPrimitiveType, MapEntry mapEntry, object mapKeyData)
		{
			StringBuilder xmlString = new StringBuilder();
			string actionString;
			Buffer stringBuf = new Buffer();

			xmlString.Append(Encodeindents());
			indents++;

			switch (mapEntry.Action)
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

				break;
			}
			/* Don't print the data element for deleted rows, there should not be any. */
			xmlString.Append("<mapEntry flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)mapEntry.Flags));

			if (mapEntry.CheckHasPermData())
			{
				xmlString.Append(" (HAS_PERM_DATA)");
			}

			xmlString.Append("\" action=\"").Append(actionString).Append("\" key=\"");
			if (Decoders.PrimitiveToString(mapKeyData, keyPrimitiveType, stringBuf) < 0)
			{
				(stringBuf).Data_internal("<Unknown>");
			}
			xmlString.Append(stringBuf.ToString().Replace("\"", "\'")).Append("\" ");

			if (mapEntry.CheckHasPermData())
			{
				xmlString.Append(" permData=\"");
				xmlString.Append(XmlDumpHexBuffer(mapEntry.PermData));
				xmlString.Append("\">").AppendLine();
			}
			else
			{
				xmlString.Append(">").AppendLine();
			}

			return xmlString.ToString();
		}

		private static string XmlDumpLocalElementSetDefDb(LocalElementSetDefDb elListSetDb)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);

			xmlString.Append(Encodeindents()).Append("<elementSetDefs>").AppendLine();
			indents++;

			for (int i = 0; i <= LocalElementSetDefDb.MAX_LOCAL_ID; ++i)
			{
				if (elListSetDb.Definitions[i].SetId != LocalElementSetDefDb.BLANK_ID)
				{
					ElementSetDef setDef = elListSetDb.Definitions[i];
					xmlString.Append(Encodeindents());
					xmlString.Append("<elementSetDef setId=\"").Append(elListSetDb.Definitions[i].SetId).Append("\">").AppendLine();

					++indents;
					for (int j = 0; j < setDef.Count; ++j)
					{
						ElementSetDefEntry entry = setDef.Entries[j];
						xmlString.Append(Encodeindents());
						xmlString.Append("<elementSetDefEntry name=\"").Append(entry.Name.ToString()).Append("\" dataType=\"");
						xmlString.Append(XmlDumpDataType(entry.DataType));
						xmlString.Append("\" />").AppendLine();
					}
					--indents;

					xmlString.Append(Encodeindents());
					xmlString.Append("</elementSetDef>").AppendLine();
				}
			}

			indents--;
			xmlString.Append(Encodeindents());
			xmlString.Append("</elementSetDefs>").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpLocalFieldSetDefDb(LocalFieldSetDefDb flListSetDb)
		{
			StringBuilder xmlString = new StringBuilder(INIT_STRING_SIZE);

			xmlString.Append(Encodeindents()).Append("<fieldSetDefs>").AppendLine();

			indents++;

			for (int i = 0; i <= LocalFieldSetDefDb.MAX_LOCAL_ID; ++i)
			{
				if (flListSetDb.Definitions[i].SetId != FieldSetDefDb.BLANK_ID)
				{
					FieldSetDef setDef = flListSetDb.Definitions[i];
					xmlString.Append(Encodeindents());
					xmlString.Append("<fieldSetDef setId=\"").Append(flListSetDb.Definitions[i].SetId).Append("\">").AppendLine();

					++indents;
					for (int j = 0; j < setDef.Count; ++j)
					{
						FieldSetDefEntry entry = setDef.Entries[j];
						xmlString.Append(Encodeindents());
						xmlString.Append("<fieldSetDefEntry fieldId=\"").Append(entry.FieldId).Append("\" dataType=\"");
						xmlString.Append(XmlDumpDataType((int)entry.DataType));
						xmlString.Append("\" />").AppendLine();
					}
					--indents;

					xmlString.Append(Encodeindents());
					xmlString.Append("</fieldSetDef>").AppendLine();
				}
			}

			indents--;
			xmlString.Append(Encodeindents());
			xmlString.Append("</fieldSetDefs>").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpMapBegin(Map map)
		{
			StringBuilder xmlString = new StringBuilder();
			bool firstFlag = true;

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<map flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)map.Flags));

			if (map.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (map.CheckHasSetDefs())
			{
				xmlString.Append("HAS_SET_DEFS");
				firstFlag = false;
			}
			if (map.CheckHasSummaryData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_SUMMARY_DATA");
			}
			if (map.CheckHasPerEntryPermData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_PER_ENTRY_PERM_DATA");
			}
			if (map.CheckHasTotalCountHint())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_TOTAL_COUNT_HINT");
			}
			if (map.CheckHasKeyFieldId())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_KEY_FIELD_ID");
			}
			if (map.Flags != 0)
			{
				xmlString.Append(")");
			}
			xmlString.Append("\"");

			xmlString.Append(" countHint=\"").Append(map.TotalCountHint).Append("\" keyPrimitiveType=\"");
			xmlString.Append(XmlDumpDataType(map.KeyPrimitiveType));
			xmlString.Append("\" containerType=\"");
			xmlString.Append(XmlDumpDataType(map.ContainerType));
			xmlString.Append("\" ");
			if (map.CheckHasKeyFieldId())
			{
				xmlString.Append("keyFieldId=\"").Append(map.KeyFieldId).Append("\" ");
			}
			xmlString.Append(">").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpSummaryDataEnd()
		{
			indents--;

			return (Encodeindents() + $"</summaryData>{NewLine}");
        }

		private static string XmlDumpSummaryDataBegin()
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<summaryData>").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpFilterListEnd()
		{
			indents--;

			return (Encodeindents() + $"</filterList>{NewLine}");
        }

		private static string XmlDumpFilterItemEnd()
		{
			indents--;

			return (Encodeindents() + $"</filterEntry>{NewLine}");
        }

		private static string XmlDumpFilterItemBegin(FilterEntry filterItem)
		{
			StringBuilder xmlString = new StringBuilder();
			bool firstFlag = true;
			string actionString;

			xmlString.Append(Encodeindents());
			indents++;
			switch (filterItem.Action)
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
				break;
			}
			/* Don't print the data element for deleted rows, there should not be any. */
			xmlString.Append("<filterEntry id=\"").Append(filterItem.Id).Append("\" action=\"").Append(actionString).Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)filterItem.Flags));

			if (filterItem.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (filterItem.CheckHasPermData())
			{
				xmlString.Append("HAS_PERM_DATA");
				firstFlag = false;
			}
			if (filterItem.CheckHasContainerType())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_CONTAINER_TYPE");
			}
			if (filterItem.Flags != 0)
			{
				xmlString.Append(")");
			}

			xmlString.Append("\" containerType=\"");
			xmlString.Append(XmlDumpDataType(filterItem.ContainerType));
			if (filterItem.CheckHasPermData())
			{
				xmlString.Append("\" permData=\"");
				xmlString.Append(XmlDumpHexBuffer(filterItem.PermData));
				xmlString.Append("\">").AppendLine();
			}
			else
			{
				xmlString.Append("\">").AppendLine();
			}

			return xmlString.ToString();
		}

		private static string XmlDumpFilterListBegin(FilterList fList)
		{
			StringBuilder xmlString = new StringBuilder();
			bool firstFlag = true;

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<filterList containerType=\"");
			xmlString.Append(XmlDumpDataType(fList.ContainerType));
			xmlString.Append("\" countHint=\"").Append(fList.TotalCountHint).Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)fList.Flags));

			if (fList.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (fList.CheckHasPerEntryPermData())
			{
				xmlString.Append("HAS_PER_ENTRY_PERM_DATA");
				firstFlag = false;
			}
			if (fList.CheckHasTotalCountHint())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_TOTAL_COUNT_HINT");
			}
			if (fList.Flags != 0)
			{
				xmlString.Append(")");
			}
			xmlString.Append("\">").AppendLine();

			return xmlString.ToString();
		}

		private static object CreateKeyData(int dataType)
		{
			object data = null;

			switch (dataType)
			{
				case DataTypes.INT:
					data = new Int();
					break;
				case DataTypes.UINT:
					data =new UInt();
					break;
				case DataTypes.FLOAT:
					data = new Float();
					break;
				case DataTypes.DOUBLE:
					data = new Double();
					break;
				case DataTypes.REAL:
					data = new Real();
					break;
				case DataTypes.DATE:
					data = new Date();
					break;
				case DataTypes.TIME:
					data = new Time();
					break;
				case DataTypes.DATETIME:
					data = new DateTime();
					break;
				case DataTypes.QOS:
					data = new Qos();
					break;
				case DataTypes.STATE:
					data = new State();
					break;
				case DataTypes.ENUM:
					data = new Enum();
					break;
				case DataTypes.ASCII_STRING:
				case DataTypes.RMTES_STRING:
				case DataTypes.UTF8_STRING:
				case DataTypes.BUFFER:
					data = new Buffer();
					break;
				default:
					break;
			}

			return data;
		}

		private static string XmlDumpArrayItemEnd()
		{
			return $"/>{NewLine}";
		}

		private static string XmlDumpArrayItemBegin()
        {
            return (Encodeindents() + "<arrayEntry");
        }

        private static string XmlDumpArrayEnd()
		{
			indents--;

			return (Encodeindents() + $"</array>{NewLine}");
        }

		private static string XmlDumpArrayBegin(Array array)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<array itemLength=\"").Append(array.ItemLength).Append("\" primitiveType=\"");
			xmlString.Append(XmlDumpDataType(array.PrimitiveType));
			xmlString.Append("\">").AppendLine();

			return xmlString.ToString();
		}

		private static void CopyIteratorInfo(DecodeIterator destIter, DecodeIterator sourceIter)
		{
			destIter.Clear();
			destIter.SetBufferAndRWFVersion(sourceIter._buffer, sourceIter.MajorVersion(), sourceIter.MinorVersion());
			destIter._curBufPos = sourceIter._curBufPos;
			if (destIter._curBufPos == 3 && sourceIter._buffer.Data().Contents[2] == MsgClasses.REFRESH && sourceIter._buffer.Data().Contents[3] == (byte)DomainType.DICTIONARY)
			{
				destIter._curBufPos = 0;
			}
			destIter._decodingLevel = sourceIter._decodingLevel;
			for (int i = 0; i < destIter._levelInfo.Length; i++)
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
					destIter._levelInfo[i]._elemListSetDef.Count = sourceIter._levelInfo[i]._elemListSetDef.Count;
					destIter._levelInfo[i]._elemListSetDef.SetId = sourceIter._levelInfo[i]._elemListSetDef.SetId;
					destIter._levelInfo[i]._elemListSetDef.Entries = sourceIter._levelInfo[i]._elemListSetDef.Entries;
				}
				if (destIter._levelInfo[i]._fieldListSetDef != null && sourceIter._levelInfo[i]._fieldListSetDef != null)
				{
					destIter._levelInfo[i]._fieldListSetDef.Count = sourceIter._levelInfo[i]._fieldListSetDef.Count;
					destIter._levelInfo[i]._fieldListSetDef.SetId = sourceIter._levelInfo[i]._fieldListSetDef.SetId;
					destIter._levelInfo[i]._fieldListSetDef.Entries = sourceIter._levelInfo[i]._fieldListSetDef.Entries;
				}
			}
		}

		private static string XmlDumpSeriesRowEnd()
		{
			indents--;

			return (Encodeindents() + $"</seriesEntry>{NewLine}");
        }

		private static string XmlDumpSeriesRowBegin(SeriesEntry row)
		{
			StringBuilder xmlString = new StringBuilder();

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<seriesEntry>").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpSeriesEnd()
		{
			indents--;

			return (Encodeindents() + $"</series>{NewLine}");
        }

		private static string XmlDumpSeriesBegin(Series series)
		{
			StringBuilder xmlString = new StringBuilder();
			bool firstFlag = true;

			xmlString.Append(Encodeindents());
			indents++;

			xmlString.Append("<series  flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)series.Flags));
			if (series.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (series.CheckHasSetDefs())
			{
				xmlString.Append("HAS_SET_DEFS");
				firstFlag = false;
			}
			if (series.CheckHasSummaryData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_SUMMARY_DATA");
			}
			if (series.CheckHasTotalCountHint())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_TOTAL_COUNT_HINT");
			}
			if (series.Flags != 0)
			{
				xmlString.Append(")");
			}

			xmlString.Append("\" countHint=\"").Append(series.TotalCountHint).Append("\" containerType=\"");
			xmlString.Append(XmlDumpDataType(series.ContainerType));
			xmlString.Append("\">").AppendLine();

			return xmlString.ToString();
		}

		private static string XmlDumpVectorEnd()
		{
			indents--;

			return (Encodeindents() + $"</vector>{NewLine}");
        }

		private static string XmlDumpVectorEntryEnd()
		{
			indents--;

			return (Encodeindents() + $"</vectorEntry>{NewLine}");
		}

		private static string XmlDumpVectorEntryBegin(VectorEntry vectorEntry)
		{
			StringBuilder xmlString = new StringBuilder();
			string actionString;

			xmlString.Append(Encodeindents());
			indents++;
			switch (vectorEntry.Action)
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
				break;
			}
			/* Don't print the data element for deleted rows, there should not be
			 * any. */
			xmlString.Append("<vectorEntry index=\"").Append(vectorEntry.Index).Append("\" action=\"").Append(actionString).Append("\" flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)vectorEntry.Flags));
			if (vectorEntry.CheckHasPermData())
			{
				xmlString.Append(" (HAS_PERM_DATA)\"");
				xmlString.Append(" permData=\"");
				xmlString.Append(XmlDumpHexBuffer(vectorEntry.PermData));
				xmlString.Append("\">").AppendLine();
			}
			else
			{
				xmlString.Append("\">").AppendLine();
			}

			return xmlString.ToString();
		}

		private static string XmlDumpVectorBegin(Vector vec)
		{
			StringBuilder xmlString = new StringBuilder();
			bool firstFlag = true;

			xmlString.Append(Encodeindents());
			indents++;
			xmlString.Append("<vector flags=\"0x").AppendFormat(string.Format("{0:X2}", (byte)vec.Flags));

			if (vec.Flags != 0)
			{
				xmlString.Append(" (");
			}
			if (vec.CheckHasSetDefs())
			{
				xmlString.Append("HAS_SET_DEFS");
				firstFlag = false;
			}
			if (vec.CheckHasSummaryData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_SUMMARY_DATA");
			}
			if (vec.CheckHasPerEntryPermData())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_PER_ENTRY_PERM_DATA");
			}
			if (vec.CheckHasTotalCountHint())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("HAS_TOTAL_COUNT_HINT");
			}
			if (vec.CheckSupportsSorting())
			{
				if (!firstFlag)
				{
					xmlString.Append("|");
				}
				else
				{
					firstFlag = false;
				}
				xmlString.Append("SUPPORTS_SORTING");
			}
			if (vec.Flags != 0)
			{
				xmlString.Append(")");
			}
			xmlString.Append("\"");

			xmlString.Append(" countHint=\"").Append(vec.TotalCountHint).Append("\" containerType=\"");
			xmlString.Append(XmlDumpDataType(vec.ContainerType));
			xmlString.Append("\">").AppendLine();

			return xmlString.ToString();
		}

	}
}