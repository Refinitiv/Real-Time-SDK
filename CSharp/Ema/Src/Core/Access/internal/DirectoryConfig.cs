/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using NLog.Config;
using NLog.LayoutRenderers.Wrappers;

namespace LSEG.Ema.Access
{
    internal class EmaServiceConfig
    {
        public Service Service { get; set; }
        public List<string> DictionariesUsedList { get; set; }
        public List<string> DictionariesProvidedList { get; set; }
        private bool isNiProv;

        public EmaServiceConfig(bool isNiProv, bool isDefaultService)
        {
            Service = new Service();
            DictionariesUsedList = new List<string>();
            DictionariesProvidedList = new List<string>();
            if (!isDefaultService)
            {
                
                this.isNiProv = isNiProv;

                Clear();
            }
            else
            {
                if (isNiProv)
                {
                    Service.Info.ServiceName.Data("NI_PUB");
                    Service.ServiceId = 0;

                    Service.State.HasAcceptingRequests = true;
                    Service.State.AcceptingRequests = 0;

                    Service.Info.HasSupportQosRange = true;
                    Service.Info.SupportsQosRange = 0;
                }
                else
                {
                    Service.Info.ServiceName.Data("DIRECT_FEED");
                    Service.ServiceId = 1;

                    Service.State.HasAcceptingRequests = true;
                    Service.State.AcceptingRequests = 1;

                    Service.Info.HasSupportQosRange = true;
                    Service.Info.SupportsQosRange = 1;

                    Service.Info.HasDictionariesProvided = true;

                    Service.Info.DictionariesProvidedList.Add("RWFFld");
                    Service.Info.DictionariesProvidedList.Add("RWFEnum");

                    Service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_DICTIONARY);
                }

                Service.Action = MapEntryActions.ADD;

                Service.HasInfo = true;
                Service.Info.Action = FilterEntryActions.SET;

                Service.Info.HasVendor = true;
                Service.Info.Vendor.Data("");

                Service.Info.HasIsSource = true;
                Service.Info.IsSource = 0;

                Service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_PRICE);
                Service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_BY_ORDER);
                Service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_BY_PRICE);
                Service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_MAKER);

                Service.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                Service.Info.QosList.Add(qos);

                Service.Info.HasDictionariesUsed = true;
                Service.Info.DictionariesUsedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID);
                Service.Info.DictionariesUsedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM);

                Service.Info.HasItemList = true;
                Service.Info.ItemList.Data("");

                Service.Info.HasAcceptingConsStatus = true;
                Service.Info.AcceptConsumerStatus = 1;

                Service.Info.HasSupportOOBSnapshots = true;
                Service.Info.SupportsOOBSnapshots = 0;

                Service.HasState = true;
                Service.State.Action = FilterEntryActions.SET;
                Service.State.ServiceStateVal = 1;
            }
        }

        public EmaServiceConfig(EmaServiceConfig oldConfig)
        {
            Service = new Service();
            oldConfig.Service.Copy(Service);
            
            DictionariesUsedList = new List<string>();
            foreach (string dictionaryUsed in oldConfig.DictionariesUsedList)
            {
                DictionariesUsedList.Add(dictionaryUsed);
            }

            DictionariesProvidedList = new List<string>();
            foreach (string dictionaryProvided in oldConfig.DictionariesProvidedList)
            {
                DictionariesProvidedList.Add(dictionaryProvided);
            }
        }

        // This will shallow copy oldService into the service, preserving the object.
        public EmaServiceConfig(bool niProv, Service oldService)
        {
            Service = oldService;
            DictionariesUsedList = new List<string>();
            DictionariesProvidedList = new List<string>();
            isNiProv = niProv;
        }

        void Clear()
        {
            Service.Clear();
            Service.Action = MapEntryActions.ADD;
            Service.Info.Action = FilterEntryActions.SET;
            Service.Load.Action = FilterEntryActions.SET;
            // Set defaults for the service depending if this is for an niProv or iProv service.
            Service.HasInfo = true;
            Service.Info.HasIsSource = true;
            Service.Info.IsSource = 0;
            Service.Info.HasAcceptingConsStatus = true;
            Service.Info.AcceptConsumerStatus = 1;
            Service.Info.HasQos = true;
            // Qos will be added with a realtime/tick-by-tick QoS if nothing else is in the list
            Service.Info.HasSupportOOBSnapshots = true;
            Service.Info.SupportsOOBSnapshots = 0;


            Service.HasState = true;
            if (isNiProv)
            {
                Service.State.HasAcceptingRequests = true;
                Service.State.AcceptingRequests = 0;
            }
            else
            {
                Service.State.HasAcceptingRequests = true;
                Service.State.AcceptingRequests = 1;
            }

            Service.State.HasStatus = true;
            Service.State.Status.DataState(DataStates.OK);
            Service.State.Status.StreamState(StreamStates.OPEN);
            Service.State.Status.Code(StateCodes.NONE);

            Service.HasLoad = true;

            DictionariesUsedList.Clear();
            DictionariesProvidedList.Clear();
        }
    }
    // This class represents the Directory group configuration
	internal class DirectoryConfig
    {
        // Name of this Directory config.
        public string Name { get; set; } = string.Empty;

        public Dictionary<string, EmaServiceConfig> ServiceMap { get; set; }

        public DirectoryConfig()
        {
            ServiceMap = new Dictionary<string, EmaServiceConfig>();
            Clear();
        }

        public DirectoryConfig(DirectoryConfig oldConfig)
        {
            Name = oldConfig.Name;
            ServiceMap = new Dictionary<string, EmaServiceConfig>();
            foreach (KeyValuePair <string, EmaServiceConfig> servicePair in oldConfig.ServiceMap)
            {
                EmaServiceConfig newConfig = new EmaServiceConfig(servicePair.Value);
                ServiceMap.Add(servicePair.Key, newConfig);
            }
        }

        public void Clear()
        {
            Name = string.Empty;
            ServiceMap.Clear();
        }

        /// <summary>
        /// Returns the QosRate value from the input string.
        /// </summary>
        /// <returns>the rate value of the string </returns>
        public static uint StringToTimeliness(string timeliness) => timeliness switch
        {
            "RealTime" => OmmQos.Timelinesses.REALTIME,
            "InexactDelayed" => OmmQos.Timelinesses.INEXACT_DELAYED,
            _ => throw new OmmInvalidConfigurationException("QoS Timeliness: " + timeliness + " not recognized. Acceptable inputs: \"TickByTick\" or \"JustInTimeConflated\".")
        };

        /// <summary>
        /// Returns the QosRate value from the input string.
        /// </summary>
        /// <returns>the rate value of the string </returns>
        public static uint StringToRate(string rate) => rate switch
        {
            "TickByTick" => OmmQos.Rates.TICK_BY_TICK,
            "JustInTimeConflated" => OmmQos.Rates.JUST_IN_TIME_CONFLATED,
            _ => throw new OmmInvalidConfigurationException("QoS Rate: " + rate + " not recognized. Acceptable inputs: \"TickByTick\" or \"JustInTimeConflated\".")
        };

        /// <summary>
        /// Returns the QosRate value from the input string.
        /// </summary>
        /// <returns>the rate value of the string </returns>
        public static int StringToCapability(string capability) => capability switch
        {
            "MMT_LOGIN" => EmaConfig.CapabilitiesEnum.MMT_LOGIN,
            "MMT_DIRECTORY" => EmaConfig.CapabilitiesEnum.MMT_DIRECTORY,
            "MMT_DICTIONARY" => EmaConfig.CapabilitiesEnum.MMT_DICTIONARY,
            "MMT_MARKET_PRICE" => EmaConfig.CapabilitiesEnum.MMT_MARKET_PRICE,
            "MMT_MARKET_BY_ORDER" => EmaConfig.CapabilitiesEnum.MMT_MARKET_BY_ORDER,
            "MMT_MARKET_BY_PRICE" => EmaConfig.CapabilitiesEnum.MMT_MARKET_BY_PRICE,
            "MMT_MARKET_MAKER" => EmaConfig.CapabilitiesEnum.MMT_MARKET_MAKER,
            "MMT_SYMBOL_LIST" => EmaConfig.CapabilitiesEnum.MMT_SYMBOL_LIST,
            "MMT_SERVICE_PROVIDER_STATUS" => EmaConfig.CapabilitiesEnum.MMT_SERVICE_PROVIDER_STATUS,
            "MMT_HISTORY" => EmaConfig.CapabilitiesEnum.MMT_HISTORY,
            "MMT_HEADLINE" => EmaConfig.CapabilitiesEnum.MMT_HEADLINE,
            "MMT_STORY" => EmaConfig.CapabilitiesEnum.MMT_STORY,
            "MMT_REPLAYHEADLINE" => EmaConfig.CapabilitiesEnum.MMT_REPLAYHEADLINE,
            "MMT_REPLAYSTORY" => EmaConfig.CapabilitiesEnum.MMT_REPLAYSTORY,
            "MMT_TRANSACTION" => EmaConfig.CapabilitiesEnum.MMT_TRANSACTION,
            "MMT_YIELD_CURVE" => EmaConfig.CapabilitiesEnum.MMT_YIELD_CURVE,
            "MMT_CONTRIBUTION" => EmaConfig.CapabilitiesEnum.MMT_CONTRIBUTION,
            "MMT_PROVIDER_ADMIN" => EmaConfig.CapabilitiesEnum.MMT_PROVIDER_ADMIN,
            "MMT_ANALYTICS" => EmaConfig.CapabilitiesEnum.MMT_ANALYTICS,
            "MMT_REFERENCE" => EmaConfig.CapabilitiesEnum.MMT_REFERENCE,
            "MMT_NEWS_TEXT_ANALYTICS" => EmaConfig.CapabilitiesEnum.MMT_NEWS_TEXT_ANALYTICS,
            "MMT_SYSTEM" => EmaConfig.CapabilitiesEnum.MMT_SYSTEM,
            _ => throw new OmmInvalidConfigurationException("Capability: " + capability + " not recognized. Acceptable input is either a numeric value or string inputs listed in EmaConfig.CapabilitiesEnum.")
        };

        public static int StringToStreamState(string state) => state switch
        {
            OmmState.OPEN_STRING => OmmState.StreamStates.OPEN,
            OmmState.NONSTREAMING_STRING => OmmState.StreamStates.NON_STREAMING,
            OmmState.CLOSED_STRING => OmmState.StreamStates.CLOSED,
            OmmState.CLOSEDRECOVER_STRING => OmmState.StreamStates.CLOSED_RECOVER,
            OmmState.CLOSEDREDIRECTED_STRING => OmmState.StreamStates.CLOSED_REDIRECTED,
            _ => throw new OmmInvalidConfigurationException("Stream state: " + state + " not recognized. Acceptable inputs: \"Open\", \"NonStreaming\", \"Closed\", \"ClosedRecover\", or \"ClosedRedirected\".")
        };

        public static int StringToDataState(string state) => state switch
        {
            OmmState.NOCHANGE_STRING => OmmState.DataStates.NO_CHANGE,
            OmmState.OK_STRING => OmmState.DataStates.OK,
            OmmState.SUSPECT_STRING => OmmState.DataStates.SUSPECT,
            _ => throw new OmmInvalidConfigurationException("Data state: " + state + " not recognized. Acceptable inputs: \"NoChange\", \"Ok\", or \"Suspect\".")
        };

        public static int StringToStatusCode(string code) => code switch
        {
            OmmState.NONE_STRING => OmmState.StatusCodes.NONE,
            OmmState.NOTFOUND_STRING => OmmState.StatusCodes.NOT_FOUND,
            OmmState.TIMEOUT_STRING => OmmState.StatusCodes.TIMEOUT,
            OmmState.NOTAUTHORIZED_STRING => OmmState.StatusCodes.NOT_AUTHORIZED,
            OmmState.INVALIDARGUMENT_STRING => OmmState.StatusCodes.INVALID_ARGUMENT,
            OmmState.USAGEERROR_STRING => OmmState.StatusCodes.USAGE_ERROR,
            OmmState.PREEMPTED_STRING => OmmState.StatusCodes.PREEMPTED,
            OmmState.JUSTINTIMECONFLATIONSTARTED_STRING => OmmState.StatusCodes.JUST_IN_TIME_CONFLATION_STARTED,
            OmmState.TICKBYTICKRESUMED_STRING => OmmState.StatusCodes.TICK_BY_TICK_RESUMED,
            OmmState.FAILOVERSTARTED_STRING => OmmState.StatusCodes.FAILOVER_STARTED,
            OmmState.FAILOVERCOMPLETED_STRING => OmmState.StatusCodes.FAILOVER_COMPLETED,
            OmmState.GAPDETECTED_STRING => OmmState.StatusCodes.GAP_DETECTED,
            OmmState.NORESOURCES_STRING => OmmState.StatusCodes.NO_RESOURCES,
            OmmState.TOOMANYITEMS_STRING => OmmState.StatusCodes.TOO_MANY_ITEMS,
            OmmState.ALREADYOPEN_STRING => OmmState.StatusCodes.ALREADY_OPEN,
            OmmState.SOURCEUNKNOWN_STRING => OmmState.StatusCodes.SOURCE_UNKNOWN,
            OmmState.NOTOPEN_STRING => OmmState.StatusCodes.NOT_OPEN,
            OmmState.NONUPDATINGITEM_STRING => OmmState.StatusCodes.NON_UPDATING_ITEM,
            OmmState.UNSUPPORTEDVIEWTYPE_STRING => OmmState.StatusCodes.UNSUPPORTED_VIEW_TYPE,
            OmmState.INVALIDVIEW_STRING => OmmState.StatusCodes.INVALID_VIEW,
            OmmState.FULLVIEWPROVIDED_STRING => OmmState.StatusCodes.FULL_VIEW_PROVIDED,
            OmmState.UNABLETOREQUESTASBATCH_STRING => OmmState.StatusCodes.UNABLE_TO_REQUEST_AS_BATCH,
            OmmState.NOBATCHVIEWSUPPORTINREQ_STRING => OmmState.StatusCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ,
            OmmState.EXCEEDEDMAXMOUNTSPERUSER_STRING => OmmState.StatusCodes.EXCEEDED_MAX_MOUNTS_PER_USER,
            OmmState.ERROR_STRING => OmmState.StatusCodes.ERROR,
            OmmState.DACSDOWN_STRING => OmmState.StatusCodes.DACS_DOWN,
            OmmState.USERUNKNOWNTOPERMSYS_STRING => OmmState.StatusCodes.USER_UNKNOWN_TO_PERM_SYS,
            OmmState.DACSMAXLOGINSREACHED_STRING => OmmState.StatusCodes.DACS_MAX_LOGINS_REACHED,
            OmmState.DACSUSERACCESSTOAPPDENIED_STRING => OmmState.StatusCodes.DACS_USER_ACCESS_TO_APP_DENIED,
            OmmState.GAPFILL_STRING => OmmState.StatusCodes.GAP_FILL,
            OmmState.APPAUTHORIZATIONFAILED_STRING => OmmState.StatusCodes.APP_AUTHORIZATION_FAILED,
            _ => throw new OmmInvalidConfigurationException("Status code: " + code +
                                                            " not recognized. Acceptable inputs are the strings listed in OmmState.cs.")
        };
    }
}