/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using System.Text;
using static Refinitiv.Eta.Rdm.Directory;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    public class ConsumerStatusService
    {
        public long ServiceId { get; set; }

        public MapEntryActions Action { get; set; }

        public long SourceMirroringModeVal { get; set; }

        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_ElementEntry = new ElementEntry();
        private UInt m_TmpUInt = new UInt();
        private StringBuilder m_StringBuffer = new StringBuilder();

        private const string eol = "\n";
        private const string tab = "\t";

        public void Clear()
        {
            Action = MapEntryActions.ADD;
            ServiceId = 0;
            SourceMirroringModeVal = SourceMirroringMode.ACTIVE_NO_STANDBY;
        }

        public CodecReturnCode Copy(ConsumerStatusService destConsumerStatusService)
        {
            if (destConsumerStatusService is not null)
            {
                destConsumerStatusService.Clear();
                destConsumerStatusService.ServiceId = ServiceId;
                destConsumerStatusService.Action = Action;
                destConsumerStatusService.SourceMirroringModeVal = SourceMirroringModeVal;
                return CodecReturnCode.SUCCESS;
            } else
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }            
        }

        public CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();
            m_ElementList.Clear();
            CodecReturnCode ret;
            if ((ret = m_ElementList.Decode(dIter, null)) < CodecReturnCode.SUCCESS)
            {
                return ret;
            }
            m_ElementEntry.Clear();
            bool foundSourceMirroringMode = false;
            while ((ret = m_ElementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                if (m_ElementEntry.Name.Equals(ElementNames.SOURCE_MIRROR_MODE))
                {
                    if (m_ElementEntry.DataType != Codec.DataTypes.UINT)
                    {
                        return ret;
                    }

                    ret = m_TmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    SourceMirroringModeVal = m_TmpUInt.ToLong();
                    foundSourceMirroringMode = true;
                }
            }

            if (!foundSourceMirroringMode)
            {
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();
            CodecReturnCode ret = m_ElementList.EncodeInit(encodeIter, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            m_ElementEntry.Clear();
            m_ElementEntry.Name = ElementNames.SOURCE_MIRROR_MODE;
            m_ElementEntry.DataType = Codec.DataTypes.UINT;
            m_TmpUInt.Value(SourceMirroringModeVal);
            ret = m_ElementEntry.Encode(encodeIter, m_TmpUInt);
            
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return m_ElementList.EncodeComplete(encodeIter, true);
        }

        public StringBuilder BuildStringBuf()
        {
            m_StringBuffer.Clear();
            m_StringBuffer.Insert(0, "ConsumerStatusService: \n");
            m_StringBuffer.Append(tab);

            m_StringBuffer.Append(tab);
            m_StringBuffer.Append("action: ");
            m_StringBuffer.Append(Action);
            m_StringBuffer.Append(eol);
            m_StringBuffer.Append(tab);
            m_StringBuffer.Append("serviceId: ");
            m_StringBuffer.Append(ServiceId);
            m_StringBuffer.Append(eol);
            m_StringBuffer.Append(tab);
            m_StringBuffer.Append("sourceMirroringMode: ");
            m_StringBuffer.Append(SourceMirroringModeVal);
            m_StringBuffer.Append(eol);

            return m_StringBuffer;
        }

        public override string ToString()
        {
            return BuildStringBuf().ToString();
        }

    }
}
