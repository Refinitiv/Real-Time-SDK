/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/


#ifndef _RTR_RSSL_VA_YIELDCURVE_ITEMS_H
#define _RTR_RSSL_VA_YIELDCURVE_ITEMS_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rsslVAItemEncode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SET_DEFS_DB_IDS
{
	SWAP_RATES_REFRESH_SET_DEFS_DB,
	SWAP_RATES_UPDATE_SET_DEFS_DB,
	CASH_RATES_REFRESH_SET_DEFS_DB,
	CASH_RATES_UPDATE_SET_DEFS_DB,
	FUTR_RATES_REFRESH_SET_DEFS_DB,
	FUTR_RATES_UPDATE_SET_DEFS_DB,
	YIELD_CURVE_REFRESH_SET_DEFS_DB,
	YIELD_CURVE_UPDATE_SET_DEFS_DB
} setDefsDBIds;

#define TIMACT_FID				5
#define TRADE_DATE_FID			16
#define ACTIVE_DATE_FID			17

#define CRV_ID_FID				30100
#define CRV_NAME_FID			30101
#define CCY_CODE_FID			30102
#define CRV_TYPE_FID			30103
#define CRV_STYPE_FID			30104
#define CRV_DATE_FID			30105
#define CITIES_FID				30106
#define VAL_DATE_FID			30107
#define SETTL_DATE_FID			30108
#define CRV_ALGTHM_FID			30109
#define INTER_MTHD_FID			30110
#define EXTRP_MTHD_FID			30111
#define CC_METHOD_FID			30112
#define ROLL_CONV_FID			30113
#define ZC_BASIS_FID			30114
#define SPOT_LAG_FID			30115
#define DSCT_FACT_FID			30116
#define DSCT_BASIS_FID			30117
#define CURVE_STS_FID			30118
#define USER_ID_FID				30119
#define MOD_USERID_FID			30120
#define CRT_TIME_FID			30121
#define MOD_TIME_FID			30122
#define COMMENT_FID				30123
#define FWD_BASIS_FID			30124
#define FWD_PERIOD_FID			30125
#define TENORS_FID				30126
#define CASH_BASIS_FID			30140
#define CASH_RATES_FID			30141
#define CASH_SDATE_FID			30142
#define CASH_MDATE_FID			30143
#define CASH_RATE_FID			30144
#define CASH_SRC_FID			30145
#define CASH_INTER_MTHD_FID		30146
#define FUTR_BASIS_FID			30150
#define FUTR_FREQ_FID			30154
#define FUTR_PRCS_FID			30151
#define FUTR_SDATE_FID			30152
#define FUTR_MDATE_FID			30153
#define FUTR_PRICE_FID			30155
#define FUTR_SRC_FID			30156
#define FUTR_INTER_MTHD_FID		30157
#define SWAP_INTER_MTHD_ST_FID	30158
#define SWAP_INTER_MTHD_LT_FID	30159
#define SWAP_BASIS_ST_FID		30160
#define SWAP_RATES_FID			30161
#define SWAP_SDATE_FID			30162
#define SWAP_RDATE_FID			30163
#define SWAP_MDATE_FID			30164
#define SWAP_FREQ_ST_FID		30165
#define SWAP_RATE_VAL_FID		30166
#define SWAP_SRC_FID			30167
#define SWAP_BASIS_LT_FID		30168
#define SWAP_FREQ_LT_FID		30169
#define SPRD_BASIS_FID			30170
#define SPRD_FREQ_FID			30175
#define SPRD_RATES_FID			30171
#define SPRD_SDATE_FID			30172
#define SPRD_RDATE_FID			30173
#define SPRD_MDATE_FID			30174
#define SPRD_RATE_FID			30176
#define SPRD_SRC_FID			30177
#define YLD_CURVE_FID			30200
#define YCT_DATE_FID			30201
#define YCT_ZRATE_FID			30202
#define YCT_DISFAC_FID			30203
#define YCT_YTM_FID				30204
#define YCT_FWDATE_FID			30205
#define YCT_FWRATE_FID			30206

#define MAX_CRV_NAME_STRLEN 64
#define MAX_ENTRIES 5

/* Yield Curve */
typedef struct {
	RsslDate		YCT_DATE;
	RsslReal		YCT_ZRATE;
	RsslReal		YCT_DISFAC;
	RsslReal		YCT_YTM;
	RsslDate		YCT_FWDATE[MAX_ENTRIES];
	RsslReal		YCT_FWRATE[MAX_ENTRIES];
} RsslYieldCurve;

/* Yield Curve Forward Period */
typedef struct {
	char			FWD_PERIOD[MAX_CRV_NAME_STRLEN];
} RsslYieldCurveForwardPeriod;

