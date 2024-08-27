/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#define Debug

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;

#if Debug
using static LSEG.Ema.Access.DataType;
#endif

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
    {
#if Debug
        Console.WriteLine(Thread.CurrentThread.ManagedThreadId);
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>" + " thread: " + Thread.CurrentThread.ManagedThreadId));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
#endif
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _)
    {
#if Debug
        Console.WriteLine(Thread.CurrentThread.ManagedThreadId);
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>" + " thread: " + Thread.CurrentThread.ManagedThreadId));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
#endif
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _)
    {
#if Debug
        Console.WriteLine(Thread.CurrentThread);
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>" + " thread: " + Thread.CurrentThread.ManagedThreadId));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
#endif
    }

    private void Decode(FieldList fieldList)
    {
#if Debug
        Console.WriteLine(Thread.CurrentThread);
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;

                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
                        break;

                    case DataTypes.TIME:
                        Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
                        break;

                    case DataTypes.INT:
                        Console.WriteLine(fieldEntry.IntValue());
                        break;

                    case DataTypes.UINT:
                        Console.WriteLine(fieldEntry.UIntValue());
                        break;

                    case DataTypes.ASCII:
                        Console.WriteLine(fieldEntry.OmmAsciiValue());
                        break;

                    case DataTypes.ENUM:
                        Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
                        break;

                    case DataTypes.RMTES:
                        Console.WriteLine(fieldEntry.OmmRmtesValue());
                        break;

                    case DataTypes.ERROR:
                        Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;

                    default:
                        Console.WriteLine();
                        break;
                }
        }
#endif
    }
}

internal class ConsThread
{
    private readonly OmmConsumer? consumer = null;
    private readonly Thread thread;
    private readonly ElementList view;
    private readonly AppClient appClient = new();
    private readonly string uniqueName;

    public ConsThread(ElementList view, string uniqueName)
    {
        this.view = view;
        this.uniqueName = uniqueName;
        thread = new Thread(Run);
        consumer = new OmmConsumer(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
    }

    public void Start() => thread.Start();

    private void Run()
    {
        RequestMsg request = new();
        request.Clear();
        for (int i = 0; i < 1000; ++i)
        {
            long itemHandle = consumer!.RegisterClient(request.ServiceName("DIRECT_FEED").Name("IBM.N").Payload(view), appClient);

#if Debug
            Console.WriteLine(uniqueName + "   registered - Trial " + i);
#endif
            try
            {
                Thread.Sleep(i);
            }
            catch (ThreadInterruptedException e)
            {
                Console.WriteLine(e);
            }

            request.Clear();
            try
            {
                consumer.Reissue(request, itemHandle);
            }
            catch (OmmInvalidHandleException invalidHandleException)
            {
                Console.WriteLine("InvalidHandle: " + invalidHandleException.Handle);
            }
            catch
#if Debug
                (OmmInvalidUsageException excp)
#endif
            {
#if Debug
                Console.WriteLine(excp.Message);
#endif
            }

            try
            {
                Thread.Sleep(i);
            }
            catch (ThreadInterruptedException e)
            {
                Console.WriteLine(e);
            }

            consumer.Unregister(itemHandle);

#if Debug
            Console.WriteLine(uniqueName + "   unregistered - Trial " + i);
#endif
        }
    }
}

public class Consumer
{
    public static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            OmmArray array = new()
            {
                FixedWidth = 2
            };

            array.AddInt(22)
                .AddInt(25)
                .Complete();

            var view = new ElementList()
                .AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1)
                .AddArray(EmaRdm.ENAME_VIEW_DATA, array)
                .Complete();

            ConsThread consumerThread_One = new(view, "A");
            consumerThread_One.Start();
            ConsThread consumerThread_Two = new(view, "B");
            consumerThread_Two.Start();
            ConsThread consumerThread_Three = new(view, "C");
            consumerThread_Three.Start();
            ConsThread consumerThread_Four = new(view, "D");
            consumerThread_Four.Start();
            ConsThread consumerThread_Five = new(view, "E");
            consumerThread_Five.Start();
            ConsThread consumerThread_Six = new(view, "F");
            consumerThread_Six.Start();
            ConsThread consumerThread_Seven = new(view, "G");
            consumerThread_Seven.Start();

            Thread.Sleep(60000);            // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}