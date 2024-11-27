/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Link. 
    /// Contains information about an upstream source associated with the service.
    /// </summary>
    sealed public class ServiceLink
    {
        private Buffer m_Name = new Buffer();
        private Buffer m_Text = new Buffer();
        private long m_LinkCode;
        private long m_Type;
        /// <summary>
        /// The upstream source name to the user specified buffer. 
        /// Data and position of name field buffer will be set to passed in buffer's data and position.
        /// </summary>
        public Buffer Name { get => m_Name; set { BufferHelper.CopyBuffer(value, m_Name); } }
        
        /// <summary>
        /// Checks the presence of Type field.
        /// </summary>
        public bool HasType { get => (Flags & ServiceLinkFlags.HAS_TYPE) != 0; set { if (value) Flags |= ServiceLinkFlags.HAS_TYPE; else Flags &= ~(ServiceLinkFlags.HAS_TYPE); } }

        /// <summary>
        /// The type of this service link. Populated by <see cref="LinkTypes"/>
        /// </summary>
        public long Type { get => m_Type; set { Debug.Assert(HasType); HasType = true; m_Type = value; } }

        /// <summary>
        /// Flag indicating whether the source is up or down. Populated by <see cref="LinkStates"/>
        /// </summary>
        public long LinkState { get; set; }

        /// <summary>
        /// Checks the presence of the Code field.
        /// </summary>
        public bool HasCode { get => (Flags & ServiceLinkFlags.HAS_CODE) != 0; set { if (value) Flags |= ServiceLinkFlags.HAS_CODE; else Flags &= ~(ServiceLinkFlags.HAS_CODE); } }

        /// <summary>
        /// Code indicating additional information about the status of the source. Populated by <see cref="LinkCodes"/>
        /// </summary>
        public long LinkCode { get => m_LinkCode; set { Debug.Assert(HasCode); m_LinkCode = value; } }

        /// <summary>
        /// Checks the presence of Text field.
        /// </summary>
        public bool HasText { get => (Flags & ServiceLinkFlags.HAS_TEXT) != 0; set { if (value) Flags |= ServiceLinkFlags.HAS_TEXT; else Flags &= ~(ServiceLinkFlags.HAS_TEXT); } }

        /// <summary>
        /// Text further describing the state provided by the linkState and linkCode members.
        /// </summary>
        public Buffer Text { 
            get => m_Text; 
            set 
            { 
                Debug.Assert(value != null); 
                BufferHelper.CopyBuffer(value, m_Text);
            } 
        }

        /// <summary>
        /// Filter ID for this Service Link.
        /// </summary>
        public int FilterId { get => ServiceFilterIds.LINK; }

        /// <summary>
        /// The service sink flags. Populated by <see cref="ServiceLinkFlags"/>
        /// </summary>
        public ServiceLinkFlags Flags { get; set; }

        /// <summary>
        /// The service sink flags. Populated by <see cref="MapEntryActions"/>
        /// </summary>
        public MapEntryActions Action { get; set; }

        private const string tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();
        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_ElementEntry = new ElementEntry();
        private UInt tmpUInt = new UInt();

        /// <summary>
        /// Service Link constructor.
        /// </summary>
        public ServiceLink()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Service Link object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
          Flags = 0;
          Name.Clear();
          HasType = true;
          Type = LinkTypes.INTERACTIVE;
          LinkState = 0;
          HasCode = true;
          LinkCode = LinkCodes.NONE;
          Text.Clear();
          Action = MapEntryActions.ADD;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destServiceLink</c>.
        /// </summary>
        /// <param name="destServiceLink">ServiceLink object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(ServiceLink destServiceLink)
        {
            Debug.Assert(destServiceLink != null);
            if (HasCode)
            {
                destServiceLink.HasCode = true;
                destServiceLink.LinkCode = LinkCode;
            } else
            {
                destServiceLink.HasCode = false;
            }

            if (HasText)
            {
                destServiceLink.HasText = true;
                BufferHelper.CopyBuffer(Text, destServiceLink.Text);
            } else
            {
                destServiceLink.HasText = false;
            }

            if (HasType)
            {
                destServiceLink.HasType = true;
                destServiceLink.Type = Type;
            } else
            {
                destServiceLink.HasType = false;
            }

            BufferHelper.CopyBuffer(Name, destServiceLink.Name);
            destServiceLink.LinkState = LinkState;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Service Link using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();
            CodecReturnCode ret = m_ElementList.EncodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }    

            if (HasCode)
            {
                m_ElementEntry.Clear();
                m_ElementEntry.Name = ElementNames.LINK_CODE;
                m_ElementEntry.DataType = Codec.DataTypes.UINT;
                tmpUInt.Value(LinkCode);
                ret = m_ElementEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasText)
            {
                m_ElementEntry.Clear();
                m_ElementEntry.Name = ElementNames.TEXT;
                m_ElementEntry.DataType = Codec.DataTypes.ASCII_STRING;
                ret = m_ElementEntry.Encode(encodeIter, Text);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }                   
            }

            if (HasType)
            {
                m_ElementEntry.Clear();
                m_ElementEntry.Name = ElementNames.TYPE;
                m_ElementEntry.DataType = Codec.DataTypes.UINT;
                tmpUInt.Value(Type);
                ret = m_ElementEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }     
            }

            m_ElementEntry.Clear();
            m_ElementEntry.Name = ElementNames.LINK_STATE;
            m_ElementEntry.DataType = Codec.DataTypes.UINT;
            tmpUInt.Value(LinkState);
            ret = m_ElementEntry.Encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }     

            return m_ElementList.EncodeComplete(encodeIter, true);
        }

        /// <summary>
        /// Decodes this Service Link using the provided <c>decodeIter</c> .
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter)
        {
            m_ElementList.Clear();
            m_ElementEntry.Clear();

            bool foundType = false;
            bool foundCode = false;

            CodecReturnCode ret = m_ElementList.Decode(decodeIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            while ((ret = m_ElementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                if (m_ElementEntry.Name.Equals(ElementNames.TYPE))
                {
                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasType = true;
                    foundType = true;
                    Type = tmpUInt.ToLong();
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.LINK_STATE))
                {
                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    LinkState = tmpUInt.ToLong();
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.LINK_CODE))
                {
                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasCode = true;
                    foundCode = true;
                    LinkCode = tmpUInt.ToLong();
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.TEXT))
                {
                    Buffer encodedData = m_ElementEntry.EncodedData;
                    Text.Data(encodedData.Data(), encodedData.Position, encodedData.Length);
                    HasText = true;
                }
            }

            if (!foundType)
            {
                HasType = false;
            }
            if (!foundCode)
            {
                HasCode = false;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Service Link.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            stringBuf.Clear();

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("Name: ");
            stringBuf.Append(Name);
            stringBuf.AppendLine();

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("LinkType: ");
            stringBuf.Append(Type);
            stringBuf.AppendLine();

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("LinkState: ");
            stringBuf.Append(LinkState);
            stringBuf.AppendLine();

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("LinkCode: ");
            stringBuf.Append(LinkCode);
            stringBuf.AppendLine();

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("LinkText: ");
            stringBuf.Append(Text);
            stringBuf.AppendLine();

            return stringBuf.ToString();
        }

    }
}
