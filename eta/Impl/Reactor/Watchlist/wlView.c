/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/wlView.h"
#include "rtr/rsslArray.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslReactorUtils.h"
#include "rtr/rsslHeapBuffer.h"
#include <stdlib.h>
#include <assert.h>

/* Field ID compare function; used to sort fields in a view. */
static int wlaCompareFieldId(const void *p1, const void *p2)
{
	const RsslFieldId fid1 = *((const RsslFieldId*)p1);
	const RsslFieldId fid2 = *((const RsslFieldId*)p2);
	
	if (fid1 < fid2) return -1;
	else if (fid1 == fid2) return 0;
	else return 1;
}

/* Field ID compare function; used to sort fields in an aggregate view. */
static int wlaCompareViewField(const void *p1, const void *p2)
{
	const WlViewField *pViewFid1 = (const WlViewField*)p1;
	const WlViewField *pViewFid2 = (const WlViewField*)p2;
	
	if (pViewFid1->fieldId < pViewFid2->fieldId) return -1;
	else if (pViewFid1->fieldId == pViewFid2->fieldId) return 0;
	else return 1;
}

/* Field ID compare function; used to find fields in an aggregate view by their ID. */
static int wlaCompareViewFieldWithId(const void *p1, const void *p2)
{
	const RsslFieldId fieldId = *(const RsslFieldId*)p1;
	const WlViewField *pViewFid = (const WlViewField*)p2;
	
	if (fieldId < pViewFid->fieldId) return -1;
	else if (fieldId == pViewFid->fieldId) return 0;
	else return 1;
}

/* Compare RsslBuffers as strings(view element names are required to be ASCII strings). */
static int wlaCompareRsslString(const RsslBuffer *pString1, const RsslBuffer *pString2)
{
	if (pString1->length == pString2->length)
		return strncmp(pString1->data, pString2->data, pString1->length);

	else if (pString1->length < pString2->length)
	{
		/* String 1 is shorter, so it is lesser if string 2's substring matches it. */
		int ret = strncmp(pString1->data, pString2->data, pString1->length);
		return ret == 0 ? -1 : ret;
	}
	else
	{
		/* String 2 is shorter, so it is lesser if string 1's substring matches it. */
		int ret = strncmp(pString1->data, pString2->data, pString2->length);
		return ret == 0 ? 1 : ret;
	}
}

/* Element name compare function; used to sort elements in an aggregate view. */
static int wlaCompareName(const void *p1, const void *p2)
{
	const RsslBuffer *pName1 = ((const RsslBuffer*)p1);
	const RsslBuffer *pName2 = ((const RsslBuffer*)p2);

	return wlaCompareRsslString(pName1, pName2);
}

/* Element name compare function; used to sort elements in an aggregate view. */
static int wlaCompareViewName(const void *p1, const void *p2)
{
	const WlViewName *pViewName1 = (const WlViewName*)p1;
	const WlViewName *pViewName2 = (const WlViewName*)p2;

	return wlaCompareRsslString(&pViewName1->name, &pViewName2->name);
}

/* Element name compare function; find elements in an aggregate view by their name. */
int wlaCompareViewNameWithName(const void *p1, const void *p2)
{
	const RsslBuffer *pName = (const RsslBuffer*)p1;
	const WlViewName *pViewName = (const WlViewName*)p2;
	
	return wlaCompareRsslString(pName, &pViewName->name);
}

/* Merges a view into the aggregated view. */
static RsslRet wlaMergeView(WlAggregateView *pAggView, WlView *pView, RsslBool *pUpdated,
		RsslErrorInfo *pErrorInfo);

/* Un-merges a view from the aggregated view. */
static RsslBool wlaUnmergeView(WlAggregateView *pAggView, WlView *pView, RsslBool removeZeroFields);

