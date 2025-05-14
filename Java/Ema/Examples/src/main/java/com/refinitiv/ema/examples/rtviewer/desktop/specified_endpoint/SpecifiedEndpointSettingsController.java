/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.specified_endpoint;

import java.util.Objects;
import java.util.concurrent.ExecutorService;

import com.refinitiv.ema.examples.rtviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationLayouts;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rtviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rtviewer.desktop.common.StatefulController;
import com.refinitiv.ema.examples.rtviewer.desktop.common.SupportedJsonVersions;
import com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents.DictionaryLoaderComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents.EmaConfigComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents.ErrorDebugAreaComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents.FilePickerComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents.PasswordEyeComponent;
import com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents.ScrollableTextField;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.ConnectionDataModel;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.EmaConfigModel;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.EncryptionDataModel;
import com.refinitiv.eta.codec.CodecReturnCodes;

import javafx.collections.FXCollections;
import javafx.concurrent.Task;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.geometry.Rectangle2D;
import javafx.scene.control.Button;
import javafx.scene.control.CheckBox;
import javafx.scene.control.CheckMenuItem;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.MenuButton;
import javafx.scene.control.MenuItem;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TabPane;
import javafx.scene.control.Tooltip;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Screen;

public class SpecifiedEndpointSettingsController implements StatefulController {

    private static final String DEFAULT_APP_ID = "256";

    private static final String RWF = "rssl.rwf";
    
    private static final double DEFAULT_PREF_HEIGHT = 910;
    /* This value is set more than the maximum width of VBOX at the top level */
    private static final double DEFAULT_PREF_WIDTH = 600;

    private static final double DEFAULT_RATIO = 0.93;

    private SceneController sceneController;
    private SpecifiedEndpointSettingsService specifiedEndpointSettingsService = new SpecifiedEndpointSettingsServiceImpl();

    private final OMMViewerError error = new OMMViewerError();

    @FXML
    private ScrollableTextField host;

    @FXML
    private ScrollableTextField port;

    @FXML
    private ScrollableTextField host2;

    @FXML
    private ScrollableTextField port2;

    @FXML
    private ComboBox<SpecifiedEndpointConnectionTypes> connType;

    @FXML
    private Label protocolsLabel;

    @FXML
    private CheckBox rwfCheckbox;

    @FXML
    private CheckBox jsonCheckbox;

    @FXML
    private Label jsonVersions;

    @FXML
    private CheckBox encrypted;

    @FXML
    private FilePickerComponent keyFilePicker;

    @FXML
    private Label passwLabel;

    @FXML
    private PasswordEyeComponent keyPassword;

    @FXML
    private ScrollableTextField appId;

    @FXML
    private ScrollableTextField position;

    @FXML
    private ScrollableTextField username;

    @FXML
    private DictionaryLoaderComponent dictionaryLoader;

    @FXML
    private VBox encryptedVBox;

    @FXML
    private MenuButton jsonVersionsMenu;

    @FXML
    private Button backButton;
    
    @FXML
    private Button connect;

    @FXML
    private ErrorDebugAreaComponent errorDebugArea;

    @FXML
    private VBox connectionSettingsBox;

    @FXML
    private EmaConfigComponent emaConfigComponent;

    @FXML
    private ScrollPane scrollPane;

    @FXML
    private TabPane tabPane;

    @FXML
    private HBox controlButtons;
    
    @FXML
    private CheckBox encryptionOptionCheckboxTLSVersion12;
    
    @FXML
    private CheckBox encryptionOptionCheckboxTLSVersion13;

    private Tooltip encryptionTooltip;

