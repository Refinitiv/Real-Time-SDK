/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;

using static LSEG.Eta.Example.Common.MarketPriceRequestFlags;

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

internal class ItemRequest
{
    private readonly List<string> m_ItemNames = new();
    private readonly Qos m_Qos = new();
    private readonly Qos m_WorstQos = new();
    private int m_Identifier;
    private List<int> m_ViewFieldList = new();
    private MarketPriceRequestFlags m_Flags;
    private int m_RequestFlags;

    /// <summary>
    /// The Constant VIEW_TYPE.
    /// </summary>
    public static readonly Buffer VIEW_TYPE = new();

    /// <summary>
    /// ViewData
    /// </summary>
    public static readonly Buffer VIEW_DATA = new();

    private readonly ElementList m_ElementList = new();
    private readonly ElementList m_BehaviorList = new();
    private readonly ElementEntry m_ElementEntry = new();
    private readonly ElementEntry m_DataStreamEntry = new();
    private readonly Array m_Array = new();
    private readonly ArrayEntry m_ArrayEntry = new();
    private readonly Buffer m_ItemNameBuf = new();

    private readonly Int m_TempInt = new();
    private readonly UInt m_TempUInt = new();
    private readonly Buffer m_ElementNameBuf = new();
    private readonly Array m_ViewArray = new();

    public int DomainType { get; set; }
    public int StreamId { get; set; }
    public bool IsSymbolListData { get; set; }
    public Msg RequestMsg { get; set; } = new();

    /// <summary>
    /// Instantiates a new item request.
    /// </summary>
    public ItemRequest() : this((int)Eta.Rdm.DomainType.MARKET_PRICE)
    {
        IsSymbolListData = false;
    }

    /// <summary>
    /// Instantiates a new item request.
    /// </summary>
    /// <param name="domainType">the domain type</param>
    public ItemRequest(int domainType)
    {
        m_Flags = 0;
        m_Identifier = -1;
        DomainType = domainType;
    }

    /// <summary>
    /// Clears the current contents of this object and prepares it for re-use.
    /// </summary>
    public void Clear()
    {
        m_Flags = 0;
        m_Qos.Clear();
        m_WorstQos.Clear();
        m_ItemNames.Clear();
        m_Identifier = -1;
        IsSymbolListData = false;
        if (m_ViewFieldList != null) m_ViewFieldList.Clear();
        if (ViewElementNameList != null) ViewElementNameList.Clear();
    }

    /// <summary>
    /// Checks the presence of private stream flag.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasPrivateStream
    {
        get => (m_Flags & PRIVATE_STREAM) != 0;
        set
        {
            if (value)
                m_Flags |= PRIVATE_STREAM;
            else
                m_Flags &= ~PRIVATE_STREAM;
        }
    }

    /// <summary>
    /// Checks the presence of streaming.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasStreaming
    {
        get => (m_Flags & STREAMING) != 0;
        set
        {
            if (value)
                m_Flags |= STREAMING;
            else
                m_Flags &= ~STREAMING;
        }
    }

    /// <summary>
    /// Checks the presence of Qos.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasQos
    {
        get => (m_Flags & HAS_QOS) != 0;
        set
        {
            if (value)
                m_Flags |= HAS_QOS;
            else
                m_Flags &= ~HAS_QOS;
        }
    }

    /// <summary>
    /// Checks the presence of WorstQos.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasWorstQos
    {
        get => (m_Flags & HAS_WORST_QOS) != 0;
        set
        {
            if (value)
                m_Flags |= HAS_WORST_QOS;
            else
                m_Flags &= ~HAS_WORST_QOS;
        }
    }

    /// <summary>
    /// Checks the presence of Priority flag.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasPriority
    {
        get => (m_Flags & HAS_PRIORITY) != 0;
        set
        {
            if (value)
                m_Flags |= HAS_PRIORITY;
            else
                m_Flags &= ~HAS_PRIORITY;
        }
    }

