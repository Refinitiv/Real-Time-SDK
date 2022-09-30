/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System.Diagnostics;
using System.Text;
using static Refinitiv.Eta.Rdm.Directory;
using DataTypes = Refinitiv.Eta.Codec.DataTypes;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// A Directory request message is encoded and sent by Consumer applications.
    /// A consumer can request information about all services by omitting serviceId information, 
    /// or specify a serviceId to request information about only that service.
    /// </summary>
    public class DirectoryRequest : MsgBase
    {
        private IRequestMsg m_RequestMsg = new Msg();
        private int m_ServiceId = 0;

        /// <summary>
        /// Checks if this request is streaming or not.
        /// </summary>
        public bool Streaming { get => (Flags & DirectoryRequestFlags.STREAMING) != 0; set { if (value) { Flags |= DirectoryRequestFlags.STREAMING; m_RequestMsg.ApplyStreaming(); } else { Flags &= ~DirectoryRequestFlags.STREAMING; } } }
        /// <summary>
        /// Handles the serviceId presence flag.
        /// </summary>
        public bool HasServiceId { get => (Flags & DirectoryRequestFlags.HAS_SERVICE_ID) != 0; set { if (value) { Flags |= DirectoryRequestFlags.HAS_SERVICE_ID; } else { Flags &= ~DirectoryRequestFlags.HAS_SERVICE_ID; } } }
        /// <summary>
        /// The ID of the service to request the directory from.
        /// </summary>
        public int ServiceId { get => m_ServiceId; set { Debug.Assert(HasServiceId); m_ServiceId = value; } }
        /// <summary>
        /// A filter indicating which filters of information the Consumer is interested in. Populated by <see cref="Directory.ServiceFilterFlags"/>
        /// </summary>
        public long Filter { get => m_RequestMsg.MsgKey.Filter; set { m_RequestMsg.MsgKey.Filter = value; } }
        
        public override int StreamId { get => m_RequestMsg.StreamId; set { m_RequestMsg.StreamId = value; } }
        public override int DomainType { get => m_RequestMsg.DomainType; }
        public override int MsgClass { get => m_RequestMsg.MsgClass; }

        public DirectoryRequestFlags Flags { get; set; }

        /// <summary>
        /// Instantiates a new DirectoryRequest message.
        /// </summary>
        public DirectoryRequest()
        {
            Clear();
        }

        public override void Clear()
        {
            Flags = 0;
            m_RequestMsg.Clear();
            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            m_RequestMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_RequestMsg.ContainerType = DataTypes.NO_DATA;
            m_RequestMsg.MsgKey.ApplyHasFilter();           
        }

        public override CodecReturnCode Decode(DecodeIterator decIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.REQUEST)
            {
                return CodecReturnCode.FAILURE;
            }

            IRequestMsg requestMsg = (IRequestMsg)msg;

            if (requestMsg.CheckStreaming())
            {
                Streaming = true;
            }

            IMsgKey msgKey = msg.MsgKey;
            if (msgKey == null || !msgKey.CheckHasFilter())
            {
                return CodecReturnCode.FAILURE;
            }

            if (msgKey.CheckHasFilter())
            {
                Filter = requestMsg.MsgKey.Filter;
            }
            StreamId = msg.StreamId;
            if (msgKey.CheckHasServiceId())
            {
                HasServiceId = true;
                ServiceId = msgKey.ServiceId;
            }
            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            if (Streaming)
            {
                m_RequestMsg.ApplyStreaming();
            }
            if (HasServiceId)
            {
                m_RequestMsg.MsgKey.ApplyHasServiceId();
                m_RequestMsg.MsgKey.ServiceId = ServiceId;
            }
            return m_RequestMsg.Encode(encIter);
        }

        public CodecReturnCode Copy(DirectoryRequest destRequestMsg)
        {
            Debug.Assert(destRequestMsg != null);
            destRequestMsg.Clear();

            destRequestMsg.StreamId = StreamId;
            destRequestMsg.Filter = Filter;

            if (HasServiceId)
            {
                destRequestMsg.HasServiceId = true;
                destRequestMsg.ServiceId = ServiceId;
            }
            if (Streaming)
            {
                destRequestMsg.Streaming = true;
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "DirectoryRequest: \n");

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.Append(eol);
            }

            stringBuf.Append(tab);
            stringBuf.Append("streaming: ");
            stringBuf.Append(Streaming);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("filter: ");
            bool addOr = false;
            if ((Filter & ServiceFilterFlags.INFO) != 0)
            {
                stringBuf.Append("INFO");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.DATA) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("DATA");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.GROUP) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("GROUP");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.LINK) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("LINK");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.LOAD) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("LOAD");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.STATE) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("STATE");
            }
            stringBuf.Append(eol);

            return stringBuf.ToString();
        }
    }
}
