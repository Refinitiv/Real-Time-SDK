﻿/*|-----------------------------------------------------------------------------
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
    /// GenericMsg allows applications to bidirectionally send messages without any implied message semantics.
    /// GenericMsg may be sent on any item stream using <see cref="OmmConsumer.Submit(GenericMsg, long)"/>.
    ///
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform setting and getting of information from GenericMsg.
    /// Objects of this class are not cache-able.
    /// Decoding of just encoded GenericMsg in the same application is not supported.
    /// </summary>
    public sealed class GenericMsg : Msg, ICloneable
    {
        internal GenericMsgEncoder m_genericMsgEncoder;

        /// <summary>
        /// Constructor for GenericMsg class.
        /// </summary>
        public GenericMsg()
        {
            m_msgClass = MsgClasses.GENERIC;
            m_rsslMsg.MsgClass = MsgClasses.GENERIC;

            m_genericMsgEncoder = new GenericMsgEncoder(this);
            Encoder = m_genericMsgEncoder;
            m_msgEncoder = m_genericMsgEncoder;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.GENERIC_MSG;

            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);
        }

        /// <summary>
        /// Copy constructor for <see cref="GenericMsg"/>
        /// </summary>
        /// <param name="source"><see cref="GenericMsg"/> to create current message from.</param>
        public GenericMsg(GenericMsg source)
        {
            m_msgClass = MsgClasses.GENERIC;
            m_genericMsgEncoder = new GenericMsgEncoder(this);
            Encoder = m_genericMsgEncoder;
            m_msgEncoder = m_genericMsgEncoder;
            source.CopyMsg(this);
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.GENERIC_MSG;
        }

        internal GenericMsg(EmaObjectManager objectManager) : base(objectManager)
        {
            m_msgClass = MsgClasses.GENERIC;
            m_rsslMsg.MsgClass = MsgClasses.GENERIC;

            m_genericMsgEncoder = new GenericMsgEncoder(this);
            Encoder = m_genericMsgEncoder;
            m_msgEncoder = m_genericMsgEncoder;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.GENERIC_MSG;
        }

        /// <summary>
        /// Indicates presence of SeqNum.
        /// </summary>
        /// <returns>true if the message has a sequence number, false if not</returns>
        public bool HasSeqNum { get => m_rsslMsg.CheckHasSeqNum(); }

        /// <summary>
        /// Indicates presence of SecondarySeqNum.
        /// Secondary sequence number is an optional member of GenericMsg.
        /// </summary>
        /// <returns>true if the messaage has a secondary sequence number, false if not</returns>
        public bool HasSecondarySeqNum { get => m_rsslMsg.CheckHasSecondarySeqNum(); }

        /// <summary>
        /// Indicates presence of PartNum.
        /// Part number is an optional member of GenericMsg.
        /// </summary>
        /// <returns>true if the messaage has a part number, false if not</returns>
        public bool HasPartNum { get => m_rsslMsg.CheckHasPartNum(); }

        /// <summary>
        /// Indicates presence of PermissionData.
        /// Permission data is optional on GenericMsg.
        /// </summary>
        /// <returns>true if the messaage has permission data, false if not</returns>
        public bool HasPermissionData { get => m_rsslMsg.CheckHasPermData(); }

        /// <summary>
        /// Returns SeqNum.
        /// </summary>
        /// <returns>The configured sequence number</returns>
        public long SeqNum()
        {
            if (!m_rsslMsg.CheckHasSeqNum())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call SeqNum() while it is not set.");
            }

            return m_rsslMsg.SeqNum;
        }

        /// <summary>
        /// Returns SecondarySeqNum.
        /// </summary>
        /// <returns>secondary sequence number</returns>
        public long SecondarySeqNum()
        {
            if (!m_rsslMsg.CheckHasSecondarySeqNum())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call SecondarySeqNum() while it is not set.");
            }

            return m_rsslMsg.SecondarySeqNum;
        }

        /// <summary>
        /// Returns PartNum.
        /// </summary>
        /// <returns>part number</returns>
        public int PartNum()
        {
            if (!m_rsslMsg.CheckHasPartNum())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PartNum() while it is not set.");
            }

            return m_rsslMsg.PartNum;
        }

        /// <summary>
        /// Returns PermissionData.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing permission data</returns>
        public EmaBuffer PermissionData()
        {
            if (!m_rsslMsg.CheckHasPermData())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PermissionData() while it is not set.");
            }

            return GetPermDataEmaBuffer();
        }

        /// <summary>
        /// The value is true if this is a one part generic message, false otherwise.
        /// </summary>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>

        public bool Complete()
        {
            return (m_rsslMsg.Flags & GenericMsgFlags.MESSAGE_COMPLETE) > 0;
        }

        /// <summary>
        /// Returns ProviderDriven.
        /// </summary>
        /// <returns>true if this is provider driven generic message.</returns>
        public bool ProviderDriven()
        {
            return (m_rsslMsg.Flags & GenericMsgFlags.PROVIDER_DRIVEN) > 0;
        }

        /// <summary>
        /// Clears the GenericMsg.
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>Reference to the current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg Clear()
        {
            Clear_All();
            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);
            return this;
        }

        /// <summary>
        /// Specifies StreamId
        /// </summary>
        /// <param name="streamId">The Stream Id</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg StreamId(int streamId)
        {
            m_genericMsgEncoder.StreamId(streamId);
            return this;
        }

        /// <summary>
        /// Specifies DomainType
        /// </summary>
        /// <param name="domainType">The RDM domain type defined in <see cref="Rdm.EmaRdm"/> or user defined.</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg DomainType(int domainType)
        {
            m_genericMsgEncoder.DomainType(domainType);
            return this;
        }

        /// <summary>
        /// Specifies the name
        /// </summary>
        /// <param name="name">The name</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg Name(string name)
        {
            m_genericMsgEncoder.Name(name);
            return this;
        }

        /// <summary>
        /// Specifies Name type
        /// </summary>
        /// <param name="nameType">The RDM name type defined in <see cref="Rdm.EmaRdm"/>.</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg NameType(int nameType)
        {
            m_genericMsgEncoder.NameType(nameType);
            return this;
        }

        /// <summary>
        /// Specifies the ServiceId.
        /// </summary>
        /// <param name="serviceId">The serviceId</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg ServiceId(int serviceId)
        {
            m_genericMsgEncoder.ServiceId(serviceId);
            return this;
        }

        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">the id to be set</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg Id(int id)
        {
            m_genericMsgEncoder.Identifier(id);
            return this;
        }

        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">filter value to be set</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg Filter(long filter)
        {
            m_genericMsgEncoder.Filter(filter);
            return this;
        }

        /// <summary>
        /// Specifies SeqNum.
        /// </summary>
        /// <param name="seqNum">sequence number value to be set</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg SeqNum(long seqNum)
        {
            m_genericMsgEncoder.SeqNum(seqNum);
            return this;
        }

        /// <summary>
        /// Specifies SecondarySeqNum.
        /// </summary>
        /// <param name="secondarySeqNum">specifies secondary sequence number</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg SecondarySeqNum(long secondarySeqNum)
        {
            m_genericMsgEncoder.SecndarySeqNum(secondarySeqNum);
            return this;
        }

        /// <summary>
        /// Specifies PartNum.
        /// </summary>
        /// <param name="partNum">specifies part number value</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg PartNum(int partNum)
        {
            m_genericMsgEncoder.PartNum(partNum);
            return this;
        }

        /// <summary>
        /// Specifies PermissionData.
        /// </summary>
        /// <param name="permissionData">a <see cref="EmaBuffer"/> object with permission data information</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg PermissionData(EmaBuffer permissionData)
        {
            m_genericMsgEncoder.PermissionData(permissionData);
            return this;
        }

        /// <summary>
        /// Specifies Attrib.
        /// </summary>
        /// <param name="data">an object of IComplexType type that contains attribute information</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg Attrib(ComplexType data)
        {
            m_attrib.SetExternalData(data);
            m_genericMsgEncoder.Attrib(data);
            return this;
        }

        /// <summary>
        /// Specifies Payload.
        /// </summary>
        /// <param name="payload">payload to be set</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg Payload(ComplexType payload)
        {
            m_payload.SetExternalData(payload);
            m_genericMsgEncoder.Payload(payload);
            return this;
        }

        /// <summary>
        /// Specifies ExtendedHeader.
        /// </summary>
        /// <param name="buffer">a EmaBuffer containing extendedHeader information</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg ExtendedHeader(EmaBuffer buffer)
        {
            m_genericMsgEncoder.ExtendedHeader(buffer);
            return this;
        }

        /// <summary>
        /// Specifies Complete.
        /// Must be set to true for one part generic message.
        /// </summary>
        /// <param name="complete">specifies if this is the last part of the multi part generic message</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg Complete(bool complete)
        {
            m_genericMsgEncoder.MsgComplete(complete);
            return this;
        }

        /// <summary>
        /// Specifies whether the message is provider driven.
        /// </summary>
        /// <param name="providerDriven">specifies if this message is provider driven,
        /// i.e. was initiated by the provider and not the part of the response to any consumer request</param>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public GenericMsg ProviderDriven(bool providerDriven)
        {
            m_genericMsgEncoder.ProviderDriven(providerDriven);
            return this;
        }

        /// <summary>
        /// Completes encoding current GenericMsg instance.
        /// </summary>
        internal override void EncodeComplete()
        {
            m_genericMsgEncoder.EncodeComplete();
        }

        /// <summary>
        /// Provides string representation of the current GenericMsg instance
        /// </summary>
        /// <returns>Reference to current <see cref="GenericMsg"/> object.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        /// <summary>
        /// Creates object that is a copy of the current object.
        /// </summary>
        /// <returns><see cref="GenericMsg"/> object that is a copy of the current GenericMsg.</returns>
        public GenericMsg Clone()
        {
            var copy = new GenericMsg();
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
            Utilities.AddIndent(m_ToString, indent++).Append("GenericMsg");

            Utilities.AddIndent(m_ToString, indent, true).Append("streamId=\"")
                                                              .Append(StreamId())
                                                              .Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("domain=\"")
                                                          .Append(Utilities.RdmDomainAsString(DomainType()))
                                                          .Append("\"");

            if (Complete())
                Utilities.AddIndent(m_ToString, indent, true).Append("MessageComplete");

            if (HasSeqNum)
                Utilities.AddIndent(m_ToString, indent, true).Append("seqNum=\"")
                                                              .Append(SeqNum()).Append("\"");

            if (HasSecondarySeqNum)
                Utilities.AddIndent(m_ToString, indent, true).Append("secondarySeqNum=\"")
                                                                .Append(SecondarySeqNum()).Append("\"");

            if (HasPartNum)
                Utilities.AddIndent(m_ToString, indent, true).Append("partNum=\"")
                                                                .Append(PartNum()).Append("\"");

            if (HasPermissionData)
            {
                Utilities.AddIndent(m_ToString, indent, true).Append("permissionData=\"");
                Utilities.AsHexString(m_ToString, PermissionData()).Append("\"");
            }
            if (ProviderDriven())
                Utilities.AddIndent(m_ToString, indent, true).Append("ProviderDriven");

            indent--;
            if (HasMsgKey)
            {
                indent++;
                if (HasName)
                    Utilities.AddIndent(m_ToString, indent, true).Append("name=\"")
                                                                 .Append(Name())
                                                                 .Append("\"");

                if (HasNameType)
                    Utilities.AddIndent(m_ToString, indent, true).Append("nameType=\"")
                                                                 .Append(NameType())
                                                                 .Append("\"");

                if (HasServiceId)
                    Utilities.AddIndent(m_ToString, indent, true).Append("serviceId=\"")
                                                                 .Append(ServiceId())
                                                                 .Append("\"");

                if (HasServiceName)
                    Utilities.AddIndent(m_ToString, indent, true).Append("serviceName=\"")
                                                                 .Append(ServiceName())
                                                                 .Append("\"");

                if (HasFilter)
                    Utilities.AddIndent(m_ToString, indent, true).Append("filter=\"")
                                                                 .Append(Filter())
                                                                 .Append("\"");

                if (HasId)
                    Utilities.AddIndent(m_ToString, indent, true).Append("id=\"")
                                                                 .Append(Id())
                                                                 .Append("\"");

                indent--;

                if (HasAttrib)
                {
                    indent++;
                    Utilities.AddIndent(m_ToString, indent, true).Append("Attrib dataType=\"")
                                                                 .Append(Access.DataType.AsString(Attrib().DataType))
                                                                 .Append($"\"{NewLine}");

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
                                                         .Append(Access.DataType.AsString(Payload().DataType))
                                                         .Append($"\"{NewLine}");

            indent++;
            m_ToString.Append(Payload().ToString(indent));
            indent--;

            Utilities.AddIndent(m_ToString, indent).Append("PayloadEnd");
            indent--;

            Utilities.AddIndent(m_ToString, indent, true).Append($"GenericMsgEnd{NewLine}");
            return m_ToString.ToString();
        }
    }
}