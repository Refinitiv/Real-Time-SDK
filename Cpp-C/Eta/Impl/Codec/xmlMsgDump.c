/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#include "xmlMsgDump.h"
#include "xmlDump.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslMsgDecoders.h"
#include "rtr/rsslRDM.h"
#include "decodeRoutines.h"


extern int indents;


void xmlDumpExtendedHeader(FILE * file, const RsslBuffer * header)
{
	encodeindents(file);
	fprintf(file, "<extendedHeader data=\"");
	xmlDumpHexBuffer(file, header);
	fprintf(file, "\"/>\n");
}

void xmlDumpKeyBeginInternal(FILE * file, const RsslMsgKey * key)
{
	RsslBool firstFlag = RSSL_TRUE;

	/* print out flags */
	fprintf ( file, " flags=\"0x%X", key->flags);
	
	if (key->flags != 0)
		fputs(" (", file);
	if (key->flags & RSSL_MKF_HAS_SERVICE_ID)
	{
		fprintf(file, "RSSL_MKF_HAS_SERVICE_ID");
		firstFlag = RSSL_FALSE;
	}
	if (key->flags & RSSL_MKF_HAS_NAME)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MKF_HAS_NAME");
	}
	if (key->flags & RSSL_MKF_HAS_NAME_TYPE)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MKF_HAS_NAME_TYPE");
	}
	if (key->flags & RSSL_MKF_HAS_FILTER)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MKF_HAS_FILTER");
	}
	if (key->flags & RSSL_MKF_HAS_IDENTIFIER)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MKF_HAS_IDENTIFIER");
	}
	if (key->flags & RSSL_MKF_HAS_ATTRIB)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MKF_HAS_ATTRIB");
	}

	if (key->flags != 0)
		fputc(')', file);
	fputc('"', file);
	fputc(' ', file);

	if (key->flags & RSSL_MKF_HAS_SERVICE_ID)
		fprintf(file, " serviceId=\"%d\"", key->serviceId);

	if (key->flags & RSSL_MKF_HAS_NAME)
	{
		fprintf(file, " name=\"");
 		xmlDumpBuffer(file, &key->name);
		fprintf(file, "\"");
	}
	
	if (key->flags & RSSL_MKF_HAS_NAME_TYPE)
	{
		fprintf(file, " nameType=\"%d\"", key->nameType);
	}
	
	if (key->flags & RSSL_MKF_HAS_FILTER)
	{
		fprintf(file, " filter=\"%u\"", key->filter);
	}
		
	if (key->flags & RSSL_MKF_HAS_IDENTIFIER)
		fprintf(file, " identifier=\"%d\"", key->identifier);

	if (key->flags & RSSL_MKF_HAS_ATTRIB)
	{
		fprintf(file, " attribContainerType=\"");
		xmlDumpDataType(file, key->attribContainerType);
		fprintf(file, "\">\n");
		indents++;
		encodeindents(file);
	}
	else
		fprintf(file, "/>\n");
}

void xmlDumpReqKeyBegin(FILE * file, const RsslMsgKey * key)
{
	encodeindents(file);
	fprintf(file, "<requestKey ");
	xmlDumpKeyBeginInternal(file, key);
}

void xmlDumpKeyBegin(FILE * file, const RsslMsgKey * key)
{
	encodeindents(file);
	fprintf(file, "<key ");
	xmlDumpKeyBeginInternal(file, key);
}

void xmlDumpKeyEnd(FILE * file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</key>\n");	
}

void xmlDumpReqKeyEnd(FILE * file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</requestKey>\n");	
}

void xmlDumpMessageClass(FILE * file, RsslUInt8 msgClass)
{
	const char * str = rsslMsgClassToString(msgClass);
	if (!str)
		fprintf(file, "%d", msgClass);
	else
		fprintf(file, "%s", str);
}

void xmlDumpDomainType(FILE * file, RsslUInt8 domainType)
{
	/* if 'Unknown' is returned, dump the number */
	const char * str = rsslDomainTypeToString(domainType);
	if (str[0] == 'U')
		fprintf(file, "%d", domainType);
	else
		fprintf(file, "%s", str);
}

