/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using System;
using System.Threading;
using System.Collections.Generic;
using LSEG.Ema.Access;

public class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(refreshMsg);
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(updateMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(statusMsg);
    }
}

public class Consumer
{
    private const string DEFAULT_SERVICE_NAME_1 = "DIRECT_FEED";
    private const string DEFAULT_SERVICE_NAME_2 = "DIRECT_FEED";
    private const string DEFAULT_CONSUMER_NAME = "Consumer_7";
    private const string DEFAULT_ITEM_NAME_1 = "IBM.N";
    private const string DEFAULT_ITEM_NAME_2 = "TRI.N";

    private const string SERVICE_NAME_1 = "serviceName1";
    private const string SERVICE_NAME_2 = "serviceName2";
    private const string ITEM_NAME_1 = "itemName1";
    private const string ITEM_NAME_2 = "itemName2";

    private static void AddCommandLineArgs()
    {
        CommandLine.ProgramName("Consumer");

        CommandLine.AddOption(
            SERVICE_NAME_1,
            DEFAULT_SERVICE_NAME_1,
            "Specifies first service name. Default value is DIRECT_FEED");

        CommandLine.AddOption(
            SERVICE_NAME_2,
            DEFAULT_SERVICE_NAME_2,
            "Specifies second service name. Default value is DIRECT_FEED");

        CommandLine.AddOption(
            ITEM_NAME_1,
            DEFAULT_ITEM_NAME_1,
            "Specifies first item name. Default value is IBM.N");

        CommandLine.AddOption(
            ITEM_NAME_2,
            DEFAULT_ITEM_NAME_2,
            "Specifies second item name. Default value is TRI.N");
    }

    private static void Init(string[] args)
    {
        AddCommandLineArgs();

        try
        {
            CommandLine.ParseArgs(args);
        }
        catch (ArgumentException ex)
        {
            FinishWithError(ex.Message);
        }
    }

    private static void FinishWithError(string message)
    {
        Console.Error.WriteLine("Error loading command line arguments:");
        Console.Error.WriteLine(message);
        Console.Error.WriteLine();
        Console.Error.WriteLine(CommandLine.OptionHelpString());

        Console.WriteLine("Consumer exits...");
        Environment.Exit(-1);
    }

    static void Main(string[] args)
    {
        Init(args);

        try
        {
            AppClient appClient1 = new();
            AppClient appClient2 = new();

            OmmConsumerConfig config =
                new OmmConsumerConfig()
                    .ConsumerName(DEFAULT_CONSUMER_NAME);

            using OmmConsumer consumer = new OmmConsumer(config);

            consumer.RegisterClient(
                new RequestMsg()
                    .ServiceName(CommandLine.Value(SERVICE_NAME_1))
                    .Name(CommandLine.Value(ITEM_NAME_1)),
                appClient1);

            consumer.RegisterClient(
                new RequestMsg()
                    .ServiceName(CommandLine.Value(SERVICE_NAME_2))
                    .Name(CommandLine.Value(ITEM_NAME_2)),
                appClient2);

            Thread.Sleep(60000);
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
    }
}

public static class CommandLine
{
    private static readonly Dictionary<string, string> Options = new();

    public static void ProgramName(string name)
    {
    }

    public static void AddOption(
        string key,
        string defaultValue,
        string description)
    {
        Options[key] = defaultValue;
    }

    public static void ParseArgs(string[] args)
    {
        foreach (string arg in args)
        {
            if (arg == "-?" ||
                arg == "-help" ||
                arg == "--help")
            {
                Console.WriteLine(OptionHelpString());
                Environment.Exit(0);
            }
        }

        for (int i = 0; i < args.Length; i++)
        {
            if (!args[i].StartsWith("-"))
                continue;

            string key = args[i].TrimStart('-');

            if (i + 1 < args.Length &&
                !args[i + 1].StartsWith("-"))
            {
                Options[key] = args[++i];
            }
        }
    }

    public static string Value(string key)
    {
        return Options.TryGetValue(key, out var value)
            ? value
            : "";
    }

    public static string OptionHelpString()
    {
        return @"
Consumer command line options

-serviceName1 <service>
    Specifies first service name.
    Default: DIRECT_FEED

-serviceName2 <service>
    Specifies second service name.
    Default: DIRECT_FEED

-itemName1 <item>
    Specifies first item name.
    Default: IBM.N

-itemName2 <item>
    Specifies second item name.
    Default: TRI.N

-?
-help
--help
    Display this help message
";
    }
}
