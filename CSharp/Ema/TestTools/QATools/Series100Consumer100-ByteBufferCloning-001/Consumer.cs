/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using System.Diagnostics;
using System.Threading;

internal class AppClient : IOmmConsumerClient
{
    private static readonly byte[] newPermissionBytes = new byte[] { 0x10, 0x20, 0x30, 0x40 };
    private static readonly byte[] newExtendedHeader = new byte[] { 0x50, 0x51, 0x52, 0x53, 0x54 };
    private static readonly string newName = "TRI.N";

    public static string Replace(string src)
    {
        return src.Replace("03 01 2C 56 25 C0", "10 20 30 40")
                .Replace("61 62 63 64 65 66 67 45 4E 44", "50 51 52 53 54")
                .Replace("IBM.N", "TRI.N")
                .Replace("serviceId=\"1\"", "serviceId=\"123\"");
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("onRefreshMsg START");
        string expectedMsgstring = refreshMsg.ToString();
        Console.WriteLine(expectedMsgstring);

        Console.WriteLine("---");
        RefreshMsg clonedMsg = new(refreshMsg);
        string clonedMsgstring = clonedMsg.ToString();
        Console.WriteLine(clonedMsgstring);
        Debug.Assert(expectedMsgstring.Equals(clonedMsgstring), "RefreshMsg.ToString|() calls should return equal strings");

        Console.WriteLine("---Altering");
        clonedMsg.PermissionData(new EmaBuffer(newPermissionBytes));
        clonedMsg.ExtendedHeader(new EmaBuffer(newExtendedHeader));
        clonedMsg.Name(newName);
        clonedMsg.ServiceId(123);
        clonedMsgstring = clonedMsg.ToString();
        Console.WriteLine(clonedMsgstring);
        expectedMsgstring = Replace(expectedMsgstring);
        Debug.Assert(expectedMsgstring.Equals(clonedMsgstring), "RefreshMsg.ToString|() should match replaced string");

        Console.WriteLine("onRefreshMsg DONE");
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("onUpdateMsg START");
        string expectedMsgstring = updateMsg.ToString();
        Console.WriteLine(expectedMsgstring);

        Console.WriteLine("---");
        UpdateMsg clonedMsg = new(updateMsg);
        string clonedMsgstring = clonedMsg.ToString();
        Console.WriteLine(clonedMsgstring);
        Debug.Assert(expectedMsgstring.Equals(clonedMsgstring), "UpdateMsg.ToString|() calls should return equal strings");
        Console.WriteLine("onUpdateMsg DONE");
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("onStatusMsg START");
        string expectedMsgstring = statusMsg.ToString();
        Console.WriteLine(expectedMsgstring);
        Console.WriteLine("---");
        StatusMsg clonedMsg = new(statusMsg);
        string clonedMsgstring = clonedMsg.ToString();
        Console.WriteLine(clonedMsgstring);
        Debug.Assert(expectedMsgstring.Equals(clonedMsgstring), "StatusMsg.ToString|() calls should return equal strings");
        Console.WriteLine("onStatusMsg END");
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("onGenericMsg START");
        string expectedMsgstring = genericMsg.ToString();
        Console.WriteLine(expectedMsgstring);

        Console.WriteLine("---");
        GenericMsg clonedMsg = new(genericMsg);
        string clonedMsgstring = clonedMsg.ToString();
        Console.WriteLine(clonedMsgstring);
        Debug.Assert(expectedMsgstring.Equals(clonedMsgstring), "GenericMsg.ToString|() calls should return equal strings");
        Console.WriteLine("onGenericMsg END");
    }

    public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine("onAckMsg START");
        string expectedMsgstring = ackMsg.ToString();
        Console.WriteLine(expectedMsgstring);

        Console.WriteLine("---");
        AckMsg clonedMsg = new(ackMsg);
        string clonedMsgstring = clonedMsg.ToString();
        Console.WriteLine(clonedMsgstring);
        Debug.Assert(expectedMsgstring.Equals(clonedMsgstring), "AckMsg.ToString|() calls should return equal strings");
        Console.WriteLine("onAckMsg END");
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
            consumer = new(config.Host("localhost:14002").UserName("user"));
            RequestMsg reqMsg = new();
            consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
            Thread.Sleep(4000);         // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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