//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmNiProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmProviderClient;
import com.thomsonreuters.ema.access.OmmProviderEvent;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    boolean _connectionUp;

    boolean isConnectionUp()
    {
        return _connectionUp;
    }

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
    {
        System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());

        System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        System.out.println("Item State: " + refreshMsg.state());

        if (refreshMsg.state().streamState() == OmmState.StreamState.OPEN)
        {
            if (refreshMsg.state().dataState() == OmmState.DataState.OK)
                _connectionUp = true;
            else
                _connectionUp = false;
        }
        else
            _connectionUp = false;
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event)
    {
        System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

        System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.hasState())
        {
            System.out.println("Item State: " + statusMsg.state());
            if (statusMsg.state().streamState() == OmmState.StreamState.OPEN)
            {
                if (statusMsg.state().dataState() == OmmState.DataState.OK)
                    _connectionUp = true;
                else
                {
                    _connectionUp = false;
                }
            }
            else
                _connectionUp = false;
        }
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event)
    {
    }

    public void onAllMsg(Msg msg, OmmProviderEvent event)
    {
    }

    public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent)
    {
    }
}

public class NiProvider
{
    private static String authenticationToken = "";
    private static String appId = "256";

    public static void printHelp()
    {

        System.out.println("\nOptions:\n" + "  -?                            Shows this usage\n\n" + "  -at <token>           Authentication token to use in login request [default = \"\"]\n"
                + "  -aid <applicationId>        ApplicationId set as login Attribute [default = 256]\n" + "\n");
    }

    public static void printInvalidOption()
    {
        System.out.println("Detected a missing argument. Please verify command line options [-?]");
    }

    public static boolean init(String[] argv)
    {
        int count = argv.length;
        int idx = 0;

        while (idx < count)
        {
            if (0 == argv[idx].compareTo("-?"))
            {
                printHelp();
                return false;
            }
            else if (0 == argv[idx].compareTo("-aid"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                appId = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].compareTo("-at"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                authenticationToken = argv[idx];
                ++idx;
            }
            else
            {
                System.out.println("Unrecognized option. Please see command line help. [-?]");
                return false;
            }
        }

        return true;
    }

    private static void printActiveConfig()
    {
        System.out.println("Following options are selected:");

        System.out.println("appId = " + appId);
        System.out.println("Authentication Token = " + authenticationToken);
    }

    public static void main(String[] args)
    {
        if (!init(args))
            return;
        AppClient appClient = new AppClient();
        boolean sendRefreshMsg = false;
        OmmProvider provider = null;
        printActiveConfig();
        try
        {
            OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();

            ElementList elementList = EmaFactory.createElementList();
            elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, appId));
            provider = EmaFactory.createOmmProvider(config.operationModel(OmmNiProviderConfig.OperationModel.USER_DISPATCH).username(authenticationToken).applicationId(appId));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long loginHandle = provider.registerClient(reqMsg.domainType(EmaRdm.MMT_LOGIN), appClient);

            provider.dispatch(1000000);

            long itemHandle = 6;

            FieldList fieldList = EmaFactory.createFieldList();

            fieldList.add(EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add(EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
            fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

            provider.submit(EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
                                    .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed").payload(fieldList).complete(true), itemHandle);

            provider.dispatch(1000000);

            for (int i = 0; i < 60; i++)
            {
                if (appClient.isConnectionUp())
                {
                    if (sendRefreshMsg)
                    {
                        fieldList.clear();
                        fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                        fieldList.add(EmaFactory.createFieldEntry().real(25, 14700 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                        fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
                        fieldList.add(EmaFactory.createFieldEntry().real(31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));

                        provider.submit(EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
                                                .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed").payload(fieldList).complete(true),
                                        itemHandle);

                        sendRefreshMsg = false;
                    }
                    else
                    {
                        fieldList.clear();
                        fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                        fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));

                        provider.submit(EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("TRI.N").payload(fieldList), itemHandle);
                    }
                }
                else
                {
                    sendRefreshMsg = true;
                }
                provider.dispatch(1000000);
            }
        }
        catch (OmmException excp)
        {
            System.out.println(excp.getMessage());
        }
        finally
        {
            if (provider != null)
                provider.uninitialize();
        }
    }
}
//END APIQA