void xmlDumpUpdateType(FILE * file, RsslUInt8 updateType)
{
	char *updateString;
	switch(updateType)
	{
		case RDM_UPD_EVENT_TYPE_UNSPECIFIED:
			updateString = "RDM_UPD_EVENT_TYPE_UNSPECIFIED";
			break;
		case RDM_UPD_EVENT_TYPE_QUOTE:
			updateString = "RDM_UPD_EVENT_TYPE_QUOTE";
			break;
		case RDM_UPD_EVENT_TYPE_TRADE:
			updateString = "RDM_UPD_EVENT_TYPE_TRADE";
			break;
		case RDM_UPD_EVENT_TYPE_NEWS_ALERT:
			updateString = "RDM_UPD_EVENT_TYPE_NEWS_ALERT";
			break;
		case RDM_UPD_EVENT_TYPE_VOLUME_ALERT:
			updateString = "RDM_UPD_EVENT_TYPE_VOLUME_ALERT";
			break;
		case RDM_UPD_EVENT_TYPE_ORDER_INDICATION:
			updateString = "RDM_UPD_EVENT_TYPE_ORDER_INDICATION";
			break;
		case RDM_UPD_EVENT_TYPE_CLOSING_RUN:
			updateString = "RDM_UPD_EVENT_TYPE_CLOSING_RUN";
			break;
		case RDM_UPD_EVENT_TYPE_CORRECTION:
			updateString = "RDM_UPD_EVENT_TYPE_CORRECTION";
			break;
		case RDM_UPD_EVENT_TYPE_MARKET_DIGEST:
			updateString = "RDM_UPD_EVENT_TYPE_MARKET_DIGEST";
			break;
		case RDM_UPD_EVENT_TYPE_QUOTES_TRADE:
			updateString = "RDM_UPD_EVENT_TYPE_QUOTES_TRADE";
			break;
		case RDM_UPD_EVENT_TYPE_MULTIPLE:
			updateString = "RDM_UPD_EVENT_TYPE_MULTIPLE";
			break;
		case RDM_UPD_EVENT_TYPE_VERIFY:
			updateString = "RDM_UPD_EVENT_TYPE_VERIFY";
			break;
		default:
			fprintf(file, " updateType=\"%d\"", updateType);
			return;
	}

	fprintf(file, " updateType=\"%d (%s)\"", updateType, updateString);
}