    @FXML
    public void initialize() {

        sceneController = ApplicationSingletonContainer.getBean(SceneController.class);

        encryptionTooltip = new Tooltip();
        encryptionTooltip.setText("Set custom encryption options: key file and key password.\nThese parameters are optional.");
        encrypted.setTooltip(encryptionTooltip);

        emaConfigComponent.getEmaConfigCheckbox().addEventHandler(ActionEvent.ACTION, this::handleEmaConfigCheckbox);

        setupVersionMenu();

        Rectangle2D screenBounds = Screen.getPrimary().getBounds();
        scrollPane.setPrefHeight(Math.min(screenBounds.getMaxY() * DEFAULT_RATIO, DEFAULT_PREF_HEIGHT));
        scrollPane.setPrefWidth(DEFAULT_PREF_WIDTH);

        errorDebugArea.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() - 20);
        tabPane.setPrefWidth(550);
        errorDebugArea.setAreaHeight(50);
        connect.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 40);
        backButton.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 10);

        sceneController.getPrimaryStage().getScene().getWindow().widthProperty().addListener(e -> {
            scrollPane.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get());
            double width = Math.max(200, sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get() / 2 - 80);
            host.setCustomWidth(width);
            appId.setCustomWidth(width);
            port.setCustomWidth(width);
            host2.setCustomWidth(width);
            port2.setCustomWidth(width);
            position.setCustomWidth(width);
            username.setCustomWidth(width);
            keyFilePicker.setFilePickerWidth(width);
            keyPassword.setCustomWidth(width);
            connect.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 40);
            backButton.setPrefWidth(sceneController.getPrimaryStage().getScene().getWindow().getWidth() / 2 - 10);
            tabPane.setPrefWidth(Math.max(550, sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get()) - 20);
            errorDebugArea.setPrefWidth(Math.max(550, sceneController.getPrimaryStage().getScene().getWindow().widthProperty().get() - 20));
            dictionaryLoader.setCustomWidth(width);

            emaConfigComponent.setConsumerNameWidth(width - 30);
            emaConfigComponent.setFilePickerTextLength(width - 30);
        });

        sceneController.getPrimaryStage().getScene().getWindow().heightProperty().addListener(e ->{
            errorDebugArea.setAreaHeight(Math.max(50, sceneController.getPrimaryStage().getScene().getWindow().heightProperty().get() -
                    tabPane.getHeight() - controlButtons.getHeight() - 100));
        });
    }

    @FXML
    public void onConnectionTypeChanged(ActionEvent event) {
    	checkConnectionType();
    }

    @FXML
    public void onJsonSelected(ActionEvent event) {
        if (jsonCheckbox.isSelected()) {
            jsonVersionsMenu.setDisable(false);
            jsonVersions.setDisable(false);
        } else {
            jsonVersionsMenu.setDisable(true);
            jsonVersions.setDisable(true);
        }
    }

    @FXML
    public void onEncryptionChanged(ActionEvent event) {
        if (encrypted.isSelected()) {
            keyFilePicker.setDisable(false);
            passwLabel.setDisable(false);
            keyPassword.setDisable(false);
        } else {
            keyFilePicker.setDisable(true);
            passwLabel.setDisable(true);
            keyPassword.setDisable(true);
        }
    }

    public void handleEmaConfigCheckbox(ActionEvent event) {
        final boolean isEmaConfigSelected = emaConfigComponent.getEmaConfigCheckbox().isSelected();
        connectionSettingsBox.setDisable(isEmaConfigSelected);
        dictionaryLoader.setDisable(isEmaConfigSelected);
    }

    public void onBackButtonAction(ActionEvent event) {
        clear();
        sceneController.showLayout(ApplicationLayouts.APPLICATION_SETTINGS);
    }

    @FXML
    public void onSubmitBtnClick(ActionEvent event) {

        SpecifiedEndpointSettingsModel settings;
        SpecifiedEndpointSettingsModel.SpecifiedEndpointSettingsModelBuilder specifiedEndpointBuilder = SpecifiedEndpointSettingsModel.builder();

        final EmaConfigModel model = emaConfigComponent.createEmaConfigModel();

        error.clear();

        specifiedEndpointBuilder.useEmaConfigFile(model);

        if (!model.isUseEmaConfig()) {
            /* Connectivity options */
            if (!host.getText().trim().isEmpty() && !port.getText().trim().isEmpty()) {
                specifiedEndpointBuilder.addServer(host.getText().trim(), port.getText().trim());
            }

            if (!host2.getText().trim().isEmpty() && !port2.getText().trim().isEmpty()) {
                specifiedEndpointBuilder.addServer(host2.getText().trim(), port2.getText().trim());
            }

            ConnectionDataModel.ConnectionDataModelBuilder connSettingsBuilder = ConnectionDataModel.builder();
            connSettingsBuilder.connectionType(connType.getSelectionModel().getSelectedItem());

            StringBuilder sb = new StringBuilder();
            if (rwfCheckbox.isSelected()) {
                appendProtocol(sb, RWF);
            }
            /* As for now, basically only one json version is supported */
            if (jsonCheckbox.isSelected()) {
                if (jsonVersionsMenu.getText() != null) {
                    appendProtocol(sb, jsonVersionsMenu.getText());
                }
            }
            connSettingsBuilder.protocolList(sb.toString());

            specifiedEndpointBuilder.connectionSettings(connSettingsBuilder.build());

            if (isEncryptedConnection() && encrypted.isSelected()) {
                specifiedEndpointBuilder.hasCustomEncrOptions(true);
                EncryptionDataModel encryptModel = EncryptionDataModel.builder()
                        .keyFilePath(keyFilePicker.getFilePickerTextField().getText())
                        .keyPassword(keyPassword.getPasswordField().getText())
                        .build();
                specifiedEndpointBuilder.encryptionSettings(encryptModel);
                specifiedEndpointBuilder.isTLSv12Enabled(encryptionOptionCheckboxTLSVersion12.isSelected());
                specifiedEndpointBuilder.isTLSv13Enabled(encryptionOptionCheckboxTLSVersion13.isSelected());
            }
        }

        /* Application options */
        specifiedEndpointBuilder.applicationId(appId.getText() != null && !appId.getText().isEmpty() ? appId.getText() : DEFAULT_APP_ID);

        if(position.getText() != null) {
            specifiedEndpointBuilder.position(position.getText());
        }

        if(username.getText() != null) {
            specifiedEndpointBuilder.username(username.getText());
        }

        /*dictionary options*/
        if (!model.isUseEmaConfig()) {
            dictionaryLoader.validate(error);
            specifiedEndpointBuilder.dictionaryData(dictionaryLoader.createDictionaryModel());
        }

        settings = specifiedEndpointBuilder.build();


        if (model.isUseEmaConfig()) {
            emaConfigComponent.validate(error);
        }
        else {
            settings.validate(error);
        }

        if (!error.isFailed()) {
            SceneController sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
            connect.setDisable(true);
            error.clear();
            Task<Boolean> connectTask = new Task<Boolean>() {
                @Override
                protected Boolean call() {
                    return specifiedEndpointSettingsService.connect(settings, error) != CodecReturnCodes.FAILURE;
                }
            };

            connectTask.setOnSucceeded(e -> {
                if (error.isFailed()) {
                    errorDebugArea.writeError(error.toString());
                    connect.setDisable(false);
                } else {
                    clear();
                    sceneController.showLayout(ApplicationLayouts.ITEM_VIEW);
                }
            });
            connectTask.setOnFailed(e -> {
                errorDebugArea.writeError(error.toString());
                connect.setDisable(false);
            });
            Objects.requireNonNull(ApplicationSingletonContainer.getBean(ExecutorService.class))
                    .submit(connectTask);
        } else {
            errorDebugArea.writeError(error.toString());
        }
    }

    private void appendProtocol(StringBuilder sb, String protocolText) {
        if (sb.length() > 0) {
            sb.append(",");
        }
        sb.append(protocolText);
    }

    private boolean isEncryptedConnection() {
        return connType.getSelectionModel().getSelectedItem().equals(SpecifiedEndpointConnectionTypes.ENCRYPTED_SOCKET)
                || connType.getSelectionModel().getSelectedItem().equals(SpecifiedEndpointConnectionTypes.ENCRYPTED_WEBSOCKET);
    }

    private void setupVersionMenu() {
        CheckMenuItem v2 = new CheckMenuItem(SupportedJsonVersions.TR_JSON.toString());
        v2.setMnemonicParsing(false);

        v2.setOnAction(e -> {
            StringBuilder sb = new StringBuilder();
            for (MenuItem item : jsonVersionsMenu.getItems()) {
                if (((CheckMenuItem)item).isSelected()) {
                    sb.append(item.getText());
                }
            }
            jsonVersionsMenu.setText(sb.toString());
            jsonVersionsMenu.setMnemonicParsing(false);
        });

        CheckMenuItem v3 = new CheckMenuItem(SupportedJsonVersions.RSSL_JSON.toString());

        v3.setOnAction(e -> {
            StringBuilder sb = new StringBuilder();
            for (MenuItem item : jsonVersionsMenu.getItems()) {
                if (((CheckMenuItem)item).isSelected()) {
                    sb.append(item.getText());
                    sb.append(',');
                }
            }

            if(sb.length() > 0) {
                sb.delete(sb.length() - 1, sb.length());
            }
            jsonVersionsMenu.setText(sb.toString());
        });


        jsonVersionsMenu.getItems().addAll(FXCollections.observableArrayList(v2, v3));
    }
    
    public void checkConnectionType()
    {
    	switch (connType.getSelectionModel().getSelectedItem()) {
        case SOCKET:
            jsonVersionsMenu.setDisable(true);
            jsonVersions.setDisable(true);
            jsonCheckbox.setSelected(false);
            rwfCheckbox.setSelected(true);
            jsonCheckbox.setDisable(true);
            rwfCheckbox.setDisable(true);
            protocolsLabel.setDisable(true);
            encryptedVBox.setDisable(true);
            encrypted.setDisable(true);
            break;
        case WEBSOCKET:
            jsonCheckbox.setDisable(false);
            rwfCheckbox.setDisable(false);
            protocolsLabel.setDisable(false);
            encryptedVBox.setDisable(true);
            encrypted.setDisable(true);
            break;
        case ENCRYPTED_SOCKET:
            jsonVersions.setDisable(true);
            jsonVersionsMenu.setDisable(true);
            jsonCheckbox.setSelected(false);
            rwfCheckbox.setSelected(true);
            jsonCheckbox.setDisable(true);
            rwfCheckbox.setDisable(true);
            protocolsLabel.setDisable(true);
            encryptedVBox.setDisable(false);
            encrypted.setDisable(false);
            break;
        case ENCRYPTED_WEBSOCKET:
            jsonCheckbox.setDisable(false);
            rwfCheckbox.setDisable(false);
            protocolsLabel.setDisable(false);
            encryptedVBox.setDisable(false);
            encrypted.setDisable(false);
            break;
        default:
            break;
    }
    }

    @Override
    public void executeOnShow() {
        errorDebugArea.processDebug();
        Rectangle2D screenBounds = Screen.getPrimary().getBounds();
        scrollPane.setPrefHeight(Math.min(screenBounds.getMaxY() * DEFAULT_RATIO, DEFAULT_PREF_HEIGHT));
        scrollPane.setPrefWidth(Math.min(screenBounds.getMaxX() * DEFAULT_RATIO, DEFAULT_PREF_WIDTH));
        checkConnectionType();
    }

    @Override
    public void clear() {
        errorDebugArea.stopDebugStreaming();
        connect.setDisable(false);
    }
}
