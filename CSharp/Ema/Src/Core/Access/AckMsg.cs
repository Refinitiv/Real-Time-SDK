/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;


namespace LSEG.Ema.Access
{
    /// <summary>
    /// AckMsg indicates success or failure of PostMsg. 
    /// If requested, AckMsg is sent by provider acting on PostMsg received from consumer.
    /// AckMsg indicates success or failure of caching / processing of received PostMsg.
    /// 
    /// AckMsg is optional.
    /// 
    /// Consumer requests provider to send AckMsg by calling
    /// PostMsg.SolicitAck(bool) with true.
    /// </summary>
    public sealed class AckMsg : Msg, ICloneable
    {
        private const string ACCESSDENIED_STRING = "AccessDenied";
        private const string DENIEDBYSOURCE_STRING = "DeniedBySource";
        private const string SOURCEDOWN_STRING = "SourceDown";
        private const string SOURCEUNKNOWN_STRING = "SourceUnknown";
        private const string NORESOURCES_STRING = "NoResources";
        private const string NORESPONSE_STRING = "NoResponse";
        private const string SYMBOLUNKNOWN_STRING = "SymbolUnknown";
        private const string NOTOPEN_STRING = "NotOpen";
        private const string GATEWAYDOWN_STRING = "GatewayDown";
        private const string NONE_STRING = "None";
        private const string INVALIDCONTENT_STRING = "InvalidContent";
        private const string UNKNOWNNACKCODE_STRING = "Unknown NackCode value ";

        internal AckMsgEncoder m_ackMsgEncoder;

        /// <summary>
        /// Constructor for AckMsg
        /// </summary>
        public AckMsg()
        {
            m_msgClass = MsgClasses.ACK;
            m_rsslMsg.MsgClass = MsgClasses.ACK;
            
            m_ackMsgEncoder = new AckMsgEncoder(this);
            Encoder = m_ackMsgEncoder;
        }

        /// <summary>
        /// Copy constructor for <see cref="AckMsg"/>
        /// </summary>
        /// <param name="source"><see cref="AckMsg"/> to create current message from.</param>
        public AckMsg(AckMsg source)
        {
            m_msgClass = MsgClasses.ACK;
            m_ackMsgEncoder = new AckMsgEncoder(this);
            Encoder = m_ackMsgEncoder;
            source.CopyMsg(this);
        }

        internal AckMsg(EmaObjectManager objectManager) : base(objectManager)
        {
            m_msgClass = MsgClasses.ACK;
            m_rsslMsg.MsgClass = MsgClasses.ACK;

            m_ackMsgEncoder = new AckMsgEncoder(this);
            Encoder = m_ackMsgEncoder;
        }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.ACK_MSG;

        /// <summary>
        /// Indicates the presence of SeqNum. Sequence Number is an optional member of AckMsg.
        /// </summary>
        public bool HasSeqNum { get => m_rsslMsg.CheckHasSeqNum(); }
        
        /// <summary>
        /// Indicates the presence of NackCode. Negative acknowledgement code is an optional member of AckMsg.
        /// </summary>
        public bool HasNackCode { get => m_rsslMsg.CheckHasNakCode(); }
        
        /// <summary>
        /// Indicates the presence of Text. Text is an optional member of AckMsg.
        /// </summary>
        public bool HasText { get => m_rsslMsg.CheckHasText(); }
        
        /// <summary>
        /// The sequence number.
        /// </summary>
        public long SeqNum() 
        { 
            if (!m_rsslMsg.CheckHasSeqNum())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call SeqNum() while it is not set.");
            }

            return m_rsslMsg.SeqNum;
        }
        
        /// <summary>
        /// The Ack Id
        /// </summary>
        public long AckId() 
        { 
            return m_rsslMsg.AckId;
        }
        
        /// <summary>
        /// Negative acknowledgement code.
        /// </summary>
        public int NackCode()
        {
            if (!m_rsslMsg.CheckHasNakCode())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call NackCode() while it is not set.");
            }

