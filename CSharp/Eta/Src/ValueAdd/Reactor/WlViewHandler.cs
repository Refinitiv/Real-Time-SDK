/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using Array = LSEG.Eta.Codec.Array;
using Buffer = LSEG.Eta.Codec.Buffer;

using WlViewFieldList = System.Collections.Generic.List<LSEG.Eta.ValueAdd.Reactor.WlViewElement<int>>;
using WlViewNameList = System.Collections.Generic.List<LSEG.Eta.ValueAdd.Reactor.WlViewElement<string>>;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal sealed class WlViewHandler
    {
        ElementList m_ElementList = new ();
        ElementEntry m_ElementEntry = new();
        Array m_ViewArray = new();
        ArrayEntry m_ArrayEntry = new();
        Int m_TempInt = new();
        UInt m_TempUInt = new();
        Buffer m_ElementNameBuf = new ();
        //bool m_resorted;

        List<int> fidsToRemove = new ();

        public Watchlist Watchlist { get; set; }

        public WlViewHandler(Watchlist watchlist)
        {
            Watchlist = watchlist;
        }

        public WlView? CreateView(object fieldList, int viewType, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            WlView? wlView = null;

            if (fieldList is null)
            {
                Reactor.PopulateErrorInfo(out errorInfo,
                           ReactorReturnCode.FAILURE, "ViewHandder.CreateView", "View's field list is null");
                return null;
            }

            switch(viewType)
            {
                case ViewTypes.FIELD_ID_LIST:
                    {
                        List<int> fieldIdList = (List<int>)fieldList;
                        wlView = new WlView();
                        wlView.ViewHandler = this;
                        wlView.ElemCount = 0;
                        wlView.ViewType = viewType;
                        List<int> tempFieldList;
                        tempFieldList = new List<int>(fieldIdList.Count);

                        wlView.FieldList = tempFieldList;

                        CreateViewList<int>(wlView, fieldIdList, tempFieldList);

                        return wlView;
                    }
                case ViewTypes.ELEMENT_NAME_LIST:
                    {
                        List<string> elementNameList = (List<string>)fieldList;
                        wlView = new WlView();
                        wlView.ViewHandler = this;
                        wlView.ElemCount = 0;
                        wlView.ViewType = viewType;
                        List<string> tempFieldNameList = new(elementNameList.Count);

                        wlView.FieldList = tempFieldNameList;

                        CreateViewList<string>(wlView, elementNameList, tempFieldNameList);

                        return wlView;
                    }
                default:
                    {
                        Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE, "ViewHandder.CreateView",
                        "Invalid ViewType  <" + viewType + ">");
                        break;
                    }
            }

            return wlView;
        }

        public CodecReturnCode EncodeViewRequest(EncodeIterator encodeIter, WlAggregateView aggregateView)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();

            CodecReturnCode codecRet = m_ElementList.EncodeInit(encodeIter, null, 0);

            if(codecRet < CodecReturnCode.SUCCESS)
            {
                return codecRet;
            }

            switch(aggregateView.ViewType)
            {
                case ViewTypes.FIELD_ID_LIST:
                    {
                        m_ElementEntry.Clear();
                        m_ElementEntry.Name = ElementNames.VIEW_TYPE;
                        m_ElementEntry.DataType = DataTypes.UINT;
                        m_TempUInt.Value(ViewTypes.FIELD_ID_LIST);
                        if((codecRet = m_ElementEntry.Encode(encodeIter, m_TempUInt)) < CodecReturnCode.SUCCESS)
                        {
                            return codecRet;
                        }

                        m_ElementEntry.Clear();
                        m_ElementEntry.Name = ElementNames.VIEW_DATA;
                        m_ElementEntry.DataType = DataTypes.ARRAY;
                        if((codecRet = m_ElementEntry.EncodeInit(encodeIter,0)) < CodecReturnCode.SUCCESS)
                        {
                            return codecRet;
                        }

                        m_ViewArray.Clear();
                        m_ViewArray.PrimitiveType = DataTypes.INT;
                        m_ViewArray.ItemLength = 2;

                        if((codecRet = m_ViewArray.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
                        {
                            return codecRet;
                        }

                        WlViewFieldList fieldList = (WlViewFieldList)aggregateView.ElementList!;
                        for(int i = 0; i < aggregateView.ElementCount; i++)
                        {
                            if (fieldList[i].Count == 0)
                                continue;

                            m_ArrayEntry.Clear();
                            m_TempInt.Value(fieldList[i].Value);
                            if((codecRet = m_ArrayEntry.Encode(encodeIter, m_TempInt)) < CodecReturnCode.SUCCESS)
                            {
                                return codecRet;
                            }
                        }

                        break;
                    }
                case ViewTypes.ELEMENT_NAME_LIST:
                    {
                        m_ElementEntry.Clear();
                        m_ElementEntry.Name = ElementNames.VIEW_TYPE;
                        m_ElementEntry.DataType = DataTypes.UINT;
                        m_TempUInt.Value(ViewTypes.ELEMENT_NAME_LIST);
                        if ((codecRet = m_ElementEntry.Encode(encodeIter, m_TempUInt)) < CodecReturnCode.SUCCESS)
                        {
                            return codecRet;
                        }

                        m_ElementEntry.Clear();
                        m_ElementEntry.Name = ElementNames.VIEW_DATA;
                        m_ElementEntry.DataType = DataTypes.ARRAY;
                        if ((codecRet = m_ElementEntry.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
                        {
                            return codecRet;
                        }

                        m_ViewArray.Clear();
                        m_ViewArray.PrimitiveType = DataTypes.ASCII_STRING;

                        if ((codecRet = m_ViewArray.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
                        {
                            return codecRet;
                        }

                        WlViewNameList fieldList = (WlViewNameList)aggregateView.ElementList!;
                        for (int i = 0; i < aggregateView.ElementCount; i++)
                        {
                            if (fieldList[i].Count == 0)
                                continue;

                            m_ArrayEntry.Clear();
                            m_ElementNameBuf.Data(fieldList[i].Value);
                            if ((codecRet = m_ArrayEntry.Encode(encodeIter, m_ElementNameBuf)) < CodecReturnCode.SUCCESS)
                            {
                                return codecRet;
                            }
                        }

                        break;
                    }

                default:
                    {
                        return CodecReturnCode.FAILURE;
                    }
            }

            if ((codecRet = m_ViewArray.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                return codecRet;
            }

            if ((codecRet = m_ElementEntry.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                return codecRet;
            }

            if ((codecRet = m_ElementList.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                return codecRet;
            }

            return CodecReturnCode.SUCCESS;
        }

        private void CreateViewList<T>(WlView wlView, List<T> userReqFidList, List<T> wlViewList)
        {
            wlView.ViewHandler = this;
            wlView.ElemCount = 0;

            foreach (T fieldId in userReqFidList)
            {
                wlViewList.Add(fieldId);
                ++wlView.ElemCount;
            }

            if (wlView.ElemCount > 0)
            {
                wlViewList.Sort();

                // Removes duplicate items
                int startingCount = wlView.ElemCount;
                int newCount = startingCount;
                int nextValidPos = 1;
                for (int i = 1; i < startingCount; i++)
                {
                    if (wlViewList[i]!.Equals(wlViewList[nextValidPos - 1]))
                    {
                        --newCount;
                    }
                    else
                    {
                        if (i > nextValidPos)
                        {
                            wlViewList[nextValidPos] = wlViewList[i];
                        }
                        nextValidPos++;
                    }
                }
                wlView.ElemCount = newCount;
            }
            else
            {
                wlView.ElemCount = 0;
            }
        }
    }
}
