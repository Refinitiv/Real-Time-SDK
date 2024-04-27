/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// StatusMsg conveys item state information.<br/>
    /// StatusMsg is used to convey item state information, permission change or item group id change.
    /// </summary>
    /// <remarks>
    /// Calling an accessor method on an optional member of StatusMsg must be preceded by a call to respective Has*** property.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and getting of information from RefreshMsg.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// Decoding of just encoded StatusMsg in the same application is not supported.<br/>
    /// </remarks>
    public sealed class StatusMsg : Msg, ICloneable
    {
        private OmmState m_state = new OmmState();
        private bool m_stateSet = false;

        internal StatusMsgEncoder m_statusMsgEncoder;

        /// <summary>
        /// Constructor for StatusMsg
        /// </summary>
        public StatusMsg()
        {
            m_msgClass = MsgClasses.STATUS;
            m_rsslMsg.MsgClass = MsgClasses.STATUS;
            m_statusMsgEncoder = new StatusMsgEncoder(this);
            Encoder = m_statusMsgEncoder;
            m_msgEncoder = m_statusMsgEncoder;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            ClearTypeSpecific_All = ClearStatus_All;
            ClearTypeSpecific_Decode = ClearStatus_Decode;
            m_dataType = Access.DataType.DataTypes.STATUS_MSG;
            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);
        }

        /// <summary>
        /// Copy constructor for <see cref="StatusMsg"/>
        /// </summary>
        /// <param name="source"><see cref="StatusMsg"/> to create current message from.</param>
        public StatusMsg(StatusMsg source)
        {
            m_msgClass = MsgClasses.STATUS;
            m_statusMsgEncoder = new StatusMsgEncoder(this);
            Encoder = m_statusMsgEncoder;
            m_msgEncoder = m_statusMsgEncoder;
            source.CopyMsg(this);
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            ClearTypeSpecific_All = ClearStatus_All;
            m_dataType = Access.DataType.DataTypes.STATUS_MSG;
        }

        internal StatusMsg(EmaObjectManager objectManager) : base(objectManager)
        {
            m_msgClass = MsgClasses.STATUS;
            m_rsslMsg.MsgClass = MsgClasses.STATUS;
            m_statusMsgEncoder = new StatusMsgEncoder(this);
            Encoder = m_statusMsgEncoder;
            m_msgEncoder = m_statusMsgEncoder;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            ClearTypeSpecific_All = ClearStatus_All;
            m_dataType = Access.DataType.DataTypes.STATUS_MSG;
        }

        /// <summary>
        /// Indicates presence of ItemGroup.
        /// Item Group is an optional member of StatusMsg.
        /// </summary>
        public bool HasItemGroup { get => m_rsslMsg.CheckHasGroupId(); }

        /// <summary>
        /// Indicates presence of State.
        /// Item State is an optional member of StatusMsg.
        /// </summary>
        public bool HasState { get => m_rsslMsg.CheckHasState(); }

        /// <summary>
        /// Indicates presence of PermissionData.
        /// Permission data is an optional member of StatusMsg.
        /// </summary>
        public bool HasPermissionData { get => m_rsslMsg.CheckHasPermData(); }

        /// <summary>
        /// Indicates presence of PublisherId.
        /// Publisher id is an optional member of StatusMsg.
        /// </summary>
        public bool HasPublisherId { get => m_rsslMsg.CheckHasPostUserInfo(); }

        /// <summary>
        /// Returns State.
        /// Calling this method must be preceded by a call to <see cref="HasState"/>
        /// </summary>
        /// <returns>state of item (e.g. Open / Ok)</returns>
        public OmmState State()
        {
            if (!m_rsslMsg.CheckHasState())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call State() while it is not set.");
            }

            if (!m_stateSet)
            {
                m_state.Clear_All();
                m_state.Decode(m_rsslMsg.State);
                m_stateSet = true;
            }

            return m_state;
        }

        /// <summary>
        /// Returns ItemGroup.
        /// Calling this method must be preceded by a call to <see cref="HasItemGroup"/>
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing item group information</returns>
        public EmaBuffer ItemGroup()
        {
            if (!m_rsslMsg.CheckHasGroupId())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call ItemGroup() while it is not set.");
            }

            return GetItemGroupBuffer();
        }

        /// <summary>
        /// Returns PermissionData.
        /// Calling this method must be preceded by a call to <see cref="HasPermissionData"/>
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing permission data information</returns>
        public EmaBuffer PermissionData()
        {
            if (!m_rsslMsg.CheckHasPermData())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PermissionData() while it is not set.");
            }

            return GetPermDataEmaBuffer();
        }

        /// <summary>
        /// Returns PublisherIdUserId.
        /// Calling this method must be preceded by a call to <see cref="HasPublisherId"/>
        /// </summary>
        /// <returns>publisher's user Id</returns>
        public long PublisherIdUserId()
        {
            if (!m_rsslMsg.CheckHasPostUserInfo())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PublisherIdUserId() while PostUserInfo is not set.");
            }

            return m_rsslMsg.PostUserInfo.UserId;
        }

        /// <summary>
        /// Returns PublisherIdUserAddress.
        /// Calling this method must be preceded by a call to <see cref="HasPublisherId"/>
        /// </summary>
        /// <returns>The Publisher Id User Address</returns>
        /// <exception cref="NotImplementedException"></exception>
        public long PublisherIdUserAddress()
        {
            if (!m_rsslMsg.CheckHasPostUserInfo())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PublisherIdUserAddress() while PostUserInfo is not set.");
            }

            return m_rsslMsg.PostUserInfo.UserAddr;
        }

        /// <summary>
        /// Returns ClearCache.
        /// </summary>
        /// <returns>true if cache needs to be cleared on receipt of this status message;
        /// false otherwise</returns>
        public bool ClearCache()
        {
            return m_rsslMsg.CheckClearCache();
        }

        /// <summary>
        /// Returns PrivateStream.
        /// </summary>
        /// <returns>true if this is private stream item; false otherwise</returns>
        /// <exception cref="NotImplementedException"></exception>
        public bool PrivateStream()
        {
            return m_rsslMsg.CheckPrivateStream();
        }

        /// <summary>
        /// Clears the StatusMsg.
        /// Invoking Clear() method clears all the values and resets all the defaults
        /// </summary>
        /// <returns>Reference to the current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg Clear()
        {
            Clear_All();
            return this;
        }

        /// <summary>
        /// Clears current StatusMsg instance.
        /// </summary>
        internal void ClearStatus_All()
        {
            ClearMsg_All();
            m_state.Clear_All();
            m_stateSet = false;

            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);
        }

        internal void ClearStatus_Decode()
        {
            ClearMsg_Decode();
            m_state.Clear_All();
            m_stateSet = false;
        }

        /// <summary>
        /// Specifies StreamId
        /// </summary>
        /// <param name="streamId">Stream ID to be set</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg StreamId(int streamId)
        {
            m_statusMsgEncoder.StreamId(streamId);
            return this;
        }

        /// <summary>
        /// Specifies DomainType
        /// </summary>
        /// <param name="domainType">The RDM domain type defined in defined in <see cref="Rdm.EmaRdm"/> or user defined.</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg DomainType(int domainType)
        {
            m_statusMsgEncoder.DomainType(domainType);
            return this;
        }

        /// <summary>
        /// Specifies item name
        /// </summary>
        /// <param name="name">Item name to be set</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg Name(string name)
        {
            m_statusMsgEncoder.Name(name);
            return this;
        }

        /// <summary>
        /// Specifies Name type
        /// </summary>
        /// <param name="nameType">specifies RDM Instrument NameType defined in <see cref="Rdm.EmaRdm"/>.</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg NameType(int nameType)
        {
            m_statusMsgEncoder.NameType(nameType);
            return this;
        }

        /// <summary>
        /// Specifies ServiceName.
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceName">specifies service name</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg ServiceName(string serviceName)
        {
            SetMsgServiceName(serviceName);
            return this;
        }

        /// <summary>
        /// Specifies ServiceId.
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceId">specifies service id</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg ServiceId(int serviceId)
        {
            m_statusMsgEncoder.ServiceId(serviceId);
            return this;
        }

        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">the id to be set</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg Id(int id)
        {
            m_statusMsgEncoder.Identifier(id);
            return this;
        }

        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">filter value to be set</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg Filter(long filter)
        {
            m_statusMsgEncoder.Filter(filter);
            return this;
        }

        /// <summary>
        /// Specifies State.
        /// </summary>
        /// <param name="streamState">conveys item stream state value</param>
        /// <param name="dataState">conveys item data state value</param>
        /// <param name="statusCode">conveys specific item state code</param>
        /// <param name="statusText">conveys item status explanation</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg State(int streamState, int dataState, int statusCode = 0, string statusText = "")
        {
            m_statusMsgEncoder.State(streamState, dataState, statusCode, statusText);
            return this;
        }

        /// <summary>
        /// Specifies ItemGroup.
        /// </summary>
        /// <param name="itemGroup">a <see cref="EmaBuffer"/> object with item group information</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg ItemGroup(EmaBuffer itemGroup)
        {
            m_statusMsgEncoder.ItemGroup(itemGroup);
            return this;
        }

        /// <summary>
        /// Specifies PermissionData.
        /// </summary>
        /// <param name="permissionData">a <see cref="EmaBuffer"/> object with permission data information</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg PermissionData(EmaBuffer permissionData)
        {
            m_statusMsgEncoder.PermissionData(permissionData);
            return this;
        }

        /// <summary>
        /// Specifies PublisherId.
        /// </summary>
        /// <param name="userId">specifies publisher's user id</param>
        /// <param name="userAddress">specifies publisher's user address</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg PublisherId(long userId, long userAddress)
        {
            m_statusMsgEncoder.PublisherId(userId, userAddress);
            return this;
        }

        /// <summary>
        /// Specifies Attrib.
        /// </summary>
        /// <param name="data">an object of IComplexType type that contains attribute information</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg Attrib(ComplexType data)
        {
            m_attrib.SetExternalData(data);
            m_statusMsgEncoder.Attrib(data);
            return this;
        }

        /// <summary>
        /// Specifies Payload.
        /// </summary>
        /// <param name="payload">payload to be set</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg Payload(ComplexType payload)
        {
            m_payload.SetExternalData(payload);
            m_statusMsgEncoder.Payload(payload);
            return this;
        }

        /// <summary>
        /// Specifies ExtendedHeader.
        /// </summary>
        /// <param name="buffer">a EmaBuffer containing extendedHeader information</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg ExtendedHeader(EmaBuffer buffer)
        {
            m_statusMsgEncoder.ExtendedHeader(buffer);
            return this;
        }

        /// <summary>
        /// Specifies ClearCache.
        /// </summary>
        /// <param name="clearCache">set to true if cache needs to be cleared; false otherwise</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg ClearCache(bool clearCache)
        {
            m_statusMsgEncoder.ClearCache(clearCache);
            return this;
        }

        /// <summary>
        /// Specifies PrivateStream.
        /// </summary>
        /// <param name="privateStream">specifies if this is a private stream (default is false)</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        public StatusMsg PrivateStream(bool privateStream)
        {
            m_statusMsgEncoder.PrivateStream(privateStream);
            return this;
        }

        /// <summary>
        /// Provides string representation of the current StatusMsg instance
        /// </summary>
        /// <returns>string representing current <see cref="StatusMsg"/> object.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        /// <summary>
        /// Creates object that is a copy of the current object.
        /// </summary>
        /// <returns><see cref="StatusMsg"/> instance that is a copy of the current StatusMsg.</returns>
        public StatusMsg Clone()
        {
            var copy = new StatusMsg();
            CopyMsg(copy);
            return copy;
        }

        object ICloneable.Clone()
        {
            return Clone();
        }

        /// <summary>
        /// Completes encoding current StatusMsg
        /// </summary>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        internal override void EncodeComplete()
        {
            m_statusMsgEncoder.EncodeComplete();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string FillString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent++).Append("StatusMsg");

            Utilities.AddIndent(m_ToString, indent, true).Append("streamId=\"").Append(StreamId()).Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("domain=\"").Append(Utilities.RdmDomainAsString(DomainType())).Append("\"");

            if (PrivateStream())
                Utilities.AddIndent(m_ToString, indent, true).Append("privateStream");

            if (HasState)
                Utilities.AddIndent(m_ToString, indent, true).Append("state=\"").Append(State().ToString()).Append("\"");

            if (HasItemGroup)
            {
                Utilities.AddIndent(m_ToString, indent, true).Append("itemGroup=\"");
                Utilities.AsHexString(m_ToString, ItemGroup()).Append("\"");
            }

            if (HasPermissionData)
            {
                Utilities.AddIndent(m_ToString, indent, true).Append("permissionData=\"");
                Utilities.AsHexString(m_ToString, PermissionData()).Append("\"");
            }

            if (ClearCache())
                Utilities.AddIndent(m_ToString, indent, true).Append("clearCache");

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
                    Utilities.AddIndent(m_ToString, indent, true).Append("Attrib dataType=\"").Append(Access.DataType.AsString(Attrib().DataType))
                                                                 .Append("\"\n");

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
                                                         .Append(Access.DataType.AsString(Payload().DataType))
                                                         .Append("\"\n");

            indent++;
            m_ToString.Append(Payload().Data.ToString(indent));
            indent--;

            Utilities.AddIndent(m_ToString, indent).Append("PayloadEnd");
            indent--;

            Utilities.AddIndent(m_ToString, indent, true).Append("StatusMsgEnd\n");
            return m_ToString.ToString();
        }
    }
}