static RsslRet wlaMergeView(WlAggregateView *pAggView, WlView *pView, RsslBool *pUpdated,
		RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 ui;
	RsslUInt32 mergedCount;

	assert(pView->pParentQueue == &pAggView->newViews);

	mergedCount = pAggView->mergedViews.count + pAggView->committedViews.count;

	switch(pAggView->viewType)
	{

	case RDM_VIEW_TYPE_FIELD_ID_LIST:
	{
		WlViewField *aggViewFieldList = (WlViewField*)pAggView->elemList;
		RsslUInt32 aggViewFieldCapacity;
		RsslUInt32 aggViewFieldCount;
		RsslFieldId *viewFieldList;

		if (!aggViewFieldList)
		{
			aggViewFieldList = (WlViewField*)malloc(pView->elemCount 
					* sizeof(WlViewField));
			verify_malloc(aggViewFieldList, pErrorInfo, RSSL_RET_FAILURE);
			pAggView->elemCapacity = pView->elemCount;
			pAggView->elemCount = 0;
			pAggView->elemList = (void*)aggViewFieldList;
		}

		aggViewFieldCapacity = pAggView->elemCapacity;
		aggViewFieldCount = pAggView->elemCount;
		viewFieldList = (RsslFieldId*)pView->elemList;

		for(ui = 0; ui < pView->elemCount; ++ui)
		{
			WlViewField *pViewField = (WlViewField*)bsearch(&viewFieldList[ui],
					aggViewFieldList, pAggView->elemCount, sizeof(WlViewField),
					wlaCompareViewFieldWithId);

			if (pViewField)
			{
				/* Field exists in list already; just update count. */
				++pViewField->count;
			}
			else
			{
				/* Need to add this field to the view. */
				if (aggViewFieldCount == aggViewFieldCapacity)
				{
					/* Out of space -- grow the list. */
					aggViewFieldCapacity *= 2;
					aggViewFieldList = (WlViewField*)realloc(aggViewFieldList,
							aggViewFieldCapacity * sizeof(WlViewField));
					verify_malloc(aggViewFieldList, pErrorInfo, RSSL_RET_FAILURE);
					pAggView->elemList = (void*)aggViewFieldList;
				}

				/* Add the new field to the end (we'll sort it in later -- we only need to
				 * bsearch for fields that were in the aggregate list to begin with; the
				 * component views already have any duplicates removed). */
				aggViewFieldList[aggViewFieldCount].fieldId = viewFieldList[ui];
				aggViewFieldList[aggViewFieldCount].count = 1;
				aggViewFieldList[aggViewFieldCount].committed = RSSL_FALSE;
				++aggViewFieldCount;
			}
		}

		pAggView->elemCapacity = aggViewFieldCapacity;
		pAggView->elemCount = aggViewFieldCount;

		qsort(aggViewFieldList, pAggView->elemCount, sizeof(WlViewField), 
				wlaCompareViewField);
		break;
	}

	case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
	{
		WlViewName *aggViewNameList = (WlViewName*)pAggView->elemList;
		RsslUInt32 aggViewNameCapacity;
		RsslUInt32 aggViewNameCount;
		RsslBuffer *viewNameList;

		if (!aggViewNameList)
		{
			aggViewNameList = (WlViewName*)malloc(pView->elemCount 
					* sizeof(WlViewName));
			verify_malloc(aggViewNameList, pErrorInfo, RSSL_RET_FAILURE);
			pAggView->elemCapacity = pView->elemCount;
			pAggView->elemCount = 0;
			pAggView->elemList = (void*)aggViewNameList;
		}

		aggViewNameCapacity = pAggView->elemCapacity;
		aggViewNameCount = pAggView->elemCount;
		viewNameList = (RsslBuffer*)pView->elemList;

		for(ui = 0; ui < pView->elemCount; ++ui)
		{
			WlViewName *pViewName = (WlViewName*)bsearch(&viewNameList[ui],
					aggViewNameList, pAggView->elemCount, sizeof(WlViewName),
					wlaCompareViewNameWithName);

			if (pViewName)
			{
				/* Name exists in list already; just update count. */
				++pViewName->count;
			}
			else
			{
				/* Need to add this field to the view. */

				*pUpdated = RSSL_TRUE;
				if (aggViewNameCount == aggViewNameCapacity)
				{
					/* Out of space -- grow the list. */
					aggViewNameCapacity *= 2;
					aggViewNameList = (WlViewName*)realloc(aggViewNameList,
							aggViewNameCapacity * sizeof(WlViewName));
					verify_malloc(aggViewNameList, pErrorInfo, RSSL_RET_FAILURE);
					pAggView->elemList = (void*)aggViewNameList;
				}

				/* Add the new field to the end (we'll sort it in later -- we only need to
				 * bsearch for fields that were in the aggregate list to begin with; the
				 * component views already have any duplicates removed). */
				aggViewNameList[aggViewNameCount].count = 1;
				aggViewNameList[aggViewNameCount].committed = RSSL_FALSE;
				if (rsslHeapBufferCreateCopy(&aggViewNameList[aggViewNameCount].name, 
						&viewNameList[ui]) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
							"Memory allocation failed.");
					pAggView->elemCount = aggViewNameCount;
					return RSSL_RET_FAILURE;
				}
				++aggViewNameCount;
			}
		}

		pAggView->elemCapacity = aggViewNameCapacity;
		pAggView->elemCount = aggViewNameCount;

		qsort(aggViewNameList, pAggView->elemCount, sizeof(WlViewName), 
				wlaCompareViewName);
		break;
	}

	default: 
		assert(0); 
		break;
	}

	rsslQueueRemoveLink(&pAggView->newViews, &pView->qlComponentViews);
	rsslQueueAddLinkToBack(&pAggView->mergedViews, &pView->qlComponentViews);
	pView->pParentQueue = &pAggView->mergedViews;

	return RSSL_RET_SUCCESS;

}