void xmlDumpMsgBegin(FILE * file, const RsslMsg * msg, const char* tagName)
{
	char postUserAddrString[16];
	RsslBool firstFlag = RSSL_TRUE;

	encodeindents(file);
	indents++;
	fprintf(file, "<%s", tagName );
	fprintf(file, " domainType=\"");
	xmlDumpDomainType(file, msg->msgBase.domainType);
	fprintf(file, "\" streamId=\"%d\" containerType=\"", msg->msgBase.streamId);
	xmlDumpDataType(file, msg->msgBase.containerType);
	switch ( msg->msgBase.msgClass )
	{
		case RSSL_MC_UPDATE:
			fprintf(file, "\" flags=\"0x%X", msg->updateMsg.flags);
			
			if (msg->updateMsg.flags != 0)
				fputs(" (", file);
			if (msg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_UPMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_HAS_PERM_DATA");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_HAS_MSG_KEY");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_HAS_SEQ_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_HAS_SEQ_NUM");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_HAS_CONF_INFO)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_HAS_CONF_INFO");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_DO_NOT_CACHE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_DO_NOT_CACHE");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_DO_NOT_CONFLATE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_DO_NOT_CONFLATE");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_DO_NOT_RIPPLE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_DO_NOT_RIPPLE");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_HAS_POST_USER_INFO)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_HAS_POST_USER_INFO");
			}
			if (msg->updateMsg.flags & RSSL_UPMF_DISCARDABLE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_UPMF_DISCARDABLE");
			}

			if (msg->updateMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);

			xmlDumpUpdateType(file, msg->updateMsg.updateType);
			if (msg->updateMsg.flags & RSSL_UPMF_HAS_SEQ_NUM)
			{
				fprintf(file, " seqNum=\"%u\"", msg->updateMsg.seqNum);
			}

			if (msg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA)
			{
				fprintf(file, " permData=\"");
				xmlDumpHexBuffer(file, &msg->updateMsg.permData);
				fprintf(file, "\"");
			}

			if (msg->updateMsg.flags & RSSL_UPMF_HAS_CONF_INFO)
			{
				fprintf(file, " conflationCount=\"%d\" conflationTime=\"%d\"", msg->updateMsg.conflationCount, msg->updateMsg.conflationTime);
			}

			if (msg->updateMsg.flags & RSSL_UPMF_HAS_POST_USER_INFO)
			{
				rsslIPAddrUIntToString(msg->updateMsg.postUserInfo.postUserAddr, postUserAddrString);
				fprintf(file, "postUserId=\"%u\" postUserAddr=\"%s\"", msg->updateMsg.postUserInfo.postUserId, postUserAddrString);
			}
			break;
	
		case RSSL_MC_GENERIC:
			fprintf(file, "\" flags=\"0x%X", msg->genericMsg.flags);
			
			if (msg->genericMsg.flags != 0)
				fputs(" (", file);
			if (msg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_GNMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_GNMF_HAS_PERM_DATA");
			}
			if (msg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_GNMF_HAS_MSG_KEY");
			}
			if (msg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_GNMF_HAS_REQ_MSG_KEY");
			}
			if (msg->genericMsg.flags & RSSL_GNMF_HAS_SEQ_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_GNMF_HAS_SEQ_NUM");
			}
			if (msg->genericMsg.flags & RSSL_GNMF_MESSAGE_COMPLETE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_GNMF_MESSAGE_COMPLETE");
			}
			if (msg->genericMsg.flags & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_GNMF_HAS_SECONDARY_SEQ_NUM");
			}
			if (msg->genericMsg.flags & RSSL_GNMF_HAS_PART_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_GNMF_HAS_PART_NUM");
			}
			if (msg->genericMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);

			if (msg->genericMsg.flags & RSSL_GNMF_HAS_SEQ_NUM)
			{
				fprintf(file, " seqNum=\"%u\"", msg->genericMsg.seqNum);
			}

			if (msg->genericMsg.flags & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM)
			{
				fprintf(file, " secondarySeqNum=\"%u\"", msg->genericMsg.secondarySeqNum);
			}

			if (msg->genericMsg.flags & RSSL_GNMF_HAS_PART_NUM)
			{
				fprintf(file, " partNum=\"%u\"", msg->genericMsg.partNum);
			}

			if (msg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA)
			{
				fprintf(file, " permData=\"");
				xmlDumpHexBuffer(file, &msg->genericMsg.permData);
				fprintf(file, "\"");
			}
			break;

		case RSSL_MC_REFRESH:
			fprintf ( file, "\" flags=\"0x%X", msg->refreshMsg.flags); 

			if (msg->refreshMsg.flags != 0)
				fputs(" (", file);
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_RFMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_HAS_PERM_DATA");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_HAS_MSG_KEY");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_HAS_REQ_MSG_KEY");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_HAS_SEQ_NUM");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_SOLICITED)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_SOLICITED");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_REFRESH_COMPLETE");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_QOS)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_HAS_QOS");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_CLEAR_CACHE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_CLEAR_CACHE");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_DO_NOT_CACHE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_DO_NOT_CACHE");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_PRIVATE_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_PRIVATE_STREAM");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_POST_USER_INFO)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_HAS_POST_USER_INFO");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_PART_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_HAS_PART_NUM");
			}
			if (msg->refreshMsg.flags & RSSL_RFMF_QUALIFIED_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RFMF_QUALIFIED_STREAM");
			}
			if (msg->refreshMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);

			fprintf ( file, " groupId=\"");
			xmlDumpGroupId(file, &msg->refreshMsg.groupId);
			fprintf(file, "\"");

			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
			{
				fprintf(file, " seqNum=\"%u\"", msg->refreshMsg.seqNum);
			}

			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_PART_NUM)
			{
				fprintf(file, " partNum=\"%u\"", msg->refreshMsg.partNum);
			}

		    if (msg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA)
			{
				fprintf(file, " permData=\"");
				xmlDumpHexBuffer(file, &msg->refreshMsg.permData);
				fprintf(file, "\"");
			}

			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_QOS)
			{
				xmlDumpQos(file, &msg->refreshMsg.qos);
			}

			xmlDumpState(file, &msg->refreshMsg.state);
		
			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_POST_USER_INFO)
			{
				rsslIPAddrUIntToString(msg->refreshMsg.postUserInfo.postUserAddr, postUserAddrString);
				fprintf(file, "postUserId=\"%u\" postUserAddr=\"%s\"", msg->refreshMsg.postUserInfo.postUserId, postUserAddrString);
			}
			break;

		case RSSL_MC_POST:
			fprintf(file, "\" flags=\"0x%X", msg->postMsg.flags);

			if (msg->postMsg.flags != 0)
				fputs(" (", file);
			if (msg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_PSMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->postMsg.flags & RSSL_PSMF_HAS_POST_ID)
			{
				if (!firstFlag)
					fputc('|', file);
				else			firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_HAS_POST_ID");
			}
			if (msg->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_HAS_MSG_KEY");
			}
			if (msg->postMsg.flags & RSSL_PSMF_HAS_SEQ_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_HAS_SEQ_NUM");
			}
			if (msg->postMsg.flags & RSSL_PSMF_POST_COMPLETE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_POST_COMPLETE");
			}
			if (msg->postMsg.flags & RSSL_PSMF_ACK)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_ACK");
			}
			if (msg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_HAS_PERM_DATA");
			}
			if (msg->postMsg.flags & RSSL_PSMF_HAS_PART_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_HAS_PART_NUM");
			}
			if (msg->postMsg.flags & RSSL_PSMF_HAS_POST_USER_RIGHTS)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_PSMF_HAS_POST_USER_RIGHTS");
			}
			if (msg->postMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);
			
			if (msg->postMsg.flags & RSSL_PSMF_HAS_SEQ_NUM)
			{
				fprintf(file, " seqNum=\"%u\"", msg->postMsg.seqNum);
			}

			if (msg->postMsg.flags & RSSL_PSMF_HAS_POST_ID)
			{
				fprintf(file, " postId=\"%u\"", msg->postMsg.postId);
			}
			
			if (msg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA)
			{
				fprintf(file, " permData=\"");
				xmlDumpHexBuffer(file, &msg->postMsg.permData);
				fprintf(file, "\"");
			}

			if (msg->postMsg.flags & RSSL_PSMF_HAS_PART_NUM)
			{
				fprintf(file, " partNum=\"%u\"", msg->postMsg.partNum);
			}

			if (msg->postMsg.flags & RSSL_PSMF_HAS_POST_USER_RIGHTS)
			{
				fprintf(file, " postUserRights=\"%u\"", msg->postMsg.postUserRights);
			}
			
			/* print user info */
			rsslIPAddrUIntToString(msg->postMsg.postUserInfo.postUserAddr, postUserAddrString);
			fprintf(file, " postUserId=\"%u\" postUserAddr=\"%s\"", msg->postMsg.postUserInfo.postUserId, postUserAddrString);
			break;
	
		case RSSL_MC_REQUEST:
			fprintf ( file, "\" flags=\"0x%X", msg->requestMsg.flags);

			if (msg->requestMsg.flags != 0)
				fputs(" (", file);
			if (msg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_RQMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->requestMsg.flags & RSSL_RQMF_HAS_PRIORITY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_HAS_PRIORITY");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_STREAMING)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_STREAMING");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_MSG_KEY_IN_UPDATES)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_MSG_KEY_IN_UPDATES");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_CONF_INFO_IN_UPDATES)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_CONF_INFO_IN_UPDATES");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_NO_REFRESH)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_NO_REFRESH");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_HAS_QOS)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_HAS_QOS");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_HAS_WORST_QOS)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_HAS_WORST_QOS");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_PRIVATE_STREAM");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_PAUSE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_PAUSE");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_HAS_VIEW)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_HAS_VIEW");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_HAS_BATCH)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_HAS_BATCH");
			}
			if (msg->requestMsg.flags & RSSL_RQMF_QUALIFIED_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_RQMF_QUALIFIED_STREAM");
			}
			if (msg->requestMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);

			if (msg->requestMsg.flags & RSSL_RQMF_HAS_QOS)
			{
				xmlDumpQos(file, &msg->requestMsg.qos);
			}
			if (msg->requestMsg.flags & RSSL_RQMF_HAS_WORST_QOS)
			{
				xmlDumpWorstQos(file, &msg->requestMsg.worstQos);
			}

			if (msg->requestMsg.flags & RSSL_RQMF_HAS_PRIORITY)
			{
				fprintf(file, " priorityClass=\"%d\" priorityCount=\"%d\"", msg->requestMsg.priorityClass, msg->requestMsg.priorityCount);
			}
			break;

		case RSSL_MC_STATUS:
			fprintf(file, "\" flags=\"0x%X", msg->statusMsg.flags);
					  				 
			if (msg->statusMsg.flags != 0)
				fputs(" (", file);
			if (msg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_STMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_HAS_PERM_DATA");
			}
			if (msg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_HAS_MSG_KEY");
			}
			if (msg->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_HAS_REQ_MSG_KEY");
			}
			if (msg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_HAS_GROUP_ID");
			}
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_HAS_STATE");
			}
			if (msg->statusMsg.flags & RSSL_STMF_CLEAR_CACHE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_CLEAR_CACHE");
			}
			if (msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_PRIVATE_STREAM");
			}
			if (msg->statusMsg.flags & RSSL_STMF_HAS_POST_USER_INFO)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_HAS_POST_USER_INFO");
			}
			if (msg->statusMsg.flags & RSSL_STMF_QUALIFIED_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_STMF_QUALIFIED_STREAM");
			}
			if (msg->statusMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);

			if (msg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID)
			{
				fprintf(file, " groupId=\"");
				xmlDumpGroupId(file, &msg->statusMsg.groupId);
				fprintf(file, "\"");
			}
		
			if (msg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA)
			{
				fprintf(file, " permData=\"");
		  		xmlDumpHexBuffer(file, &msg->statusMsg.permData);
				fprintf(file, "\"");
			}

			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				xmlDumpState(file, &msg->statusMsg.state);
			}
		
			if (msg->statusMsg.flags & RSSL_STMF_HAS_POST_USER_INFO)
			{
				rsslIPAddrUIntToString(msg->statusMsg.postUserInfo.postUserAddr, postUserAddrString);
				fprintf(file, "postUserId=\"%u\" postUserAddr=\"%s\"", msg->statusMsg.postUserInfo.postUserId, postUserAddrString);
			}
			break;

		case RSSL_MC_CLOSE:
			fprintf(file, "\" flags=\"0x%X", msg->closeMsg.flags);
			if (msg->closeMsg.flags != 0)
				fputs(" (", file);
			if (msg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_CLMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->closeMsg.flags & RSSL_CLMF_ACK)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_CLMF_ACK");
			}

			if (msg->closeMsg.flags & RSSL_CLMF_HAS_BATCH)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_CLMF_HAS_BATCH");
			}

			if (msg->closeMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);
			break;

		case RSSL_MC_ACK:
			fprintf(file, "\" flags=\"0x%X", msg->ackMsg.flags);

			if (msg->ackMsg.flags != 0)
				fputs(" (", file);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER)
			{
				fprintf(file, "RSSL_AKMF_HAS_EXTENDED_HEADER");
				firstFlag = RSSL_FALSE;
			}
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_AKMF_HAS_TEXT");
			}
			if (msg->ackMsg.flags & RSSL_AKMF_PRIVATE_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_AKMF_PRIVATE_STREAM");
			}
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_AKMF_HAS_SEQ_NUM");
			}
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_AKMF_HAS_MSG_KEY");
			}
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_AKMF_HAS_NAK_CODE");
			}
			if (msg->ackMsg.flags & RSSL_AKMF_QUALIFIED_STREAM)
			{
				if (!firstFlag)
					fputc('|', file);
				else
					firstFlag = RSSL_FALSE;
				fprintf(file, "RSSL_AKMF_QUALIFIED_STREAM");
			}
			if (msg->ackMsg.flags != 0)
				fputc(')', file);
			fputc('"', file);

			fprintf(file, " ackId=\"%u\"", msg->ackMsg.ackId);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
			{
				fprintf(file, " nakCode=\"%s\"", getNakCodeAsString(msg->ackMsg.nakCode) );
			}

			if (msg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
			{
				fprintf(file, " text=\"");
				xmlDumpBuffer(file, &msg->ackMsg.text);
				fprintf(file, "\"");
			}

			if (msg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
			{
				fprintf(file, " seqNum=\"%u\"", msg->ackMsg.seqNum);
			}
			break;

		default:
			fprintf(file, "\"");
		}
	fprintf(file, " dataSize=\"%u\">\n", msg->msgBase.encDataBody.length);
}

void xmlDumpMsgEnd(FILE * file, const char *tagName, RsslBool isNested)
{
	indents--;
	encodeindents(file);
	if (isNested)
	{
		fprintf(file, "</%s>\n", tagName);
	}
	else
	{
		fprintf(file, "</%s>\n\n", tagName);
	}
}
