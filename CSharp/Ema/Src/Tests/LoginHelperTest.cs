/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;

using LSEG.Ema.Domain.Login;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;

namespace LSEG.Ema.Access.Tests;

public class LoginHelperTest
{
    [Fact]
    public void EncodeLoginRequest_Test()
    {
        LoginReq loginReq = new LoginReq();

        loginReq.Name("UserName");
        Assert.True(loginReq.HasName);
        Assert.Equal("UserName", loginReq.Name());

        loginReq.Pause(true);
        Assert.True(loginReq.HasPause);
        Assert.True(loginReq.Pause());

        loginReq.AllowSuspectData(true);
        Assert.True(loginReq.HasAllowSuspectData);
        Assert.True(loginReq.AllowSuspectData());

        loginReq.DownloadConnectionConfig(true);
        Assert.True(loginReq.HasDownloadConnectionConfig);
        Assert.True(loginReq.DownloadConnectionConfig());

        loginReq.ApplicationId("123");
        Assert.True(loginReq.HasApplicationId);
        Assert.Equal("123", loginReq.ApplicationId());

        loginReq.ApplicationName("application name test");
        Assert.True(loginReq.HasApplicationName);
        Assert.Equal("application name test", loginReq.ApplicationName());

        loginReq.ApplicationAuthorizationToken("application authentication token");
        Assert.True(loginReq.HasApplicationAuthorizationToken);
        Assert.Equal("application authentication token", loginReq.ApplicationAuthorizationToken());

        loginReq.InstanceId("555");
        Assert.True(loginReq.HasInstanceId);
        Assert.Equal("555", loginReq.InstanceId());

        loginReq.Password("@password");
        Assert.True(loginReq.HasPassword);
        Assert.Equal("@password", loginReq.Password());

        loginReq.Position("127.0.0.1/net");
        Assert.True(loginReq.HasPosition);
        Assert.Equal("127.0.0.1/net", loginReq.Position());

        loginReq.ProvidePermissionExpressions(true);
        Assert.True(loginReq.HasProvidePermissionExpressions);
        Assert.True(loginReq.ProvidePermissionExpressions());

        loginReq.ProvidePermissionProfile(true);
        Assert.True(loginReq.HasProvidePermissionProfile);
        Assert.True(loginReq.ProvidePermissionProfile());

        loginReq.Role(EmaRdm.LOGIN_ROLE_PROV);
        Assert.True(loginReq.HasRole);
        Assert.Equal(EmaRdm.LOGIN_ROLE_PROV, loginReq.Role());

        loginReq.SingleOpen(true);
        Assert.True(loginReq.HasSingleOpen);
        Assert.True(loginReq.SingleOpen());

        loginReq.SupportProviderDictionaryDownload(true);
        Assert.True(loginReq.HasSupportProviderDictionaryDownload);
        Assert.True(loginReq.SupportProviderDictionaryDownload());

        EmaBuffer authenticationExtended = new EmaBuffer(Encoding.ASCII.GetBytes("authenticationExtended"));

        loginReq.AuthenticationExtended(authenticationExtended);
        Assert.True(loginReq.HasAuthenticationExtended);
        Assert.Equal(authenticationExtended, loginReq.AuthenticationExtended());

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RequestMsg decReqMsg = new RequestMsg();

        var msg = loginReq.Message();
        decReqMsg.Decode(msg.m_rsslMsg, Codec.MajorVersion(), Codec.MinorVersion(), dictionary);

        Assert.True(decReqMsg.Pause());

        Ema.Access.ElementList decodedEl = new ElementList();

        decodedEl.Decode(Codec.MajorVersion(), Codec.MinorVersion(), decReqMsg.Attrib().ElementList().m_bodyBuffer!, dictionary, null);

        string? elementName;

        foreach (ElementEntry elementEntry in decodedEl)
        {
            elementName = elementEntry.Name;

            switch (elementName)
            {
                case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_APP_ID:
                    Assert.Equal("123", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_APP_NAME:
                    Assert.Equal("application name test", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_APPAUTH_TOKEN:
                    Assert.Equal("application authentication token", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_DOWNLOAD_CON_CONFIG:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_INST_ID:
                    Assert.Equal("555", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_PASSWORD:
                    Assert.Equal("@password", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_POSITION:
                    Assert.Equal("127.0.0.1/net", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_PROV_PERM_EXP:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_PROV_PERM_PROF:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_ROLE:
                    Assert.Equal((ulong)EmaRdm.LOGIN_ROLE_PROV, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SINGLE_OPEN:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_AUTHN_EXTENDED:
                    Assert.Equal(authenticationExtended, elementEntry.OmmBufferValue().Value);
                    break;
                default:
                    break;
            }
        }

        loginReq.Clear();
        decReqMsg.Clear();
        decodedEl.Clear();
        msg.Clear();
    }

    [Fact]
    public void HeaderLoginReq_Test()
    {
        LoginReq loginReq = new LoginReq();

        loginReq.Name("UserName");
        loginReq.NameType(EmaRdm.USER_AUTH_TOKEN);

        Assert.True(loginReq.HasName);
        Assert.Equal("UserName", loginReq.Name());

        Assert.True(loginReq.HasNameType);
        Assert.Equal(EmaRdm.USER_AUTH_TOKEN, loginReq.NameType());

        RequestMsg reqMsg = loginReq.Message();

        Assert.Equal("\0",reqMsg.Name());

        reqMsg.Clear();
        loginReq.Clear();
    }

    [Fact]
    public void ClearLoginReq_Test()
    {
        LoginReq loginReq = new LoginReq();

        loginReq.AllowSuspectData(false);
        loginReq.DownloadConnectionConfig(true);
        loginReq.ProvidePermissionProfile(false);
        loginReq.ProvidePermissionExpressions(false);
        loginReq.SingleOpen(false);
        loginReq.SupportProviderDictionaryDownload(true);
        loginReq.Role(EmaRdm.LOGIN_ROLE_PROV);
        loginReq.ApplicationId("AppId");
        loginReq.ApplicationName("AppName");
        loginReq.ApplicationAuthorizationToken("AppAuthToken");
        loginReq.InstanceId("InstanceId");
        loginReq.Password("12345");
        loginReq.Position("Position");
        loginReq.AuthenticationExtended(new EmaBuffer(Encoding.ASCII.GetBytes("AuthExtended")));
        loginReq.Name("Name");
        loginReq.NameType(EmaRdm.USER_EMAIL_ADDRESS);
        loginReq.Pause(true);

        Assert.True(loginReq.HasAllowSuspectData);
        Assert.False(loginReq.AllowSuspectData());
        Assert.True(loginReq.HasDownloadConnectionConfig);
        Assert.True(loginReq.DownloadConnectionConfig());
        Assert.True(loginReq.HasProvidePermissionProfile);
        Assert.False(loginReq.ProvidePermissionProfile());
        Assert.True(loginReq.HasProvidePermissionExpressions);
        Assert.False(loginReq.ProvidePermissionExpressions());
        Assert.True(loginReq.HasSingleOpen);
        Assert.False(loginReq.SingleOpen());
        Assert.True(loginReq.HasSupportProviderDictionaryDownload);
        Assert.True(loginReq.SupportProviderDictionaryDownload());
        Assert.True(loginReq.HasRole);
        Assert.Equal(EmaRdm.LOGIN_ROLE_PROV, loginReq.Role());
        Assert.True(loginReq.HasApplicationId);
        Assert.Equal("AppId", loginReq.ApplicationId());
        Assert.True(loginReq.HasApplicationName);
        Assert.Equal("AppName", loginReq.ApplicationName());
        Assert.True(loginReq.HasApplicationAuthorizationToken);
        Assert.Equal("AppAuthToken", loginReq.ApplicationAuthorizationToken());
        Assert.True(loginReq.HasInstanceId);
        Assert.Equal("InstanceId", loginReq.InstanceId());
        Assert.True(loginReq.HasPassword);
        Assert.Equal("12345", loginReq.Password());
        Assert.True(loginReq.HasPosition);
        Assert.Equal("Position", loginReq.Position());
        Assert.True(loginReq.HasAuthenticationExtended);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes("AuthExtended")),
            loginReq.AuthenticationExtended());
        Assert.True(loginReq.HasName);
        Assert.Equal("Name", loginReq.Name());
        Assert.True(loginReq.HasNameType);
        Assert.Equal(EmaRdm.USER_EMAIL_ADDRESS, loginReq.NameType());
        Assert.True(loginReq.HasPause);
        Assert.True(loginReq.Pause());

        loginReq.Clear();

        Assert.True(loginReq.HasAllowSuspectData);
        Assert.True(loginReq.AllowSuspectData());
        Assert.False(loginReq.HasDownloadConnectionConfig);
        Assert.False(loginReq.DownloadConnectionConfig());
        Assert.True(loginReq.HasProvidePermissionProfile);
        Assert.True(loginReq.ProvidePermissionProfile());
        Assert.True(loginReq.HasProvidePermissionExpressions);
        Assert.True(loginReq.ProvidePermissionExpressions());
        Assert.True(loginReq.HasSingleOpen);
        Assert.True(loginReq.SingleOpen());
        Assert.False(loginReq.HasSupportProviderDictionaryDownload);
        Assert.False(loginReq.SupportProviderDictionaryDownload());
        Assert.True(loginReq.HasRole);
        Assert.Equal(EmaRdm.LOGIN_ROLE_CONS, loginReq.Role());
        Assert.True(loginReq.HasApplicationId);
        Assert.True(loginReq.HasApplicationName);
        Assert.False(loginReq.HasApplicationAuthorizationToken);
        Assert.False(loginReq.HasInstanceId);
        Assert.False(loginReq.HasPassword);
        Assert.True(loginReq.HasPosition);
        Assert.False(loginReq.HasAuthenticationExtended);
        Assert.True(loginReq.HasName);
        Assert.True(loginReq.HasNameType);
        Assert.Equal(EmaRdm.USER_NAME, loginReq.NameType());
        Assert.False(loginReq.HasPause);
        Assert.False(loginReq.Pause());

        loginReq.Clear();
    }

    [Fact]
    public void DecodeLoginRequest_Test()
    {
        ElementList encodedElementList = new ElementList();

        encodedElementList.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
        encodedElementList.AddAscii(EmaRdm.ENAME_APP_ID, "123");
        encodedElementList.AddAscii(EmaRdm.ENAME_APP_NAME, "application name test");
        encodedElementList.AddAscii(EmaRdm.ENAME_APPAUTH_TOKEN, "application authentication token");
        encodedElementList.AddUInt(EmaRdm.ENAME_DOWNLOAD_CON_CONFIG, 1);
        encodedElementList.AddAscii(EmaRdm.ENAME_INST_ID, "555");
        encodedElementList.AddAscii(EmaRdm.ENAME_PASSWORD, "@password");
        encodedElementList.AddAscii(EmaRdm.ENAME_POSITION, "127.0.0.1/net");
        encodedElementList.AddUInt(EmaRdm.ENAME_PROV_PERM_EXP, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_PROV_PERM_PROF, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_ROLE, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, 1);
        EmaBuffer authenticationExtended = new EmaBuffer(Encoding.ASCII.GetBytes("authenticationExtended"));
        encodedElementList.AddBuffer(EmaRdm.ENAME_AUTHN_EXTENDED, authenticationExtended);
        encodedElementList.Complete();

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RequestMsg encReqMsg = new RequestMsg();

        encReqMsg.DomainType(EmaRdm.MMT_LOGIN);
        encReqMsg.Attrib(encodedElementList);
        encReqMsg.EncodeComplete();

        RequestMsg decReqMsg = new RequestMsg();

        decReqMsg.Decode(encReqMsg.m_requestMsgEncoder.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginReq loginReq = new LoginReq();

        loginReq.Message(decReqMsg);

        Assert.True(loginReq.HasAllowSuspectData);
        Assert.True(loginReq.AllowSuspectData());

        Assert.True(loginReq.HasDownloadConnectionConfig);
        Assert.True(loginReq.DownloadConnectionConfig());

        Assert.True(loginReq.HasApplicationId);
        Assert.Equal("123", loginReq.ApplicationId());

        Assert.True(loginReq.HasApplicationName);
        Assert.Equal("application name test", loginReq.ApplicationName());

        Assert.True(loginReq.HasApplicationAuthorizationToken);
        Assert.Equal("application authentication token", loginReq.ApplicationAuthorizationToken());

        Assert.True(loginReq.HasInstanceId);
        Assert.Equal("555", loginReq.InstanceId());

        Assert.True(loginReq.HasPassword);
        Assert.Equal("@password", loginReq.Password());

        Assert.True(loginReq.HasPosition);
        Assert.Equal("127.0.0.1/net", loginReq.Position());

        Assert.True(loginReq.HasProvidePermissionExpressions);
        Assert.True(loginReq.ProvidePermissionExpressions());

        Assert.True(loginReq.HasProvidePermissionProfile);
        Assert.True(loginReq.ProvidePermissionProfile());

        Assert.True(loginReq.HasRole);
        Assert.Equal(EmaRdm.LOGIN_ROLE_PROV, loginReq.Role());

        Assert.True(loginReq.HasSingleOpen);
        Assert.True(loginReq.SingleOpen());

        Assert.True(loginReq.HasSupportProviderDictionaryDownload);
        Assert.True(loginReq.SupportProviderDictionaryDownload());

        Assert.True(loginReq.HasAuthenticationExtended);
        Assert.Equal(authenticationExtended, loginReq.AuthenticationExtended());

        encodedElementList.Clear();
        encReqMsg.Clear();
        decReqMsg.Clear();
        loginReq.Clear();
    }

    [Fact]
    public void BlankLoginRequest_Test()
    {
        ElementList encodedElementList = new ElementList();

        encodedElementList.AddAscii(EmaRdm.ENAME_APP_ID, "");
        encodedElementList.AddAscii(EmaRdm.ENAME_APP_NAME, "");
        encodedElementList.AddAscii(EmaRdm.ENAME_APPAUTH_TOKEN, "");
        encodedElementList.AddAscii(EmaRdm.ENAME_INST_ID, "");
        encodedElementList.AddAscii(EmaRdm.ENAME_PASSWORD, "");
        encodedElementList.AddAscii(EmaRdm.ENAME_POSITION, "");

        EmaBuffer authenticationExtended = new EmaBuffer();
        encodedElementList.AddBuffer(EmaRdm.ENAME_AUTHN_EXTENDED, authenticationExtended);

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RequestMsg encReqMsg = new RequestMsg();

        encReqMsg.DomainType(EmaRdm.MMT_LOGIN);
        encReqMsg.Attrib(encodedElementList.Complete());
        encReqMsg.EncodeComplete();

        RequestMsg decReqMsg = new RequestMsg();

        decReqMsg.Decode(encReqMsg.m_requestMsgEncoder.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginReq loginReq = new LoginReq();
        loginReq.ApplicationId("123");
        loginReq.ApplicationName("name");
        loginReq.ApplicationAuthorizationToken("token");
        loginReq.InstanceId("id");
        loginReq.Password("password");
        loginReq.Position("123.456.789.012");
        authenticationExtended = new EmaBuffer(Encoding.ASCII.GetBytes("tests"));
        loginReq.AuthenticationExtended(authenticationExtended);

        loginReq.Message(decReqMsg);

        Assert.False(loginReq.HasApplicationId);
        Assert.False(loginReq.HasApplicationName);
        Assert.False(loginReq.HasApplicationAuthorizationToken);
        Assert.False(loginReq.HasInstanceId);
        Assert.False(loginReq.HasPassword);
        Assert.False(loginReq.HasPosition);
        Assert.False(loginReq.HasAuthenticationExtended);

        encodedElementList.Clear();
        encReqMsg.Clear();
        decReqMsg.Clear();
        loginReq.Clear();
    }

    [Fact]
    public void ErrorHandlingLoginRequest_Test()
    {
        LoginReq loginReq = new LoginReq();

        try
        {
            loginReq.ApplicationAuthorizationToken();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("ApplicationAuthorizationToken element is not set", ex.Message);
        }

        try
        {
            loginReq.InstanceId();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("InstanceId element is not set", ex.Message);
        }

        try
        {
            loginReq.Password();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("Password element is not set", ex.Message);
        }

        try
        {
            // NOTE departure from EMAJ TODO: LoginReq.Clear() method sets Position,
            // ApplicationName, ApplicationId, and Name properties to their DEFAULT
            // values. This automatically turns corresponding Has***Set indicators to
            // TRUE.
            //
            // In the end, scenario tested here doesn't throw an exception, but returns
            // DEFAULT_POSITION value instead.
            loginReq.Position();
            // Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("Position element is not set", ex.Message);
        }

        try
        {
            loginReq.AuthenticationExtended();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("AuthenticationExtended element is not set", ex.Message);
        }

        loginReq.Clear();
    }

    [Fact]
    public void DecodeLoginReqInvalidType_Test()
    {
        ElementList encodedElementList = new ElementList();

        encodedElementList.AddAscii(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, "1");
        encodedElementList.Complete();

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RequestMsg encReqMsg = new RequestMsg();

        encReqMsg.DomainType(EmaRdm.MMT_LOGIN);
        encReqMsg.Attrib(encodedElementList);
        encReqMsg.EncodeComplete();

        RequestMsg decReqMsg = new RequestMsg();

        decReqMsg.Decode(encReqMsg.m_requestMsgEncoder.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginReq loginReq = new LoginReq();

        try
        {
            loginReq.Message(decReqMsg);
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("Decoding error for AllowSuspectData element. Attempt to uintValue() while actual entry data type is Ascii", ex.Message);
        }

        encodedElementList.Clear();
        loginReq.Clear();
        decReqMsg.Clear();
        encReqMsg.Clear();
    }

    [Fact]
    public void EncodeLoginRefresh_Test()
    {
        LoginRefresh loginRefresh = new LoginRefresh();

        // NOTE departure from EMAJ TODO: without State information ETA RefreshMsg Encode returns INVALID_DATA
        loginRefresh.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE,
            "StatusText");

        loginRefresh.Solicited(true);

        Assert.True(loginRefresh.HasSolicited);

        Assert.True(loginRefresh.Solicited());

        loginRefresh.AllowSuspectData(true);
        Assert.True(loginRefresh.HasAllowSuspectData);
        Assert.True(loginRefresh.AllowSuspectData());

        loginRefresh.ApplicationId("123");
        Assert.True(loginRefresh.HasApplicationId);
        Assert.Equal("123", loginRefresh.ApplicationId());

        loginRefresh.ApplicationName("application name test");
        Assert.True(loginRefresh.HasApplicationName);
        Assert.Equal("application name test", loginRefresh.ApplicationName());

        loginRefresh.Position("127.0.0.1/net");
        Assert.True(loginRefresh.HasPosition);
        Assert.Equal("127.0.0.1/net", loginRefresh.Position());

        loginRefresh.ProvidePermissionExpressions(true);
        Assert.True(loginRefresh.HasProvidePermissionExpressions);
        Assert.True(loginRefresh.ProvidePermissionExpressions());

        loginRefresh.ProvidePermissionProfile(true);
        Assert.True(loginRefresh.HasProvidePermissionProfile);
        Assert.True(loginRefresh.ProvidePermissionProfile());

        loginRefresh.SingleOpen(true);
        Assert.True(loginRefresh.HasSingleOpen);
        Assert.True(loginRefresh.SingleOpen());

        loginRefresh.SupportBatchRequests(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE);
        Assert.True(loginRefresh.HasSupportBatchRequests);
        Assert.Equal(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE, loginRefresh.SupportBatchRequests());

        loginRefresh.SupportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS);
        Assert.True(loginRefresh.HasSupportEnhancedSymbolList);
        Assert.Equal(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS, loginRefresh.SupportEnhancedSymbolList());

        loginRefresh.SupportOMMPost(true);
        Assert.True(loginRefresh.HasSupportOMMPost);
        Assert.True(loginRefresh.SupportOMMPost());

        loginRefresh.SupportOptimizedPauseResume(true);
        Assert.True(loginRefresh.HasSupportOptimizedPauseResume);
        Assert.True(loginRefresh.SupportOptimizedPauseResume());

        loginRefresh.SupportProviderDictionaryDownload(true);
        Assert.True(loginRefresh.HasSupportProviderDictionaryDownload);
        Assert.True(loginRefresh.SupportProviderDictionaryDownload());

        loginRefresh.SupportViewRequests(true);
        Assert.True(loginRefresh.HasSupportViewRequests);
        Assert.True(loginRefresh.SupportViewRequests());

        loginRefresh.SupportStandby(true);
        Assert.True(loginRefresh.HasSupportStandby);
        Assert.True(loginRefresh.SupportStandby());

        EmaBuffer authenticationExtended = new EmaBuffer(Encoding.ASCII.GetBytes("authenticationExtended"));

        loginRefresh.AuthenticationExtendedResp(authenticationExtended);
        Assert.True(loginRefresh.HasAuthenticationExtended);
        Assert.Equal(authenticationExtended, loginRefresh.AuthenticationExtended());

        loginRefresh.AuthenticationTTReissue(2);
        Assert.True(loginRefresh.HasAuthenticationTTReissue);
        Assert.Equal(2u, loginRefresh.AuthenticationTTReissue());

        loginRefresh.AuthenticationErrorCode(3);
        Assert.True(loginRefresh.HasAuthenticationErrorCode);
        Assert.Equal(3u, loginRefresh.AuthenticationErrorCode());

        loginRefresh.AuthenticationErrorText("authenticationErrorText");
        Assert.True(loginRefresh.HasAuthenticationErrorText);
        Assert.Equal("authenticationErrorText", loginRefresh.AuthenticationErrorText());

        RefreshMsg encRefreshMsg = loginRefresh.Message();

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RefreshMsg decRefreshMsg = new RefreshMsg();

        decRefreshMsg.Decode(encRefreshMsg.m_refreshMsgEncoder.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        Ema.Access.ElementList decodedEl = new ElementList();

        decodedEl.Decode(Codec.MajorVersion(), Codec.MinorVersion(), decRefreshMsg.Attrib().ElementList().m_bodyBuffer!, dictionary, null);

        string? elementName;

        foreach (ElementEntry elementEntry in decodedEl)
        {
            elementName = elementEntry.Name;

            switch (elementName)
            {
                case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_APP_ID:
                    Assert.Equal("123", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_APP_NAME:
                    Assert.Equal("application name test", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_POSITION:
                    Assert.Equal("127.0.0.1/net", elementEntry.OmmAsciiValue().Value);
                    break;
                case EmaRdm.ENAME_PROV_PERM_EXP:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_PROV_PERM_PROF:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SINGLE_OPEN:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SUPPORT_BATCH:
                    Assert.Equal(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE,
                        (int)elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SUPPORT_POST:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SUPPORT_OPR:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SUPPORT_VIEW:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_SUPPORT_STANDBY:
                    Assert.Equal(1u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_AUTHN_EXTENDED:
                    Assert.Equal(authenticationExtended, elementEntry.OmmBufferValue().Value);
                    break;
                case EmaRdm.ENAME_AUTHN_TT_REISSUE:
                    Assert.Equal(2u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_AUTHN_ERRORCODE:
                    Assert.Equal(3u, elementEntry.UIntValue());
                    break;
                case EmaRdm.ENAME_AUTHN_ERRORTEXT:
                    Assert.Equal("authenticationErrorText", elementEntry.OmmAsciiValue().Value);
                    break;
                default:
                    break;
            }
        }

        loginRefresh.Clear();
        decodedEl.Clear();
        decRefreshMsg.Clear();
        encRefreshMsg.Clear();
    }

    [Fact]
    public void DecodeLoginRefresh_Test()
    {
        ElementList encodedElementList = new ElementList();

        encodedElementList.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
        encodedElementList.AddAscii(EmaRdm.ENAME_APP_ID, "123");
        encodedElementList.AddAscii(EmaRdm.ENAME_APP_NAME, "application name test");
        encodedElementList.AddAscii(EmaRdm.ENAME_POSITION, "127.0.0.1/net");
        encodedElementList.AddUInt(EmaRdm.ENAME_PROV_PERM_EXP, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_PROV_PERM_PROF, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_BATCH, 7);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_OPR, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_VIEW, 1);
        encodedElementList.AddUInt(EmaRdm.ENAME_SUPPORT_STANDBY, 1);
        EmaBuffer authenticationExtended = new EmaBuffer(Encoding.ASCII.GetBytes("authenticationExtended"));
        encodedElementList.AddBuffer(EmaRdm.ENAME_AUTHN_EXTENDED_RESP, authenticationExtended);
        encodedElementList.AddUInt(EmaRdm.ENAME_AUTHN_TT_REISSUE, 2);
        encodedElementList.AddUInt(EmaRdm.ENAME_AUTHN_ERRORCODE, 3);
        encodedElementList.AddAscii(EmaRdm.ENAME_AUTHN_ERRORTEXT, "authenticationErrorText");
        encodedElementList.Complete();

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RefreshMsg encRefreshMsg = new RefreshMsg();
        // NOTE departure from EMAJ TODO: without State information ETA RefreshMsg Encode returns INVALID_DATA
        encRefreshMsg.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE,
            "StatusText");
        encRefreshMsg.DomainType(EmaRdm.MMT_LOGIN);
        encRefreshMsg.Attrib(encodedElementList);
        encRefreshMsg.EncodeComplete();

        RefreshMsg decRefreshMsg = new RefreshMsg();

        decRefreshMsg.Decode(encRefreshMsg.m_refreshMsgEncoder.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginRefresh loginRefresh = new LoginRefresh();

        loginRefresh.Message(decRefreshMsg);

        Assert.True(loginRefresh.HasAllowSuspectData);
        Assert.True(loginRefresh.AllowSuspectData());

        Assert.True(loginRefresh.HasApplicationId);
        Assert.Equal("123", loginRefresh.ApplicationId());

        Assert.True(loginRefresh.HasApplicationName);
        Assert.Equal("application name test", loginRefresh.ApplicationName());

        Assert.True(loginRefresh.HasPosition);
        Assert.Equal("127.0.0.1/net", loginRefresh.Position());

        Assert.True(loginRefresh.HasProvidePermissionExpressions);
        Assert.True(loginRefresh.ProvidePermissionExpressions());

        Assert.True(loginRefresh.HasProvidePermissionProfile);
        Assert.True(loginRefresh.ProvidePermissionProfile());

        Assert.True(loginRefresh.HasSingleOpen);
        Assert.True(loginRefresh.SingleOpen());

        Assert.True(loginRefresh.HasSupportBatchRequests);
        Assert.Equal(EmaRdm.SUPPORT_BATCH_REQUEST | EmaRdm.SUPPORT_BATCH_REISSUE | EmaRdm.SUPPORT_BATCH_CLOSE,
            (int)loginRefresh.SupportBatchRequests());
        Assert.True(loginRefresh.HasSupportEnhancedSymbolList);
        Assert.Equal(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS, loginRefresh.SupportEnhancedSymbolList());

        Assert.True(loginRefresh.HasSupportOMMPost);
        Assert.True(loginRefresh.SupportOMMPost());

        Assert.True(loginRefresh.HasSupportOptimizedPauseResume);
        Assert.True(loginRefresh.SupportOptimizedPauseResume());

        Assert.True(loginRefresh.HasSupportProviderDictionaryDownload);
        Assert.True(loginRefresh.SupportProviderDictionaryDownload());

        Assert.True(loginRefresh.HasSupportViewRequests);
        Assert.True(loginRefresh.SupportViewRequests());

        Assert.True(loginRefresh.HasSupportStandby);
        Assert.True(loginRefresh.SupportStandby());

        Assert.True(loginRefresh.HasAuthenticationExtended);
        Assert.Equal(authenticationExtended, loginRefresh.AuthenticationExtended());

        Assert.True(loginRefresh.HasAuthenticationTTReissue);
        Assert.Equal(2u, loginRefresh.AuthenticationTTReissue());

        Assert.True(loginRefresh.HasAuthenticationErrorCode);
        Assert.Equal(3u, loginRefresh.AuthenticationErrorCode());

        Assert.True(loginRefresh.HasAuthenticationErrorText);
        Assert.Equal("authenticationErrorText", loginRefresh.AuthenticationErrorText());

        encodedElementList.Clear();
        encRefreshMsg.Clear();
        decRefreshMsg.Clear();
        loginRefresh.Clear();
    }

    [Fact]
    public void BlankLoginRefresh_Test()
    {
        ElementList encodedElementList = new ElementList();

        encodedElementList.AddAscii(EmaRdm.ENAME_APP_ID, "");
        encodedElementList.AddAscii(EmaRdm.ENAME_APP_NAME, "");
        encodedElementList.AddAscii(EmaRdm.ENAME_POSITION, "");
        EmaBuffer authenticationExtended = new EmaBuffer();
        encodedElementList.AddBuffer(EmaRdm.ENAME_AUTHN_EXTENDED_RESP, authenticationExtended);
        encodedElementList.AddAscii(EmaRdm.ENAME_AUTHN_ERRORTEXT, "");
        encodedElementList.Complete();

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RefreshMsg encRefreshMsg = new RefreshMsg();
        // NOTE departure from EMAJ TODO: without State information ETA RefreshMsg Encode returns INVALID_DATA
        encRefreshMsg.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE,
            "StatusText");
        encRefreshMsg.DomainType(EmaRdm.MMT_LOGIN);
        encRefreshMsg.Attrib(encodedElementList);
        encRefreshMsg.EncodeComplete();

        RefreshMsg decRefreshMsg = new RefreshMsg();

        decRefreshMsg.Decode(encRefreshMsg.m_refreshMsgEncoder.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginRefresh loginRefresh = new LoginRefresh();

        loginRefresh.ApplicationId("123");
        loginRefresh.ApplicationName("name");
        loginRefresh.Position("123.456.789.012");
        authenticationExtended = new EmaBuffer(Encoding.ASCII.GetBytes("tests"));
        loginRefresh.AuthenticationExtendedResp(authenticationExtended);
        loginRefresh.AuthenticationErrorText("error");

        loginRefresh.Message(decRefreshMsg);

        Assert.False(loginRefresh.HasApplicationId);
        Assert.False(loginRefresh.HasApplicationName);
        Assert.False(loginRefresh.HasPosition);
        Assert.False(loginRefresh.HasAuthenticationExtended);
        Assert.False(loginRefresh.HasAuthenticationErrorText);

        encodedElementList.Clear();
        loginRefresh.Clear();
        decRefreshMsg.Clear();
        encRefreshMsg.Clear();
    }

    [Fact]
    public void HeaderLoginRefresh_Test()
    {
        LoginRefresh loginRefresh = new LoginRefresh();

        loginRefresh.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE,
            "headerLoginRefreshTest");
        loginRefresh.Name("UserName");
        loginRefresh.NameType(EmaRdm.USER_NAME);
        loginRefresh.SeqNum(5);
        loginRefresh.Solicited(true);

        Assert.True(loginRefresh.HasState);

        Assert.Equal(OmmState.StreamStates.OPEN, loginRefresh.State().StreamState);
        Assert.Equal(OmmState.DataStates.OK, loginRefresh.State().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, loginRefresh.State().StatusCode);
        Assert.Equal("headerLoginRefreshTest", loginRefresh.State().StatusText);
        Assert.True(loginRefresh.HasName);
        Assert.Equal("UserName", loginRefresh.Name());
        Assert.True(loginRefresh.HasNameType);
        Assert.Equal(EmaRdm.USER_NAME, loginRefresh.NameType());
        Assert.True(loginRefresh.HasSeqNum);
        Assert.Equal(5, loginRefresh.SeqNum());
        Assert.True(loginRefresh.HasSolicited);
        Assert.True(loginRefresh.Solicited());

        loginRefresh.Clear();
    }

    [Fact]
    public void ClearLoginRefresh_Test()
    {
        LoginRefresh loginRefresh = new LoginRefresh();

        loginRefresh.AllowSuspectData(false);
        loginRefresh.ProvidePermissionProfile(false);
        loginRefresh.ProvidePermissionExpressions(false);
        loginRefresh.SingleOpen(false);
        loginRefresh.SupportProviderDictionaryDownload(true);
        loginRefresh.SupportBatchRequests(EmaRdm.SUPPORT_BATCH_REQUEST);
        loginRefresh.SupportOptimizedPauseResume(true);
        loginRefresh.SupportOMMPost(true);
        loginRefresh.SupportViewRequests(true);
        loginRefresh.SupportStandby(true);
        loginRefresh.SupportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS);
        loginRefresh.AuthenticationTTReissue(10);
        loginRefresh.AuthenticationErrorCode(10);
        loginRefresh.AuthenticationErrorText("ErrorText");
        loginRefresh.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE,
            "StatusText");
        loginRefresh.SeqNum(10);
        loginRefresh.ApplicationId("AppId");
        loginRefresh.ApplicationName("AppName");
        loginRefresh.Position("Position");
        loginRefresh.AuthenticationExtendedResp(new EmaBuffer(Encoding.ASCII.GetBytes("AuthExtended")));
        loginRefresh.Name("Name");
        loginRefresh.NameType(EmaRdm.USER_EMAIL_ADDRESS);
        loginRefresh.Solicited(false);

        Assert.True(loginRefresh.HasAllowSuspectData);
        Assert.False(loginRefresh.AllowSuspectData());

        Assert.True(loginRefresh.HasProvidePermissionProfile);
        Assert.False(loginRefresh.ProvidePermissionProfile());

        Assert.True(loginRefresh.HasProvidePermissionExpressions);
        Assert.False(loginRefresh.ProvidePermissionExpressions());

        Assert.True(loginRefresh.HasSingleOpen);
        Assert.False(loginRefresh.SingleOpen());

        Assert.True(loginRefresh.HasSupportProviderDictionaryDownload);
        Assert.True(loginRefresh.SupportProviderDictionaryDownload());

        Assert.True(loginRefresh.HasSupportBatchRequests);
        Assert.Equal(EmaRdm.SUPPORT_BATCH_REQUEST, loginRefresh.SupportBatchRequests());

        Assert.True(loginRefresh.HasSupportOptimizedPauseResume);
        Assert.True(loginRefresh.SupportOptimizedPauseResume());

        Assert.True(loginRefresh.HasSupportOMMPost);
        Assert.True(loginRefresh.SupportOMMPost());

        Assert.True(loginRefresh.HasSupportViewRequests);
        Assert.True(loginRefresh.SupportViewRequests());

        Assert.True(loginRefresh.HasSupportStandby);
        Assert.True(loginRefresh.SupportStandby());

        Assert.True(loginRefresh.HasSupportEnhancedSymbolList);
        Assert.Equal(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS, loginRefresh.SupportEnhancedSymbolList());

        Assert.True(loginRefresh.HasAuthenticationTTReissue);
        Assert.Equal(10u, loginRefresh.AuthenticationTTReissue());

        Assert.True(loginRefresh.HasAuthenticationErrorCode);
        Assert.Equal(10u, loginRefresh.AuthenticationErrorCode());

        Assert.True(loginRefresh.HasAuthenticationErrorText);
        Assert.Equal("ErrorText", loginRefresh.AuthenticationErrorText());

        Assert.True(loginRefresh.HasState);
        Assert.Equal(OmmState.StreamStates.OPEN, loginRefresh.State().StreamState);
        Assert.Equal(OmmState.DataStates.OK, loginRefresh.State().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, loginRefresh.State().StatusCode);
        Assert.Equal("StatusText", loginRefresh.State().StatusText);

        Assert.True(loginRefresh.HasSeqNum);
        Assert.Equal(10, loginRefresh.SeqNum());

        Assert.True(loginRefresh.HasApplicationId);
        Assert.Equal("AppId", loginRefresh.ApplicationId());

        Assert.True(loginRefresh.HasApplicationName);
        Assert.Equal("AppName", loginRefresh.ApplicationName());

        Assert.True(loginRefresh.HasPosition);
        Assert.Equal("Position", loginRefresh.Position());

        Assert.True(loginRefresh.HasAuthenticationExtended);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes("AuthExtended")), loginRefresh.AuthenticationExtended());

        Assert.True(loginRefresh.HasName);
        Assert.Equal("Name", loginRefresh.Name());

        Assert.True(loginRefresh.HasNameType);
        Assert.Equal(EmaRdm.USER_EMAIL_ADDRESS, loginRefresh.NameType());

        Assert.True(loginRefresh.HasSolicited);
        Assert.False(loginRefresh.Solicited());

        loginRefresh.Clear();

        Assert.True(loginRefresh.HasAllowSuspectData);
        Assert.True(loginRefresh.AllowSuspectData());
        Assert.True(loginRefresh.HasProvidePermissionProfile);
        Assert.True(loginRefresh.ProvidePermissionProfile());
        Assert.True(loginRefresh.HasProvidePermissionExpressions);
        Assert.True(loginRefresh.ProvidePermissionExpressions());
        Assert.False(loginRefresh.HasSupportBatchRequests);
        Assert.False(loginRefresh.HasSupportOptimizedPauseResume);
        Assert.False(loginRefresh.HasSupportOMMPost);
        Assert.False(loginRefresh.HasSupportViewRequests);
        Assert.False(loginRefresh.HasSupportStandby);
        Assert.False(loginRefresh.HasSupportEnhancedSymbolList);
        Assert.False(loginRefresh.HasAuthenticationTTReissue);
        Assert.False(loginRefresh.HasAuthenticationErrorCode);
        Assert.False(loginRefresh.HasAuthenticationErrorText);
        Assert.False(loginRefresh.HasState);
        Assert.False(loginRefresh.HasSeqNum);
        Assert.True(loginRefresh.HasSingleOpen);
        Assert.True(loginRefresh.SingleOpen());
        Assert.False(loginRefresh.HasSupportProviderDictionaryDownload);
        Assert.False(loginRefresh.SupportProviderDictionaryDownload());
        Assert.False(loginRefresh.HasApplicationId);
        Assert.False(loginRefresh.HasApplicationName);
        Assert.False(loginRefresh.HasPosition);
        Assert.False(loginRefresh.HasAuthenticationExtended);
        Assert.False(loginRefresh.HasName);
        Assert.True(loginRefresh.HasNameType);
        Assert.Equal(EmaRdm.USER_NAME, loginRefresh.NameType());
        Assert.True(loginRefresh.HasSolicited);
        Assert.True(loginRefresh.Solicited());

        loginRefresh.Clear();
    }

    [Fact]
    public void ErrorHandlingLoginRefresh_Test()
    {
        LoginRefresh loginRefresh = new LoginRefresh();

        try
        {
            loginRefresh.ApplicationId();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("ApplicationId element is not set", ex.Message);
        }

        try
        {
            loginRefresh.ApplicationName();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("ApplicationName element is not set", ex.Message);
        }

        try
        {
            loginRefresh.Position();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("Position element is not set", ex.Message);
        }

        try
        {
            loginRefresh.AuthenticationExtended();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("AuthenticationExtended element is not set", ex.Message);
        }

        loginRefresh.Clear();
    }

    [Fact]
    public void DecodeLoginRefreshInvalidType_Test()
    {
        ElementList encodedElementList = new ElementList();

        encodedElementList.AddAscii(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, "1");
        encodedElementList.Complete();

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        RefreshMsg encRefreshMsg = new RefreshMsg();
        // NOTE departure from EMAJ TODO: without State information ETA RefreshMsg Encode returns INVALID_DATA
        encRefreshMsg.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE,
            "StatusText");
        encRefreshMsg.DomainType(EmaRdm.MMT_LOGIN);
        encRefreshMsg.Attrib(encodedElementList);
        encRefreshMsg.EncodeComplete();

        RefreshMsg decRefreshMsg = new RefreshMsg();

        decRefreshMsg.Decode(encRefreshMsg.m_refreshMsgEncoder.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginRefresh loginRefresh = new LoginRefresh();

        try
        {
            loginRefresh.Message(decRefreshMsg);
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("Decoding error for AllowSuspectData element. Attempt to uintValue() while actual entry data type is Ascii", ex.Message);
        }

        encodedElementList.Clear();
        loginRefresh.Clear();
        decRefreshMsg.Clear();
        encRefreshMsg.Clear();
        encRefreshMsg.Clear();
    }

    [Fact]
    public void EncodeLoginStatus_Test()
    {
        LoginStatus loginStatus = new LoginStatus();

        loginStatus.AuthenticationErrorCode(3);
        Assert.True(loginStatus.HasAuthenticationErrorCode);
        Assert.Equal(3u, loginStatus.AuthenticationErrorCode());

        loginStatus.AuthenticationErrorText("authenticationErrorText");
        Assert.True(loginStatus.HasAuthenticationErrorText);
        Assert.Equal("authenticationErrorText", loginStatus.AuthenticationErrorText());

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        StatusMsg decStatusMsg = new StatusMsg();

        var msg = loginStatus.Message();
        decStatusMsg.Decode(msg.m_rsslMsg, Codec.MajorVersion(), Codec.MinorVersion(), dictionary);

        Ema.Access.ElementList decodedEl = new ElementList();

        decodedEl.Decode(Codec.MajorVersion(), Codec.MinorVersion(),
            decStatusMsg.Attrib().ElementList().m_bodyBuffer!,
            dictionary, null);

        string? elementName;

        foreach (ElementEntry elementEntry in decodedEl)
        {
            elementName = elementEntry.Name;

            switch (elementName)
            {
                case EmaRdm.ENAME_AUTHN_ERRORCODE:
                    {
                        Assert.Equal(3u, elementEntry.UIntValue());
                    }
                    break;
                case EmaRdm.ENAME_AUTHN_ERRORTEXT:
                    {
                        Assert.Equal("authenticationErrorText", elementEntry.OmmAsciiValue().Value);
                    }
                    break;
                default:
                    break;
            }
        }

        loginStatus.Clear();
        decStatusMsg.Clear();
        decodedEl.Clear();
        msg.Clear();
    }

    [Fact]
    public void DecodeLoginStatus_Test()
    {
        ElementList encodedElementList = new ElementList();
        encodedElementList.AddUInt(EmaRdm.ENAME_AUTHN_ERRORCODE, 3);
        encodedElementList.AddAscii(EmaRdm.ENAME_AUTHN_ERRORTEXT, "authenticationErrorText");
        encodedElementList.Complete();

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        StatusMsg encStatusMsg = new StatusMsg();
        encStatusMsg.DomainType(EmaRdm.MMT_LOGIN);
        encStatusMsg.Attrib(encodedElementList);
        encStatusMsg.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "decodeLoginStatusTest");
        encStatusMsg.EncodeComplete();

        StatusMsg decStatusMsg = new StatusMsg();

        decStatusMsg.Decode(encStatusMsg.m_statusMsgEncoder.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginStatus loginStatus = new LoginStatus();

        loginStatus.Message(decStatusMsg);

        Assert.True(loginStatus.HasState);
        Assert.Equal(OmmState.StreamStates.OPEN, loginStatus.State().StreamState);
        Assert.Equal(OmmState.DataStates.OK, loginStatus.State().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, loginStatus.State().StatusCode);
        Assert.Equal("decodeLoginStatusTest", loginStatus.State().StatusText);

        // values are encoded in the encodedElementList
        Assert.True(loginStatus.HasAuthenticationErrorCode);
        Assert.Equal(3u, loginStatus.AuthenticationErrorCode());

        Assert.True(loginStatus.HasAuthenticationErrorText);
        Assert.Equal("authenticationErrorText", loginStatus.AuthenticationErrorText());

        encodedElementList.Clear();
        encStatusMsg.Clear();
        decStatusMsg.Clear();
        loginStatus.Clear();
    }

    [Fact]
    public void BlankLoginStatus_Test()
    {
        ElementList encodedElementList = new ElementList();

        encodedElementList.AddAscii(EmaRdm.ENAME_AUTHN_ERRORTEXT, "");

        Eta.Codec.DataDictionary dictionary = new Eta.Codec.DataDictionary();
        TestUtilities.eta_EncodeDictionaryMsg(dictionary);

        StatusMsg encStatusMsg = new StatusMsg();

        encStatusMsg.DomainType(EmaRdm.MMT_LOGIN);
        encStatusMsg.Attrib(encodedElementList);
        encStatusMsg.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "decodeLoginStatusTest");
        encStatusMsg.EncodeComplete();

        StatusMsg decStatusMsg = new StatusMsg();

        decStatusMsg.Decode(encStatusMsg.m_statusMsgEncoder.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion(), dictionary, null);

        LoginStatus loginStatus = new LoginStatus();
        loginStatus.AuthenticationErrorText("test");

        loginStatus.Message(decStatusMsg);

        Assert.True(loginStatus.HasState);
        Assert.Equal(OmmState.StreamStates.OPEN, loginStatus.State().StreamState);
        Assert.Equal(OmmState.DataStates.OK, loginStatus.State().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, loginStatus.State().StatusCode);
        Assert.Equal("decodeLoginStatusTest", loginStatus.State().StatusText);

        Assert.False(loginStatus.HasAuthenticationErrorText);

        encodedElementList.Clear();
        encStatusMsg.Clear();
        decStatusMsg.Clear();
        loginStatus.Clear();
    }


    [Fact]
    public void HeaderLoginStatus_Test()
    {
        LoginStatus loginStatus = new LoginStatus();

        loginStatus.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "headerLoginStatusTest");
        loginStatus.Name("UserName");
        loginStatus.NameType(EmaRdm.USER_NAME);

        Assert.True(loginStatus.HasState);

        Assert.Equal(OmmState.StreamStates.OPEN, loginStatus.State().StreamState);
        Assert.Equal(OmmState.DataStates.OK, loginStatus.State().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, loginStatus.State().StatusCode);
        Assert.Equal("headerLoginStatusTest", loginStatus.State().StatusText);

        Assert.True(loginStatus.HasName);
        Assert.Equal("UserName", loginStatus.Name());

        Assert.True(loginStatus.HasNameType);
        Assert.Equal(EmaRdm.USER_NAME, loginStatus.NameType());

        loginStatus.Clear();
    }

    [Fact]
    public void ClearLoginStatus_Test()
    {
        LoginStatus loginStatus = new LoginStatus();

        loginStatus.AuthenticationErrorCode(10);
        loginStatus.AuthenticationErrorText("ErrorText");
        loginStatus.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "StatusText");
        loginStatus.Name("Name");
        loginStatus.NameType(EmaRdm.USER_EMAIL_ADDRESS);

        Assert.True(loginStatus.HasAuthenticationErrorCode);
        Assert.Equal(10u, loginStatus.AuthenticationErrorCode());

        Assert.True(loginStatus.HasAuthenticationErrorText);
        Assert.Equal("ErrorText", loginStatus.AuthenticationErrorText());

        Assert.True(loginStatus.HasState);
        Assert.Equal(OmmState.StreamStates.OPEN, loginStatus.State().StreamState);
        Assert.Equal(OmmState.DataStates.OK, loginStatus.State().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, loginStatus.State().StatusCode);
        Assert.Equal("StatusText", loginStatus.State().StatusText);

        Assert.True(loginStatus.HasName);
        Assert.Equal("Name", loginStatus.Name());

        Assert.True(loginStatus.HasNameType);
        Assert.Equal(EmaRdm.USER_EMAIL_ADDRESS, loginStatus.NameType());

        loginStatus.Clear();

        Assert.False(loginStatus.HasAuthenticationErrorCode);
        Assert.False(loginStatus.HasAuthenticationErrorText);
        Assert.False(loginStatus.HasState);
        Assert.False(loginStatus.HasName);
        Assert.True(loginStatus.HasNameType);
        Assert.Equal(EmaRdm.USER_NAME, loginStatus.NameType());

        loginStatus.Clear();
    }

    [Fact]
    public void ErrorHandlingLoginStatus_Test()
    {
        LoginStatus loginStatus = new LoginStatus();

        try
        {
            loginStatus.AuthenticationErrorCode();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("AuthenticationErrorCode element is not set", ex.Message);
        }

        try
        {
            loginStatus.AuthenticationErrorText();
            Assert.Fail("Exception is expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.Contains("AuthenticationErrorText element is not set", ex.Message);
        }
        loginStatus.Clear();
    }
}
