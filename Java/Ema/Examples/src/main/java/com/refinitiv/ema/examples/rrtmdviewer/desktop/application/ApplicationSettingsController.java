package com.refinitiv.ema.examples.rrtmdviewer.desktop.application;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.SceneController;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationSingletonContainer;
import javafx.fxml.FXML;
import javafx.scene.control.RadioButton;
import javafx.scene.control.ToggleGroup;

public class ApplicationSettingsController {

    @FXML
    private ToggleGroup applicationTypeToggleGroup;

    private GlobalApplicationSettings globalApplicationSettings;

    private SceneController sceneController;


    @FXML
    private void initialize() {
        globalApplicationSettings = ApplicationSingletonContainer.getBean(GlobalApplicationSettings.class);
        sceneController = ApplicationSingletonContainer.getBean(SceneController.class);
        assert globalApplicationSettings != null;
        globalApplicationSettings.setApplicationType(ApplicationTypes.DISCOVERED_ENDPOINT);
        applicationTypeToggleGroup.selectedToggleProperty().addListener(
                (observable, oldValue, newValue) -> handleRadioButtonChanging()
        );
    }

    private void handleRadioButtonChanging() {
        RadioButton radioButton = (RadioButton) applicationTypeToggleGroup.getSelectedToggle();
        String radioStrValue = radioButton.getUserData().toString();
        globalApplicationSettings.setApplicationType(ApplicationTypes.getApplicationType(radioStrValue));
    }

    @FXML
    private void handleActionButton() {
        sceneController.showLayout(globalApplicationSettings.getApplicationType().getLayout());
    }
}
