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
    /// The RDM Directory Status. 
    /// Used by a Provider application to indicate changes to the Directory stream.
    /// </summary>
    public class DirectoryStatus : MsgBase
    {
        private IStatusMsg m_StatusMsg = new Msg();
        private State m_State = new State();

        public DirectoryStatus()
        {
            Clear();
        }

        public DirectoryStatusFlags Flags { get; set; }

        /// <summary>
        /// Handles the presence of the filter field.
        /// </summary>
        public bool HasFilter { 
            get => (Flags & DirectoryStatusFlags.HAS_FILTER) != 0; 
            set 
            { 
                if (value) 
                { 
                    Flags |= DirectoryStatusFlags.HAS_FILTER; 
                } 
                else 
                { 
                    Flags &= ~DirectoryStatusFlags.HAS_FILTER; 
                } 
            } 
        }
        /// <summary>
        /// Handles the service id flag.
        /// </summary>
        public bool HasServiceId
        {
            get => (Flags & DirectoryStatusFlags.HAS_SERVICE_ID) != 0;
            set
            {
                if (value)
                {
                    Flags |= DirectoryStatusFlags.HAS_SERVICE_ID;
                }
                else
                {
                    Flags &= ~DirectoryStatusFlags.HAS_SERVICE_ID;
                }
            }
        }
        /// <summary>
        /// Handles state presence flag.
        /// </summary>
        public bool HasState
        {
            get => (Flags & DirectoryStatusFlags.HAS_STATE) != 0;
            set
            {
                if (value)
                {
                    Flags |= DirectoryStatusFlags.HAS_STATE;
                }
                else
                {
                    Flags &= ~DirectoryStatusFlags.HAS_STATE;
                }
            }
        }
        /// <summary>
        /// Handles the presence of clear cache flag.
        /// </summary>
        public bool ClearCache
        {
            get => (Flags & DirectoryStatusFlags.CLEAR_CACHE) != 0;
            set
            {
                if (value)
                {
                    Flags |= DirectoryStatusFlags.CLEAR_CACHE;
                }
                else
                {
                    Flags &= ~DirectoryStatusFlags.CLEAR_CACHE;
                }
            }
        }
        /// <summary>
        /// Filter indicating which filters may appear on this stream. 
        /// Where possible, this should match the consumer's request. Populated by <see cref="Directory.ServiceFilterFlags"/>
        /// </summary>
        public long Filter { get; set; }
        
        public override int StreamId { get => m_StatusMsg.StreamId; set { m_StatusMsg.StreamId = value; } }
        public override int DomainType { get => m_StatusMsg.DomainType; }
        public override int MsgClass { get => m_StatusMsg.MsgClass; }

        /// <summary>
        /// The ID of the service whose information is provided by this stream 
        /// (if not present, all services should be provided). 
        /// Should match the Consumer's request if possible.
        /// </summary>
        public int ServiceId { get; set; }

        /// <summary>
        /// Returns current state of the stream.
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

        public override void Clear()
        {
            Flags = 0;
            m_StatusMsg.Clear();
            m_StatusMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.SOURCE;
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_State.Clear();
        }

        public override CodecReturnCode Decode(DecodeIterator decIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.STATUS)
                return CodecReturnCode.FAILURE;

            IStatusMsg statusMsg = (IStatusMsg)msg;
            StreamId = msg.StreamId;
            if (statusMsg.CheckHasState())
            {
                HasState = true;
                statusMsg.State.Copy(m_State);
            }

            if (statusMsg.CheckClearCache())
            {
                ClearCache = true;
            }

            IMsgKey key = msg.MsgKey;
            if (key != null)
            {
                if (key.CheckHasFilter())
                {
                    HasFilter = true;
                    Filter = key.Filter;
                }

                if (key.CheckHasServiceId())
                {
                    HasServiceId = true;
                    ServiceId = key.ServiceId;
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            if (HasFilter)
            {
                m_StatusMsg.ApplyHasMsgKey();
                m_StatusMsg.MsgKey.ApplyHasFilter();
                m_StatusMsg.MsgKey.Filter = Filter;
            }

            if (HasServiceId)
            {
                m_StatusMsg.ApplyHasMsgKey();
                m_StatusMsg.MsgKey.ApplyHasServiceId();
                m_StatusMsg.MsgKey.ServiceId = ServiceId;
            }

            if (HasState)
            {
                m_StatusMsg.ApplyHasState();
                m_State.Copy(m_StatusMsg.State);
            }

            if (ClearCache)
            {
                m_StatusMsg.ApplyClearCache();
            }

            return m_StatusMsg.Encode(encIter);
        }

        public CodecReturnCode Copy(DirectoryStatus destStatusMsg)
        {
            Debug.Assert(destStatusMsg != null);
            destStatusMsg.Clear();

            destStatusMsg.StreamId = StreamId;
            if (HasFilter)
            {
                destStatusMsg.HasFilter = true;
                destStatusMsg.Filter = Filter;
            }
            if (HasServiceId)
            {
                destStatusMsg.HasServiceId = true;
                destStatusMsg.ServiceId = ServiceId;
            }

            if (ClearCache)
            {
                destStatusMsg.ClearCache = true;
            }

            if (HasState)
            {
                destStatusMsg.HasState = true;
                State.Copy(destStatusMsg.State);
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "DirectoryStatus: \n");

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.Append(eol);
            }

            if (HasFilter)
            {
                stringBuf.Append(tab);
                stringBuf.Append("filter: ");
                stringBuf.Append(Filter);
                stringBuf.Append(eol);
            }

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