RsslBool wlAggregateViewContains(WlAggregateView *pAggView, WlView *pView)
{
	RsslUInt32 ui;
	RsslUInt32 mergedCount;
	RsslBool viewUpdated = RSSL_FALSE;

	mergedCount = pAggView->mergedViews.count + pAggView->committedViews.count;

	switch(pAggView->viewType)
	{

	case RDM_VIEW_TYPE_FIELD_ID_LIST:
	{
		WlViewField *aggViewFieldList = (WlViewField*)pAggView->elemList;

		switch(mergedCount)
		{
			case 0:
				return RSSL_FALSE;
			default:
			{
				/* Check against aggregate view. */
				RsslFieldId *viewFieldList = (RsslFieldId*)pView->elemList;
				WlViewField *aggViewFieldList = (WlViewField*)pAggView->elemList;

				if (pView->elemCount > pAggView->elemCount)
					return RSSL_FALSE;

				for(ui = 0; ui < pView->elemCount; ++ui)
				{
					WlViewField *pViewField = (WlViewField*)bsearch(&viewFieldList[ui], aggViewFieldList, 
							pAggView->elemCount, sizeof(WlViewField), wlaCompareViewFieldWithId);

					if (!pViewField || !pViewField->committed)
						return RSSL_FALSE;
				}

				return RSSL_TRUE;
			}
		}
		break;
	}

	case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
	{
		WlViewName *aggViewNameList = (WlViewName*)pAggView->elemList;

		switch(mergedCount)
		{
			case 0:
				return RSSL_FALSE;
			default:
			{
				/* Check against aggregate view. */
				RsslBuffer *viewNameList = (RsslBuffer*)pView->elemList;
				WlViewName *aggViewNameList = (WlViewName*)pAggView->elemList;

				if (pView->elemCount > pAggView->elemCount)
					return RSSL_FALSE;

				for(ui = 0; ui < pView->elemCount; ++ui)
				{
					WlViewName *pViewName = (WlViewName*)bsearch(&viewNameList[ui], aggViewNameList, 
							pAggView->elemCount, sizeof(WlViewName), wlaCompareViewNameWithName);

					if (!pViewName || !pViewName->committed)
						return RSSL_FALSE;
				}

				return RSSL_TRUE;
			}
		}
		break;
	}

	default: 
		assert(0); 
		break;
	}

	rsslQueueRemoveLink(&pAggView->newViews, &pView->qlComponentViews);
	rsslQueueAddLinkToBack(&pAggView->mergedViews, &pView->qlComponentViews);
	pView->pParentQueue = &pAggView->mergedViews;

	return viewUpdated;

}

