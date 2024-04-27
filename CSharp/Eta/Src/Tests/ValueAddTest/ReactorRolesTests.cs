/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using Xunit;
using Xunit.Categories;
using static LSEG.Eta.Rdm.Dictionary;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class ReactorRolesTests
    {
        private void VerifyLoginRequest(LoginRequest loginRequest, int role, int streamId)
        {
            Assert.True(loginRequest.HasRole);
            Assert.Equal(role, loginRequest.Role);
            Assert.Equal(streamId, loginRequest.StreamId);
        }
        private void VerifyDirectoryRefresh(DirectoryRefresh directoryRefresh, int streamId, string serviceName)
        {
            Assert.True(directoryRefresh.ClearCache);
            Assert.False(directoryRefresh.Solicited);
            Assert.Equal(NIProviderRole.FILTER_TO_REFRESH, directoryRefresh.Filter);
            Assert.Equal(streamId, directoryRefresh.StreamId);
            Assert.Single(directoryRefresh.ServiceList);
            Assert.Equal(directoryRefresh.ServiceList[0].Info.ServiceName.ToString(), serviceName);
        }

        private void VerifyDirectoryRequest(DirectoryRequest directoryRequest, int streamId)
        {
            Assert.True(directoryRequest.Streaming);
            Assert.Equal(ConsumerRole.FILTER_TO_REQUEST, directoryRequest.Filter);
            Assert.Equal(streamId, directoryRequest.StreamId);
        }

        private void VerifyDictionaryRequest(DictionaryRequest dictionaryRequest, int streamId, String name)
        {
            Assert.True(dictionaryRequest.Streaming);
            Assert.Equal(VerbosityValues.NORMAL, dictionaryRequest.Verbosity);
            Assert.Equal(streamId, dictionaryRequest.StreamId);
            Assert.Equal(dictionaryRequest.DictionaryName.ToString(), name);
        }

        [Fact]
        [Category("Unit")]
        [Category("ReactorRoles")]
        public void CreateReactorRolesDefaultTest()
        {
            ConsumerRole consumerRole = new ConsumerRole();
            ProviderRole providerRole = new ProviderRole();
            NIProviderRole niProviderRole = new NIProviderRole();

            Assert.Equal(ReactorRoleType.CONSUMER, consumerRole.Type);
            Assert.Equal(ReactorRoleType.PROVIDER, providerRole.Type);
            Assert.Equal(ReactorRoleType.NIPROVIDER, niProviderRole.Type);

            Assert.Equal(DictionaryDownloadMode.NONE, consumerRole.DictionaryDownloadMode);
            consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.FIRST_AVAILABLE;
            Assert.Equal(DictionaryDownloadMode.FIRST_AVAILABLE, consumerRole.DictionaryDownloadMode);
            consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.NONE;
            Assert.Equal(DictionaryDownloadMode.NONE, consumerRole.DictionaryDownloadMode);

            consumerRole.InitDefaultRDMLoginRequest();
            Assert.NotNull(consumerRole.RdmLoginRequest);
            VerifyLoginRequest(consumerRole.RdmLoginRequest, Login.RoleTypes.CONS, ConsumerRole.LOGIN_STREAM_ID);

            consumerRole.InitDefaultRDMDirectoryRequest();
            Assert.NotNull(consumerRole.RdmDirectoryRequest);
            VerifyDirectoryRequest(consumerRole.RdmDirectoryRequest, ConsumerRole.DIRECTORY_STREAM_ID);

            consumerRole.InitDefaultRDMFieldDictionaryRequest();
            Assert.NotNull(consumerRole.RdmFieldDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmFieldDictionaryRequest, ConsumerRole.FIELD_DICTIONARY_STREAM_ID, "RWFFld");

            consumerRole.InitDefaultRDMEnumDictionaryRequest();
            Assert.NotNull(consumerRole.RdmEnumTypeDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmEnumTypeDictionaryRequest, ConsumerRole.ENUM_DICTIONARY_STREAM_ID, "RWFEnum");

            niProviderRole.InitDefaultRDMLoginRequest();
            Assert.NotNull(niProviderRole.RdmLoginRequest);
            VerifyLoginRequest(niProviderRole.RdmLoginRequest, Login.RoleTypes.PROV, NIProviderRole.LOGIN_STREAM_ID);

            niProviderRole.InitDefaultRDMDirectoryRefresh("DIRECT_FEED", 1);
            Assert.NotNull(niProviderRole.RdmDirectoryRefresh);
            VerifyDirectoryRefresh(niProviderRole.RdmDirectoryRefresh, NIProviderRole.DIRECTORY_STREAM_ID, "DIRECT_FEED");

            // test callbacks in ConsumerRole, ProviderRole and NIProviderRole
            TestComponent consumerCallbackHandler = new TestComponent();
            TestComponent providerCallbackHandler = new TestComponent();
            TestComponent niproviderCallbackHandler = new TestComponent();
            ReactorChannelEvent reactorChannelEvent = new ReactorChannelEvent();
            ReactorMsgEvent reactorMsgEvent = new ReactorMsgEvent();
            reactorChannelEvent.EventType = ReactorChannelEventType.CHANNEL_UP;
            RDMLoginMsgEvent rdmLoginMsgEvent = new RDMLoginMsgEvent();
            RDMDirectoryMsgEvent rdmDirectoryMsgEvent = new RDMDirectoryMsgEvent();
            RDMDictionaryMsgEvent rdmDictionaryMsgEvent = new RDMDictionaryMsgEvent();

            // ConsumerRole callback test
            consumerRole.ChannelEventCallback = consumerCallbackHandler;
            Assert.Equal(consumerCallbackHandler, consumerRole.ChannelEventCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, consumerRole.ChannelEventCallback.ReactorChannelEventCallback(reactorChannelEvent));
            Assert.Equal(1, consumerCallbackHandler.NumChannelUpEvent);

            consumerRole.DefaultMsgCallback = consumerCallbackHandler;
            Assert.Equal(consumerCallbackHandler, consumerRole.DefaultMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, consumerRole.DefaultMsgCallback.DefaultMsgCallback(reactorMsgEvent));
            Assert.Equal(1, consumerCallbackHandler.NumDefaultMsgEvent);

            consumerRole.LoginMsgCallback = consumerCallbackHandler;
            Assert.Equal(consumerCallbackHandler, consumerRole.LoginMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, consumerRole.LoginMsgCallback.RdmLoginMsgCallback(rdmLoginMsgEvent));
            Assert.Equal(1, consumerCallbackHandler.NumLoginMsgEvent); 

            consumerRole.DirectoryMsgCallback = consumerCallbackHandler;
            Assert.Equal(consumerCallbackHandler, consumerRole.DirectoryMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, consumerRole.DirectoryMsgCallback.RdmDirectoryMsgCallback(rdmDirectoryMsgEvent));
            Assert.Equal(1, consumerCallbackHandler.NumDirectoryMsgEvent);

            consumerRole.DictionaryMsgCallback = consumerCallbackHandler;
            Assert.Equal(consumerCallbackHandler, consumerRole.DictionaryMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, consumerRole.DictionaryMsgCallback.RdmDictionaryMsgCallback(rdmDictionaryMsgEvent));
            Assert.Equal(1, consumerCallbackHandler.NumDictionaryMsgEvent);

            // ProviderRole callback test
            providerRole.ChannelEventCallback = providerCallbackHandler;
            Assert.Equal(providerCallbackHandler, providerRole.ChannelEventCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, providerRole.ChannelEventCallback.ReactorChannelEventCallback(reactorChannelEvent));
            Assert.Equal(1, providerCallbackHandler.NumChannelUpEvent);

            providerRole.DefaultMsgCallback = providerCallbackHandler;
            Assert.Equal(providerCallbackHandler, providerRole.DefaultMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, providerRole.DefaultMsgCallback.DefaultMsgCallback(reactorMsgEvent));
            Assert.Equal(1, providerCallbackHandler.NumDefaultMsgEvent);
 
            providerRole.LoginMsgCallback = providerCallbackHandler;
            Assert.Equal(providerCallbackHandler, providerRole.LoginMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, providerRole.LoginMsgCallback.RdmLoginMsgCallback(rdmLoginMsgEvent));
            Assert.Equal(1, providerCallbackHandler.NumLoginMsgEvent); 

            providerRole.DirectoryMsgCallback = providerCallbackHandler;
            Assert.Equal(providerCallbackHandler, providerRole.DirectoryMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, providerRole.DirectoryMsgCallback.RdmDirectoryMsgCallback(rdmDirectoryMsgEvent));
            Assert.Equal(1, providerCallbackHandler.NumDirectoryMsgEvent);

            providerRole.DictionaryMsgCallback = providerCallbackHandler;
            Assert.Equal(providerCallbackHandler, providerRole.DictionaryMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, providerRole.DictionaryMsgCallback.RdmDictionaryMsgCallback(rdmDictionaryMsgEvent));
            Assert.Equal(1, providerCallbackHandler.NumDictionaryMsgEvent);

            // NIProviderRole callback test
            niProviderRole.ChannelEventCallback = niproviderCallbackHandler;
            Assert.Equal(niproviderCallbackHandler, niProviderRole.ChannelEventCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, niProviderRole.ChannelEventCallback.ReactorChannelEventCallback(reactorChannelEvent));
            Assert.Equal(1, niproviderCallbackHandler.NumChannelUpEvent);

            niProviderRole.DefaultMsgCallback = niproviderCallbackHandler;
            Assert.Equal(niproviderCallbackHandler, niProviderRole.DefaultMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, niProviderRole.DefaultMsgCallback.DefaultMsgCallback(reactorMsgEvent));
            Assert.Equal(1, niproviderCallbackHandler.NumDefaultMsgEvent);

            niProviderRole.LoginMsgCallback = niproviderCallbackHandler;
            Assert.Equal(niproviderCallbackHandler, niProviderRole.LoginMsgCallback);
            Assert.Equal(ReactorCallbackReturnCode.SUCCESS, niProviderRole.LoginMsgCallback.RdmLoginMsgCallback(rdmLoginMsgEvent));
            Assert.Equal(1, niproviderCallbackHandler.NumLoginMsgEvent); 
        }

        [Fact]
        [Category("Unit")]
        [Category("ReactorRoles")]
        public void CreateReactorRolesUserProvidedTest()
        {
            ConsumerRole consumerRole = new ConsumerRole();
            ProviderRole providerRole = new ProviderRole();
            NIProviderRole niProviderRole = new NIProviderRole();

            Assert.Equal(ReactorRoleType.CONSUMER, consumerRole.Type);
            Assert.Equal(ReactorRoleType.PROVIDER, providerRole.Type);
            Assert.Equal(ReactorRoleType.NIPROVIDER, niProviderRole.Type);

            Assert.Equal(DictionaryDownloadMode.NONE, consumerRole.DictionaryDownloadMode);
            consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.FIRST_AVAILABLE;
            Assert.Equal(DictionaryDownloadMode.FIRST_AVAILABLE, consumerRole.DictionaryDownloadMode);
            consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.NONE;
            Assert.Equal(DictionaryDownloadMode.NONE, consumerRole.DictionaryDownloadMode);

            // verify user provided login request and stream id
            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(11);
 
            consumerRole.RdmLoginRequest = loginRequest;
            consumerRole.InitDefaultRDMLoginRequest(); 
            Assert.NotNull(consumerRole.RdmLoginRequest);
            VerifyLoginRequest(consumerRole.RdmLoginRequest, Login.RoleTypes.CONS, 11);
            // verify user provided stream id equals 0 is overridden to default value
            loginRequest.InitDefaultRequest(0);
            consumerRole.RdmLoginRequest = loginRequest;
            consumerRole.InitDefaultRDMLoginRequest();
            Assert.NotNull(consumerRole.RdmLoginRequest);
            VerifyLoginRequest(consumerRole.RdmLoginRequest, Login.RoleTypes.CONS, ConsumerRole.LOGIN_STREAM_ID); 


            // verify user provided directory request and stream id
            DirectoryRequest directoryRequest = new DirectoryRequest();
            directoryRequest.StreamId = 22;
            consumerRole.RdmDirectoryRequest = directoryRequest;
            consumerRole.InitDefaultRDMDirectoryRequest();
            Assert.NotNull(consumerRole.RdmDirectoryRequest);
            VerifyDirectoryRequest(consumerRole.RdmDirectoryRequest, 22);
            
            // verify user provided stream id equals 0 is overridden to default value
            directoryRequest.StreamId = 0;
            consumerRole.RdmDirectoryRequest = directoryRequest;
            consumerRole.InitDefaultRDMDirectoryRequest();
            Assert.NotNull(consumerRole.RdmDirectoryRequest);
            VerifyDirectoryRequest(consumerRole.RdmDirectoryRequest, ConsumerRole.DIRECTORY_STREAM_ID);

            // verify user provided field dictionary request and stream id
            DictionaryRequest dictionaryRequest = new DictionaryRequest();
            dictionaryRequest.StreamId = 33;
            consumerRole.RdmFieldDictionaryRequest = dictionaryRequest;
            consumerRole.InitDefaultRDMFieldDictionaryRequest();
            Assert.NotNull(consumerRole.RdmFieldDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmFieldDictionaryRequest, 33, "RWFFld");
            
            // verify user provided stream id equals 0 is overridden to default value
            dictionaryRequest.StreamId = 0;
            consumerRole.RdmFieldDictionaryRequest = dictionaryRequest;
            consumerRole.InitDefaultRDMFieldDictionaryRequest();
            Assert.NotNull(consumerRole.RdmFieldDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmFieldDictionaryRequest, ConsumerRole.FIELD_DICTIONARY_STREAM_ID, "RWFFld");
            
            // verify user provided stream id that collides with directory id is overridden to one plus directory id
            dictionaryRequest.StreamId = ConsumerRole.DIRECTORY_STREAM_ID;
            consumerRole.RdmFieldDictionaryRequest = dictionaryRequest;
            consumerRole.InitDefaultRDMFieldDictionaryRequest();
            Assert.NotNull(consumerRole.RdmFieldDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmFieldDictionaryRequest, ConsumerRole.DIRECTORY_STREAM_ID + 1, "RWFFld");

            // verify user provided enum type dictionary request and stream id
            dictionaryRequest = new DictionaryRequest();
            dictionaryRequest.StreamId = 44;
            consumerRole.RdmEnumTypeDictionaryRequest = dictionaryRequest;
            consumerRole.InitDefaultRDMEnumDictionaryRequest();
            Assert.NotNull(consumerRole.RdmEnumTypeDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmEnumTypeDictionaryRequest, 44, "RWFEnum");
            
            // verify user provided stream id equals 0 is overridden to default value
            dictionaryRequest.StreamId = 0;
            consumerRole.RdmEnumTypeDictionaryRequest = dictionaryRequest;
            consumerRole.InitDefaultRDMEnumDictionaryRequest();
            Assert.NotNull(consumerRole.RdmEnumTypeDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmEnumTypeDictionaryRequest, ConsumerRole.ENUM_DICTIONARY_STREAM_ID, "RWFEnum");
            
            // verify user provided stream id that collides with field dictionary id is overridden to one plus field dictionary id
            dictionaryRequest.StreamId = ConsumerRole.FIELD_DICTIONARY_STREAM_ID;
            consumerRole.RdmEnumTypeDictionaryRequest = dictionaryRequest;
            consumerRole.InitDefaultRDMEnumDictionaryRequest();
            Assert.NotNull(consumerRole.RdmEnumTypeDictionaryRequest);
            VerifyDictionaryRequest(consumerRole.RdmEnumTypeDictionaryRequest, ConsumerRole.FIELD_DICTIONARY_STREAM_ID + 1, "RWFEnum");

            // verify user provided login request and stream id
            loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(55);

            niProviderRole.RdmLoginRequest = loginRequest;
            niProviderRole.InitDefaultRDMLoginRequest();
            Assert.NotNull(niProviderRole.RdmLoginRequest);
            VerifyLoginRequest(niProviderRole.RdmLoginRequest, Login.RoleTypes.PROV, 55);
            // verify user provided stream id equals 0 is overridden to default value
            loginRequest.InitDefaultRequest(0);
            niProviderRole.RdmLoginRequest = loginRequest;
            niProviderRole.InitDefaultRDMLoginRequest();
            Assert.NotNull(niProviderRole.RdmLoginRequest);
            VerifyLoginRequest(niProviderRole.RdmLoginRequest, Login.RoleTypes.PROV, NIProviderRole.LOGIN_STREAM_ID); 


            // verify user provided directory refresh and stream id
            DirectoryRefresh directoryRefresh = new DirectoryRefresh();
            directoryRefresh.StreamId = 66;
            niProviderRole.RdmDirectoryRefresh = directoryRefresh;
            niProviderRole.InitDefaultRDMDirectoryRefresh("DIRECT_FEED", 1);
            Assert.NotNull(niProviderRole.RdmDirectoryRefresh);
            VerifyDirectoryRefresh(niProviderRole.RdmDirectoryRefresh, 66, "DIRECT_FEED");
            // verify user provided stream id equals 0 is overridden to default value
            directoryRefresh.StreamId = 0;
            niProviderRole.RdmDirectoryRefresh = directoryRefresh;
            niProviderRole.InitDefaultRDMDirectoryRefresh("DIRECT_FEED", 1);
            Assert.NotNull(niProviderRole.RdmDirectoryRefresh);
            VerifyDirectoryRefresh(niProviderRole.RdmDirectoryRefresh, NIProviderRole.DIRECTORY_STREAM_ID, "DIRECT_FEED");            
        }
    }
}