            return m_rsslMsg.NakCode;
        }

        /// <summary>
        /// AckMsg text.
        /// </summary>
        public string Text()
        {
            if (!m_rsslMsg.CheckHasText())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call Text() while it is not set.");
            }

            return m_rsslMsg.Text.ToString();
        }
        
        /// <summary>
        /// Determines whether this stream is private or not.
        /// </summary>
        public bool PrivateStream { get => m_rsslMsg.CheckPrivateStream(); }

        /// <summary>
        /// Returns the NackCode value in a string format
        /// </summary>
        /// <returns>string representation of the NackCode</returns>
        public string NackCodeAsString()
        {
            switch (NackCode())
            {
                case Access.NackCode.NONE:
                    return NONE_STRING;
                case Access.NackCode.ACCESS_DENIED:
                    return ACCESSDENIED_STRING;
                case Access.NackCode.DENIED_BY_SOURCE:
                    return DENIEDBYSOURCE_STRING;
                case Access.NackCode.SOURCE_DOWN:
                    return SOURCEDOWN_STRING;
                case Access.NackCode.SOURCE_UNKNOWN:
                    return SOURCEUNKNOWN_STRING;
                case Access.NackCode.NO_RESOURCES:
                    return NORESOURCES_STRING;
                case Access.NackCode.NO_RESPONSE:
                    return NORESPONSE_STRING;
                case Access.NackCode.GATEWAY_DOWN:
                    return GATEWAYDOWN_STRING;
                case Access.NackCode.SYMBOL_UNKNOWN:
                    return SYMBOLUNKNOWN_STRING;
                case Access.NackCode.NOT_OPEN:
                    return NOTOPEN_STRING;
                case Access.NackCode.INVALID_CONTENT:
                    return INVALIDCONTENT_STRING;
                default:
                    return UNKNOWNNACKCODE_STRING + NackCode();
            }
        }

        /// <summary>
        /// Specifies StreamId
        /// </summary>
        public AckMsg StreamId(int streamId) 
        {
            m_ackMsgEncoder.StreamId(streamId);
            return this;
        }
        /// <summary>
        /// Specifies DomainType
        /// </summary>
        public AckMsg DomainType(int domainType)
        {
            m_ackMsgEncoder.DomainType(domainType);
            return this;
        }
        
        /// <summary>
        /// Specifies the name
        /// </summary>
        public AckMsg Name(string name)
        {
            m_ackMsgEncoder.Name(name);
            return this;
        }
        
        /// <summary>
        /// Specifies Name type
        /// </summary>
        public AckMsg NameType(int nameType) 
        {
            m_ackMsgEncoder.NameType(nameType);
            return this;
        }
        
        /// <summary>
        /// Specifies the ServiceName within MsgKey.
        /// </summary>
        public AckMsg ServiceName(string serviceName)
        {
            SetMsgServiceName(serviceName);
            return this;
        }

        /// <summary>
        /// Specifies the ServiceId.
        /// </summary>
        public AckMsg ServiceId(int serviceId)
        {
            m_ackMsgEncoder.ServiceId(serviceId);
            return this;
        }

        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">the id to be set</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg Id(int id)
        {
            m_ackMsgEncoder.Identifier(id);
            return this;
        }

        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">filter value to be set</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg Filter(long filter)
        {
            m_ackMsgEncoder.Filter(filter);
            return this;
        }
        
        /// <summary>
        /// Specifies SeqNum.
        /// </summary>
        /// <param name="seqNum">sequence number to be set</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg SeqNum(long seqNum)
        {
            m_ackMsgEncoder.SeqNum(seqNum);
            return this;
        }
        
        /// <summary>
        /// Specifies AckId.
        /// </summary>
        /// <param name="ackId">acknowledgement ID to be set</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg AckId(long ackId)
        {
            m_ackMsgEncoder.AckId(ackId);
            return this;
        }
        
        /// <summary>
        /// Specifies NackCode.
        /// </summary>
        /// <param name="nackCode">negative acknowledgement code to be set</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg NackCode(int nackCode)
        {
            m_ackMsgEncoder.NackCode(nackCode);
            return this;
        }
        
        /// <summary>
        /// Specifies Text.
        /// </summary>
        /// <param name="text">Text value to be set</param>
        /// <returns></returns>
        public AckMsg Text(string text)
        {
            m_ackMsgEncoder.Text(text);
            return this;
        }

        /// <summary>
        /// Specifies Attributes.
        /// </summary>
        /// <param name="attrib">attributes to be set</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg Attrib(ComplexType attrib) 
        {
            m_attrib.SetExternalData(attrib);
            m_ackMsgEncoder.Attrib(attrib);
            return this;
        }
        
        /// <summary>
        /// Specifies Payload.
        /// </summary>
        /// <param name="payload">payload to be set</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg Payload(ComplexType payload) 
        {
            m_payload.SetExternalData(payload);
            m_ackMsgEncoder.Payload(payload);
            return this;
        }
        
        /// <summary>
        /// Specifies ExtendedHeader.
        /// </summary>
        /// <param name="buffer">a EmaBuffer containing extendedHeader information</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg ExtendedHeader(EmaBuffer buffer) 
        {
            m_ackMsgEncoder.ExtendedHeader(buffer);
            return this;
        }

        /// <summary>
        /// Specifies PrivateStream.
        /// </summary>
        /// <param name="privateStream">specifies if this is a private stream (default is false)</param>
        /// <returns>reference to current <see cref="AckMsg"/> instance</returns>
        public AckMsg SetPrivateStream(bool privateStream) 
        { 
            m_ackMsgEncoder.PrivateStream(privateStream);
            return this;
        }

        /// <summary>
        /// Clears current AckMsg instance
        /// </summary>
        public AckMsg Clear()
        {
            ClearInt();
            return this;
        }

        /// <summary>
        /// Provides string representation of the current AckMsg instance
        /// </summary>
        /// <returns>string representing current <see cref="AckMsg"/> instance.</returns>
        public override string ToString()
        {
            return ToString(0);
        }       

        /// <summary>
        /// Creates object that is a copy of the current object.
        /// </summary>
        /// <returns><see cref="AckMsg"/> instance that is a copy of the current AckMsg.</returns>
        public AckMsg Clone()
        {
            var copy = new AckMsg();
            CopyMsg(copy);
            return copy;
        }

        object ICloneable.Clone()
        {
            return Clone();
        }

        internal override string ToString(int indent)
        {
            if (m_objectManager == null)
                return "\nDecoding of just encoded object in the same application is not supported\n";

            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent++).Append("AckMsg");
            Utilities.AddIndent(m_ToString, indent, true).Append("streamId=\"").Append(StreamId()).Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("domain=\"").Append(Utilities.RdmDomainAsString(DomainType())).Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("ackId=\"").Append(AckId()).Append("\"");

            if (HasSeqNum)
                Utilities.AddIndent(m_ToString, indent, true).Append("seqNum=\"").Append(SeqNum()).Append("\"");

            if (HasNackCode)
                Utilities.AddIndent(m_ToString, indent, true).Append("nackCode=\"").Append(NackCodeAsString()).Append("\"");

            if (HasText)
                Utilities.AddIndent(m_ToString, indent, true).Append("text=\"").Append(Text()).Append("\"");

            indent--;
            if (HasMsgKey)
            {
                indent++;
                if (HasName)
                    Utilities.AddIndent(m_ToString, indent, true).Append("name=\"").Append(Name()).Append("\"");

                if (HasNameType)
                    Utilities.AddIndent(m_ToString, indent, true).Append("nameType=\"").Append(NameType()).Append("\"");

                if (HasServiceId)
                    Utilities.AddIndent(m_ToString, indent, true).Append("serviceId=\"").Append(ServiceId()).Append("\"");

                if (HasServiceName)
                    Utilities.AddIndent(m_ToString, indent, true).Append("serviceName=\"").Append(ServiceName())
                            .Append("\"");

                if (HasFilter)
                    Utilities.AddIndent(m_ToString, indent, true).Append("filter=\"").Append(Filter()).Append("\"");

                if (HasId)
                    Utilities.AddIndent(m_ToString, indent, true).Append("id=\"").Append(Id()).Append("\"");

                indent--;

                if (HasAttrib)
                {
                    indent++;
                    Utilities.AddIndent(m_ToString, indent, true).Append("Attrib dataType=\"")
                            .Append(Access.DataType.AsString(Attrib().DataType)).Append("\"\n");

                    indent++;
                    m_ToString.Append(Attrib().ToString(indent));
                    indent--;

                    Utilities.AddIndent(m_ToString, indent, false).Append("AttribEnd");
                    indent--;
                }
            }

            if (HasExtendedHeader)
            {
                indent++;
                Utilities.AddIndent(m_ToString, indent, true).Append("ExtendedHeader\n");

                indent++;
                Utilities.AddIndent(m_ToString, indent);
                Utilities.AsHexString(m_ToString, ExtendedHeader());
                indent--;

                Utilities.AddIndent(m_ToString, indent, true).Append("ExtendedHeaderEnd");
                indent--;
            }

            indent++;
            Utilities.AddIndent(m_ToString, indent, true).Append("Payload dataType=\"")
                    .Append(Access.DataType.AsString(Payload().DataType)).Append("\"\n");

            indent++;
            m_ToString.Append(Payload().ToString(indent));
            indent--;

            Utilities.AddIndent(m_ToString, indent).Append("PayloadEnd");
            indent--;

            Utilities.AddIndent(m_ToString, indent, true).Append("AckMsgEnd\n");

            return m_ToString.ToString();
        }

        internal override void EncodeComplete()
        {
            m_ackMsgEncoder.EncodeComplete();
        }
    }
}