static RsslBool wlaUnmergeView(WlAggregateView *pAggView, WlView *pView, RsslBool removeZeroFields)
{
	RsslUInt32 ui;
	RsslUInt32 mergedCount;

	assert(pView->pParentQueue == &pAggView->mergedViews
			|| pView->pParentQueue == &pAggView->committedViews);

	rsslQueueRemoveLink(pView->pParentQueue, &pView->qlComponentViews);
	pView->pParentQueue = NULL;

	mergedCount = pAggView->mergedViews.count + pAggView->committedViews.count;

	switch(pAggView->viewType)
	{

	case RDM_VIEW_TYPE_FIELD_ID_LIST:
	{
		WlViewField *aggViewFieldList = (WlViewField*)pAggView->elemList;
		RsslBool viewUpdated = RSSL_FALSE;
		RsslFieldId *viewFieldList = (RsslFieldId*)pView->elemList;

		assert(aggViewFieldList);

		/* Decrement field counts. */
		for(ui = 0; ui < pView->elemCount; ++ui)
		{
			WlViewField *pViewField = (WlViewField*)bsearch(&viewFieldList[ui],
					aggViewFieldList, pAggView->elemCount, sizeof(WlViewField),
					wlaCompareViewFieldWithId);

			assert(pViewField && pViewField->count > 0);

			--pViewField->count;
		}

		/* Now remove fields that have been zeroed(can't remove them in the first pass
		 * since we would likely break the sorted ordering needed for binary search). */
		if (removeZeroFields)
			for(ui = 0; ui < pView->elemCount; ++ui)
			{
				if (aggViewFieldList[ui].count == 0 && !aggViewFieldList[ui].committed)
				{
					viewUpdated = RSSL_TRUE;
					aggViewFieldList[ui] = 
						aggViewFieldList[pAggView->elemCount - 1];
					--pAggView->elemCount;
				}

			}

		/* Resort list if anything changed. */
		if (viewUpdated)
			qsort(aggViewFieldList, pAggView->elemCount, sizeof(WlViewField), 
					wlaCompareViewField);

		return viewUpdated;
	}

	case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
	{
		WlViewName *aggViewNameList = (WlViewName*)pAggView->elemList;
		RsslBool viewUpdated = RSSL_FALSE;
		RsslBuffer *viewNameList = (RsslBuffer*)pView->elemList;

		assert(aggViewNameList);

		/* Decrement field counts. */
		for(ui = 0; ui < pView->elemCount; ++ui)
		{
			WlViewName *pViewName = (WlViewName*)bsearch(&viewNameList[ui],
					aggViewNameList, pAggView->elemCount, sizeof(WlViewName),
					wlaCompareViewNameWithName);

			assert(pViewName && pViewName->count > 0);

			--pViewName->count;
		}

		/* Now remove fields that have been zeroed(can't remove them in the first pass
		 * since we would likely break the sorted ordering needed for binary search). */
		if (removeZeroFields)
			for(ui = 0; ui < pView->elemCount; ++ui)
			{
				if (aggViewNameList[ui].count == 0 && !aggViewNameList[ui].committed)
				{
					viewUpdated = RSSL_TRUE;
					rsslHeapBufferCleanup(&aggViewNameList[ui].name);
					aggViewNameList[ui] = 
						aggViewNameList[pAggView->elemCount - 1];
					--pAggView->elemCount;
				}

			}

		/* Resort list if anything changed. */
		if (viewUpdated)
			qsort(aggViewNameList, pAggView->elemCount, sizeof(WlViewName), 
					wlaCompareViewName);

		return viewUpdated;
	}

	default: assert(0); return RSSL_FALSE;
	}
}



