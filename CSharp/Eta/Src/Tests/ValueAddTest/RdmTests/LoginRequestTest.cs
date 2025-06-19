/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

using Xunit;
using Xunit.Categories;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;

using static LSEG.Eta.Rdm.Login;

using Buffer = LSEG.Eta.Codec.Buffer;
using System.Diagnostics;

namespace LSEG.Eta.ValuedAdd.Tests
{

    /// Unit-tests for LoginRequest class.
    ///
    [Collection("ValueAdded")]
    public class LoginRequestTest
    {
        private DecodeIterator dIter = new();
        private EncodeIterator encIter = new();
        private Msg msg = new();
        private ElementList elementList = new();
        private ElementEntry element = new();
        private Buffer zeroLengthBuf = new();

        // calculate  the sum of all the flags for each message class ( 2 * max_flag - 1)
        private const LoginStatusFlags allStatusMsgFlags = (LoginStatusFlags)(2 * (int)LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT - 1);
        // The HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD = 0x0100 flag is currently skipped in the RefreshMsgFlags
        private const LoginRefreshFlags allRefreshMsgFlags = (LoginRefreshFlags)(2 * (int)LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_TEXT - 1 - 0x0010);

        #region Login Request

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRequestClearTest()
        {
            LoginRequest reqRDMMsg = new();

            reqRDMMsg.Clear();

            Console.WriteLine("LoginRequest clear test...");

            int streamId = -5;
            long allowSuspectData = 2;
            string userName = "userName";
            string applicationId = "applicationId";
            string applicationName = "applicationName";
            string position = "position";
            long providePermissionProfile = 2;
            long providePermissionExpressions = 2;
            string instanceId = "instanceId";
            string password = "password";
            long singleOpen = 1;
            var userNameType = UserIdTypes.NAME;
            long downloadConnectionConfig = 2;
            long role = 1;
            string authenticationExtended = "authenticationExtended";

            reqRDMMsg.StreamId = streamId;
            reqRDMMsg.UserName.Data(userName);
            reqRDMMsg.HasUserNameType = true;
            reqRDMMsg.UserNameType = userNameType;

            reqRDMMsg.HasAttrib = true;
            reqRDMMsg.LoginAttrib.HasAllowSuspectData = true;
            reqRDMMsg.LoginAttrib.AllowSuspectData = allowSuspectData;
            reqRDMMsg.LoginAttrib.HasApplicationId = true;
            reqRDMMsg.LoginAttrib.ApplicationId.Data(applicationId);
            reqRDMMsg.LoginAttrib.HasApplicationName = true;
            reqRDMMsg.LoginAttrib.ApplicationName.Data(applicationName);
            reqRDMMsg.HasDownloadConnectionConfig = true;
            reqRDMMsg.DownloadConnectionConfig = downloadConnectionConfig;
            reqRDMMsg.HasInstanceId = true;
            reqRDMMsg.InstanceId.Data(instanceId);
            reqRDMMsg.HasPassword = true;
            reqRDMMsg.Password.Data(password);
            reqRDMMsg.LoginAttrib.HasPosition = true;
            reqRDMMsg.LoginAttrib.Position.Data(position);
            reqRDMMsg.LoginAttrib.HasProvidePermissionExpressions = true;
            reqRDMMsg.LoginAttrib.ProvidePermissionExpressions = providePermissionExpressions;
            reqRDMMsg.LoginAttrib.HasProvidePermissionProfile = true;
            reqRDMMsg.LoginAttrib.ProvidePermissionProfile = providePermissionProfile;
            reqRDMMsg.HasRole = true;
            reqRDMMsg.Role = role;
            reqRDMMsg.LoginAttrib.HasSingleOpen = true;
            reqRDMMsg.LoginAttrib.SingleOpen = singleOpen;
            reqRDMMsg.HasAuthenticationExtended = true;
            reqRDMMsg.AuthenticationExtended.Data(authenticationExtended);

            Assert.True(reqRDMMsg.Flags != 0);
            Assert.True(reqRDMMsg.LoginAttrib.Flags != 0);

            reqRDMMsg.Clear();

            Assert.Equal(LoginRequestFlags.NONE, reqRDMMsg.Flags);
            Assert.Equal(0, (int)reqRDMMsg.LoginAttrib.Flags);

            Assert.Equal(RoleTypes.CONS, reqRDMMsg.Role);
            Assert.Equal(0, reqRDMMsg.DownloadConnectionConfig);
            Assert.Equal(1, reqRDMMsg.LoginAttrib.ProvidePermissionProfile);
            Assert.Equal(1, reqRDMMsg.LoginAttrib.ProvidePermissionExpressions);
            Assert.Equal(1, reqRDMMsg.LoginAttrib.SingleOpen);
            Assert.Equal(1, reqRDMMsg.LoginAttrib.AllowSuspectData);
            Assert.Equal(0, reqRDMMsg.LoginAttrib.SupportProviderDictionaryDownload);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRequestCopyTest()
        {
            LoginRequest reqRDMMsg1 = new();
            LoginRequest reqRDMMsg2 = new();
            //reqRDMMsg1.rdmMsgType(LoginMsgType.REQUEST);
            //reqRDMMsg2.rdmMsgType(LoginMsgType.REQUEST);

            Console.WriteLine("LoginRequest copy test...");

            int streamId = -5;
            long allowSuspectData = 2;
            string userName = "userName";
            string applicationId = "applicationId";
            string applicationName = "applicationName";
            string position = "position";
            long providePermissionProfile = 2;
            long providePermissionExpressions = 2;
            string instanceId = "instanceId";
            string password = "password";
            long singleOpen = 1;
            var userNameType = UserIdTypes.NAME;
            long downloadConnectionConfig = 2;
            long role = 1;
            string authenticationExtended = "authenticationExtended";

            reqRDMMsg1.StreamId = streamId;
            reqRDMMsg1.UserName.Data(userName);
            reqRDMMsg1.HasUserNameType = true;
            reqRDMMsg1.UserNameType = userNameType;

            reqRDMMsg1.HasAttrib = true;
            reqRDMMsg1.LoginAttrib.HasAllowSuspectData = true;
            reqRDMMsg1.LoginAttrib.AllowSuspectData = allowSuspectData;
            reqRDMMsg1.LoginAttrib.HasApplicationId = true;
            reqRDMMsg1.LoginAttrib.ApplicationId.Data(applicationId);
            reqRDMMsg1.LoginAttrib.HasApplicationName = true;
            reqRDMMsg1.LoginAttrib.ApplicationName.Data(applicationName);
            reqRDMMsg1.HasDownloadConnectionConfig = true;
            reqRDMMsg1.DownloadConnectionConfig = downloadConnectionConfig;
            reqRDMMsg1.HasInstanceId = true;
            reqRDMMsg1.InstanceId.Data(instanceId);
            reqRDMMsg1.HasPassword = true;
            reqRDMMsg1.Password.Data(password);
            reqRDMMsg1.LoginAttrib.HasPosition = true;
            reqRDMMsg1.LoginAttrib.Position.Data(position);
            reqRDMMsg1.LoginAttrib.HasProvidePermissionExpressions = true;
            reqRDMMsg1.LoginAttrib.ProvidePermissionExpressions = providePermissionExpressions;
            reqRDMMsg1.LoginAttrib.HasProvidePermissionProfile = true;
            reqRDMMsg1.LoginAttrib.ProvidePermissionProfile = providePermissionProfile;
            reqRDMMsg1.HasRole = true;
            reqRDMMsg1.Role = role;
            reqRDMMsg1.LoginAttrib.HasSingleOpen = true;
            reqRDMMsg1.LoginAttrib.SingleOpen = singleOpen;
            reqRDMMsg1.HasAuthenticationExtended = true;
            reqRDMMsg1.AuthenticationExtended.Data(authenticationExtended);

            // deep copy
            var ret = reqRDMMsg1.Copy(reqRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // verify deep copy
            Assert.Equal(reqRDMMsg1.Flags, reqRDMMsg2.Flags);
            Assert.Equal(reqRDMMsg1.LoginAttrib.AllowSuspectData, reqRDMMsg2.LoginAttrib.AllowSuspectData);

            Assert.True(reqRDMMsg1.LoginAttrib.ApplicationId != reqRDMMsg2.LoginAttrib.ApplicationId);
            Assert.Equal(reqRDMMsg1.LoginAttrib.ApplicationId.ToString(), reqRDMMsg2.LoginAttrib.ApplicationId.ToString());

            Assert.True(reqRDMMsg1.LoginAttrib.ApplicationName != reqRDMMsg2.LoginAttrib.ApplicationName);
            Assert.Equal(reqRDMMsg1.LoginAttrib.ApplicationName.ToString(), reqRDMMsg2.LoginAttrib.ApplicationName.ToString());

            Assert.Equal(reqRDMMsg1.DownloadConnectionConfig, reqRDMMsg2.DownloadConnectionConfig);

            Assert.True(reqRDMMsg1.InstanceId != reqRDMMsg2.InstanceId);
            Assert.Equal(reqRDMMsg1.InstanceId.ToString(), reqRDMMsg2.InstanceId.ToString());

            Assert.True(reqRDMMsg1.Password != reqRDMMsg2.Password);
            Assert.Equal(reqRDMMsg1.Password.ToString(), reqRDMMsg2.Password.ToString());

            Assert.True(reqRDMMsg1.LoginAttrib.Position != reqRDMMsg2.LoginAttrib.Position);
            Assert.Equal(reqRDMMsg1.LoginAttrib.Position.ToString(), reqRDMMsg2.LoginAttrib.Position.ToString());

            Assert.Equal(reqRDMMsg1.LoginAttrib.ProvidePermissionExpressions, reqRDMMsg2.LoginAttrib.ProvidePermissionExpressions);

            Assert.Equal(reqRDMMsg1.LoginAttrib.ProvidePermissionProfile, reqRDMMsg2.LoginAttrib.ProvidePermissionProfile);

            Assert.Equal(reqRDMMsg1.Role, reqRDMMsg2.Role);

            Assert.Equal(reqRDMMsg1.LoginAttrib.SingleOpen, reqRDMMsg2.LoginAttrib.SingleOpen);

            Assert.Equal(reqRDMMsg1.StreamId, reqRDMMsg2.StreamId);

            Assert.True(reqRDMMsg1.UserName != reqRDMMsg2.UserName);
            Assert.Equal(reqRDMMsg1.UserName.ToString(), reqRDMMsg2.UserName.ToString());

            Assert.Equal(reqRDMMsg1.UserNameType, reqRDMMsg2.UserNameType);

            Assert.True(reqRDMMsg1.AuthenticationExtended != reqRDMMsg2.AuthenticationExtended);
            Assert.Equal(reqRDMMsg1.AuthenticationExtended.ToString(), reqRDMMsg2.AuthenticationExtended.ToString());

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRequestBlankTest()
        {
            LoginRequest requestEnc = new();
            //requestEnc.rdmMsgType(LoginMsgType.REQUEST);
            LoginRequest requestDec = new();
            //requestDec.rdmMsgType(LoginMsgType.REQUEST);
            zeroLengthBuf.Data("");

            Console.WriteLine("LoginRequest blank test...");
            // Test 1: Encode side
            //parameters setup
            int streamId = -5;

            Buffer name = new();
            name.Data("user");

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            //status msg setup
            //int flags = 0;
            requestEnc.Clear();
            requestEnc.Flags = 0;
            requestEnc.StreamId = streamId;

            requestEnc.UserName = zeroLengthBuf;
            requestEnc.HasUserNameType = true;
            requestEnc.UserNameType = UserIdTypes.AUTHN_TOKEN;

            requestEnc.HasPassword = true;
            requestEnc.Password.Data("");
            requestEnc.HasInstanceId = true;
            requestEnc.InstanceId.Data("");
            requestEnc.HasAttrib = true;
            requestEnc.LoginAttrib.HasApplicationName = true;
            requestEnc.LoginAttrib.ApplicationName.Data("");
            requestEnc.LoginAttrib.HasPosition = true;
            requestEnc.LoginAttrib.Position.Data("");
            requestEnc.LoginAttrib.HasApplicationId = true;
            requestEnc.LoginAttrib.ApplicationId.Data("");

            dIter.Clear();
            encIter.Clear();

            Buffer membuf = new();
            membuf.Data(new ByteBuffer(1024));

            // Encode
            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            var ret = requestEnc.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Decode
            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = requestDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(LoginRequestFlags.HAS_USERNAME_TYPE, requestDec.Flags);
            Assert.Equal(LoginAttribFlags.NONE, requestDec.LoginAttrib.Flags);

            // Test 2: decode side is properly erroring out on blank inputs
            IRequestMsg requestMsg = new Msg();

            // AppID
            requestMsg.Clear();
            msg.Clear();
            membuf.Data(new ByteBuffer(1024));

            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.LOGIN;
            requestMsg.ApplyStreaming();

            requestMsg.MsgKey.ApplyHasAttrib();
            requestMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = name;

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = requestMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.APPID;
            ret = element.Encode(encIter, zeroLengthBuf);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            requestDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = requestDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(requestDec.HasAttrib);
            Assert.True(requestDec.LoginAttrib.HasApplicationId);
            Assert.Equal(0, requestDec.LoginAttrib.ApplicationId.Length);

            // AppName
            requestMsg.Clear();
            msg.Clear();
            membuf.Data(new ByteBuffer(1024));

            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.LOGIN;
            requestMsg.ApplyStreaming();

            requestMsg.MsgKey.ApplyHasAttrib();
            requestMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = name;

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = requestMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.APPNAME;
            ret = element.Encode(encIter, zeroLengthBuf);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(),
                    Codec.Codec.MinorVersion());
            requestDec.Clear();
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(requestDec.HasAttrib);
            Assert.True(requestDec.LoginAttrib.HasApplicationName);
            Assert.True(0 == requestDec.LoginAttrib.ApplicationName.Length);

            // Position
            requestMsg.Clear();
            msg.Clear();
            membuf.Data(new ByteBuffer(1024));

            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.LOGIN;
            requestMsg.ApplyStreaming();

            requestMsg.MsgKey.ApplyHasAttrib();
            requestMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = name;

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = requestMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.POSITION;
            ret = element.Encode(encIter, zeroLengthBuf);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(),
                    Codec.Codec.MinorVersion());
            requestDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = requestDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(requestDec.HasAttrib);
            Assert.True(requestDec.LoginAttrib.HasPosition);
            Assert.True(0 == requestDec.LoginAttrib.Position.Length);

            // Password
            requestMsg.Clear();
            msg.Clear();
            membuf.Data(new ByteBuffer(1024));

            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.LOGIN;
            requestMsg.ApplyStreaming();

            requestMsg.MsgKey.ApplyHasAttrib();
            requestMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = name;

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = requestMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);

            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.PASSWORD;
            ret = element.Encode(encIter, zeroLengthBuf);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            requestDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = requestDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(requestDec.HasPassword);
            Assert.True(0 == requestDec.Password.Length);

            // INST_ID
            requestMsg.Clear();
            msg.Clear();
            membuf.Data(new ByteBuffer(1024));

            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.LOGIN;
            requestMsg.ApplyStreaming();

            requestMsg.MsgKey.ApplyHasAttrib();
            requestMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = name;

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = requestMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.INST_ID;
            ret = element.Encode(encIter, zeroLengthBuf);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = requestMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(),
                    Codec.Codec.MinorVersion());
            requestDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = requestDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(requestDec.HasInstanceId);
            Assert.Equal(0, requestDec.InstanceId.Length);

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRequestToStringTest()
        {
            LoginRequest reqRDMMsg1 = new();
            //reqRDMMsg1.rdmMsgType(LoginMsgType.REQUEST);

            int streamId = -5;
            long allowSuspectData = 2;
            string userName = "userName";
            string applicationId = "applicationId";
            string applicationName = "applicationName";
            string position = "position";
            long providePermissionProfile = 2;
            long providePermissionExpressions = 2;
            string instanceId = "instanceId";
            string password = "password";
            long singleOpen = 1;
            var userNameType = UserIdTypes.NAME;
            long downloadConnectionConfig = 2;
            long role = 1;
            string authenticationExtended = "authenticationExtended";

            Console.WriteLine("LoginRequest toString test...");

            reqRDMMsg1.StreamId = streamId;
            reqRDMMsg1.UserName.Data(userName);
            reqRDMMsg1.UserNameType = userNameType;
            reqRDMMsg1.HasAttrib = true;
            reqRDMMsg1.LoginAttrib.HasAllowSuspectData = true;
            reqRDMMsg1.LoginAttrib.AllowSuspectData = allowSuspectData;
            reqRDMMsg1.LoginAttrib.HasApplicationId = true;
            reqRDMMsg1.LoginAttrib.ApplicationId.Data(applicationId);
            reqRDMMsg1.LoginAttrib.HasApplicationName = true;
            reqRDMMsg1.LoginAttrib.ApplicationName.Data(applicationName);
            reqRDMMsg1.HasDownloadConnectionConfig = true;
            reqRDMMsg1.DownloadConnectionConfig = downloadConnectionConfig;
            reqRDMMsg1.HasInstanceId = true;
            reqRDMMsg1.InstanceId.Data(instanceId);
            reqRDMMsg1.HasPassword = true;
            reqRDMMsg1.Password.Data(password);
            reqRDMMsg1.LoginAttrib.Position.Data(position);
            reqRDMMsg1.LoginAttrib.HasProvidePermissionExpressions = true;
            reqRDMMsg1.LoginAttrib.ProvidePermissionExpressions = providePermissionExpressions;
            reqRDMMsg1.LoginAttrib.HasProvidePermissionProfile = true;
            reqRDMMsg1.LoginAttrib.ProvidePermissionProfile = providePermissionProfile;
            reqRDMMsg1.HasRole = true;
            reqRDMMsg1.Role = role;
            reqRDMMsg1.LoginAttrib.HasSingleOpen = true;
            reqRDMMsg1.LoginAttrib.SingleOpen = singleOpen;
            reqRDMMsg1.HasAuthenticationExtended = true;
            reqRDMMsg1.AuthenticationExtended.Data(authenticationExtended);

            Assert.NotNull(reqRDMMsg1.ToString());
            Console.WriteLine(reqRDMMsg1.ToString());
            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRequestFlagsTests()
        {
            string test = "loginRequestTests";
            Console.WriteLine(test + "...");

            // parameters to test with
            int streamId = -5;
            long allowSuspectData = 2;
            string userName = "userName";
            string applicationId = "applicationId";
            string applicationName = "applicationName";
            string position = "position";
            long providePermissionProfile = 2;
            long providePermissionExpressions = 2;
            string instanceId = "instanceId";
            string password = "password";
            long singleOpen = 1;
            long downloadConnectionConfig = 2;
            long role = 1;
            string authenticationToken = "authenticationToken";
            string authenticationExtended = "authenticationExtended";

            LoginRequestFlags[] flagsBase = {
                LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG,
                LoginRequestFlags.HAS_INSTANCE_ID,
                LoginRequestFlags.HAS_PASSWORD,
                LoginRequestFlags.HAS_ROLE,
                LoginRequestFlags.HAS_USERNAME_TYPE,
                LoginRequestFlags.PAUSE_ALL,
                LoginRequestFlags.NO_REFRESH
            };
            LoginRequestFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);
            UserIdTypes[] userNameTypeList = {
                UserIdTypes.NAME,
                UserIdTypes.EMAIL_ADDRESS,
                UserIdTypes.TOKEN,
                UserIdTypes.COOKIE,
                UserIdTypes.AUTHN_TOKEN
            };

            Console.WriteLine("LoginRequest tests...");
            LoginRequest encRDMMsg = new();
            LoginRequest decRDMMsg = new();

            foreach (var userNameType in userNameTypeList)
            {
                // loop over all the userNameTypes
                foreach (var flags in flagsList)
                {
                    int authExtFlag = 0;

                    if (userNameType == UserIdTypes.AUTHN_TOKEN)
                        authExtFlag = 1;

                    // loop once, unless the userNameType is AUTHN_TOKEN, then loop twice
                    for (; authExtFlag >= 0; --authExtFlag)
                    {
                        dIter.Clear();
                        encIter.Clear();
                        Buffer membuf = new();
                        membuf.Data(new ByteBuffer(1024));

                        encRDMMsg.Clear();
                        encRDMMsg.Flags = flags;
                        encRDMMsg.StreamId = streamId;

                        if (encRDMMsg.HasUserNameType)
                        {
                            encRDMMsg.UserNameType = userNameType;
                            if (userNameType == UserIdTypes.AUTHN_TOKEN)
                                encRDMMsg.UserName.Data(authenticationToken);
                            else
                                encRDMMsg.UserName.Data(userName);

                            if (authExtFlag == 1)
                            {   // apply the extended data flag on the extra loop for the AUTHN_TOKEN userNameType
                                encRDMMsg.HasAuthenticationExtended = true;
                                encRDMMsg.AuthenticationExtended.Data(authenticationExtended);
                            }
                        }
                        else
                        {
                            encRDMMsg.UserName.Data(userName);
                        }

                        encRDMMsg.HasAttrib = true;
                        {
                            encRDMMsg.LoginAttrib.HasAllowSuspectData = true;
                            encRDMMsg.LoginAttrib.AllowSuspectData = allowSuspectData;
                            encRDMMsg.LoginAttrib.HasApplicationId = true;
                            encRDMMsg.LoginAttrib.ApplicationId.Data(applicationId);
                            encRDMMsg.LoginAttrib.HasApplicationName = true;
                            encRDMMsg.LoginAttrib.ApplicationName.Data(applicationName);
                            encRDMMsg.LoginAttrib.HasPosition = true;
                            encRDMMsg.LoginAttrib.Position.Data(position);
                            encRDMMsg.LoginAttrib.HasProvidePermissionExpressions = true;
                            encRDMMsg.LoginAttrib.ProvidePermissionExpressions = providePermissionExpressions;
                            encRDMMsg.LoginAttrib.HasProvidePermissionProfile = true;
                            encRDMMsg.LoginAttrib.ProvidePermissionProfile = providePermissionProfile;
                            encRDMMsg.LoginAttrib.HasSingleOpen = true;
                            encRDMMsg.LoginAttrib.SingleOpen = singleOpen;
                        }
                        if (encRDMMsg.HasDownloadConnectionConfig)
                            encRDMMsg.DownloadConnectionConfig = downloadConnectionConfig;

                        if (encRDMMsg.HasInstanceId)
                            encRDMMsg.InstanceId.Data(instanceId);

                        if (encRDMMsg.HasPassword)
                            encRDMMsg.Password.Data(password);

                        if (encRDMMsg.HasRole)
                            encRDMMsg.Role = role;

                        // Encode
                        encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        var ret = encRDMMsg.Encode(encIter);
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);

                        // Decode
                        dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        ret = msg.Decode(dIter);
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);
                        ret = decRDMMsg.Decode(dIter, msg);
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);
                        Assert.Equal(encRDMMsg.Flags, decRDMMsg.Flags);

                        if (decRDMMsg.HasUserNameType)
                        {
                            Assert.Equal(userNameType, decRDMMsg.UserNameType);
                            if (decRDMMsg.UserNameType == UserIdTypes.AUTHN_TOKEN)
                                Assert.Equal(authenticationToken, decRDMMsg.UserName.ToString());
                            else
                                Assert.Equal(userName, decRDMMsg.UserName.ToString());
                        }

                        if (decRDMMsg.HasDownloadConnectionConfig)
                            Assert.Equal(downloadConnectionConfig, decRDMMsg.DownloadConnectionConfig);

                        if (decRDMMsg.HasInstanceId)
                            Assert.Equal(instanceId, decRDMMsg.InstanceId.ToString());

                        if (decRDMMsg.HasPassword)
                            Assert.Equal(password, decRDMMsg.Password.ToString());

                        if (decRDMMsg.HasAuthenticationExtended)
                            Assert.Equal(authenticationExtended, decRDMMsg.AuthenticationExtended.ToString());

                        if (decRDMMsg.HasAttrib)
                        {
                            Assert.Equal(allowSuspectData, decRDMMsg.LoginAttrib.AllowSuspectData);
                            Assert.Equal(applicationId, decRDMMsg.LoginAttrib.ApplicationId.ToString());
                            Assert.Equal(applicationName, decRDMMsg.LoginAttrib.ApplicationName.ToString());
                            Assert.Equal(position, decRDMMsg.LoginAttrib.Position.ToString());
                            Assert.Equal(providePermissionProfile, decRDMMsg.LoginAttrib.ProvidePermissionProfile);
                            Assert.Equal(providePermissionExpressions, decRDMMsg.LoginAttrib.ProvidePermissionExpressions);
                            Assert.Equal(singleOpen, decRDMMsg.LoginAttrib.SingleOpen);
                        }
                    }
                }
            }
            Console.WriteLine(test + " Done (Ran " + flagsList.Length * (userNameTypeList.Length + 1)
                    + " tests: using " + flagsBase.Length + " flags producing " + flagsList.Length
                    + " flag combinations with " + userNameTypeList.Length
                    + " userNameTypes and 1 extended attrib).");
        }

        #endregion

        #region Login RTT

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRTTTest()
        {
            LoginRTT encLoginRTTMsg = new();
            //encLoginRTTMsg.rdmMsgType(LoginMsgType.RTT);
            LoginRTT decLoginRTTMsg = new();
            //decLoginRTTMsg.rdmMsgType(LoginMsgType.RTT);

            LoginRTTFlags[] flagsBase = {
                    LoginRTTFlags.ROUND_TRIP_LATENCY,
                    LoginRTTFlags.HAS_TCP_RETRANS,
                    LoginRTTFlags.PROVIDER_DRIVEN
            };

            int streamId = 1;
            long ticks = System.DateTime.Now.Ticks;
            long rtLatency = 500;
            long retrans = 5;

            Console.WriteLine("LoginRTT encode/decode tests...");

            var flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            foreach (var flags in flagsList)
            {
                dIter.Clear();
                encIter.Clear();

                encLoginRTTMsg.StreamId = streamId;
                encLoginRTTMsg.Flags = flags;
                if (encLoginRTTMsg.HasRTLatency)
                {
                    encLoginRTTMsg.RTLatency = rtLatency;
                }
                if (encLoginRTTMsg.HasTCPRetrans)
                {
                    encLoginRTTMsg.TCPRetrans = retrans;
                }
                encLoginRTTMsg.Ticks = ticks;

                Buffer membuf = new();
                membuf.Data(new ByteBuffer(1024));

                // Encode
                encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                var ret = encLoginRTTMsg.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                // Decode
                dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = msg.Decode(dIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                ret = decLoginRTTMsg.Decode(dIter, msg);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                //Assert.Equal(LoginMsgType.RTT, decLoginRTTMsg.MsgType());
                Assert.Equal(encLoginRTTMsg.StreamId, decLoginRTTMsg.StreamId);
                Assert.Equal(encLoginRTTMsg.Flags, decLoginRTTMsg.Flags);
                Assert.Equal(encLoginRTTMsg.MsgClass, decLoginRTTMsg.MsgClass);
                Assert.Equal(decLoginRTTMsg.Ticks, ticks);

                if (encLoginRTTMsg.HasRTLatency)
                {
                    Assert.Equal(decLoginRTTMsg.RTLatency, rtLatency);
                }

                if (encLoginRTTMsg.HasTCPRetrans)
                {
                    Assert.Equal(decLoginRTTMsg.TCPRetrans, retrans);
                }
            }
            Console.WriteLine("Done.");
        }

        private static readonly double TicksPerNanosecond = Stopwatch.Frequency / 1_000_000_000.0;

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRTTinitTest()
        {
            LoginRTT encLoginRTTMsg = new();
            //encLoginRTTMsg.rdmMsgType(LoginMsgType.RTT);
            LoginRTT decLoginRTTMsg = new();
            //decLoginRTTMsg.rdmMsgType(LoginMsgType.RTT);
            int streamId = 1;

            Console.WriteLine("LoginRTT initRTT test...");
            var t1 = (long)(Stopwatch.GetTimestamp() / TicksPerNanosecond);
            encLoginRTTMsg.InitDefaultRTT(streamId);
            var t2 = (long)(Stopwatch.GetTimestamp() / TicksPerNanosecond);

            Buffer membuf = new();
            membuf.Data(new ByteBuffer(1024));
            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            var ret = encLoginRTTMsg.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = decLoginRTTMsg.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(1, decLoginRTTMsg.StreamId);
            Assert.True(decLoginRTTMsg.Ticks >= t1);
            Assert.True(decLoginRTTMsg.Ticks <= t2);
            Assert.True(decLoginRTTMsg.ProviderDriven);
            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRTTcopyTest()
        {
            int streamId = 1;

            LoginRTT rttMsg1 = new();
            LoginRTT rttMsg2 = new();
            //rttMsg1.rdmMsgType(LoginMsgType.RTT);
            //rttMsg2.rdmMsgType(LoginMsgType.RTT);

            long latency = 300;
            long retrans = 5;

            Console.WriteLine("LoginRTT copy test...");

            rttMsg1.InitDefaultRTT(streamId);
            rttMsg1.HasRTLatency = true;
            rttMsg1.RTLatency = latency;
            rttMsg1.HasTCPRetrans = true;
            rttMsg1.TCPRetrans = retrans;

            var ret = rttMsg1.Copy(rttMsg2);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.Equal(rttMsg1.Ticks, rttMsg2.Ticks);
            Assert.Equal(rttMsg1.TCPRetrans, rttMsg2.TCPRetrans);
            Assert.Equal(retrans, rttMsg2.TCPRetrans);
            Assert.Equal(rttMsg1.RTLatency, rttMsg2.RTLatency);
            Assert.Equal(latency, rttMsg2.RTLatency);
            Assert.Equal(rttMsg1.ProviderDriven, rttMsg2.ProviderDriven);

            Console.WriteLine("Done.");
        }

        #endregion

        #region Login Connection Status

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginConnStatusTests()
        {
            LoginConsumerConnectionStatus encRDMMsg = new();
            //encRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
            LoginConsumerConnectionStatus decRDMMsg = new();
            //decRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);

            LoginConsumerConnectionStatusFlags[] flagsBase = {
                LoginConsumerConnectionStatusFlags.NONE,
                    LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO
                };

            //Parameters to test with
            int streamId = -5;
            long warmStandbyMode = 2;

            Console.WriteLine("LoginConnStatus tests...");
            //int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

            //ConnStatus
            foreach (var flags in flagsBase)
            {
                dIter.Clear();
                encIter.Clear();
                encRDMMsg.Clear();
                // encRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);

                encRDMMsg.StreamId = streamId;
                encRDMMsg.Flags = flags;
                //Set parameters based on flags
                if (encRDMMsg.HasWarmStandbyInfo)
                {
                    encRDMMsg.WarmStandbyInfo.Action = MapEntryActions.ADD;
                    encRDMMsg.WarmStandbyInfo.WarmStandbyMode = warmStandbyMode;
                }

                Buffer membufFlag = new();
                membufFlag.Data(new ByteBuffer(1024));

                // Encode

                encIter.SetBufferAndRWFVersion(membufFlag, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                var retFlag = encRDMMsg.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, retFlag);

                // Decode

                dIter.SetBufferAndRWFVersion(membufFlag, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                retFlag = msg.Decode(dIter);
                Assert.Equal(CodecReturnCode.SUCCESS, retFlag);

                retFlag = decRDMMsg.Decode(dIter, msg);
                Assert.Equal(CodecReturnCode.SUCCESS, retFlag);

                //Assert.Equal(decRDMMsg.rdmMsgType(), LoginMsgType.CONSUMER_CONNECTION_STATUS);

                Assert.Equal(encRDMMsg.StreamId, decRDMMsg.StreamId);
                Assert.Equal(encRDMMsg.Flags, decRDMMsg.Flags);
                //Assert.Equal(encRDMMsg.rdmMsgType(), decRDMMsg.rdmMsgType());

                //Check parameters
                if (decRDMMsg.HasWarmStandbyInfo)
                {
                    Assert.True(decRDMMsg.WarmStandbyInfo.Action == encRDMMsg.WarmStandbyInfo.Action);
                    Assert.True(decRDMMsg.WarmStandbyInfo.WarmStandbyMode == encRDMMsg.WarmStandbyInfo.WarmStandbyMode);
                }
            }

            //Test delete action
            dIter.Clear();
            encIter.Clear();
            encRDMMsg.Clear();
            // encRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
            encRDMMsg.StreamId = streamId;
            encRDMMsg.Flags = LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO;
            encRDMMsg.WarmStandbyInfo.Action = MapEntryActions.DELETE;

            Buffer membuf = new();
            membuf.Data(new ByteBuffer(1024));

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            var ret = encRDMMsg.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = decRDMMsg.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Assert.Equal(decRDMMsg.rdmMsgType(), LoginMsgType.CONSUMER_CONNECTION_STATUS);

            Assert.Equal(encRDMMsg.StreamId, decRDMMsg.StreamId);
            Assert.Equal(encRDMMsg.Flags, decRDMMsg.Flags);
            // Assert.Equal(encRDMMsg.rdmMsgType(), decRDMMsg.rdmMsgType());

            //Check parameters
            Assert.Equal(MapEntryActions.DELETE, encRDMMsg.WarmStandbyInfo.Action);

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginConnStatusCopyTest()
        {
            LoginConsumerConnectionStatus loginConnStatusMsg1 = new();
            LoginConsumerConnectionStatus loginConnStatusMsg2 = new();
            //loginConnStatusMsg1.MsgType = LoginMsgType.CONSUMER_CONNECTION_STATUS);
            //loginConnStatusMsg2.MsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);

            //Parameters to test with
            int streamId = -5;
            long warmStandbyMode = 1;

            LoginWarmStandByInfo loginWarmStandbyInfo1 = new();
            loginWarmStandbyInfo1.WarmStandbyMode = warmStandbyMode;
            loginWarmStandbyInfo1.Action = MapEntryActions.UPDATE;

            Console.WriteLine("LoginConsumerConnectionStatus copy test...");
            loginConnStatusMsg1.Flags = LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO;
            loginConnStatusMsg1.WarmStandbyInfo.Action = loginWarmStandbyInfo1.Action;
            loginConnStatusMsg1.WarmStandbyInfo.WarmStandbyMode = loginWarmStandbyInfo1.WarmStandbyMode;
            loginConnStatusMsg1.StreamId = streamId;
            CodecReturnCode ret = loginConnStatusMsg1.Copy(loginConnStatusMsg2);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            //verify deep copy
            Assert.Equal(loginConnStatusMsg1.StreamId, loginConnStatusMsg1.StreamId);
            Assert.True(loginConnStatusMsg1.WarmStandbyInfo != loginConnStatusMsg2.WarmStandbyInfo);
            Assert.Equal(loginConnStatusMsg1.WarmStandbyInfo.WarmStandbyMode, loginConnStatusMsg2.WarmStandbyInfo.WarmStandbyMode);
            Assert.Equal(loginConnStatusMsg1.WarmStandbyInfo.Action, loginConnStatusMsg2.WarmStandbyInfo.Action);
            Console.WriteLine("Done.");
        }


        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginConnStatusToStringTest()
        {
            LoginConsumerConnectionStatus loginConnStatusMsg1 = new();

            //Parameters to test with
            int streamId = -5;
            long warmStandbyMode = 1;

            LoginWarmStandByInfo loginWarmStandbyInfo1 = new()
            {
                WarmStandbyMode = warmStandbyMode,
                Action = MapEntryActions.UPDATE
            };

            Console.WriteLine("loginConnStatusTests toString test...");
            loginConnStatusMsg1.Flags = LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO;
            loginConnStatusMsg1.WarmStandbyInfo.WarmStandbyMode = loginWarmStandbyInfo1.WarmStandbyMode;
            loginConnStatusMsg1.WarmStandbyInfo.Action = loginWarmStandbyInfo1.Action;
            loginConnStatusMsg1.StreamId = streamId;
            loginConnStatusMsg1.ToString();
            Console.WriteLine("Done.");
        }

        #endregion

        #region Login Warm Stanbdy

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginWarmStandbyTests()
        {
            LoginWarmStandByInfo loginWarmStandbyInfo = new();
            LoginWarmStandByInfo loginWarmStandbyInfoDecoding = new();

            long warmStandbyMode = 1;
            loginWarmStandbyInfo.WarmStandbyMode = warmStandbyMode;
            Console.WriteLine("WarmStandby tests...");
            // allocate a ByteBuffer and associate it with a Buffer
            ByteBuffer bb = new ByteBuffer(1024);
            Buffer buffer = new();
            buffer.Data(bb);

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            var ret = loginWarmStandbyInfo.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            dIter.Clear();
            dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = loginWarmStandbyInfoDecoding.Decode(dIter, msg);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(1, loginWarmStandbyInfoDecoding.WarmStandbyMode);

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginWarmStandbyCopyTest()
        {
            LoginWarmStandByInfo loginWarmStandbyInfo1 = new();
            LoginWarmStandByInfo loginWarmStandbyInfo2 = new();

            long warmStandbyMode = 1;

            Console.WriteLine("WarmStandby copy test...");

            loginWarmStandbyInfo1.WarmStandbyMode = warmStandbyMode;
            loginWarmStandbyInfo1.Action = MapEntryActions.UPDATE;

            var ret = loginWarmStandbyInfo1.Copy(loginWarmStandbyInfo2);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // verify deep copy
            Assert.Equal(warmStandbyMode, loginWarmStandbyInfo2.WarmStandbyMode);
            Assert.Equal(MapEntryActions.UPDATE, loginWarmStandbyInfo2.Action);
            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginWarmStandbyToStringTest()
        {
            LoginWarmStandByInfo loginWarmStandbyInfo1 = new();
            LoginWarmStandByInfo loginWarmStandbyInfo2 = new();

            long warmStandbyMode = 1;
            Console.WriteLine("WarmStandby toString test...");
            loginWarmStandbyInfo1.WarmStandbyMode = warmStandbyMode;
            loginWarmStandbyInfo1.Action = MapEntryActions.UPDATE;

            var ret = loginWarmStandbyInfo1.Copy(loginWarmStandbyInfo2);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            //verify deep copy
            Assert.Equal(warmStandbyMode, loginWarmStandbyInfo2.WarmStandbyMode);
            Assert.Equal(MapEntryActions.UPDATE, loginWarmStandbyInfo2.Action);
            Console.WriteLine("Done.");
        }
        #endregion

        #region Login Close

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginCloseCopyTest()
        {
            LoginClose closeRDMMsg1 = new();
            LoginClose closeRDMMsg2 = new();
            // closeRDMMsg1.rdmMsgType(LoginMsgType.CLOSE);
            // closeRDMMsg2.rdmMsgType(LoginMsgType.CLOSE);
            int streamId = -5;
            closeRDMMsg1.StreamId = streamId;

            Console.WriteLine("LoginClose copy test...");

            //deep copy
            var ret = closeRDMMsg1.Copy(closeRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            //verify deep copy
            Assert.Equal(closeRDMMsg1.StreamId, closeRDMMsg2.StreamId);
            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginCloseToStringTest()
        {
            LoginClose closeRDMMsg1 = new();
            // closeRDMMsg1.rdmMsgType(LoginMsgType.CLOSE);
            int streamId = -5;
            closeRDMMsg1.StreamId = streamId;

            Console.WriteLine("LoginClose toString test...");

            closeRDMMsg1.ToString();
            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginCloseTests()
        {
            LoginClose encRDMMsg = new();
            LoginClose decRDMMsg = new();
            //decRDMMsg.rdmMsgType(LoginMsgType.CLOSE);
            int streamId = -5;

            dIter.Clear();
            encIter.Clear();
            Buffer membuf = new();
            membuf.Data(new ByteBuffer(1024));

            Console.WriteLine("LoginClose tests...");
            encRDMMsg.Clear();

            // encRDMMsg.rdmMsgType(LoginMsgType.CLOSE);
            encRDMMsg.StreamId = streamId;
            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            var ret = encRDMMsg.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = decRDMMsg.Decode(dIter, msg);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(streamId, decRDMMsg.StreamId);

            Console.WriteLine("Done.");
        }
        #endregion

        #region Login Refresh

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRefreshCopyTest()
        {
            LoginRefresh refRDMMsg1 = new();
            LoginRefresh refRDMMsg2 = new();
            // refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);
            // refRDMMsg2.rdmMsgType(LoginMsgType.REFRESH);
            Console.WriteLine("LoginRefresh copy test...");

            //parameters to test with
            int streamId = -5;
            long allowSuspectData = 2;
            string userName = "userName";
            string applicationId = "applicationId";
            string applicationName = "applicationName";
            string position = "position";
            long providePermissionProfile = 2;
            long providePermissionExpressions = 2;
            long singleOpen = 1;
            var userNameType = UserIdTypes.TOKEN;
            long supportStandby = 6;
            long supportBatchRequests = 1;
            long supportBatchReissues = 1;
            long supportBatchCloses = 1;
            long supportViewRequests = 8;
            long seqNum = 9;
            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);
            long authenticationTTReissue = 987654321;
            long authenticationErrorCode = 404;
            string authenticationErrorText = "some kind of authenticationErrorText";
            string authenticationExtendedResp = "some sort of authenticationExtendedResp info";

            //connection config parameters
            ServerInfo serverInfo = new ServerInfo();
            serverInfo.HasLoadFactor = true;
            serverInfo.HasType = true;
            Buffer hostName = new();
            hostName.Data("hostName");
            serverInfo.HostName = hostName;
            serverInfo.LoadFactor = 1;
            serverInfo.Port = 14444;
            serverInfo.ServerIndex = 1;
            serverInfo.ServerType = ServerTypes.ACTIVE;

            LoginConnectionConfig connectionConfig = new();
            connectionConfig.NumStandByServers = 1;
            connectionConfig.ServerList.Add(serverInfo);

            // refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);

            refRDMMsg1.StreamId = streamId;

            refRDMMsg1.State.Code(state.Code());
            refRDMMsg1.State.DataState(state.DataState());
            refRDMMsg1.State.Text().Data("state");
            refRDMMsg1.State.StreamState(state.StreamState());

            refRDMMsg1.ClearCache = true;
            refRDMMsg1.Solicited = true;
            refRDMMsg1.UserName.Data(userName);
            refRDMMsg1.HasUserName = true;
            refRDMMsg1.UserNameType = userNameType;
            refRDMMsg1.HasUserNameType = true;

            refRDMMsg1.HasConnectionConfig = true;
            refRDMMsg1.ConnectionConfig = connectionConfig;

            refRDMMsg1.HasAttrib = true;
            refRDMMsg1.LoginAttrib.HasAllowSuspectData = true;
            refRDMMsg1.LoginAttrib.AllowSuspectData = allowSuspectData;

            refRDMMsg1.LoginAttrib.HasApplicationId = true;
            refRDMMsg1.LoginAttrib.ApplicationId.Data(applicationId);

            refRDMMsg1.LoginAttrib.HasPosition = true;
            refRDMMsg1.LoginAttrib.Position.Data(position);

            refRDMMsg1.LoginAttrib.HasApplicationName = true;
            refRDMMsg1.LoginAttrib.ApplicationName.Data(applicationName);

            refRDMMsg1.LoginAttrib.HasProvidePermissionProfile = true;
            refRDMMsg1.LoginAttrib.ProvidePermissionProfile = providePermissionProfile;

            refRDMMsg1.LoginAttrib.HasProvidePermissionExpressions = true;
            refRDMMsg1.LoginAttrib.ProvidePermissionExpressions = providePermissionExpressions;

            refRDMMsg1.LoginAttrib.HasSingleOpen = true;
            refRDMMsg1.LoginAttrib.SingleOpen = singleOpen;

            refRDMMsg1.HasFeatures = true;
            refRDMMsg1.SupportedFeatures.HasSupportBatchRequests = true;
            refRDMMsg1.SupportedFeatures.SupportBatchRequests = supportBatchRequests;

            refRDMMsg1.SupportedFeatures.HasSupportBatchReissues = true;
            refRDMMsg1.SupportedFeatures.SupportBatchReissues = supportBatchReissues;

            refRDMMsg1.SupportedFeatures.HasSupportBatchCloses = true;
            refRDMMsg1.SupportedFeatures.SupportBatchCloses = supportBatchCloses;

            refRDMMsg1.SupportedFeatures.HasSupportViewRequests = true;
            refRDMMsg1.SupportedFeatures.SupportViewRequests = supportViewRequests;

            refRDMMsg1.SupportedFeatures.HasSupportStandby = true;
            refRDMMsg1.SupportedFeatures.SupportStandby = supportStandby;

            refRDMMsg1.HasSequenceNumber = true;
            refRDMMsg1.SequenceNumber = seqNum;

            refRDMMsg1.HasAuthenicationTTReissue = true;
            refRDMMsg1.AuthenticationTTReissue = authenticationTTReissue;

            refRDMMsg1.HasAuthenticationExtendedResp = true;
            refRDMMsg1.AuthenticationExtendedResp.Data(authenticationExtendedResp);

            refRDMMsg1.HasAuthenticationErrorText = true;
            refRDMMsg1.AuthenticationErrorText.Data(authenticationErrorText);

            refRDMMsg1.HasAuthenticationErrorCode = true;
            refRDMMsg1.AuthenticationErrorCode = authenticationErrorCode;

            var ret = refRDMMsg1.Copy(refRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            //verify deep copy
            Assert.Equal(refRDMMsg1.Flags, refRDMMsg2.Flags);
            Assert.True(refRDMMsg1.State != refRDMMsg2.State);

            State refState1 = refRDMMsg1.State;
            State refState2 = refRDMMsg2.State;
            Assert.NotNull(refState2);
            Assert.Equal(refState1.Code(), refState2.Code());
            Assert.Equal(refState1.DataState(), refState2.DataState());
            Assert.Equal(refState1.StreamState(), refState2.StreamState());
            Assert.Equal(refState1.Text().ToString(), refState2.Text().ToString());
            Assert.True(refState1.Text() != refState2.Text());

            Assert.True(refRDMMsg1.UserName != refRDMMsg2.UserName);
            Assert.Equal(refRDMMsg1.UserName.ToString(), refRDMMsg2.UserName.ToString());

            Assert.Equal(refRDMMsg1.UserNameType, refRDMMsg2.UserNameType);

            Assert.Equal(refRDMMsg1.LoginAttrib.AllowSuspectData, refRDMMsg2.LoginAttrib.AllowSuspectData);
            Assert.Equal(refRDMMsg1.LoginAttrib.ApplicationId.ToString(), refRDMMsg2.LoginAttrib.ApplicationId.ToString());
            Assert.Equal(refRDMMsg1.LoginAttrib.ProvidePermissionProfile, refRDMMsg2.LoginAttrib.ProvidePermissionProfile);
            Assert.Equal(refRDMMsg1.LoginAttrib.ProvidePermissionExpressions, refRDMMsg2.LoginAttrib.ProvidePermissionExpressions);
            Assert.Equal(refRDMMsg1.LoginAttrib.SingleOpen, refRDMMsg2.LoginAttrib.SingleOpen);
            Assert.Equal(refRDMMsg1.SupportedFeatures.SupportBatchRequests, refRDMMsg2.SupportedFeatures.SupportBatchRequests);
            Assert.Equal(refRDMMsg1.SupportedFeatures.SupportBatchReissues, refRDMMsg2.SupportedFeatures.SupportBatchReissues);
            Assert.Equal(refRDMMsg1.SupportedFeatures.SupportBatchCloses, refRDMMsg2.SupportedFeatures.SupportBatchCloses);
            Assert.Equal(refRDMMsg1.SupportedFeatures.SupportViewRequests, refRDMMsg2.SupportedFeatures.SupportViewRequests);

            LoginConnectionConfig connectionConfig2 = refRDMMsg2.ConnectionConfig;
            Assert.Equal(connectionConfig.NumStandByServers, connectionConfig2.NumStandByServers);
            Assert.Single(connectionConfig2.ServerList);
            ServerInfo serverInfo2 = connectionConfig2.ServerList[0];
            Assert.Equal(serverInfo.Flags, serverInfo2.Flags);
            Assert.Equal(hostName.ToString(), serverInfo2.HostName.ToString());
            Assert.Equal(serverInfo.LoadFactor, serverInfo2.LoadFactor);
            Assert.Equal(serverInfo.Port, serverInfo2.Port);
            Assert.Equal(serverInfo.ServerIndex, serverInfo2.ServerIndex);
            Assert.Equal(serverInfo.ServerType, serverInfo2.ServerType);

            Assert.Equal(refRDMMsg1.SequenceNumber, refRDMMsg2.SequenceNumber);

            Assert.Equal(refRDMMsg1.AuthenticationTTReissue, refRDMMsg2.AuthenticationTTReissue);
            Assert.Equal(refRDMMsg1.AuthenticationErrorCode, refRDMMsg2.AuthenticationErrorCode);
            Assert.Equal(refRDMMsg1.AuthenticationErrorText.ToString(), refRDMMsg2.AuthenticationErrorText.ToString());
            Assert.Equal(refRDMMsg1.AuthenticationExtendedResp.ToString(), refRDMMsg2.AuthenticationExtendedResp.ToString());

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRefreshBlankTest()
        {
            // original message for encoding into membuf
            LoginRefresh refreshEnc = new();
            // decoded message from membuf. must match the original
            LoginRefresh refreshDec = new();
            // refreshEnc.rdmMsgType(LoginMsgType.REFRESH);
            // refreshDec.rdmMsgType(LoginMsgType.REFRESH);
            zeroLengthBuf.Data("");

            Buffer name = new();
            name.Data("user");

            Console.WriteLine("LoginRefresh blank test...");
            // Test 1: Encode side
            //parameters setup
            int streamId = -5;

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            //status msg setup
            refreshEnc.Clear();
            refreshEnc.Flags = default;
            refreshEnc.StreamId = streamId;

            refreshEnc.HasAttrib = true;
            refreshEnc.LoginAttrib.HasApplicationId = true;
            refreshEnc.LoginAttrib.ApplicationId.Data("");
            refreshEnc.LoginAttrib.HasApplicationName = true;
            refreshEnc.LoginAttrib.ApplicationName.Data("");
            refreshEnc.LoginAttrib.HasPosition = true;
            refreshEnc.LoginAttrib.Position.Data("");
            refreshEnc.HasAuthenticationErrorCode = true;
            refreshEnc.AuthenticationErrorCode = 404;
            refreshEnc.HasAuthenticationErrorText = true;
            refreshEnc.AuthenticationErrorText.Data("");

            dIter.Clear();
            encIter.Clear();

            Buffer membuf = new();
            membuf.Data(new ByteBuffer(1024));

            // Encode LoginRefresh original
            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            var ret = refreshEnc.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Decode LoginRefresh from membuf
            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_CODE, refreshDec.Flags);
            Assert.Equal(LoginAttribFlags.NONE, refreshDec.LoginAttrib.Flags);

            // Test 2: decode side is properly erroring out on blank inputs
            // Authentication Error Text
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.Clear();
            membuf.Data(new ByteBuffer(1024));

            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.LOGIN;
            refreshMsg.ApplyHasMsgKey();

            refreshMsg.MsgKey.ApplyHasAttrib();
            refreshMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name = name;

            state.Copy(refreshMsg.State);

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = refreshMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);

            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.AUTHN_ERROR_TEXT;
            ret = element.Encode(encIter, zeroLengthBuf);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Decode
            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            refreshDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = refreshDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(refreshDec.HasAuthenticationErrorText);
            Assert.Equal(0, refreshDec.AuthenticationErrorText.Length);

            // Authentication Extended Resp Text
            refreshMsg.Clear();
            membuf.Data(new ByteBuffer(1024));

            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.LOGIN;
            refreshMsg.ApplyHasMsgKey();

            refreshMsg.MsgKey.ApplyHasAttrib();
            refreshMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name = name;

            state.Copy(refreshMsg.State);

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = refreshMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.BUFFER;
            element.Name = ElementNames.AUTHN_EXTENDED_RESP;
            ret = element.Encode(encIter, zeroLengthBuf);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            refreshDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = refreshDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(refreshDec.HasAuthenticationExtendedResp);
            Assert.Equal(0, refreshDec.AuthenticationExtendedResp.Length);

            // AppId
            refreshMsg.Clear();
            membuf.Data(new ByteBuffer(1024));

            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.LOGIN;
            refreshMsg.ApplyHasMsgKey();

            refreshMsg.MsgKey.ApplyHasAttrib();
            refreshMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name = name;
            state.Copy(refreshMsg.State);

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = refreshMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.APPID;
            ret = element.Encode(encIter, zeroLengthBuf);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            refreshDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = refreshDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(refreshDec.HasAttrib);
            Assert.True(refreshDec.LoginAttrib.HasApplicationId);
            Assert.Equal(0, refreshDec.LoginAttrib.ApplicationId.Length);

            // AppName
            refreshMsg.Clear();
            membuf.Data(new ByteBuffer(1024));

            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.LOGIN;
            refreshMsg.ApplyHasMsgKey();

            refreshMsg.MsgKey.ApplyHasAttrib();
            refreshMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name = name;
            state.Copy(refreshMsg.State);


            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = refreshMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.APPNAME;
            ret = element.Encode(encIter, zeroLengthBuf);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            refreshDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = refreshDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(refreshDec.HasAttrib);
            Assert.True(refreshDec.LoginAttrib.HasApplicationName);
            Assert.Equal(0, refreshDec.LoginAttrib.ApplicationName.Length);

            // Position
            refreshMsg.Clear();
            membuf.Data(new ByteBuffer(1024));

            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.LOGIN;
            refreshMsg.ApplyHasMsgKey();

            refreshMsg.MsgKey.ApplyHasAttrib();
            refreshMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name = name;
            state.Copy(refreshMsg.State);

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = refreshMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.POSITION;
            ret = element.Encode(encIter, zeroLengthBuf);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            refreshDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = refreshDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(refreshDec.HasAttrib);
            Assert.True(refreshDec.LoginAttrib.HasPosition);
            Assert.Equal(0, refreshDec.LoginAttrib.Position.Length);

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRefreshAuthnTokenTest()
        {
            // original message for encoding into membuf
            LoginRefresh refreshEnc = new();
            // decoded message from membuf. must match the original
            LoginRefresh refreshDec = new();
            zeroLengthBuf.Data("");

            string extResp = "ext_resp";
            string userName = "user_name";
            Buffer name = new();
            name.Data(userName);

            Console.WriteLine("LoginRefresh blank test...");
            // Test 1: Encode side
            //parameters setup

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            //status msg setup
            refreshEnc.Clear();
            // provRefresh[0].rdmMsgType(LoginMsgType.REFRESH);
            refreshEnc.StreamId = 1;
            refreshEnc.Solicited = true;
            refreshEnc.HasUserNameType = true;
            refreshEnc.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            refreshEnc.HasUserName = true;
            refreshEnc.UserName.Data(userName);
            refreshEnc.HasAuthenicationTTReissue = true;
            refreshEnc.AuthenticationTTReissue = 12345;
            refreshEnc.HasAuthenticationExtendedResp = true;
            refreshEnc.AuthenticationExtendedResp.Data(extResp);
            refreshEnc.State.StreamState(StreamStates.OPEN);
            refreshEnc.State.DataState(DataStates.OK);

            dIter.Clear();
            encIter.Clear();

            Buffer membuf = new();
            membuf.Data(new ByteBuffer(1024));

            // Encode LoginRefresh original
            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            var ret = refreshEnc.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Decode LoginRefresh from membuf
            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = refreshDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(LoginRefreshFlags.HAS_USERNAME | LoginRefreshFlags.HAS_USERNAME_TYPE | LoginRefreshFlags.SOLICITED
                | LoginRefreshFlags.HAS_AUTHENTICATION_TT_REISSUE | LoginRefreshFlags.HAS_AUTHENTICATION_EXTENDED_RESP,
                refreshDec.Flags);
            Assert.Equal(LoginAttribFlags.NONE, refreshDec.LoginAttrib.Flags);

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRefreshToStringTest()
        {
            LoginRefresh refRDMMsg1 = new();
            //refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);

            Console.WriteLine("LoginRefresh toString test...");

            //parameters to test with
            int streamId = -5;
            long allowSuspectData = 2;
            string userName = "userName";
            string applicationId = "applicationId";
            string applicationName = "applicationName";
            string position = "position";
            long providePermissionProfile = 2;
            long providePermissionExpressions = 2;
            long singleOpen = 1;
            var userNameType = UserIdTypes.NAME;
            long supportStandby = 6;
            long supportBatchRequests = 1;
            long supportBatchReissues = 1;
            long supportBatchCloses = 1;
            long supportViewRequests = 8;
            long seqNum = 9;
            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            long authenticationTTReissue = 987654321;
            long authenticationErrorCode = 404;
            string authenticationErrorText = "some kind of authenticationErrorText";
            string authenticationExtendedResp = "some sort of authenticationExtendedResp info";

            //connection config parameters
            ServerInfo serverInfo = new();
            serverInfo.HasLoadFactor = true;
            serverInfo.HasType = true;
            Buffer hostName = new();
            hostName.Data("hostName");
            serverInfo.HostName = hostName;
            serverInfo.LoadFactor = 1;
            serverInfo.Port = 14444;
            serverInfo.ServerIndex = 1;
            serverInfo.ServerType = ServerTypes.ACTIVE;
            LoginConnectionConfig connectionConfig = new();
            connectionConfig.NumStandByServers = 1;
            connectionConfig.ServerList.Add(serverInfo);

            // refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);
            refRDMMsg1.Flags = allRefreshMsgFlags;

            refRDMMsg1.StreamId = streamId;
            refRDMMsg1.State.Code(state.Code());
            refRDMMsg1.State.DataState(state.DataState());
            refRDMMsg1.State.Text().Data("state");
            refRDMMsg1.State.StreamState(state.StreamState());

            refRDMMsg1.UserName.Data(userName);
            refRDMMsg1.UserNameType = userNameType;

            refRDMMsg1.HasConnectionConfig = true;
            refRDMMsg1.ConnectionConfig = connectionConfig;

            refRDMMsg1.HasAttrib = true;
            refRDMMsg1.LoginAttrib.HasAllowSuspectData = true;
            refRDMMsg1.LoginAttrib.AllowSuspectData = allowSuspectData;
            refRDMMsg1.LoginAttrib.HasApplicationId = true;
            refRDMMsg1.LoginAttrib.ApplicationId.Data(applicationId);
            refRDMMsg1.LoginAttrib.HasPosition = true;
            refRDMMsg1.LoginAttrib.Position.Data(position);
            refRDMMsg1.LoginAttrib.HasApplicationName = true;
            refRDMMsg1.LoginAttrib.ApplicationName.Data(applicationName);
            refRDMMsg1.LoginAttrib.HasProvidePermissionProfile = true;
            refRDMMsg1.LoginAttrib.ProvidePermissionProfile = providePermissionProfile;
            refRDMMsg1.LoginAttrib.HasProvidePermissionExpressions = true;
            refRDMMsg1.LoginAttrib.ProvidePermissionExpressions = providePermissionExpressions;
            refRDMMsg1.LoginAttrib.HasSingleOpen = true;
            refRDMMsg1.LoginAttrib.SingleOpen = singleOpen;

            refRDMMsg1.HasFeatures = true;
            refRDMMsg1.SupportedFeatures.HasSupportBatchRequests = true;
            refRDMMsg1.SupportedFeatures.SupportBatchRequests = supportBatchRequests;
            refRDMMsg1.SupportedFeatures.HasSupportBatchReissues = true;
            refRDMMsg1.SupportedFeatures.SupportBatchReissues = supportBatchReissues;
            refRDMMsg1.SupportedFeatures.HasSupportBatchCloses = true;
            refRDMMsg1.SupportedFeatures.SupportBatchCloses = supportBatchCloses;
            refRDMMsg1.SupportedFeatures.HasSupportViewRequests = true;
            refRDMMsg1.SupportedFeatures.SupportViewRequests = supportViewRequests;
            refRDMMsg1.SupportedFeatures.HasSupportStandby = true;
            refRDMMsg1.SupportedFeatures.SupportStandby = supportStandby;
            refRDMMsg1.HasSequenceNumber = true;
            refRDMMsg1.SequenceNumber = seqNum;

            refRDMMsg1.HasAuthenicationTTReissue = true;
            refRDMMsg1.AuthenticationTTReissue = authenticationTTReissue;
            refRDMMsg1.HasAuthenticationExtendedResp = true;
            refRDMMsg1.AuthenticationExtendedResp.Data(authenticationExtendedResp);
            refRDMMsg1.HasAuthenticationErrorText = true;
            refRDMMsg1.AuthenticationErrorText.Data(authenticationErrorText);
            refRDMMsg1.HasAuthenticationErrorCode = true;
            refRDMMsg1.AuthenticationErrorCode = authenticationErrorCode;

            Assert.NotNull(refRDMMsg1.ToString());
            Console.WriteLine(refRDMMsg1.ToString());
            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginRefreshTests()
        {
            string test = "loginRefreshTests";
            LoginRefresh encRDMMsg = new();
            LoginRefresh decRDMMsg = new();
            // encRDMMsg.rdmMsgType(LoginMsgType.REFRESH);
            // decRDMMsg.rdmMsgType(LoginMsgType.REFRESH);

            LoginRefreshFlags[] flagsBase = {
                LoginRefreshFlags.CLEAR_CACHE, LoginRefreshFlags.HAS_CONN_CONFIG,
                LoginRefreshFlags.HAS_ATTRIB, LoginRefreshFlags.HAS_SEQ_NUM,
                LoginRefreshFlags.HAS_FEATURES, LoginRefreshFlags.HAS_USERNAME,
                LoginRefreshFlags.HAS_USERNAME_TYPE, LoginRefreshFlags.SOLICITED
            };

            //parameters to test with
            int streamId = -5;
            long allowSuspectData = 2;
            string userName = "userName";
            string applicationId = "applicationId";
            string applicationName = "applicationName";
            string position = "position";
            long providePermissionProfile = 2;
            long providePermissionExpressions = 2;
            long singleOpen = 1;
            long supportStandby = 6;
            long supportBatchRequests = 1;
            long supportBatchReissues = 1;
            long supportBatchCloses = 1;
            long supportViewRequests = 8;
            long supportOMMPost = 9;
            long supportOPR = 10;

            long seqNum = 11;
            long authenticationTTReissue = 987654321;
            long authenticationErrorCode = 404;
            string authenticationErrorText = "some kind of authenticationErrorText";
            string authenticationExtendedResp = "some sort of authenticationExtendedResp info";

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            //connection config parameters
            ServerInfo serverInfo = new();
            serverInfo.HasLoadFactor = true;
            serverInfo.HasType = true;
            Buffer hostName = new();
            hostName.Data("hostName");
            serverInfo.HostName = hostName;
            serverInfo.LoadFactor = 1;
            serverInfo.Port = 14444;
            serverInfo.ServerIndex = 1;
            serverInfo.ServerType = ServerTypes.ACTIVE;

            LoginConnectionConfig connectionConfig = new();
            connectionConfig.NumStandByServers = 1;
            connectionConfig.ServerList.Add(serverInfo);

            LoginRefreshFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, true);

            UserIdTypes[] userNameTypeList = { UserIdTypes.NAME, UserIdTypes.EMAIL_ADDRESS,
                    UserIdTypes.TOKEN, UserIdTypes.COOKIE, UserIdTypes.AUTHN_TOKEN };

            Console.WriteLine(test + "...");

            foreach (var userNameType in userNameTypeList)
            {
                foreach (var flags in flagsList)
                {
                    int authExtFlag = 0;

                    if (userNameType == UserIdTypes.AUTHN_TOKEN)
                        authExtFlag = 3;

                    for (; authExtFlag >= 0; --authExtFlag)
                    {
                        dIter.Clear();
                        encIter.Clear();
                        Buffer membuf = new();
                        membuf.Data(new ByteBuffer(1024));

                        encRDMMsg.Clear();
                        encRDMMsg.Flags = flags;
                        encRDMMsg.StreamId = streamId;
                        encRDMMsg.HasUserName = true;
                        encRDMMsg.UserName.Data(userName);

                        encRDMMsg.State.Code(state.Code());
                        encRDMMsg.State.DataState(state.DataState());
                        encRDMMsg.State.Text().Data("state");
                        encRDMMsg.State.StreamState(state.StreamState());

                        if (encRDMMsg.HasConnectionConfig)
                            encRDMMsg.ConnectionConfig = connectionConfig;

                        if (encRDMMsg.HasSequenceNumber)
                            encRDMMsg.SequenceNumber = seqNum;

                        if (encRDMMsg.HasUserNameType)
                        {
                            encRDMMsg.UserNameType = userNameType;
                            if (userNameType == UserIdTypes.AUTHN_TOKEN)
                            {
                                encRDMMsg.HasAuthenicationTTReissue = true;
                                encRDMMsg.AuthenticationTTReissue = authenticationTTReissue;
                            }
                            switch (authExtFlag)
                            {
                                case 1:
                                    encRDMMsg.HasAuthenticationExtendedResp = true;
                                    encRDMMsg.AuthenticationExtendedResp.Data(authenticationExtendedResp);
                                    break;
                                case 2:
                                    encRDMMsg.HasAuthenticationErrorCode = true;
                                    encRDMMsg.AuthenticationErrorCode = authenticationErrorCode;
                                    break;
                                case 3:
                                    encRDMMsg.HasAuthenticationErrorText = true;
                                    encRDMMsg.AuthenticationErrorText.Data(authenticationErrorText);
                                    break;
                                default:
                                    break;
                            }
                        }

                        if (encRDMMsg.HasAttrib)
                        {
                            encRDMMsg.LoginAttrib.HasAllowSuspectData = true;
                            encRDMMsg.LoginAttrib.AllowSuspectData = allowSuspectData;

                            encRDMMsg.LoginAttrib.HasApplicationId = true;
                            encRDMMsg.LoginAttrib.ApplicationId.Data(applicationId);

                            encRDMMsg.LoginAttrib.HasPosition = true;
                            encRDMMsg.LoginAttrib.Position.Data(position);

                            encRDMMsg.LoginAttrib.HasApplicationName = true;
                            encRDMMsg.LoginAttrib.ApplicationName.Data(applicationName);

                            encRDMMsg.LoginAttrib.HasProvidePermissionProfile = true;
                            encRDMMsg.LoginAttrib.ProvidePermissionProfile = providePermissionProfile;

                            encRDMMsg.LoginAttrib.HasProvidePermissionExpressions = true;
                            encRDMMsg.LoginAttrib.ProvidePermissionExpressions = providePermissionExpressions;

                            encRDMMsg.LoginAttrib.HasSingleOpen = true;
                            encRDMMsg.LoginAttrib.SingleOpen = singleOpen;
                        }

                        if (encRDMMsg.HasFeatures)
                        {
                            encRDMMsg.SupportedFeatures.HasSupportBatchRequests = true;
                            encRDMMsg.SupportedFeatures.SupportBatchRequests = supportBatchRequests;

                            encRDMMsg.SupportedFeatures.HasSupportBatchReissues = true;
                            encRDMMsg.SupportedFeatures.SupportBatchReissues = supportBatchReissues;

                            encRDMMsg.SupportedFeatures.HasSupportBatchCloses = true;
                            encRDMMsg.SupportedFeatures.SupportBatchCloses = supportBatchCloses;

                            encRDMMsg.SupportedFeatures.HasSupportViewRequests = true;
                            encRDMMsg.SupportedFeatures.SupportViewRequests = supportViewRequests;

                            encRDMMsg.SupportedFeatures.HasSupportStandby = true;
                            encRDMMsg.SupportedFeatures.SupportStandby = supportStandby;

                            encRDMMsg.SupportedFeatures.HasSupportPost = true;
                            encRDMMsg.SupportedFeatures.SupportOMMPost = supportOMMPost;

                            encRDMMsg.SupportedFeatures.HasSupportOptimizedPauseResume = true;
                            encRDMMsg.SupportedFeatures.SupportOptimizedPauseResume = supportOPR;
                        }

                        encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        var ret = encRDMMsg.Encode(encIter);
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);

                        dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        msg.Decode(dIter);
                        ret = decRDMMsg.Decode(dIter, msg);
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);
                        // Assert.Equal(encRDMMsg.rdmMsgType(), LoginMsgType.REFRESH);
                        Assert.Equal(encRDMMsg.StreamId, decRDMMsg.StreamId);
                        Console.WriteLine($"userNameType = {userNameType}, flags = {flags}, authExtFlag = {authExtFlag}, encRDMMsg.Flags = {encRDMMsg.Flags}, decRDMMsg.Flags = {decRDMMsg.Flags}");
                        Assert.Equal(encRDMMsg.Flags, decRDMMsg.Flags);

                        State decState = decRDMMsg.State;
                        Assert.NotNull(decState);
                        Assert.Equal(state.Code(), decState.Code());
                        Assert.Equal(state.DataState(), decState.DataState());
                        Assert.Equal(state.StreamState(), decState.StreamState());
                        Assert.Equal(state.Text().ToString(), decState.Text().ToString());

                        if (decRDMMsg.HasUserNameType)
                        {
                            Assert.Equal(userNameType, decRDMMsg.UserNameType);
                            if (decRDMMsg.UserNameType != UserIdTypes.AUTHN_TOKEN)
                                Assert.Equal(userName, decRDMMsg.UserName.ToString());
                        }

                        if (decRDMMsg.HasSequenceNumber)
                            Assert.Equal(seqNum, decRDMMsg.SequenceNumber);

                        if (decRDMMsg.HasAuthenicationTTReissue)
                            Assert.Equal(authenticationTTReissue, decRDMMsg.AuthenticationTTReissue);

                        if (decRDMMsg.HasAuthenticationErrorCode)
                            Assert.Equal(authenticationErrorCode, decRDMMsg.AuthenticationErrorCode);

                        if (decRDMMsg.HasAuthenticationErrorText)
                            Assert.Equal(authenticationErrorText, decRDMMsg.AuthenticationErrorText.ToString());

                        if (decRDMMsg.HasAuthenticationExtendedResp)
                            Assert.Equal(authenticationExtendedResp, decRDMMsg.AuthenticationExtendedResp.ToString());

                        if (decRDMMsg.HasConnectionConfig)
                        {
                            LoginConnectionConfig decConnectionConfig = decRDMMsg.ConnectionConfig;
                            Assert.Equal(connectionConfig.NumStandByServers, decConnectionConfig.NumStandByServers);
                            Assert.Single(decConnectionConfig.ServerList);

                            ServerInfo decServerInfo = decConnectionConfig.ServerList[0];
                            Assert.Equal(serverInfo.Flags, decServerInfo.Flags);
                            Assert.Equal(hostName.ToString(), decServerInfo.HostName.ToString());
                            Assert.Equal(serverInfo.LoadFactor, decServerInfo.LoadFactor);
                            Assert.Equal(serverInfo.Port, decServerInfo.Port);
                            Assert.Equal(serverInfo.ServerIndex, decServerInfo.ServerIndex);
                            Assert.Equal(serverInfo.ServerType, decServerInfo.ServerType);
                        }
                        if (decRDMMsg.HasAttrib)
                        {
                            Assert.Equal(allowSuspectData, decRDMMsg.LoginAttrib.AllowSuspectData);
                            Assert.Equal(applicationId, decRDMMsg.LoginAttrib.ApplicationId.ToString());
                            Assert.Equal(applicationName, decRDMMsg.LoginAttrib.ApplicationName.ToString());
                            Assert.Equal(providePermissionProfile, decRDMMsg.LoginAttrib.ProvidePermissionProfile);
                            Assert.Equal(providePermissionExpressions, decRDMMsg.LoginAttrib.ProvidePermissionExpressions);
                            Assert.Equal(singleOpen, decRDMMsg.LoginAttrib.SingleOpen);

                        }
                        if (decRDMMsg.HasFeatures)
                        {
                            Assert.Equal(supportBatchRequests, decRDMMsg.SupportedFeatures.SupportBatchRequests);
                            Assert.Equal(supportBatchReissues, decRDMMsg.SupportedFeatures.SupportBatchReissues);
                            Assert.Equal(supportBatchCloses, decRDMMsg.SupportedFeatures.SupportBatchCloses);
                            Assert.Equal(supportViewRequests, decRDMMsg.SupportedFeatures.SupportViewRequests);
                            Assert.Equal(supportStandby, decRDMMsg.SupportedFeatures.SupportStandby);
                            Assert.Equal(supportOMMPost, decRDMMsg.SupportedFeatures.SupportOMMPost);
                            Assert.Equal(supportOPR, decRDMMsg.SupportedFeatures.SupportOptimizedPauseResume);
                        }
                    }
                }
            }

            Console.WriteLine(test + " Done (Ran " + flagsList.Length + " tests: using "
                            + flagsBase.Length + " flags producing " + flagsList.Length
                            + " flag combinations)");

        }
        #endregion

        #region Login Status

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginStatusTests()
        {
            string test = "loginStatusTests";
            LoginStatus encRDMMsg = new();
            //encRDMMsg.rdmMsgType(LoginMsgType.STATUS);
            LoginStatus decRDMMsg = new();
            //decRDMMsg.rdmMsgType(LoginMsgType.STATUS);

            Console.WriteLine(test + "...");

            LoginStatusFlags[] flagsBase = {
                LoginStatusFlags.HAS_STATE,
                LoginStatusFlags.HAS_USERNAME,
                LoginStatusFlags.HAS_USERNAME_TYPE,
                LoginStatusFlags.CLEAR_CACHE,
                LoginStatusFlags.HAS_AUTHENTICATION_ERROR_CODE,
                LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT
            };

            //parameters setup
            int streamId = -5;
            string userName = "userName";
            var userNameType = UserIdTypes.NAME;
            long authenticationErrorCode = 404;
            string authenticationErrorText = "some kind of authenticationErrorText";

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            var flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);
            foreach (var flags in flagsList)
            {
                dIter.Clear();
                encIter.Clear();
                encRDMMsg.Clear();
                //encRDMMsg.rdmMsgType(LoginMsgType.STATUS);
                Buffer membuf = new();
                membuf.Data(new ByteBuffer(1024));

                var msgFlags = flags;

                if ((msgFlags & LoginStatusFlags.HAS_USERNAME) == 0)
                    msgFlags &= ~LoginStatusFlags.HAS_USERNAME_TYPE;

                encRDMMsg.Flags = msgFlags;
                encRDMMsg.StreamId = streamId;

                if (encRDMMsg.HasState)
                {
                    encRDMMsg.State.Code(state.Code());
                    encRDMMsg.State.DataState(state.DataState());
                    encRDMMsg.State.Text().Data("state");
                    encRDMMsg.State.StreamState(state.StreamState());
                }
                if (encRDMMsg.HasUserName)
                {
                    encRDMMsg.UserName.Data(userName);
                    if (encRDMMsg.HasUserNameType)
                        encRDMMsg.UserNameType = userNameType;
                }
                // todo: why?
                if (encRDMMsg.ClearCache)
                {
                    encRDMMsg.ClearCache = true;
                }
                if (encRDMMsg.HasAuthenticationErrorCode)
                {
                    encRDMMsg.AuthenticationErrorCode = authenticationErrorCode;
                }
                if (encRDMMsg.HasAuthenticationErrorText)
                {
                    encRDMMsg.AuthenticationErrorText.Data(authenticationErrorText);
                }
                encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                var ret = encRDMMsg.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = msg.Decode(dIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                ret = decRDMMsg.Decode(dIter, msg);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(streamId, decRDMMsg.StreamId);
                Assert.Equal(encRDMMsg.Flags, decRDMMsg.Flags);
                Assert.Equal(encRDMMsg.ClearCache, decRDMMsg.ClearCache);

                //check parameters
                if (decRDMMsg.HasUserName)
                {
                    Assert.Equal(userName.ToString(), decRDMMsg.UserName.ToString());
                    if (decRDMMsg.HasUserNameType)
                        Assert.Equal(userNameType, decRDMMsg.UserNameType);
                }

                if (decRDMMsg.HasAuthenticationErrorCode)
                {
                    Assert.Equal(authenticationErrorCode, decRDMMsg.AuthenticationErrorCode);
                }
                if (encRDMMsg.HasAuthenticationErrorText)
                {
                    encRDMMsg.AuthenticationErrorText.Data(authenticationErrorText);
                }
                if (decRDMMsg.HasState)
                {
                    State decState = decRDMMsg.State;
                    Assert.NotNull(decState);
                    Assert.Equal(state.Code(), decState.Code());
                    Assert.Equal(state.DataState(), decState.DataState());
                    Assert.Equal(state.StreamState(), decState.StreamState());
                    Assert.Equal(state.Text().ToString(), decState.Text().ToString());
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginStatusCopyTest()
        {
            LoginStatus statusRDMMsg1 = new();
            //statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);
            LoginStatus statusRDMMsg2 = new();
            //statusRDMMsg2.rdmMsgType(LoginMsgType.STATUS);

            Console.WriteLine("LoginStatus copy test...");

            //parameters setup
            int streamId = -5;
            string userName = "userName";
            var userNameType = UserIdTypes.NAME;
            long authenticationErrorCode = 404;
            string authenticationErrorText = "some kind of authenticationErrorText";

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            //status msg setup
            var flags = allStatusMsgFlags;
            statusRDMMsg1.Clear();
            //statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);
            statusRDMMsg1.Flags = flags;
            statusRDMMsg1.StreamId = streamId;
            Assert.True(statusRDMMsg1.ClearCache);

            Assert.True(statusRDMMsg1.HasState);
            statusRDMMsg1.State.Code(state.Code());
            statusRDMMsg1.State.DataState(state.DataState());
            statusRDMMsg1.State.Text().Data("state");
            statusRDMMsg1.State.StreamState(state.StreamState());

            Assert.True(statusRDMMsg1.HasUserName);
            statusRDMMsg1.UserName.Data(userName);

            Assert.True(statusRDMMsg1.HasUserNameType);
            statusRDMMsg1.UserNameType = userNameType;

            Assert.True(statusRDMMsg1.HasAuthenticationErrorCode);
            statusRDMMsg1.AuthenticationErrorCode = authenticationErrorCode;
            Assert.True(statusRDMMsg1.HasAuthenticationErrorCode);
            statusRDMMsg1.AuthenticationErrorText.Data(authenticationErrorText);

            var ret = statusRDMMsg1.Copy(statusRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.Equal(streamId, statusRDMMsg2.StreamId);
            Assert.Equal(flags, statusRDMMsg2.Flags);
            Assert.Equal(userName, statusRDMMsg2.UserName.ToString());
            Assert.Equal(userNameType, statusRDMMsg2.UserNameType);
            Assert.Equal(statusRDMMsg1.ClearCache, statusRDMMsg2.ClearCache);

            State refState1 = statusRDMMsg1.State;
            State refState2 = statusRDMMsg2.State;
            Assert.NotNull(refState2);
            Assert.Equal(refState1.Code(), refState2.Code());
            Assert.Equal(refState1.DataState(), refState2.DataState());
            Assert.Equal(refState1.StreamState(), refState2.StreamState());
            Assert.Equal(refState1.Text().ToString(), refState2.Text().ToString());
            Assert.True(refState1.Text() != refState2.Text());

            Assert.Equal(authenticationErrorCode, statusRDMMsg2.AuthenticationErrorCode);
            Assert.Equal(authenticationErrorText.ToString(), statusRDMMsg2.AuthenticationErrorText.ToString());

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginStatusBlankTest()
        {
            LoginStatus statusEnc = new();
            //statusEnc.rdmMsgType(LoginMsgType.STATUS);
            LoginStatus statusDec = new();
            //statusDec.rdmMsgType(LoginMsgType.STATUS);
            zeroLengthBuf.Data("");

            Console.WriteLine("LoginStatus blank test...");

            // Test 1: Encode side test, encoder is not sending blank buffers
            //parameters setup
            int streamId = -5;
            long authenticationErrorCode = 404;
            string authenticationErrorText = "";

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            //status msg setup
            statusEnc.Clear();
            //statusEnc.rdmMsgType(LoginMsgType.STATUS);
            statusEnc.Flags = 0;
            statusEnc.StreamId = streamId;

            statusEnc.State.Code(state.Code());
            statusEnc.State.DataState(state.DataState());
            statusEnc.State.Text().Data("state");
            statusEnc.State.StreamState(state.StreamState());

            statusEnc.HasAuthenticationErrorCode = true;
            statusEnc.AuthenticationErrorCode = authenticationErrorCode;
            statusEnc.HasAuthenticationErrorText = true;
            statusEnc.AuthenticationErrorText.Data(authenticationErrorText);

            dIter.Clear();
            encIter.Clear();

            Buffer membuf = new();
            membuf.Data(new ByteBuffer(1024));

            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            var ret = statusEnc.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(),
                    Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = statusDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.True(!statusDec.HasAuthenticationErrorText);
            Assert.True(!statusDec.HasAuthenticationErrorText);

            // Check decode side is properly erroring out
            // Authentication Error Text
            IStatusMsg statusMsg = new Msg();
            statusMsg.Clear();

            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.LOGIN;

            statusMsg.ApplyHasMsgKey();
            statusMsg.MsgKey.ApplyHasAttrib();
            statusMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;


            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = statusMsg.EncodeInit(encIter, 0);
            Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, ret);
            element.Clear();
            elementList.Clear();
            elementList.ApplyHasStandardData();
            ret = elementList.EncodeInit(encIter, null, 0);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            element.DataType = DataTypes.ASCII_STRING;
            element.Name = ElementNames.AUTHN_ERROR_TEXT;
            ret = element.Encode(encIter, zeroLengthBuf);

            ret = elementList.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = statusMsg.EncodeKeyAttribComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            ret = statusMsg.EncodeComplete(encIter, true);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(),
                    Codec.Codec.MinorVersion());
            statusDec.Clear();
            ret = msg.Decode(dIter);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = statusDec.Decode(dIter, msg);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.True(statusDec.HasAuthenticationErrorText);
            Assert.Equal(0, statusDec.AuthenticationErrorText.Length);

            Console.WriteLine("Done.");
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void LoginStatusToStringTest()
        {
            LoginStatus statusRDMMsg1 = new();
            //statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);

            Console.WriteLine("LoginStatus toString test...");

            //parameters setup
            int streamId = -5;
            string userName = "userName";
            var userNameType = UserIdTypes.NAME;
            long authenticationErrorCode = 404;
            string authenticationErrorText = "some kind of authenticationErrorText";

            State state = new();
            Buffer buffer = new();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            //status msg setup
            var flags = allStatusMsgFlags;
            statusRDMMsg1.Clear();
            //statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);
            statusRDMMsg1.Flags = flags;
            statusRDMMsg1.StreamId = streamId;
            Assert.True(statusRDMMsg1.ClearCache);

            Assert.True(statusRDMMsg1.HasState);
            statusRDMMsg1.State.Code(state.Code());
            statusRDMMsg1.State.DataState(state.DataState());
            statusRDMMsg1.State.Text().Data("state");
            statusRDMMsg1.State.StreamState(state.StreamState());

            Assert.True(statusRDMMsg1.HasUserName);
            statusRDMMsg1.UserName.Data(userName);
            Assert.True(statusRDMMsg1.HasUserNameType);
            statusRDMMsg1.UserNameType = userNameType;

            Assert.True(statusRDMMsg1.HasAuthenticationErrorCode);
            statusRDMMsg1.AuthenticationErrorCode = authenticationErrorCode;
            Assert.True(statusRDMMsg1.HasAuthenticationErrorText);
            statusRDMMsg1.AuthenticationErrorText.Data(authenticationErrorText);

            Assert.NotNull(statusRDMMsg1.ToString());
            Console.WriteLine(statusRDMMsg1.ToString());
            Console.WriteLine("Done.");
        }
        #endregion
    }
}
