package com.thomsonreuters.upa.examples.newsviewer;

import java.awt.Component;
import java.awt.Font;
import java.util.Iterator;
import java.util.Vector;

import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;

/**
 * This class is responsible for building GUI to display news headline data.
 * 
 */
public class NewsHeadlineViewer
{
    private JList<Headline> _headlines;
    private DefaultListModel<Headline> _headlineModel;
    private JScrollPane _scroll;
    private Vector<Headline> _headlinesList;
    private NewsFilterSelector _filterSelector;

    public NewsHeadlineViewer(NewsStoryViewer storyViewer, Font font, NewsHandler MPHandler)
    {
        _filterSelector = new NewsFilterSelector(this);
        _headlinesList = new Vector<Headline>();
        _headlineModel = new DefaultListModel<Headline>();
        _headlines = new JList<Headline>(_headlineModel);
        _headlines.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        _headlines.setFont(font);
        _headlines.setFixedCellWidth(700);
        _headlines.addMouseListener(new NewsHeadlineListener(storyViewer, MPHandler));
        _scroll = new JScrollPane(_headlines);
        _scroll.setAutoscrolls(true);
    }

    Component component()
    {
        return _scroll;
    }

    NewsFilterSelector getFilterSelector()
    {
        return _filterSelector;
    }

    void addHeadline(Headline headline)
    {
        if (_filterSelector.checkFilters(headline))
        {
            _headlineModel.add(0, headline);
        }
        _headlinesList.add(headline);
        _filterSelector._codeDb.addAttribution(headline.getAttribution());
        _filterSelector._codeDb.addCompanies(headline.getCompanyCodes());
        _filterSelector._codeDb.addProducts(headline.getProdCodes());
        _filterSelector._codeDb.addLanguage(headline.getLang());
        _filterSelector._codeDb.addTopics(headline.getTopicCodes());
    }

    void applyFilter()
    {
        _headlineModel.clear();

        Iterator<Headline> iter = _headlinesList.iterator();
        while (iter.hasNext())
        {
            Headline headline = iter.next();
            if (_filterSelector.checkFilters(headline))
            {
                _headlineModel.add(0, headline);
            }
        }
    }
}
