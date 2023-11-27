/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.PerfTools.Common;
using System;
using System.Collections.Generic;
using System.Threading;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    private readonly string? consumerInstanceName;

    public AppClient(string consumerInstanceName)
    {
        this.consumerInstanceName = consumerInstanceName;
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.Write("[" + consumerInstanceName + "] ");
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
        Console.WriteLine("Item State: " + refreshMsg.State());
        if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());
        else
            Console.Write(".");
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.Write("[" + consumerInstanceName + "] ");
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.Write("[" + consumerInstanceName + "] ");
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    private void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + fieldEntry.Load is not null ? AsString(fieldEntry.Load!.DataType) : "" + " Value: ");

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
    }
}

internal class ConsumerInstance
{
    private readonly AppClient _appClient;
    private readonly OmmConsumer _consumer;
    private readonly RequestMsg _reqMsg = new();
    private readonly string[] items;
    private readonly string serviceName;
    private readonly Thread _thread;

    public ConsumerInstance(string consumerInstanceName, string host, string username, string[] items, string serviceName)
    {
        _appClient = new AppClient(consumerInstanceName);
        _consumer = new(new OmmConsumerConfig().Host(host).UserName(username));
        this.items = items;
        this.serviceName = serviceName;
        _thread = new Thread(() => Run());
    }

    public void OpenItem(string item, string serviceName)
    {
        _reqMsg.Clear().Name(item).ServiceName(serviceName).InterestAfterRefresh(false);
        _consumer.RegisterClient(_reqMsg, _appClient);
    }

    public void Start() => _thread.Start();

    private void Run()
    {
        foreach (string item in items)
        {
            Thread.Sleep(10);
            _reqMsg.Clear().Name(item).ServiceName(serviceName).InterestAfterRefresh(false);
            _consumer.RegisterClient(_reqMsg, _appClient);
        }
        Thread.Sleep(60000);
        _consumer.Uninitialize();
        Thread.Sleep(5000);
    }
}

public class Consumer
{
    public static void Main()
    {
        try
        {
            XmlItemInfoList _xmlItemInfoList;
            _xmlItemInfoList = new XmlItemInfoList(20000);
            string fileName = "20k.xml";
            if (_xmlItemInfoList.ParseFile(fileName) == PerfToolsReturnCode.FAILURE)
            {
                Console.Write($"Failed to load item list from file '{fileName}'.\n");
                Environment.Exit(-1);
            }

            string server = "localhost:14002";
            bool testThread = true;

            if (testThread)
            {
                List<string> items1 = new();
                List<string> items2 = new();
                for (int i = 0; i < _xmlItemInfoList.ItemInfoList.Length; i++)
                    if (_xmlItemInfoList.ItemInfoList[i].Name != null)
                    {
                        if (i % 2 == 0)
                        {
                            items1.Add(_xmlItemInfoList.ItemInfoList[i].Name!);
                        }
                        else
                        {
                            items2.Add(_xmlItemInfoList.ItemInfoList[i].Name!);
                        }
                    }
                string[] itemsString1 = items1.ToArray();
                string[] itemsString2 = items2.ToArray();

                Console.Error.WriteLine("init consumer1");
                ConsumerInstance consumer1 = new("consumer1", server, "user1", itemsString1, "DIRECT_FEED");
                consumer1.Start();
                Console.Error.WriteLine("init consumer2");
                ConsumerInstance consumer2 = new("consumer2", server, "user2", itemsString2, "DIRECT_FEED");
                consumer2.Start();
            }
            else
            {
                List<string> items = new();
                for (int i = 0; i < _xmlItemInfoList.ItemInfoList.Length; i++)
                    if (_xmlItemInfoList.ItemInfoList[i].Name != null)
                    {
                        items.Add(_xmlItemInfoList.ItemInfoList[i].Name!);
                    }
                string[] itemsString = items.ToArray();

                Console.Error.WriteLine("init consumer");
                ConsumerInstance consumer = new("consumer", server, "user", itemsString, "DIRECT_FEED");
                consumer.Start();
            }
            Thread.Sleep(60000);
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp);
        }
    }
}