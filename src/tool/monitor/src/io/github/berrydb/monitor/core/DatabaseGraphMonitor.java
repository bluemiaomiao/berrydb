/*******************************************************************************
 Copyright (C) 2021 BerryDB Software Inc.
 This application is free software: you can redistribute it and/or modify it
 under the terms of the GNU Affero General Public License, Version 3, as
 published by the Free Software Foundation.

 This application is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR PARTICULAR PURPOSE, See the GNU Affero General Public License for more
 details.

 You should have received a copy of the GNU Affero General Public License along
 with this application. If not, see <http://www.gnu.org/license/>
 *******************************************************************************/

package io.github.berrydb.monitor.core;

import org.jfree.ui.ApplicationFrame;

import java.awt.event.*;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Rectangle;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.Timer;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.DateTickUnit;
import org.jfree.chart.axis.DateTickUnitType;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.NumberTickUnit;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYSplineRenderer;
import org.jfree.data.time.Second;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.ui.ApplicationFrame;
import org.jfree.ui.RefineryUtilities;

import java.awt.event.ActionListener;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.Vector;

@SuppressWarnings("serial")
public class DatabaseGraphMonitor extends ApplicationFrame implements ActionListener {
    private final int TIME_PERIOD = 1000;
    private final int DOT_TIME = 10;
    private Map<TimeSeries, Database> timeSeriesMap = null;
    private Vector<Database> emeralddbVec;
    private TimeSeriesCollection localTimeSeriesCollection;
    private Vector<String> serverVec;
    private int ySize = 0;
    private String server = null;
    private boolean isMonitoring = false;
    private boolean isServerRunning = false;
    private DataGenerator dataGeneratorTimer;
    private JLabel ipLabel;
    private JTextField ipTextField;
    private JLabel portLabel;
    private JTextField portTextField;

    private JFreeChart jfreechart = null;

    private JPanel panel = null;
    private JPanel leftPanel = null;
    private JList serverList = null;
    private JPanel lowRightPanel = null;
    private JPanel serverPanel = null;
    private JPanel controlPanel = null;
    private ChartPanel chartPanel = null;

    private JButton addButton = null;
    private JButton finishButton = null;
    private JButton monitorButton = null;
    private JButton stopButton = null;
    private JPopupMenu popupMenu = null;

    private JTextArea messageLabel = null;

    public class DataGenerator extends Timer implements ActionListener {
        DataGenerator(int i) {
            super(i, null);
            addActionListener(this);
        }

        public void actionPerformed(ActionEvent actionevent) {
            Set<Map.Entry<TimeSeries, Database>> entrySet = timeSeriesMap.entrySet();

            int max = 0;
            for (Database edb : emeralddbVec) {
                edb.getUpdateTimes();
                int subMax = edb.maxRecordNumber();
                if (max < subMax) {
                    max = subMax;
                }
            }

            if (0 == max) {
                setVValueAxis(100, 100 / 25);
            } else {
                setVValueAxis(max + 50, (max + 50) / 25);
            }
            panel.updateUI();
            for (Entry<TimeSeries, Database> entry : entrySet) {
                TimeSeries ts = entry.getKey();
                Database edb = entry.getValue();
                int times = edb.getCurInsertTimes();
                if (-1 == times) {
                    isServerRunning = false;
                    end();
                    break;
                }
                ts.addOrUpdate(new Second(), (double) times);
            }
        }
    }

    public static void main(String[] args) {
        DatabaseGraphMonitor monitor = new DatabaseGraphMonitor(" Emeralddb Data Insert Monitor ");
        monitor.pack();
        RefineryUtilities.centerFrameOnScreen(monitor);
        monitor.setVisible(true);
    }

