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
    /// RefreshMsg conveys item image, state, permission and group information.
    /// 
    /// <p>RefreshMsg is sent when item data needs to be synchronized.
    /// This happens as a response to received ReqMsg or when upstream source requires it.</p>
    /// 
    /// <p>RefreshMsg sent as a response to ReqMsg is called a solicited refresh,
    /// while an unsolicited refresh is sent when upstream source requires 
    /// synchronization of downstream consumers.</p>
    /// 
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform setting and getting of information from RefreshMsg.
    /// Objects of this class are not cache-able.
    /// Decoding of just encoded RefreshMsg in the same application is not supported.
    /// </summary>
    public sealed class RefreshMsg : Msg, ICloneable
    {
        private OmmQos m_qos = new OmmQos();
        private bool m_qosSet = false;
        private OmmState m_state = new OmmState();
        private bool m_stateSet = false;

        internal RefreshMsgEncoder m_refreshMsgEncoder;

        /// <summary>
        /// Constructor for RefreshMsg
        /// </summary>
        public RefreshMsg() 
        {
            m_msgClass = MsgClasses.REFRESH;
            m_rsslMsg.MsgClass = MsgClasses.REFRESH;
            m_refreshMsgEncoder = new RefreshMsgEncoder(this);
            Encoder = m_refreshMsgEncoder;
            State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "");
        }

        /// <summary>
        /// Copy constructor for <see cref="RefreshMsg"/>
        /// </summary>
        /// <param name="source"><see cref="RefreshMsg"/> to create current message from.</param>
        public RefreshMsg(RefreshMsg source)
        {
            m_msgClass = MsgClasses.REFRESH;
            m_refreshMsgEncoder = new RefreshMsgEncoder(this);
            Encoder = m_refreshMsgEncoder;
            source.CopyMsg(this);
        }

        internal RefreshMsg(EmaObjectManager objectManager) : base(objectManager)
        {
            m_msgClass = MsgClasses.REFRESH;
            m_rsslMsg.MsgClass = MsgClasses.REFRESH;
            m_refreshMsgEncoder = new RefreshMsgEncoder(this);
            Encoder = m_refreshMsgEncoder;
        }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.REFRESH_MSG;

        /// <summary>
        /// Indicates presence of Qos.
        /// Qos is an optional member of RefreshMsg.
        /// </summary>
        public bool HasQos { get => m_rsslMsg.CheckHasQos(); }
        
        /// <summary>
        /// Indicates presence of SeqNum.
        /// Sequence number is an optional member of RefreshMsg.
        /// </summary>
        public bool HasSeqNum { get => m_rsslMsg.CheckHasSeqNum(); }
        
        /// <summary>
        /// Indicates presence of PartNum.
        /// Part number is an optional member of RefreshMsg.
        /// </summary>
        public bool HasPartNum { get => m_rsslMsg.CheckHasPartNum(); }
        
        /// <summary>
        /// Indicates presence of PermissionData.
        /// Permission data is optional on RefreshMsg.
        /// </summary>
        public bool HasPermissionData { get => m_rsslMsg.CheckHasPermData(); }
        
        /// <summary>
        /// Indicates presence of PublisherId.
        /// Publisher id is an optional member of RefreshMsg.
        /// </summary>
        public bool HasPublisherId { get => m_rsslMsg.CheckHasPostUserInfo(); }
 
        /// <summary>
        /// Returns State.
        /// </summary>
        /// <returns>state of the item (e.g. Open / Ok)</returns>
        public OmmState State() 
        { 
            if (!m_stateSet)
            {
                m_state.ClearInt();
                m_state.Decode(m_rsslMsg.State);
                m_stateSet = true;
            }

            return m_state;
        }
        
        /// <summary>
        /// Returns Qos.
        /// Calling this method must be preceded by a call to <see cref="HasQos"/>
        /// </summary>
        /// <returns>Qos of item</returns>
        public OmmQos Qos() 
        { 
            if (!m_rsslMsg.CheckHasQos())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call Qos() while it is not set.");
            }

            if (!m_qosSet)
            {
                m_qos.ClearInt();
                m_qos.Decode(m_rsslMsg.Qos);
                m_qosSet = true;
            }

            return m_qos;
        }
        
        /// <summary>
        /// Returns SeqNum.
        /// Calling this method must be preceded by a call to <see cref="HasSeqNum"/>.
        /// </summary>
        /// <returns>sequence number</returns>
        public long SeqNum()
        {
            if (!m_rsslMsg.CheckHasSeqNum())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call SeqNum() while it is not set.");
            }

            return m_rsslMsg.SeqNum;
        }

        /// <summary>
        /// Returns PartNum.
        /// Calling this method must be preceded by a call to <see cref="HasPartNum"/>.
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
        /// Returns ItemGroup.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing item group information</returns>
        public EmaBuffer ItemGroup()
        {
            return GetItemGroupBuffer();
        }
        
        /// <summary>
        /// Returns PermissionData.
        /// Calling this method must be preceded by a call to <see cref="HasPermissionData"/>.
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
        /// Returns PublisherIdUserId.
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
        /// <returns>publisher's user address</returns>
        public long PublisherIdUserAddress()
        {
            if (!m_rsslMsg.CheckHasPostUserInfo())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PublisherIdUserAddress() while PostUserInfo is not set.");
            }

            return m_rsslMsg.PostUserInfo.UserAddr;
        }

        /// <summary>
        /// Returns Solicited.
        /// </summary>
        /// <returns>true if this is solicited refresh; false otherwise</returns>
        public bool Solicited() 
        {
            return m_rsslMsg.CheckSolicited();
        }
        
        /// <summary>
        /// Returns DoNotCache.
        /// </summary>
        /// <returns>true if this refresh must not be cached; false otherwise</returns>
        public bool DoNotCache() 
        {
            return m_rsslMsg.CheckDoNotCache();
        }
        
        /// <summary>
        /// Returns Complete.
        /// </summary>
        /// <returns>true if this is the last part of the multi part refresh message</returns>
        public bool Complete() 
        {
            return (m_rsslMsg.Flags & RefreshMsgFlags.REFRESH_COMPLETE) > 0;
        }
        /// <summary>
        /// Returns ClearCache.
        /// </summary>
        /// <returns>true if cache needs to be cleared before applying this refresh;
        /// false otherwise</returns>
        public bool ClearCache() 
        {
            return m_rsslMsg.CheckClearCache();
        }
        
        /// <summary>
        /// Determines whether this stream is private or not.
        /// </summary>
        /// <returns>true if this is private stream item</returns>
        public bool PrivateStream() 
        {
            return m_rsslMsg.CheckPrivateStream();
        }
        
        /// <summary>
        /// Clears the RefreshMsg.
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Clear() 
        {
            ClearInt();
            return this;
        }

        /// <summary>
        /// Clears current RefreshMsg instance.
        /// </summary>
        internal override void ClearInt()
        {
            base.ClearInt();

            m_qosSet = false;
            m_stateSet = false;
            m_qos.ClearInt();
            m_state.ClearInt();

            State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "");
        }

        /// <summary>
        /// Specifies StreamId
        /// </summary>
        public RefreshMsg StreamId(int streamId)
        {
            m_refreshMsgEncoder.StreamId(streamId);
            return this;
        }

        /// <summary>
        /// Specifies DomainType
        /// </summary>
        public RefreshMsg DomainType(int domainType)
        {
            m_refreshMsgEncoder.DomainType(domainType);
            return this;
        }
        
        /// <summary>
        /// Specifies the name
        /// </summary>
        public RefreshMsg Name(string name)
        {
            m_refreshMsgEncoder.Name(name);
            return this;
        }

        /// <summary>
        /// Specifies Name type
        /// </summary>
        public RefreshMsg NameType(int nameType)
        {
            m_refreshMsgEncoder.NameType(nameType);
            return this;
        }

        /// <summary>
        /// Specifies the ServiceId.
        /// </summary>
        public RefreshMsg ServiceId(int serviceId)
        {
            m_refreshMsgEncoder.ServiceId(serviceId);
            return this;
        }
        
        /// <summary>
        /// Specifies the ServiceName.
        /// </summary>
        public RefreshMsg ServiceName(string serviceName)
        {
            SetMsgServiceName(serviceName);
            return this;
        }
        
        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">the id to be set</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Id(int id)
        {
            m_refreshMsgEncoder.Identifier(id);
            return this;
        }
        
        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">filter value to be set</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Filter(long filter)
        {
            m_refreshMsgEncoder.Filter(filter);
            return this;
        }
        
        /// <summary>
        /// Specifies Qos.
        /// </summary>
        /// <param name="timeliness">specifies Qos Timeliness</param>
        /// <param name="rate">specifies Qos rate</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Qos(uint timeliness, uint rate) 
        {
            m_refreshMsgEncoder.Qos(timeliness, rate);
            return this; 
        }
        
        /// <summary>
        /// Specifies State.
        /// </summary>
        /// <param name="streamState">conveys item stream state value</param>
        /// <param name="dataState">conveys item data state value</param>
        /// <param name="statusCode">conveys specific item state code</param>
        /// <param name="statusText">conveys item status explanation</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg State(int streamState, int dataState, int statusCode = 0, string statusText = "") 
        { 
            m_refreshMsgEncoder.State(streamState, dataState, statusCode, statusText);
            return this; 
        }
        
        /// <summary>
        /// Specifies SeqNum.
        /// throws OmmOutOfRangeException if seqNum is less than 0 or greater than 4294967295
        /// </summary>
        /// <param name="seqNum">specifies sequence number</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg SeqNum(long seqNum)
        {
            m_refreshMsgEncoder.SeqNum(seqNum);
            return this;
        }

        /// <summary>
        /// Specifies PartNum.
        /// </summary>
        /// <param name="partNum">specifies part number value</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg PartNum(int partNum)
        {
            m_refreshMsgEncoder.PartNum(partNum);
            return this;
        }

        /// <summary>
        /// Specifies ItemGroup.
        /// </summary>
        /// <param name="itemGroup">a <see cref="EmaBuffer"/> object with item group information</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg ItemGroup(EmaBuffer itemGroup)
        {
            m_refreshMsgEncoder.ItemGroup(itemGroup);
            return this;
        }
        
        /// <summary>
        /// Specifies PermissionData.
        /// </summary>
        /// <param name="permissionData">a <see cref="EmaBuffer"/> object with permission data information</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg PermissionData(EmaBuffer permissionData)
        {
            m_refreshMsgEncoder.PermissionData(permissionData);
            return this;
        }
        
        /// <summary>
        /// Specifies PublisherId.
        /// </summary>
        /// <param name="userId">specifies publisher's user id</param>
        /// <param name="userAddress">specifies publisher's user address</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg PublisherId(long userId, long userAddress)
        {
            m_refreshMsgEncoder.PublisherId(userId, userAddress);
            return this;
        }


        /// <summary>
        /// Specifies Attrib.
        /// </summary>
        /// <param name="data">an object of IComplexType type that contains attribute information</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Attrib(ComplexType data)
        {
            m_attrib.SetExternalData(data);
            m_refreshMsgEncoder.Attrib(data);
            return this;
        }
        
        /// <summary>
        /// Specifies Payload.
        /// </summary>
        /// <param name="payload">payload to be set</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Payload(ComplexType payload)
        {
            m_payload.SetExternalData(payload);
            m_refreshMsgEncoder.Payload(payload);
            return this;
        }

        /// <summary>
        /// Specifies ExtendedHeader.
        /// </summary>
        /// <param name="buffer">a EmaBuffer containing extendedHeader information</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg ExtendedHeader(EmaBuffer buffer)
        {
            m_refreshMsgEncoder.ExtendedHeader(buffer);
            return this;
        }

        /// <summary>
        /// Specifies Solicited.
        /// </summary>
        /// <param name="solicited">set to true if this refresh is solicited; false otherwise</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Solicited(bool solicited) 
        {
            m_refreshMsgEncoder.Solicited(solicited);
            return this;
        }
        
        /// <summary>
        /// Specifies DoNotCache.
        /// </summary>
        /// <param name="doNotCache">set to true if this refresh must not be cached; false otherwise</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg DoNotCache(bool doNotCache)
        {
            m_refreshMsgEncoder.DoNotCache(doNotCache);
            return this;
        }

        /// <summary>
        /// Specifies ClearCache.
        /// </summary>
        /// <param name="clearCache">set to true if cache needs to be cleared; false otherwise</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg ClearCache(bool clearCache)
        {
            m_refreshMsgEncoder.ClearCache(clearCache);
            return this;
        }

        /// <summary>
        /// Specifies Complete.
        /// Must be set to true for one part refresh message.
        /// </summary>
        /// <param name="complete">specifies if this is the last part 
        /// of the multi part refresh message</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg Complete(bool complete)
        {
            m_refreshMsgEncoder.Complete(complete);
            return this;
        }

        /// <summary>
        /// Specifies PrivateStream.
        /// </summary>
        /// <param name="privateStream">specifies if this is a private stream (default is false)</param>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        public RefreshMsg PrivateStream(bool privateStream)
        {
            m_refreshMsgEncoder.PrivateStream(privateStream);
            return this;
        }

        /// <summary>
        /// Provides string representation of the current RefreshMsg instance
        /// </summary>
        /// <returns>string representing current <see cref="RefreshMsg"/> instance.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        /// <summary>
        /// Creates object that is a copy of the current object.
        /// </summary>
        /// <returns><see cref="RefreshMsg"/> instance that is a copy of the current RefreshMsg.</returns>
        public RefreshMsg Clone()
        {
            var copy = new RefreshMsg();
            CopyMsg(copy);
            return copy;
        }

        object ICloneable.Clone()
        {
            return Clone();
        }

        /// <summary>
        /// Completes encoding current RefreshMsg
        /// </summary>
        /// <returns>reference to current <see cref="RefreshMsg"/> instance</returns>
        internal override void EncodeComplete()
        {
            m_refreshMsgEncoder.EncodeComplete();
        }

        internal override string ToString(int indent)
        {
            if (m_objectManager == null)
                return "\nDecoding of just encoded object in the same application is not supported\n";

            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent++).Append("RefreshMsg");
            Utilities.AddIndent(m_ToString, indent, true).Append("streamId=\"")
                                                         .Append(StreamId())
                                                         .Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("domain=\"")
                                                         .Append(Utilities.RdmDomainAsString(DomainType()))
                                                         .Append("\"");

            if (Solicited())
                Utilities.AddIndent(m_ToString, indent, true).Append("solicited");

            if (Complete())
                Utilities.AddIndent(m_ToString, indent, true).Append("RefreshComplete");

            if (PrivateStream())
                Utilities.AddIndent(m_ToString, indent, true).Append("privateStream");

            Utilities.AddIndent(m_ToString, indent, true).Append("state=\"")
                                                         .Append(State().ToString())
                                                         .Append("\"");

            Utilities.AddIndent(m_ToString, indent, true).Append("itemGroup=\"");
            Utilities.AsHexString(m_ToString, ItemGroup()).Append("\"");

            if (HasPermissionData)
            {
                Utilities.AddIndent(m_ToString, indent, true).Append("permissionData=\"");
                Utilities.AsHexString(m_ToString, PermissionData()).Append("\"");
            }

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
            m_ToString.Append(Payload().ToString(indent));
            indent--;

            Utilities.AddIndent(m_ToString, indent).Append("PayloadEnd");
            indent--;

            Utilities.AddIndent(m_ToString, indent, true).Append("RefreshMsgEnd\n");

            return m_ToString.ToString();
        }
    }
}
