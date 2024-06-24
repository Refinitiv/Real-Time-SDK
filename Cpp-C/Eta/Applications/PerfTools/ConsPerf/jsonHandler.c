/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's
 * LICENSE.md for details.
 * Copyright (C) 2022 LSEG. All rights reserved.
*/

#include "jsonHandler.h"

RsslDomainTypes jsonGetDomainType(cJSON* json)
{
	cJSON * jsonDomain = cJSON_GetObjectItem(json, "Domain");

	if (jsonDomain != NULL)
	{
		const char* str = "Login\"Source\"Dictionary\"MarketPrice\"MarketByOrder";
		const char* value = cJSON_GetStringValue(jsonDomain);
		const char* pos = strstr(str, value);
		if (pos != NULL)
		{
			switch (pos - str)
			{
			case 0:
				return RSSL_DMT_LOGIN;
			case 6:
				return RSSL_DMT_SOURCE;
			case 13:
				return RSSL_DMT_DICTIONARY;
			case 24:
				return RSSL_DMT_MARKET_PRICE;
			case 36:
				return RSSL_DMT_MARKET_BY_ORDER;
			}
		}

		return RSSL_DMT_MAX_VALUE;
	}
	else
		return RSSL_DMT_MARKET_PRICE;
}

RsslRDMLoginMsgType jsonGetRDMLoginMsgType(cJSON* json)
{
	cJSON * jsonMsgType = cJSON_GetObjectItem(json, "Type");

	if (jsonMsgType != NULL)
	{
		const char* str = "Refresh\"Status";
		const char* value = cJSON_GetStringValue(jsonMsgType);
		const char* pos = strstr(str, value);
		if (pos != NULL)
		{
			switch (pos - str)
			{
			case 0:
				return RDM_LG_MT_REFRESH;
			case 8:
				return RDM_LG_MT_STATUS;
			}
		}
		else
			return RDM_LG_MT_UNKNOWN;
	}
	else
		return RDM_LG_MT_UNKNOWN;

	return RDM_LG_MT_UNKNOWN;
}

RsslRDMDirectoryMsgType jsonGetRDMDirectoryMsgType(cJSON* json)
{
	cJSON * jsonMsgType = cJSON_GetObjectItem(json, "Type");

	if (jsonMsgType != NULL)
	{
		const char* str = "Refresh\"Update\"Status";
		const char* value = cJSON_GetStringValue(jsonMsgType);
		const char* pos = strstr(str, value);
		if (pos != NULL)
		{
			switch (pos - str)
			{
			case 0:
				return RDM_DR_MT_REFRESH;
			case 8:
				return RDM_DR_MT_UPDATE;
			case 15:
				return RDM_DR_MT_STATUS;
			}
		}
		else
			return RDM_DR_MT_UNKNOWN;
	}
	else
		return RDM_DR_MT_UNKNOWN;

	return RDM_DR_MT_UNKNOWN;
}

const char* jsonGetRefreshStateText(cJSON* json)
{
	cJSON * jsonStateNode = cJSON_GetObjectItem(json, "State");
	if (jsonStateNode != NULL)
	{
		cJSON * jsonTextNode = cJSON_GetObjectItem(jsonStateNode, "Text");
		if (jsonTextNode != NULL)
		{
			const char* value = cJSON_GetStringValue(jsonTextNode);
			if (value != NULL)
			{
				return value;
			}
		}
	}

	return "";
}

RsslStreamStates jsonGetStreamState(cJSON* json)
{
	cJSON * jsonStateNode = cJSON_GetObjectItem(json, "State");
	if (jsonStateNode != NULL)
	{
		cJSON * jsonStreamNode = cJSON_GetObjectItem(jsonStateNode, "Stream");
		if (jsonStreamNode != NULL)
		{
			const char* value = cJSON_GetStringValue(jsonStreamNode);
			if (value != NULL)
			{
				if (strncmp(value, "Open", 4) == 0)
					return RSSL_STREAM_OPEN;
			}
		}
	}

	return RSSL_STREAM_UNSPECIFIED;
}

int jsonGetSupportOMMPost(cJSON* json)
{
	cJSON * jsonKeyNode = cJSON_GetObjectItem(json, "Key");
	if (jsonKeyNode != NULL)
	{
		cJSON * jsonElementsNode = cJSON_GetObjectItem(jsonKeyNode, "Elements");
		if (jsonElementsNode != NULL)
		{
			cJSON * jsonSupportOMMPostNode = cJSON_GetObjectItem(jsonElementsNode, "SupportOMMPost");
			if (jsonSupportOMMPostNode != NULL)
			{
				double value = cJSON_GetNumberValue(jsonSupportOMMPostNode);
				return (int)value;
			}
		}
	}

	return 0;
}

