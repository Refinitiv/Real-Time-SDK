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
    /// RequestMsg allows consumer application to express its interest in an item.
    /// 
    /// <p>Among other attributes, ReqMsg conveys item's name, service, domain type, and
    /// desired quality of service.</p>
    /// 
    /// <p>RequestMsg may also be used to:
    /// specify application interest in a dynamic view, 
    /// open a batch of items,
    /// or request a symbol list item with its data.</p>
    /// 
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform setting and getting of information from RequestMsg.
    /// Objects of this class are not cache-able.
    /// Decoding of just encoded ReqMsg in the same application is not supported.
    /// </summary>
    public sealed class RequestMsg : Msg, ICloneable
    {
        /// <summary>
        /// Class that represents Qos rates
        /// </summary>
        public static class Rate
        {
            /**
             * Rate is Tick By Tick, indicates every change to information is conveyed
             */
            public const uint TICK_BY_TICK = 0;

            /**
             * Rate is Just In Time Conflated, indicates extreme bursts of data that may be conflated.
             */
            public const uint JIT_CONFLATED = 0xFFFFFF00;

            /**
             * Request Rate with range from 1 millisecond conflation to maximum conflation.
             */
            public const uint BEST_CONFLATED_RATE = 0xFFFFFFFF;

            /**
             * Request Rate with range from tick-by-tick to maximum conflation.
             */
            public const uint BEST_RATE = 0xFFFFFFFE;
        }

        /// <summary>
        /// Timeliness represents the Qos Timeliness.
        /// </summary>
        public static class Timeliness
        {
            /**
             * Timeliness is REALTIME,
             * indicates information is updated as soon as new information becomes available
             */
            public const int REALTIME = 0;

            /**
             * Request Timeliness with range from one second delay to maximum delay.
             */
            public const uint BEST_DELAYED_TIMELINESS = 0xFFFFFFFF;

            /**
             * Request Timeliness with range from real-time to maximum delay.
             */
            public const uint BEST_TIMELINESS = 0xFFFFFFFE;
        }

        private const string TICKBYTICK_NAME = "TickByTick";
        private const string JUSTINTIMECONFLATEDRATE_NAME = "JustInTimeConflatedrate";
        private const string BESTCONFLATEDRATE_NAME = "BestConflatedRate";
        private const string BESTRATE_NAME = "BestRate";
        private const string UNKNOWNREQMSGQOSRATE_NAME = "Rate on ReqMsg. Value = ";
        private const string REALTIME_NAME = "RealTime";
        private const string BESTDELAYEDTIMELINESS_NAME = "BestDelayedTimeliness";
        private const string BESTTIMELINESS_NAME = "BestTimeliness";
        private const string UNKNOWNREQMSGQOSTIMELINESS_NAME = "QosTimeliness on ReqMsg. Value = ";
        private const string VIEW_DATA_STRING = ":ViewData";
        private const string ITEM_LIST_STRING = ":ItemList";

        private OmmQos m_qos = new OmmQos();
        private bool m_qosSet = false;

        internal RequestMsgEncoder m_requestMsgEncoder;

        /// <summary>
        /// Constructor for RequestMsg
        /// </summary>
        public RequestMsg()
        {
            m_msgClass = MsgClasses.REQUEST;
            m_rsslMsg.MsgClass = MsgClasses.REQUEST;
            m_requestMsgEncoder = new RequestMsgEncoder(this);
            Encoder = m_requestMsgEncoder;

            InterestAfterRefresh(true);
            DomainType((int)Eta.Rdm.DomainType.MARKET_PRICE);
        }

        /// <summary>
        /// Copy constructor for <see cref="RequestMsg"/>
        /// </summary>
        /// <param name="source"><see cref="RequestMsg"/> to create current message from.</param>
        public RequestMsg(RequestMsg source)
        {
            m_msgClass = MsgClasses.REQUEST;
            m_requestMsgEncoder = new RequestMsgEncoder(this);
            Encoder = m_requestMsgEncoder;
            source.CopyMsg(this);
        }

        internal RequestMsg(EmaObjectManager manager) : base(manager)
        {
            m_rsslMsg.MsgClass = MsgClasses.REQUEST;
            m_msgClass = MsgClasses.REQUEST;
            m_requestMsgEncoder = new RequestMsgEncoder(this);
            Encoder = m_requestMsgEncoder;

            InterestAfterRefresh(true);
            DomainType((int)Eta.Rdm.DomainType.MARKET_PRICE);
        }

        /// <summary>
        /// Determines whether MsgKey of the message holds the name
        /// </summary>
        public override bool HasName { get => m_rsslMsg.MsgKey.CheckHasName(); }

        /// <summary>
        /// Determines whether the message has message key
        /// </summary>
        public override bool HasMsgKey { get => m_rsslMsg.MsgKey != null; }

        /// <summary>
        /// Determines whether MsgKey has NameType property
        /// </summary>
        public override bool HasNameType { get => m_rsslMsg.MsgKey.CheckHasNameType(); }

        /// <summary>
        /// Determines whether the message key has serviceId present
        /// </summary>
        public override bool HasServiceId { get => m_rsslMsg.MsgKey.CheckHasServiceId(); }

        /// <summary>
        /// Determines whether the message has identifier
        /// </summary>
        public override bool HasId { get => m_rsslMsg.MsgKey.CheckHasIdentifier(); }

        /// <summary>
        /// Determines whether the message has filter
        /// </summary>
        public override bool HasFilter { get => m_rsslMsg.MsgKey.CheckHasFilter(); }

        /// <summary>
        /// Checks whether the message contains MsgKey attributes
        /// </summary>
        public override bool HasAttrib { get => m_rsslMsg.MsgKey.CheckHasAttrib(); }

        /// <summary>
        /// Indicates presence of Priority.
        /// Priority is an optional member of RequestMsg.
        /// </summary>
        public bool HasPriority { get => m_rsslMsg.CheckHasPriority(); }
        
        /// <summary>
        /// Indicates presence of Qos.
        /// Qos is an optional member of RequestMsg.
        /// </summary>
        public bool HasQos { get => m_rsslMsg.CheckHasQos(); }
        
        /// <summary>
        /// Indicates presence of View.
        /// View specification is an optional member of RequestMsg.
        /// </summary>
        public bool HasView { get => m_rsslMsg.CheckHasView(); }
        
        /// <summary>
        /// Indicates presence of Batch.
        /// Batch specification is an optional member of RequestMsg.
        /// </summary>
        public bool HasBatch { get => m_rsslMsg.CheckHasBatch(); }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.REQ_MSG;

        /// <summary>
        /// Returns PriorityClass.
        /// Calling this method must be preceded by a call to <see cref="HasPriority"/>.
        /// </summary>
        /// <returns>priority class</returns>
        public int PriorityClass() 
        { 
            if (!m_rsslMsg.CheckHasPriority())
            {
                throw new OmmInvalidUsageException("Attempt to call PriorityClass() while it is NOT set.");
            }
            return m_rsslMsg.Priority.PriorityClass; 
        }
        
        /// <summary>
        /// Returns PriorityCount.
        /// Calling this method must be preceded by a call to <see cref="HasPriority"/>.
        /// </summary>
        /// <returns>priority count</returns>
        public int PriorityCount()
        {
            if (!m_rsslMsg.CheckHasPriority())
            {
                throw new OmmInvalidUsageException("Attempt to call PriorityCount() while it is NOT set.");
            }
            return m_rsslMsg.Priority.Count;
        }
        
        /// <summary>
        /// Returns RequestMsg's Qos.
        /// Calling this method must be preceded by a call to <see cref="HasQos"/>.
        /// </summary>
        /// <returns><see cref="OmmQos"/> instance</returns>
        public OmmQos Qos() 
        {
            if (!m_rsslMsg.CheckHasQos())
            {
                throw new OmmInvalidUsageException("Invalid attempt to call Qos() while it is not set.");
            }

            if (!m_qosSet)
            {
                m_qos.ClearInt();
                m_qos.Decode(m_rsslMsg.Qos);
                m_qosSet = true;
            }

            return m_qos;
        }

        /// <summary>
        /// Returns InitialImage.
        /// </summary>
        /// <returns>true if an initial image is requested; false otherwise</returns>
        public bool InitialImage() 
        {
            return m_rsslMsg.CheckNoRefresh(); 
        }
        
        /// <summary>
        /// Returns InterestAfterRefresh.
        /// </summary>
        /// <returns>true if an interest after refresh is requested; false otherwise</returns>
        public bool InterestAfterRefresh() 
        { 
            return m_rsslMsg.CheckStreaming(); 
        }
        
        /// <summary>
        /// Returns ConflatedInUpdates.
        /// </summary>
        /// <returns>true if conflation is requested; false otherwise</returns>
        public bool ConflatedInUpdates() 
        { 
            return m_rsslMsg.CheckConfInfoInUpdates(); 
        }
        
        /// <summary>
        /// Returns Pause.
        /// </summary>
        /// <returns>true if pause is requested; false otherwise</returns>
        public bool Pause() 
        {
            return m_rsslMsg.CheckPause();
        }
        
        /// <summary>
        /// Returns PrivateStream.
        /// </summary>
        /// <returns>true if private stream is requested; false otherwise</returns>
        public bool PrivateStream() 
        { 
            return m_rsslMsg.CheckPrivateStream(); 
        }
        
        /// <summary>
        /// Specifies StreamId
        /// </summary>
        public RequestMsg StreamId(int streamId)
        {
            m_requestMsgEncoder.StreamId(streamId);
            return this;
        }

        /// <summary>
        /// Specifies DomainType
        /// </summary>
        public RequestMsg DomainType(int domainType)
        {
            m_requestMsgEncoder.DomainType(domainType);
            return this;
        }
        
        /// <summary>
        /// Specifies Name
        /// </summary>
        /// <param name="name">specifies item name</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Name(string name)
        {
            m_requestMsgEncoder.Name(name);
            return this;
        }
        
        /// <summary>
        /// Specifies Name type
        /// </summary>
        /// <param name="nameType">specifies RDM Instrument NameType</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg NameType(int nameType)
        {
            m_requestMsgEncoder.NameType(nameType);
            return this;
        }
        
        /// <summary>
        /// Specifies ServiceName.
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceName">specifies service name</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg ServiceName(string serviceName)
        {
            SetMsgServiceName(serviceName);
            return this;
        }
        
        /// <summary>
        /// Specifies ServiceId.
        /// One service identification must be set, either id or name.
        /// </summary>
        /// <param name="serviceId">specifies service id</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg ServiceId(int serviceId)
        {
            m_requestMsgEncoder.ServiceId(serviceId);
            return this;
        }

        /// <summary>
        /// Specifies Id.
        /// </summary>
        /// <param name="id">the id to be set</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Id(int id)
        {
            m_requestMsgEncoder.Identifier(id);
            return this;
        }
        
        /// <summary>
        /// Specifies Filter.
        /// </summary>
        /// <param name="filter">filter value to be set</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Filter(long filter)
        {
            m_requestMsgEncoder.Filter(filter);
            return this;
        }

        /// <summary>
        /// Specifies Priority.
        /// </summary>
        /// <param name="priorityClass">specifies priority class</param>
        /// <param name="priorityCount">specifies priority count within priority class</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Priority(int priorityClass, int priorityCount) 
        {
            m_requestMsgEncoder.Priority(priorityClass, priorityCount);
            return this;
        }
        
        /// <summary>
        /// Specifies Qos as timeliness and rate.
        /// </summary>
        /// <param name="timeliness">specifies Qos Timeliness </param>
        /// <param name="rate">specifies Qos rate</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Qos(uint timeliness, uint rate) 
        {
            m_requestMsgEncoder.Qos(timeliness, rate);
            return this; 
        }

        /// <summary>
        /// Specifies Attrib.
        /// </summary>
        /// <param name="data">an object of IComplexType type that contains attribute information</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Attrib(ComplexType data) 
        {
            m_attrib.SetExternalData(data);
            m_requestMsgEncoder.Attrib(data);
            return this; 
        }
        
        /// <summary>
        /// Specifies Payload.
        /// </summary>
        /// <param name="payload">payload to be set</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Payload(ComplexType payload) 
        {
            m_payload.SetExternalData(payload);
            m_requestMsgEncoder.RequestMsgPayload(payload);
            return this; 
        }
        
        /// <summary>
        /// Specifies ExtendedHeader.
        /// </summary>
        /// <param name="buffer">a EmaBuffer containing extendedHeader information</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg ExtendedHeader(EmaBuffer buffer) 
        { 
            m_requestMsgEncoder.ExtendedHeader(buffer);
            return this; 
        }
        
        /// <summary>
        /// Specifies InitialImage.
        /// </summary>
        /// <param name="initialImage">specifies if initial image / refresh is requested</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg InitialImage(bool initialImage) 
        {
            m_requestMsgEncoder.InitialImage(initialImage);
            return this; 
        }
        
        /// <summary>
        /// Specifies InterestAfterRefresh.
        /// </summary>
        /// <param name="interestAfterRefresh">specifies if streaming or snapshot item is requested</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg InterestAfterRefresh(bool interestAfterRefresh)
        {
            m_requestMsgEncoder.InterestAfterRefresh(interestAfterRefresh);
            return this;
        }

        /// <summary>
        /// Specifies Pause.
        /// </summary>
        /// <param name="pause">specifies if pause is requested</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Pause(bool pause)
        {
            m_requestMsgEncoder.Pause(pause);
            return this;
        }

        /// <summary>
        /// Specifies ConflatedInUpdates.
        /// </summary>
        /// <param name="conflatedInUpdates">specifies if conflated update is requested</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg ConflatedInUpdates(bool conflatedInUpdates)
        {
            m_requestMsgEncoder.ConflatedInUpdates(conflatedInUpdates);
            return this;
        }

        /// <summary>
        /// Specifies PrivateStream.
        /// </summary>
        /// <param name="privateStream">specifies if private stream is requested</param>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg PrivateStream(bool privateStream)
        {
            m_requestMsgEncoder.PrivateStream(privateStream);
            return this;
        }

        /// <summary>
        /// Clears the RequestMsg.
        /// Invoking ClearMsg() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>reference to current <see cref="RequestMsg"/> instance</returns>
        public RequestMsg Clear()
        {
            ClearInt();
            return this;
        }

        /// <summary>
        /// Clears current RequestMsg instance.
        /// </summary>
        internal override void ClearInt()
        {
            base.ClearInt();
            m_qos.ClearInt();
            m_qosSet = false;

            InterestAfterRefresh(true);
            DomainType((int)Eta.Rdm.DomainType.MARKET_PRICE);
        }

        /// <summary>
        /// Returns QosRate.
        /// Calling this method must be preceded by a call to <see cref="HasQos"/>
        /// </summary>
        /// <returns>Qos Rate</returns>
        /// <exception cref="OmmInvalidUsageException"> if <see cref="HasQos"/> returns false.</exception>
        public uint QosRate()
        {
            if (!HasQos) 
                throw new OmmInvalidUsageException("Attempt to QosRate while it is NOT set.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            uint rate = RequestMsg.Rate.BEST_RATE;
            int reqRate = m_rsslMsg.Qos.Rate();

            if (m_rsslMsg.CheckHasWorstQos())
            {
                int wReqRate = m_rsslMsg.WorstQos.Rate();

                if (reqRate == wReqRate)
                {
                    switch (reqRate)
                    {
                        case QosRates.TICK_BY_TICK:
                            rate = Rate.TICK_BY_TICK;
                            break;
                        case QosRates.JIT_CONFLATED:
                            rate = Rate.JIT_CONFLATED;
                            break;
                        case QosRates.TIME_CONFLATED:
                            if (m_rsslMsg.Qos.RateInfo() == m_rsslMsg.WorstQos.RateInfo())
                                rate = (uint)m_rsslMsg.Qos.RateInfo();
                            else
                                rate = Rate.BEST_CONFLATED_RATE;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    if (reqRate == QosRates.TICK_BY_TICK)
                        rate = Rate.BEST_RATE;
                    else
                        rate = Rate.BEST_CONFLATED_RATE;
                }
            }
            else
            {
                switch (reqRate)
                {
                    case QosRates.TICK_BY_TICK:
                        rate = Rate.TICK_BY_TICK;
                        break;
                    case QosRates.JIT_CONFLATED:
                        rate = Rate.JIT_CONFLATED;
                        break;
                    case QosRates.TIME_CONFLATED:
                        rate = (uint)m_rsslMsg.Qos.RateInfo();
                        break;
                    default:
                        break;
                }
            }

            return rate;
        }

        /// <summary>
        /// Returns QosTimeliness.
        /// Calling this method must be preceded by a call to <see cref="HasQos"/>
        /// </summary>
        /// <returns>Qos Timeliness</returns>
        /// <exception cref="OmmInvalidUsageException"> if <see cref="HasQos"/> returns false.</exception>
        public uint QosTimeliness()
        {
            if (!HasQos)
                throw new OmmInvalidUsageException("Attempt to QosTimeliness while it is NOT set.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            uint timeliness = RequestMsg.Timeliness.BEST_TIMELINESS;
            int reqTimeliness = m_rsslMsg.Qos.Timeliness();

            if (m_rsslMsg.CheckHasWorstQos())
            {
                int wReqTimeliness = m_rsslMsg.WorstQos.Timeliness();

                if (reqTimeliness == wReqTimeliness)
                {
                    switch (reqTimeliness)
                    {
                        case Eta.Codec.QosTimeliness.REALTIME:
                            timeliness = Timeliness.REALTIME;
                            break;
                        case Eta.Codec.QosTimeliness.DELAYED_UNKNOWN:
                            timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
                            break;
                        case Eta.Codec.QosTimeliness.DELAYED:
                            if (m_rsslMsg.Qos.TimeInfo() == m_rsslMsg.WorstQos.TimeInfo())
                                timeliness = (uint)m_rsslMsg.Qos.TimeInfo();
                            else
                                timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    if (reqTimeliness == Eta.Codec.QosTimeliness.REALTIME)
                        timeliness = Timeliness.BEST_TIMELINESS;
                    else
                        timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
                }
            }
            else
            {
                switch (reqTimeliness)
                {
                    case Eta.Codec.QosTimeliness.REALTIME:
                        timeliness = Timeliness.REALTIME;
                        break;
                    case Eta.Codec.QosTimeliness.DELAYED_UNKNOWN:
                        timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
                        break;
                    case Eta.Codec.QosTimeliness.DELAYED:
                        timeliness = (uint)m_rsslMsg.Qos.TimeInfo();
                        break;
                    default:
                        break;
                }
            }

            return timeliness;
        }

        /// <summary>
        /// Returns the Rate value as a string format.
        /// </summary>
        /// <returns>string representation of Qos Rate</returns>
        public string RateAsString()
        {
            switch (QosRate())
            {
                case Rate.TICK_BY_TICK:
                    return TICKBYTICK_NAME;
                case Rate.JIT_CONFLATED:
                    return JUSTINTIMECONFLATEDRATE_NAME;
                case Rate.BEST_CONFLATED_RATE:
                    return BESTCONFLATEDRATE_NAME;
                case Rate.BEST_RATE:
                    return BESTRATE_NAME;
                default:
                    return UNKNOWNREQMSGQOSRATE_NAME + QosRate();
            }
        }

        /// <summary>
        /// Returns the Timeliness value as a string format.
        /// </summary>
        /// <returns>string representation of Qos Timeliness</returns>
        public string TimelinessAsString()
        {
            switch (QosTimeliness())
            {
                case Timeliness.REALTIME:
                    return REALTIME_NAME;
                case Timeliness.BEST_DELAYED_TIMELINESS:
                    return BESTDELAYEDTIMELINESS_NAME;
                case Timeliness.BEST_TIMELINESS:
                    return BESTTIMELINESS_NAME;
                default:
                    return UNKNOWNREQMSGQOSTIMELINESS_NAME + QosTimeliness();
            }
        }

        /// <summary>
        /// Provides string representation of the current RequestMsg instance
        /// </summary>
        /// <returns>string representing current <see cref="RequestMsg"/> instance.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        /// <summary>
        /// Creates object that is a copy of the current object.
        /// </summary>
        /// <returns><see cref="RequestMsg"/> instance that is a copy of the current RequestMsg.</returns>
        public RequestMsg Clone()
        {
            var copy = new RequestMsg();
            CopyMsg(copy);
            return copy;
        }

        object ICloneable.Clone()
        {
            return Clone();
        }

        /// <summary>
        /// Completes the encoding of Request message
        /// </summary>
        internal override void EncodeComplete()
        {
            m_requestMsgEncoder.EncodeComplete();
        }

        internal override string ToString(int indent)
        {
            if (m_objectManager == null)
                return "\nDecoding of just encoded object in the same application is not supported\n";

            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent++).Append("ReqMsg");
            Utilities.AddIndent(m_ToString, indent, true).Append("streamId=\"")
                                                         .Append(StreamId())
                                                         .Append("\"");
            Utilities.AddIndent(m_ToString, indent, true).Append("domain=\"")
                                                         .Append(Utilities.RdmDomainAsString(DomainType()))
                                                         .Append("\"");

            if (PrivateStream())
                Utilities.AddIndent(m_ToString, indent, true).Append("privateStream");

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

            Utilities.AddIndent(m_ToString, indent, true).Append("PayloadEnd");
            indent--;

            Utilities.AddIndent(m_ToString, indent, true).Append("ReqMsgEnd\n");

            return m_ToString.ToString();
        }
    }
}