    /// <summary>
    /// Checks the presence of View flag.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasView
    {
        get => (m_Flags & HAS_VIEW) != 0;
        set
        {
            if (value)
                m_Flags |= HAS_VIEW;
            else
                m_Flags &= ~HAS_VIEW;
        }
    }

    /// <summary>
    /// Checks the presence of Pause.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasPause
    {
        get => (m_RequestFlags & RequestMsgFlags.PAUSE) != 0;
        set
        {
            if (value)
                m_RequestFlags |= RequestMsgFlags.PAUSE;
            else
                m_RequestFlags &= ~RequestMsgFlags.PAUSE;
        }
    }

    /// <summary>
    /// Sets the view fields.
    /// </summary>
    /// <param name="viewList">list of view fields</param>
    public List<int> ViewFields
    {
        set{
            m_ViewFieldList = value;
        }
    }
    
    /// <summary>
    /// Checks the presence of service id flag.
    /// </summary>
    /// <returns>true - if exists; false if does not exist.</returns>
    public bool HasServiceId
    {
        get => (m_Flags & HAS_SERVICE_ID) != 0;
        set
        {
            if (value)
                m_Flags |= HAS_SERVICE_ID;
            else
                m_Flags &= ~HAS_SERVICE_ID;
        }
    }

    /// <summary>
    /// Checks the presence of an identifier.
    /// </summary>
    /// <returns>true - if exists; false if does not exist;</returns>
    public bool HasIdentifier => m_Identifier >= 0;

    /// <summary>
    /// Element names.
    /// </summary>
    public List<string> ViewElementNameList { get; set; } = new();

    /// <summary>
    /// Encodes the stream.
    /// </summary>
    public void Encode()
    {
        RequestMsg.Clear();

        /* set-up message */
        RequestMsg.MsgClass = MsgClasses.REQUEST;
        RequestMsg.StreamId = StreamId;
        RequestMsg.DomainType = (int)DomainType;

        RequestMsg.ContainerType = DataTypes.NO_DATA;

        if (HasQos)
        {
            RequestMsg.ApplyHasQos();
            RequestMsg.Qos.IsDynamic = m_Qos.IsDynamic;
            RequestMsg.Qos.Rate(m_Qos.Rate());
            RequestMsg.Qos.Timeliness(m_Qos.Timeliness());
            RequestMsg.Qos.RateInfo(m_Qos.RateInfo());
            RequestMsg.Qos.TimeInfo(m_Qos.TimeInfo());
        }

        if (HasWorstQos)
        {
            RequestMsg.ApplyHasWorstQos();
            RequestMsg.WorstQos.IsDynamic = m_WorstQos.IsDynamic;
            RequestMsg.WorstQos.Rate(m_WorstQos.Rate());
            RequestMsg.WorstQos.Timeliness(m_WorstQos.Timeliness());
            RequestMsg.WorstQos.RateInfo(m_WorstQos.RateInfo());
            RequestMsg.WorstQos.TimeInfo(m_WorstQos.TimeInfo());
        }

        if (HasPriority)
        {
            RequestMsg.ApplyHasPriority();
        }

        if (HasStreaming)
        {
            RequestMsg.ApplyStreaming();
        }

        if (HasPause)
        {
            RequestMsg.ApplyPause();
        }

        bool isBatchRequest = m_ItemNames.Count > 1;
        ApplyFeatureFlags(isBatchRequest);

        /* specify msgKey members */
        RequestMsg.MsgKey.ApplyHasNameType();
        RequestMsg.MsgKey.NameType = InstrumentNameTypes.RIC;

        /* If user set Identifier */
        if (HasIdentifier)
        {
            RequestMsg.MsgKey.ApplyHasIdentifier();
            RequestMsg.MsgKey.Identifier = m_Identifier;
        }

        if (!isBatchRequest && !HasIdentifier)
        {
            RequestMsg.MsgKey.ApplyHasName();
            RequestMsg.MsgKey.Name.Data(m_ItemNames[0]);
        }
    }

