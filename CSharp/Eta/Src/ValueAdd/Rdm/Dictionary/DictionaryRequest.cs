/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Dictionary Request. 
    /// Used by a Consumer application to request a dictionary from a service that provides it.
    /// </summary>
    public class DictionaryRequest : MsgBase
    {

        private IRequestMsg m_DictionaryRequest = new Msg();
        
        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get => m_DictionaryRequest.StreamId; set { m_DictionaryRequest.StreamId = value; } }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.DICTIONARY"/>.
        /// </summary>
        public override int DomainType { get => m_DictionaryRequest.DomainType; }
        
        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.REQUEST"/>
        /// </summary>
        public override int MsgClass { get => m_DictionaryRequest.MsgClass; }

        /// <summary>
        /// The ID of the service to request the dictionary from.
        /// </summary>
        public int ServiceId { get => m_DictionaryRequest.MsgKey.ServiceId; set { m_DictionaryRequest.MsgKey.ServiceId = value; } }

        /// <summary>
        /// The name of the dictionary being requested.
        /// </summary>
        public Buffer DictionaryName { get => m_DictionaryRequest.MsgKey.Name; set { m_DictionaryRequest.MsgKey.Name = value; } }

        /// <summary>
        /// Flags for this message.  See <see cref="DictionaryRequestFlags"/>.
        /// </summary>
        public DictionaryRequestFlags Flags { get; set; }

        /// <summary>
        /// Checks if this request is streaming or not.
        /// </summary>
        public bool Streaming
        {
            get => (Flags & DictionaryRequestFlags.STREAMING) != 0;
            set
            {
                if (value)
                {
                    Flags |= DictionaryRequestFlags.STREAMING;
                }
                else
                {
                    Flags &= ~DictionaryRequestFlags.STREAMING;
                }
            }
        }

        /// <summary>
        /// The verbosity of information desired.
        /// </summary>
        public long Verbosity { get => m_DictionaryRequest.MsgKey.Filter; set { m_DictionaryRequest.MsgKey.Filter = value; } }

        /// <summary>
        /// Dictionary Request Message constructor.
        /// </summary>
        public DictionaryRequest()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Dictionary Request object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            m_DictionaryRequest.Clear();
            m_DictionaryRequest.MsgClass = MsgClasses.REQUEST;
            m_DictionaryRequest.MsgKey.ApplyHasFilter();
            m_DictionaryRequest.MsgKey.ApplyHasServiceId();
            m_DictionaryRequest.MsgKey.ApplyHasName();
            m_DictionaryRequest.ContainerType = DataTypes.NO_DATA;
            m_DictionaryRequest.DomainType = (int)LSEG.Eta.Rdm.DomainType.DICTIONARY;
            Flags = 0;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destRequestMsg</c>.
        /// </summary>
        /// <param name="destRequestMsg">DictionaryRequest object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(DictionaryRequest destRequestMsg)
        {
            Debug.Assert(destRequestMsg != null);

            destRequestMsg.StreamId = StreamId;
            destRequestMsg.ServiceId = ServiceId;
            destRequestMsg.Verbosity = Verbosity;
            BufferHelper.CopyBuffer(DictionaryName, destRequestMsg.DictionaryName);
            destRequestMsg.Streaming = Streaming;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Dictionary Request message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            if (Streaming)
            {
                m_DictionaryRequest.ApplyStreaming();
            }
            return m_DictionaryRequest.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Dictionary Request message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this DictionaryRequest message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.REQUEST)
            {
                return CodecReturnCode.FAILURE;
            }
                
            IRequestMsg requestMsg = (IRequestMsg)msg;
            StreamId = requestMsg.StreamId;
            IMsgKey msgKey = msg.MsgKey;
            if (requestMsg.CheckStreaming())
            {
                Streaming = true;
            }
            if (msgKey == null || !msgKey.CheckHasFilter() || !msgKey.CheckHasServiceId() || !msgKey.CheckHasName())
            {
                return CodecReturnCode.FAILURE;
            }

            ServiceId = msgKey.ServiceId;
            Buffer name = msgKey.Name;
            DictionaryName = name;

            Verbosity = msgKey.Filter;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Dictionary Request message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "DictionaryRequest: \n");

            stringBuf.Append(tab);
            stringBuf.Append("serviceId: ");
            stringBuf.Append(ServiceId);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("dictionaryName: ");
            stringBuf.Append(DictionaryName);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("streaming: ");
            stringBuf.Append(Streaming);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("verbosity: ");
            bool addOr = false;
            if ((Verbosity & Dictionary.VerbosityValues.INFO) != 0)
            {
                stringBuf.Append("INFO");
                addOr = true;
            }
            if ((Verbosity & Dictionary.VerbosityValues.MINIMAL) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("MINIMAL");
                addOr = true;
            }
            if ((Verbosity & Dictionary.VerbosityValues.NORMAL) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("NORMAL");
                addOr = true;
            }
            if ((Verbosity & Dictionary.VerbosityValues.VERBOSE) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("VERBOSE");
                addOr = true;
            }
            stringBuf.Append(eol);

            return stringBuf.ToString();
        }
    }
}
