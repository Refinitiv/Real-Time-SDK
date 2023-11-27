/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.Consumer;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

internal class AppClient : IOmmConsumerClient
{
    private long numOfUpdates = 0;

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
    {
        if (refreshMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + refreshMsg.Name() + " Service Name: " + refreshMsg.ServiceName());

        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        if (updateMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + updateMsg.Name() + " Service Name: " + updateMsg.ServiceName());

        if (DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();

    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _)
    {
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
                        Console.WriteLine(fieldEntry.OmmErrorValue().ErrorCode + " (" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;

                    default:
                        Console.WriteLine();
                        break;
                }
        }
    }
}

public class Consumer
{
    private static String AuthenticationToken = "";
    private static String AppId = "256";

    public static void printHelp()
    {

        Console.WriteLine("\nOptions:\n" + "  -?                            Shows this usage\n\n" + "  -at <token>           Authentication token to use in login request [default = \"\"]\n"
                + "  -aid <applicationId>        ApplicationId set as login Attribute [default = 256]\n" + "\n");
    }

    public static void printInvalidOption()
    {
        Console.WriteLine("Detected a missing argument. Please verify command line options [-?]");
    }

    public static bool init(String[] argv)
    {
        int count = argv.Length;
        int idx = 0;

        while (idx < count)
        {
            if (0 == argv[idx].CompareTo("-?"))
            {
                printHelp();
                return false;
            }
            else if (0 == argv[idx].CompareTo("-aid"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                AppId = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].CompareTo("-at"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                AuthenticationToken = argv[idx];
                ++idx;
            }
            else
            {
                Console.WriteLine("Unrecognized option. Please see command line help. [-?]");
                return false;
            }
        }

        return true;
    }

    private static void printActiveConfig()
    {
        Console.WriteLine("Following options are selected:");

        Console.WriteLine("appId = " + AppId);
        Console.WriteLine("Authentication Token = " + AuthenticationToken);
    }


    public static void Main(String[] args)
    {
        printActiveConfig();
        if (!init(args))
            return;

        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();


            RequestMsg reqMsg = new();
            ElementList elementList = new();
       
            elementList.AddAscii(EmaRdm.ENAME_APP_ID, AppId);
            elementList.AddAscii(EmaRdm.ENAME_POSITION, "127.0.0.1/net");
            elementList.AddAscii(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1.ToString());

            consumer = new(new OmmConsumerConfig().OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH)
                    .AddAdminMsg(reqMsg.DomainType(EmaRdm.MMT_LOGIN).Name(AuthenticationToken).NameType(EmaRdm.USER_NAME).Attrib(elementList))
                    .AddAdminMsg(reqMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER | EmaRdm.SERVICE_GROUP_FILTER)));

            consumer.RegisterClient(reqMsg.Clear().ServiceName("NI_PUB").Name("TRI.N"), appClient, null);

            var endTime = DateTime.Now + TimeSpan.FromMilliseconds(60000);

            while (DateTime.Now < endTime)
                consumer.Dispatch(10); 
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
        finally
        {
            if (consumer != null)
                consumer?.Uninitialize();
        }
    }
}