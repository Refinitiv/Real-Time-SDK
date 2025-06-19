/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppClient : IOmmProviderClient
{

    // APIQA
    public static int NUMOFITEMUPDATEFORTEST = 600; // control how long the app is running
    public static bool USERDISPATCH = false; // test case for diff dispatch mode with channel set
    public static bool DIRADMINCONTROL = false; // test case admin Control with channel set
    public static bool TESTCHANNELINFOWITHLOGINHANDLE = false;

    bool m_sendRefreshMsg = false;
    // END APAQA
    bool m_connectionUp;

    public bool IsConnectionUp()
    {
        return m_connectionUp;
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent evt)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + evt.Handle + " Closure: " + evt.Closure);
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
        Console.WriteLine("Item State: " + refreshMsg.State());

        // APIQA 
        Console.WriteLine("event channel info (refresh)\n" + evt.ChannelInformation());
        // END APIQA

        if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN)
        {
            if (refreshMsg.State().DataState == OmmState.DataStates.OK)
            {
                m_connectionUp = true;
                // APIQA
                m_sendRefreshMsg = true;
                // END APIQA
            }
            else
                m_connectionUp = false;
        }
        else
            m_connectionUp = false;
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent evt)
    {
        Console.WriteLine("Received Status. Item Handle: " + evt.Handle + " Closure: " + evt.Closure);
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));
        Console.WriteLine("Item State: " + statusMsg.State());
        // APIQA 
        Console.WriteLine("event channel info (status)\n" + evt.ChannelInformation());
        // END APIQA
    }
    // APIQA
    public bool SendRefreshMsg()
    {
        return m_sendRefreshMsg;
    }

    public void SendRefreshMsg(bool sending)
    {
        m_sendRefreshMsg = sending;
    }
    // END APIQA
    public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent evt) { }
    public void OnAllMsg(Msg msg, IOmmProviderEvent evt) { }
    public void OnClose(RequestMsg reqMsg, IOmmProviderEvent evt)
    {
        Console.WriteLine("channel information (niprovider close event):\n" + evt.ChannelInformation());
    }
}