/* Yield Curve Tenors */
typedef struct {
	char			TENORS[MAX_CRV_NAME_STRLEN];
} RsslYieldCurveTenors;

/* Yield Curve Swap Rates */
typedef struct {
	RsslDate		SWAP_SDATE;
	RsslDate		SWAP_RDATE;
	RsslDate		SWAP_MDATE;
	RsslReal		SWAP_RATE_VAL;
	char			SWAP_SRC[MAX_CRV_NAME_STRLEN];
} RsslYieldCurveSwapRates;

/* Yield Curve Cash Rates */
typedef struct {
	RsslDate		CASH_SDATE;
	RsslDate		CASH_MDATE;
	RsslReal		CASH_RATE;
	char			CASH_SRC[MAX_CRV_NAME_STRLEN];
} RsslYieldCurveCashRates;

/* Yield Curve Future Prices */
typedef struct {
	RsslDate		FUTR_SDATE;
	RsslDate		FUTR_MDATE;
	RsslReal		FUTR_PRICE;
	char			FUTR_SRC[MAX_CRV_NAME_STRLEN];
} RsslYieldCurveFuturePrices;

#define FWD_PERIOD_NUM 4
#define TENORS_NUM 13
#define YC_NUM 13
#define SWAP_RATES_NUM 13

/* yield curve item data */
typedef struct {
	RsslBool					isInUse;
	RsslInt						CRV_ID;
	RsslInt						SPOT_LAG;	
	RsslDateTime				CRV_DATE;
	RsslDateTime				CRT_TIME;
	RsslDateTime				MOD_TIME;
	RsslDate					VAL_DATE;
	RsslDate					SETTL_DATE;
	RsslDate					TRADE_DATE;
	RsslDate					ACTIVE_DATE;
	RsslTime					TIMACT;
	RsslReal					CASH_RATE;
	RsslReal					SPRD_RATE;
	RsslYieldCurve				yc[YC_NUM];
	RsslYieldCurveCashRates		cashRates;
	RsslYieldCurveFuturePrices	futurePrices;
	RsslYieldCurveForwardPeriod fwdPeriod[FWD_PERIOD_NUM];
	RsslYieldCurveTenors		tenors[TENORS_NUM];
	RsslYieldCurveSwapRates		swapRates[SWAP_RATES_NUM];
	char						CRV_NAME[MAX_CRV_NAME_STRLEN];
	char						CRV_TYPE[MAX_CRV_NAME_STRLEN];
	char						CRV_STYPE[MAX_CRV_NAME_STRLEN];
	char						CITIES[MAX_CRV_NAME_STRLEN];
	char						CRV_ALGTHM[MAX_CRV_NAME_STRLEN];
	char						INTER_MTHD[MAX_CRV_NAME_STRLEN];
	char						EXTRP_MTHD[MAX_CRV_NAME_STRLEN];
	char						CC_METHOD[MAX_CRV_NAME_STRLEN];
	char						ROLL_CONV[MAX_CRV_NAME_STRLEN];
	char						ZC_BASIS[MAX_CRV_NAME_STRLEN];
	char						DSCT_FACT[MAX_CRV_NAME_STRLEN];
	char						DSCT_BASIS[MAX_CRV_NAME_STRLEN];
	char						CURVE_STS[MAX_CRV_NAME_STRLEN];
	char						USER_ID[MAX_CRV_NAME_STRLEN];
	char						MOD_USERID[MAX_CRV_NAME_STRLEN];
	char						COMMENT[MAX_CRV_NAME_STRLEN];
	char						FWD_BASIS[MAX_CRV_NAME_STRLEN];
	char						CASH_BASIS[MAX_CRV_NAME_STRLEN];
	char						CASH_INTER_MTHD[MAX_CRV_NAME_STRLEN];
	char						FUTR_BASIS[MAX_CRV_NAME_STRLEN];
	char						FUTR_INTER_MTHD[MAX_CRV_NAME_STRLEN];
	char						SWAP_INTER_MTHD_LT[MAX_CRV_NAME_STRLEN];
	char						SWAP_FREQ_ST[MAX_CRV_NAME_STRLEN];
	char						SWAP_BASIS_LT[MAX_CRV_NAME_STRLEN];
	char						SWAP_FREQ_LT[MAX_CRV_NAME_STRLEN];
} RsslYieldCurveItem;

void initYieldCurveItemFields(RsslYieldCurveItem* ycItem);
void updateYieldCurveItemFields(RsslYieldCurveItem* ycInfo);
RsslRet encodeYieldCurveResponse(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary);

/*
 * Clears the item information.
 * itemInfo - The item information to be cleared
 */