const char* jsonGetLoginRefreshApplicationName(cJSON* json)
{
	cJSON * jsonKeyNode = cJSON_GetObjectItem(json, "Key");
	if (jsonKeyNode != NULL)
	{
		cJSON * jsonElementsNode = cJSON_GetObjectItem(jsonKeyNode, "Elements");
		if (jsonElementsNode != NULL)
		{
			cJSON * jsonApplicationNameNode = cJSON_GetObjectItem(jsonElementsNode, "ApplicationName");
			if (jsonApplicationNameNode != NULL)
			{
				const char* value = cJSON_GetStringValue(jsonApplicationNameNode);
				if (value != NULL)
				{
					return value;
				}
			}
		}
	}

	return NULL;
}

RsslInt32 jsonGetStreamId(cJSON* json)
{
	cJSON * jsonStreamId = cJSON_GetObjectItem(json, "ID");

	if (jsonStreamId != NULL)
	{
		return (RsslInt32)cJSON_GetNumberValue(jsonStreamId);
	}

	return -1;
}

RsslMsgClasses jsonGetMsgClass(cJSON* json)
{
	cJSON * jsonMsgType = cJSON_GetObjectItem(json, "Type");

	if (jsonMsgType != NULL)
	{
		const char* str = "Refresh\"Update\"Status\"Generic\"Ping\"Pong";
		const char* value = cJSON_GetStringValue(jsonMsgType);
		const char* pos = strstr(str, value);
		if (pos != NULL)
		{
			switch (pos - str)
			{
			case 0:
				return RSSL_MC_REFRESH;
			case 8:
				return RSSL_MC_UPDATE;
			case 15:
				return RSSL_MC_STATUS;
			case 22:
				return RSSL_MC_GENERIC;
			case 30:
				return RSSL_MC_JSON_PING;
			case 35:
				return RSSL_MC_JSON_PONG;
			}
		}
		else
			return RSSL_MC_JSON_UNKNOWN;
	}
	else
		return RSSL_MC_JSON_UNKNOWN;

	return RSSL_MC_JSON_UNKNOWN;
}

RsslBool jsonIsFinalState(cJSON* json)
{
	RsslBool ret = RSSL_FALSE;

	cJSON * jsonStateNode = cJSON_GetObjectItem(json, "State");
	if (jsonStateNode != NULL)
	{
		cJSON * jsonStreamNode = cJSON_GetObjectItem(jsonStateNode, "Stream");
		if (jsonStreamNode != NULL)
		{
			const char* value = cJSON_GetStringValue(jsonStreamNode);
			if (value != NULL)
			{
				if (strncmp(value, "Open", 4) != 0)
					ret = RSSL_TRUE;
			}
		}
	}

	return ret;
}

const char* jsonGetMsgKeyName(cJSON* json)
{
	const char* name = NULL;

	cJSON * jsonKeyNode = cJSON_GetObjectItem(json, "Key");
	if (jsonKeyNode != NULL)
	{
		cJSON * jsonNameNode = cJSON_GetObjectItem(jsonKeyNode, "Name");
		if (jsonNameNode != NULL)
		{
			name = cJSON_GetStringValue(jsonNameNode);
		}
	}

	return name;
}

RsslBool jsonIsRefreshComplete(cJSON* json)
{
	cJSON * jsonCompleteNode = cJSON_GetObjectItem(json, "Complete");
	if (jsonCompleteNode == NULL)
	{
		return RSSL_TRUE;
	}
	else if (cJSON_IsTrue(jsonCompleteNode))
	{
		return RSSL_TRUE;
	}
	else if (cJSON_IsFalse(jsonCompleteNode))
	{ 
		return RSSL_FALSE;
	}
	else if (cJSON_IsString(jsonCompleteNode) &&
		strncmp(cJSON_GetStringValue(jsonCompleteNode), "false", 5) == 0)
	{
		return RSSL_FALSE;
	}
	else if (cJSON_IsNumber(jsonCompleteNode) && cJSON_GetNumberValue(jsonCompleteNode) == 0.)
	{
		return RSSL_FALSE;
	}
	else
	{
		return RSSL_TRUE;
	}
}

RsslBool jsonRefreshMsgDataStateOK(cJSON* json)
{
	RsslBool ret = RSSL_FALSE;

	cJSON * jsonStateNode = cJSON_GetObjectItem(json, "State");
	if (jsonStateNode != NULL)
	{
		cJSON * jsonDataNode = cJSON_GetObjectItem(jsonStateNode, "Data");
		if (jsonDataNode != NULL)
		{
			const char* value = cJSON_GetStringValue(jsonDataNode);
			if (value != NULL)
			{
				if (strncmp(value, "Ok", 2) == 0)
					ret = RSSL_TRUE;
			}
		}
	}

	return ret;
}
