/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Example.Common;

namespace LSEG.Eta.ValuedAdd.Tests
{
    internal class ClientComponentTest : TestComponent
    {
        public LoginRequest LoginRequestMsg { get; set; }
        public DirectoryRequest DirectoryRequestMsg { get; set; }
        public DictionaryRequest FieldDictionaryRequestMsg { get; set; } = new DictionaryRequest();
        public DictionaryRequest EnumTypeDictionaryRequestMsg { get; set; } = new DictionaryRequest();
        public MarketPriceRequest MarketPriceRequestMsg { get; set; } = new MarketPriceRequest();
        public string DefaultMarketPriceRequestName { get; set; } = "TRI.N";
        public LoginRequest CustomLoginRequestMsg { get; set; }

        public int NumMarketPriceRefresh { get; set; }

        public ClientComponentTest()
        {
            CheckLoginMsg = msg => { Assert.Equal(LoginMsgType.REFRESH, msg.LoginMsgType); Assert.NotNull(msg.LoginRefresh); };
            CheckDirectoryMsg = msg => { Assert.Equal(DirectoryMsgType.REFRESH, msg.DirectoryMsgType); Assert.NotNull(msg.DirectoryRefresh); };
            CheckDictionaryMsg = msg => { Assert.Equal(DictionaryMsgType.REFRESH, msg.DictionaryMsgType); Assert.NotNull(msg.DictionaryRefresh); };
        }

        public void InitDefaultTestDictionaryRequestMsgs(int fdStreamId, int etStreamId, int serviceId, string fdName, string etName, int verbosity)
        {
            FieldDictionaryRequestMsg.Clear();
            FieldDictionaryRequestMsg.DictionaryName.Data(fdName);
            FieldDictionaryRequestMsg.StreamId = fdStreamId;
            FieldDictionaryRequestMsg.ServiceId = serviceId;
            FieldDictionaryRequestMsg.Verbosity = verbosity;

            EnumTypeDictionaryRequestMsg.Clear();
            EnumTypeDictionaryRequestMsg.DictionaryName.Data(etName);
            EnumTypeDictionaryRequestMsg.StreamId = etStreamId;
            EnumTypeDictionaryRequestMsg.ServiceId = serviceId;
            EnumTypeDictionaryRequestMsg.Verbosity = verbosity;
        }

        public void InitDefaultTestDirectoryRequest(int streamId, int filterToRequest)
        {
            DirectoryRequestMsg = new DirectoryRequest();
            DirectoryRequestMsg.StreamId = streamId;
            DirectoryRequestMsg.Filter = filterToRequest;
            DirectoryRequestMsg.Streaming = true;
        }

        public void InitTestLoginRequest(int streamId, string appName, string appId, string userName, bool enableRtt)
        {
            LoginRequestMsg = new LoginRequest();
            LoginRequestMsg.InitDefaultRequest(streamId);

            if (userName != null && !userName.Equals(string.Empty))
            {
                LoginRequestMsg.UserName.Data(userName);
            }

            if (appId != null && !appId.Equals(string.Empty))
            {
                LoginRequestMsg.HasAttrib = true;
                LoginRequestMsg.LoginAttrib.HasApplicationId = true;
                LoginRequestMsg.LoginAttrib.ApplicationId.Data(appId);
            }

            if (appName != null && !appName.Equals(string.Empty))
            {
                LoginRequestMsg.HasAttrib = true;
                LoginRequestMsg.LoginAttrib.HasApplicationName = true;
                LoginRequestMsg.LoginAttrib.ApplicationName.Data(appName);
            }

            LoginRequestMsg.LoginAttrib.HasSupportRoundTripLatencyMonitoring = enableRtt;
        }

        public void InitDefaultTestMarketPriceRequest(int serviceId, int streamId, string name)
        {
            MarketPriceRequestMsg.Clear();
            MarketPriceRequestMsg.Streaming = true;
            MarketPriceRequestMsg.StreamId = streamId;
            MarketPriceRequestMsg.HasServiceId = true;
            MarketPriceRequestMsg.ServiceId = serviceId;
            MarketPriceRequestMsg.HasPriority = true;
            MarketPriceRequestMsg.PriorityClass = 1;
            MarketPriceRequestMsg.PriorityCount = 1;
            MarketPriceRequestMsg.ItemNames.Add(name == null ? DefaultMarketPriceRequestName : name);
        }
    }
}
