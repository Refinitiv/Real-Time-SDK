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

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Directory Status. 
    /// Used by a Provider application to indicate changes to the Directory stream.
    /// </summary>
    sealed public class DirectoryStatus : MsgBase
    {
        private IStatusMsg m_StatusMsg = new Msg();
        private State m_State = new State();

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get => m_StatusMsg.StreamId; set { m_StatusMsg.StreamId = value; } }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.SOURCE"/>.
        /// </summary>
        public override int DomainType { get => m_StatusMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.STATUS"/>
        /// </summary>
        public override int MsgClass { get => m_StatusMsg.MsgClass; }

        /// <summary>
        /// Flags for this message.  See <see cref="DirectoryStatusFlags"/>.
        /// </summary>
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
        /// Where possible, this should match the consumer's request. Populated by <see cref="Eta.Rdm.Directory.ServiceFilterFlags"/>
        /// </summary>
        public long Filter { get; set; }
        
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

        /// <summary>
        /// Directory Status Message constructor.
        /// </summary>
        public DirectoryStatus()
        {
            Clear();
        }
        
        /// <summary>
        /// Clears the current contents of the Directory Status object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            Flags = 0;
            m_StatusMsg.Clear();
            m_StatusMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.SOURCE;
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_State.Clear();
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destStatusMsg</c>.
        /// </summary>
        /// <param name="destStatusMsg">DirectoryStatus object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
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

        /// <summary>
        /// Encodes this Directory Status message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
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

            return m_StatusMsg.Encode(encodeIter);
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

        /// <summary>
        /// Returns a human readable string representation of the Directory Status message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, $"DirectoryStatus: {NewLine}");

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.AppendLine();
            }

            if (HasFilter)
            {
                stringBuf.Append(tab);
                stringBuf.Append("filter: ");
                stringBuf.Append(Filter);
                stringBuf.AppendLine();
            }

            if (HasState)
            {
                stringBuf.Append(tab);
                stringBuf.Append("state: ");
                stringBuf.Append(State);
                stringBuf.AppendLine();
            }

            return stringBuf.ToString();
        }
    }
}
