/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;
using Directory = LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Data. Contains information provided by the Source Directory Data filter.
    /// </summary>
    sealed public class ServiceData
    {
        private Buffer m_Data = new Buffer();

        private const string tab = "\t";

        private StringBuilder m_StringBuf = new StringBuilder();
        private UInt m_TmpUInt = new UInt();
        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_Element = new ElementEntry();

        /// <summary>
        /// The action associated with data filter.
        /// </summary>
        public FilterEntryActions Action { get; set; }

        /// <summary>
        /// The filterId. Populated by <see cref="ServiceFilterIds"/>
        /// </summary>
        public int FilterId { get => ServiceFilterIds.DATA; }

        /// <summary>
        /// Checks the presence of the data field.
        /// </summary>
        public bool HasData { get => (Flags & ServiceDataFlags.HAS_DATA) != 0; set { if (value) Flags |= ServiceDataFlags.HAS_DATA; else Flags &= ~ServiceDataFlags.HAS_DATA; } }

        /// <summary>
        /// The buffer representing the encoded data, 
        /// to be applied to all items being provided by this service.
        /// </summary>
        public Buffer Data { get => m_Data; set { BufferHelper.CopyBuffer(value, m_Data); } }

        /// <summary>
        /// Directory data type. Populated by <see cref="Directory.DataTypes"/>
        /// </summary>
        public long Type { get; set; }

        /// <summary>
        /// The OMM type of the data. Populated by <see cref="Eta.Codec.DataTypes"/>
        /// </summary>
        public int DataType { get; set; }

        /// <summary>
        /// The service data flags. Populated by <see cref="ServiceDataFlags"/>
        /// </summary>
        public ServiceDataFlags Flags { get; set; }

        /// <summary>
        /// RDM Service Data constructor.
        /// </summary>
        public ServiceData()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the RDM Service Data object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Flags = 0;
            DataType = Codec.DataTypes.NO_DATA;
            m_Data.Clear();
            Action = FilterEntryActions.SET;
            Type = Directory.DataTypes.NONE;
        }
       
        /// <summary>
        /// Encode an RDM Service Data.
        /// </summary>
        /// <param name="encIter"><see cref="EncodeIterator"/> instance used for encoding.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode Encode(EncodeIterator encIter)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();
            CodecReturnCode ret = m_ElementList.EncodeInit(encIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (HasData)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.TYPE;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(Type);
                ret = m_Element.Encode(encIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                m_Element.Clear();
                m_Element.Name = ElementNames.DATA;
                m_Element.DataType = DataType;
                ret = m_Element.Encode(encIter, Data);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            return m_ElementList.EncodeComplete(encIter, true);
        }

        /// <summary>
        /// Decode a ETA service data field into an RDM service data field.
        /// </summary>
        /// <param name="dIter"><see cref="DecodeIterator"/> instance used for decoding.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode Decode(DecodeIterator dIter)
        {
            Clear();
            m_ElementList.Clear();
            m_Element.Clear();

            CodecReturnCode ret = m_ElementList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            bool foundType = false;
            bool foundData = false;
            while ((ret = m_Element.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                /* get service data information */
                /* Type */
                if (m_Element.Name.Equals(ElementNames.TYPE))
                {
                    m_TmpUInt.Clear();
                    ret = m_TmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    foundType = true;
                    Type = m_TmpUInt.ToLong();
                }
                /* Data */
                else if (m_Element.Name.Equals(ElementNames.DATA))
                {
                    Data = m_Element.EncodedData;
                    DataType = m_Element.DataType;
                    HasData = true;
                    foundData = true;
                }
            }

            /* If Data element is present, type must be too. */
            if ((foundData && !foundType) || (foundType && !foundData))
                return CodecReturnCode.FAILURE;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destServiceData</c>.
        /// </summary>
        /// <param name="destServiceData">ServiceData object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(ServiceData destServiceData)
        {
            Debug.Assert(destServiceData != null);
            destServiceData.Clear();
            destServiceData.Action = Action;
            destServiceData.Type = Type;
            destServiceData.Flags = Flags;
            if (HasData)
            {
                destServiceData.HasData = true;
                destServiceData.DataType = DataType;
                BufferHelper.CopyBuffer(Data, destServiceData.Data);
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Performs an update of ServiceData object.
        /// </summary>
        /// <param name="destServiceData">ServiceData object to update with information from this object. It cannot be null.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode Update(ServiceData destServiceData)
        {
            Debug.Assert(destServiceData != null);
            destServiceData.Action = Action;
            destServiceData.Type = Type;
            destServiceData.Flags = Flags;
            if (HasData)
            {
                destServiceData.HasData = true;
                destServiceData.DataType = DataType;
                BufferHelper.CopyBuffer(Data, destServiceData.Data);
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Service Data.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            m_StringBuf.Clear();
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("DataFilter:");
            m_StringBuf.AppendLine();

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("Type: ");
            m_StringBuf.Append(Type);
            m_StringBuf.AppendLine();

            if (HasData)
            {
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append("Data: ");
                m_StringBuf.Append(Data);
                m_StringBuf.AppendLine();

                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append("DataType: ");
                m_StringBuf.Append(DataType);
                m_StringBuf.AppendLine();
            }
            return m_StringBuf.ToString();
        }

    }
}
