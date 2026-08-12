using LSEG.Ema.Access;
using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Service Info. Contains information provided by the Source Directory Info filter.
    /// </summary>
    public sealed class DirectoryServiceInfo : DirectoryServiceFilter<ElementList, DirectoryServiceInfo>, ICloneable
    {
        private OptionalField<string> m_Vendor = new("Vendor element");
        private OptionalField<bool> m_IsSource = new("IsSource element");
        private OptionalField<bool> m_SupportsQosRange = new("SupportsQosRange element");
        private OptionalField<bool> m_SupportsOutOfBandSnapshots = new("SupportsOutOfBandSnapshots element");
        private OptionalField<bool> m_AcceptingConsumerStatus = new("AcceptingConsumerStatus element");
        private OptionalField<string> m_ItemList = new("ItemList element");
        private readonly List<ulong> m_CapabilitiesList = new();
        private readonly FluentListBuilder<ulong> m_CapabilitiesListBuilder;
        private readonly List<string> m_DictionariesProvidedList = new();
        private readonly FluentListBuilder<string> m_DictionariesProvidedListBuilder;
        private readonly List<string> m_DictionariesUsedList = new();
        private readonly FluentListBuilder<string> m_DictionariesUsedListBuilder;
        private readonly List<OmmQos> m_OmmQosList = new();
        private readonly List<DirectoryQos> m_QosList = new();
        private readonly FluentListBuilder<DirectoryQos> m_QosListBuilder;
        private string m_ServiceName = "";

        /// <summary>
        /// Initializes a new instance of the <see cref="DirectoryServiceInfo"/> class.
        /// </summary>
        public DirectoryServiceInfo()
        {
            m_CapabilitiesListBuilder = new(m_CapabilitiesList);
            m_DictionariesProvidedListBuilder = new(m_DictionariesProvidedList);
            m_DictionariesUsedListBuilder = new(m_DictionariesUsedList);
            m_QosListBuilder = new(m_QosList);
        }

        /// <inheritdoc />
        public override int FilterId => EmaRdm.SERVICE_INFO_ID;

        /// <summary>
        /// Service name that identifies this service.
        /// </summary>
        public string ServiceName() => m_ServiceName;
        /// <summary>
        /// Service name that identifies this service.
        /// </summary>
        public DirectoryServiceInfo ServiceName(string value)
        {
            m_ServiceName = value;
            return this;
        }

        /// <summary>
        /// Vendor name of data provided by this service.
        /// </summary>
        public string Vendor() => m_Vendor.Value;
        /// <summary>
        /// Vendor name of data provided by this service.
        /// </summary>
        public DirectoryServiceInfo Vendor(string value)
        {
            m_Vendor.Value = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of the vendor field.
        /// </summary>
        public bool HasVendor => m_Vendor.HasValue;

        /// <summary>
        /// Flag that indicates whether the service is provided directly by a
        /// publisher or consolidated from multiple sources.
        /// </summary>
        public bool IsSource() => m_IsSource.Value;
        /// <summary>
        /// Flag that indicates whether the service is provided directly by a
        /// publisher or consolidated from multiple sources.
        /// </summary>
        public DirectoryServiceInfo IsSource(bool value)
        {
            m_IsSource.Value = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of the isSource field.
        /// </summary>
        public bool HasIsSource => m_IsSource.HasValue;

        /// <summary>
        /// Flag that indicates whether items can be requested using a
        /// QoS range(using both the qos and worstQos members of a <see cref="LSEG.Ema.Access.RequestMsg"/>).
        /// </summary>
        public bool SupportsQosRange() => m_SupportsQosRange.Value;
        /// <summary>
        /// Flag that indicates whether items can be requested using a
        /// QoS range(using both the qos and worstQos members of a <see cref="LSEG.Ema.Access.RequestMsg"/>).
        /// </summary>
        public DirectoryServiceInfo SupportsQosRange(bool value)
        {
            m_SupportsQosRange.Value = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of the supportsQosRange field.
        /// </summary>
        public bool HasSupportsQosRange => m_SupportsQosRange.HasValue;

        /// <summary>
        /// Flag that indicates whether Snapshot(requests without the STREAMING flag) can be made when the OpenLimit is reached.
        /// </summary>
        public bool SupportsOutOfBandSnapshots() => m_SupportsOutOfBandSnapshots.Value;
        /// <summary>
        /// Flag that indicates whether Snapshot(requests without the STREAMING flag) can be made when the OpenLimit is reached.
        /// </summary>
        public DirectoryServiceInfo SupportsOutOfBandSnapshots(bool value)
        {
            m_SupportsOutOfBandSnapshots.Value = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of the supportsOutOfBandSnapshots field.
        /// </summary>
        public bool HasSupportsOutOfBandSnapshots => m_SupportsOutOfBandSnapshots.HasValue;

        /// <summary>
        /// Flag that indicates whether the service accepts messages related to Source Mirroring.
        /// </summary>
        public bool AcceptingConsumerStatus() => m_AcceptingConsumerStatus.Value;
        /// <summary>
        /// Flag that indicates whether the service accepts messages related to Source Mirroring.
        /// </summary>
        public DirectoryServiceInfo AcceptingConsumerStatus(bool value)
        {
            m_AcceptingConsumerStatus.Value = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of the acceptingConsumerStatus field.
        /// </summary>
        public bool HasAcceptingConsumerStatus => m_AcceptingConsumerStatus.HasValue;

        /// <summary>
        /// List of item names a Consumer can request to get a symbol list
        /// of all item names available from this service.
        /// </summary>
        public string ItemList() => m_ItemList.Value;
        /// <summary>
        /// List of item names a Consumer can request to get a symbol list
        /// of all item names available from this service.
        /// </summary>
        public DirectoryServiceInfo ItemList(string value)
        {
            m_ItemList.Value = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of the itemList field.
        /// </summary>
        public bool HasItemList => m_ItemList.HasValue;

        /// <summary>
        /// List of capabilities the service supports. Capability in the
        /// list is populated by <see cref="LSEG.Ema.Rdm.EmaRdm"/>.
        /// </summary>
        public IList<ulong> CapabilitiesList() => m_CapabilitiesList;
        /// <summary>
        /// List of capabilities the service supports. Capability in the
        /// list is populated by <see cref="LSEG.Ema.Rdm.EmaRdm"/>.
        /// </summary>
        public DirectoryServiceInfo CapabilitiesList(IList<ulong> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("CapabilitiesList cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_CapabilitiesList.Clear();
            m_CapabilitiesList.AddRange(value);
            return this;
        }
        /// <summary>
        /// List of capabilities the service supports. Capability in the
        /// list is populated by <see cref="LSEG.Ema.Rdm.EmaRdm"/>.
        /// </summary>
        public DirectoryServiceInfo CapabilitiesList(Action<IFluentListBuilder<ulong>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            buildAction(m_CapabilitiesListBuilder);
            return this;
        }

        /// <summary>
        /// Dictionary names provided by this service.
        /// </summary>
        public IList<string> DictionariesProvidedList()
        {
            if (!HasDictionariesProvidedList)
                throw new OmmInvalidUsageException("DictionariesProvidedList element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_DictionariesProvidedList;
        }
        /// <summary>
        /// Dictionary names provided by this service.
        /// </summary>
        public DirectoryServiceInfo DictionariesProvidedList(IList<string> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("DictionariesProvidedList cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_DictionariesProvidedList.Clear();
            m_DictionariesProvidedList.AddRange(value);
            HasDictionariesProvidedList = true;
            return this;
        }
        /// <summary>
        /// Dictionary names provided by this service.
        /// </summary>
        public DirectoryServiceInfo DictionariesProvidedList(Action<IFluentListBuilder<string>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            buildAction(m_DictionariesProvidedListBuilder);
            HasDictionariesProvidedList = true;
            return this;
        }

        /// <summary>
        /// Gets the presence of the dictionariesProvided field.
        /// </summary>
        public bool HasDictionariesProvidedList { get; private set; }

        /// <summary>
        /// Dictionary names that a consumer will require to decode the
        /// service's market data content.
        /// </summary>
        public IList<string> DictionariesUsedList()
        {
            if (!HasDictionariesUsedList)
                throw new OmmInvalidUsageException("DictionariesUsedList element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_DictionariesUsedList;
        }
        /// <summary>
        /// Dictionary names that a consumer will require to decode the
        /// service's market data content.
        /// </summary>
        public DirectoryServiceInfo DictionariesUsedList(IList<string> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("DictionariesUsedList cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_DictionariesUsedList.Clear();
            m_DictionariesUsedList.AddRange(value);
            HasDictionariesUsedList = true;
            return this;
        }
        /// <summary>
        /// Dictionary names that a consumer will require to decode the
        /// service's market data content.
        /// </summary>
        public DirectoryServiceInfo DictionariesUsedList(Action<IFluentListBuilder<string>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            buildAction(m_DictionariesUsedListBuilder);
            HasDictionariesUsedList = true;
            return this;
        }

        /// <summary>
        /// Gets the presence of the dictionariesUsed field.
        /// </summary>
        public bool HasDictionariesUsedList { get; private set; }

        /// <summary>
        /// List of qualities of service that this service provides.
        /// </summary>
        public IList<OmmQos> QosList()
        {
            if (!HasQosList)
                throw new OmmInvalidUsageException("QosList element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            m_OmmQosList.Clear();
            m_OmmQosList.AddRange(m_QosList.Select(qos => qos.ToOmmQos()));
            return m_OmmQosList;
        }
        /// <summary>
        /// List of qualities of service that this service provides.
        /// </summary>
        public DirectoryServiceInfo QosList(IList<DirectoryQos> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("QosList cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_QosList.Clear();
            m_QosList.AddRange(value);
            HasQosList = true;
            return this;
        }
        /// <summary>
        /// List of qualities of service that this service provides.
        /// </summary>
        public DirectoryServiceInfo QosList(Action<IFluentListBuilder<DirectoryQos>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            buildAction(m_QosListBuilder);
            HasQosList = true;
            return this;
        }

        /// <summary>
        /// Gets the presence of the qosList field.
        /// </summary>
        public bool HasQosList { get; private set; }

        /// <inheritdoc />
        public override DirectoryServiceInfo Clear()
        {
            base.Clear();
            ServiceName("");
            m_Vendor.Clear();
            m_IsSource.Clear();
            m_SupportsQosRange.Clear();
            m_SupportsOutOfBandSnapshots.Clear();
            m_AcceptingConsumerStatus.Clear();
            m_ItemList.Clear();
            m_CapabilitiesList.Clear();
            m_DictionariesProvidedList.Clear();
            HasDictionariesProvidedList = false;
            m_DictionariesUsedList.Clear();
            HasDictionariesUsedList = false;
            m_OmmQosList.Clear();
            m_QosList.Clear();
            HasQosList = false;
            return this;
        }

        /// <inheritdoc />
        public override DirectoryServiceInfo CopyFrom(DirectoryServiceInfo source)
        {
            base.CopyFrom(source);
            m_ServiceName = source.m_ServiceName;
            m_Vendor.CopyFrom(source.m_Vendor);
            m_IsSource.CopyFrom(source.m_IsSource);
            m_SupportsQosRange.CopyFrom(source.m_SupportsQosRange);
            m_SupportsOutOfBandSnapshots.CopyFrom(source.m_SupportsOutOfBandSnapshots);
            m_AcceptingConsumerStatus.CopyFrom(source.m_AcceptingConsumerStatus);
            m_ItemList.CopyFrom(source.m_ItemList);

            m_CapabilitiesList.Clear();
            m_CapabilitiesList.AddRange(source.m_CapabilitiesList);

            m_DictionariesProvidedList.CopyFrom(source.m_DictionariesProvidedList);
            HasDictionariesProvidedList = source.HasDictionariesProvidedList;
            m_DictionariesUsedList.CopyFrom(source.m_DictionariesUsedList);
            HasDictionariesUsedList = source.HasDictionariesUsedList;
            
            m_QosList.Clear();
            foreach (var qos in source.m_QosList)
            {
                m_QosList.Add(qos.Clone());
            }

            HasQosList = source.HasQosList;
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryServiceInfo"/> instance that is a copy of this instance.</returns>
        public DirectoryServiceInfo Clone() => new DirectoryServiceInfo().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        internal override void AppendToString(StringBuilder builder, int indent)
        {
            base.AppendToString(builder, indent);
            builder.AddIndent(indent).AppendLine($"ServiceName: {m_ServiceName}");
            if (HasVendor)
            {
                builder.AddIndent(indent).AppendLine($"Vendor: {m_Vendor}");
            }
            if (HasIsSource)
            {
                builder.AddIndent(indent).AppendLine($"IsSource: {m_IsSource}");
            }
            if (HasSupportsQosRange)
            {
                builder.AddIndent(indent).AppendLine($"SupportsQosRange: {m_SupportsQosRange}");
            }
            if (HasSupportsOutOfBandSnapshots)
            {
                builder.AddIndent(indent).AppendLine($"SupportsOutOfBandSnapshots: {m_SupportsOutOfBandSnapshots}");
            }
            if (HasAcceptingConsumerStatus)
            {
                builder.AddIndent(indent).AppendLine($"AcceptingConsumerStatus: {m_AcceptingConsumerStatus}");
            }
            if (HasItemList)
            {
                builder.AddIndent(indent).AppendLine($"ItemList: {m_ItemList}");
            }
            
            builder.AddIndent(indent).AppendLine($"CapabilitiesList:");
            m_CapabilitiesList.AppendToString(builder, indent + 1, (sb, item, i) => sb.AddIndent(i).Append(item).AppendLine());

            if (HasDictionariesProvidedList)
            {
                builder.AddIndent(indent).AppendLine($"DictionariesProvidedList:");
                m_DictionariesProvidedList.AppendToString(builder, indent + 1, (sb, item, i) => sb.AddIndent(i).AppendLine(item));
            }
            if (HasDictionariesUsedList)
            {
                builder.AddIndent(indent).AppendLine($"DictionariesUsedList:");
                m_DictionariesUsedList.AppendToString(builder, indent + 1, (sb, item, i) => sb.AddIndent(i).AppendLine(item));
            }
            if (HasQosList)
            {
                builder.AddIndent(indent).AppendLine($"QosList:");
                m_QosList.AppendToString(builder, indent + 1, (sb, item, i) => sb.AddIndent(i).AppendLine(item.ToString()));
            }
        }

        internal override void DecodeFrom(ElementList elementList)
        {
            Clear();

            var foundServiceName = false;
            var foundCapabilities = false;
            foreach (var elementEntry in elementList)
            {
                var elementName = elementEntry.Name;
                switch (elementName)
                {
                    case EmaRdm.ENAME_NAME:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            ServiceName(elementEntry.OmmAsciiValue().Value);
                            foundServiceName = true;
                        }
                        break;
                    case EmaRdm.ENAME_VENDOR:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            Vendor(elementEntry.OmmAsciiValue().Value);
                        }
                        break;
                    case EmaRdm.ENAME_IS_SOURCE:
                        var isSource = elementEntry.UIntValue();
                        if (isSource == 0 || isSource == 1)
                        {
                            IsSource(isSource == 1);
                        }
                        else
                        {
                            Clear();
                            throw new OmmInvalidUsageException($"Invalid element value of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                        break;
                    case EmaRdm.ENAME_CAPABILITIES:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            var capabilities = elementEntry.OmmArrayValue();
                            foreach (var capability in capabilities)
                            {
                                m_CapabilitiesList.Add(capability.OmmUIntValue().Value);
                            }
                            foundCapabilities = true;
                        }
                        break;
                    case EmaRdm.ENAME_DICTIONARYS_PROVIDED:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            var dictionaries = elementEntry.OmmArrayValue();
                            foreach (var dictionary in dictionaries)
                            {
                                m_DictionariesProvidedList.Add(dictionary.OmmAsciiValue().Value);
                            }
                        }
                        HasDictionariesProvidedList = true;
                        break;
                    case EmaRdm.ENAME_DICTIONARYS_USED:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            var dictionaries = elementEntry.OmmArrayValue();
                            foreach (var dictionary in dictionaries)
                            {
                                m_DictionariesUsedList.Add(dictionary.OmmAsciiValue().Value);
                            }
                        }
                        HasDictionariesUsedList = true;
                        break;
                    case EmaRdm.ENAME_QOS:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            var qosArray = elementEntry.OmmArrayValue();
                            foreach (var arrEntry in qosArray)
                            {
                                var qos = arrEntry.OmmQosValue();
                                m_QosList.Add(new DirectoryQos().Timeliness(qos.Timeliness).Rate(qos.Rate));
                            }
                        }
                        HasQosList = true;
                        break;
                    case EmaRdm.ENAME_SUPPS_QOS_RANGE:
                        var supportsQosRange = elementEntry.UIntValue();
                        if (supportsQosRange == 0 || supportsQosRange == 1)
                        {
                            SupportsQosRange(supportsQosRange == 1);
                        }
                        else
                        {
                            Clear();
                            throw new OmmInvalidUsageException($"Invalid element value of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                        break;
                    case EmaRdm.ENAME_ITEM_LIST:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            ItemList(elementEntry.OmmAsciiValue().Value);
                        }
                        break;
                    case EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS:
                        var supportsOOBSnapshots = elementEntry.UIntValue();
                        if (supportsOOBSnapshots == 0 || supportsOOBSnapshots == 1)
                        {
                            SupportsOutOfBandSnapshots(supportsOOBSnapshots == 1);
                        }
                        else
                        {
                            Clear();
                            throw new OmmInvalidUsageException($"Invalid element value of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                        break;
                    case EmaRdm.ENAME_ACCEPTING_CONS_STATUS:
                        var acceptingConsumerStatus = elementEntry.UIntValue();
                        if (acceptingConsumerStatus == 0 || acceptingConsumerStatus == 1)
                        {
                            AcceptingConsumerStatus(acceptingConsumerStatus == 1);
                        }
                        else
                        {
                            Clear();
                            throw new OmmInvalidUsageException($"Invalid element value of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                        break;
                    default:
                        break;
                }
            }
            if (!foundServiceName || !foundCapabilities)
            {
                Clear();
                throw new OmmInvalidUsageException($"{EmaRdm.ENAME_NAME} or {EmaRdm.ENAME_CAPABILITIES} element is absent", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }   

        internal override void EncodeTo(ElementList elementList)
        {
            elementList.AddAscii(EmaRdm.ENAME_NAME, m_ServiceName);

            if (HasVendor)
            {
                elementList.AddAscii(EmaRdm.ENAME_VENDOR, m_Vendor.Value);
            }

            if (HasIsSource)
            {
                elementList.AddUInt(EmaRdm.ENAME_IS_SOURCE, m_IsSource.Value ? 1ul : 0ul);
            }

            var ommArray = new OmmArray();

            elementList.AddArray(EmaRdm.ENAME_CAPABILITIES, m_CapabilitiesList.ToOmmArray((arr, item) => arr.AddUInt(item), ommArray));
            ommArray.Clear();

            if (HasDictionariesProvidedList)
            {
                elementList.AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED,
                    m_DictionariesProvidedList.ToOmmArray((arr, item) => arr.AddAscii(item), ommArray));
                ommArray.Clear();
            }

            if (HasDictionariesUsedList)
            {
                elementList.AddArray(EmaRdm.ENAME_DICTIONARYS_USED,
                    m_DictionariesUsedList.ToOmmArray((arr, item) => arr.AddAscii(item), ommArray));
                ommArray.Clear();
            }

            if (HasQosList)
            {
                elementList.AddArray(EmaRdm.ENAME_QOS,
                    m_QosList.ToOmmArray((arr, item) => arr.AddQos(item.Timeliness(), item.Rate()), ommArray));
                ommArray.Clear();
            }

            if (HasSupportsQosRange)
            {
                elementList.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, m_SupportsQosRange.Value ? 1u : 0u);
            }

            if (HasItemList)
            {
                elementList.AddAscii(EmaRdm.ENAME_ITEM_LIST, m_ItemList.Value);
            }

            if (HasSupportsOutOfBandSnapshots)
            {
                elementList.AddUInt(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, m_SupportsOutOfBandSnapshots.Value ? 1u : 0u);
            }

            if (HasAcceptingConsumerStatus)
            {
                elementList.AddUInt(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, m_AcceptingConsumerStatus.Value ? 1u : 0u);
            }
        }
    }
}