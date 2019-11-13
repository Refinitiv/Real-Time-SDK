/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef WL_VIEW_H
#define WL_VIEW_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslErrorInfo.h"

/* Implements aggregation of multiple lists of field IDs into a single list, to
 * aid in aggregation of requested views on a stream. */

/* These functions are written to properly maintain the overall view of a stream while
 * changing it only when necessary.
 *
 * The functions work as follows:
 *   Adding a view:
 *     Don't change the overall view yet; just add it to a list of new views.
 *
 *   Remove:
 *     If view was merged into the overall view, decrement the counts of matching fields.
 *     If the view was not committed, remove these fields immediately. Otherwise, leave them
 *     (see Commit section below).
 *   
 *   Merge:
 *     Add all the views to the overall view's list of fields.  If fields were added,
 *     or zero-count fields were found, the view should be considered updated.
 * 
 *   Unmerge:
 *     This should only be done when the overall view has been merged but the attempt
 *     to send the updated view failed.
 *     Reverts the overall view to its state before the new views were merged in.
 *     Any fields that the unmerge sets to zero-count should be removed(but zero-count fields
 *     that were not a result of the unmerge should remain).
 *
 *   Encode:
 *     Encodes the RsslArray representing the overall view. Zero-count fields should be skipped.
 *
 *   Commit:
 *     Perform this after an updated overall view is successfully sent on a stream.
 *     Considers all newly-merged views to be committed.
 *     All zero-count fields can now be removed.
 *     
 *   By keeping the zero-count fields in place until a commit, the overall view
 *   can efficiently maintain the full list of fields while only reporting a change when fields
 *   are actually added or removed.
 *   
 */

/* Storage for a single view. */
typedef struct
{
	RsslQueueLink	qlComponentViews;
	RsslQueue		*pParentQueue;
	RsslUInt		viewType;
	RsslUInt32		elemCount;
	void			*elemList;
	char			*nameBuf;			/* Memory that stores names for element list views. */
} WlView;

/* Initializes a view structure. Sorts the fields and removes any duplicates or instances
 * of field ID 0(which should not appear in a view). */
WlView *wlViewCreate(void *elemList, RsslUInt32 elemCount, RsslUInt viewType,
		RsslErrorInfo *pErrorInfo);

void wlViewDestroy(WlView *pView);

/* If the viewType is RDM_VIEW_TYPE_FIELD_ID_LIST, the aggregate viewElemList will consist of 
 * these.*/
typedef struct
{
	RsslFieldId	fieldId;
	RsslBool	committed;
	RsslUInt32	count;
} WlViewField;

/* If the viewType is RDM_VIEW_TYPE_ELEMENT_NAME_LIST, the aggregate viewElemList will consist of 
 * these.*/
typedef struct
{
	RsslBuffer	name;
	RsslBool	committed;
	RsslUInt32	count;
} WlViewName;

/* Aggregated view. */
typedef struct
{
	RsslUInt	viewType;
	RsslQueue	newViews;			/* Views that have not been added to the overall view. */
	RsslQueue	mergedViews;		/* Views that have been added to the overall view but
									 * may need to be undone if sending the new view fails.	*/
	RsslQueue	committedViews;		/* Views that have been added to the overall view,
									 * and won't need not be undone. */
	RsslUInt32	elemCapacity;		/* Max capacity of viewElemList. */
	RsslUInt32	elemCount;			/* Number of actual fields in viewElemList. */
	void		*elemList;			/* List of aggregated elements. */
} WlAggregateView;

/* Initializes an aggregate view structure. */
WlAggregateView *wlAggregateViewCreate(RsslErrorInfo *pErrorInfo);

/* Destroys an aggregate view structure. */
void wlAggregateViewDestroy(WlAggregateView *pView);

/* Adds a list of field IDs to the aggregate view. 
 * Returns true if the addition updated the aggregate view, false otherwise. */
void wlAggregateViewAdd(WlAggregateView *pAggView, WlView *pView);

/* Checks whether a given view's fields are already present in the committed view. */
RsslBool wlAggregateViewContains(WlAggregateView *pAggView, WlView *pView);

/* Removes a view from the aggregate view.
 * Returns false if the removal updated the aggregate view, false otherwise. */
void wlAggregateViewRemove(WlAggregateView *pAggView, WlView *pView);

/* Merges any new views into the overall view. Returns true if the view has changed since
 * the last commit. */
RsslRet wlAggregateViewMerge(WlAggregateView *pAggView, RsslBool *pUpdated,
		RsslErrorInfo *pErrorInfo);

/* Un-merges any new views. Use this when an attempt to send the overall view fails. */
void wlAggregateViewUnmerge(WlAggregateView *pAggView);

/* Estimates the encoded length of the view entries. Requires all new views to be merged before 
 * calling. */
RsslUInt32 wlAggregateViewEstimateEncodedLength(WlAggregateView *pView);

/* Encodes an RsslArray of the aggregated field IDs, as per the model for encoding views. */
RsslRet wlAggregateViewEncodeArray(RsslEncodeIterator *pIter, WlAggregateView *pView);

/* When a view is succesfully sent on a stream, call this to commit the component views. */
void wlAggregateViewCommitViews(WlAggregateView *pAggView);

#endif
