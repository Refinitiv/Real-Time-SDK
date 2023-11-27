/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
//QATools modify to send multiple item request 

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using static LSEG.Ema.Access.OmmConsumerConfig;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine(refreshMsg);
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine(updateMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine(statusMsg);
    }
}

public class Consumer
{
    public static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();

            OmmConsumerConfig config = new();

            consumer = new OmmConsumer(config.OperationModel(OperationModelMode.USER_DISPATCH).Host("localhost:14002").UserName("user"));

            RequestMsg reqMsg = new();

            consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
            reqMsg.Clear();
            consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("TRI.N"), appClient);
            reqMsg.Clear();
            consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("TRI1.N"), appClient);
            reqMsg.Clear();
            consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("TRI2.N"), appClient);

            var startTime = DateTime.Now;
            while (DateTime.Now < startTime + TimeSpan.FromMilliseconds(60000))
                consumer.Dispatch(10); // calls to OnRefreshMsg(),
                                       // OnUpdateMsg(), or OnStatusMsg()
                                       // execute on this thread
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}

//END APIQA
