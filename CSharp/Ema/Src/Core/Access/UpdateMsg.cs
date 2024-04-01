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
    /// UpdateMsg conveys changes to item data.
    /// </summary>
    /// <remarks>
    /// Calling an accessor method on an optional member of UpdateMsg must be
    /// preceded by a call to respective Has*** property.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and getting of information from RefreshMsg.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// Decoding of just encoded UpdateMsg in the same application is not supported.
    /// </remarks>
    public sealed class UpdateMsg : Msg, ICloneable
    {
        internal UpdateMsgEncoder m_updateMsgEncoder;

        /// <summary>
        /// Constructor for UpdateMsg
        /// </summary>
        public UpdateMsg()
        {
            m_msgClass = MsgClasses.UPDATE;
            m_rsslMsg.MsgClass = MsgClasses.UPDATE;
            m_updateMsgEncoder = new UpdateMsgEncoder(this);
            Encoder = m_updateMsgEncoder;
            m_msgEncoder = m_updateMsgEncoder;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.UPDATE_MSG;

            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);
        }

        /// <summary>
        /// Copy constructor for <see cref="UpdateMsg"/>
        /// </summary>
        /// <param name="source"><see cref="UpdateMsg"/> to create current message from.</param>
        public UpdateMsg(UpdateMsg source)
        {
            m_msgClass = MsgClasses.UPDATE;
            m_updateMsgEncoder = new UpdateMsgEncoder(this);
            Encoder = m_updateMsgEncoder;
            m_msgEncoder = m_updateMsgEncoder;
            source.CopyMsg(this);
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.UPDATE_MSG;
        }

        internal UpdateMsg(EmaObjectManager objectManager) : base(objectManager)
        {
            m_msgClass = MsgClasses.UPDATE;
            m_rsslMsg.MsgClass = MsgClasses.UPDATE;
            m_updateMsgEncoder = new UpdateMsgEncoder(this);
            Encoder = m_updateMsgEncoder;
            m_msgEncoder = m_updateMsgEncoder;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.UPDATE_MSG;
        }

        /// <summary>
        /// Indicates presence of SeqNum.<br/>
        /// Sequence number is an optional member of UpdateMsg.
        /// </summary>
        public bool HasSeqNum { get => m_rsslMsg.CheckHasSeqNum(); }

        /// <summary>
        /// Indicates presence of PermissionData.<br/>
        /// Permission data is an optional member of UpdateMsg.
        /// </summary>
        public bool HasPermissionData { get => m_rsslMsg.CheckHasPermData(); }

        /// <summary>
        /// Indicates presence of Conflated.
        /// </summary>
        public bool HasConflated { get => m_rsslMsg.CheckHasConfInfo(); }

        /// <summary>
        /// Indicates presence of PublisherId.<br/>
        /// Publisher id is an optional member of UpdateMsg.
        /// </summary>
        public bool HasPublisherId { get => m_rsslMsg.CheckHasPostUserInfo(); }

        /// <summary>
        /// Returns UpdateTypeNum.
        /// </summary>
        /// <returns>update type number</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int UpdateTypeNum()
        {
            return m_rsslMsg.UpdateType;
        }

        /// <summary>
        /// Returns SeqNum.<br/>
        /// Calling this method must be preceded by a call to <see cref="HasSeqNum"/>.
        /// </summary>
        /// <returns>sequence number</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long SeqNum()
        {
            if (!m_rsslMsg.CheckHasSeqNum())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call SeqNum() while it is not set.");
            }

            return m_rsslMsg.SeqNum;
        }

        /// <summary>
        /// Returns PermissionData.<br/>
        /// Calling this method must be preceded by a call to <see cref="HasPermissionData"/>.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing permission data</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public EmaBuffer PermissionData()
        {
            if (!m_rsslMsg.CheckHasPermData())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PermissionData() while it is not set.");
            }

            return GetPermDataEmaBuffer();
        }

        /// <summary>
        /// Returns ConflatedTime.<br/>
        /// Calling this method must be preceded by a call to <see cref="HasConflated"/>.
        /// </summary>
        /// <returns>time conflation was on</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int ConflatedTime()
        {
            if (!m_rsslMsg.CheckHasConfInfo())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call ConflatedTime() while ConfInfo is not set.");
            }

            return m_rsslMsg.ConflationTime;
        }

        /// <summary>
        /// Returns ConflatedCount.<br/>
        /// Calling this method must be preceded by a call to <see cref="HasConflated"/>.
        /// </summary>
        /// <returns>number of conflated updates</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int ConflatedCount()
        {
            if (!m_rsslMsg.CheckHasConfInfo())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call ConflatedCount() while ConfInfo is not set.");
            }

            return m_rsslMsg.ConflationCount;
        }

        /// <summary>
        /// Returns PublisherIdUserId.
        /// </summary>
        /// <returns>publisher's user Id</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long PublisherIdUserId()
        {
            if (!m_rsslMsg.CheckHasPostUserInfo())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PublisherIdUserId() while PostUserInfo is not set.");
            }

            return m_rsslMsg.PostUserInfo.UserId;
        }

        /// <summary>
        /// Returns PublisherIdUserAddress.<br/>
        /// Calling this method must be preceded by a call to <see cref="HasPublisherId"/>
        /// </summary>
        /// <returns>publisher's user address</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long PublisherIdUserAddress()
        {
            if (!m_rsslMsg.CheckHasPostUserInfo())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PublisherIdUserAddress() while PostUserInfo is not set.");
            }

            return m_rsslMsg.PostUserInfo.UserAddr;
        }

        /// <summary>
        /// Returns DoNotCache.
        /// </summary>
        /// <returns>true if this refresh must not be cached; false otherwise</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool DoNotCache()
        {
            return m_rsslMsg.CheckDoNotCache();
        }

        /// <summary>
        /// Returns DoNotConflate.
        /// </summary>
        /// <returns>true if this update must not be conflated; false otherwise</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool DoNotConflate()
        {
            return m_rsslMsg.CheckDoNotConflate();
        }

        /// <summary>
        /// Returns DoNotRipple.
        /// </summary>
        /// <returns>true if this update does not ripple; false otherwise</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool DoNotRipple()
        {
            return m_rsslMsg.CheckDoNotRipple();
        }

        /// <summary>
        /// Clears the RefreshMsg.
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Clear()
        {
            Clear_All();
            DomainType(Rdm.EmaRdm.MMT_MARKET_PRICE);
            return this;
        }

        /// <summary>
        /// Specifies StreamId
        /// </summary>
        /// <param name="streamId">Stream ID to be set</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg StreamId(int streamId)
        {
            m_updateMsgEncoder.StreamId(streamId);
            return this;
        }

        /// <summary>
        /// Specifies DomainType
        /// </summary>
        /// <param name="domainType">The RDM domain type defined in <see cref="Rdm.EmaRdm"/> or user defined.</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg DomainType(int domainType)
        {
            m_updateMsgEncoder.DomainType(domainType);
            return this;
        }

        /// <summary>
        /// Specifies item name
        /// </summary>
        /// <param name="name">Stream item name to be set</param>
        /// <returns>Reference to current <see cref="StatusMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Name(string name)
        {
            m_updateMsgEncoder.Name(name);
            return this;
        }

        /// <summary>
        /// Specifies Name type
        /// </summary>
        /// <param name="nameType">specifies the RDM Instrument NameType defined in <see cref="Rdm.EmaRdm"/>.</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg NameType(int nameType)
        {
            m_updateMsgEncoder.NameType(nameType);
            return this;
        }

        /// <summary>
        /// Specifies ServiceName.<br/>
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceName">specifies service name</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg ServiceName(string serviceName)
        {
            SetMsgServiceName(serviceName);
            return this;
        }

        /// <summary>
        /// Specifies ServiceId.<br/>
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceId">specifies service id</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg ServiceId(int serviceId)
        {
            m_updateMsgEncoder.ServiceId(serviceId);
            return this;
        }

        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">the id to be set</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Id(int id)
        {
            m_updateMsgEncoder.Identifier(id);
            return this;
        }

        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">filter value to be set</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Filter(long filter)
        {
            m_updateMsgEncoder.Filter(filter);
            return this;
        }

        /// <summary>
        /// Specifies UpdateTypeNum.
        /// </summary>
        /// <param name="updateTypeNum">specifies update type number</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg UpdateTypeNum(int updateTypeNum)
        {
            m_updateMsgEncoder.UpdateTypeNum(updateTypeNum);
            return this;
        }

        /// <summary>
        /// Specifies SeqNum.<br/>
        /// throws OmmOutOfRangeException if seqNum is less than 0 or greater than 4294967295
        /// </summary>
        /// <param name="seqNum">specifies sequence number</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg SeqNum(long seqNum)
        {
            m_updateMsgEncoder.SeqNum(seqNum);
            return this;
        }

        /// <summary>
        /// Specifies PermissionData.
        /// </summary>
        /// <param name="permissionData">a <see cref="EmaBuffer"/> object with permission data information</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg PermissionData(EmaBuffer permissionData)
        {
            m_updateMsgEncoder.PermissionData(permissionData);
            return this;
        }

        /// <summary>
        /// Specifies PublisherId.
        /// </summary>
        /// <param name="userId">specifies publisher's user id</param>
        /// <param name="userAddress">specifies publisher's user address</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg PublisherId(long userId, long userAddress)
        {
            m_updateMsgEncoder.PublisherId(userId, userAddress);
            return this;
        }

        /// <summary>
        /// Specifies Conflated.
        /// </summary>
        /// <param name="count">specifies how many updates were conflated</param>
        /// <param name="time">specifies how long the conflation was on</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Conflated(int count, int time)
        {
            m_updateMsgEncoder.Conflated(count, time);
            return this;
        }

        /// <summary>
        /// Specifies Attrib.
        /// </summary>
        /// <param name="data">an object of IComplexType type that contains attribute information</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Attrib(ComplexType data)
        {
            m_attrib.SetExternalData(data);
            m_updateMsgEncoder.Attrib(data);
            return this;
        }

        /// <summary>
        /// Specifies Payload.
        /// </summary>
        /// <param name="payload">payload to be set</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Payload(ComplexType payload)
        {
            m_payload.SetExternalData(payload);
            m_updateMsgEncoder.Payload(payload);
            return this;
        }

        /// <summary>
        /// Specifies ExtendedHeader.
        /// </summary>
        /// <param name="buffer">a EmaBuffer containing extendedHeader information</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg ExtendedHeader(EmaBuffer buffer)
        {
            m_updateMsgEncoder.ExtendedHeader(buffer);
            return this;
        }

        /// <summary>
        /// Specifies DoNotCache.
        /// </summary>
        /// <param name="doNotCache">true if this update must not be cached; false otherwise</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg DoNotCache(bool doNotCache)
        {
            m_updateMsgEncoder.DoNotCache(doNotCache);
            return this;
        }

        /// <summary>
        /// Specifies DoNotConflate.
        /// </summary>
        /// <param name="doNotConflate">true if this update must not be conflated; false otherwise</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg DoNotConflate(bool doNotConflate)
        {
            m_updateMsgEncoder.DoNotConflate(doNotConflate);
            return this;
        }

        /// <summary>
        /// Specifies DoNotRipple.
        /// </summary>
        /// <param name="doNotRipple">true if this update does not ripple; false otherwise</param>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg DoNotRipple(bool doNotRipple)
        {
            m_updateMsgEncoder.DoNotRipple(doNotRipple);
            return this;
        }

        /// <summary>
        /// Provides string representation of the current UpdateMsg instance
        /// </summary>
        /// <returns>string representing current <see cref="UpdateMsg"/> object.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        /// <summary>
        /// Creates object that is a copy of the current object.
        /// </summary>
        /// <returns><see cref="UpdateMsg"/> instance that is a copy of the current UpdateMsg.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public UpdateMsg Clone()
        {
            var copy = new UpdateMsg();
            CopyMsg(copy);
            return copy;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        object ICloneable.Clone()
        {
            return Clone();
        }

        /// <summary>
        /// Completes encoding current UpdateMsg
        /// </summary>
        /// <returns>Reference to current <see cref="UpdateMsg"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override void EncodeComplete()
        {
            m_updateMsgEncoder.EncodeComplete();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string FillString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent++).Append("UpdateMsg");

            Utilities.AddIndent(m_ToString, indent, true).Append("streamId=\"").Append(StreamId()).Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("domain=\"").Append(Utilities.RdmDomainAsString(DomainType())).Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("updateTypeNum=\"").Append(UpdateTypeNum()).Append("\"");

            if (HasPermissionData)
            {
                Utilities.AddIndent(m_ToString, indent, true).Append("permissionData=\"");
                Utilities.AsHexString(m_ToString, PermissionData()).Append("\"");
            }

            if (HasSeqNum)
                Utilities.AddIndent(m_ToString, indent, true).Append("seqNum=\"").Append(SeqNum()).Append("\"");

            if (DoNotCache())
                Utilities.AddIndent(m_ToString, indent, true).Append("doNotCache");

            if (DoNotRipple())
                Utilities.AddIndent(m_ToString, indent, true).Append("doNotRipple");

            if (DoNotConflate())
                Utilities.AddIndent(m_ToString, indent, true).Append("doNotConflate");

            if (HasConflated)
                Utilities.AddIndent(m_ToString, indent, true).Append("confInfo")
                                                         .Append(" count=\"")
                                                         .Append(ConflatedCount()).Append("\"")
                                                         .Append(" time=\"").Append(ConflatedTime())
                                                         .Append("\"");

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
            Utilities.AddIndent(m_ToString, indent, true).Append("Payload dataType=\"").Append(Access.DataType.AsString(Payload().DataType))
                                                         .Append("\"\n");

            indent++;
            m_ToString.Append(Payload().ToString(indent));
            indent--;

            Utilities.AddIndent(m_ToString, indent).Append("PayloadEnd");
            indent--;

            Utilities.AddIndent(m_ToString, indent, true).Append("UpdateMsgEnd\n");
            return m_ToString.ToString();
        }
    }
}