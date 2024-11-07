/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using FakeItEasy;

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
        var provider = A.Fake<IQueryServiceDiscoveryProvider>();
        var error = new ReactorErrorInfo();
        A.CallTo(() => provider.QueryServiceDiscovery(A<ReactorServiceDiscoveryOptions>.That.Matches(o => o.ClientId.ToString() == "ClientId"), out error))
            .Returns(ReactorReturnCode.FAILURE);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new()
        {
            ClientId = "ClientId"
        };

        var client = A.Fake<IServiceEndpointDiscoveryClient>();
        Assert.Throws<OmmInvalidUsageException>(() => serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client));
    }

    [Fact]
    public void CallsQueryServiceDiscoveryTest()
    {
        var provider = A.Fake<IQueryServiceDiscoveryProvider>();
        ReactorErrorInfo? error = null;
        A.CallTo(() => provider.QueryServiceDiscovery(A<ReactorServiceDiscoveryOptions>.That.Matches(o => o.ClientId.ToString() == "ClientId"), out error))
            .Returns(ReactorReturnCode.SUCCESS);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new()
        {
            ClientId = "ClientId"
        };

        var client = A.Fake<IServiceEndpointDiscoveryClient>();
        serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client);
        ReactorErrorInfo? errorInfo = null;
        var clientIdBuffer = new Eta.Codec.Buffer();
        clientIdBuffer.Data("ClientId");
        A.CallTo(() => provider.QueryServiceDiscovery(A<ReactorServiceDiscoveryOptions>.That.Matches(o => o.ClientId.Equals(clientIdBuffer)), out errorInfo))
            .MustHaveHappened();
    }

    [Fact]
    public void OnErrorTest()
    {
        var provider = A.Fake<IQueryServiceDiscoveryProvider>();
        ReactorErrorInfo? error = null;
        A.CallTo(() => provider.QueryServiceDiscovery(A<ReactorServiceDiscoveryOptions>._, out error)).Returns(ReactorReturnCode.SUCCESS);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new();
        var client = A.Fake<IServiceEndpointDiscoveryClient>();
        serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client);
        serviceEndpointDiscoveryImpl.ReactorServiceEndpointEventCallback(new ReactorServiceEndpointEvent() { ReactorErrorInfo = new ReactorErrorInfo() { Code = ReactorReturnCode.FAILURE } });
        A.CallTo(() => client.OnError(A<string>._, A<ServiceEndpointDiscoveryEvent>._)).MustHaveHappened();
    }

    [Fact]
    public void OnSuccessTest()
    {
        var provider = A.Fake<IQueryServiceDiscoveryProvider>();
        ReactorErrorInfo? error = null;
        A.CallTo(() => provider.QueryServiceDiscovery(A<ReactorServiceDiscoveryOptions>._, out error)).Returns(ReactorReturnCode.SUCCESS);

        IQueryServiceDiscoveryProvider? QueryServiceDiscoveryProviderFactory(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = new ReactorErrorInfo();
            return provider;
        }

        ServiceEndpointDiscovery serviceEndpointDiscoveryImpl = new(QueryServiceDiscoveryProviderFactory);
        ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption = new();
        var client = A.Fake<IServiceEndpointDiscoveryClient>();
        serviceEndpointDiscoveryImpl.RegisterClient(serviceEndpointDiscoveryOption, client);
        serviceEndpointDiscoveryImpl.ReactorServiceEndpointEventCallback(new ReactorServiceEndpointEvent() { ReactorErrorInfo = new ReactorErrorInfo() { Code = ReactorReturnCode.SUCCESS } });
        A.CallTo(() => client.OnSuccess(A<ServiceEndpointDiscoveryResp>._, A<ServiceEndpointDiscoveryEvent>._)).MustHaveHappened();
    }
}