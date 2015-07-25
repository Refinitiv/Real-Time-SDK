

#ifndef _RTR_RSSL_ITEM_HANDLER_H
#define _RTR_RSSL_ITEM_HANDLER_H

#include "rsslNIProvider.h"
#include "rsslItemEncode.h"

#ifdef __cplusplus
extern "C" {
#endif

void initItemHandler();
void resetRefreshComplete();
void addItemName(const char* itemname, RsslUInt8 domainType);
void updateItemInfo();
RsslRet sendItemUpdates(RsslChannel* chnl);
void sendItemCloseStatusMsgs(RsslChannel* chnl);

#ifdef __cplusplus
};
#endif

#endif

