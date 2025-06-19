/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Diagnostics;
using System.Text;
using static LSEG.Eta.Rdm.Directory;
using DataTypes = LSEG.Eta.Codec.DataTypes;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// A Directory request message is encoded and sent by Consumer applications.
    /// A consumer can request information about all services by omitting serviceId information, 
    /// or specify a serviceId to request information about only that service.
    /// </summary>
    sealed public class DirectoryRequest : MsgBase
    {
        private IRequestMsg m_RequestMsg = new Msg();

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
        public int ServiceId { get; set; }
        
        /// <summary>
        /// A filter indicating which filters of information the Consumer is interested in. Populated by <see cref="ServiceFilterFlags"/>
        /// </summary>
        public long Filter { get => m_RequestMsg.MsgKey.Filter; set { m_RequestMsg.MsgKey.Filter = value; } }

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get => m_RequestMsg.StreamId; set { m_RequestMsg.StreamId = value; } }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.SOURCE"/>.
        /// </summary>
        public override int DomainType { get => m_RequestMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.REQUEST"/>
        /// </summary>
        public override int MsgClass { get => m_RequestMsg.MsgClass; }

        /// <summary>
        /// Flags for this message.  See <see cref="DirectoryRequestFlags"/>.
        /// </summary>
        public DirectoryRequestFlags Flags { get; set; }

        /// <summary>
        /// Directory Refresh Message constructor.
        /// </summary>
        public DirectoryRequest()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Directory Refresh object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            Flags = 0;
            m_RequestMsg.Clear();
            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            m_RequestMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_RequestMsg.ContainerType = DataTypes.NO_DATA;
            m_RequestMsg.MsgKey.ApplyHasFilter();           
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destRequestMsg</c>.
        /// </summary>
        /// <param name="destRequestMsg">DirectoryRequest object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
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

        /// <summary>
        /// Encodes this Directory Request message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
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
            return m_RequestMsg.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Directory Request message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this DirectoryRequest message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
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

        /// <summary>
        /// Returns a human readable string representation of the Directory Request message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, $"DirectoryRequest: {NewLine}");

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.AppendLine();
            }

            stringBuf.Append(tab);
            stringBuf.Append("streaming: ");
            stringBuf.Append(Streaming);
            stringBuf.AppendLine();

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
            stringBuf.AppendLine();

            return stringBuf.ToString();
        }
    }
}
