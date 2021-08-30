package com.refinitiv.ema.examples.rrtmdviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.*;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.fxcomponents.*;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.*;
import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.geometry.Rectangle2D;
import javafx.scene.Node;
import javafx.scene.control.*;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.stage.Screen;

import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ExecutorService;

public class DiscoveredEndpointSettingsController implements StatefulController {

    private static final String BUTTON_CONNECT_TEXT = "Connect";

    private static final String BUTTON_CONNECT_STYLE = "connect-button";

    private static final String BUTTON_RETRIEVE_ENDPOINTS_TEXT = "Retrieve Service Endpoints";

    private static final String DEFAULT_TOKEN_SERVICE_URL = "https://api.refinitiv.com/auth/oauth2/v1/token";

    private static final String DEFAULT_SERVICE_ENDPOINT_URL = "https://api.refinitiv.com/streaming/pricing/v1/";

    private static final String CLIENT_DISCOVERED_ENDPOINT_SETTINGS_ERROR = "Discovered Endpoint Settings - Validation Failure:";

    private static final String SERVICE_ENDPOINT_SELECTION_EMPTY = "Validation failure: you should select at least one available " +
            "service discovery endpoint";

    private static final String EMPTY_VALIDATION_POSTFIX = " is empty.";

    private static final double DEFAULT_PREF_HEIGHT = 900;

    /* This value is set more than the maximum width of VBOX at the top level */
    private static final double DEFAULT_PREF_WIDTH = 550;

    private static final double DEFAULT_RATIO = 0.93;

    @FXML
    private ScrollableTextField clientIdTextField;

    @FXML
    private ScrollableTextField usernameTextField;

    @FXML
    private PasswordEyeComponent usernamePasswordComponent;

    @FXML
    private VBox connectionTypeBox;

    @FXML
    private ComboBox<DiscoveredEndpointConnectionTypes> connectionTypesComboBox;

    @FXML
    private Pane encryptionOptionsPane;

    @FXML
    private CheckBox encryptionOptionCheckbox;

    @FXML
    private FilePickerComponent keyFilePicker;

    @FXML
    private PasswordEyeComponent keyPasswordComponent;

    @FXML
    private Pane customServiceUrlsPane;

    @FXML
    private CheckBox customServiceUrlsCheckbox;

    @FXML
    private ScrollableTextField tokenServiceUrl;

    @FXML
    private ScrollableTextField serviceDiscoveryUrl;

    @FXML
    private Pane useProxyPane;

    @FXML
    private CheckBox useProxyCheckbox;

    @FXML
    private ScrollableTextField proxyHostTextFld;

    @FXML
    private ScrollableTextField proxyPortTextFld;

    @FXML
    private Pane useProxyAuthenticationPane;

    @FXML
    private CheckBox useProxyAuthenticationCheckbox;

    @FXML
    private ScrollableTextField proxyAuthLogin;

    @FXML
    private PasswordEyeComponent proxyAuthPassword;

    @FXML
    private ScrollableTextField proxyAuthDomain;

    @FXML
    private FilePickerComponent krbFilePicker;

    @FXML
    private Button primaryActionButton;

    @FXML
    private ListView<DiscoveredEndpointInfoModel> serviceEndpointChoiceBox;

    @FXML
    private VBox serviceEndpointVBox;

    @FXML
    private TabPane primaryTabPane;

    @FXML
    private ErrorDebugAreaComponent errorDebugArea;

    @FXML
    private DictionaryLoaderComponent dictionaryLoader;

    @FXML
    private EmaConfigComponent emaConfigComponent;

    @FXML
    private ScrollPane primaryPane;

    @FXML
    private HBox controlButtons;

    @FXML
    private Button backButton;

    private final OMMViewerError ommViewerError = new OMMViewerError();
    private AsyncResponseModel asyncResponseObserver;

    private ExecutorService executorService;
    private SceneController sceneController;
    private DiscoveredEndpointSettingsService discoveredEndpointSettingsService;
    private DiscoveredEndpointSettingsModel discoveredEndpointSettings;
    private final ServiceEndpointDataModel serviceEndpointData = new ServiceEndpointDataModel();

