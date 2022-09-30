/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports;
using System.Net;
using System.Net.Http.Headers;
using System.Text;
using System.Text.Json;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    internal class ReactorRestClient : IDisposable
    {
        Reactor m_Reactor;

        static readonly string GRANT_TYPE = "grant_type";
        static readonly string GRANT_TYPE_VALUE = "client_credentials";
        static readonly string TOKEN_SCOPE = "scope";
        static readonly string TOKEN_SCOPE_VALUE = "trapi.streaming.pricing.read";
        static readonly string CLIENT_ID = "client_id";
        static readonly string CLIENT_SECRET = "client_secret";
        static readonly string MEDIA_TYPE = "application/x-www-form-urlencoded";
        static readonly MediaTypeWithQualityHeaderValue ACCEPT_MEDIA_TYPE = new MediaTypeWithQualityHeaderValue("application/json");
        static readonly StringWithQualityHeaderValue ACCEPT_GZIP = new StringWithQualityHeaderValue("gzip");
        static readonly StringWithQualityHeaderValue ACCEPT_DEFLATE = new StringWithQualityHeaderValue("deflate");
        static readonly string TRANSPORT_TCP_QUERY = "transport=tcp";
        static readonly string TRANSPORT_WEBSOCKET_QUERY = "transport=websocket";
        static readonly string DATAFORMAT_RWF_QUERY = "dataformat=rwf";
        static readonly string DATAFORMAT_JSON2_QUERY = "dataformat=tr_json2";
        public static readonly string RDP_RT_TRANSPORT_PROTOCOL_WEBSOCKET = "websocket";
	    public static readonly string RDP_RT_TRANSPORT_PROTOCOL_TCP = "tcp";

        Dictionary<string, string> encodedContentDict = new Dictionary<string, string>(5);
        Dictionary<ProxyOptions, HttpClient> httpClientDict = new Dictionary<ProxyOptions, HttpClient>(5);
        private bool disposedValue;

        public ReactorRestClient(Reactor reactor)
        {
            m_Reactor = reactor;
            /* Set encoded content dictionary */
            encodedContentDict.Add(GRANT_TYPE, GRANT_TYPE_VALUE);
            encodedContentDict.Add(TOKEN_SCOPE, TOKEN_SCOPE_VALUE);
            encodedContentDict.Add(CLIENT_ID, "");
            encodedContentDict.Add(CLIENT_SECRET, "");
        }

        public HttpClient GetHttpClient(ReactorRestConnectOptions restConnetOptions)
        {
            if(!httpClientDict.TryGetValue(restConnetOptions.ProxyOptions, out HttpClient? httpClient))
            {
                if(!string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyHostName) && !string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyPort))
                {
                    NetworkCredential? networkCredential = null;

                    if(!string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyUserName) && !string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyPassword))
                    {
                        networkCredential = new NetworkCredential(restConnetOptions.ProxyOptions.ProxyUserName, restConnetOptions.ProxyOptions.ProxyPassword); // proxy authentication 
                    }

                    var proxy = new WebProxy
                    {
                        Address = new Uri($"http://{restConnetOptions.ProxyOptions.ProxyHostName}:{restConnetOptions.ProxyOptions.ProxyPort}"),
                        Credentials = networkCredential
                    };

                    var m_ClientHandler = new HttpClientHandler();
                    m_ClientHandler.AutomaticDecompression = System.Net.DecompressionMethods.All;
                    m_ClientHandler.AllowAutoRedirect = false;
                    m_ClientHandler.Proxy = proxy;
                    httpClient = new HttpClient(m_ClientHandler);

                    httpClient.Timeout = TimeSpan.FromMilliseconds(m_Reactor.m_ReactorOptions.GetRestRequestTimeout());

                    httpClientDict.Add(restConnetOptions.ProxyOptions, httpClient);
                }
                else
                {
                    /* No proxy */
                    var m_ClientHandler = new HttpClientHandler();
                    m_ClientHandler.AutomaticDecompression = System.Net.DecompressionMethods.All;
                    m_ClientHandler.AllowAutoRedirect = false;
                    httpClient = new HttpClient(m_ClientHandler);

                    httpClient.Timeout = TimeSpan.FromMilliseconds(m_Reactor.m_ReactorOptions.GetRestRequestTimeout());

                    httpClientDict.Add(restConnetOptions.ProxyOptions, httpClient);
                }
            }

            return httpClient;
        }

        public ReactorReturnCode SendTokenRequest(ReactorOAuthCredential oAuthCredential, ReactorRestConnectOptions restConnetOptions,
            ReactorAuthTokenInfo authTokenInfo, out ReactorErrorInfo? errorInfo)
        {
            string url;
            if(restConnetOptions.AuthRedirect && !string.IsNullOrEmpty(restConnetOptions.AuthRedirectLocation))
            {
                url = restConnetOptions.AuthRedirectLocation;
            }
            else
            {
                url = m_Reactor.m_ReactorOptions.GetTokenServiceURL();
            }

            var request = new HttpRequestMessage(HttpMethod.Post, url);
            request.Headers.Accept.Add(ACCEPT_MEDIA_TYPE);
            request.Headers.AcceptEncoding.Add(ACCEPT_GZIP);
            request.Headers.AcceptEncoding.Add(ACCEPT_DEFLATE);

            /* Specify client ID and client secret. */
            encodedContentDict[CLIENT_ID] = oAuthCredential.ClientId.ToString();
            encodedContentDict[CLIENT_SECRET] = oAuthCredential.ClientSecret.ToString();

            var content = new FormUrlEncodedContent(encodedContentDict);
            content.Headers.ContentType = new MediaTypeHeaderValue(MEDIA_TYPE);

            request.Content = content;

            try
            {
                var response = GetHttpClient(restConnetOptions).Send(request);

                var restResponse = HandleTokenServiceHttpResponse(response, oAuthCredential, restConnetOptions, authTokenInfo, null,
                    false, out errorInfo);
                
                AccessTokenInformation? tokenInfo = JsonSerializer.Deserialize<AccessTokenInformation>(restResponse.Content!);
                if(tokenInfo != null)
                {
                    if (!string.IsNullOrEmpty(tokenInfo.access_token) && tokenInfo.expires_in != -1)
                    {
                        authTokenInfo.SetAccessTokenInfo(tokenInfo);
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "ReactorRestClient.SendTokenRequest", $"Failed to get access token information from the token service. Text: {restResponse.Content}");
                    }
                }
            }
            catch(JsonException jsonEx)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "ReactorRestClient.SendTokenRequest", $"Failed to parse access token information from the response. Text: {jsonEx.Message}");
            }
            catch (Exception ex)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "ReactorRestClient.SendTokenRequest", $"Failed to perform a REST request to the token service. Text: {ex.Message}");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void SendTokenRequestAsync(ReactorOAuthCredential oAuthCredential, ReactorRestConnectOptions restConnetOptions,
            ReactorAuthTokenInfo authTokenInfo, ReactorTokenSession tokenSession)
        {
            string url;
            if (restConnetOptions.AuthRedirect && !string.IsNullOrEmpty(restConnetOptions.AuthRedirectLocation))
            {
                url = restConnetOptions.AuthRedirectLocation;
            }
            else
            {
                url = m_Reactor.m_ReactorOptions.GetTokenServiceURL();
            }

            var request = new HttpRequestMessage(HttpMethod.Post, url);
            request.Headers.Accept.Add(ACCEPT_MEDIA_TYPE);
            request.Headers.AcceptEncoding.Add(ACCEPT_GZIP);
            request.Headers.AcceptEncoding.Add(ACCEPT_DEFLATE);

            /* Specify client ID and client secret. */
            var localEncodedContentDict = new Dictionary<string, string>(5);
            localEncodedContentDict.Add(GRANT_TYPE, GRANT_TYPE_VALUE);
            localEncodedContentDict.Add(TOKEN_SCOPE, TOKEN_SCOPE_VALUE);
            localEncodedContentDict[CLIENT_ID] = oAuthCredential.ClientId.ToString();
            localEncodedContentDict[CLIENT_SECRET] = oAuthCredential.ClientSecret.ToString();

            var content = new FormUrlEncodedContent(localEncodedContentDict);
            content.Headers.ContentType = new MediaTypeHeaderValue(MEDIA_TYPE);

            request.Content = content;

            RestResponse restResponse = new RestResponse();

            try
            {
                var response = GetHttpClient(restConnetOptions).SendAsync(request);

                response.GetAwaiter().OnCompleted(() =>
                {
                    if(response.IsCompleted)
                    {
                        HandleTokenServiceHttpResponse(response.Result, oAuthCredential, restConnetOptions, authTokenInfo,
                            tokenSession, true, out _);
                    }
                    else if(response.IsFaulted)
                    {
                        RestEvent restEvent = new RestEvent();
                        restEvent.Type = RestEvent.EventType.FAILED;
                        tokenSession.RestErrorCallback(restEvent, response.Exception?.Message);
                    }
                    else if(response.IsCanceled)
                    {
                        RestEvent restEvent = new RestEvent();
                        restEvent.Type = RestEvent.EventType.CANCELLED;
                        tokenSession.RestErrorCallback(restEvent, "The request is cancelled.");
                    }

                });

            }
            catch (Exception ex)
            {
                RestEvent restEvent = new RestEvent();
                restEvent.Type = RestEvent.EventType.FAILED;
                tokenSession.RestErrorCallback(restEvent, $"Failed to perform a REST request to the token service. Text: {ex.Message}");
            }
        }

        public ReactorReturnCode SendServiceDirectoryRequest(ReactorRestConnectOptions restConnetOptions, ReactorAuthTokenInfo authTokenInfo,
            List<ReactorServiceEndpointInfo> endpointInfoList, out ReactorErrorInfo? errorInfo)
        {
            string url;
            if (restConnetOptions.DiscoveryRedirect && !string.IsNullOrEmpty(restConnetOptions.DiscoveryRedirectLocation))
            {
                url = restConnetOptions.DiscoveryRedirectLocation;
            }
            else
            {
                url = m_Reactor.m_ReactorOptions.GetServiceDiscoveryURL();
            }

            var request = new HttpRequestMessage(HttpMethod.Get, GenerateServiceDiscoveryUri(url, restConnetOptions));

            request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
            request.Headers.AcceptEncoding.Add(new StringWithQualityHeaderValue("gzip"));
            request.Headers.AcceptEncoding.Add(new StringWithQualityHeaderValue("deflate"));

            request.Headers.Authorization = new AuthenticationHeaderValue(authTokenInfo.TokenType!, authTokenInfo.AccessToken);

            try
            {
                var response = GetHttpClient(restConnetOptions).Send(request);

                var restResponse = HandleServiceDiscoveryHttpResponse(response, restConnetOptions, authTokenInfo, null, false, endpointInfoList,
                    out errorInfo);

                if (ParserServiceDiscoveryEndpoint(restResponse, endpointInfoList, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    return errorInfo!.Code;
                }
            }
            catch(Exception ex)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "ReactorRestClient.SendServiceDirectoryRequest", $"Failed to perform a REST request to the service discovery. Text: {ex.Message}");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void SendServiceDirectoryRequestAsync(ReactorRestConnectOptions restConnetOptions, ReactorAuthTokenInfo authTokenInfo,
            ReactorTokenSession tokenSession)
        {
            string url;
            if (restConnetOptions.DiscoveryRedirect && !string.IsNullOrEmpty(restConnetOptions.DiscoveryRedirectLocation))
            {
                url = restConnetOptions.DiscoveryRedirectLocation;
            }
            else
            {
                url = m_Reactor.m_ReactorOptions.GetServiceDiscoveryURL();
            }

            var request = new HttpRequestMessage(HttpMethod.Get, GenerateServiceDiscoveryUri(url, restConnetOptions));

            request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
            request.Headers.AcceptEncoding.Add(new StringWithQualityHeaderValue("gzip"));
            request.Headers.AcceptEncoding.Add(new StringWithQualityHeaderValue("deflate"));

            request.Headers.Authorization = new AuthenticationHeaderValue(authTokenInfo.TokenType!, authTokenInfo.AccessToken);

            RestResponse restResponse = new RestResponse();

            try
            {
                var response = GetHttpClient(restConnetOptions).SendAsync(request);

                response.GetAwaiter().OnCompleted(() =>
                {
                    if (response.IsCompleted)
                    {
                        HandleServiceDiscoveryHttpResponse(response.Result, restConnetOptions, authTokenInfo, tokenSession,
                            true, null, out _);
                    }
                    else if (response.IsFaulted)
                    {
                        RestEvent restEvent = new RestEvent();
                        restEvent.Type = RestEvent.EventType.FAILED;
                        tokenSession.RestErrorCallback(restEvent, response.Exception?.Message);
                    }
                    else if (response.IsCanceled)
                    {
                        RestEvent restEvent = new RestEvent();
                        restEvent.Type = RestEvent.EventType.CANCELLED;
                        tokenSession.RestErrorCallback(restEvent, "The request is cancelled.");
                    }
                });
            }
            catch (Exception ex)
            {
                RestEvent restEvent = new RestEvent();
                restEvent.Type = RestEvent.EventType.FAILED;
                tokenSession.RestErrorCallback(restEvent, $"Failed to perform a REST request to the service discovery. Text: {ex.Message}");
            }
        }

        private static string GenerateServiceDiscoveryUri(string serviceDiscoveryUrl, ReactorRestConnectOptions restConnetOptions)
        {
            if(restConnetOptions.Transport == ReactorDiscoveryTransportProtocol.RD_TP_INIT &&
                restConnetOptions.DataFormat == ReactorDiscoveryDataFormatProtocol.RD_DP_INIT)
            {
                return serviceDiscoveryUrl;
            }

            StringBuilder queryStringUrl = new StringBuilder(serviceDiscoveryUrl.Length + TRANSPORT_WEBSOCKET_QUERY.Length +
                                            DATAFORMAT_JSON2_QUERY.Length + 2);

            queryStringUrl.Append(serviceDiscoveryUrl);
            queryStringUrl.Append('?');
            bool hasTransport = false;

            if (restConnetOptions.Transport != ReactorDiscoveryTransportProtocol.RD_TP_INIT)
            {
                hasTransport = true;
                if (restConnetOptions.Transport == ReactorDiscoveryTransportProtocol.RD_TP_TCP)
                {
                    queryStringUrl.Append(TRANSPORT_TCP_QUERY);
                }
                else if (restConnetOptions.Transport == ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET)
                {
                    queryStringUrl.Append(TRANSPORT_WEBSOCKET_QUERY);
                }
            }

            if (restConnetOptions.DataFormat != ReactorDiscoveryDataFormatProtocol.RD_DP_INIT)
            {
                if(hasTransport)
                {
                    queryStringUrl.Append('&');
                }

                if (restConnetOptions.DataFormat == ReactorDiscoveryDataFormatProtocol.RD_DP_RWF)
                {
                    queryStringUrl.Append(DATAFORMAT_RWF_QUERY);
                }
                else if (restConnetOptions.DataFormat == ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2)
                {
                    queryStringUrl.Append(DATAFORMAT_JSON2_QUERY);
                }
            }

            return queryStringUrl.ToString();
        }

        private RestResponse HandleTokenServiceHttpResponse(HttpResponseMessage responseMessage, ReactorOAuthCredential oAuthCredential, 
            ReactorRestConnectOptions restConnetOptions, ReactorAuthTokenInfo authTokenInfo,
            ReactorTokenSession? tokenSession, bool isAsync, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            RestResponse restResponse = new RestResponse();

            restResponse.StatusCode = responseMessage.StatusCode;
            restResponse.StatusText = responseMessage.ReasonPhrase;
            restResponse.Version = responseMessage.Version;

            var readAsString = responseMessage.Content.ReadAsStringAsync();
            readAsString.Wait();

            restResponse.Content = readAsString.Result;

            switch (responseMessage.StatusCode)
            {
                case HttpStatusCode.OK:
                    {
                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.COMPLETED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }

                        break;
                    }
                case HttpStatusCode.MovedPermanently:
                case HttpStatusCode.Found:
                case HttpStatusCode.TemporaryRedirect:
                case HttpStatusCode.PermanentRedirect:
                    {
                        Uri? newUri = responseMessage.Headers.Location;
                        if (newUri != null && !string.IsNullOrEmpty(newUri.AbsoluteUri))
                        {
                            if (!restConnetOptions.AuthRedirect)
                            {
                                restConnetOptions.AuthRedirect = true;
                                restConnetOptions.AuthRedirectLocation = newUri.AbsoluteUri;

                                if (isAsync)
                                {
                                    SendTokenRequestAsync(oAuthCredential, restConnetOptions, authTokenInfo, tokenSession!);
                                }
                                else
                                {
                                    SendTokenRequest(oAuthCredential, restConnetOptions, authTokenInfo, out errorInfo);
                                }

                                if (responseMessage.StatusCode == HttpStatusCode.MovedPermanently ||
                                    responseMessage.StatusCode == HttpStatusCode.PermanentRedirect)
                                {
                                    restConnetOptions.ReactorOptions.SetTokenServiceURL(newUri.AbsoluteUri);
                                }

                                return restResponse;
                            }
                            else
                            {
                                restConnetOptions.ClearAuthRedirectParamerters();

                            }
                        }

                        if (restConnetOptions.AuthRedirect)
                        {
                            restConnetOptions.ClearAuthRedirectParamerters();
                        }

                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.FAILED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }

                        break;
                    }
                case HttpStatusCode.Forbidden:
                case HttpStatusCode.NotFound:
                case HttpStatusCode.Gone:
                case HttpStatusCode.UnavailableForLegalReasons:
                    {
                        if (restConnetOptions.AuthRedirect)
                        {
                            restConnetOptions.ClearAuthRedirectParamerters();
                        }

                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.STOPPED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }
                        break;
                    }
                default:
                    {
                        if (restConnetOptions.AuthRedirect)
                        {
                            restConnetOptions.ClearAuthRedirectParamerters();
                        }

                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.FAILED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }

                        break;
                    }
            }

            return restResponse;
        }

        private RestResponse HandleServiceDiscoveryHttpResponse(HttpResponseMessage responseMessage, ReactorRestConnectOptions restConnetOptions, 
            ReactorAuthTokenInfo authTokenInfo, ReactorTokenSession? tokenSession, bool isAsync,
            List<ReactorServiceEndpointInfo>? endpointInfoList, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            RestResponse restResponse = new RestResponse();

            restResponse.StatusCode = responseMessage.StatusCode;
            restResponse.StatusText = responseMessage.ReasonPhrase;
            restResponse.Version = responseMessage.Version;

            var readAsString = responseMessage.Content.ReadAsStringAsync();
            readAsString.Wait();

            restResponse.Content = readAsString.Result;

            switch (responseMessage.StatusCode)
            {
                case HttpStatusCode.OK:
                    {
                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.COMPLETED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }

                        break;
                    }
                case HttpStatusCode.MovedPermanently:
                case HttpStatusCode.Found:
                case HttpStatusCode.TemporaryRedirect:
                case HttpStatusCode.PermanentRedirect:
                    {
                        Uri? newUri = responseMessage.Headers.Location;
                        if (newUri != null && !string.IsNullOrEmpty(newUri.AbsoluteUri))
                        {
                            if (!restConnetOptions.DiscoveryRedirect)
                            {
                                restConnetOptions.DiscoveryRedirect = true;
                                restConnetOptions.DiscoveryRedirectLocation = newUri.AbsoluteUri;

                                if (isAsync)
                                {
                                    SendServiceDirectoryRequestAsync(restConnetOptions, authTokenInfo, tokenSession!);
                                }
                                else
                                {
                                    SendServiceDirectoryRequest(restConnetOptions, authTokenInfo, endpointInfoList!, out errorInfo);
                                }

                                if (responseMessage.StatusCode == HttpStatusCode.MovedPermanently ||
                                    responseMessage.StatusCode == HttpStatusCode.PermanentRedirect)
                                {
                                    restConnetOptions.ReactorOptions.SetServiceDiscoveryURL(newUri.AbsoluteUri);
                                }

                                return restResponse;
                            }
                            else
                            {
                                restConnetOptions.ClearDiscoveryRedirectParameters();

                            }
                        }

                        if (restConnetOptions.DiscoveryRedirect)
                        {
                            restConnetOptions.ClearDiscoveryRedirectParameters();
                        }

                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.FAILED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }

                        break;
                    }
                case HttpStatusCode.Forbidden:
                case HttpStatusCode.NotFound:
                case HttpStatusCode.Gone:
                case HttpStatusCode.UnavailableForLegalReasons:
                    {
                        if (restConnetOptions.DiscoveryRedirect)
                        {
                            restConnetOptions.ClearDiscoveryRedirectParameters();
                        }

                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.STOPPED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }
                        break;
                    }
                default:
                    {
                        if (restConnetOptions.DiscoveryRedirect)
                        {
                            restConnetOptions.ClearDiscoveryRedirectParameters();
                        }

                        if (isAsync)
                        {
                            RestEvent restEvent = new RestEvent();
                            restEvent.Type = RestEvent.EventType.FAILED;
                            tokenSession!.RestResponseCallback(restResponse, restEvent);
                        }
                        else
                        {
                            return restResponse;
                        }

                        break;
                    }
            }

            return restResponse;
        }

        internal ReactorReturnCode ParserServiceDiscoveryEndpoint(RestResponse restResponse, List<ReactorServiceEndpointInfo> endpointInfoList, 
            out ReactorErrorInfo? errorInfo)
        {
            try
            {
                using var doc = JsonDocument.Parse(restResponse.Content!);
                var element = doc.RootElement;

                var services = element.GetProperty("services");
                ReactorServiceEndpointInfo endpointInfo;

                endpointInfoList.Clear();

                foreach (var eachElement in services.EnumerateArray())
                {
                    endpointInfo = new ReactorServiceEndpointInfo();

                    endpointInfo.Transport = eachElement.GetProperty("transport").GetString()!;
                    endpointInfo.Port = eachElement.GetProperty("port").GetInt32().ToString();
                    endpointInfo.Provider = eachElement.GetProperty("provider").GetString()!;
                    endpointInfo.EndPoint = eachElement.GetProperty("endpoint").GetString()!;

                    // location
                    var locationArray = eachElement.GetProperty("location");
                    foreach (var innerElement in locationArray.EnumerateArray())
                    {
                        var location = innerElement.GetString();
                        if (location is not null)
                        {
                            endpointInfo.LocationList.Add(location);
                        }
                    }

                    // dataFormat
                    var dataFormatArray = eachElement.GetProperty("dataFormat");
                    foreach (var innerElement in dataFormatArray.EnumerateArray())
                    {
                        var dataformat = innerElement.GetString();
                        if (dataformat is not null)
                        {
                            endpointInfo.DataFormatList.Add(dataformat);
                        }
                    }

                    endpointInfoList.Add(endpointInfo);
                }
            }
            catch (Exception ex)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "ReactorRestClient.SendServiceDirectoryRequest", $"Failed to perform a REST request to the service discovery. Text: {ex.Message}");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    foreach(var value in httpClientDict.Values)
                    {
                        value.Dispose();
                    }

                    httpClientDict.Clear();
                }

                disposedValue = true;
            }
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }
}
