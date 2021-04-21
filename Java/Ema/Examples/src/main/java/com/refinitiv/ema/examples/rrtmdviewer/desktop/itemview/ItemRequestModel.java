package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

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
