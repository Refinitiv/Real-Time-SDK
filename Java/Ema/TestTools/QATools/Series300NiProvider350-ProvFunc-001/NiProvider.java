///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.niprovider.series300.example350__Dictionary__Streaming;

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
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.rdm.DataDictionary;
import com.thomsonreuters.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    public DataDictionary dataDictionary = EmaFactory.createDataDictionary();
    public boolean fldDictComplete = false;
    public boolean enumTypeComplete = false;
    private boolean dumpDictionary = false;
    // APIQA
    public int filter = EmaRdm.DICTIONARY_NORMAL;

    AppClient(String[] args)
    {
        int idx = 0;

        while (idx < args.length)
        {
            if (args[idx].compareTo("-dumpDictionary") == 0)
            {
                dumpDictionary = true;
            }
            // APIQA
            else if (args[idx].compareTo("-filter") == 0)
            {
                if (++idx == args.length)
                    break;

                if (args[idx].compareToIgnoreCase("INFO") == 0)
                {
                    filter = EmaRdm.DICTIONARY_INFO;
                }
                if (args[idx].compareToIgnoreCase("MINIMAL") == 0)
                {
                    filter = EmaRdm.DICTIONARY_MINIMAL;
                }
                if (args[idx].compareToIgnoreCase("NORMAL") == 0)
                {
                    filter = EmaRdm.DICTIONARY_NORMAL;
                }
                if (args[idx].compareToIgnoreCase("VERBOSE") == 0)
                {
                    filter = EmaRdm.DICTIONARY_VERBOSE;
                }
            }
            // END APIQA
            ++idx;
        }
    }

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent)
    {
        System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        System.out.println("Item State: " + refreshMsg.state());

        decode(refreshMsg, refreshMsg.complete());
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent)
    {
        System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.hasState())
            System.out.println("Item State: " + statusMsg.state());

        System.out.println();
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onAllMsg(Msg msg, OmmProviderEvent providerEvent)
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

    void decode(Msg msg, boolean complete)
    {
        switch (msg.payload().dataType())
        {
            case DataTypes.SERIES:

                if (msg.name().equals("RWFFld"))
                {
                    dataDictionary.decodeFieldDictionary(msg.payload().series(), filter);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if (msg.name().equals("RWFEnum"))
                {
                    dataDictionary.decodeEnumTypeDictionary(msg.payload().series(), filter);

                    if (complete)
                    {
                        enumTypeComplete = true;
                    }
                }

                if (fldDictComplete && enumTypeComplete)
                {
                    System.out.println();
                    System.out.println("Dictionary download complete");
                    System.out.println("Dictionary Id : " + dataDictionary.dictionaryId());
                    System.out.println("Dictionary field version : " + dataDictionary.fieldVersion());
                    System.out.println("Number of dictionary entries : " + dataDictionary.entries().size());

                    if (dumpDictionary)
                        System.out.println(dataDictionary);
                }

                break;
            default:
                break;
        }
    }
}

public class NiProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;
        try
        {
            AppClient appClient = new AppClient(args);
            OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();

            provider = EmaFactory.createOmmProvider(config.username("user"));

            long rwfFld = provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(appClient.filter).serviceName("NI_PUB").domainType(EmaRdm.MMT_DICTIONARY).interestAfterRefresh(false),
                                                  appClient);

            long rwfEnum = provider
                    .registerClient(EmaFactory.createReqMsg().name("RWFEnum").interestAfterRefresh(false).filter(appClient.filter).serviceName("NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), appClient);

            long triHandle = 6;

            FieldList fieldList = EmaFactory.createFieldList();

            fieldList.add(EmaFactory.createFieldEntry().real(22, 4100, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add(EmaFactory.createFieldEntry().real(25, 4200, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add(EmaFactory.createFieldEntry().real(30, 20, OmmReal.MagnitudeType.EXPONENT_0));
            fieldList.add(EmaFactory.createFieldEntry().real(31, 40, OmmReal.MagnitudeType.EXPONENT_0));

            provider.submit(EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("TRI.N")
                    .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed").payload(fieldList).complete(true), triHandle);

            Thread.sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                fieldList.clear();
                fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));

                provider.submit(EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("TRI.N").payload(fieldList), triHandle);
                Thread.sleep(1000);
                // APIQA
                if (i == 6)
                {
                    System.out.println("Unregister Dictionary handles");
                    provider.unregister(rwfFld);
                    provider.unregister(rwfEnum);
                }
                // END APIQA

            }
        }
        catch (InterruptedException | OmmException excp)
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
