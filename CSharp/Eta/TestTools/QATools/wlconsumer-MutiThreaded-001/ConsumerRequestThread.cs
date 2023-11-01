
//APIQA
namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using static LSEG.Eta.Rdm.Dictionary;
using static Rdm.LoginMsgType;

/**
 * <p>
 * This interface is responsible for sending item requests to provider.
 * </p>
 */
class ConsumerRequestThread
{
    private Eta.Codec.Buffer? m_Payload;
    private ItemRequest? m_ItemReuest = null;
    private ChannelInfo? m_ChnlInfo;
    private ConsumerCallbackThread? m_ConsumerCallbackThread;

    private readonly ItemRequest m_ItemRequest = new();
    private readonly EncodeIterator m_EncIter = new();
    private readonly List<int> m_ViewFieldList = new();
    private readonly ReactorSubmitOptions m_SubmitOptions = new();

    public List<WatchlistConsumerConfig.ItemInfo>? m_ItemInfoList;
    public ConsumerRequestThread(ChannelInfo channelInfo, ConsumerCallbackThread consumer)
    {
        m_ItemReuest = CreateItemRequest();
        m_ChnlInfo = channelInfo;
        m_ConsumerCallbackThread = consumer;
        m_ItemInfoList = consumer!.m_WatchlistConsumerConfig!.ItemList!;
    }

    protected ItemRequest CreateItemRequest()
    {
        return new ItemRequest();
    }
    
    /* Runs the Value Add consumer application. */
    public void Run()		
	{		
		while (!m_ConsumerCallbackThread!.m_ShutDown)
		{

            //send  item requests
            if (m_ConsumerCallbackThread.IsRequestedServiceUp(m_ChnlInfo) && m_ConsumerCallbackThread.IsDictionaryReady()
                                   && !m_ConsumerCallbackThread.m_ItemsRequested)
            { 
                foreach (WatchlistConsumerConfig.ItemInfo item in m_ItemInfoList!)
    		    {
                    try
                    {
                        Thread.Sleep(10);
                        RequestItems(item);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e.ToString());
                    }
    		    }
                m_ConsumerCallbackThread.m_ItemsRequested = true;
            }
	        try
			{
				Thread.Sleep(10);
			} catch (Exception e)
			{
                Console.WriteLine(e.ToString());
            }
		}
		
		 // Handle run-time
        if (m_ConsumerCallbackThread.m_ShutDown)
        {
            Console.WriteLine("ConsumerRequestThread  " + this.GetHashCode() + " closing now...");
        }
	}

    private CodecReturnCode RequestItems(WatchlistConsumerConfig.ItemInfo? itemInfo)
    {
        CodecReturnCode ret = CodecReturnCode.SUCCESS;

        m_ItemRequest.Clear();

        if (m_ConsumerCallbackThread!.m_WatchlistConsumerConfig!.EnableSnapshot)
            m_ItemRequest.HasStreaming = true;

        m_ItemRequest.AddItem(itemInfo!.Name!);

        var domainType = itemInfo.Domain;

        m_ItemRequest.DomainType = domainType;
        m_ItemRequest.StreamId = itemInfo.StreamId;

        if (itemInfo.IsPrivateStream)
            m_ItemRequest.HasPrivateStream = true;

        if (domainType == (int)DomainType.SYMBOL_LIST && itemInfo.SymbolListData)
        {
            m_ItemRequest.IsSymbolListData = true;
            m_Payload = new();
            m_Payload.Data(new ByteBuffer(1024));

            m_EncIter.Clear();
            m_EncIter.SetBufferAndRWFVersion(m_Payload, m_ChnlInfo!.ReactorChannel!.MajorVersion, m_ChnlInfo!.ReactorChannel!.MinorVersion);

            ret = m_ItemRequest.Encode(m_EncIter);

            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("RequestItem.Encode() failed");
                return ret;
            }

            m_ItemRequest.RequestMsg.EncodedDataBody = m_Payload;
        }
        else if (domainType == (int)DomainType.MARKET_PRICE && m_ConsumerCallbackThread!.m_WatchlistConsumerConfig!.EnableView)
        {
            m_Payload = new();// move it to the top to share
            m_Payload.Data(new ByteBuffer(1024));

            m_EncIter.Clear();
            m_EncIter.SetBufferAndRWFVersion(m_Payload, m_ChnlInfo!.ReactorChannel!.MajorVersion, m_ChnlInfo!.ReactorChannel!.MinorVersion);

            m_ItemRequest.HasView = true;
            m_ViewFieldList.Add(22);
            m_ViewFieldList.Add(25);
            m_ViewFieldList.Add(30);
            m_ViewFieldList.Add(31);
            m_ViewFieldList.Add(1025);
            m_ItemRequest.ViewFields = m_ViewFieldList;
            ret = m_ItemRequest.Encode(m_EncIter);

            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("RequestItem.Encode() failed");
                return ret;
            }
            m_ItemRequest.RequestMsg.EncodedDataBody = m_Payload;
        }
        else
        {
            m_ItemRequest.Encode();
        }

        m_SubmitOptions.Clear();
        if (m_ConsumerCallbackThread!.m_WatchlistConsumerConfig!.ServiceName != null)
        {
            m_SubmitOptions.ServiceName = m_ConsumerCallbackThread!.m_WatchlistConsumerConfig.ServiceName;
        }

        var subRet = m_ChnlInfo!.ReactorChannel!.Submit(m_ItemRequest.RequestMsg, m_SubmitOptions, out var errorInfo);
        if (subRet < ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("\nReactorChannel.Submit() failed: " + ret + "(" + errorInfo?.Error.Text + ")\n");
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }

        return CodecReturnCode.SUCCESS;
    }
}
