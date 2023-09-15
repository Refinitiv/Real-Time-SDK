/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Text;
using System.Threading;
using static LSEG.Ema.Access.DataformatProtocol;
using static LSEG.Ema.Access.TransportProtocol;

namespace LSEG.Ema.Access;

internal delegate IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo);

/// <summary>
/// Allows to query service discovery(this interface is exposed as public for unit testing only).
/// </summary>
public interface IQueryServiceDiscoveryProvider : IDisposable
{
    /// <summary>
    /// Queries EDP-RT service discovery to get service endpoint information.
    /// </summary>
    /// <returns><see cref="ReactorReturnCode"/> indicating success or failure</returns>
    ReactorReturnCode? QueryServiceDiscovery(ReactorServiceDiscoveryOptions serviceDiscoveryOptions, out ReactorErrorInfo? errorInfo);
}

internal class QueryServiceDiscoveryProvider : IQueryServiceDiscoveryProvider
{
    private ReaderWriterLockSlim _usrLock = new(LockRecursionPolicy.SupportsRecursion);
    private readonly Reactor? _reactor;

    public QueryServiceDiscoveryProvider(Reactor? reactor)
    {
        _reactor = reactor;
    }

    /// <summary>
    /// Queries EDP-RT service discovery to get service endpoint information.
    /// </summary>
    /// <returns>ReactorReturnCode</returns>
    public ReactorReturnCode? QueryServiceDiscovery(ReactorServiceDiscoveryOptions serviceDiscoveryOptions, out ReactorErrorInfo? errorInfo)
    {
        _usrLock.EnterWriteLock();
        try
        {
            errorInfo = null;
            return _reactor?.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo);
        }
        finally
        {
            _usrLock.ExitWriteLock();
        }
    }
    /// <summary>
    /// Safelly disposes the provider.
    /// </summary>
    public void Dispose()
    {
        _usrLock.EnterWriteLock();
        try
        {
            if ((!_reactor?.IsShutdown) ?? false)
            {
                _reactor?.Shutdown(out _);
            }
        }
        finally
        {
            _usrLock.ExitWriteLock();
        }
    }
}

/// <summary>
/// ServiceEndpointDiscovery provides the functionality to query endpoints from RDP service discovery.
/// </summary>
public sealed class ServiceEndpointDiscovery : IReactorServiceEndpointEventCallback
{
    private IQueryServiceDiscoveryProvider? _queryServiceDiscoveryProvider;
    private ReactorErrorInfo? _reactorErrorInfo = new();
    private ReactorServiceDiscoveryOptions _reactorServiceDiscoveryOptions = new();
    private ServiceEndpointDiscoveryResp _serviceEndpointDiscoveryResp = new();
    private ServiceEndpointDiscoveryEvent _ServiceEndpointDiscoveryEvent = new();
    private IServiceEndpointDiscoveryClient? _client;
    private StringBuilder? _strBuilder;

    /// <summary>
    /// Default constructor.
    /// </summary>
    /// <exception cref="OmmInvalidUsageException">Cannot create Reactor.</exception>
    public ServiceEndpointDiscovery() : this(
        delegate (ReactorOptions ro, out ReactorErrorInfo? e) { return new QueryServiceDiscoveryProvider(Reactor.CreateReactor(ro, out e)); })
    {
    }