    /// <summary>
    /// Encodes the item request.
    /// </summary>
    /// <param name="encodeIter">The Encode Iterator</param>
    /// <returns>{@link CodecReturnCode#SUCCESS} if encoding succeeds or failure if encoding fails.</returns>
    /// <remarks>This method is only used within the Market By Price Handler and each handler has its own implementation, although much is similar.</remarks>
    public CodecReturnCode Encode(EncodeIterator encodeIter)
    {
        bool isBatchRequest = m_ItemNames.Count > 1;
        CodecReturnCode ret = CodecReturnCode.SUCCESS;

        Encode();

        /* encode request message payload */
        if (HasView || IsSymbolListData || isBatchRequest)
        {
            ret = EncodeRequestPayload(isBatchRequest, encodeIter);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
        }

        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode EncodeRequestPayload(bool isBatchRequest, EncodeIterator encodeIter)
    {
        m_ElementList.Clear();
        m_ElementList.ApplyHasStandardData();

        CodecReturnCode ret = m_ElementList.EncodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        if (isBatchRequest
                && (EncodeBatchRequest(encodeIter) < CodecReturnCode.SUCCESS))
        {
            return CodecReturnCode.FAILURE;
        }

        if (HasView)
        {
            if (m_ViewFieldList != null && m_ViewFieldList.Count > 0)
            {
                if (EncodeViewRequest(encodeIter, ViewTypes.FIELD_ID_LIST) < CodecReturnCode.SUCCESS)
                {
                    return CodecReturnCode.FAILURE;
                }
            }
            else if (ViewElementNameList != null && ViewElementNameList.Count > 0)
            {
                if (EncodeViewRequest(encodeIter, ViewTypes.ELEMENT_NAME_LIST) < CodecReturnCode.SUCCESS)
                {
                    return CodecReturnCode.FAILURE;
                }
            }
        }

        if (IsSymbolListData &&
                (EncodeSymbolListData(encodeIter) < CodecReturnCode.SUCCESS))
        {
            return CodecReturnCode.FAILURE;
        }

        ret = m_ElementList.EncodeComplete(encodeIter, true);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }
        return CodecReturnCode.SUCCESS;
    }

    private void ApplyFeatureFlags(bool isBatchRequest)
    {
        if (HasPrivateStream)
        {
            RequestMsg.ApplyPrivateStream();
        }

        if (HasView || IsSymbolListData || isBatchRequest)
        {
            RequestMsg.ContainerType = DataTypes.ELEMENT_LIST;
            if (HasView)
            {
                RequestMsg.ApplyHasView();
            }
            if (isBatchRequest)
            {
                RequestMsg.ApplyHasBatch();
            }
        }
    }

    private CodecReturnCode EncodeBatchRequest(EncodeIterator encodeIter)
    {
        /*
         * For Batch requests, the message has a payload of an element list that
         * contains an array of the requested items
         */

        m_ElementEntry.Clear();
        m_Array.Clear();
        m_ItemNameBuf.Clear();

        m_ElementEntry.Name = ElementNames.BATCH_ITEM_LIST;
        m_ElementEntry.DataType = DataTypes.ARRAY;
        CodecReturnCode ret = m_ElementEntry.EncodeInit(encodeIter, 0);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        /* Encode the array of requested item names */
        m_Array.PrimitiveType = DataTypes.ASCII_STRING;
        m_Array.ItemLength = 0;

        ret = m_Array.EncodeInit(encodeIter);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        foreach (var itemName in m_ItemNames)
        {
            m_ArrayEntry.Clear();
            m_ItemNameBuf.Data(itemName);
            ret = m_ArrayEntry.Encode(encodeIter, m_ItemNameBuf);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }
        }

