/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;


namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// The directory handler for the ProvPerf and NIProvPerf. 
    /// Configures a single service and provides encoding and sending of a directory message.
    /// </summary>
    public class DirectoryProvider
    {
        protected const int REFRESH_MSG_SIZE = 1024;

        // vendor name
        protected const string vendor = "LSEG";

        // field dictionary used and provided for the source. 
        protected const string fieldDictionaryName = "RWFFld";

        // enum dictionary used and provided for the source.
        protected const string enumTypeDictionaryName = "RWFEnum";

        protected DirectoryRefresh m_DirectoryRefresh;
        protected EncodeIterator m_EncodeIter;

        // Source service information 
        protected Service m_Service;

        // Open limit
        protected int m_OpenLimit;

        // Service id 
        protected int m_ServiceId;

        // Service name
        protected string? m_ServiceName;

        // Service qos
        protected Qos m_Qos;

        public DirectoryRefresh DirectoryRefresh { get => m_DirectoryRefresh; }

        /// <summary>
        /// Instantiates a new directory provider.
        /// </summary>
        public DirectoryProvider()
        {
            m_DirectoryRefresh = new DirectoryRefresh();
            m_Service = new Service();
            m_Qos = new Qos();

            m_Qos.IsDynamic = false;
            m_Qos.Rate(QosRates.TICK_BY_TICK);
            m_Qos.Timeliness(QosTimeliness.REALTIME);

            m_EncodeIter = new EncodeIterator();
        }

        /// <summary>
        /// Iniitalizes Service
        /// </summary>
        /// <param name="xmlMsgData"><see cref="XmlMsgData"/> instance</param>
        public void InitService(XmlMsgData xmlMsgData)
        {
            m_Service.Clear();

            m_Service.Flags = ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_LOAD;
            m_Service.ServiceId = m_ServiceId;
            m_Service.Action = MapEntryActions.ADD;

            // Info 
            m_Service.Info.Action = FilterEntryActions.SET;
            m_Service.Info.ServiceName.Data(m_ServiceName);

            m_Service.Info.HasVendor = true;
            m_Service.Info.Vendor.Data(vendor);

            m_Service.Info.HasIsSource = true;
            m_Service.Info.IsSource = 1;

            m_Service.Info.CapabilitiesList.Add((long)DomainType.DICTIONARY);
            m_Service.Info.CapabilitiesList.Add((long)DomainType.SYSTEM);
            if (xmlMsgData.HasMarketPrice)
            {
                m_Service.Info.CapabilitiesList.Add((long)DomainType.MARKET_PRICE);
            }

            m_Service.Info.HasDictionariesProvided = true;
            m_Service.Info.DictionariesProvidedList.Add(enumTypeDictionaryName);
            m_Service.Info.DictionariesProvidedList.Add(fieldDictionaryName);

            m_Service.Info.HasDictionariesUsed = true;
            m_Service.Info.DictionariesUsedList.Add(enumTypeDictionaryName);
            m_Service.Info.DictionariesUsedList.Add(fieldDictionaryName);

            m_Service.Info.HasQos = true;
            m_Service.Info.QosList.Add(m_Qos);

            m_Service.Info.HasSupportQosRange = true;
            m_Service.Info.SupportsQosRange = 0;

            m_Service.Info.HasSupportOOBSnapshots = true;
            m_Service.Info.SupportsOOBSnapshots = 0;

            // State
            m_Service.State.Action = FilterEntryActions.SET;
            m_Service.State.ServiceStateVal = 1;
            m_Service.State.HasAcceptingRequests = true;
            m_Service.State.AcceptingRequests = 1;

            // Load 
            if (m_OpenLimit > 0)
            {
                m_Service.Load.Action = FilterEntryActions.SET;
                m_Service.Load.HasOpenLimit = true;
                m_Service.Load.OpenLimit = m_OpenLimit;
            }
        }
    }
}
