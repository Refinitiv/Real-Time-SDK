/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Diagnostics;
using System.Text;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Dictionary Status. 
    /// Used by a Provider application to indicate changes to the Dictionary stream.
    /// </summary>
    public class DictionaryStatus : MsgBase
    {
        private IStatusMsg m_DictionaryStatus = new Msg();
        private State m_State = new State();

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get => m_DictionaryStatus.StreamId; set { m_DictionaryStatus.StreamId = value; } }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.DICTIONARY"/>.
        /// </summary>
        public override int DomainType { get => m_DictionaryStatus.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.STATUS"/>
        /// </summary>
        public override int MsgClass { get => m_DictionaryStatus.MsgClass; }

        /// <summary>
        /// Flags for this message.  See <see cref="DictionaryStatusFlags"/>.
        /// </summary>
        public DictionaryStatusFlags Flags { get; set; }

        /// <summary>
        /// Checks the presence of clear cache flag.
        /// </summary>
        public bool ClearCache
        {
            get => (Flags & DictionaryStatusFlags.CLEAR_CACHE) != 0;
            set
            {
                if (value)
                {
                    Flags |= DictionaryStatusFlags.CLEAR_CACHE;
                }
                else
                {
                    Flags &= ~DictionaryStatusFlags.CLEAR_CACHE;
                }
            }
        }
        /// <summary>
        /// Checks the presence of state field.
        /// </summary>
        public bool HasState
        {
            get => (Flags & DictionaryStatusFlags.HAS_STATE) != 0;
            set
            {
                if (value)
                {
                    Flags |= DictionaryStatusFlags.HAS_STATE;
                }
                else
                {
                    Flags &= ~DictionaryStatusFlags.HAS_STATE;
                }
            }
        }

        /// <summary>
        /// The current state of the stream.
        /// </summary>
        public State State
        {
            get => m_State;
            set
            {
                Debug.Assert(value != null);
                value.Copy(m_State);
            }
        }

        /// <summary>
        /// Dictionary Status Message constructor.
        /// </summary>
        public DictionaryStatus()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Dictionary Status object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            Flags = 0;
            m_DictionaryStatus.Clear();
            m_DictionaryStatus.MsgClass = MsgClasses.STATUS;
            m_DictionaryStatus.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
            m_State.StreamState(StreamStates.OPEN);
            m_State.DataState(DataStates.OK);
            m_State.Code(StateCodes.NONE);
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destStatusMsg</c>.
        /// </summary>
        /// <param name="destStatusMsg">DictionaryStatus object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(DictionaryStatus destStatusMsg)
        {
            if (destStatusMsg == null)
            {
                return CodecReturnCode.FAILURE;
            }

            destStatusMsg.Clear();
            destStatusMsg.StreamId = StreamId;
            destStatusMsg.ClearCache = ClearCache;
            if (HasState)
            {
                destStatusMsg.HasState = true;
                destStatusMsg.State = State;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Directory Status message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            if (ClearCache)
            {
                m_DictionaryStatus.ApplyClearCache();
            }
            if (HasState)
            {
                m_DictionaryStatus.ApplyHasState();
                m_DictionaryStatus.State = m_State;
            }

            return m_DictionaryStatus.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Directory Status using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this Directory Status message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.STATUS)
            {
                return CodecReturnCode.FAILURE;
            }
            IStatusMsg statusMsg = (IStatusMsg)msg;
            StreamId = msg.StreamId;

            if (statusMsg.CheckHasState())
            {
                HasState = true;
                State = statusMsg.State;
            }

            ClearCache = statusMsg.CheckClearCache();

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Dictionary Status message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "DictionaryStatus: \n");

            if (HasState)
            {
                stringBuf.Append(tab);
                stringBuf.Append("state: ");
                stringBuf.Append(State);
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