WlView *wlViewCreate(void *elemList, RsslUInt32 elemCount, RsslUInt viewType,
		RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 ui;

	assert(viewType == RDM_VIEW_TYPE_FIELD_ID_LIST || viewType == RDM_VIEW_TYPE_ELEMENT_NAME_LIST);

	switch(viewType)
	{

	case RDM_VIEW_TYPE_FIELD_ID_LIST:
	{
		RsslFieldId *fieldIdList = (RsslFieldId*)elemList;
		RsslBool hasDuplicateFields = RSSL_FALSE;

		if (fieldIdList)
		{
			WlView *pView = (WlView*)malloc(sizeof(WlView) + sizeof(RsslFieldId) * elemCount);
			RsslFieldId *viewFieldIdList = (RsslFieldId*)((char*)pView + sizeof(WlView));
			verify_malloc(pView, pErrorInfo, NULL);

			pView->viewType = RDM_VIEW_TYPE_FIELD_ID_LIST;
			pView->elemCount = elemCount;
			pView->elemList = (void*)viewFieldIdList;
			pView->nameBuf = NULL;
			pView->pParentQueue = NULL;

			memcpy(viewFieldIdList, fieldIdList, elemCount * sizeof(RsslFieldId));

			qsort(viewFieldIdList, pView->elemCount, sizeof(RsslFieldId), wlaCompareFieldId);

			/* Check for fields that are 0 or duplicated, and remove them. */

			if (pView->elemCount && viewFieldIdList[0] == 0)
				hasDuplicateFields = RSSL_TRUE;
			for(ui = 1; ui < pView->elemCount; ++ui)
			{
				if (viewFieldIdList[ui] == 0 || viewFieldIdList[ui] == viewFieldIdList[ui-1])
				{
					viewFieldIdList[ui] = 0;
					hasDuplicateFields = RSSL_TRUE;
				}
			}

			if (hasDuplicateFields)
			{
				for(ui = 0; ui < pView->elemCount;)
				{
					if (viewFieldIdList[ui] == 0)
					{
						viewFieldIdList[ui] = viewFieldIdList[elemCount-1];
						(pView->elemCount)--;
					}
					else
						++ui;
				}

				/* Resort list. */
				qsort(viewFieldIdList, pView->elemCount, sizeof(RsslFieldId), wlaCompareFieldId);
			}

			return pView;
		}
		else
		{
			WlView *pView = (WlView*)malloc(sizeof(WlView));
			verify_malloc(pView, pErrorInfo, NULL);
			assert(elemCount == 0);
			pView->elemList = NULL;
			pView->elemCount = 0;
			pView->nameBuf = NULL;
			return pView;
		}
	}

	case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
	{
		RsslBuffer *nameList = (RsslBuffer*)elemList;
		RsslBool hasDuplicateFields = RSSL_FALSE;

		if (nameList)
		{
			WlView *pView;
			RsslBuffer *viewNameList;
			RsslUInt32 elementNameBufSize = 0;

			pView = (WlView*)malloc(sizeof(WlView) + sizeof(RsslBuffer) * elemCount);
			verify_malloc(pView, pErrorInfo, NULL);
			viewNameList = (RsslBuffer*)((char*)pView + sizeof(WlView));

			pView->viewType = RDM_VIEW_TYPE_ELEMENT_NAME_LIST;
			pView->elemCount = elemCount;
			pView->elemList = (void*)viewNameList;
			pView->pParentQueue = NULL;

			memcpy(viewNameList, nameList, elemCount * sizeof(RsslBuffer));

			qsort(viewNameList, pView->elemCount, sizeof(RsslBuffer), wlaCompareName);

			/* Check for any elements that are duplicate, and remove them. */
			for(ui = 1; ui < pView->elemCount; ++ui)
			{
				if (viewNameList[ui-1].data &&
						rsslBufferIsEqual(&viewNameList[ui], &viewNameList[ui-1]))
				{
					/* Any duplicate elements should be removed. 
					 * (Set name to a clearly invalid value so we know to remove it).*/
					viewNameList[ui].data = NULL;
					viewNameList[ui].length = (RsslUInt32)-1;
					hasDuplicateFields = RSSL_TRUE;
				}
			}

			if (hasDuplicateFields)
			{
				/* Remove the duplicated fields. */
				for(ui = 0; ui < pView->elemCount;)
				{
					if (viewNameList[ui].data == NULL && viewNameList[ui].length == (RsslUInt32)-1)
					{
						viewNameList[ui] = viewNameList[elemCount-1];
						(pView->elemCount)--;
					}
					else
						++ui;
				}

				/* Resort list. */
				qsort(viewNameList, pView->elemCount, sizeof(RsslBuffer), wlaCompareName);
			}

			/* Copy element names. */
			for(ui = 0; ui < pView->elemCount; ++ui)
				elementNameBufSize += viewNameList[ui].length; 

			if (elementNameBufSize)
			{
				char *nameBuf = pView->nameBuf = (char*)malloc(elementNameBufSize);

				if (!pView->nameBuf)
				{
					free(pView);
					verify_malloc(0, pErrorInfo, NULL);
				}

				for(ui = 0; ui < pView->elemCount; ++ui)
				{
					strncpy(nameBuf, viewNameList[ui].data, viewNameList[ui].length);
					viewNameList[ui].data = nameBuf;
					nameBuf += viewNameList[ui].length;
				}
			}

			return pView;
		}
		else
		{
			WlView *pView = (WlView*)malloc(sizeof(WlView));
			verify_malloc(pView, pErrorInfo, NULL);
			assert(elemCount == 0);
			pView->elemList = NULL;
			pView->elemCount = 0;
			pView->nameBuf = NULL;
			return pView;
		}
	}

	default: assert(0); return NULL;
	}
}

