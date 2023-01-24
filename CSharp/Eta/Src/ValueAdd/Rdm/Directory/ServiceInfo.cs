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
using static LSEG.Eta.Rdm.Directory;
using Array = LSEG.Eta.Codec.Array;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Info. Contains information provided by the Source Directory Info filter.
    /// </summary>
    sealed public class ServiceInfo
    {
        private Buffer _serviceName;
        private Buffer _vendor;
        private long _isSource;
        private List<long> _capabilitiesList;
        private List<string> _dictionariesProvidedList;
        private List<string> _dictionariesUsedList;
        private List<Qos> _qosList;
        private Buffer _itemList;
        private Qos? _bestQos;

        private long _supportsQosRange;
        private long _supportsOutOfBandSnapshots;
        private long _acceptingConsumerStatus;

        private StringBuilder stringBuf = new StringBuilder();
        private const string eol = "\n";
        private const string tab = "\t";

        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_ElementEntry = new ElementEntry();
        private UInt tmpUInt = new UInt();
        private Array array = new Array();
        private ArrayEntry arrayEntry = new ArrayEntry();
        private Buffer tmpBuffer = new Buffer();

        /// <summary>
        /// Checks the presence of the vendor field.
        /// </summary>
        public bool HasVendor { get => (Flags & ServiceInfoFlags.HAS_VENDOR) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_VENDOR; else Flags &= ~ServiceInfoFlags.HAS_VENDOR; } }

        /// <summary>
        /// Checks the presence of the isSource field.
        /// </summary>
        public bool HasIsSource { get => (Flags & ServiceInfoFlags.HAS_IS_SOURCE) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_IS_SOURCE; else Flags &= ~ServiceInfoFlags.HAS_IS_SOURCE; } }

        /// <summary>
        /// Checks the presence of the dictionariesProvided field.
        /// </summary>
        public bool HasDictionariesProvided { get => (Flags & ServiceInfoFlags.HAS_DICTS_PROVIDED) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_DICTS_PROVIDED; else Flags &= ~ServiceInfoFlags.HAS_DICTS_PROVIDED; } }

        /// <summary>
        /// Checks the presence of the dictionariesUsed field.
        /// </summary>
        public bool HasDictionariesUsed { get => (Flags & ServiceInfoFlags.HAS_DICTS_USED) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_DICTS_USED; else Flags &= ~ServiceInfoFlags.HAS_DICTS_USED; } }

        /// <summary>
        /// Checks the presence of the qosList field.
        /// </summary>
        public bool HasQos { get => (Flags & ServiceInfoFlags.HAS_QOS) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_QOS; else Flags &= ~ServiceInfoFlags.HAS_QOS; } }

        /// <summary>
        /// Checks the presence of the supportsQosRange field.
        /// </summary>
        public bool HasSupportQosRange { get => (Flags & ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE; else Flags &= ~ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE; } }

        /// <summary>
        /// Checks the presence of the itemList field.
        /// </summary>
        public bool HasItemList { get => (Flags & ServiceInfoFlags.HAS_ITEM_LIST) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_ITEM_LIST; else Flags &= ~ServiceInfoFlags.HAS_ITEM_LIST; } }

        /// <summary>
        /// Checks the presence of the supportsOutOfBandSnapshots field.
        /// </summary>
        public bool HasSupportOOBSnapshots { get => (Flags & ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS; else Flags &= ~ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS; } }

        /// <summary>
        /// Checks the presence of the acceptingConsumerStatus field.
        /// </summary>
        public bool HasAcceptingConsStatus { get => (Flags & ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS) != 0; set { if (value) Flags |= ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS; else Flags &= ~ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS; } }

        
        /// <summary>
        /// Action associated with this info filter.
        /// </summary>
        public FilterEntryActions Action { get; set; }

        /// <summary>
        /// The service name that identifies this service.
        /// </summary>
        public Buffer ServiceName { get => _serviceName; set { Debug.Assert(value != null); BufferHelper.CopyBuffer(value, _serviceName); } }

        /// <summary>
        /// The vendor name of data provided by this service.
        /// </summary>
        public Buffer Vendor { get => _vendor; set { Debug.Assert(value != null); BufferHelper.CopyBuffer(value, _vendor); } }

        /// <summary>
        /// isSource field that indicates whether the service is provided directly by a publisher or consolidated from multiple sources.
        /// </summary>
        public long IsSource { get => _isSource; set { Debug.Assert(HasIsSource); _isSource = value; } }

        /// <summary>
        /// Field that indicates whether items can be requested using a QoS range 
        /// (using both the qos and worstQos members of a Request message).
        /// </summary>
        public long SupportsQosRange { get => _supportsQosRange; set { Debug.Assert(HasSupportQosRange); _supportsQosRange = value; } }

        /// <summary>
        /// Field that indicates whether Snapshot (requests without the STREAMING flag) 
        /// can be made when the OpenLimit is reached.
        /// </summary>
        public long SupportsOOBSnapshots { get => _supportsOutOfBandSnapshots; set { Debug.Assert(HasSupportOOBSnapshots); _supportsOutOfBandSnapshots = value; } }

        /// <summary>
        /// Field that indicates whether the service accepts messages related to Source Mirroring.
        /// </summary>
        public long AcceptConsumerStatus { get => _acceptingConsumerStatus; set { Debug.Assert(HasAcceptingConsStatus); _acceptingConsumerStatus = value; } }

        /// <summary>
        /// The list of item names a Consumer can request to get 
        /// a symbol list of all item names available from this service.
        /// </summary>
        public Buffer ItemList { get => _itemList; set { Debug.Assert(value != null); BufferHelper.CopyBuffer(value, _itemList); } }

        /// <summary>
        /// The list of capabilities the service supports. Capability in the list is populated by <see cref="DomainTypes"/>
        /// </summary>
        public List<long> CapabilitiesList { 
            get => _capabilitiesList; 
            set 
            {
                Debug.Assert(value != null);
                _capabilitiesList.Clear();
                _capabilitiesList.AddRange(value);
            } 
        }

        /// <summary>
        /// The list of dictionary names that this service provides.
        /// </summary>
        public List<string> DictionariesProvidedList { 
            get => _dictionariesProvidedList; 
            set
            {
                Debug.Assert(value != null);
                _dictionariesProvidedList.Clear();
                _dictionariesProvidedList.AddRange(value);
            }
        }

        /// <summary>
        /// The list of qualities of service that this service provides.
        /// </summary>
        public List<Qos> QosList
        {
            get => _qosList;
            set
            {
                Debug.Assert(value != null);
                _qosList.Clear();
                foreach (Qos qos in value)
                {
                    Qos copyqos = new Qos();
                    copyqos.IsDynamic = qos.IsDynamic;
                    copyqos.Rate(qos.Rate());
                    copyqos.RateInfo(qos.RateInfo());
                    copyqos.TimeInfo(qos.TimeInfo());
                    copyqos.Timeliness(qos.Timeliness());
                    _qosList.Add(copyqos);
                }
            }
        }

        /// <summary>
        /// The list of dictionary names that a Consumer will require to decode the service's market data.
        /// </summary>
        public List<string> DictionariesUsedList
        {
            get => _dictionariesUsedList;
            set
            {
                Debug.Assert(value != null);
                _dictionariesUsedList.Clear();
                _dictionariesUsedList.AddRange(value);
            }
        }

        /// <summary>
        /// The filterId - Populated by <see cref="ServiceFilterIds"/>
        /// </summary>
        public int FilterId { get => ServiceFilterIds.INFO; }

        /// <summary>
        /// The best quality of service that this service provides.
        /// </summary>
        public Qos BestQos
        {
            get 
            {
                if (_bestQos == null)
                {
                    _bestQos = new Qos();
                    foreach (Qos qos in QosList)
                    {
                        if (qos.IsBetter(_bestQos))
                        {
                            _bestQos = qos;
                        }
                    }
                }
                return _bestQos;
            }
        }

        /// <summary>
        /// The service info flags. Populated by <see cref="ServiceInfoFlags"/>
        /// </summary>
        public ServiceInfoFlags Flags { get; set; }

        /// <summary>
        /// Performs an update of this Service Info object to the destination object. 
        /// </summary>
        /// <param name="destServiceInfo">Service Info object to be updated by this object. It cannot be null.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation.</returns>
        public CodecReturnCode Update(ServiceInfo destServiceInfo)
        {
            Debug.Assert(destServiceInfo != null);
            BufferHelper.CopyBuffer(ServiceName, destServiceInfo.ServiceName);
            destServiceInfo.Action = Action;
            destServiceInfo.CapabilitiesList = CapabilitiesList;
            if (HasAcceptingConsStatus)
            {
                destServiceInfo.HasAcceptingConsStatus = true;
                destServiceInfo.AcceptConsumerStatus = AcceptConsumerStatus;
            }
            if (HasDictionariesProvided)
            {
                destServiceInfo.HasDictionariesProvided = true;
                destServiceInfo.DictionariesProvidedList = DictionariesProvidedList;
            }
            if (HasDictionariesUsed)
            {
                destServiceInfo.HasDictionariesUsed = true;
                destServiceInfo.DictionariesUsedList = DictionariesUsedList;
            }
            if (HasIsSource)
            {
                destServiceInfo.HasIsSource = true;
                destServiceInfo.IsSource = IsSource;
            }
            if (HasItemList)
            {
                destServiceInfo.HasItemList = true;
                BufferHelper.CopyBuffer(ItemList, destServiceInfo.ItemList);
            }
            if (HasQos)
            {
                destServiceInfo.HasQos = true;
                destServiceInfo.QosList.Clear();
                foreach (Qos qos in QosList)
                {
                    Qos copyqos = new Qos();
                    copyqos.IsDynamic = qos.IsDynamic;
                    copyqos.Rate(qos.Rate());
                    copyqos.RateInfo(qos.RateInfo());
                    copyqos.TimeInfo(qos.TimeInfo());
                    copyqos.Timeliness(qos.Timeliness());
                    destServiceInfo.QosList.Add(copyqos);
                }
            }
            if (HasSupportOOBSnapshots)
            {
                destServiceInfo.HasSupportOOBSnapshots = true;
                destServiceInfo.SupportsOOBSnapshots = SupportsOOBSnapshots;
            }
            if (HasSupportQosRange)
            {
                destServiceInfo.HasSupportQosRange = true;
                destServiceInfo.SupportsQosRange = SupportsQosRange;
            }

            if (HasVendor)
            {
                destServiceInfo.HasVendor = true;
                BufferHelper.CopyBuffer(Vendor, destServiceInfo.Vendor);
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Directory Service Info constructor.
        /// </summary>
        public ServiceInfo()
        {
            _capabilitiesList = new List<long>();
            _dictionariesProvidedList = new List<string>();
            _dictionariesUsedList = new List<string>();
            _qosList = new List<Qos>();
            _supportsOutOfBandSnapshots = 1;
            _acceptingConsumerStatus = 1;
            _serviceName = new Buffer();
            _vendor = new Buffer();
            _itemList = new Buffer();
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Directory Service Info object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Flags = 0;
            Action = FilterEntryActions.SET;
            _isSource = 0;
            _capabilitiesList.Clear();
            _dictionariesProvidedList.Clear();
            _dictionariesUsedList.Clear();
            _qosList.Clear();
            _serviceName.Clear();
            _itemList.Clear();
            _vendor.Clear();
            _supportsOutOfBandSnapshots = 1;
            _acceptingConsumerStatus = 1;
            _supportsQosRange = 0;
            _bestQos = null;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destServiceInfo</c>.
        /// </summary>
        /// <param name="destServiceInfo">ServiceInfo object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(ServiceInfo destServiceInfo)
        {
            Debug.Assert(destServiceInfo != null);
            destServiceInfo.Clear();
            BufferHelper.CopyBuffer(ServiceName, destServiceInfo.ServiceName);
            destServiceInfo.Action = Action;
            destServiceInfo.CapabilitiesList = CapabilitiesList;
            if (HasAcceptingConsStatus)
            {
                destServiceInfo.HasAcceptingConsStatus = true;
                destServiceInfo.AcceptConsumerStatus = AcceptConsumerStatus;
            }
            if (HasDictionariesProvided)
            {
                destServiceInfo.HasDictionariesProvided = true;
                destServiceInfo.DictionariesProvidedList = DictionariesProvidedList;
            }
            if (HasDictionariesUsed)
            {
                destServiceInfo.HasDictionariesUsed = true;
                destServiceInfo.DictionariesUsedList = DictionariesUsedList;
            }
            if (HasIsSource)
            {
                destServiceInfo.HasIsSource = true;
                destServiceInfo.IsSource = IsSource;
            }
            if (HasItemList)
            {
                destServiceInfo.HasItemList = true;
                BufferHelper.CopyBuffer(ItemList, destServiceInfo.ItemList);
            }
            if (HasQos)
            {
                destServiceInfo.HasQos = true;
                foreach (Qos qos in QosList)
                {
                    Qos copyqos = new Qos();
                    copyqos.IsDynamic = qos.IsDynamic;
                    copyqos.Rate(qos.Rate());
                    copyqos.RateInfo(qos.RateInfo());
                    copyqos.TimeInfo(qos.TimeInfo());
                    copyqos.Timeliness(qos.Timeliness());
                    destServiceInfo.QosList.Add(copyqos);
                }
            }
            if (HasSupportOOBSnapshots)
            {
                destServiceInfo.HasSupportOOBSnapshots = true;
                destServiceInfo.SupportsOOBSnapshots = SupportsOOBSnapshots;
            }
            if (HasSupportQosRange)
            {
                destServiceInfo.HasSupportQosRange = true;
                destServiceInfo.SupportsQosRange = SupportsQosRange;
            }

            if (HasVendor)
            {
                destServiceInfo.HasVendor = true;
                BufferHelper.CopyBuffer(Vendor, destServiceInfo.Vendor);
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Service Info using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();
            CodecReturnCode ret = m_ElementList.EncodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            m_ElementEntry.Clear();
            m_ElementEntry.Name = ElementNames.NAME;
            m_ElementEntry.DataType = Codec.DataTypes.ASCII_STRING;
            ret = m_ElementEntry.Encode(encodeIter, _serviceName);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (HasVendor)
            {
                m_ElementEntry.Name = ElementNames.VENDOR;
                m_ElementEntry.DataType = Codec.DataTypes.ASCII_STRING;
                ret = m_ElementEntry.Encode(encodeIter, Vendor);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasIsSource)
            {
                m_ElementEntry.Name = ElementNames.IS_SOURCE;
                m_ElementEntry.DataType = Codec.DataTypes.UINT;
                tmpUInt.Value(IsSource);
                ret = m_ElementEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            /* Capabilities */
            m_ElementEntry.Name = ElementNames.CAPABILITIES;
            m_ElementEntry.DataType = Codec.DataTypes.ARRAY;
            ret = m_ElementEntry.EncodeInit(encodeIter, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            array.Clear();
            array.PrimitiveType = Codec.DataTypes.UINT;
            array.ItemLength = 0;
            ret = array.EncodeInit(encodeIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            foreach (long capability in CapabilitiesList)
            {
                tmpUInt.Value(capability);
                ret = arrayEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            ret = array.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            ret = m_ElementEntry.EncodeComplete(encodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            /* DictionariesProvided */
            if (HasDictionariesProvided)
            {
                m_ElementEntry.Name = ElementNames.DICTIONARIES_PROVIDED;
                m_ElementEntry.DataType = Codec.DataTypes.ARRAY;
                ret = m_ElementEntry.EncodeInit(encodeIter, 0);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                array.Clear();
                array.PrimitiveType = Codec.DataTypes.ASCII_STRING;
                array.ItemLength = 0;
                ret = array.EncodeInit(encodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                foreach (string dictProvided in DictionariesProvidedList)
                {
                    tmpBuffer.Data(dictProvided);
                    ret = arrayEntry.Encode(encodeIter, tmpBuffer);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
                ret = array.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                ret = m_ElementEntry.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            /* DictionariesUsed */
            if (HasDictionariesUsed)
            {
                m_ElementEntry.Name = ElementNames.DICTIONARIES_USED;
                m_ElementEntry.DataType = Codec.DataTypes.ARRAY;
                ret = m_ElementEntry.EncodeInit(encodeIter, 0);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                array.Clear();
                array.PrimitiveType = Codec.DataTypes.ASCII_STRING;
                array.ItemLength = 0;
                ret = array.EncodeInit(encodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                foreach (string dictUsed in DictionariesUsedList)
                {
                    tmpBuffer.Data(dictUsed);
                    ret = arrayEntry.Encode(encodeIter, tmpBuffer);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
                ret = array.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                ret = m_ElementEntry.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasQos)
            {
                m_ElementEntry.Name = ElementNames.QOS;
                m_ElementEntry.DataType = Codec.DataTypes.ARRAY;
                ret = m_ElementEntry.EncodeInit(encodeIter, 0);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                array.Clear();
                array.PrimitiveType = Codec.DataTypes.QOS;
                array.ItemLength = 0;
                ret = array.EncodeInit(encodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                foreach (Qos qos in QosList)
                {
                    ret = arrayEntry.Encode(encodeIter, qos);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
                ret = array.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                ret = m_ElementEntry.EncodeComplete(encodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasSupportQosRange)
            {
                m_ElementEntry.Name = ElementNames.SUPPS_QOS_RANGE;
                m_ElementEntry.DataType = Codec.DataTypes.UINT;
                tmpUInt.Value(SupportsQosRange);
                ret = m_ElementEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasItemList)
            {
                m_ElementEntry.Name = ElementNames.ITEM_LIST;
                m_ElementEntry.DataType = Codec.DataTypes.ASCII_STRING;
                ret = m_ElementEntry.Encode(encodeIter, ItemList);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasSupportOOBSnapshots)
            {
                m_ElementEntry.Name = ElementNames.SUPPS_OOB_SNAPSHOTS;
                m_ElementEntry.DataType = Codec.DataTypes.UINT;
                tmpUInt.Value(SupportsOOBSnapshots);
                ret = m_ElementEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasAcceptingConsStatus)
            {
                m_ElementEntry.Name = ElementNames.ACCEPTING_CONS_STATUS;
                m_ElementEntry.DataType = Codec.DataTypes.UINT;
                tmpUInt.Value(AcceptConsumerStatus);
                ret = m_ElementEntry.Encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
            return m_ElementList.EncodeComplete(encodeIter, true);
        }

        /// <summary>
        /// Decodes this Service Info using the provided <c>decodeIter</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter)
        {
            Clear();
            m_ElementList.Clear();
            m_ElementEntry.Clear();
            array.Clear();

            CodecReturnCode ret = m_ElementList.Decode(decodeIter, null);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            while ((ret = m_ElementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                if (m_ElementEntry.Name.Equals(ElementNames.NAME))
                {
                    ServiceName = m_ElementEntry.EncodedData;
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.VENDOR))
                {
                    HasVendor = true;
                    Vendor = m_ElementEntry.EncodedData;
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.IS_SOURCE))
                {
                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasIsSource = true;
                    IsSource = tmpUInt.ToLong();
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.CAPABILITIES))
                {
                    if ((ret = array.Decode(decodeIter)) < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    while ((ret = arrayEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCode.SUCCESS)
                        {
                            ret = tmpUInt.Decode(decodeIter);
                            CapabilitiesList.Add(tmpUInt.ToLong());
                            if (ret != CodecReturnCode.SUCCESS
                                    && ret != CodecReturnCode.BLANK_DATA)
                            {
                                return ret;
                            }
                        }
                        else if (ret != CodecReturnCode.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.DICTIONARIES_PROVIDED))
                {
                    ret = array.Decode(decodeIter);
                    if (ret < CodecReturnCode.SUCCESS)
                        return ret;
                    HasDictionariesProvided = true;
                    while ((ret = arrayEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCode.SUCCESS)
                        {
                            DictionariesProvidedList.Add(arrayEntry.EncodedData.ToString());
                        }
                        else if (ret != CodecReturnCode.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.DICTIONARIES_USED))
                {
                    ret = array.Decode(decodeIter);
                    if (ret < CodecReturnCode.SUCCESS)
                        return ret;

                    HasDictionariesUsed = true;
                    while ((ret = arrayEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCode.SUCCESS)
                        {
                            DictionariesUsedList.Add(arrayEntry.EncodedData.ToString());
                        }
                        else if (ret != CodecReturnCode.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.QOS))
                {
                    if ((ret = array.Decode(decodeIter)) < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    HasQos = true;
                    while ((ret = arrayEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (ret == CodecReturnCode.SUCCESS)
                        {
                            Qos qos = new Qos();
                            ret = qos.Decode(decodeIter);
                            QosList.Add(qos);
                            if (ret != CodecReturnCode.SUCCESS
                                    && ret != CodecReturnCode.BLANK_DATA)
                            {
                                return ret;
                            }
                        }
                        else if (ret != CodecReturnCode.BLANK_DATA)
                        {
                            return ret;
                        }
                    }
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.SUPPS_QOS_RANGE))
                {
                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasSupportQosRange = true;
                    SupportsQosRange = tmpUInt.ToLong();
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.ITEM_LIST))
                {
                    HasItemList = true;
                    ItemList = m_ElementEntry.EncodedData;
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.SUPPS_OOB_SNAPSHOTS))
                {
                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasSupportOOBSnapshots = true;
                    SupportsOOBSnapshots = tmpUInt.ToLong();
                }
                else if (m_ElementEntry.Name.Equals(ElementNames.ACCEPTING_CONS_STATUS))
                {
                    ret = tmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasAcceptingConsStatus = true;
                    AcceptConsumerStatus = tmpUInt.ToLong();
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Directory Service Info.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            stringBuf.Clear();
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("InfoFilter:");
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("serviceName: ");
            stringBuf.Append(ServiceName);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("vendor: ");
            stringBuf.Append(Vendor);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("isSource: ");
            stringBuf.Append(IsSource);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("supportsQosRange: ");
            stringBuf.Append(SupportsQosRange);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("supportsOutOfBandSnapshots: ");
            stringBuf.Append(SupportsOOBSnapshots);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("acceptingConsumerStatus: ");
            stringBuf.Append(AcceptConsumerStatus);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("capabilities: ");
            stringBuf.Append(eol);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            foreach (var cap in CapabilitiesList)
            {                
                stringBuf.Append(cap);
                stringBuf.Append(" ");
            }
            stringBuf.Append(eol);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("dictionariesProvided: ");
            stringBuf.Append(eol);
            foreach (var dict in DictionariesProvidedList)
            {
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(dict);
                stringBuf.Append(eol);
            }
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("dictionariesUsed: ");
            stringBuf.Append(eol);
            foreach (var dict in DictionariesUsedList)
            {
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(dict);
                stringBuf.Append(eol);
            }
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("qos: ");
            stringBuf.Append(eol);
            foreach (var q in QosList)
            {
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(q.ToString());
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
