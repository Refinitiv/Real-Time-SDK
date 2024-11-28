/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023, 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// AckMsg indicates success or failure of PostMsg.
    /// </summary>
    /// <remarks>
    /// If requested, AckMsg is sent by provider acting on PostMsg received from consumer.<br/>
    /// AckMsg indicates success or failure of caching / processing of received PostMsg.<br/>
    /// AckMsg is optional.<br/>
    /// Consumer requests provider to send AckMsg by calling PostMsg.SolicitAck(bool) with true.<br/>
    /// </remarks>
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
            m_msgEncoder = m_ackMsgEncoder;

            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.ACK_MSG;

            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);
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
            m_msgEncoder = m_ackMsgEncoder;
            source.CopyMsg(this);

            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.ACK_MSG;
        }

        internal AckMsg(EmaObjectManager objectManager) : base(objectManager)
        {
            m_msgClass = MsgClasses.ACK;
            m_rsslMsg.MsgClass = MsgClasses.ACK;

            m_ackMsgEncoder = new AckMsgEncoder(this);
            m_msgEncoder = m_ackMsgEncoder;
            Encoder = m_ackMsgEncoder;

            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.ACK_MSG;
        }

        /// <summary>
        /// Indicates the presence of the Sequence Number member. Sequence Number is an optional member of AckMsg.
        /// </summary>
        /// <returns>bool value indicating the presence of the Sequence NUmber member</returns>
        public bool HasSeqNum { get => m_rsslMsg.CheckHasSeqNum(); }

        /// <summary>
        /// Indicates the presence of Negative Acknowledgement Code member. Negative acknowledgement code is an optional member of AckMsg.
        /// </summary>
        /// <returns>bool value indicating the presence of the Negative Acknowledgement Code member</returns>
        public bool HasNackCode { get => m_rsslMsg.CheckHasNakCode(); }

        /// <summary>
        /// Indicates the presence of the Text member. Text is an optional member of AckMsg.
        /// </summary>
        /// <returns>bool value indicating the presence of the Text member</returns>
        public bool HasText { get => m_rsslMsg.CheckHasText(); }

        /// <summary>
        /// The sequence number.
        /// </summary>
        /// <returns><see cref="AckMsg"/>'s sequence number</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if the sequence number has not been set.</exception>
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
        /// <returns><see cref="AckMsg"/>'s ackId</returns>
        public long AckId()
        {
            return m_rsslMsg.AckId;
        }

        /// <summary>
        /// Negative acknowledgement code.
        /// </summary>
        /// <returns><see cref="AckMsg"/>'s negative acknowledgement code</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if the negative acknowledgement code has not been set.</exception>
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
        /// <returns><see cref="AckMsg"/>'s text string</returns>
        /// /// <exception cref="OmmInvalidUsageException">Thrown if the text has not been set.</exception>
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
        /// <returns>bool indicating if this is a private string</returns>
        public bool PrivateStream()
        {
            return m_rsslMsg.CheckPrivateStream();
        }

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
        /// Specifies the StreamId
        /// </summary>
        /// <param name="streamId">The stream Id</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg StreamId(int streamId)
        {
            m_ackMsgEncoder.StreamId(streamId);
            return this;
        }

        /// <summary>
        /// Specifies DomainType
        /// </summary>
        /// <param name="domainType">The RDM Domain type defined in <see cref="Rdm.EmaRdm"/> or user defined.</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg DomainType(int domainType)
        {
            m_ackMsgEncoder.DomainType(domainType);
            return this;
        }

        /// <summary>
        /// Specifies the name
        /// </summary>
        /// <param name="name">The item name</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg Name(string name)
        {
            m_ackMsgEncoder.Name(name);
            return this;
        }

        /// <summary>
        /// Specifies Name type
        /// </summary>
        /// <param name="nameType">The RDM Instrument NameType defined in <see cref="Rdm.EmaRdm"/></param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg NameType(int nameType)
        {
            m_ackMsgEncoder.NameType(nameType);
            return this;
        }

        /// <summary>
        /// Specifies the ServiceName.<br/>
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceName">The name of the service</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg ServiceName(string serviceName)
        {
            SetMsgServiceName(serviceName);
            return this;
        }

        /// <summary>
        /// Specifies the ServiceId.<br/>
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceId">The numerical service Id</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg ServiceId(int serviceId)
        {
            m_ackMsgEncoder.ServiceId(serviceId);
            return this;
        }

        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">The ack message Id</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg Id(int id)
        {
            m_ackMsgEncoder.Identifier(id);
            return this;
        }

        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">The filter value</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg Filter(long filter)
        {
            m_ackMsgEncoder.Filter(filter);
            return this;
        }

        /// <summary>
        /// Specifies SeqNum.
        /// </summary>
        /// <param name="seqNum">The sequence number</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg SeqNum(long seqNum)
        {
            m_ackMsgEncoder.SeqNum(seqNum);
            return this;
        }

        /// <summary>
        /// Specifies AckId.
        /// </summary>
        /// <param name="ackId">The acknowledgement ID</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg AckId(long ackId)
        {
            m_ackMsgEncoder.AckId(ackId);
            return this;
        }

        /// <summary>
        /// Specifies NackCode.
        /// </summary>
        /// <param name="nackCode">negative acknowledgement code to be set</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg NackCode(int nackCode)
        {
            m_ackMsgEncoder.NackCode(nackCode);
            return this;
        }

        /// <summary>
        /// Specifies Text.
        /// </summary>
        /// <param name="text">Text value to be set</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg Text(string text)
        {
            m_ackMsgEncoder.Text(text);
            return this;
        }

        /// <summary>
        /// Specifies Attributes.
        /// </summary>
        /// <param name="attrib">attributes to be set</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
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
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
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
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg ExtendedHeader(EmaBuffer buffer)
        {
            m_ackMsgEncoder.ExtendedHeader(buffer);
            return this;
        }

        /// <summary>
        /// Specifies PrivateStream.
        /// </summary>
        /// <param name="privateStream">specifies if this is a private stream (default is false)</param>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg PrivateStream(bool privateStream)
        {
            m_ackMsgEncoder.PrivateStream(privateStream);
            return this;
        }

        /// <summary>
        /// Clears current AckMsg instance
        /// </summary>
        /// <returns>Reference to current <see cref="AckMsg"/> object.</returns>
        public AckMsg Clear()
        {
            Clear_All();

            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);

            return this;
        }

        /// <summary>
        /// Provides string representation of the current AckMsg instance
        /// </summary>
        /// <returns>string representing current <see cref="AckMsg"/> object.</returns>
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string FillString(int indent)
        {
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
                    Utilities.AddIndent(m_ToString, indent, true).Append("serviceName=\"").Append(ServiceName()).Append("\"");

                if (HasFilter)
                    Utilities.AddIndent(m_ToString, indent, true).Append("filter=\"").Append(Filter()).Append("\"");

                if (HasId)
                    Utilities.AddIndent(m_ToString, indent, true).Append("id=\"").Append(Id()).Append("\"");

                indent--;

                if (HasAttrib)
                {
                    indent++;
                    Utilities.AddIndent(m_ToString, indent, true).Append("Attrib dataType=\"")
                            .Append(Access.DataType.AsString(Attrib().DataType)).Append($"\"{NewLine}");

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
                Utilities.AddIndent(m_ToString, indent, true).Append($"ExtendedHeader{NewLine}");

                indent++;
                Utilities.AddIndent(m_ToString, indent);
                Utilities.AsHexString(m_ToString, ExtendedHeader());
                indent--;

                Utilities.AddIndent(m_ToString, indent, true).Append("ExtendedHeaderEnd");
                indent--;
            }

            indent++;
            Utilities.AddIndent(m_ToString, indent, true).Append("Payload dataType=\"")
                    .Append(Access.DataType.AsString(Payload().DataType)).Append($"\"{NewLine}");

            indent++;
            m_ToString.Append(Payload().ToString(indent));
            indent--;

            Utilities.AddIndent(m_ToString, indent).Append("PayloadEnd");
            indent--;

            Utilities.AddIndent(m_ToString, indent, true).Append($"AckMsgEnd{NewLine}");
            return m_ToString.ToString();
        }

        internal override void EncodeComplete()
        {
            m_ackMsgEncoder.EncodeComplete();
        }
    }
}