        ret = m_Array.EncodeComplete(encodeIter, true);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        ret = m_ElementEntry.EncodeComplete(encodeIter, true);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Encodes the View Element Entry. This entry contains an array of FIDs that the consumer wishes to receive from the provider.
    /// </summary>
    /// <param name="encodeIter">Iterator.</param>
    /// <param name="viewType">View type.</param>
    /// <returns>Codec return code.</returns>
    /// <remarks>This method is only used within the Market Price Handler.</remarks>
    private CodecReturnCode EncodeViewRequest(EncodeIterator encodeIter, int viewType)
    {
        m_ElementEntry.Clear();
        m_TempUInt.Clear();
        m_ElementEntry.Name = ElementNames.VIEW_TYPE;
        m_ElementEntry.DataType = DataTypes.UINT;
        if (viewType == ViewTypes.FIELD_ID_LIST)
            m_TempUInt.Value(ViewTypes.FIELD_ID_LIST);
        else
            m_TempUInt.Value(ViewTypes.ELEMENT_NAME_LIST);
        CodecReturnCode ret = m_ElementEntry.Encode(encodeIter, m_TempUInt);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        m_ElementEntry.Clear();
        m_ElementEntry.Name = ElementNames.VIEW_DATA;
        m_ElementEntry.DataType = DataTypes.ARRAY;
        if ((ret = m_ElementEntry.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        if (viewType == ViewTypes.FIELD_ID_LIST)
        {
            m_ViewArray.PrimitiveType = DataTypes.INT;
            m_ViewArray.ItemLength = 2;
        }
        else
            m_ViewArray.PrimitiveType = DataTypes.ASCII_STRING;

        if ((ret = m_ViewArray.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        if (viewType == ViewTypes.FIELD_ID_LIST)
        {
            foreach (var viewField in m_ViewFieldList)
            {
                m_ArrayEntry.Clear();
                m_TempInt.Value(viewField);
                ret = m_ArrayEntry.Encode(encodeIter, m_TempInt);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
        }
        else
        {
            foreach (var elementName in ViewElementNameList)
            {
                m_ArrayEntry.Clear();
                m_ElementNameBuf.Data(elementName);
                ret = m_ArrayEntry.Encode(encodeIter, m_ElementNameBuf);

                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
        }
        ret = m_ViewArray.EncodeComplete(encodeIter, true);

        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        ret = m_ElementEntry.EncodeComplete(encodeIter, true);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Encode symbol list data.
    /// </summary>
    /// <param name="encodeIter">The encode iter.</param>
    /// <returns>Returns the SUCCESS code if successful.</returns>
    public CodecReturnCode EncodeSymbolListData(EncodeIterator encodeIter)
    {
        m_ElementEntry.Clear();
        m_ElementEntry.Name = SymbolList.ElementNames.SYMBOL_LIST_BEHAVIORS;
        m_ElementEntry.DataType = DataTypes.ELEMENT_LIST;

        CodecReturnCode ret = m_ElementEntry.EncodeInit(encodeIter, 0);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        m_BehaviorList.Clear();
        m_BehaviorList.ApplyHasStandardData();
        ret = m_BehaviorList.EncodeInit(encodeIter, null, 0);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        m_DataStreamEntry.Clear();
        m_DataStreamEntry.Name = SymbolList.ElementNames.SYMBOL_LIST_DATA_STREAMS;
        m_DataStreamEntry.DataType = DataTypes.UINT;

        if (HasStreaming)
            m_TempUInt.Value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
        else
            m_TempUInt.Value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS);
            //APIQA
            System.Console.WriteLine("\n------ APIQA: Encoding SLD request with SYMBOL_LIST_DATA_SNAPSHOTS:  " + SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS + "\n\n");
            m_TempUInt.Value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS);
            //END APIQA
        if ((ret = m_DataStreamEntry.Encode(encodeIter, m_TempUInt)) < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        ret = m_BehaviorList.EncodeComplete(encodeIter, true);

        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        ret = m_ElementEntry.EncodeComplete(encodeIter, true);
        if (ret < CodecReturnCode.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Adds the item.
    /// </summary>
    /// <param name="itemName">Item name.</param>
    public void AddItem(string itemName)
    {
        m_ItemNames.Add(itemName);
    }
}
