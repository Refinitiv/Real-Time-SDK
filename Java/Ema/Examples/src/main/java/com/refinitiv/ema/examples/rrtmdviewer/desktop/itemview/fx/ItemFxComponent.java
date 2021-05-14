package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.fx;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.ItemNotificationModel;

public interface ItemFxComponent {
    String BRAND_BUTTON_STYLE = "brand-button";

    void handleItem(ItemNotificationModel notification);

    void handleMsgState(ItemNotificationModel notification);

    void updateUsingLastActualInfo();

    void clear();
}
