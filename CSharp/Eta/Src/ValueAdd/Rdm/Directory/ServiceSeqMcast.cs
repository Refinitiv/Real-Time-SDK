/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static Refinitiv.Eta.Rdm.Directory;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Sequenced Multicast. 
    /// Contains information about an upstream source associated with the service.
    /// </summary>
    public class ServiceSeqMcast
    {
        private Buffer m_Name = new Buffer();
        private long m_Type;
        private long m_LinkState;
        private long m_LinkCode;
        private bool foundSnapshotPort = false, foundSnapshotAddr = false,
                                      foundGapRecPort = false, foundGapRecAddr = false,
                                      foundRefDataPort = false, foundRefDataAddr = false;
        private Buffer m_Text = new Buffer();
       
        private const string eol = "\n";
        private const string tab = "\t";

        private StringBuilder m_StringBuf = new StringBuilder();
        private Vector m_Vector = new Vector();
        private VectorEntry m_VectorEntry = new VectorEntry();
        private ElementList m_VectorElementList = new ElementList();
        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_Element = new ElementEntry();
        private UInt m_TmpUInt = new UInt();
        private Buffer m_TmpBuffer = new Buffer();

        private bool foundPort = false;
        private bool foundMCGroup = false;
        private bool foundDomain = false;

        private LocalElementSetDefDb m_ElementSetDefDb = new LocalElementSetDefDb();

        public ServiceSeqMcastInfo SeqMcastInfo { get; set; } = new ServiceSeqMcastInfo();
        /// <summary>
        /// The action associated with the current ServiceSeqMcast instance.
        /// </summary>
        public MapEntryActions Action { get; set; }
        /// <summary>
        /// Indicates the presence of the Type field.
        /// </summary>
        public bool HasType { get => (Flags & ServiceSeqMcastFlags.HAS_TYPE) != 0; set { if (value) Flags |= ServiceSeqMcastFlags.HAS_TYPE; else Flags &= ~ServiceSeqMcastFlags.HAS_TYPE; } }
        /// <summary>
        /// Indicates the presence of the Code field.
        /// </summary>
        public bool HasCode { get => (Flags & ServiceSeqMcastFlags.HAS_CODE) != 0; set { if (value) Flags |= ServiceSeqMcastFlags.HAS_CODE; else Flags &= ~ServiceSeqMcastFlags.HAS_CODE; } }
        /// <summary>
        /// Indicates the presence of the Text field.
        /// </summary>
        public bool HasText { get => (Flags & ServiceSeqMcastFlags.HAS_TEXT) != 0; set { if (value) Flags |= ServiceSeqMcastFlags.HAS_TEXT; else Flags &= ~ServiceSeqMcastFlags.HAS_TEXT; } }
        /// <summary>
        /// Text further describing the state provided by the linkState and linkCode members.
        /// </summary>
        public Buffer Text { get => m_Text; set { BufferHelper.CopyBuffer(value, m_Text); } }
        /// <summary>
        /// The name identifying this upstream source.
        /// </summary>
        public Buffer Name { get => m_Name; set { BufferHelper.CopyBuffer(value, m_Name); } }
        /// <summary>
        /// Code indicating additional information about the status of the source. Populated by <see cref="Directory.LinkCodes"/>
        /// </summary>
        public long LinkCode { get => m_LinkCode; set { Debug.Assert(HasCode); m_LinkCode = value; } }
        /// <summary>
        /// Flag indicating whether the source is up or down. Populated by <see cref="Directory.LinkStates"/>
        /// </summary>
        public long LinkState { get => m_LinkState; set { m_LinkState = value; } }
        /// <summary>
        /// The type of this service link. Populated by <see cref="Directory.LinkTypes"/>
        /// </summary>
        public long Type { get => m_Type; set { Debug.Assert(HasType); m_Type = value; } }
        /// <summary>
        /// The filterId - Populated by <see cref="Directory.ServiceFilterIds"/>
        /// </summary>
        public int FilterId { get => ServiceFilterIds.LINK; }
        /// <summary>
        /// The service link flags.
        /// </summary>
        public ServiceSeqMcastFlags Flags { get; set; }

        /// <summary>
        /// Instantiates a new service seq mcast.
        /// </summary>
        public ServiceSeqMcast()
        {
            Clear();
        }

        public override string ToString()
        {
            m_StringBuf.Clear();

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("Name: ");
            m_StringBuf.Append(Name);
            m_StringBuf.Append(eol);

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("Snapshort Address:Port: ");
            m_StringBuf.Append(SeqMcastInfo.SnapshotServer.Address.Data().Contents);
            m_StringBuf.Append(":");
            m_StringBuf.Append(SeqMcastInfo.SnapshotServer.Port);
            m_StringBuf.Append(eol);

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("Gap Receive Address:Port: ");
            m_StringBuf.Append(SeqMcastInfo.GapRecoveryServer.Address.Data().Contents);
            m_StringBuf.Append(":");
            m_StringBuf.Append(SeqMcastInfo.GapRecoveryServer.Port);
            m_StringBuf.Append(eol);

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("Reference Data Address:Port: ");
            m_StringBuf.Append(SeqMcastInfo.RefDataServer.Address.Data().Contents);
            m_StringBuf.Append(":");
            m_StringBuf.Append(SeqMcastInfo.RefDataServer.Port);
            m_StringBuf.Append(eol);

            return m_StringBuf.ToString();
        }

        public void Clear()
        {
            Flags = 0;
            m_Name.Clear();
            m_Type = LinkTypes.INTERACTIVE;
            m_LinkState = 0;
            m_LinkCode = LinkCodes.NONE;
            m_Text.Clear();
            Action = MapEntryActions.ADD;
        }

        public CodecReturnCode Copy(ServiceLink destServiceLink)
        {
            Debug.Assert(destServiceLink != null);
            if (HasCode)
            {
                destServiceLink.HasCode = true;
                destServiceLink.LinkCode = LinkCode;
            }

            if (HasText)
            {
                destServiceLink.HasText = true;
                BufferHelper.CopyBuffer(Text, destServiceLink.Text);
            }

            if (HasType)
            {
                destServiceLink.HasType = true;
                destServiceLink.Type = Type;
            }

            destServiceLink.LinkState = LinkState;

            return CodecReturnCode.SUCCESS;
        }

        public CodecReturnCode Encode(EncodeIterator encIter)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();
            CodecReturnCode ret = m_ElementList.EncodeInit(encIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (HasCode)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.LINK_CODE;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(LinkCode);
                ret = m_Element.Encode(encIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasText)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.TEXT;
                m_Element.DataType = Codec.DataTypes.ASCII_STRING;
                ret = m_Element.Encode(encIter, Text);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasType)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.TYPE;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(Type);
                ret = m_Element.Encode(encIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            m_Element.Clear();
            m_Element.Name = ElementNames.LINK_STATE;
            m_Element.DataType = Codec.DataTypes.UINT;
            m_TmpUInt.Value(LinkState);
            ret = m_Element.Encode(encIter, m_TmpUInt);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            return m_ElementList.EncodeComplete(encIter, true);
        }
       
        public CodecReturnCode Decode(DecodeIterator dIter)
        {
            m_ElementList.Clear();
            m_Element.Clear();

            CodecReturnCode ret = m_ElementList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            while ((ret = m_Element.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                if (m_Element.Name.Equals(ElementNames.SNAPSHOT_SERVER_HOST))
                {
                    ret = m_TmpBuffer.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    SeqMcastInfo.SnapshotServer.Address = m_TmpBuffer;
                    foundSnapshotAddr = true;
                }
                else if (m_Element.Name.Equals(ElementNames.SNAPSHOT_SERVER_PORT))
                {
                    ret = m_TmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    SeqMcastInfo.SnapshotServer.Port = m_TmpUInt.ToLong();
                    foundSnapshotPort = true;
                }
                else if (m_Element.Name.Equals(ElementNames.GAP_RECOVERY_SERVER_HOST))
                {
                    ret = m_TmpBuffer.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    SeqMcastInfo.GapRecoveryServer.Address = m_TmpBuffer;
                    foundGapRecAddr = true;
                }
                else if (m_Element.Name.Equals(ElementNames.GAP_RECOVERY_SERVER_PORT))
                {
                    ret = m_TmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    SeqMcastInfo.GapRecoveryServer.Port = m_TmpUInt.ToLong();
                    foundGapRecPort = true;
                }
                else if (m_Element.Name.Equals(ElementNames.REFERENCE_DATA_SERVER_HOST))
                {
                    ret = m_TmpBuffer.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    m_TmpBuffer.Copy(SeqMcastInfo.RefDataServer.Address);
                    foundRefDataAddr = true;
                }
                else if (m_Element.Name.Equals(ElementNames.REFERENCE_DATA_SERVER_PORT))
                {
                    ret = m_TmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    SeqMcastInfo.RefDataServer.Port = m_TmpUInt.ToLong();
                    foundRefDataPort = true;
                }
                else if (m_Element.Name.Equals(ElementNames.STREAMING_MCAST_CHANNELS))
                {
                    m_Vector.Clear();
                    ret = m_Vector.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }

                    if (m_Vector.CheckHasSetDefs())
                    {
                        m_ElementSetDefDb.Clear();
                        m_ElementSetDefDb.Decode(dIter);
                    }

                    m_VectorEntry.Clear();
                    if ((ret = m_VectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        foundPort = false;
                        foundMCGroup = false;
                        foundDomain = false;
                        do
                        {
                            m_VectorElementList.Clear();
                            m_VectorElementList.Decode(dIter, m_ElementSetDefDb);

                            while ((ret = m_Element.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                            {
                                if (m_Element.Name.Equals(ElementNames.MULTICAST_GROUP))
                                {
                                    ret = m_TmpBuffer.Decode(dIter);
                                    if (ret != CodecReturnCode.SUCCESS
                                            && ret != CodecReturnCode.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    SeqMcastInfo.StreamingMcastChanServerList.AddAddress(m_TmpBuffer, SeqMcastInfo.StreamingMCastChanServerCount);
                                    foundMCGroup = true;
                                }
                                else if (m_Element.Name.Equals(ElementNames.PORT))
                                {
                                    ret = m_TmpUInt.Decode(dIter);
                                    if (ret != CodecReturnCode.SUCCESS
                                            && ret != CodecReturnCode.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    SeqMcastInfo.StreamingMcastChanServerList.SetPort(m_TmpUInt.ToLong(), SeqMcastInfo.StreamingMCastChanServerCount);
                                    foundPort = true;
                                }
                                else if (m_Element.Name.Equals(ElementNames.DOMAIN))
                                {
                                    ret = m_TmpUInt.Decode(dIter);
                                    if (ret != CodecReturnCode.SUCCESS
                                            && ret != CodecReturnCode.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    SeqMcastInfo.StreamingMcastChanServerList.SetDomain(m_TmpUInt.ToLong(), SeqMcastInfo.StreamingMCastChanServerCount);
                                    foundDomain = true;
                                }
                            }

                            SeqMcastInfo.StreamingMCastChanServerCount++;

                        } while ((ret = m_VectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER);

                        if (foundPort && foundMCGroup && foundDomain)
                            SeqMcastInfo.Flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_SMC_SERV;
                    }
                }
                else if (m_Element.Name.Equals(ElementNames.GAP_MCAST_CHANNELS))
                {
                    m_Vector.Clear();
                    ret = m_Vector.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }

                    if (m_Vector.CheckHasSetDefs())
                    {
                        m_ElementSetDefDb.Clear();
                        m_ElementSetDefDb.Decode(dIter);
                    }

                    m_VectorEntry.Clear();
                    if ((ret = m_VectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        foundPort = false;
                        foundMCGroup = false;
                        foundDomain = false;
                        do
                        {
                            m_VectorElementList.Clear();
                            m_VectorElementList.Decode(dIter, m_ElementSetDefDb);
                            while ((ret = m_Element.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                            {
                                if (m_Element.Name.Equals(ElementNames.MULTICAST_GROUP))
                                {
                                    ret = m_TmpBuffer.Decode(dIter);
                                    if (ret != CodecReturnCode.SUCCESS
                                            && ret != CodecReturnCode.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    SeqMcastInfo.GapMCastChanServerList.AddAddress(m_TmpBuffer, SeqMcastInfo.GapMCastChanServerCount);
                                    foundMCGroup = true;
                                }
                                else if (m_Element.Name.Equals(ElementNames.PORT))
                                {
                                    ret = m_TmpUInt.Decode(dIter);
                                    if (ret != CodecReturnCode.SUCCESS
                                            && ret != CodecReturnCode.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    SeqMcastInfo.GapMCastChanServerList.SetPort(m_TmpUInt.ToLong(), SeqMcastInfo.GapMCastChanServerCount);
                                    foundPort = true;
                                }
                                else if (m_Element.Name.Equals(ElementNames.DOMAIN))
                                {
                                    ret = m_TmpUInt.Decode(dIter);
                                    if (ret != CodecReturnCode.SUCCESS
                                            && ret != CodecReturnCode.BLANK_DATA)
                                    {
                                        return ret;
                                    }
                                    SeqMcastInfo.GapMCastChanServerList.SetDomain(m_TmpUInt.ToLong(), SeqMcastInfo.GapMCastChanServerCount);
                                    foundDomain = true;
                                }
                            }

                            SeqMcastInfo.GapMCastChanServerCount++;

                        } while ((ret = m_VectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER);


                        if (foundPort && foundMCGroup && foundDomain)
                            SeqMcastInfo.Flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_GMC_SERV;
                    }
                }

                if (foundSnapshotPort && foundSnapshotAddr)
                    SeqMcastInfo.Flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_SNAPSHOT_SERV;

                if (foundGapRecPort && foundGapRecAddr)
                    SeqMcastInfo.Flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_GAP_REC_SERV;

                if (foundRefDataPort && foundRefDataAddr)
                    SeqMcastInfo.Flags |= ServiceSeqMcastInfo.RDM_SVC_SMF_HAS_REF_DATA_SERV;
            }

            return CodecReturnCode.SUCCESS;
        }
    }
}
