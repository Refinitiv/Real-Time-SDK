/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;
using System.Text.Json;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorTokenSession : VaNode, IRestResponseCallback
    {
        private Reactor m_Reactor;
        private ReactorRestClient m_RestClient;

        private long m_AuthTokenExpireTime;
        private long m_AuthTokenExpiresIn;
        private bool m_SetProxyInfo = false;

        private ReaderWriterLockSlim SessionLock { get; set; } =
            new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

        public int OriginalExpiresIn { get; set; } // Keeps the original expires in seconds.

        public ReactorOAuthCredential ReactorOAuthCredential { get; private set; }

        public ReactorOAuthCredentialRenewal ReactorOAuthCredentialRenewal { get; private set; }

        public ReactorAuthTokenInfo ReactorAuthTokenInfo { get; private set; }

        public ReactorRestConnectOptions ReactorRestConnectOptions { get; private set; }

        public SessionState SessionMgntState { get; set; }

        public ReactorChannel? ReactorChannel { get; set; }

        public bool InTokenSessionQueue { get; set; } = false;

        /* This is used for unit testing to override the expires in seconds */
        internal static int ExpiresInTest { get; set; } = -1;

        public enum SessionState
        {
            UNKNOWN = 0,
            REQ_INIT_AUTH_TOKEN = 1,
            REQ_AUTH_TOKEN_USING_CLIENT_CRED = 2,
            RECEIVED_AUTH_TOKEN = 3,
            AUTHENTICATE_USING_CLIENT_CRED = 4,
            QUERYING_SERVICE_DISCOVERY = 5,
            RECEIVED_ENDPOINT_INFO = 6,
            REQUEST_TOKEN_FAILURE = 7,
            REQUEST_SERVICE_DISCOVERY_FAILURE = 8,
            STOP_TOKEN_REQUEST = 9,
            STOP_QUERYING_SERVICE_DISCOVERY = 10
        }

        public ReactorTokenSession(Reactor reactor, ReactorOAuthCredential reactorOAuthCredential)
        {
            m_Reactor = reactor;
            m_RestClient = reactor.m_ReactorRestClient!;
            ReactorOAuthCredential = reactorOAuthCredential;
            ReactorRestConnectOptions = new ReactorRestConnectOptions(m_Reactor.m_ReactorOptions);
            m_AuthTokenExpireTime = 0;
            OriginalExpiresIn = 0;
            ReactorOAuthCredentialRenewal = new ReactorOAuthCredentialRenewal();
            ReactorAuthTokenInfo = new ReactorAuthTokenInfo();
            SessionMgntState = SessionState.AUTHENTICATE_USING_CLIENT_CRED;
        }

        public void SetProxyInfo(ReactorConnectInfo connectInfo)
        {
            if(!m_SetProxyInfo)
            {
                m_SetProxyInfo=true;
                ReactorRestConnectOptions.ApplyProxyInfo(connectInfo.ConnectOptions);
            }
        }

        public void CalculateNextAuthTokenExpireTime(int expiresIn)
        {
            if(expiresIn > 0)
            {
                m_AuthTokenExpiresIn = (long)((expiresIn * m_Reactor.m_ReactorOptions.GetTokenExpireRatio()) * 1000);
            }

            m_AuthTokenExpireTime = ReactorUtil.GetCurrentTimeMilliSecond() + m_AuthTokenExpiresIn;
        }

        public long GetNextAuthTokenExpireTime()
        {
            return m_AuthTokenExpireTime;
        }

        public void SendAuthRequestWithSensitiveInfo(ReactorOAuthCredentialRenewal reactorOAuthCredRenewal,
            ReactorOAuthCredentialRenewalModes renewalModes)
        {
            SessionMgntState = SessionState.REQ_AUTH_TOKEN_USING_CLIENT_CRED;

            if (renewalModes == ReactorOAuthCredentialRenewalModes.CLIENT_SECRET)
            {
                ReactorOAuthCredential.ClientSecret.Data(reactorOAuthCredRenewal.ClientSecret.ToString());
            }
            else if (renewalModes == ReactorOAuthCredentialRenewalModes.CLIENT_JWK)
            {
                ReactorOAuthCredential.ClientJwk.Data(reactorOAuthCredRenewal.ClientJwk.ToString());
                ReactorOAuthCredential.JsonWebKey = null;

                if ( ReactorRestClient.ValidateJWK(ReactorOAuthCredential, out ReactorErrorInfo? errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    RestEvent restEvent = new RestEvent();
                    restEvent.Type = RestEvent.EventType.FAILED;
                    RestErrorCallback(restEvent, errorInfo!.Error.Text);
                    return;
                }
            }

            m_RestClient.SendTokenRequestAsync(ReactorOAuthCredential, ReactorRestConnectOptions, ReactorAuthTokenInfo, this);

            ReactorOAuthCredential.ClientSecret.Clear();
            ReactorOAuthCredential.ClientJwk.Clear();
        }

        public void HandleTokenRequest()
        {
            // Don't send token request when the reactor is shutting down.
            if (m_Reactor.IsShutdown)
                return;

            if(SessionMgntState == SessionState.REQUEST_TOKEN_FAILURE || SessionMgntState == SessionState.AUTHENTICATE_USING_CLIENT_CRED)
            {
                if(ReactorOAuthCredential.ReactorOAuthCredentialEventCallback is null)
                {
                    SessionMgntState = SessionState.REQ_AUTH_TOKEN_USING_CLIENT_CRED;
                    m_RestClient.SendTokenRequestAsync(ReactorOAuthCredential, ReactorRestConnectOptions, ReactorAuthTokenInfo, this);
                }
                else
                {
                    ReactorOAuthCredentialRenewal.Clear();
                    ReactorOAuthCredentialRenewal.ClientId.Data(ReactorOAuthCredential.ClientId.ToString());
                    ReactorOAuthCredentialRenewal.TokenScope.Data(ReactorOAuthCredential.TokenScope.ToString());
                    ReactorOAuthCredentialRenewal.Audience.Data(ReactorOAuthCredential.Audience.ToString());

                    /* Creates the token renewal event to the Reactor's queue for the application to submit sensitive information */
                    m_Reactor.SendCredentialRenewalEvent(ReactorChannel!, this, null);
                }
            }
        }

        public bool HasAccessToken()
        {
            Lock();

            try
            {
                if(!string.IsNullOrEmpty(ReactorAuthTokenInfo.AccessToken))
                {
                    return true;
                }
            }
            finally
            {
                Unlock();
            }

            return false;
        }

        public void Lock()
        {
            SessionLock.EnterWriteLock();
        }

        public void Unlock()
        {
            SessionLock?.ExitWriteLock();
        }

        public void RestResponseCallback(RestResponse response, RestEvent restEvent)
        {
            /* Do nothing as the Reactor is shutting down. */
            if(m_Reactor.IsShutdown)
            {
                return;
            }

            if (restEvent.RespType == RestEvent.ResponseType.TOKEN_SERVICE_RESP)
            {
                try
                {
                    Lock();

                    switch (restEvent.Type)
                    {
                        case RestEvent.EventType.COMPLETED:
                            {
                                SessionMgntState = SessionState.RECEIVED_AUTH_TOKEN;

                                AccessTokenInformation? tokenInfo = JsonSerializer.Deserialize<AccessTokenInformation>(response.Content!);
                                if (tokenInfo != null)
                                {
                                    if (!string.IsNullOrEmpty(tokenInfo.access_token) && tokenInfo.expires_in != -1)
                                    {
                                        ReactorAuthTokenInfo.SetAccessTokenInfo(tokenInfo);

                                        if (OriginalExpiresIn != 0)
                                        {
                                            OriginalExpiresIn = ReactorAuthTokenInfo.ExpiresIn;
                                        }

                                        if (ReactorChannel is not null)
                                        {
                                            m_Reactor.SendAuthTokenEvent(ReactorChannel, this, null);

                                            /* Send to worker thread to calculate next expire time */
                                            m_Reactor.SendReactorImplEvent(ReactorEventImpl.ImplType.TOKEN_MGNT, ReactorChannel);

                                            if (ReactorChannel.State == ReactorChannelState.RDP_RT)
                                            {
                                                ReactorConnectInfo reactorConnectInfo = ReactorChannel.GetReactorConnectInfo();

                                                /* Checks whether to get a host and port from RDP service discovery */
                                                if (Reactor.RequestServiceDiscovery(reactorConnectInfo))
                                                {
                                                    SessionMgntState = SessionState.QUERYING_SERVICE_DISCOVERY;
                                                    ReactorChannel.ServiceEndpointInfoList.Clear();
                                                    if (m_RestClient.SendServiceDirectoryRequest(ReactorRestConnectOptions, ReactorAuthTokenInfo,
                                                        ReactorChannel.ServiceEndpointInfoList, out ReactorErrorInfo? errorInfo) != ReactorReturnCode.SUCCESS)
                                                    {
                                                        ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                                                        Reactor.PopulateErrorInfo(ReactorChannel.ReactorErrorInfo, ReactorReturnCode.FAILURE,
                                                            "ReactorTokenSession.RestResponseCallback", $"Failed to get endpoint information from the service discovery. Text = {errorInfo?.Error.Text}");
                                                    }
                                                    else
                                                    {
                                                        if (ReactorChannel.ApplyServiceDiscoveryEndpoint(out errorInfo) != ReactorReturnCode.SUCCESS)
                                                        {
                                                            ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                                                            Reactor.PopulateErrorInfo(ReactorChannel.ReactorErrorInfo, ReactorReturnCode.FAILURE,
                                                                "ReactorTokenSession.RestResponseCallback", $"Failed to apply endpoint information from the service discovery. Text = {errorInfo?.Error.Text}");
                                                        }
                                                        else
                                                        {
                                                            ReactorChannel.State = ReactorChannelState.RDP_RT_DONE;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    ReactorChannel.State = ReactorChannelState.RDP_RT_DONE;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (ReactorChannel is not null)
                                        {
                                            ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                                            Reactor.PopulateErrorInfo(ReactorChannel.ReactorErrorInfo, ReactorReturnCode.FAILURE,
                                                "ReactorTokenSession.RestResponseCallback", "Failed to get access token information from the token service");
                                        }
                                    }
                                }

                                break;
                            }
                        case RestEvent.EventType.FAILED:
                        case RestEvent.EventType.STOPPED:
                            {
                                string errorText = "unavaliable";

                                if (response.Content != null)
                                {
                                    errorText = response.Content;
                                }

                                if (restEvent.Type != RestEvent.EventType.STOPPED)
                                {
                                    SessionMgntState = SessionState.REQUEST_TOKEN_FAILURE;
                                }
                                else
                                {
                                    SessionMgntState = SessionState.STOP_TOKEN_REQUEST;
                                }

                                Reactor.PopulateErrorInfo(ReactorChannel!.ReactorErrorInfo, ReactorReturnCode.FAILURE,
                                    "ReactorTokenSession.RestResponseCallback",
                                    $"Failed REST request from the token service with HTTP status code {response.StatusCode} " +
                                    $"for client ID {ReactorOAuthCredential.ClientId}. Text: {errorText}");

                                /* Send the warning only when the ReactorChannel is active */
                                if (ReactorChannel!.State == ReactorChannelState.READY || ReactorChannel.State == ReactorChannelState.UP)
                                {
                                    if (ReactorChannel.EnableSessionManagement())
                                    {
                                        m_Reactor!.SendChannelWarningEvent(ReactorChannel, ReactorChannel.ReactorErrorInfo);
                                    }
                                }

                                m_Reactor!.SendAuthTokenEvent(ReactorChannel, this, ReactorChannel.ReactorErrorInfo);

                                /* This is used to indicate that the is no longer valid */
                                ReactorChannel!.RDMLoginRequestRDP!.UserName.Data("");

                                if (ReactorChannel.State == ReactorChannelState.RDP_RT)
                                {
                                    ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                                }

                                break;
                            }
                    }
                }
                finally
                {
                    Unlock();
                }
            }
            else if (restEvent.RespType == RestEvent.ResponseType.SERVICE_DISCOVERY_RESP)
            {
                switch (restEvent.Type)
                {
                    case RestEvent.EventType.COMPLETED:
                        {
                            if (ReactorChannel is not null)
                            {
                                if (m_RestClient.ParserServiceDiscoveryEndpoint(response, ReactorChannel.ServiceEndpointInfoList, out ReactorErrorInfo? errorInfo) 
                                    != ReactorReturnCode.SUCCESS)
                                {
                                    string errorText = "unavaliable";

                                    if (response.Content != null)
                                    {
                                        errorText = response.Content;
                                    }

                                    Reactor.PopulateErrorInfo(ReactorChannel.ReactorErrorInfo, ReactorReturnCode.FAILURE,
                                        "ReactorTokenSession.RestResponseCallback",
                                        $"Failed REST request from the service discovery with HTTP status code {response.StatusCode} " +
                                        $"Text: {errorText}");

                                    m_Reactor.SendChannelWarningEvent(ReactorChannel, ReactorChannel.ReactorErrorInfo);

                                    if (ReactorChannel.State == ReactorChannelState.RDP_RT)
                                    {
                                        ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                                    }
                                }
                                else
                                {
                                    SessionMgntState = SessionState.RECEIVED_ENDPOINT_INFO;

                                    if(ReactorChannel.State == ReactorChannelState.RDP_RT)
                                    {
                                        ReactorChannel.State = ReactorChannelState.RDP_RT_DONE;
                                    }
                                }
                            }

                            break;
                        }
                    case RestEvent.EventType.FAILED:
                    case RestEvent.EventType.STOPPED:
                        {
                            if(ReactorChannel is not null)
                            {
                                string errorText = "unavaliable";

                                if (response.Content != null)
                                {
                                    errorText = response.Content;
                                }

                                Reactor.PopulateErrorInfo(ReactorChannel.ReactorErrorInfo, ReactorReturnCode.FAILURE,
                                    "ReactorTokenSession.RestResponseCallback",
                                    $"Failed REST request from the service discovery with HTTP status code {response.StatusCode} " +
                                    $"Text: {errorText}");

                                m_Reactor.SendChannelWarningEvent(ReactorChannel, ReactorChannel.ReactorErrorInfo);

                                if(ReactorChannel.State == ReactorChannelState.RDP_RT)
                                {
                                    if(restEvent.Type == RestEvent.EventType.STOPPED)
                                    {
                                        SessionMgntState = SessionState.STOP_QUERYING_SERVICE_DISCOVERY;
                                    }
                                    else
                                    {
                                        SessionMgntState = SessionState.REQUEST_SERVICE_DISCOVERY_FAILURE;
                                    }

                                    ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                                }
                            }
                            break;
                        }
                }
            }
        }

        public void RestErrorCallback(RestEvent restEvent, string? errorText)
        {
            /* Do nothing as the Reactor is shutting down. */
            if (m_Reactor.IsShutdown)
            {
                return;
            }

            if (restEvent.RespType == RestEvent.ResponseType.TOKEN_SERVICE_RESP)
            {
                if (ReactorChannel is not null)
                {
                    Reactor.PopulateErrorInfo(ReactorChannel!.ReactorErrorInfo, ReactorReturnCode.FAILURE,
                                    "ReactorTokenSession.RestErrorCallback",
                                    $"Failed REST request from the token service" +
                                    $"for client ID {ReactorOAuthCredential.ClientId}. Text: {errorText}");

                    /* Send the warning only when the ReactorChannel is active */
                    if (ReactorChannel.State == ReactorChannelState.UP || ReactorChannel.State == ReactorChannelState.READY)
                    {
                        if (ReactorChannel.EnableSessionManagement())
                        {
                            m_Reactor.SendChannelWarningEvent(ReactorChannel, ReactorChannel.ReactorErrorInfo);
                        }
                    }

                    /* This is used to indicate that the token is no longer valid */
                    ReactorChannel.RDMLoginRequestRDP!.UserName.Data("");

                    if (ReactorChannel.State == ReactorChannelState.RDP_RT)
                    {
                        ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                    }

                    SessionMgntState = SessionState.REQUEST_TOKEN_FAILURE;

                    m_Reactor.SendAuthTokenEvent(ReactorChannel, this, ReactorChannel.ReactorErrorInfo);
                }
            }
            else if (restEvent.RespType == RestEvent.ResponseType.SERVICE_DISCOVERY_RESP)
            {
                if (ReactorChannel is not null)
                {
                    Reactor.PopulateErrorInfo(ReactorChannel.ReactorErrorInfo,
                    ReactorReturnCode.FAILURE,
                    "ReactorTokenSession.RestErrorCallback", "Failed REST request for the service discovery. Text: " + errorText);

                    SessionMgntState = SessionState.REQUEST_SERVICE_DISCOVERY_FAILURE;

                    m_Reactor.SendChannelWarningEvent(ReactorChannel, ReactorChannel.ReactorErrorInfo);
                    if (ReactorChannel.State == ReactorChannelState.RDP_RT)
                    {
                        ReactorChannel.State = ReactorChannelState.RDP_RT_FAILED;
                    }
                }
            }
        }
    }
}
