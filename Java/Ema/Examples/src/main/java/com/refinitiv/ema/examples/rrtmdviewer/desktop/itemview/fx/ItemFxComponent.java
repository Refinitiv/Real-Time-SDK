package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.fx;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.ItemNotificationModel;

public interface ItemFxComponent {
    void handleItem(ItemNotificationModel notification);

    void handleMsgState(ItemNotificationModel notification);

    void updateUsingLastActualInfo();

    void clear();
}