    internal ServiceEndpointDiscovery(QueryServiceDiscoveryProviderFactory queryServiceDiscoveryProvider)
    {
        ReactorOptions reactorOptions = new()
        {
            UserSpecObj = this
        };
        _queryServiceDiscoveryProvider = queryServiceDiscoveryProvider(reactorOptions, out _reactorErrorInfo);
        if ((_reactorErrorInfo?.Code ?? ReactorReturnCode.SUCCESS) != ReactorReturnCode.SUCCESS)
        {
            var strBuilder = GetStringBuilder().Append("Failed to create ServiceEndpointDiscovery (ReactorFactory.createReactor).")
            .Append("' Error Id='").Append(_reactorErrorInfo?.Error.ErrorId).Append("' Internal sysError='")
            .Append(_reactorErrorInfo?.Error.SysError).Append("' Error Location='")
            .Append(_reactorErrorInfo?.Location).Append("' Error Text='")
            .Append(_reactorErrorInfo?.Error.Text).Append("'. ");
            throw new OmmInvalidUsageException(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
        }
    }

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="tokenServiceUrlV2"></param>
    /// <param name="serviceDiscoveryUrl"></param>
    /// <exception cref="OmmInvalidUsageException"></exception>
    public ServiceEndpointDiscovery(string tokenServiceUrlV2, string serviceDiscoveryUrl)
         : this(tokenServiceUrlV2, serviceDiscoveryUrl,
               delegate (ReactorOptions ro, out ReactorErrorInfo? e) { return new QueryServiceDiscoveryProvider(Reactor.CreateReactor(ro, out e)); })
    {
    }

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="tokenServiceUrlV2"></param>
    /// <param name="serviceDiscoveryUrl"></param>
    /// <param name="queryServiceDiscoveryProvider"></param>
    /// <exception cref="OmmInvalidUsageException"></exception>
    internal ServiceEndpointDiscovery(string tokenServiceUrlV2, string serviceDiscoveryUrl, QueryServiceDiscoveryProviderFactory queryServiceDiscoveryProvider)
    {
        ReactorOptions reactorOptions = new();
        reactorOptions.Clear();

        reactorOptions.UserSpecObj = this;

        if (!string.IsNullOrEmpty(tokenServiceUrlV2))
        {
            reactorOptions.SetTokenServiceURL(tokenServiceUrlV2);
        }
        if (!string.IsNullOrEmpty(serviceDiscoveryUrl))
        {
            reactorOptions.SetServiceDiscoveryURL(serviceDiscoveryUrl);
        }

        _queryServiceDiscoveryProvider = queryServiceDiscoveryProvider(reactorOptions, out _reactorErrorInfo);
        if ((_reactorErrorInfo?.Code ?? ReactorReturnCode.SUCCESS) != ReactorReturnCode.SUCCESS)
        {
            var strBuilder = GetStringBuilder().Append("Failed to create ServiceEndpointDiscovery (ReactorFactory.createReactor).")
            .Append("' Error Id='").Append(_reactorErrorInfo?.Error.ErrorId).Append("' Internal sysError='")
            .Append(_reactorErrorInfo?.Error.SysError).Append("' Error Location='")
            .Append(_reactorErrorInfo?.Location).Append("' Error Text='")
            .Append(_reactorErrorInfo?.Error.Text).Append("'. ");

            throw new OmmInvalidUsageException(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
        }
    }

    /// <inheritdoc/>
    public void RegisterClient(ServiceEndpointDiscoveryOption option, IServiceEndpointDiscoveryClient client)
    {
        if (client == null)
        {
            throw new OmmInvalidUsageException("Client not set", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        RegisterClient(option, client, null);
    }

    /// <inheritdoc/>
    public void RegisterClient(ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption, IServiceEndpointDiscoveryClient client, object? closure)
    {
        if (client == null)
        {
            throw new OmmInvalidUsageException("Client not set", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }
        _reactorServiceDiscoveryOptions.Clear();
        _reactorServiceDiscoveryOptions.ClientId.Data(serviceEndpointDiscoveryOption.ClientId);
        _reactorServiceDiscoveryOptions.ClientSecret.Data(serviceEndpointDiscoveryOption.ClientSecret);
        _reactorServiceDiscoveryOptions.ClientJwk.Data(serviceEndpointDiscoveryOption.ClientJWK);
        _reactorServiceDiscoveryOptions.Audience.Data(serviceEndpointDiscoveryOption.Audience);
        if (!string.IsNullOrEmpty(serviceEndpointDiscoveryOption.ProxyHostName))
        {
            _reactorServiceDiscoveryOptions.ProxyHostName.Data(serviceEndpointDiscoveryOption.ProxyHostName);
        }

        if (!string.IsNullOrEmpty(serviceEndpointDiscoveryOption.ProxyPort))
        {
            _reactorServiceDiscoveryOptions.ProxyPort.Data(serviceEndpointDiscoveryOption.ProxyPort);
        }

        if (!string.IsNullOrEmpty(serviceEndpointDiscoveryOption.ProxyUserName))
        {
            _reactorServiceDiscoveryOptions.ProxyUserName.Data(serviceEndpointDiscoveryOption.ProxyUserName);
        }

        if (!string.IsNullOrEmpty(serviceEndpointDiscoveryOption.ProxyPassword))
        {
            _reactorServiceDiscoveryOptions.ProxyPassword.Data(serviceEndpointDiscoveryOption.ProxyPassword);
        }

        _reactorServiceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
        _client = client;

        if (closure != null) _reactorServiceDiscoveryOptions.UserSpecObject = closure;

        switch (serviceEndpointDiscoveryOption.Transport)
        {
            case TransportProtocol.UNKNOWN:
                break;

            case TCP:
                _reactorServiceDiscoveryOptions.Transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
                break;

            case WEB_SOCKET:
                _reactorServiceDiscoveryOptions.Transport = ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET;
                break;

            default:
                throw new OmmInvalidUsageException(
                    $"Invalid transport protocol {serviceEndpointDiscoveryOption.Transport} specified in ServiceEndpointDiscoveryOption.Transport", 
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        switch (serviceEndpointDiscoveryOption.DataFormat)
        {
            case DataformatProtocol.UNKNOWN:
                break;

            case RWF:
                _reactorServiceDiscoveryOptions.DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_RWF;
                break;

            case JSON2:
                _reactorServiceDiscoveryOptions.DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2;
                break;

            default:
                throw new OmmInvalidUsageException(
                    $"Invalid dataformat protocol {serviceEndpointDiscoveryOption.DataFormat} specified in ServiceEndpointDiscoveryOption.DataFormat", 
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        if (_queryServiceDiscoveryProvider?.QueryServiceDiscovery(_reactorServiceDiscoveryOptions, out _reactorErrorInfo) != ReactorReturnCode.SUCCESS)
        {
            var sb = GetStringBuilder().Append("Failed to query service discovery (Reactor.QueryServiceDiscovery).")
            .Append("' Error Id='").Append(_reactorErrorInfo?.Error.ErrorId).Append("' Internal sysError='")
            .Append(_reactorErrorInfo?.Error.SysError).Append("' Error Location='")
            .Append(_reactorErrorInfo?.Location).Append("' Error Text='")
            .Append(_reactorErrorInfo?.Error.Text).Append("'. ");

            throw new OmmInvalidUsageException(sb.ToString());
        }
    }

    /// <inheritdoc/>
    public void Uninitialize()
    {
        _queryServiceDiscoveryProvider?.Dispose();
    }

    private StringBuilder GetStringBuilder()
    {
        if (_strBuilder == null)
        {
            _strBuilder = new StringBuilder(255);
        }
        else
        {
            _strBuilder.Length = 0;
        }

        return _strBuilder;
    }

    /// <inheritdoc/>
    public ReactorCallbackReturnCode ReactorServiceEndpointEventCallback(ReactorServiceEndpointEvent @event)
    {
        ReactorServiceEndpointInfo reactorServiceEndpointInfo;
        ServiceEndpointDiscoveryInfo serviceEndpointDiscoveryInfo;

        _ServiceEndpointDiscoveryEvent.Closure = @event.UserSpecObject;
        _ServiceEndpointDiscoveryEvent.ServiceEndpointDiscovery = this;
        if (@event.ReactorErrorInfo.Code == ReactorReturnCode.SUCCESS)
        {
            _serviceEndpointDiscoveryResp.ServiceEndpointInfoList.Clear();
            for (int index = 0; index < @event.ServiceEndpointInfoList?.Count; index++)
            {
                reactorServiceEndpointInfo = @event.ServiceEndpointInfoList[index];
                serviceEndpointDiscoveryInfo = new ServiceEndpointDiscoveryInfo();

                for (int valueIndex = 0; valueIndex < reactorServiceEndpointInfo.DataFormatList.Count; valueIndex++)
                {
                    serviceEndpointDiscoveryInfo.DataFormatList.Add(reactorServiceEndpointInfo.DataFormatList[valueIndex]);
                }

                for (int valueIndex = 0; valueIndex < reactorServiceEndpointInfo.LocationList.Count; valueIndex++)
                {
                    serviceEndpointDiscoveryInfo.LocationList.Add(reactorServiceEndpointInfo.LocationList[valueIndex]);
                }

                serviceEndpointDiscoveryInfo.Endpoint = reactorServiceEndpointInfo.EndPoint;
                serviceEndpointDiscoveryInfo.Port = reactorServiceEndpointInfo.Port;
                serviceEndpointDiscoveryInfo.Provider = reactorServiceEndpointInfo.Provider;
                serviceEndpointDiscoveryInfo.Transport = reactorServiceEndpointInfo.Transport;
                _serviceEndpointDiscoveryResp.ServiceEndpointInfoList.Add(serviceEndpointDiscoveryInfo);
            }

            _client?.OnSuccess(_serviceEndpointDiscoveryResp, _ServiceEndpointDiscoveryEvent);
        }
        else
        {
            _client?.OnError(@event.ReactorErrorInfo.Error.Text, _ServiceEndpointDiscoveryEvent);
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }
}