/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using Moq;

namespace LSEG.Ema.Access.Tests;

public class ServiceEndpointDiscoveryTests
{
    [Fact]
    public void QueryServiceDiscoveryProviderFactoryFailsTest()
    {
        static IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo() { Code = ReactorReturnCode.FAILURE };
            return null;
        }

        Assert.Throws<OmmInvalidUsageException>(() => new ServiceEndpointDiscovery(QueryServiceDiscoveryProviderFactory));
    }

    [Fact]
    public void RegisterClientFailsTest()
    {
        Mock<IQueryServiceDiscoveryProvider> provider = new();
        ReactorErrorInfo? error = It.IsAny<ReactorErrorInfo>();
        provider.Setup(p => p.QueryServiceDiscovery(It.Is<ReactorServiceDiscoveryOptions>(o => o.ClientId.ToString() == "ClientId"), out error)).Returns(ReactorReturnCode.FAILURE);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider.Object;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new()
        {
            ClientId = "ClientId"
        };
        Mock<IServiceEndpointDiscoveryClient> client = new();
        Assert.Throws<OmmInvalidUsageException>(() => serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client.Object));
    }

    [Fact]
    public void CallsQueryServiceDiscoveryTest()
    {
        Mock<IQueryServiceDiscoveryProvider> provider = new();
        ReactorErrorInfo? error = It.IsAny<ReactorErrorInfo>();
        provider.Setup(p => p.QueryServiceDiscovery(It.Is<ReactorServiceDiscoveryOptions>(o => o.ClientId.ToString() == "ClientId"), out error)).Returns(ReactorReturnCode.SUCCESS);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider.Object;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new()
        {
            ClientId = "ClientId"
        };
        Mock<IServiceEndpointDiscoveryClient> client = new();
        serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client.Object);
        ReactorErrorInfo? errorInfo = It.IsAny<ReactorErrorInfo?>();
        var clientIdBuffer = new Eta.Codec.Buffer();
        clientIdBuffer.Data("ClientId");
        provider.Verify(p => p.QueryServiceDiscovery(It.Is<ReactorServiceDiscoveryOptions>(o => o.ClientId.Equals(clientIdBuffer)), out errorInfo));
    }

    [Fact]
    public void OnErrorTest()
    {
        Mock<IQueryServiceDiscoveryProvider> provider = new();
        ReactorErrorInfo? error = It.IsAny<ReactorErrorInfo>();
        provider.Setup(p => p.QueryServiceDiscovery(It.IsAny<ReactorServiceDiscoveryOptions>(), out error)).Returns(ReactorReturnCode.SUCCESS);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider.Object;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new();
        Mock<IServiceEndpointDiscoveryClient> client = new();
        serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client.Object);
        serviceEndpointDiscoveryImpl.ReactorServiceEndpointEventCallback(new ReactorServiceEndpointEvent() { ReactorErrorInfo = new ReactorErrorInfo() { Code = ReactorReturnCode.FAILURE } });
        client.Verify(c => c.OnError(It.IsAny<string>(), It.IsAny<ServiceEndpointDiscoveryEvent>()));
    }

    [Fact]
    public void OnSuccessTest()
    {
        Mock<IQueryServiceDiscoveryProvider> provider = new();
        ReactorErrorInfo? error = It.IsAny<ReactorErrorInfo>();
        provider.Setup(p => p.QueryServiceDiscovery(It.IsAny<ReactorServiceDiscoveryOptions>(), out error)).Returns(ReactorReturnCode.SUCCESS);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider.Object;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new();
        Mock<IServiceEndpointDiscoveryClient> client = new();
        serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client.Object);
        serviceEndpointDiscoveryImpl.ReactorServiceEndpointEventCallback(new ReactorServiceEndpointEvent() { ReactorErrorInfo = new ReactorErrorInfo() { Code = ReactorReturnCode.SUCCESS } });
        client.Verify(c => c.OnSuccess(It.IsAny<ServiceEndpointDiscoveryResp>(), It.IsAny<ServiceEndpointDiscoveryEvent>()));
    }
}