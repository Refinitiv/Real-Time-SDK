/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;


namespace LSEG.Eta.ValuedAdd.Tests;


/// <summary>
/// Represents a consumer component.
/// </summary>
public class Consumer : TestReactorComponent, IConsumerCallback
{
    public Consumer(TestReactor testReactor) : base(testReactor)
    {
        ReactorRole = new ConsumerRole();
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
}
