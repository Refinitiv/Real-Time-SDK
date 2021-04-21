package com.refinitiv.ema.examples.rrtmdviewer.desktop;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationLayouts;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationStyles;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.FileChooser;
import javafx.stage.Stage;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class SceneController {
    private static final String FILE_CHOOSER_TITLE = "Select a file";

    private final Stage primaryStage;
    private final FileChooser fileChooser;
    private final Map<ApplicationLayouts, Scene> sceneMap = new HashMap<>();

    public SceneController(Stage primaryStage) {
        this.primaryStage = primaryStage;

        fileChooser = new FileChooser();
        fileChooser.setTitle(FILE_CHOOSER_TITLE);
    }

    public Stage getPrimaryStage() {
        return primaryStage;
    }

    public void addScene(ApplicationLayouts layout) {
        try {
            if (!sceneMap.containsKey(layout)) {
                final Parent parentNode = FXMLLoader.load(layout.getResource());
                final Scene scene = new Scene(parentNode);
                for (ApplicationStyles style : layout.getStyles()) {
                    scene.getStylesheets().add(style.getResource().toExternalForm());
                }
                sceneMap.put(layout, scene);
            }
        } catch (IOException e) {
            e.printStackTrace();
            System.err.println("Error in time of creation the adding scene: " + layout.name());
            System.exit(0);
        }
    }

    public void deleteScene(ApplicationLayouts layout) {
        sceneMap.remove(layout);
    }

    public void showLayout(ApplicationLayouts layout) {
        primaryStage.hide();
        primaryStage.setScene(sceneMap.get(layout));
        primaryStage.centerOnScreen();
        primaryStage.show();
    }

    public File showFileChooserWindow() {
        return fileChooser.showOpenDialog(primaryStage);
    }
}
