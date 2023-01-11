/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;

namespace LSEG.Eta.ValueAdd.Rdm
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

        private static readonly double TicksPerNanosecond = Stopwatch.Frequency / 1_000_000_000.0;


        #endregion
        #region Public Message Properties

        /// <summary>
        /// Enumeration for the time value used in <see cref="CalculateRTTLatency(TimeUnit)"/>
        /// </summary>
        public enum TimeUnit
        {
            /// <summary><see cref="CalculateRTTLatency(TimeUnit)"/> returns time in microseconds </summary>
            MICRO_SECONDS,
            /// <summary><see cref="CalculateRTTLatency(TimeUnit)"/> returns time in nanoseconds </summary>
            NANO_SECONDS
        };

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get; set; }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.LOGIN"/>.
        /// </summary>
        public override int DomainType { get => m_GenericMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.GENERIC"/>
        /// </summary>
        public override int MsgClass { get => m_GenericMsg.MsgClass; }

        /// <summary>
        /// Flags for this message.  See <see cref="LoginAttribFlags"/>.
        /// </summary>
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

        /// <summary>
        /// Login RTT Message constructor.
        /// </summary>
        public LoginRTT()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the login RTT object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.LOGIN;
            m_GenericMsg.ContainerType = DataTypes.ELEMENT_LIST;
            Flags = default;
        }

        /// <summary>
        /// Initializes a LoginRTT with default information, clearing it and filling in
        /// the current system tick time.
        /// </summary>
        /// <param name="streamId">Stream ID to be used for this login Request.</param>
        public void InitDefaultRTT(int streamId)
        {
            Clear();
            StreamId = streamId;
            ProviderDriven = true;
            // A single system tick represents one hundred nanoseconds
            Ticks = (long)(Stopwatch.GetTimestamp() / TicksPerNanosecond);
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destRTTMsg</c>.
        /// </summary>
        /// <param name="destRTTMsg">LoginRTT object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
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

        /// <summary>
        /// Encodes this login RTT message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.LOGIN;
            m_GenericMsg.StreamId = StreamId;
            m_GenericMsg.ContainerType = DataTypes.ELEMENT_LIST;

            if (ProviderDriven)
            {
                m_GenericMsg.Flags |= GenericMsgFlags.PROVIDER_DRIVEN;
            }

            CodecReturnCode ret = m_GenericMsg.EncodeInit(encodeIter, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
                return CodecReturnCode.FAILURE;

            elementEntry.Clear();
            elementList.Clear();
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            if (elementList.EncodeInit(encodeIter, null, 0) < CodecReturnCode.SUCCESS)
            {
                return CodecReturnCode.FAILURE;
            }

            elementEntry.DataType = DataTypes.UINT;
            elementEntry.Name = ElementNames.TICKS;
            tmpUInt.Value(Ticks);

            if ((ret = elementEntry.Encode(encodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                return ret;

            if (HasRTLatency)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.ROUND_TRIP_LATENCY;
                tmpUInt.Value(RTLatency);

                if ((ret = elementEntry.Encode(encodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasTCPRetrans)
            {
                elementEntry.DataType = DataTypes.UINT;
                elementEntry.Name = ElementNames.TCP_RETRANS;
                tmpUInt.Value(TCPRetrans);

                if ((ret = elementEntry.Encode(encodeIter, tmpUInt)) != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if ((ret = elementList.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                return ret;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Decodes this Login RTT message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this LoginRTT message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            if (msg.MsgClass != MsgClasses.GENERIC || msg.DomainType != (int)LSEG.Eta.Rdm.DomainType.LOGIN)
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
            CodecReturnCode ret = elementList.Decode(decodeIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            elementEntry.Clear();
            while ((ret = elementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {

                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                if (elementEntry.Name.Equals(ElementNames.TICKS))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;

                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    Ticks = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.ROUND_TRIP_LATENCY))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;

                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasRTLatency = true;
                    RTLatency = tmpUInt.ToLong();
                }
                else if (elementEntry.Name.Equals(ElementNames.TCP_RETRANS))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return CodecReturnCode.FAILURE;

                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                    HasTCPRetrans = true;
                    TCPRetrans = tmpUInt.ToLong();
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Login RTT message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
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

        /// <summary>
        /// Returns the time difference in ticks between the current time and the RTT Message's Ticks count.        
        /// </summary>
        /// <returns>The time difference.</returns>
        public long CalculateRTTLatency()
        {
            long nowNs = (long)((double)Stopwatch.GetTimestamp() / TicksPerNanosecond);
            RTLatency = nowNs - Ticks;
            return RTLatency;
        }

        /// <summary>
        /// Returns the latency difference in TimeUnit.
        /// </summary>
        /// <param name="timeUnit">The <see cref="TimeUnit"/> to calculate latency</param>
        /// <returns>The time difference.</returns>
        public long CalculateRTTLatency(TimeUnit timeUnit)
        {
            long nanosRTTLatency = CalculateRTTLatency();
            if (timeUnit != TimeUnit.NANO_SECONDS)
            {
                return (nanosRTTLatency * 1000);
            }

            return nanosRTTLatency;
        }

        /// <summary>
        /// Updates this RTT Message's current Ticks timestamp.     
        /// </summary>
        public void UpdateRTTActualTicks()
        {
            // A single system tick represents one hundred nanoseconds
            Ticks = (long)(Stopwatch.GetTimestamp() / TicksPerNanosecond);
        }
    }
}