    public DatabaseGraphMonitor(String s) {
        super(s);

        localTimeSeriesCollection = new TimeSeriesCollection();
        timeSeriesMap = new HashMap<TimeSeries, Database>();

        addButton = new JButton("add");
        addButton.addActionListener(this);
        finishButton = new JButton("finish");
        finishButton.addActionListener(this);
        monitorButton = new JButton("monitor");
        monitorButton.setEnabled(false);
        monitorButton.addActionListener(this);
        stopButton = new JButton("end");
        stopButton.setEnabled(false);
        stopButton.addActionListener(this);
        messageLabel = new JTextArea("");

        popupMenu = new JPopupMenu();
        JMenuItem deleteItem = new JMenuItem("delete");
        popupMenu.add(deleteItem);
        deleteItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                deleteServerFromList(e);
            }
        });

        ipLabel = new JLabel("IP:");
        portLabel = new JLabel("PORT:");
        ipTextField = new JTextField(12);
        ipTextField.setText("localhost");
        portTextField = new JTextField(5);
        portTextField.setText("48127");

        serverList = new JList(serverVec);
        panel = new JPanel();
        leftPanel = new JPanel();
        lowRightPanel = new JPanel();
        serverPanel = new JPanel();
        controlPanel = new JPanel();

        createComponent(1000, 600);
        this.setContentPane(panel);

        KeyStroke strokeAdd = KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, InputEvent.CTRL_MASK, true);
        addButton.registerKeyboardAction(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                addButton.grabFocus();
                add();
                portTextField.grabFocus();
            }
        }, strokeAdd, JComponent.WHEN_IN_FOCUSED_WINDOW);


        serverList.addMouseListener(new MouseListener() {
            @Override
            public void mouseEntered(MouseEvent e) {

            }

            @Override
            public void mouseExited(MouseEvent e) {

            }

            @Override
            public void mouseClicked(MouseEvent e) {

            }

            @Override
            public void mousePressed(MouseEvent e) {

            }

            @Override
            public void mouseReleased(MouseEvent e) {
                selectServer(e);
            }
        });

        this.addWindowStateListener(new WindowStateListener() {
            @Override
            public void windowStateChanged(WindowEvent e) {
                mainWindowStateChanged(e);
            }
        });
        this.addComponentListener(new ComponentListener() {
            @Override
            public void componentHidden(ComponentEvent e) {

            }

            @Override
            public void componentMoved(ComponentEvent e) {

            }

            @Override
            public void componentResized(ComponentEvent e) {
                mainWindowStateResized(e);
            }

            @Override
            public void componentShown(ComponentEvent e) {

            }
        });

        emeralddbVec = new Vector<Database>();
    }

    private void clearErrorMsg(MouseEvent e) {

    }

    private void selectServer(MouseEvent e) {
        if (e.isPopupTrigger() && serverList.getSelectedIndex() != -1) {
            server = serverList.getSelectedValue().toString();
            popupMenu.show(e.getComponent(), e.getX(), e.getY());
        }
    }

    private void deleteServerFromList(ActionEvent e) {
        if (!finishButton.isEnabled()) {
            showError("can not delete the server after finish.");
            return;
        }
        serverVec.remove(server);
        serverList.updateUI();
    }

    private void mainWindowStateResized(ComponentEvent e) {
        serverPanel.removeAll();
        controlPanel.removeAll();
        lowRightPanel.removeAll();
        leftPanel.removeAll();
        panel.removeAll();
        createComponent(this.getWidth() - 10, this.getHeight() - 30);
        panel.updateUI();
    }

    private void mainWindowStateChanged(WindowEvent e) {
        int newState = e.getNewState();
        switch (newState) {
            case MAXIMIZED_BOTH:
                serverPanel.removeAll();
                controlPanel.removeAll();
                lowRightPanel.removeAll();
                leftPanel.removeAll();
                panel.removeAll();
                createComponent(this.getWidth() - 10, this.getHeight() - 30);
                break;
            case NORMAL:
                panel.removeAll();
                serverPanel.removeAll();
                controlPanel.removeAll();
                lowRightPanel.removeAll();
                leftPanel.removeAll();
                panel.removeAll();
                createComponent(1000, 600);
                break;
            default:
                createComponent(1000, 600);
                break;
        }
    }

    private void placeLeftPanel(int x, int y, int width, int heigth) {
        Rectangle leftRect = new Rectangle(x, y, width, heigth);
        leftPanel.setBounds(leftRect);
        leftPanel.setBorder(BorderFactory.createTitledBorder("BerryDB Moinitor Chart"));

        if (chartPanel != null) {
            placeJfreeChart(leftRect.x, leftRect.y,
                    leftRect.width, leftRect.height);
        }
        panel.add(leftPanel);
    }

    private void placeJfreeChart(int x, int y, int width, int height) {
        Rectangle chartPanelRect = new Rectangle(x, y, width, height);
        chartPanel.setBounds(chartPanelRect);
        leftPanel.add(chartPanel);
        leftPanel.updateUI();
    }

    private void placerightPannel(int x, int y, int width, int height) {
        int lowRightPanelHeight = 220;
        int controlPanelHeight = 40;
        int serverPanelHeight = 40;

        Rectangle rightRect = new Rectangle(x, y, width, height - lowRightPanelHeight - 10);
        //serverList.setBounds( rightRect );
        serverList.setBorder(BorderFactory.createTitledBorder("Server List"));
        JScrollPane serverListScroll = new JScrollPane(serverList);
        serverListScroll.setBounds(rightRect);
        serverListScroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        panel.add(serverListScroll);


        Rectangle lowRightRect = new Rectangle(x, height - lowRightPanelHeight, width, lowRightPanelHeight);
        lowRightPanel.setBounds(lowRightRect);
        panel.add(lowRightPanel);

        Rectangle serverPanelRect = new Rectangle(x, (int) lowRightRect.getY() + 5, width, serverPanelHeight);
        serverPanel.setBounds(serverPanelRect);
        serverPanel.setLayout(new BoxLayout(serverPanel, BoxLayout.X_AXIS));

        lowRightPanel.add(serverPanel);

        serverPanel.add(ipLabel);
        serverPanel.add(Box.createRigidArea(new Dimension(15, 10)));
        serverPanel.add(ipTextField);
        serverPanel.add(Box.createRigidArea(new Dimension(18, 10)));
        serverPanel.add(portLabel);
        serverPanel.add(Box.createRigidArea(new Dimension(13, 10)));
        serverPanel.add(portTextField);

        Rectangle controlPanelRect = new Rectangle(x,
                (int) serverPanelRect.getY() + (int) serverPanelRect.getHeight() + 5, width, controlPanelHeight);
        controlPanel.setBounds(controlPanelRect);
        controlPanel.setLayout(new BoxLayout(controlPanel, BoxLayout.X_AXIS));

        lowRightPanel.add(controlPanel);

        controlPanel.add(addButton);
        controlPanel.add(Box.createRigidArea(new Dimension(3, 10)));
        controlPanel.add(finishButton);
        controlPanel.add(Box.createRigidArea(new Dimension(3, 10)));
        controlPanel.add(monitorButton);
        controlPanel.add(Box.createRigidArea(new Dimension(3, 10)));
        controlPanel.add(stopButton);

        messageLabel.setEditable(false);
        messageLabel.setLineWrap(true);
        messageLabel.setForeground(Color.RED);
        JScrollPane scroll = new JScrollPane(messageLabel);
        scroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        scroll.setPreferredSize(new Dimension(width, (int) (height - controlPanelRect.getY() - 13)));
        lowRightPanel.add(scroll);
    }

    private void createComponent(int width, int height) {
        panel.setLayout(null);
        Dimension panelSize = new Dimension(width, height);
        panel.setPreferredSize(panelSize);

        int rightPanelWidth = 300;
        int marginWidth = 5;
        int marginHeight = 5;
        placeLeftPanel(marginWidth, marginWidth, width - rightPanelWidth, height - marginHeight);
        placerightPannel(width - rightPanelWidth + marginWidth, marginWidth, rightPanelWidth - marginWidth, height - marginHeight);
    }

    private void createJfreechart() {
        if (jfreechart == null) {
            jfreechart = ChartFactory.createTimeSeriesChart(
                    "BerryDB Data Insert Monitor",
                    "",
                    "Record Per Unit",
                    localTimeSeriesCollection,
                    true, true, true);

            int max = 0;
            for (int i = 0; i < emeralddbVec.size(); i++) {
                Database database = emeralddbVec.get(i);
                int number = database.getUpdateTimes();
                if (max < number) {
                    max = number;
                }
            }
            if (max == 0) {
                max = 100;
            }
            ySize = max;
            setJfreeChartParameter(ySize, ySize / 25);
            chartPanel = new ChartPanel(jfreechart);
            chartPanel.setMouseZoomable(false);
            chartPanel.setPopupMenu(new DatabasePopupMenu());

            placeJfreeChart((int) leftPanel.getX(),
                    (int) leftPanel.getY(),
                    (int) leftPanel.getWidth(),
                    (int) leftPanel.getHeight());
        }
    }

    private void setJfreeChartParameter(int maxNumber, int unit) {
        if (null == jfreechart) {
            System.out.println("jfree chart is not initialized.");
            return;
        }
        XYPlot plot = jfreechart.getXYPlot();
        XYSplineRenderer splineRenderer = new XYSplineRenderer();
        splineRenderer.setSeriesStroke(0, new BasicStroke(1.0F, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND, 10.0F));
        plot.setRenderer(splineRenderer);
        plot.setDomainGridlinesVisible(false);
        plot.setRangeGridlinesVisible(true);
        plot.setBackgroundAlpha(0.3f);
        plot.setRangeGridlinePaint(Color.BLACK);
        plot.setDomainGridlinePaint(Color.BLACK);

        setVValueAxis(maxNumber, unit);

        DateAxis domainAxis = (DateAxis) plot.getDomainAxis();
        DateTickUnit tickUnit = new DateTickUnit(DateTickUnitType.SECOND, DOT_TIME, new SimpleDateFormat("ss"));
        domainAxis.setTickUnit(tickUnit);
    }

    private void setVValueAxis(int maxNumber, int unit) {
        if (null == jfreechart) {
            System.out.println("jfree chart is not initialized.");
            return;
        }

        XYPlot plot = jfreechart.getXYPlot();
        NumberAxis vValueAxis = (NumberAxis) plot.getRangeAxis();
        vValueAxis.setUpperBound(maxNumber);
        vValueAxis.setLowerBound(0);
        vValueAxis.setAutoTickUnitSelection(false);
        NumberTickUnit nt = new NumberTickUnit(unit);
        vValueAxis.setTickUnit(nt);
    }

    private void add() {

        String ip = ipTextField.getText();
        String port = portTextField.getText();
        String strServer = ip + ":" + port;
        for (String tmp : serverVec) {
            if (strServer.equals(tmp)) {
                showError("the server is also exist.");
                return;
            }
        }

        Database database = new Database(ip, Integer.parseInt(port));
        if (database.start()) {
            serverVec.add(strServer);
            serverList.updateUI();
            emeralddbVec.add(database);
        } else {
            showError(database.getErrorMsg());
        }
    }

    private void showError(String error) {
        if (messageLabel.getLineCount() > 50) {
            messageLabel.setText("");
        }
        messageLabel.append(error);
        messageLabel.append("\n");
        messageLabel.paintImmediately(messageLabel.getBounds());
    }

    private void monitor() {
        System.out.println("monitor start");
        if (serverVec.size() > 0) {
            dataGeneratorTimer = this.new DataGenerator(TIME_PERIOD);
            dataGeneratorTimer.start();
            System.out.println("start timer");
            isMonitoring = true;
            isServerRunning = true;
        } else {
            showError("there are no server connecting.");
            return;
        }
        monitorButton.setEnabled(false);
        stopButton.setEnabled(true);
    }

    private void end() {

        serverVec.removeAllElements();
        serverList.updateUI();

        emeralddbVec.removeAllElements();
        timeSeriesMap.clear();
        localTimeSeriesCollection.removeAllSeries();
        if (isMonitoring) {
            dataGeneratorTimer.stop();
        }
        isMonitoring = false;
        addButton.setEnabled(true);
        finishButton.setEnabled(true);
        monitorButton.setEnabled(true);
        stopButton.setEnabled(false);
    }

    private void finish() {
        if (emeralddbVec.size() <= 0) {
            showError("there are no server connecting.");
        }

        for (Database database : emeralddbVec) {
            TimeSeries timeSeries = new TimeSeries(database.getIp() + ":" + database.getPort());
            timeSeries.setMaximumItemAge(40);
            timeSeries.add(new Second(), 0.0f);
            timeSeriesMap.put(timeSeries, database);
            localTimeSeriesCollection.addSeries(timeSeries);
        }
        createJfreechart();
    }


    public void actionPerformed(ActionEvent e) {
        if ((JButton) e.getSource() == monitorButton) {
            monitor();
        } else if ((JButton) e.getSource() == stopButton) {
            end();
        } else if ((JButton) e.getSource() == addButton) {
            add();
        } else if ((JButton) e.getSource() == finishButton) {
            addButton.setEnabled(false);
            finishButton.setEnabled(false);
            monitorButton.setEnabled(true);
            stopButton.setEnabled(true);
            finish();
        }
    }
}
