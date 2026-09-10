/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.examples.rtviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rtviewer.desktop.application.GlobalApplicationSettings;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ChannelInformationClient;
import com.refinitiv.ema.examples.rtviewer.desktop.common.DebugAreaStream;
import com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents.ScrollableTextField;
import com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx.ItemFxComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx.MarketByComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx.MarketByOrderComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx.MarketByPriceComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx.MarketPriceComponent;
import javafx.application.Platform;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.concurrent.Task;
import javafx.concurrent.WorkerStateEvent;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.geometry.Rectangle2D;
import javafx.scene.control.*;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.stage.Screen;

import java.util.*;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicLong;
import java.util.stream.Collectors;


public class ItemViewController {

    private static final int DEBUG_LOWER_HEIGHT = 0;

    private static final double DEFAULT_PREF_HEIGHT = 650;
    private static final double DEFAULT_PREF_WIDTH = 1250;

    private static final double DEFAULT_RATIO = 0.75;

    private boolean hasService = false;
    private boolean hasDomain = false;

    @FXML
    private ComboBox<SupportedItemDomains> domainCombobox;

    @FXML
    private ComboBox<ServiceInfoModel> serviceCombobox;

    @FXML
    private VBox marketByContent;

    @FXML
    private VBox marketPriceContent;

    @FXML
    private TextArea debugTextArea;

    @FXML
    private CheckBox debugCheckbox;

    @FXML
    private TabPane contentPane;

    @FXML
    private ScrollableTextField ricMarketBy;

    @FXML
    private ScrollableTextField ricMarketPrice;

    @FXML
    private ListView<String> availableFields;

    @FXML
    private ListView<String> selectedFields;

    @FXML
    private CheckBox snapshotCheckbox;

    @FXML
    private Button submitButton;

    @FXML
    private VBox emptyContent;

    @FXML
    private VBox submitVBox;

    @FXML
    private TextField searchField;

    @FXML
    private Label connectionStatus;

    @FXML
    private ScrollPane scrollPane;

    private StringProperty connectionStatusProperty = new SimpleStringProperty(this, this.getClass().getName());

    private ExecutorService executorService = ApplicationSingletonContainer.getBean(ExecutorService.class);
    private ChannelInformationClient channelInformationClient;

    private final ItemViewService service = new ItemViewServiceImpl();

    private final ServiceInfoResponseModel serviceInfoResponse = new ServiceInfoResponseModel();

    private final TabViewModel tabViewModel = new TabViewModel();

    private Map<String, Integer> fields;

    private Map<Long, ItemFxComponent> componentTabs;
    private Map<Long, Integer> pendingToUnregister;

    private AtomicLong tabIdCounter = new AtomicLong(0);

    private long getTabId() {
        return tabIdCounter.getAndIncrement();
    }

    @FXML
    public void initialize() {

        SceneController sceneController = ApplicationSingletonContainer.getBean(SceneController.class);

        sceneController.getPrimaryStage().getScene().getWindow().widthProperty().addListener(e ->{
            contentPane.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get() - submitVBox.getWidth() - 30);
        });
        sceneController.getPrimaryStage().getScene().getWindow().heightProperty().addListener(e ->{
            if (debugCheckbox.isSelected()) {
                setDebugAreaSize();
            }
        });

        Rectangle2D screenBounds = Screen.getPrimary().getBounds();
        scrollPane.setPrefHeight(Math.min(screenBounds.getMaxY() * DEFAULT_RATIO, DEFAULT_PREF_HEIGHT));
        scrollPane.setPrefWidth(Math.min(screenBounds.getMaxX() * DEFAULT_RATIO, DEFAULT_PREF_WIDTH));

        contentPane.getSelectionModel().selectedItemProperty().addListener(this::handleTabChanging);

        componentTabs = new HashMap<>();
        pendingToUnregister = new HashMap<>();

        ricMarketPrice.textProperty().addListener((obj, oldVal, newVal) -> checkSubmitButton());
        ricMarketBy.textProperty().addListener((obj, oldVal, newVal) -> checkSubmitButton());
        searchField.textProperty().addListener((obj, oldVal, newVal) -> filterAvailableFields(newVal));

        serviceInfoResponse.dictionaryLoadedProperty().addListener((observable, oldValue, newValue) -> setDictionaryData(serviceInfoResponse));
        serviceInfoResponse.sourceRefreshCompletedProperty().addListener((observable, oldValue, newValue) -> handleServiceRefresh(serviceInfoResponse));

        tabViewModel.messageCounterProperty().addListener((observable, oldValue, newValue) -> {
            final IntegerProperty property = (IntegerProperty) observable;
            final TabViewModel tabViewModel = (TabViewModel) property.getBean();
            final ItemNotificationModel itemNotification = Objects.requireNonNull(tabViewModel.getNotificationQueue().poll());
            handleMessage(itemNotification);
        });

