package com.thomsonreuters.upa.examples.newsviewer;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

/**
 * This class provides a component to filter headline data.
 * 
 * It is responsible for the following actions:
 * <ul>
 * <li>Set up filter GUI
 * <li>Create and apply a filter expression to headline data
 * </ul>
 */
public class NewsFilterSelector
{
    
    /**
     * Instantiates a new news filter selector.
     *
     * @param newsHeadlineViewer the news headline viewer
     */
    public NewsFilterSelector(NewsHeadlineViewer newsHeadlineViewer)
    {
        _codeDb = new NewsCodeDb(this);
        _newsHeadlineViewer = newsHeadlineViewer;
        initGUI();
    }

    /**
     * Inits the GUI.
     */
    public void initGUI()
    {
        _panel = new JPanel();
        _panel.setLayout(new BoxLayout(_panel, BoxLayout.X_AXIS));

        _open = new JButton("Select filter");
        _open.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                openDialog();
            }
        });
        _panel.add(_open);

        _filter = new JTextField(40);
        _filter.setEditable(false);
        _panel.add(_filter);

        _selectionDialog = new JFrame("News filter");
        _selectionDialog.getContentPane().setLayout(new BoxLayout(
                                                            _selectionDialog.getContentPane(),
                                                            BoxLayout.Y_AXIS));

        Box selectionPanel = Box.createHorizontalBox();
        Box expressionPanel = Box.createHorizontalBox();

        JPanel attribtn = new JPanel();
        attribtn.setLayout(new BoxLayout(attribtn, BoxLayout.Y_AXIS));
        JLabel alabel = new JLabel("ATTRIBTN");
        alabel.setToolTipText("Attributions");
        _attribtn = new JList<String>(_codeDb._attributionLM);
        JScrollPane ascroll = new JScrollPane(_attribtn);
        ascroll.setAutoscrolls(true);
        attribtn.add(alabel);
        attribtn.add(ascroll);

        JPanel prod = new JPanel();
        prod.setLayout(new BoxLayout(prod, BoxLayout.Y_AXIS));
        JLabel plabel = new JLabel("PROD_CODE");
        plabel.setToolTipText("Product codes");
        _prod_code = new JList<String>(_codeDb._prodCodesLM);
        JScrollPane pscroll = new JScrollPane(_prod_code);
        pscroll.setAutoscrolls(true);
        prod.add(plabel);
        prod.add(pscroll);

        JPanel topic = new JPanel();
        topic.setLayout(new BoxLayout(topic, BoxLayout.Y_AXIS));
        JLabel tlabel = new JLabel("TOPIC_CODE");
        tlabel.setToolTipText("Topic codes");
        _topic_code = new JList<String>(_codeDb._topicCodesLM);
        JScrollPane tscroll = new JScrollPane(_topic_code);
        tscroll.setAutoscrolls(true);
        topic.add(tlabel);
        topic.add(tscroll);

        JPanel lang = new JPanel();
        lang.setLayout(new BoxLayout(lang, BoxLayout.Y_AXIS));
        JLabel llabel = new JLabel("LANG_IND");
        llabel.setToolTipText("Languages");
        _lang_ind = new JList<String>(_codeDb._langsLM);
        JScrollPane lscroll = new JScrollPane(_lang_ind);
        lscroll.setAutoscrolls(true);
        lang.add(llabel);
        lang.add(lscroll);

        JPanel co = new JPanel();
        co.setLayout(new BoxLayout(co, BoxLayout.Y_AXIS));
        JLabel clabel = new JLabel("CO_IDS");
        clabel.setToolTipText("Company codes");
        _co_ids = new JList<String>(_codeDb._companyCodesLM);
        JScrollPane cscroll = new JScrollPane(_co_ids);
        lscroll.setAutoscrolls(true);
        co.add(clabel);
        co.add(cscroll);

        selectionPanel.add(attribtn);
        selectionPanel.add(lang);
        selectionPanel.add(topic);
        selectionPanel.add(prod);
        selectionPanel.add(co);

        expressionPanel.add(new JLabel("Expression"));
        _expression = new JTextField(10);
        _expression.setEditable(false);
        expressionPanel.add(_expression);

        JButton addButton = new JButton("Apply");
        addButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                applyExpression();
            }
        });
        expressionPanel.add(addButton);
        JButton applyButton = new JButton("Clear");
        applyButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                clearExpression();
            }
        });
        expressionPanel.add(applyButton);

        _selectionDialog.getContentPane().add(selectionPanel);
        _selectionDialog.getContentPane().add(expressionPanel);

        _selectionDialog.pack();
        _selectionDialog.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);

    }

    void clearExpression()
    {
        _attribtn.clearSelection();
        _lang_ind.clearSelection();
        _topic_code.clearSelection();
        _prod_code.clearSelection();
        _co_ids.clearSelection();
        _expression.setText("");
        _regexFilter = "";
        _newsHeadlineViewer.applyFilter();
    }

    void applyExpression()
    {
        StringBuilder expression = new StringBuilder(100);
        appendSelection(expression, "ATTRIBTN", _attribtn);
        appendSelection(expression, "LANG_IND", _lang_ind);
        appendSelection(expression, "TOPIC_CODE", _topic_code);
        appendSelection(expression, "PROD_CODE", _prod_code);
        appendSelection(expression, "CO_IDS", _co_ids);
        applyRegexFilter();
        _expression.setText(expression.toString());
        _newsHeadlineViewer.applyFilter();
    }

    void applyRegexFilter()
    {
        StringBuilder s = new StringBuilder();
        s.append(".* ");
        s.append(getSelectedValue(_attribtn));
        s.append(" .*\\|.* ");
        s.append(getSelectedValue(_topic_code));
        s.append(" .*\\|.* ");
        s.append(getSelectedValue(_co_ids));
        s.append(" .*\\|.* ");
        s.append(getSelectedValue(_prod_code));
        s.append(" .*\\|.* ");
        s.append(getSelectedValue(_lang_ind));
        s.append(" .*");
        _regexFilter = s.toString();
    }

    private String getSelectedValue(JList<String> list)
    {
        List<String> a = list.getSelectedValuesList();
        if (a.size() == 0)
            return "";
        StringBuilder s = new StringBuilder();
        s.append("(");
        for (int i = 0; i < a.size();)
        {
            s.append(escapeRegex(a.get(i)));
            if (++i < a.size())
                s.append("|");
        }
        s.append(")");
        return s.toString();
    }

    boolean checkFilters(Headline headline)
    {
        if (_regexFilter == null || _regexFilter.equals(""))
        {
            return true;
        }
        StringBuilder s = new StringBuilder();
        s.append("  ");
        s.append(((headline.getAttribution() == null) ? " " : headline.getAttribution()));
        s.append("  |  ");
        s.append(((headline.getTopicCodes() == null) ? " " : headline.getTopicCodes()));
        s.append("  |  ");
        s.append(((headline.getCompanyCodes() == null) ? " " : headline.getCompanyCodes()));
        s.append("  |  ");
        s.append(((headline.getProdCodes() == null) ? " " : headline.getProdCodes()));
        s.append("  |  ");
        s.append(((headline.getLang() == null) ? " " : headline.getLang()));
        s.append("  ");

        String headlineStr = s.toString();
        return headlineStr.matches(_regexFilter);
    }

    void appendSelection(StringBuilder expression, String attrib, JList<String> _attribtn2)
    {
        List<String> a = _attribtn2.getSelectedValuesList();
        if (a.size() == 0)
            return;
        if (expression.length() > 0)
            expression.append(" && ");
        expression.append("(");
        expression.append(attrib);
        expression.append(" == ");
        for (int i = 0; i < a.size();)
        {
            expression.append(a.get(i));
            if (++i < a.size())
                expression.append(" | ");
        }
        expression.append(")");
    }

    /**
     * Component.
     *
     * @return the component
     */
    public Component component()
    {
        return _panel;
    }

    /**
     * Open dialog.
     */
    public void openDialog()
    {
        _codeDb.dump();
        _selectionDialog.setVisible(true);
    }

    private static final Pattern GRAB_SP_CHARS = Pattern.compile("([\\\\*+\\[\\](){}\\$.?\\^|])");

    /**
     * Replaces special characters of regular expression.
     *
     * @param s the s
     * @return the string
     */
    public static String escapeRegex(String s)
    {
        if (s == null)
            return s;
        Matcher match = GRAB_SP_CHARS.matcher(s);
        return match.replaceAll("\\\\$1");
    }

    private JFrame _selectionDialog;
    private JTextField _expression;
    private JPanel _panel;
    private JTextField _filter;
    private JButton _open;
    JList<String> _prod_code, _lang_ind, _co_ids, _topic_code, _attribtn;

    private NewsHeadlineViewer _newsHeadlineViewer;
    NewsCodeDb _codeDb;
    private String _regexFilter;
}
