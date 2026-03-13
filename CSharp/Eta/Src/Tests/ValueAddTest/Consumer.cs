/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using System;


namespace LSEG.Eta.Tests.ValueAddTest;


/// <summary>
/// Represents a consumer component.
/// </summary>
public class Consumer : TestReactorComponent, IConsumerCallback
{
    public Consumer(TestReactor testReactor, bool disposeReactor = false) : base(testReactor, disposeReactor)
    {
        ReactorRole = new ConsumerRole();
    }

    public Consumer() : this(new TestReactor(), true)
    { }

    public ConsumerRole Role => (ConsumerRole)ReactorRole;

    public Consumer WithDefaultRole()
    {
        Role.InitDefaultRDMLoginRequest();
        Role.InitDefaultRDMDirectoryRequest();
        Role.ChannelEventCallback = this;
        Role.LoginMsgCallback = this;
        Role.DirectoryMsgCallback = this;
        Role.DictionaryMsgCallback = this;
        Role.DefaultMsgCallback = this;
        return this;
    }

    public Consumer WithWatchlist(Action<ConsumerWatchlistOptions> configure = null)
    {
        Role.WatchlistOptions.EnableWatchlist = true;
        Role.WatchlistOptions.ChannelOpenEventCallback = this;
        configure?.Invoke(Role.WatchlistOptions);
        return this;
    }

    public virtual ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
    {
        return TestReactor.HandleChannelEvent(evt);
    }


    public virtual ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
    {
        return TestReactor.HandleDefaultMsgEvent(evt);
    }


    public virtual ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent evt)
    {
        return TestReactor.HandleLoginMsgEvent(evt);
    }


    public virtual ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent evt)
    {
        return TestReactor.HandleDirectoryMsgEvent(evt);
    }


    public virtual ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent evt)
    {
        return TestReactor.HandleDictionaryMsgEvent(evt);
    }

    public void Connect(ConsumerProviderSessionOptions opts, params int[] portNumbers) =>
        TestReactor.Connect(opts, this, portNumbers);
}