void wlViewDestroy(WlView *pView)
{
	if (pView->nameBuf)
		free(pView->nameBuf);
	free(pView);
}

WlAggregateView *wlAggregateViewCreate(RsslErrorInfo *pErrorInfo)
{
	WlAggregateView *pView = (WlAggregateView*)malloc(sizeof(WlAggregateView));
	verify_malloc(pView, pErrorInfo, NULL);
	memset(pView, 0, sizeof(WlAggregateView));
	rsslInitQueue(&pView->newViews);
	rsslInitQueue(&pView->mergedViews);
	rsslInitQueue(&pView->committedViews);
	pView->viewType = 0;
	return pView;
}

void wlAggregateViewDestroy(WlAggregateView *pView)
{
	if (pView->elemList)
	{
		if (pView->viewType == RDM_VIEW_TYPE_ELEMENT_NAME_LIST)
		{
			RsslUInt32 ui;
			for (ui = 0; ui < pView->elemCount; ++ui)
				rsslHeapBufferCleanup(&(((WlViewName*)pView->elemList)[ui].name));
		}

		free(pView->elemList);
	}
	free(pView);
}

void wlAggregateViewAdd(WlAggregateView *pAggView, WlView *pView)
{
	assert(pView->pParentQueue == NULL);

	assert(pAggView->viewType == 0 || pAggView->viewType == pView->viewType);

	if (pAggView->viewType == 0)
		pAggView->viewType = pView->viewType;

	pView->pParentQueue = &pAggView->newViews;
	rsslQueueAddLinkToBack(&pAggView->newViews, &pView->qlComponentViews);
}

void wlAggregateViewRemove(WlAggregateView *pAggView, WlView *pView)
{

	if (pView->pParentQueue == &pAggView->committedViews)
	{
		/* When removing committed views, leave zeroed fields in place so we can detect
		 * the need to send a new view. */
		wlaUnmergeView(pAggView, pView, RSSL_FALSE);
		pView->pParentQueue = NULL;
	}
	else if (pView->pParentQueue == &pAggView->newViews)
	{
		rsslQueueRemoveLink(&pAggView->newViews, &pView->qlComponentViews);
	}
	else if (pView->pParentQueue)
	{
		assert(pView->pParentQueue == &pAggView->mergedViews);
		wlaUnmergeView(pAggView, pView, RSSL_TRUE);
		pView->pParentQueue = NULL;
	}

}

