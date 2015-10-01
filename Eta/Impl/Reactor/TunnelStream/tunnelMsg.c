#include "rtr/tunnelMsg.h"


RsslRet tunnelRequestMsgEncode(RsslEncodeIterator *pIter,
		TunnelRequestMsg *pRequestMsg)
{
	RsslRequestMsg requestMsg;

	rsslClearRequestMsg(&requestMsg);

	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_PRIVATE_STREAM;

	requestMsg.msgBase.streamId = pRequestMsg->base.streamId;
	requestMsg.msgBase.domainType = pRequestMsg->base.domainType;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID
		| RSSL_MKF_HAS_NAME;

	requestMsg.msgBase.msgKey.serviceId = pRequestMsg->serviceId;
	requestMsg.msgBase.msgKey.name = pRequestMsg->name;

	return rsslEncodeMsg(pIter, (RsslMsg*)&requestMsg);
}
