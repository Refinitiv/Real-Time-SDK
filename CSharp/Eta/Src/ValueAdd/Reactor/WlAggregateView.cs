/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Rdm;
using System.Diagnostics;

using WlViewFieldList = System.Collections.Generic.List<LSEG.Eta.ValueAdd.Reactor.WlViewElement<int>>;
using WlViewNameList = System.Collections.Generic.List<LSEG.Eta.ValueAdd.Reactor.WlViewElement<string>>;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This is used to aggreate view request for an <see cref="WlItemStream"/>.
    /// </summary>
    internal sealed class WlAggregateView
    {
        /// <summary>
        /// The default constructor
        /// </summary>
        public WlAggregateView()
        {
            ViewType = 0;
        }

        /// <summary>
        /// Gets or set a view type from the <see cref="ViewTypes"/> definition.
        /// </summary>
        public int ViewType { get; set; }

        /// <summary>
        /// Gets or sets the view handler
        /// </summary>
        public WlViewHandler? ViewHandler { get; set; }

        /// <summary>
        /// Gets or sets views that have not been added to the overall view.
        /// </summary>
        public LinkedList<WlView> NewViews { get; set; } = new();

        /// <summary>
        /// Gets or sets views that have been added to the overall view but
        /// may need to be undone if sending the new view fails.
        /// </summary>
        public LinkedList<WlView> MergedViews { get; set; } = new();

        /// <summary>
        /// Gets or sets views that have been added to the overall view,
        /// and won't need not be undone.
        /// </summary>
        public LinkedList<WlView> CommittedViews { get; set; } = new();

        /// <summary>
        /// Gets or sets a list of field IDs or names.
        /// </summary>
        public object? ElementList { get; set; }

        /// <summary>
        /// Gets or sets the number of actual element in <see cref="ElementList"/>.
        /// </summary>
        public int ElementCount { get; set; }

        
        public void AddView(WlView wlView)
        {
            Debug.Assert(ViewType == 0 || ViewType == wlView.ViewType);

            if(ViewType == 0)
            {
                ViewType = wlView.ViewType;
            }

            wlView.ViewNode = NewViews.AddLast(wlView);
        }

        public void RemoveView(WlView wlView, out bool viewChangedToZero)
        {
            viewChangedToZero = false;
            if (ReferenceEquals(wlView.ViewNode.List, CommittedViews))
            {
                /* When removing committed views, leave zeroed fields in place so we can detect
                 * the need to send a new view. */
                UnMergedAView(wlView, false, out viewChangedToZero);
            }
            else if (ReferenceEquals(wlView.ViewNode.List, NewViews))
            {
                NewViews.Remove(wlView.ViewNode);
            }
            else if (wlView.ViewNode.List != null)
            {
                Debug.Assert(ReferenceEquals(wlView.ViewNode.List, MergedViews));
                UnMergedAView(wlView, true, out viewChangedToZero);
            }
        }

        public ReactorReturnCode MergeViews(out bool updated)
        {
            updated = false;
            ReactorReturnCode reactorRetCode = ReactorReturnCode.SUCCESS;

            LinkedListNode<WlView>? currentNode = NewViews.First;
            LinkedListNode<WlView>? nextNode;
            while(currentNode != null)
            {
                nextNode = currentNode.Next;
                if ((reactorRetCode = MergeAView(currentNode.Value, out updated)) != ReactorReturnCode.SUCCESS)
                {
                    return reactorRetCode;
                }

                currentNode = nextNode;
            }

            if(updated)
            {
                return ReactorReturnCode.SUCCESS;
            }

            if (ElementList != null)
            {
                /* Check for any fields that were added or updated. If so, updtes the overall view. */
                switch (ViewType)
                {
                    case ViewTypes.FIELD_ID_LIST:
                        {
                            WlViewFieldList aggViewFieldList = (WlViewFieldList)ElementList!;
                            WlViewElement<int> viewElement;

                            for (int i = 0; i < ElementCount; i++)
                            {
                                viewElement = aggViewFieldList[i];
                                if ((viewElement.Count == 0 && viewElement.Committed)
                                    || (viewElement.Count != 0 && !viewElement.Committed))
                                {
                                    updated = true;
                                    break;
                                }
                            }

                            break;
                        }
                    case ViewTypes.ELEMENT_NAME_LIST:
                        {
                            WlViewNameList aggViewFieldList = (WlViewNameList)ElementList!;
                            WlViewElement<string> viewElement;

                            for (int i = 0; i < ElementCount; i++)
                            {
                                viewElement = aggViewFieldList[i];
                                if ((viewElement.Count == 0 && viewElement.Committed)
                                    || (viewElement.Count != 0 && !viewElement.Committed))
                                {
                                    updated = true;
                                    break;
                                }
                            }

                            break;
                        }
                    default:
                        {
                            Debug.Assert(false);
                            break;
                        }

                } // End switch
            }

            return reactorRetCode;
        }

        public ReactorReturnCode MergeAView(WlView wlView, out bool updated)
        {
            Debug.Assert(ReferenceEquals(wlView.ViewNode.List, NewViews));

            updated = false;

            switch(ViewType)
            {
                case ViewTypes.FIELD_ID_LIST:
                    {
                        WlViewFieldList? aggViewFieldList = (WlViewFieldList?)ElementList;

                        if(aggViewFieldList is null)
                        {
                            aggViewFieldList = new (wlView.ElemCount);
                            ElementCount = 0;
                            ElementList = aggViewFieldList;
                        }

                        List<int> viewFieldList = (List<int>)wlView.FieldList!;
                        WlViewElement<int> tempViewElement = new();
                        WlViewIntElementComparer viewElementComparer = new();
                        int aggViewFieldCount = ElementCount;
                        int aggViewFieldCapacity = aggViewFieldList.Count;

                        for (int i = 0; i < wlView.ElemCount; i++)
                        {
                            tempViewElement.Value = viewFieldList[i];
                            int foundIndex = aggViewFieldList.BinarySearch(0, ElementCount, tempViewElement,
                                                viewElementComparer);

                            if(foundIndex >= 0)
                            {
                                /* Field exists; update its count.*/
                                ++aggViewFieldList[foundIndex].Count;
                            }
                            else
                            {
                                if (aggViewFieldCount < aggViewFieldCapacity)
                                {
                                    aggViewFieldList[aggViewFieldCount].Value = viewFieldList[i];
                                    aggViewFieldList[aggViewFieldCount].Count = 1;
                                    aggViewFieldList[aggViewFieldCount].Committed = false;
                                }
                                else
                                {
                                    WlViewElement<int> wlViewElement = new();
                                    wlViewElement.Value = viewFieldList[i];
                                    wlViewElement.Count = 1;
                                    wlViewElement.Committed = false;

                                    /* Add the new field to the end (it will be sorted later)*/
                                    aggViewFieldList.Add(wlViewElement);
                                }

                                updated = true;
                                ++aggViewFieldCount;
                            }
                        }

                        ElementCount = aggViewFieldCount;

                        /* Resort the list if anything added. */
                        aggViewFieldList.Sort(0, ElementCount, viewElementComparer);

                        break;
                    }
                case ViewTypes.ELEMENT_NAME_LIST:
                    {
                        WlViewNameList? aggViewFieldList = (WlViewNameList?)ElementList;

                        if (aggViewFieldList is null)
                        {
                            aggViewFieldList = new(wlView.ElemCount);
                            ElementCount = 0;
                            ElementList = aggViewFieldList;
                        }

                        List<string> viewElementNameList = (List<string>)wlView.FieldList!;
                        WlViewElement<string> tempViewElement = new();
                        WlViewStringElementComparer viewElementComparer = new();
                        int aggViewFieldCount = ElementCount;
                        int aggViewFieldCapacity = aggViewFieldList.Count;

                        for (int i = 0; i < wlView.ElemCount; i++)
                        {
                            tempViewElement.Value = viewElementNameList[i];
                            int foundIndex = aggViewFieldList.BinarySearch(0, ElementCount, tempViewElement,
                                                viewElementComparer);

                            if (foundIndex >= 0)
                            {
                                /* Field exists; update its count.*/
                                ++aggViewFieldList[foundIndex].Count;
                            }
                            else
                            {
                                if (aggViewFieldCount < aggViewFieldCapacity)
                                {
                                    aggViewFieldList[aggViewFieldCount].Value = viewElementNameList[i];
                                    aggViewFieldList[aggViewFieldCount].Count = 1;
                                    aggViewFieldList[aggViewFieldCount].Committed = false;
                                }
                                else
                                {
                                    WlViewElement<string> wlViewElement = new();
                                    wlViewElement.Value = viewElementNameList[i];
                                    wlViewElement.Count = 1;
                                    wlViewElement.Committed = false;

                                    /* Add the new field to the end (it will be sorted later)*/
                                    aggViewFieldList.Add(wlViewElement);
                                }

                                updated = true;
                                ++aggViewFieldCount;
                            }
                        }

                        ElementCount = aggViewFieldCount;

                        /* Resort the list if anything added. */
                        aggViewFieldList.Sort(0, ElementCount, viewElementComparer);

                        break;
                    }
                default:
                    {
                        Debug.Assert(false);
                        break;
                    }
            } // End switch

            /* Moves the viwe from NewViews to Merged Views */
            NewViews.Remove(wlView.ViewNode);
            MergedViews.AddLast(wlView.ViewNode);
  
            return ReactorReturnCode.SUCCESS;
        }

        public bool ContainsView(WlItemRequest wlItemRequest)
        {
            int mergedCount = MergedViews.Count + CommittedViews.Count;

            if (mergedCount == 0)
            {
                return false;
            }

            switch(ViewType)
            {
                case ViewTypes.FIELD_ID_LIST:
                    {
                        WlViewFieldList aggViewFieldIdList = (WlViewFieldList)ElementList!;
                        List<int> viewFieldList = (List<int>)wlItemRequest.ViewFieldList!;
                        WlViewIntElementComparer viewIntComparer = new();
                        WlViewElement<int> wlViewElement = new();

                        if (wlItemRequest.ViewElemCount > ElementCount)
                        {
                            return false;
                        }

                        for(int i = 0; i < wlItemRequest.ViewElemCount; i++)
                        {
                            wlViewElement.Value = viewFieldList[i];
                            int index = aggViewFieldIdList.BinarySearch(0, ElementCount, wlViewElement, viewIntComparer);
                            if (index < 0) return false;
                        }

                        return true;
                    }
                case ViewTypes.ELEMENT_NAME_LIST:
                    {
                        WlViewNameList aggViewElementNameList = (WlViewNameList)ElementList!;
                        List<string> viewElementNameList = (List<string>)wlItemRequest.ViewFieldList!;
                        WlViewStringElementComparer viewStringComparer = new();
                        WlViewElement<string> wlViewElement = new();

                        if (wlItemRequest.ViewElemCount > ElementCount)
                        {
                            return false;
                        }

                        for (int i = 0; i < wlItemRequest.ViewElemCount; i++)
                        {
                            wlViewElement.Value = viewElementNameList[i];
                            int index = aggViewElementNameList.BinarySearch(0, ElementCount, wlViewElement, viewStringComparer);
                            if (index < 0) return false;
                        }

                        return true;
                    }
            }

            return false;
        }

        public bool ContainsNewView()
        {
            bool result;

            foreach(WlView wlView in NewViews)
            {
                result = ContainsView(wlView);

                /* Checks whether the aggreagted view has the new views.*/
                if(result == false)
                    return false;
            }

            return true;
        }

        public bool ContainsView(WlView wlView)
        {
            int mergedCount = MergedViews.Count + CommittedViews.Count;

            if(mergedCount == 0 || (wlView.ElemCount > ElementCount))
            {
                return false;
            }

            switch(ViewType)
            {
                case ViewTypes.FIELD_ID_LIST:
                    {
                        WlViewFieldList aggViewFieldList = (WlViewFieldList?)ElementList!;
                        List<int> viewFieldList = (List<int>)wlView.FieldList!;
                        WlViewElement<int> tempViewElement = new();
                        WlViewIntElementComparer viewElementComparer = new();

                        for (int i = 0; i < wlView.ElemCount; i++)
                        {
                            tempViewElement.Value = viewFieldList[i];
                            int foundIndex = aggViewFieldList.BinarySearch(0, ElementCount, tempViewElement,
                                                viewElementComparer);

                            if (foundIndex >= 0)
                            {
                                if(!aggViewFieldList[foundIndex].Committed)
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                return false;
                            }
                        }

                        return true;
                    }
                case ViewTypes.ELEMENT_NAME_LIST:
                    {
                        WlViewNameList aggViewFieldList = (WlViewNameList)ElementList!;
                        List<string> viewElementNameList = (List<string>)wlView.FieldList!;
                        WlViewElement<string> tempViewElement = new();
                        WlViewStringElementComparer viewElementComparer = new();

                        for (int i = 0; i < wlView.ElemCount; i++)
                        {
                            tempViewElement.Value = viewElementNameList[i];
                            int foundIndex = aggViewFieldList.BinarySearch(0, ElementCount, tempViewElement,
                                                viewElementComparer);

                            if (foundIndex >= 0)
                            {
                                if (!aggViewFieldList[foundIndex].Committed)
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                return false;
                            }
                        }

                        return true;
                    }
                default:
                    {
                        Debug.Assert(false);
                        break;
                    }
            }

            return false;
        }

        public void CommitViews()
        {
            bool fieldRemoved = false;

            Debug.Assert(NewViews.Count == 0);

            LinkedListNode<WlView>? currentNode = MergedViews.First;
            LinkedListNode<WlView>? nextNode;
            while(currentNode != null)
            {
                nextNode = currentNode.Next;
                MergedViews.Remove(currentNode);
                CommittedViews.AddLast(currentNode.Value.ViewNode);
                currentNode = nextNode;
            }

            MergedViews.Clear();

            /* Go through the list and clean up any zeroed fields. */
            switch(ViewType)
            {
                case ViewTypes.FIELD_ID_LIST:
                    {
                        WlViewFieldList aggViewFieldList = (WlViewFieldList?)ElementList!;

                        if(ElementList != null && CommittedViews.Count == 0)
                        {
                            aggViewFieldList.Clear();
                            ElementList = null;
                            break;
                        }

                        for(int i = 0; i < ElementCount;)
                        {
                            if(aggViewFieldList[i].Count == 0)
                            {
                                fieldRemoved = true;
                                aggViewFieldList[i].Committed = aggViewFieldList[ElementCount - 1].Committed;
                                aggViewFieldList[i].Count = aggViewFieldList[ElementCount - 1].Count;
                                aggViewFieldList[i].Value = aggViewFieldList[ElementCount - 1].Value;
                                --ElementCount;
                            }
                            else
                            {
                                aggViewFieldList[i].Committed = true;
                                ++i;
                            }
                        }

                        if(fieldRemoved)
                        {
                            aggViewFieldList.Sort(0, ElementCount, new WlViewIntElementComparer());
                        }

                        break;
                    }
                case ViewTypes.ELEMENT_NAME_LIST:
                    {
                        WlViewNameList aggViewFieldList = (WlViewNameList)ElementList!;

                        if (ElementList != null && CommittedViews.Count == 0)
                        {
                            aggViewFieldList.Clear();
                            ElementList = null;
                            break;
                        }

                        for (int i = 0; i < ElementCount;)
                        {
                            if (aggViewFieldList[i].Count == 0)
                            {
                                fieldRemoved = true;
                                aggViewFieldList[i].Committed = aggViewFieldList[ElementCount - 1].Committed;
                                aggViewFieldList[i].Count = aggViewFieldList[ElementCount - 1].Count;
                                aggViewFieldList[i].Value = aggViewFieldList[ElementCount - 1].Value;
                                --ElementCount;
                            }
                            else
                            {
                                aggViewFieldList[i].Committed = true;
                                ++i;
                            }
                        }

                        if (fieldRemoved)
                        {
                            aggViewFieldList.Sort(0, ElementCount, new WlViewStringElementComparer());
                        }

                        break;
                    }
                default:
                    {
                        Debug.Assert(false);
                        break;
                    }
            }

        }

        public void UnMerged()
        {
            LinkedListNode<WlView>? currentNode = NewViews.First;
            LinkedListNode<WlView>? nextNode;
            WlView currentView;

            while(currentNode != null)
            {
                nextNode = currentNode.Next;
                currentView = currentNode.Value;

                /* Unmerge removes uncommitted views, so any zeroed fields can be removed. */
                UnMergedAView(currentView, true, out _);

                currentNode = nextNode;
            }

            if (CommittedViews.First != null)
            {
                int count = CommittedViews.Count;
                foreach (var view in CommittedViews)
                {
                    NewViews.AddLast(view.ViewNode);

                    if (--count == 0)
                        break;
                }

                CommittedViews.Clear();
            }
        }

        public void UnCommit()
        {
            LinkedListNode<WlView>? currentNode = CommittedViews.First;
            LinkedListNode<WlView>? nextNode;

            while (currentNode != null)
            {
                nextNode = currentNode.Next;

                CommittedViews.Remove(currentNode);
                MergedViews.AddLast(currentNode);
                currentNode = nextNode;
            }
        }

        public bool UnMergedAView(WlView wlView, bool removeZeroFields, out bool viewChangedToZero)
        {
            Debug.Assert(ReferenceEquals(wlView.ViewNode.List, MergedViews) ||
                ReferenceEquals(wlView.ViewNode.List, CommittedViews));

            /* This resets all properties of the view node to null */
            wlView.ViewNode.List.Remove(wlView.ViewNode);

            viewChangedToZero = false;

            switch (ViewType)
            {
                case ViewTypes.FIELD_ID_LIST:
                    {
                        return RemoveFromAggregateViewList<int>(wlView.ElemCount, (WlViewFieldList)ElementList!, (List<int>)wlView.FieldList!,
                            new WlViewIntElementComparer(), removeZeroFields, out viewChangedToZero);
                    }
                case ViewTypes.ELEMENT_NAME_LIST:
                    {
                        return RemoveFromAggregateViewList<string>(wlView.ElemCount, (WlViewNameList)ElementList!, (List<string>)wlView.FieldList!,
                            new WlViewStringElementComparer(), removeZeroFields, out viewChangedToZero);
                    }
                default:
                    {
                        Debug.Assert(false);
                        return false;
                    }
            }
        }



        private bool RemoveFromAggregateViewList<T>(int wlViewElemCount, List<WlViewElement<T>> aggViewFieldList, List<T> viewFieldList, 
            IComparer<WlViewElement<T>> viewElementComparer, bool removeZeroFields, out bool viewChangedToZero)
        {
            Debug.Assert(aggViewFieldList != null);

            bool viewUpdated = false;
            WlViewElement<T> tempViewElement = new();
            viewChangedToZero = false;

            // Decreases field counts.
            for (int i = 0; i < wlViewElemCount; i++)
            {
                tempViewElement.Value = viewFieldList[i];
                int index = aggViewFieldList.BinarySearch(0, ElementCount, tempViewElement,
                    viewElementComparer);

                if (index >= 0)
                {
                    --aggViewFieldList[index].Count;

                    if(aggViewFieldList[index].Count == 0)
                    {
                        viewChangedToZero = true;
                    }
                }
            }

            /* Removes fields that have zero count(can't remove them in the first pass
             * to avoid breaking the sorted ordering needed for binary search */
            if (removeZeroFields)
            {
                for (int i = 0; i < aggViewFieldList.Count; i++)
                {
                    if (aggViewFieldList[i].Count == 0 && !aggViewFieldList[i].Committed)
                    {
                        viewUpdated = true;
                        aggViewFieldList[i] = aggViewFieldList[ElementCount - 1];
                        --ElementCount;
                    }
                }
            }

            /* Resort the list if anything changed. */
            if (viewUpdated)
            {
                aggViewFieldList.Sort(0, ElementCount, viewElementComparer);
            }

            return viewUpdated;
        }
    }
}
