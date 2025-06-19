/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview;

public class ItemRequestModel {

    private ItemViewModel view;
    private TabViewModel tabViewModel;
    private long id;

    public ItemViewModel getView() {
        return view;
    }

    public void setView(ItemViewModel view) {
        this.view = view;
    }

    public TabViewModel getTabViewModel() {
        return tabViewModel;
    }

    public void setTabViewModel(TabViewModel tabViewModel) {
        this.tabViewModel = tabViewModel;
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public ItemRequestModel(long id, ItemViewModel settings, TabViewModel tabViewModel) {
        this.id = id;
        this.tabViewModel = tabViewModel;
        this.view = settings;
    }

}
