/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.newsviewer;

import java.awt.Font;

import javax.swing.BoxLayout;
import javax.swing.JFrame;

/**
 * This class is responsible for creating the Headline and Story aspects of the
 * GUI and bounding them to one main GUI, displaying the news headline and story
 * data.
 */

@SuppressWarnings("serial")
public class NewsViewerFrame extends JFrame
{

    private JFrame _frame = new JFrame("NewsViewer");

    private NewsStoryViewer _storyViewer;
    private NewsHeadlineViewer _headlineViewer;

    NewsViewerFrame(int fontSize, String fontInput, NewsHandler MPHandler)
    {
        _frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        Font font = new Font(fontInput, Font.PLAIN, fontSize);
        _frame.getFontMetrics(font);
        _frame.setFont(font);
        _frame.getContentPane().setLayout(new BoxLayout(_frame.getContentPane(), BoxLayout.Y_AXIS));

        _storyViewer = new NewsStoryViewer(font);
        _headlineViewer = new NewsHeadlineViewer(_storyViewer, font, MPHandler);

        _frame.getContentPane().add(_headlineViewer.getFilterSelector().component());
        _frame.getContentPane().add(_headlineViewer.component());
        _frame.getContentPane().add(_storyViewer.component());

        _frame.pack();
        _frame.setVisible(true);
    }

    public void addHeadline(Headline headline)
    {
        _headlineViewer.addHeadline(headline);
    }

    public NewsStoryViewer getStoryViewer()
    {
        return _storyViewer;
    }

}
