/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Text;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// This class represents information for how a consumer is using a paticular service in regards to source mirroring.
    /// </summary>
    public class ConsumerStatusService
    {
        /// <summary>
        /// The ID of the service 
        /// </summary>
        public long ServiceId { get; set; }

        /// <summary>
        /// The map entry action for this service status. See <see cref="MapEntryActions"/>
        /// </summary>
        public MapEntryActions Action { get; set; }

        /// <summary>
        /// The source mirroring mode value of this service.  See <see cref="SourceMirroringMode"/>
        /// </summary>
        public long SourceMirroringModeVal { get; set; }

        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_ElementEntry = new ElementEntry();
        private UInt m_TmpUInt = new UInt();
        private StringBuilder m_StringBuffer = new StringBuilder();

        private const string eol = "\n";
        private const string tab = "\t";

        /// <summary>
        /// Clears the current contents of the Consumer Status Service object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Action = MapEntryActions.ADD;
            ServiceId = 0;
            SourceMirroringModeVal = SourceMirroringMode.ACTIVE_NO_STANDBY;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destConsumerStatusService</c>.
        /// </summary>
        /// <param name="destConsumerStatusService">ConsumerStatusService object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
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

        /// <summary>
        /// Encodes the Consumer Status Service info using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
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

        /// <summary>
        /// Decodes this Consumer Status Service info using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this ConsumerStatusService message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();
            m_ElementList.Clear();
            CodecReturnCode ret;
            if ((ret = m_ElementList.Decode(decodeIter, null)) < CodecReturnCode.SUCCESS)
            {
                return ret;
            }
            m_ElementEntry.Clear();
            bool foundSourceMirroringMode = false;
            while ((ret = m_ElementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
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

                    ret = m_TmpUInt.Decode(decodeIter);
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

        /// <summary>
        /// Returns a human readable string representation of the Directory ConsumerStatusService message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            return BuildStringBuf().ToString();
        }


        /// <summary>
        /// Returns a human readable string representation of the Directory ConsumerStatusService message.
        /// </summary>
        /// <returns>StringBuilder containing the string representation.</returns>
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

    }
}
