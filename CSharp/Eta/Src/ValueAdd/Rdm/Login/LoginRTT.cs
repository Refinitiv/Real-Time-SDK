/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// RDM message that carries Round Trip Latency information.
    /// </summary>
    ///
    /// <remarks>
    /// A Round Trip Time (RTT) Login Generic Message exchange is initiated by the
    /// Interactive Provider application. This message must contain the Ticks count, which
    /// is set by the provider before sending the message to a consumer that supports RTT
    /// functionality.
    /// </remarks>
    public class LoginRTT : MsgBase
    {
        #region Private Fields

        private IGenericMsg m_GenericMsg = new Msg();

        private ElementEntry elementEntry = new();
        private ElementList elementList = new();
        private UInt tmpUInt = new();

        #endregion
        #region Public Message Properties

        public enum TimeUnit
        {
            MICRO_SECONDS,
            NANO_SECONDS
        };

        public override int StreamId { get; set; }

        public override int DomainType { get => m_GenericMsg.DomainType; }

        public override int MsgClass { get => m_GenericMsg.MsgClass; }

        public LoginRTTFlags Flags { get; set; }

        /// <summary>
        /// Determines the presence of information about the number of TCP retransmissions.
        /// </summary>
        public bool HasTCPRetrans
        {
            get => (Flags & LoginRTTFlags.HAS_TCP_RETRANS) != 0;
            set
            {
                if (value)
                    Flags |= LoginRTTFlags.HAS_TCP_RETRANS;
                else
                    Flags &= ~LoginRTTFlags.HAS_TCP_RETRANS;
            }
        }
        /// <summary>
        /// Determines the presence of RTLatency in the message.
        /// </summary>
        public bool HasRTLatency
        {
            get => (Flags & LoginRTTFlags.ROUND_TRIP_LATENCY) != 0;
            set
            {
                if (value)
                    Flags |= LoginRTTFlags.ROUND_TRIP_LATENCY;
                else
                    Flags &= ~LoginRTTFlags.ROUND_TRIP_LATENCY;
            }
        }
        /// <summary>
        /// Determines whether the message exchange is driven by the provider.
        /// </summary>
        public bool ProviderDriven
        {
            get => (Flags & LoginRTTFlags.PROVIDER_DRIVEN) != 0;
            set
            {
                if (value)
                    Flags |= LoginRTTFlags.PROVIDER_DRIVEN;
                else
                    Flags &= ~LoginRTTFlags.PROVIDER_DRIVEN;
            }
        }

        /// <summary>
        /// Round-trip Latency value.
        /// </summary>
        public long RTLatency { get; set; }

        /// <summary>
        /// Number of TCP retransmissions.
        /// </summary>
        public long TCPRetrans { get; set; }

        /// <summary>
        /// Ticks value.
        /// </summary>
        public long Ticks { get; set; }

        #endregion

        public LoginRTT()
        {
            Clear();
        }

        public void InitRTT(int streamId)
        {
            Clear();
            StreamId = streamId;
            ProviderDriven = true;
            // A single tick represents one hundred nanoseconds
            Ticks = System.DateTime.Now.Ticks * 100;
        }

        public override void Clear()
        {
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.LOGIN;
            m_GenericMsg.ContainerType = DataTypes.ELEMENT_LIST;
            Flags = default;
        }

        public long CalculateRTTLatency()
        {
            RTLatency = System.DateTime.Now.Ticks * 100 - Ticks;
            return RTLatency;
        }

        public long CalculateRTTLatency(TimeUnit timeUnit)
        {
            long nanosRTTLatency = CalculateRTTLatency();
            if (timeUnit != TimeUnit.NANO_SECONDS)
            {
                return (nanosRTTLatency * 1000);
            }

            return nanosRTTLatency;
        }

        public override CodecReturnCode Decode(DecodeIterator decIter, Msg msg)
        {
            if (msg.MsgClass != MsgClasses.GENERIC || msg.DomainType != (int)Refinitiv.Eta.Rdm.DomainType.LOGIN)
                return CodecReturnCode.FAILURE;

            if (msg.ContainerType != DataTypes.ELEMENT_LIST)
                return CodecReturnCode.FAILURE;

            Clear();
            StreamId = msg.StreamId;
            if ((msg.Flags & GenericMsgFlags.PROVIDER_DRIVEN) != 0)
            {
                ProviderDriven = true;
            }

            elementList.Clear();
            CodecReturnCode ret = elementList.Decode(decIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            elementEntry.Clear();
            while ((ret = elementEntry.Decode(decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {

                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                if (elementEntry.Name.Equals(ElementNames.TICKS))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;

                    ret = tmpUInt.Decode(decIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    Ticks = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.ROUND_TRIP_LATENCY))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;

                    ret = tmpUInt.Decode(decIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasRTLatency = true;
                    RTLatency = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.TCP_RETRANS))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;

                    ret = tmpUInt.Decode(decIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasTCPRetrans = true;
                    TCPRetrans = tmpUInt.ToLong();
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.LOGIN;
            m_GenericMsg.StreamId = StreamId;
            m_GenericMsg.ContainerType = DataTypes.ELEMENT_LIST;

            if (ProviderDriven)
            {
                m_GenericMsg.Flags |= GenericMsgFlags.PROVIDER_DRIVEN;
            }

            CodecReturnCode ret = m_GenericMsg.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
                return CodecReturnCode.FAILURE;

            elementEntry.Clear();
            elementList.Clear();
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            if (elementList.EncodeInit(encIter, null, 0) < CodecReturnCode.SUCCESS)
            {
                return CodecReturnCode.FAILURE;
            }

            elementEntry.DataType = DataTypes.UINT;
            elementEntry.Name = ElementNames.TICKS;
            tmpUInt.Value(Ticks);

            if ((ret = elementEntry.Encode(encIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                return ret;

            if (HasRTLatency)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.ROUND_TRIP_LATENCY;
                tmpUInt.Value(RTLatency);

                if ((ret = elementEntry.Encode(encIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasTCPRetrans)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.TCP_RETRANS;
                tmpUInt.Value(TCPRetrans);

                if ((ret = elementEntry.Encode(encIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if ((ret = elementList.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
                return ret;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>Performs a deep copy of this object into <c>destRTTMsg</c>.
        public CodecReturnCode Copy(LoginRTT destRTTMsg)
        {
            Debug.Assert(destRTTMsg != null);
            destRTTMsg.StreamId = StreamId;
            destRTTMsg.Ticks = Ticks;

            if (HasTCPRetrans)
            {
                destRTTMsg.HasTCPRetrans = true;
                destRTTMsg.TCPRetrans = TCPRetrans;
            }

            if (HasRTLatency)
            {
                destRTTMsg.HasRTLatency = true;
                destRTTMsg.RTLatency = RTLatency;
            }

            if (ProviderDriven)
            {
                destRTTMsg.ProviderDriven = true;
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder toStringBuilder = PrepareStringBuilder();
            toStringBuilder.Insert(0, "LoginRTT: \n");

            toStringBuilder.Append(tab);
            toStringBuilder.Append("Ticks: ");
            toStringBuilder.Append(Ticks);

            if (HasRTLatency)
            {
                toStringBuilder.Append(tab);
                toStringBuilder.Append("rtLatency: ");
                toStringBuilder.Append(RTLatency);
            }
            if (HasTCPRetrans)
            {
                toStringBuilder.Append(tab);
                toStringBuilder.Append("tcpRetrans: ");
                toStringBuilder.Append(TCPRetrans);
            }
            toStringBuilder.Append(tab);
            toStringBuilder.Append("provider driven: ");
            toStringBuilder.Append(ProviderDriven);

            return toStringBuilder.ToString();
        }

        public void UpdateRTTActualTicks()
        {
            // A single tick represents one hundred nanoseconds
            Ticks = System.DateTime.Now.Ticks * 100;
        }
    }
}
