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
    /// PostMsg allows consumer applications to contribute content.
    /// 
    /// <p>PostMsg may be submitted on any market item stream or login stream.
    /// Submission on a market item stream is referred to as the "on stream posting"
    /// while submission on a login stream is considered as the "off stream posting".
    /// On stream posting content is related to the item on whose stream the posting happens,
    /// while the off stream posting may contain info about any item.</p>
    /// 
    /// <p>PostMsg may be submitted using <see cref="OmmConsumer.Submit(PostMsg, long)"/>.</p>
    ///
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform setting and getting of information from PostMsg.
    /// Objects of this class are not cache-able.
    /// Decoding of just encoded PostMsg in the same application is not supported.
    /// </summary>
    public sealed class PostMsg : Msg, ICloneable
    {
        internal PostMsgEncoder m_postMsgEncoder;

        /// <summary>
        /// Constructor for PostMsg
        /// </summary>
        public PostMsg()
        {
            m_msgClass = MsgClasses.POST;
            m_rsslMsg.MsgClass = MsgClasses.POST;
            m_postMsgEncoder = new PostMsgEncoder(this);
            Encoder = m_postMsgEncoder;
        }

        /// <summary>
        /// Copy constructor for <see cref="PostMsg"/>
        /// </summary>
        /// <param name="source"><see cref="PostMsg"/> to create current message from.</param>
        public PostMsg(PostMsg source)
        {
            m_msgClass = MsgClasses.POST;
            m_postMsgEncoder = new PostMsgEncoder(this);
            Encoder = m_postMsgEncoder;
            source.CopyMsg(this);
        }

        internal PostMsg(EmaObjectManager objectManager) : base(objectManager)
        {
            m_msgClass = MsgClasses.POST;
            m_rsslMsg.MsgClass = MsgClasses.POST;
            m_postMsgEncoder = new PostMsgEncoder(this);
            Encoder = m_postMsgEncoder;
        }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.POST_MSG;

        /// <summary>
        /// Indicates presence of SeqNum.
        /// Sequence number is an optional member of PostMsg.
        /// </summary>
        public bool HasSeqNum { get => m_rsslMsg.CheckHasSeqNum(); }
        
        /// <summary>
        /// Indicates presence of PostId.
        /// Post id is an optional member of PostMsg.
        /// </summary>
        public bool HasPostId { get => m_rsslMsg.CheckHasPostId(); }
        
        /// <summary>
        /// Indicates presence of PartNum.
        /// Part number is an optional member of PostMsg.
        /// </summary>
        public bool HasPartNum { get => m_rsslMsg.CheckHasPartNum(); }
        
        /// <summary>
        /// Indicates presence of PostUserRights.
        /// Post user rights is an optional member of PostMsg.
        /// </summary>
        public bool HasPostUserRights { get => m_rsslMsg.CheckHasPostUserRights(); }
        
        /// <summary>
        /// Indicates presence of PermissionData.
        /// Permission data is optional on PostMsg.
        /// </summary>
        public bool HasPermissionData { get => m_rsslMsg.CheckHasPermData(); }
        
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
        /// Returns the PostId.
        /// Calling this method must be preceded by a call to <see cref="HasPostId"/>.
        /// </summary>
        /// <returns></returns>
        public long PostId()
        {
            if (!m_rsslMsg.CheckHasPostId())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PostId() while it is not set.");
            }

            return m_rsslMsg.PostId;
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
        /// Returns PostUserRights.
        /// Calling this method must be preceded by a call to <see cref="HasPostUserRights"/>.
        /// </summary>
        /// <returns></returns>
        public int PostUserRights()
        {
            if (!m_rsslMsg.CheckHasPostUserRights())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call PostUserRights() while it is not set.");
            }

            return m_rsslMsg.PostUserRights;
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
        /// Indicates that acknowledgment is requested.
        /// </summary>
        /// <returns>true if acknowledgment is requested; false otherwise</returns>
        public bool SolicitAck() 
        {
            return m_rsslMsg.CheckAck();
        }

        /// <summary>
        /// True if this is the last part of the multi part post message, false otherwise.
        /// </summary>
        public bool Complete() 
        { 
            return (m_rsslMsg.Flags & PostMsgFlags.POST_COMPLETE) > 0; 
        }
        
        /// <summary>
        /// Clears the PostMsg.
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg Clear() 
        {
            ClearInt();
            return this;
        }
        
        /// <summary>
        /// Specifies StreamId
        /// </summary>
        public PostMsg StreamId(int streamId) 
        {
            m_postMsgEncoder.StreamId(streamId);
            return this;
        }
        
        /// <summary>
        /// Specifies DomainType
        /// </summary>
        public PostMsg DomainType(int domainType) 
        {
            m_postMsgEncoder.DomainType(domainType);
            return this;
        }
        
        /// <summary>
        /// Specifies the name
        /// </summary>
        public PostMsg Name(string name)
        {
            m_postMsgEncoder.Name(name);
            return this;
        }
        
        /// <summary>
        /// Specifies Name type
        /// </summary>
        public PostMsg NameType(int nameType)
        {
            m_postMsgEncoder.NameType(nameType);
            return this;
        }
        
        /// <summary>
        /// Specifies the ServiceId.
        /// </summary>
        public PostMsg ServiceId(int serviceId) 
        { 
            m_postMsgEncoder.ServiceId(serviceId);
            return this;
        }
        
        /// <summary>
        /// Specifies the ServiceName.
        /// </summary>
        public PostMsg ServiceName(string serviceName)
        {
            SetMsgServiceName(serviceName);
            return this;
        }
        
        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">the id to be set</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg Id(int id)
        {
            m_postMsgEncoder.Identifier(id);
            return this;
        }
        
        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">filter value to be set</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg Filter(long filter) 
        {
            m_postMsgEncoder.Filter(filter);
            return this;
        }
        
        /// <summary>
        /// Specifies SeqNum.
        /// </summary>
        /// <param name="seqNum">sequence number value to be set</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg SeqNum(long seqNum) 
        {
            m_postMsgEncoder.SeqNum(seqNum);
            return this;
        }

        /// <summary>
        /// Specifies PostId.
        /// </summary>
        /// <param name="postId">specifies post id value to be set</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg PostId(long postId) 
        {
            m_postMsgEncoder.PostId(postId);
            return this;
        }
        
        /// <summary>
        /// Specifies PartNum.
        /// </summary>
        /// <param name="partNum">specifies part number value</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg PartNum(int partNum) 
        {
            m_postMsgEncoder.PartNum(partNum);
            return this;
        }
        
        /// <summary>
        /// Specifies PostUserRights.
        /// </summary>
        /// <param name="postUserRights">specifies post user rights value to be set</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg PostUserRights(int postUserRights) 
        {
            m_postMsgEncoder.PostUserRights(postUserRights);
            return this;
        }
        
        /// <summary>
        /// Specifies PermissionData.
        /// </summary>
        /// <param name="permissionData">a <see cref="EmaBuffer"/> object with permission data information</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg PermissionData(EmaBuffer permissionData) 
        {
            m_postMsgEncoder.PermissionData(permissionData);
            return this;
        }
        
        /// <summary>
        /// Specifies PublisherId.
        /// </summary>
        /// <param name="userId">specifies publisher's user id</param>
        /// <param name="userAddress">specifies publisher's user address</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg PublisherId(long userId, long userAddress)
        {
            m_postMsgEncoder.PublisherId(userId, userAddress);
            return this;
        }

        /// <summary>
        /// Specifies Attrib.
        /// </summary>
        /// <param name="data">an object of IComplexType type that contains attribute information</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg Attrib(ComplexType data)
        {
            m_attrib.SetExternalData(data);
            m_postMsgEncoder.Attrib(data);
            return this;
        }
        
        /// <summary>
        /// Specifies Payload.
        /// </summary>
        /// <param name="payload">payload to be set</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg Payload(ComplexType payload)
        {
            m_payload.SetExternalData(payload);
            m_postMsgEncoder.Payload(payload);
            return this;
        }
        
        /// <summary>
        /// Specifies ExtendedHeader.
        /// </summary>
        /// <param name="buffer">a EmaBuffer containing extendedHeader information</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg ExtendedHeader(EmaBuffer buffer) 
        {
            m_postMsgEncoder.ExtendedHeader(buffer);
            return this;
        }
        
        /// <summary>
        /// Specifies acknowledgment.
        /// </summary>
        /// <param name="ack">specifies if an acknowledgment is requested</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg SolicitAck(bool ack) 
        {
            m_postMsgEncoder.SolicitAck(ack);
            return this;
        }
        
        /// <summary>
        /// Specifies Complete.
        /// Must be set to true for one part post message.
        /// </summary>
        /// <param name="complete">specifies if this is the last part 
        /// of the multi part post message</param>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        public PostMsg Complete(bool complete) 
        {           
            m_postMsgEncoder.Complete(complete);
            return this;
        }

        /// <summary>
        /// Provides string representation of the current PostMsg instance
        /// </summary>
        /// <returns>string representing current <see cref="PostMsg"/> instance.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        /// <summary>
        /// Creates object that is a copy of the current object.
        /// </summary>
        /// <returns><see cref="PostMsg"/> instance that is a copy of the current PostMsg.</returns>
        public PostMsg Clone()
        {
            var copy = new PostMsg();
            CopyMsg(copy);
            return copy;
        }

        object ICloneable.Clone()
        {
            return Clone();
        }

        /// <summary>
        /// Completes encoding current PostMsg.
        /// </summary>
        /// <returns>reference to current <see cref="PostMsg"/> instance</returns>
        internal override void EncodeComplete()
        {
            m_postMsgEncoder.EncodeComplete();
        }

        internal override string ToString(int indent)
        {
            if (m_objectManager == null)
                return "\nDecoding of just encoded object in the same application is not supported\n";

            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent++).Append("PostMsg");
            Utilities.AddIndent(m_ToString, indent, true).Append("streamId=\"")
                                                          .Append(StreamId())
                                                          .Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("domain=\"")
                                                          .Append(Utilities.RdmDomainAsString(DomainType()))
                                                          .Append("\"");

            if (SolicitAck())
                Utilities.AddIndent(m_ToString, indent, true).Append("Ack Requested");

            if (Complete())
                Utilities.AddIndent(m_ToString, indent, true).Append("MessageComplete");

            if (HasSeqNum)
                Utilities.AddIndent(m_ToString, indent, true).Append("seqNum=\"")
                                                              .Append(SeqNum()).Append("\"");

            if (HasPartNum)
                Utilities.AddIndent(m_ToString, indent, true).Append("partNum=\"")
                                                                .Append(PartNum()).Append("\"");

            if (HasPostId)
                Utilities.AddIndent(m_ToString, indent, true).Append("postId=\"")
                                                                .Append(PostId()).Append("\"");

            if (HasPostUserRights)
                Utilities.AddIndent(m_ToString, indent, true).Append("postUserRights=\"")
                                                                .Append(PostUserRights()).Append("\"");

            if (HasPermissionData)
            {
                Utilities.AddIndent(m_ToString, indent, true).Append("permissionData=\"");
                Utilities.AsHexString(m_ToString, PermissionData()).Append("\"");
            }

            Utilities.AddIndent(m_ToString, indent, true).Append("publisherIdUserId=\"")
                                                          .Append(PublisherIdUserId()).Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("publisherIdUserAddress=\"")
                                                            .Append(PublisherIdUserAddress()).Append("\"");

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

            Utilities.AddIndent(m_ToString, indent, true).Append("PostMsgEnd\n");

            return m_ToString.ToString();
        }
    }
}
