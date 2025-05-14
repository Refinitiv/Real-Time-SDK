/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationFxComponents;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationLayouts;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationStyles;
import com.refinitiv.ema.examples.rtviewer.desktop.common.StatefulController;

import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

public class SceneController {
    private static final String FILE_CHOOSER_TITLE = "Select a file";

    private final Stage primaryStage;
    private final FileChooser fileChooser;
    private final Map<ApplicationLayouts, Scene> sceneMap;

    public SceneController(Stage primaryStage) {
        this.primaryStage = primaryStage;
        this.primaryStage.addEventFilter(WindowEvent.WINDOW_SHOWING, event -> handleWindowShowing());

        fileChooser = new FileChooser();
        fileChooser.setTitle(FILE_CHOOSER_TITLE);

        this.sceneMap = new HashMap<>();
    }

    public Stage getPrimaryStage() {
        return primaryStage;
    }

    public void showLayout(ApplicationLayouts layout) {
        primaryStage.hide();
        primaryStage.setScene(loadSceneFromLayout(layout));
        primaryStage.centerOnScreen();
        primaryStage.show();
    }

    private Scene loadSceneFromLayout(ApplicationLayouts layout) {
        if (!sceneMap.containsKey(layout)) {
            try {
                final FXMLLoader loader = new FXMLLoader(layout.getResource());
                final Parent parentNode = loader.load();
                final Scene scene = new Scene(parentNode);
                scene.setUserData(loader);
                for (ApplicationStyles style : layout.getStyles()) {
                    scene.getStylesheets().add(style.getResource().toExternalForm());
                }
                if (loader.getController() instanceof StatefulController) {
                    sceneMap.put(layout, scene);
                }
                return scene;
            } catch (IOException e) {
                e.printStackTrace();
                System.err.println("Error in time of creation the adding scene: " + layout.name());
                System.exit(0);
            }
            return null;
        }
        return sceneMap.get(layout);
    }

    private void handleWindowShowing() {
        FXMLLoader loader = (FXMLLoader) primaryStage.getScene().getUserData();
        if (Objects.nonNull(loader) && loader.getController() instanceof StatefulController) {
            final StatefulController controller = loader.getController();
            controller.executeOnShow();
        }
    }

    public File showFileChooserWindow() {
        return fileChooser.showOpenDialog(primaryStage);
    }

    public static void loadComponent(ApplicationFxComponents component, Object componentController) {
        FXMLLoader fxmlLoader = new FXMLLoader(component.getResource());
        fxmlLoader.setRoot(componentController);
        fxmlLoader.setController(componentController);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }
}
