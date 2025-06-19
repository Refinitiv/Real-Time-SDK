/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx;

import com.refinitiv.ema.examples.rtviewer.desktop.itemview.ItemNotificationModel;

public interface ItemFxComponent {
    String BRAND_BUTTON_STYLE = "brand-button";

    void handleItem(ItemNotificationModel notification);

    void handleMsgState(ItemNotificationModel notification);

    void updateUsingLastActualInfo();

    void clear();
}
