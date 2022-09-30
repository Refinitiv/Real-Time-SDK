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

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Dictionary Status. 
    /// Used by a Provider application to indicate changes to the Dictionary stream.
    /// </summary>
    public class DictionaryStatus : MsgBase
    {
        private IStatusMsg m_DictionaryStatus = new Msg();
        private State m_State = new State();

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

        public override int StreamId { get => m_DictionaryStatus.StreamId; set { m_DictionaryStatus.StreamId = value; } }
        public override int DomainType { get => m_DictionaryStatus.DomainType; }
        public override int MsgClass { get => m_DictionaryStatus.MsgClass; }

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

        public DictionaryStatus()
        {
            Clear();
        }

        public override CodecReturnCode Decode(DecodeIterator encIter, Msg msg)
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

        public override CodecReturnCode Encode(EncodeIterator encIter)
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

            return m_DictionaryStatus.Encode(encIter);
        }

        public CodecReturnCode Copy(DictionaryStatus destMsg)
        {
            if (destMsg == null)
            {
                return CodecReturnCode.FAILURE;
            }

            destMsg.Clear();
            destMsg.StreamId = StreamId;
            destMsg.ClearCache = ClearCache;
            if (HasState)
            {
                destMsg.HasState = true;
                destMsg.State = State;
            }

            return CodecReturnCode.SUCCESS;
        }

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
