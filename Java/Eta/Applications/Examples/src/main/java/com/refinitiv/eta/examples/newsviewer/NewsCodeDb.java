/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.newsviewer;

import java.util.Enumeration;
import java.util.StringTokenizer;

import javax.swing.DefaultListModel;
import javax.swing.JList;

/**
 * This class provides data structure to keep all filter keys. These keys are
 * used for filtering the news headline data.
 * 
 */
public class NewsCodeDb
{
    public NewsCodeDb(NewsFilterSelector newsFilterSelector)
    {
        _newsFilterSelector = newsFilterSelector;
        _attributionLM = new DefaultListModel<String>();
        _attributionLM.addElement("RTRS");
        _topicCodesLM = new DefaultListModel<String>();
        _companyCodesLM = new DefaultListModel<String>();
        _prodCodesLM = new DefaultListModel<String>();
        _langsLM = new DefaultListModel<String>();
        _langsLM.addElement("DE");
        _langsLM.addElement("EN");
        _langsLM.addElement("ES");
        _langsLM.addElement("FR");
        _langsLM.addElement("JP");
        _langsLM.addElement("ZH");
    }

    /**
     * prints all filter keys
     * 
     */
    public void dump()
    {
        dump("Attributions", _attributionLM);
        dump("Topic codes", _topicCodesLM);
        dump("Company codes", _companyCodesLM);
        dump("Product codes", _prodCodesLM);
        dump("Languages", _langsLM);
    }

    void dump(String title, DefaultListModel<String> values)
    {
        System.out.print(title);
        int i = 0;
        for (Enumeration<?> e = values.elements(); e.hasMoreElements();)
        {
            if (i++ % 10 == 0)
            {
                System.out.println();
            }
            System.out.print("\t");
            System.out.print((String)e.nextElement());
        }
        System.out.println();
    }

    public void addAttribution(String attribution)
    {
        add(attribution, _attributionLM, _newsFilterSelector._attribtn);
    }

    public void addTopics(String topics)
    {
        if (topics == null)
        {
            return;
        }
        StringTokenizer tokenizer = new StringTokenizer(topics);
        while (tokenizer.hasMoreTokens())
        {
            add(tokenizer.nextToken(), _topicCodesLM, _newsFilterSelector._topic_code);
        }
    }

    public void addProducts(String products)
    {
        if (products == null)
        {
            return;
        }
        StringTokenizer tokenizer = new StringTokenizer(products);
        while (tokenizer.hasMoreTokens())
        {
            add(tokenizer.nextToken(), _prodCodesLM, _newsFilterSelector._prod_code);
        }
    }

    public void addCompanies(String companies)
    {
        if (companies == null)
        {
            return;
        }
        StringTokenizer tokenizer = new StringTokenizer(companies);
        while (tokenizer.hasMoreTokens())
        {
            add(tokenizer.nextToken(), _companyCodesLM, _newsFilterSelector._co_ids);
        }
    }

    public void addLanguage(String lang)
    {
        add(lang, _langsLM, _newsFilterSelector._lang_ind);
    }

    void add(String value, DefaultListModel<String> valuesLM, JList<?> jList)
    {
        if ((value == null) || (value.length() == 0))
        {
            return;
        }
        int i = 0;
        for (Enumeration<?> e = valuesLM.elements(); e.hasMoreElements(); i++)
        {
            String s = (String)e.nextElement();
            int compare = s.compareTo(value);
            if (compare == 0)
            {
                return;
            }
            else if (compare > 0)
            {
                valuesLM.add(i, value);
                if (jList.isSelectedIndex(i))
                {
                    jList.removeSelectionInterval(i, i);
                }
                return;
            }
        }
        valuesLM.addElement(value);
    }

    DefaultListModel<String> _attributionLM;
    DefaultListModel<String> _topicCodesLM;
    DefaultListModel<String> _companyCodesLM;
    DefaultListModel<String> _prodCodesLM;
    DefaultListModel<String> _langsLM;
    NewsFilterSelector _newsFilterSelector;
}