RTR_C_INLINE void clearYieldCurveItem(RsslYieldCurveItem* itemInfo)
{
	int i;
	RsslYieldCurve				*yc;
	RsslYieldCurveSwapRates		*swapRates;

	itemInfo->isInUse = RSSL_FALSE;

	itemInfo->CRV_ID = 0;
	itemInfo->SPOT_LAG = 0;

	rsslClearDateTime(&itemInfo->CRT_TIME);
	rsslClearDateTime(&itemInfo->MOD_TIME);
	rsslClearDateTime(&itemInfo->CRV_DATE);
	rsslClearDate(&itemInfo->VAL_DATE);
	rsslClearDate(&itemInfo->SETTL_DATE);
	rsslClearDate(&itemInfo->TRADE_DATE);
	rsslClearDate(&itemInfo->ACTIVE_DATE);
	rsslClearTime(&itemInfo->TIMACT);

	rsslClearReal(&itemInfo->SPRD_RATE);

	itemInfo->CRV_NAME[0]			= '\0';
	itemInfo->CRV_TYPE[0]			= '\0';
	itemInfo->CRV_STYPE[0]			= '\0';
	itemInfo->CITIES[0]				= '\0';
	itemInfo->CRV_ALGTHM[0]			= '\0';
	itemInfo->INTER_MTHD[0]			= '\0';
	itemInfo->EXTRP_MTHD[0]			= '\0';
	itemInfo->CC_METHOD[0]			= '\0';
	itemInfo->ROLL_CONV[0]			= '\0';
	itemInfo->ZC_BASIS[0]			= '\0';
	itemInfo->DSCT_FACT[0]			= '\0';
	itemInfo->DSCT_BASIS[0]			= '\0';
	itemInfo->CURVE_STS[0]			= '\0';
	itemInfo->USER_ID[0]			= '\0';
	itemInfo->MOD_USERID[0]			= '\0';
	itemInfo->COMMENT[0]			= '\0';
	itemInfo->FWD_BASIS[0]			= '\0';
	itemInfo->CASH_BASIS[0]			= '\0';
	itemInfo->CASH_INTER_MTHD[0]	= '\0';
	itemInfo->FUTR_BASIS[0]			= '\0';
	itemInfo->FUTR_INTER_MTHD[0]	= '\0';
	itemInfo->SWAP_INTER_MTHD_LT[0]	= '\0';
	itemInfo->SWAP_FREQ_ST[0]		= '\0';
	itemInfo->SWAP_BASIS_LT[0]		= '\0';
	itemInfo->SWAP_FREQ_LT[0]		= '\0';

	for (i = 0; i < YC_NUM; i++)
	{
		int j = 0;
		yc = &itemInfo->yc[i];
		rsslClearDate(&yc->YCT_DATE);
		rsslClearReal(&yc->YCT_ZRATE);
		rsslClearReal(&yc->YCT_DISFAC);
		rsslClearReal(&yc->YCT_YTM);
		for (j = 0; j < MAX_ENTRIES; j++)
		{
			rsslClearDate(&yc->YCT_FWDATE[j]);
			rsslClearReal(&yc->YCT_FWRATE[j]);
		}
	}

	for (i = 0; i < SWAP_RATES_NUM; i++)
	{
		swapRates = &itemInfo->swapRates[i];
		rsslClearDate(&swapRates->SWAP_MDATE);
		rsslClearDate(&swapRates->SWAP_RDATE);
		rsslClearDate(&swapRates->SWAP_SDATE);
		rsslClearReal(&swapRates->SWAP_RATE_VAL);
		swapRates->SWAP_SRC[0] = '\0';
	}

	rsslClearDate(&itemInfo->cashRates.CASH_MDATE);
	rsslClearDate(&itemInfo->cashRates.CASH_SDATE);
	rsslClearReal(&itemInfo->cashRates.CASH_RATE);
	itemInfo->cashRates.CASH_SRC[0] = '\0';

	rsslClearDate(&itemInfo->futurePrices.FUTR_MDATE);
	rsslClearDate(&itemInfo->futurePrices.FUTR_SDATE);
	rsslClearReal(&itemInfo->futurePrices.FUTR_PRICE);
	itemInfo->futurePrices.FUTR_SRC[0] = '\0';

	for (i = 0; i < FWD_PERIOD_NUM; i++)
	{
		itemInfo->fwdPeriod[i].FWD_PERIOD[0] = '\0';
	}

	for (i = 0; i < TENORS_NUM; i++)
	{
		itemInfo->tenors[i].TENORS[0] = '\0';
	}
}

void initYieldCurveItems();
RsslYieldCurveItem* getYieldCurveItem(char* itemName);
void freeYieldCurveItem(RsslYieldCurveItem* ycItem);

#ifdef __cplusplus
};
#endif

#endif

