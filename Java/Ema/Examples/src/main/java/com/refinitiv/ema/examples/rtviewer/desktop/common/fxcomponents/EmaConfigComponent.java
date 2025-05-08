/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common.fxcomponents;

import java.io.File;
import java.net.URL;
import java.util.Objects;

import com.refinitiv.ema.examples.rtviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationFxComponents;
import com.refinitiv.ema.examples.rtviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.EmaConfigModel;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.CheckBox;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;

public class EmaConfigComponent extends VBox {
    private static final String DEFAULT_FILE_ERROR = "No file specified and default file is not provided";

    private static final String CONSUMER_NAME_EMPTY = "EMAConfig: Consumer name is empty";

    private static final String DEFAULT_FILE_NAME = "EmaConfig.xml";

    @FXML
    private FilePickerComponent emaConfigFilePicker;

    @FXML
    private CheckBox emaConfigCheckbox;

    @FXML
    private ScrollableTextField consumerNameTextField;

    @FXML
    private HBox emaConfigWrapBox;

    private String defaultConsumerName;

    public EmaConfigComponent() {
        SceneController.loadComponent(ApplicationFxComponents.EMA_CONFIG_COMPONENT, this);
    }

    @FXML
    public void initialize() {
        //Find default EmaConfig.xml file
        final URL defaultConfigResource = EmaConfigComponent.class.getResource("/".concat(DEFAULT_FILE_NAME));
        if (Objects.nonNull(defaultConfigResource)) {
            emaConfigFilePicker.getFilePickerTextField().setText(defaultConfigResource.getPath());
        } else {
            final File emaConfigFile = new File(DEFAULT_FILE_NAME);
            if (emaConfigFile.exists()) {
                emaConfigFilePicker.getFilePickerTextField().setText(emaConfigFile.getPath());
            }
        }
    }

    @FXML
    public void handleEmaConfigCheckbox(ActionEvent event) {
        emaConfigWrapBox.setDisable(!emaConfigCheckbox.isSelected());
    }

    public CheckBox getEmaConfigCheckbox() {
        return emaConfigCheckbox;
    }

    public boolean validate(OMMViewerError error) {
        if (emaConfigCheckbox.isSelected()) {
            if (emaConfigFilePicker.getFilePickerTextField().getText().trim().isEmpty()) {
                error.clear();
                error.setFailed(true);
                error.appendErrorText(DEFAULT_FILE_ERROR);
            }
            if (consumerNameTextField.getText().isEmpty()) {
                error.clear();
                error.setFailed(true);
                error.appendErrorText(CONSUMER_NAME_EMPTY);
            }
        }
        return error.isFailed();
    }

    public EmaConfigModel createEmaConfigModel() {
        return EmaConfigModel.builder()
                .useEmaConfig(emaConfigCheckbox.isSelected())
                .emaConfigFilePath(emaConfigFilePicker.getFilePickerTextField().getText())
                .consumerName(consumerNameTextField.getText())
                .build();
    }

    public String getDefaultConsumerName() {
        return defaultConsumerName;
    }

    public void setDefaultConsumerName(String defaultConsumerName) {
        this.defaultConsumerName = defaultConsumerName;
        consumerNameTextField.setText(defaultConsumerName);
    }

    public void setFilePickerTextLength(double width) {
        this.emaConfigFilePicker.setTextFieldLength(width);
    }

    public double getFilePickerTextLength() {
        return this.emaConfigFilePicker.getTextFieldLength();
    }

    public void setConsumerNameWidth(double width) {
        consumerNameTextField.setCustomWidth(width);
    }
}