public class NIProvider
{
    // APIQA
    public static void SendDirRefresh(OmmProvider provider)
    {
        long sourceDirectoryHandle = 1;
        OmmArray capablities = new OmmArray();
        capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
        capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
        OmmArray dictionaryUsed = new OmmArray();
        dictionaryUsed.AddAscii("RWFFld");
        dictionaryUsed.AddAscii("RWFEnum");
        ElementList serviceInfoId = new ElementList();
        serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "NI_PUB");
        serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities.Complete());
        serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed.Complete());
        ElementList serviceStateId = new ElementList();
        serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
        FilterList filterList = new FilterList();
        filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId.Complete());
        filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId.Complete());
        Map map = new Map();
        map.AddKeyUInt(2, MapAction.ADD, filterList.Complete());

        RefreshMsg dirRefreshMsg = new RefreshMsg();
        provider.Submit(dirRefreshMsg.DomainType(EmaRdm.MMT_DIRECTORY)
            .Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).Payload(map.Complete()).Complete(true), sourceDirectoryHandle);
    }

    public static void PrintHelp(bool reflect)
    {
        if (!reflect)
        {
            Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n\n" + "  -numOfUpdatesForApp \tSend the number of item updates for the whole test [default = 600]\n"
                    + "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n" + "  -dirAdminControl \tSet if user controls sending directory msg [default = false]\n"
                    + "  -testChannelInfoWithLoginHandle \tSet for testing ChannelInformation and register Login client [default = false]\n" + "\n");

            Environment.Exit(-1);
        }
        else
        {
            Console.WriteLine("\n  Options will be used:\n" + "  -numOfUpdatesForApp \t " + AppClient.NUMOFITEMUPDATEFORTEST + "\n" + "  -userDispatch \t " + AppClient.USERDISPATCH + "\n"
                    + "  -dirAdminControl \t " + AppClient.DIRADMINCONTROL + "\n" + "  -testChannelInfoWithLoginHandle \t " + AppClient.TESTCHANNELINFOWITHLOGINHANDLE + "\n" + "\n");
        }
    }

    public static bool ReadCommandlineArgs(string[] argv)
    {
        int count = argv.Length;
        int idx = 0;

        while (idx < count)
        {
            if (0 == argv[idx].CompareTo("-?"))
            {
                PrintHelp(false);
                return false;
            }
            else if (argv[idx].Equals("-numOfUpdatesForApp", StringComparison.InvariantCultureIgnoreCase))
            {
                if (++idx >= count)
                {
                    PrintHelp(false);
                    return false;
                }
                if(!int.TryParse(argv[idx], out AppClient.NUMOFITEMUPDATEFORTEST))
                {
                    PrintHelp(false);
                    return false;
                }
                ++idx;
            }
            else if (argv[idx].Equals("-userDispatch", StringComparison.InvariantCultureIgnoreCase))
            {
                if (++idx >= count)
                {
                    PrintHelp(false);
                    return false;
                }
                AppClient.USERDISPATCH = argv[idx].ToLower() == "true" ? true : false;
                ++idx;
            }
            else if (argv[idx].Equals("-dirAdminControl", StringComparison.InvariantCultureIgnoreCase))
            {
                if (++idx >= count)
                {
                    PrintHelp(false);
                    return false;
                }
                AppClient.DIRADMINCONTROL = argv[idx].ToLower() == "true" ? true : false;
                ++idx;
            }
            else if (argv[idx].Equals("-testChannelInfoWithLoginHandle", StringComparison.InvariantCultureIgnoreCase))
            {
                if (++idx >= count)
                {
                    PrintHelp(false);
                    return false;
                }
                AppClient.TESTCHANNELINFOWITHLOGINHANDLE = argv[idx].ToLower() == "true" ? true : false;
                ++idx;
            }
            else
            {
                PrintHelp(false);
                return false;
            }
        }

        PrintHelp(true);
        return true;
    }

    // END APIQA
    public static void Main(string[] args)
    {
        AppClient appClient = new AppClient();
        OmmProvider? provider = null;
        try
        {
            if (!ReadCommandlineArgs(args))
                return;
            OmmNiProviderConfig config = new OmmNiProviderConfig();
            ChannelInformation ci = new ChannelInformation();
            // APIQA
            if (AppClient.USERDISPATCH)
                config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH);
            if (AppClient.DIRADMINCONTROL)
                config.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL);
            if (AppClient.TESTCHANNELINFOWITHLOGINHANDLE)
                provider = new OmmProvider(config.UserName("user"));
            else
                provider = new OmmProvider(config.UserName("user"), appClient);

            provider.ChannelInformation(ci);
            Console.WriteLine("channel information (niprovider): " + ci);

            if (AppClient.TESTCHANNELINFOWITHLOGINHANDLE)
                provider.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), appClient);
            if (AppClient.DIRADMINCONTROL)
                SendDirRefresh(provider);
            if (AppClient.USERDISPATCH)
                provider.Dispatch(1000000);
            // END APIQA
            long itemHandle = 6;
            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), itemHandle);
            // APIQA

            if (AppClient.USERDISPATCH)
                provider.Dispatch(1000000);
            appClient.SendRefreshMsg(false);
            for (int i = 0; i < AppClient.NUMOFITEMUPDATEFORTEST; i++)
            // END APIQA
            {
                if (appClient.IsConnectionUp())
                {
                    // APIQA
                    if (appClient.SendRefreshMsg())
                    {
                        if (AppClient.USERDISPATCH)
                            provider.Dispatch(1000000);
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 14700 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        try
                        {
                            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                                    .Payload(fieldList.Complete()).Complete(true), itemHandle);
                        }
                        catch (OmmException excp)
                        {
                            Console.WriteLine(excp.Message);
                        }
                        appClient.SendRefreshMsg(false);
                        // END APIQA
                    }
                    else
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        // APIQA
                        try
                        {
                            provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), itemHandle);
                        }
                        catch (OmmException excp)
                        {
                            Console.WriteLine(excp.Message);
                            Thread.Sleep(1000);
                        }
                    }
                }
                try
                {
                    if (AppClient.USERDISPATCH)
                        provider.Dispatch(1000000);
                }
                catch (OmmException excp)
                {
                    Console.WriteLine(excp.Message);
                }
                Thread.Sleep(1000);
            }
        }
        catch (OmmException excp)
		// END APIQA
		{
            Console.WriteLine(excp.Message);
        }
        finally
        {
            provider?.Uninitialize();
        }
    }
}