        if(channelInformationClient == null) {
            channelInformationClient = ApplicationSingletonContainer.getBean(ChannelInformationClient.class);
            connectionStatus.textProperty().bind(connectionStatusProperty);

            connectionStatusProperty.setValue(channelInformationClient.toString());
        }

        tabViewModel.getConnectionProperty().addListener(observable -> {
            Platform.runLater(() -> {

                if(channelInformationClient.getChannelState() == ChannelInformation.ChannelState.ACTIVE) {
                    connectionStatus.setStyle("-fx-font-size: 11px; -fx-text-background-color: blue; -fx-font-weight: BOLD;");
                }
                else
                {
                    connectionStatus.setStyle("-fx-font-size: 11px; -fx-text-background-color: red; -fx-font-weight: BOLD;");
                }

                connectionStatusProperty.setValue(channelInformationClient.toString());
            });
        });

        channelInformationClient.setTabViewModel(tabViewModel);

        service.loadServiceData(serviceInfoResponse);
    }

    private void setDebugAreaSize() {
        SceneController sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
        debugTextArea.setPrefHeight(Math.max(sceneController.getPrimaryStage().getScene().getWindow().getHeight() - contentPane.getHeight() - 80, 0));
        debugTextArea.setMaxHeight(Math.max(sceneController.getPrimaryStage().getScene().getWindow().getHeight() - contentPane.getHeight() - 80, 0));
    }

    private void filterAvailableFields(String value) {
        availableFields.setItems(FXCollections.observableArrayList(fields.keySet().stream()
                .filter(v -> v.startsWith(value))
                .collect(Collectors.toList())));
    }

    private void handleServiceRefresh(ServiceInfoResponseModel response) {
        Platform.runLater(() -> {
            serviceCombobox.getSelectionModel().clearSelection();
            hasService = false;
            domainCombobox.getSelectionModel().clearSelection();
            domainCombobox.getItems().clear();
            onSelectDomain(null);

            response.acquireServiceInfoLock();
            serviceCombobox.setItems(FXCollections.observableArrayList(response.getServiceInfoMap()
                    .values().stream()
                    .filter(v -> v.addToList())
                    .map(v -> v.copy())
                    .collect(Collectors.toList())));
            response.releaseServiceInfoLock();
        });
    }

    private void handleMessage(ItemNotificationModel message) {
        if ((message.getMsg() instanceof RefreshMsg) && pendingToUnregister.containsKey(message.getRequestHandle())) { /* Check if the message is a response to batch request that was unregistered */
            int count = pendingToUnregister.get(message.getRequestHandle());
            if (count > 1) {
                pendingToUnregister.put(message.getRequestHandle(), count - 1);
            } else {
                pendingToUnregister.remove(message.getRequestHandle());
            }
            unregister(message.getHandle());
        } else {
            if (!componentTabs.containsKey(message.getRequest().getId())) {
                setupTab(message.getRequestHandle(), message);
            } else {
                processMessage(message);
            }
        }
    }

    private void processMessage(ItemNotificationModel notification) {

        Msg msg = notification.getMsg();
        ItemFxComponent component = componentTabs.get(notification.getRequest().getId());
        if ((msg instanceof RefreshMsg) || (msg instanceof UpdateMsg)) {
            component.handleItem(notification);
        } else if (msg instanceof StatusMsg) {
            Platform.runLater(() -> component.handleMsgState(notification));
        }
    }

    private void setDictionaryData(ServiceInfoResponseModel response) {
        Platform.runLater(() -> {
            fields = response.getFids();
            availableFields.setItems(FXCollections.observableArrayList(fields.keySet()));
        });
    }

    private void submitRequestTask(ItemRequestModel request) {
        Task<Void> requestTask = new Task<Void>() {
            @Override
            protected Void call() {
                service.sendItemRequest(request);
                return null;
            }
        };
        requestTask.addEventHandler(WorkerStateEvent.WORKER_STATE_FAILED, e -> {
            Platform.runLater(() -> {
                debugTextArea.appendText("Item Request submission failed.\n"
                        + e.getSource().getException() != null ? e.getSource().getException().getMessage() : "Exception occurred during worker thread execution.");
            });
        });

        executorService.submit(requestTask);
    }

    @FXML
    public void onSubmitBtnPressed(ActionEvent event) {

        ItemViewModel params = getItemRequestSettings();
        ItemRequestModel request = new ItemRequestModel(getTabId(), params, tabViewModel);
        submitRequestTask(request);
    }

    private ItemViewModel getItemRequestSettings() {
        ItemViewModel.ItemViewModelBuilder modelBuilder = ItemViewModel.builder();
        modelBuilder.serviceName(serviceCombobox.getSelectionModel().getSelectedItem().getServiceName());
        SupportedItemDomains domain = domainCombobox.getSelectionModel().getSelectedItem();
        modelBuilder.domain(domain);
        switch (domain) {
            case MARKET_PRICE:
                String[] rics = ricMarketPrice.getText().split("\\s*,\\s*");
                if (rics.length == 1) {
                    modelBuilder.isBatch(false);
                    modelBuilder.RIC(rics[0]);
                } else {
                    modelBuilder.isBatch(true);
                    modelBuilder.batchRICs(
                            Arrays.stream(rics)
                                    .filter(s -> !s.trim().isEmpty())
                                    .collect(Collectors.toList())
                    );
                }
                ObservableList<String> items = selectedFields.getItems();
                if (items.size() > 0) {
                    modelBuilder.hasView(true);
                    List<Integer> view = new LinkedList<>();
                    for (String item : items) {
                        view.add(fields.get(item));
                    }
                    modelBuilder.marketPriceView(view);
                }
                break;
            case MARKET_BY_ORDER:
            case MARKET_BY_PRICE:
                modelBuilder.RIC(ricMarketBy.getText());
                break;
            default:
                break;
        }
        if (snapshotCheckbox.isSelected()) {
            modelBuilder.isSnapshot(true);
        }

        return modelBuilder.build();
    }

    private void setupTab(long handle, ItemNotificationModel notification) {
        final ItemRequestModel request = notification.getRequest();
        Tab tab = new Tab();
        tab.setText(getTabName(request.getView()));
        switch (request.getView().getDomain()) {
            case MARKET_PRICE:
                setupMarketPriceTab(tab, handle, request);
                break;
            case MARKET_BY_ORDER:
            case MARKET_BY_PRICE:
                setupMarketByTab(tab, handle, request);
                break;
            default:
                break;
        }
        ((Pane) tab.getContent()).prefWidthProperty().bind(contentPane.widthProperty());
        componentTabs.put(request.getId(), (ItemFxComponent) tab.getContent());
        Platform.runLater(() -> {
            contentPane.getTabs().add(tab);
            contentPane.getSelectionModel().select(tab);
            processMessage(notification);
        });
    }

    private void setupMarketPriceTab(Tab tab, long handle, ItemRequestModel request) {
        MarketPriceComponent mp = new MarketPriceComponent(tab);
        if (request.getView().isSnapshot()) {
            mp.setDisableBtn(true);
            if (request.getView().isBatch()) {
                mp.setNumOfRefreshes(request.getView().getBatchRICs().size());
            }
            mp.configureButton("Refresh", e -> {
                mp.clear();
                submitRequestTask(request);
            });
        } else {
            mp.configureButton("Unregister", e -> {
                Set<Long> currentHandles = mp.getCurrentHandles();
                if (request.getView().isBatch() && (currentHandles.size() < request.getView().getBatchRICs().size())) {
                    pendingToUnregister.put(handle, (request.getView().isBatch() ? request.getView().getBatchRICs().size() : 1) - currentHandles.size());
                }
                mp.setDisableBtn(true);
                unregister(currentHandles);
            });
        }
        tab.setContent(mp);
        tab.setOnClosed(e -> {
            Set<Long> currentHandles = mp.getCurrentHandles();
            if (request.getView().isBatch() && (currentHandles.size() < request.getView().getBatchRICs().size())) {
                pendingToUnregister.put(handle, (request.getView().isBatch() ? request.getView().getBatchRICs().size() : 1) - currentHandles.size());
            }
            unregister(currentHandles);
            componentTabs.remove(request.getId());
        });
    }

    private void setupMarketByTab(Tab tab, long handle, ItemRequestModel itemRequestModel) {
        MarketByComponent mb = itemRequestModel.getView().getDomain() == SupportedItemDomains.MARKET_BY_ORDER ?
                new MarketByOrderComponent(tab) : new MarketByPriceComponent(tab) ;
        if (itemRequestModel.getView().isSnapshot()) {
            mb.setDisableBtn(true);
            mb.configureButton("Refresh", e -> {
                mb.clear();
                submitRequestTask(itemRequestModel);
            });
        } else {
            mb.configureButton("Unregister", e -> {
                mb.setDisableBtn(true);
                unregister(handle);
            });
        }

        tab.setOnClosed(e -> {
            unregister(handle);
            componentTabs.remove(itemRequestModel.getId());
        });
    }

    private void unregister(long handle) {
        Task<Void> unregisterTask = new Task<Void>() {

            @Override
            protected Void call() {
                service.unregister(handle);
                return null;
            }
        };

        executorService.submit(unregisterTask);
    }

    private void unregister(Set<Long> handles) {
        Task<Void> unregisterTask = new Task<Void>() {

            @Override
            protected Void call() {
                for (Long handle : handles) {
                    service.unregister(handle);
                }
                return null;
            }
        };

        executorService.submit(unregisterTask);
    }

    @FXML
    public void onDebugChanged(ActionEvent event) {
        processDebug();
    }

    private void processDebug() {
        final DebugAreaStream das = ApplicationSingletonContainer.getBean(DebugAreaStream.class);
        if (debugCheckbox.isSelected()) {
            debugTextArea.setVisible(true);
            setDebugAreaSize();
            das.enable(debugTextArea);
        } else {
            das.disable();
            debugTextArea.setVisible(false);
            debugTextArea.maxHeight(DEBUG_LOWER_HEIGHT);
        }
    }

    @FXML
    public void onSelectDomain(ActionEvent event) {

        SupportedItemDomains selected = domainCombobox.getSelectionModel().getSelectedItem();
        if (selected == null || selected.toString().equals("")) {
            emptyContent.setVisible(true);
            marketByContent.setVisible(false);
            marketPriceContent.setVisible(false);
            hasDomain = false;
            submitButton.setDisable(true);
            return;
        }
        switch (selected) {
            case MARKET_PRICE:
                marketByContent.setVisible(false);
                marketPriceContent.setVisible(true);
                emptyContent.setVisible(false);
                break;
            case MARKET_BY_ORDER:
            case MARKET_BY_PRICE:
                marketByContent.setVisible(true);
                marketPriceContent.setVisible(false);
                emptyContent.setVisible(false);
                break;
            default:
                break;
        }
        hasDomain = true;
        checkSubmitButton();
    }

    @FXML
    public void onSelectService(ActionEvent event) {
        ServiceInfoModel selected = serviceCombobox.getSelectionModel().getSelectedItem();
        if (selected == null) {
            hasService = false;
            submitButton.setDisable(true);
        } else {
            hasService = true;
            domainCombobox.setItems(FXCollections.observableArrayList(selected.getSupportedDomains()));
            checkSubmitButton();
        }
    }

    @FXML
    public void onMoveFieldToSelectedBtnClick(MouseEvent event) {
        ObservableList<String> selected = availableFields.getSelectionModel().getSelectedItems();
        if (selected.size() > 0) {
            for (String elem: selected) {
                if (!selectedFields.getItems().contains(elem))
                    selectedFields.getItems().add(elem);
            }
        }
    }

    @FXML
    public void onMoveFieldToDeselectedBtnClick(MouseEvent event) {
        ObservableList<String> selected = selectedFields.getSelectionModel().getSelectedItems();
        if (selected.size() > 0) {
            selectedFields.getItems().removeAll(selected);
            selectedFields.refresh();
        }
    }

    @FXML
    public void onRemoveAllFieldsBtnClick(MouseEvent event) {
        selectedFields.getItems().clear();
    }

    @FXML
    public void onBackBtnPressed(ActionEvent event) {
        uninitialize();
        ApplicationSingletonContainer
                .getBean(SceneController.class)
                .showLayout(ApplicationSingletonContainer.getBean(GlobalApplicationSettings.class).getApplicationType().getLayout());
    }

    private void handleTabChanging(ObservableValue<? extends Tab> observable, Tab oldTab, Tab newTab) {
        if (Objects.nonNull(oldTab) && Objects.nonNull(newTab)) {
            //We can also store only columns which are related only to current table and recreate table between tabs. (if it needed)
            final ItemFxComponent itemFxComponent = (ItemFxComponent) newTab.getContent();
            if (itemFxComponent != null) {
                itemFxComponent.updateUsingLastActualInfo();
            }
        }
    }

    private void checkSubmitButton() {
        if (hasDomain && hasService
                && (domainCombobox.getSelectionModel().getSelectedItem().equals(SupportedItemDomains.MARKET_PRICE) ?
                ricMarketPrice.getText() != null && ricMarketPrice.getText().trim().length() > 0 : ricMarketBy.getText() != null && ricMarketBy.getText().trim().length() > 0)) {
            submitButton.setDisable(false);
        } else {
            submitButton.setDisable(true);
        }
    }

    private String getTabName(ItemViewModel settings) {
        return settings.getServiceName() + ", " + settings.getDomain() + ", " + (settings.isBatch() ? (settings.getBatchRICs().get(0) + ",...") : settings.getRIC());
    }

    public void uninitialize() {
        ApplicationSingletonContainer.getBean(DebugAreaStream.class).disable();
        debugTextArea.clear();
        ApplicationSingletonContainer.getBean(OmmConsumer.class).uninitialize();
        ApplicationSingletonContainer.deleteBean(OmmConsumer.class);
    }
}
