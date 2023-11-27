/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.PerfTools.Common
{
    public class EmaDirectoryHandler : IOmmConsumerClient
    {
        internal class ServiceState
        {
            internal int StreamState;
            internal int DataState;
        }

        private const long FILTER_TO_REQUEST = EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER | EmaRdm.SERVICE_GROUP_FILTER;

        private string? _serviceName;    // requested service
        private int _serviceId;  //requested service id
        private ServiceState _state;            // requested service state
        private RequestMsg _directoryRequest = new RequestMsg();
        private bool _acceptRequest;

        /// <summary>
        /// Instantiates a new directory handler.
        /// </summary>
        public EmaDirectoryHandler()
        {
            _state = new ServiceState();
        }

        /**
         * Sets the service name requested by the application.
         * 
         * @param servicename - The service name requested by the application
         */
        /// <summary>
        /// 
        /// </summary>
        /// <param name="servicename"></param>
        public void ServiceName(string? servicename)
        {
            _serviceName = servicename;
        }

        /// <summary>
        /// Checks if is requested service up.
        /// </summary>
        /// <returns>true if service requested by application is up, false if not.</returns>
        public bool IsRequestedServiceUp()
        {
            return _acceptRequest && _state.StreamState == OmmState.StreamStates.OPEN && _state.DataState == OmmState.DataStates.OK;
        }

        /**
         * Service id.
         *
         * @return service id associated with the service name requested by application.
         */
        public int ServiceId()
        {
            return _serviceId;
        }

        /// <summary>
        /// Sends a source directory request to a channel. This consists of getting a message buffer, 
        /// encoding the source directory request, and sending the source directory request to the server.
        /// </summary>
        /// <returns>the request message</returns>
        public RequestMsg GetRequest()
        {
            //initialize directory _state
            //this will be updated as refresh and status messages are received
            _state.DataState = OmmState.DataStates.NO_CHANGE;
            _state.StreamState = OmmState.StreamStates.CLOSED;

            _directoryRequest.Clear();
            _directoryRequest.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName(_serviceName!).Filter(FILTER_TO_REQUEST).InterestAfterRefresh(true);

            return _directoryRequest;
        }

        private void Decode(Map map)
        {
            FilterList filterList;
            bool findMapEntry = false;

            foreach (MapEntry mapEntry in map)
            {
                if (mapEntry.LoadType == DataTypes.FILTER_LIST)
                {
                    filterList = mapEntry.FilterList();
                    foreach (FilterEntry filterEntry in filterList)
                    {
                        if (filterEntry.LoadType == DataTypes.ELEMENT_LIST)
                        {
                            if (Decode(filterEntry.ElementList()))
                            {
                                _serviceId = (int)mapEntry.Key.UInt();
                                findMapEntry = true;
                            }
                        }
                    }
                }

                if (findMapEntry)
                    return;
            }
        }

        private bool Decode(ElementList elementList)
        {
            bool foundService = false;
            foreach (ElementEntry elementEntry in elementList)
            {
                if (Data.DataCode.BLANK != elementEntry.Code)
                {
                    switch (elementEntry.LoadType)
                    {
                        case DataTypes.ASCII:
                            if (elementEntry.Name.Equals("Name") && elementEntry.OmmAsciiValue().Value.Equals(_serviceName))
                                foundService = true;
                            break;
                        case DataTypes.UINT:
                            if (elementEntry.Name.Equals("AcceptingRequests"))
                            {
                                if (elementEntry.UIntValue() == 1)
                                    _acceptRequest = true;
                                else
                                    _acceptRequest = false;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            return foundService;
        }

        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
        {
            Console.WriteLine("Received Source Directory Refresh\n");

            _state.DataState = refreshMsg.State().DataState;
            _state.StreamState = refreshMsg.State().StreamState;

            if (refreshMsg.Payload().DataType != DataTypes.MAP)
                Console.WriteLine("Received invalid source directory refresh msg.");

            else
                Decode(refreshMsg.Payload().Map());

            if (!IsRequestedServiceUp())
                Console.WriteLine("Requested service '" + _serviceName + "' not up. Waiting for service to be up...");
        }

        public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
        {
            Console.WriteLine("Received Source Directory Update\n");

            if (updateMsg.Payload().DataType != DataTypes.MAP)
                Console.WriteLine("Received invalid source directory update msg.");
            else
                Decode(updateMsg.Payload().Map());
        }

        public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
        {
            Console.WriteLine("Received Source Directory Status\n");

            Console.WriteLine(statusMsg.ToString());

            if (statusMsg.HasState)
            {
                _state.DataState = statusMsg.State().DataState;
                _state.StreamState = statusMsg.State().StreamState;
            }
        }

        public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent) { }

        public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent) { }

        public void OnAllMsg(Msg msg, IOmmConsumerEvent consumerEvent) { }
    }
}