RsslRet wlAggregateViewMerge(WlAggregateView *pAggView, RsslBool *pUpdated,
		RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;

	*pUpdated = RSSL_FALSE;

	RSSL_QUEUE_FOR_EACH_LINK(&pAggView->newViews, pLink)
	{
		WlView *pView = RSSL_QUEUE_LINK_TO_OBJECT(WlView, qlComponentViews, pLink);
		RsslRet ret;

		if ((ret = wlaMergeView(pAggView, pView, pUpdated, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;
	}

	if (*pUpdated == RSSL_TRUE)
		return RSSL_RET_SUCCESS;

	if (pAggView->elemList)
	{
		RsslUInt32 ui;

		/* Check for any fields that were added or removed. If so, that updates
		 * the overall view. */

		switch(pAggView->viewType)
		{
			case RDM_VIEW_TYPE_FIELD_ID_LIST:
			{
				WlViewField* viewFieldList = (WlViewField*)pAggView->elemList;

				for(ui = 0; ui < pAggView->elemCount; ++ui)
				{
					if (viewFieldList[ui].count == 0 && viewFieldList[ui].committed
							|| viewFieldList[ui].count && !viewFieldList[ui].committed)
					{
						*pUpdated = RSSL_TRUE;
						break;
					}
				}

				break;
			}
			case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
			{
				WlViewName* viewNameList = (WlViewName*)pAggView->elemList;

				for(ui = 0; ui < pAggView->elemCount; ++ui)
				{
					if (viewNameList[ui].count == 0 && viewNameList[ui].committed
							|| viewNameList[ui].count && !viewNameList[ui].committed)
					{
						*pUpdated = RSSL_TRUE;
						break;
					}
				}

				break;
			}
			default:
				assert(0);
				break;
		}

	}

	return RSSL_RET_SUCCESS;
}

void wlAggregateViewUnmerge(WlAggregateView *pAggView)
{
	RsslQueueLink *pLink;

	RSSL_QUEUE_FOR_EACH_LINK(&pAggView->newViews, pLink)
	{
		WlView *pView = RSSL_QUEUE_LINK_TO_OBJECT(WlView, qlComponentViews, pLink);

		/* Unmerge removes uncommitted views, so any zeroed fields can be removed. */
		wlaUnmergeView(pAggView, pView, RSSL_TRUE);
	}

	rsslQueueAppend(&pAggView->newViews, &pAggView->mergedViews);
}

RsslUInt32 wlAggregateViewEstimateEncodedLength(WlAggregateView *pAggView)
{
	assert(pAggView->newViews.count == 0);
	assert(pAggView->elemList);

	switch(pAggView->viewType)
	{

		case RDM_VIEW_TYPE_FIELD_ID_LIST:
			{
				WlViewField *aggViewFieldList = (WlViewField*)pAggView->elemList;
				RsslUInt32 encodedLength = 0;
				RsslUInt32 ui;

				for(ui = 0; ui < pAggView->elemCount; ++ui)
				{
					if (aggViewFieldList[ui].count != 0)
						encodedLength += 3;
				}
				return encodedLength;
			}

		case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
			{
				WlViewName *aggViewNameList = (WlViewName*)pAggView->elemList;
				RsslUInt32 encodedLength = 0;
				RsslUInt32 ui;

				for(ui = 0; ui < pAggView->elemCount; ++ui)
				{
					if (aggViewNameList[ui].count != 0)
						encodedLength += 3 + aggViewNameList[ui].name.length;
				}
				return encodedLength;
			}

		default: 
			assert(0); 
			return 1;
	}
}

RsslRet wlAggregateViewEncodeArray(RsslEncodeIterator *pIter, WlAggregateView *pAggView)
{
	RsslRet ret;
	RsslArray rsslArray;
	RsslUInt32 ui;

	rsslClearArray(&rsslArray);

	switch(pAggView->viewType)
	{
		case RDM_VIEW_TYPE_FIELD_ID_LIST: 
			/* RsslFieldID is an Int16. */
			assert(sizeof(RsslFieldId) == 2);
			rsslArray.itemLength = 2;
			rsslArray.primitiveType = RSSL_DT_INT; 
			break;
		case RDM_VIEW_TYPE_ELEMENT_NAME_LIST: 
			rsslArray.primitiveType = RSSL_DT_ASCII_STRING; 
			break;
		default: assert(0); break;
	}


	if ((ret = rsslEncodeArrayInit(pIter, &rsslArray)) != RSSL_RET_SUCCESS)
	{
		return ret;
	}

	assert(pAggView->elemList);

	switch(pAggView->viewType)
	{

		case RDM_VIEW_TYPE_FIELD_ID_LIST:
			{
				WlViewField *aggViewFieldList = (WlViewField*)pAggView->elemList;

				/* Encode aggregate view. */
				for(ui = 0; ui < pAggView->elemCount; ++ui)
				{
					RsslInt fieldId;
					if (aggViewFieldList[ui].count == 0)
						continue;
					fieldId = aggViewFieldList[ui].fieldId;
					if ((ret = rsslEncodeArrayEntry(pIter, NULL, &fieldId)) != RSSL_RET_SUCCESS)
						return ret;
				}
				break;
			}

		case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
			{
				WlViewName *aggViewNameList = (WlViewName*)pAggView->elemList;

				/* Encode aggregate view. */
				for(ui = 0; ui < pAggView->elemCount; ++ui)
				{
					if (aggViewNameList[ui].count == 0)
						continue;
					if ((ret = rsslEncodeArrayEntry(pIter, NULL, &aggViewNameList[ui].name)) 
							!= RSSL_RET_SUCCESS)
						return ret;
				}
				break;
			}

		default: 
			assert(0); 
			break;
	}

	if ((ret = rsslEncodeArrayComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

void wlAggregateViewCommitViews(WlAggregateView *pAggView)
{
	RsslBool fieldsRemoved = RSSL_FALSE;
	RsslUInt32 ui;
	RsslQueueLink *pLink;

	assert(pAggView->newViews.count == 0);

	RSSL_QUEUE_FOR_EACH_LINK(&pAggView->mergedViews, pLink)
	{
		WlView *pView = RSSL_QUEUE_LINK_TO_OBJECT(WlView, qlComponentViews, pLink);
		pView->pParentQueue = &pAggView->committedViews;
	}

	rsslQueueAppend(&pAggView->committedViews, &pAggView->mergedViews);

	/* Go through the list and clean up any zeroed fields. */

	switch(pAggView->viewType)
	{
		case RDM_VIEW_TYPE_FIELD_ID_LIST:
		{
			WlViewField *aggViewFieldList = (WlViewField*)pAggView->elemList;

			if (pAggView->elemList && pAggView->committedViews.count == 0)
			{
				/* We can free the overall list. */
				free(pAggView->elemList);
				pAggView->elemList = NULL;
				break;
			}

			for(ui = 0; ui < pAggView->elemCount;)
			{
				if (aggViewFieldList[ui].count == 0)
				{
					fieldsRemoved = RSSL_TRUE;
					aggViewFieldList[ui] = aggViewFieldList[pAggView->elemCount - 1];
					--pAggView->elemCount;
				}
				else
				{
					aggViewFieldList[ui].committed = RSSL_TRUE;
					++ui;
				}
			}

			if (fieldsRemoved)
				qsort(aggViewFieldList, pAggView->elemCount, sizeof(WlViewField), 
						wlaCompareViewField);
			break;
		}

		case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
		{
			WlViewName *aggViewNameList = (WlViewName*)pAggView->elemList;
			
			if (pAggView->elemList && pAggView->committedViews.count == 0)
			{
				/* We can free the overall list. */
				for(ui = 0; ui < pAggView->elemCount; ++ui)
					rsslHeapBufferCleanup(&aggViewNameList[ui].name);
				free(pAggView->elemList);
				pAggView->elemList = NULL;
				break;
			}

			for(ui = 0; ui < pAggView->elemCount;)
			{
				if (aggViewNameList[ui].count == 0)
				{
					fieldsRemoved = RSSL_TRUE;
					rsslHeapBufferCleanup(&aggViewNameList[ui].name);
					aggViewNameList[ui] = aggViewNameList[pAggView->elemCount - 1];
					--pAggView->elemCount;
				}
				else
				{
					aggViewNameList[ui].committed = RSSL_TRUE;
					++ui;
				}
			}

			if (fieldsRemoved)
				qsort(aggViewNameList, pAggView->elemCount, sizeof(WlViewName), 
						wlaCompareViewName);
			break;
		}

		default:
			assert(0);
			break;
	}

}