    private boolean serviceEndpointsRetrieved;

    private boolean actionButtonIsClicked;

    @FXML
    public void initialize() {
        sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
        executorService = ApplicationSingletonContainer.getBean(ExecutorService.class);
        tokenServiceUrl.setText(DiscoveredEndpointSettingsModel.DEFAULT_TOKEN_SERVICE_URL);
        serviceDiscoveryUrl.setText(DiscoveredEndpointSettingsModel.DEFAULT_DISCOVERY_ENDPOINT_URL);
        discoveredEndpointSettingsService = new DiscoveredEndpointSettingsServiceImpl();
        ApplicationSingletonContainer.addBean(DiscoveredEndpointSettingsService.class, discoveredEndpointSettings);

        //Prepare ListView selection
        serviceEndpointChoiceBox.getSelectionModel().setSelectionMode(SelectionMode.MULTIPLE);
        serviceEndpointChoiceBox.addEventFilter(MouseEvent.MOUSE_PRESSED, this::handleEndpointSelection);

        //Add checkbox handler for config file
        emaConfigComponent.getEmaConfigCheckbox().addEventHandler(ActionEvent.ACTION, this::handleEmaConfigCheckbox);

        Rectangle2D screenBounds = Screen.getPrimary().getBounds();
        primaryPane.setPrefHeight(Math.min(screenBounds.getMaxY() * DEFAULT_RATIO, DEFAULT_PREF_HEIGHT));
        primaryPane.setPrefWidth(DEFAULT_PREF_WIDTH);
        primaryActionButton.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 10);
        backButton.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 10);
        errorDebugArea.setAreaHeight(Math.max(60, sceneController.getPrimaryStage().getScene().getWindow().heightProperty().get() -
                primaryTabPane.getHeight() - controlButtons.getHeight() - 225));

        sceneController.getPrimaryStage().getScene().getWindow().widthProperty().addListener(e -> {
            double width = Math.max(200, sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get() / 2 - 80);
            usernameTextField.setCustomWidth(width);
            clientIdTextField.setCustomWidth(width);
            tokenServiceUrl.setCustomWidth(width);
            serviceDiscoveryUrl.setCustomWidth(width);
            krbFilePicker.setFilePickerWidth(width);
            proxyAuthDomain.setCustomWidth(width);
            proxyAuthLogin.setCustomWidth(width);
            proxyAuthPassword.setCustomWidth(width);
            usernamePasswordComponent.getPasswordField().setPrefWidth(width);
            clientIdTextField.setCustomWidth(width);
            keyPasswordComponent.setCustomWidth(width);
            proxyHostTextFld.setCustomWidth(width);
            proxyPortTextFld.setCustomWidth(width);

            keyFilePicker.setFilePickerWidth(width);
            usernamePasswordComponent.setCustomWidth(width);
            primaryTabPane.setPrefWidth(Math.max(450, sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get()) - 30);
            errorDebugArea.setPrefWidth(Math.max(450, sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get() - 30));
            dictionaryLoader.setCustomWidth(width);

            emaConfigComponent.setConsumerNameWidth(width - 30);
            emaConfigComponent.setFilePickerTextLength(width - 30);
            primaryActionButton.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 30);
            backButton.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 30);
        });

        sceneController.getPrimaryStage().getScene().getWindow().heightProperty().addListener(e ->{
            errorDebugArea.setAreaHeight(Math.max(60, sceneController.getPrimaryStage().getScene().getWindow().heightProperty().get() -
                    primaryTabPane.getHeight() - controlButtons.getHeight() - 225));
        });
    }

    public void handleBackBtnAction(ActionEvent event) {
        clear();

        /* Clears the Discovery UI only once */
        actionButtonIsClicked = false;
    }

    @FXML
    public void handleSubmitBtnAction(ActionEvent actionEvent) {
        primaryActionButton.setDisable(true);
        validateFields();

        actionButtonIsClicked = true;

        if (ommViewerError.isFailed()) {
            errorDebugArea.writeError(ommViewerError.toString());
            primaryActionButton.setDisable(false);
            return;
        }
        if (emaConfigComponent.getEmaConfigCheckbox().isSelected() || !serviceEndpointsRetrieved) {
            this.discoveredEndpointSettings = mapDiscoveredEndpointSettings();
            primaryTabPane.setDisable(true);
        }

        if (!serviceEndpointsRetrieved && !emaConfigComponent.getEmaConfigCheckbox().isSelected()) {
            /* Create and register an observer for every requests */
            asyncResponseObserver = new AsyncResponseModel();
            prepareAsyncResponseObserver();

            if(!discoveredEndpointSettingsService.isInitialized())
            {
                discoveredEndpointSettingsService.initialize();
            }

            executorService.submit(new FxRunnableTask(() -> discoveredEndpointSettingsService
                    .requestServiceDiscovery(discoveredEndpointSettings, asyncResponseObserver)));
        } else {
            final FxRunnableTask fxRunnableTask = new FxRunnableTask(
                    () -> discoveredEndpointSettingsService.createOmmConsumer(
                            discoveredEndpointSettings, serviceEndpointData, ommViewerError)
            );
            fxRunnableTask.setOnSucceeded(e -> {
                if (!ommViewerError.isFailed()) {
                    sceneController.showLayout(ApplicationLayouts.ITEM_VIEW);
                } else {
                    errorDebugArea.writeError(ommViewerError.toString());
                    primaryActionButton.setDisable(false);
                    if (emaConfigComponent.getEmaConfigCheckbox().isSelected()) {
                        primaryTabPane.setDisable(false);
                    }
                }
            });
            executorService.submit(fxRunnableTask);
        }
    }

    public void handleEndpointSelection(MouseEvent event) {
        Node node = event.getPickResult().getIntersectedNode();

        while (node != null && node != serviceEndpointChoiceBox && !(node instanceof ListCell)) {
            node = node.getParent();
        }

        if (node instanceof ListCell<?>) {
            event.consume(); //suspend further execution of the event

            ListCell<?> cell = (ListCell<?>) node;
            int index = cell.getIndex();
            DiscoveredEndpointInfoModel discoveredInfo = serviceEndpointChoiceBox.getItems().get(index);
            if (serviceEndpointChoiceBox.getSelectionModel().isSelected(index)) {
                serviceEndpointChoiceBox.getSelectionModel().clearSelection(index);
                serviceEndpointData.getEndpoints().remove(discoveredInfo);
            } else {
                serviceEndpointChoiceBox.getSelectionModel().select(index);
                serviceEndpointData.getEndpoints().add(discoveredInfo);
            }
        }
    }

    @FXML
    public void handleEncryptionOptionsCheckbox(ActionEvent event) {
        encryptionOptionsPane.setDisable(!encryptionOptionCheckbox.isSelected());
    }

    @FXML
    public void handleCustomServiceUrlsCheckbox(ActionEvent event) {
        customServiceUrlsPane.setDisable(!customServiceUrlsCheckbox.isSelected());
    }

    @FXML
    public void handleUseProxyCheckbox(ActionEvent event) {
        useProxyPane.setDisable(!useProxyCheckbox.isSelected());
        useProxyAuthenticationPane.setDisable(!useProxyCheckbox.isSelected() || !useProxyAuthenticationCheckbox.isSelected());
        useProxyAuthenticationCheckbox.setDisable(!useProxyCheckbox.isSelected());
    }

    @FXML
    public void handleUseProxyAuthenticationCheckbox(ActionEvent event) {
        useProxyAuthenticationPane.setDisable(!useProxyAuthenticationCheckbox.isSelected());
    }

    @FXML
    public void handleConnectionTypeComboBox(ActionEvent event) {
        DiscoveredEndpointConnectionTypes connectionType = connectionTypesComboBox.getValue();
        final boolean isWebSocket = Objects.equals(DiscoveredEndpointConnectionTypes.ENCRYPTED_WEBSOCKET, connectionType);
        dictionaryLoader.getDownloadDictCheckbox().setSelected(!isWebSocket);
        dictionaryLoader.onDownloadDictionaryChanged(event);
    }

    private void handleEmaConfigCheckbox(ActionEvent event) {
        final boolean isEmaConfigSelected = emaConfigComponent.getEmaConfigCheckbox().isSelected();
        connectionTypeBox.setDisable(isEmaConfigSelected);
        dictionaryLoader.setDisable(isEmaConfigSelected);
        if (isEmaConfigSelected) {
            primaryActionButton.setText(BUTTON_CONNECT_TEXT);
            primaryActionButton.getStyleClass().add("connect-button");
        } else {
            primaryActionButton.setText(BUTTON_RETRIEVE_ENDPOINTS_TEXT);
            primaryActionButton.getStyleClass().remove("connect-button");
        }
    }

    private void prepareAsyncResponseObserver() {
        asyncResponseObserver.responseStatusProperty().addListener(
                (observable, oldValue, newValue) -> onServiceDiscoveryRetrieved()
        );
    }

    private void onServiceDiscoveryRetrieved() {
        if (Objects.equals(AsyncResponseStatuses.SUCCESS, this.asyncResponseObserver.getResponseStatus())) {
            final ServiceEndpointResponseModel serviceEndpointResponseModel = (ServiceEndpointResponseModel) this.asyncResponseObserver.getResponse();
            final List<DiscoveredEndpointInfoModel> info = serviceEndpointResponseModel.getResponseData();
            this.serviceEndpointData.setDictionaryData(this.dictionaryLoader.createDictionaryModel());
            serviceEndpointsRetrieved = true;
            Platform.runLater(() -> {
                if (!info.isEmpty()) {
                    primaryActionButton.setText(BUTTON_CONNECT_TEXT);
                    primaryActionButton.getStyleClass().add(BUTTON_CONNECT_STYLE);
                    serviceEndpointVBox.setDisable(false);
                    serviceEndpointChoiceBox.setItems(FXCollections.observableList(info));
                }
                primaryActionButton.setDisable(false);
            });
        } else if (Objects.equals(AsyncResponseStatuses.FAILED, this.asyncResponseObserver.getResponseStatus())) {
            final String msg = this.asyncResponseObserver.getResponse().toString();
            Platform.runLater(() -> {
                errorDebugArea.writeError(msg + System.lineSeparator());
                primaryTabPane.setDisable(false);
                primaryActionButton.setDisable(false);
            });
        }
    }

    private DiscoveredEndpointSettingsModel mapDiscoveredEndpointSettings() {
        EncryptionDataModel encryptionDataModel = null;
        ProxyDataModel proxyDataModel = null;
        String tokenServiceUrl = DEFAULT_TOKEN_SERVICE_URL;
        String serviceDiscoveryUrl = DEFAULT_SERVICE_ENDPOINT_URL;
        if (encryptionOptionCheckbox.isSelected()) {
            encryptionDataModel = EncryptionDataModel.builder()
                    .keyFilePath(keyFilePicker.getFilePickerTextField().getText().trim())
                    .keyPassword(keyPasswordComponent.getPasswordField().getText().trim())
                    .build();

        }

        if (useProxyCheckbox.isSelected()) {
            ProxyAuthenticationDataModel proxyAuthentication = null;
            if (useProxyAuthenticationCheckbox.isSelected()) {
                proxyAuthentication = ProxyAuthenticationDataModel.builder()
                        .login(proxyAuthLogin.getText().trim())
                        .password(proxyAuthPassword.getPasswordField().getText().trim())
                        .domain(proxyAuthDomain.getText().trim())
                        .krbFilePath(krbFilePicker.getFilePickerTextField().getText().trim())
                        .build();
            }
            proxyDataModel = ProxyDataModel.builder()
                    .host(proxyHostTextFld.getText().trim())
                    .port(proxyPortTextFld.getText().trim())
                    .useProxyAuthentication(proxyAuthentication)
                    .build();
        }

        EmaConfigModel emaConfigModel = emaConfigComponent.createEmaConfigModel();

        if (customServiceUrlsCheckbox.isSelected() && !emaConfigModel.isUseEmaConfig()) {
            tokenServiceUrl = this.tokenServiceUrl.getText().trim();
            serviceDiscoveryUrl = this.serviceDiscoveryUrl.getText().trim();
        }

        return DiscoveredEndpointSettingsModel.builder()
                .clientId(clientIdTextField.getText().trim())
                .username(usernameTextField.getText().trim())
                .password(usernamePasswordComponent.getPasswordField().getText().trim())
                .connectionType(connectionTypesComboBox.getValue())
                .useEncryption(encryptionDataModel)
                .useProxy(proxyDataModel)
                .tokenServiceUrl(tokenServiceUrl)
                .serviceEndpointUrl(serviceDiscoveryUrl)
                .useEmaConfig(emaConfigModel)
                .build();
    }

    private void validateFields() {
        ommViewerError.clear();
        if (serviceEndpointsRetrieved) {
            if (serviceEndpointChoiceBox.getSelectionModel().isEmpty()) {
                ommViewerError.setFailed(true);
                ommViewerError.appendErrorText(SERVICE_ENDPOINT_SELECTION_EMPTY);
            }
        } else {
            ommViewerError.appendErrorText(CLIENT_DISCOVERED_ENDPOINT_SETTINGS_ERROR);
            boolean hasError = hasErrorField("Username", usernameTextField.getTextField())
                    | hasErrorField("Password", usernamePasswordComponent.getPasswordField())
                    | hasErrorField("Client ID", clientIdTextField.getTextField());
            if (encryptionOptionCheckbox.isSelected()) {
                hasError |= hasErrorField("Key File", keyFilePicker.getFilePickerTextField())
                        | hasErrorField("Key Password", keyPasswordComponent.getPasswordField());
            }

            if (useProxyCheckbox.isSelected()) {
                hasError |= hasErrorField("Proxy host", proxyPortTextFld.getTextField())
                        | hasErrorField("Proxy port", proxyPortTextFld.getTextField());
            }

            if (!emaConfigComponent.getEmaConfigCheckbox().isSelected()) {
                if (customServiceUrlsCheckbox.isSelected()){
                    hasError |= hasErrorField("Service Discovery URL", serviceDiscoveryUrl.getTextField())
                            | hasErrorField("Token Service URL", tokenServiceUrl.getTextField());
                }
                hasError |= dictionaryLoader.validate(ommViewerError);
            }
            hasError |= emaConfigComponent.validate(ommViewerError);
            ommViewerError.setFailed(hasError);
        }
    }

    private boolean hasErrorField(String name, TextInputControl field) {
        if (Objects.isNull(field.getText()) || field.getText().trim().isEmpty()) {
            ommViewerError.appendErrorText(name + EMPTY_VALIDATION_POSTFIX);
            return true;
        }
        return false;
    }

    private ServiceEndpointDataModel getServiceEndpointConfig() {
        DictionaryDataModel dictionaryData = this.dictionaryLoader.createDictionaryModel();
        List<DiscoveredEndpointInfoModel> endpointInfo = Collections.unmodifiableList(
                serviceEndpointChoiceBox.getSelectionModel().getSelectedItems()
        );
        return new ServiceEndpointDataModel();
    }

    @Override
    public void executeOnShow() {
        errorDebugArea.processDebug();
        if (serviceEndpointsRetrieved) {
            primaryActionButton.setDisable(false);
        } else {
            discoveredEndpointSettingsService.initialize();
        }
    }

    @Override
    public void clear() {
        if(actionButtonIsClicked) {
            if (serviceEndpointsRetrieved) {
                //unlock configuration tab and clear service endpoints.
                primaryTabPane.setDisable(false);
                primaryActionButton.setDisable(false);
                serviceEndpointsRetrieved = false;
                serviceEndpointVBox.setDisable(true);
                serviceEndpointChoiceBox.getItems().clear();
                primaryActionButton.getStyleClass().remove(BUTTON_CONNECT_STYLE);
                primaryActionButton.setText(BUTTON_RETRIEVE_ENDPOINTS_TEXT);
            } else {
                discoveredEndpointSettingsService.uninitialize();
                primaryTabPane.setDisable(false);
                primaryActionButton.setDisable(false);
            }
        }
        else
        {
            errorDebugArea.stopDebugStreaming();
            sceneController.showLayout(ApplicationLayouts.APPLICATION_SETTINGS);
        }
    }
}
