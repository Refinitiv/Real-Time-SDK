/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.SimpleFormatter;

import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.examples.rtviewer.desktop.application.GlobalApplicationSettings;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationLayouts;
import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.examples.rtviewer.desktop.common.DebugAreaStream;
import com.refinitiv.ema.examples.rtviewer.desktop.common.emalogging.DebugHandler;
import com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint.DiscoveredEndpointSettingsController;
import com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint.DiscoveredEndpointSettingsService;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.scene.image.Image;
import javafx.stage.Stage;

public class RTViewerDesktopMain {

    public static class RRTMDViewerDesktopApplication extends Application {

        private static final String APPLICATION_NAME = "LSEG Real-Time Viewer";

        @Override
        public void start(Stage primaryStage) {
            primaryStage.setTitle(APPLICATION_NAME);
            primaryStage.getIcons().add(new Image("/rtviewer/desktop/icons/lseg_sym.jpg"));
            createApplicationScopedBeans();
            setupEmaLogging();
            initializeSceneController(primaryStage);
            primaryStage.show();
        }

        @Override
        public void stop() {
            uninitializeBeans();
            Platform.exit();
            System.exit(0);
        }

        private void initializeSceneController(Stage primaryStage) {
            final SceneController sceneController = new SceneController(primaryStage);
            ApplicationSingletonContainer.addBean(SceneController.class, sceneController);
            sceneController.showLayout(ApplicationLayouts.APPLICATION_SETTINGS);
        }

        private void createApplicationScopedBeans() {
            ApplicationSingletonContainer.addBean(ExecutorService.class, Executors.newSingleThreadExecutor());
            ApplicationSingletonContainer.addBean(GlobalApplicationSettings.class, new GlobalApplicationSettings());
            ApplicationSingletonContainer.addBean(DebugHandler.class, new DebugHandler());
            ApplicationSingletonContainer.addBean(DebugAreaStream.class, new DebugAreaStream());
        }

        private void setupEmaLogging() {
            if (ApplicationSingletonContainer.containsBean(DebugHandler.class)) {
                DebugHandler debugHandler = ApplicationSingletonContainer.getBean(DebugHandler.class);
                debugHandler.setLevel(Level.FINE);
                debugHandler.setFormatter(new SimpleFormatter());
                LogManager.getLogManager().getLogger("").addHandler(debugHandler);
            }
        }

        private void uninitializeBeans() {
            if (ApplicationSingletonContainer.containsBean(OmmConsumer.class)) {
                OmmConsumer consumer = ApplicationSingletonContainer.getBean(OmmConsumer.class);
                consumer.uninitialize();
            }
            if (ApplicationSingletonContainer.containsBean(DiscoveredEndpointSettingsController.class)) {
                DiscoveredEndpointSettingsService discoveredEndpointService = ApplicationSingletonContainer.getBean(DiscoveredEndpointSettingsService.class);
                discoveredEndpointService.uninitialize();
            }
            if (ApplicationSingletonContainer.containsBean(DebugAreaStream.class)) {
                DebugAreaStream debugAreaStream = ApplicationSingletonContainer.getBean(DebugAreaStream.class);
                debugAreaStream.clear();
                debugAreaStream.getDebugPrintStream().close();
            }
        }
    }

    public static void main(String[] args) {
        final String outputBufferTarget = System.getProperty("os.name").toLowerCase().contains("win")
                ? "NUL:"
                : "/dev/null";
        try (PrintStream defaultPrint = new PrintStream(new FileOutputStream(outputBufferTarget))) {
            System.setOut(defaultPrint);
            Application.launch(RRTMDViewerDesktopApplication.class, args);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
