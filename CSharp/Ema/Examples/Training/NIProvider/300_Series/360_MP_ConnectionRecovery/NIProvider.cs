/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppClient : IOmmProviderClient
{
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

        if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN)
        {
            if (refreshMsg.State().DataState == OmmState.DataStates.OK)
                m_connectionUp = true;
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

        if (statusMsg.HasState)
        {
            Console.WriteLine("Item State: " + statusMsg.State());
            if (statusMsg.State().StreamState == OmmState.StreamStates.OPEN)
            {
                if (statusMsg.State().DataState == OmmState.DataStates.OK)
                    m_connectionUp = true;
                else
                {
                    m_connectionUp = false;
                }
            }
            else
                m_connectionUp = false;
        }
    }
}


public class NIProvider
{
    public static void Main()
    {
        AppClient appClient = new AppClient();
        bool sendRefreshMsg = false;

        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            provider = new OmmProvider(config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH)
                    .UserName("user"), appClient);

            provider.Dispatch(1000000);

            long itemHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), itemHandle);

            provider.Dispatch(1000000);

            for (int i = 0; i < 60; i++)
            {
                if (appClient.IsConnectionUp())
                {
                    if (sendRefreshMsg)
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 14700 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("TRI.N")
                                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                                .Payload(fieldList.Complete()).Complete(true), itemHandle);

                        sendRefreshMsg = false;
                    }
                    else
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new UpdateMsg().ServiceName("TEST_NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), itemHandle);
                    }
                }
                else
                {
                    sendRefreshMsg = true;
                }
                provider.Dispatch(1000000);
            }
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            provider?.Uninitialize();
        }
    }
}
