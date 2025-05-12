/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview;

public interface ItemViewService {

    void loadServiceData(ServiceInfoResponseModel serviceInfoResponse);

    void sendItemRequest(ItemRequestModel request);

    void unregister(long handle);
}
