#include "candlestickchart.h"

CandlestickChart::CandlestickChart(QWidget *parent):
    QCustomPlot(parent)
{
    SetupGraph();
}

void CandlestickChart::SetFinancialMap(QSharedPointer<QCPFinancialDataContainer> finMap)
{
    ohlc->setData(finMap);
    this->rescaleAxes();
    this->replot();
}

void CandlestickChart::SetBackgroundToBlack()
{
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, Qt::black );
    plotGradient.setColorAt(1, Qt::black );
    setBackground(plotGradient);
}

void CandlestickChart::SetTicksToWhite()
{
    xAxis->setTickPen(QPen(Qt::white, 1));
    yAxis2->setTickPen(QPen(Qt::white, 1));
   // yAxis2->setSubTickCount(10);
    xAxis->setSubTickPen(QPen(Qt::white, 1));
    yAxis2->setSubTickPen(QPen(Qt::white, 1));
    xAxis->setTickLabelColor(Qt::white);
    yAxis2->setTickLabelColor(Qt::white);
}

void CandlestickChart::SetSubGrid()
{
    xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    yAxis2->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    yAxis2->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    xAxis->grid()->setSubGridVisible(true);
    yAxis2->grid()->setSubGridVisible(true);
    yAxis->grid()->setVisible(false);
    yAxis2->grid()->setVisible(true);
}

void CandlestickChart::AddSubPlot()
{
    QCPAxisRect *IndicatorRect = new QCPAxisRect(this);
    IndicatorRect->setMaximumSize(100000,100);
    plotLayout()->addElement(1,0,IndicatorRect);
}

void CandlestickChart::SetOhlc()
{
    ohlc = new QCPFinancial(xAxis, yAxis2);

    registerPlottable(ohlc);
    ohlc->setName("OHLC");

    QColor TradestationGreen=QColor(0,255,0,255);
    QColor TradeStationRed= QColor(255,0,0,255);

    QBrush TradestationBrush = QBrush(ohlc->brushNegative());
    TradestationBrush.setColor(TradestationGreen);

    ohlc->setBrushPositive(TradestationBrush);

    QPen TradeStationPen = QPen(ohlc->penNegative());
    TradeStationPen.setColor(TradestationGreen);
    ohlc->setPenPositive(TradeStationPen);
    TradeStationPen.setColor(TradeStationRed);
    ohlc->setPenNegative(TradeStationPen);
    TradestationBrush.setColor(TradeStationRed);
    ohlc->setBrushNegative(TradestationBrush );

    ohlc->setChartStyle(QCPFinancial::csHollowCandle);
    //ohlc->setChartStyle(QCPFinancial::csLineClose);

   // DataAdapter->setStockName("BA");
   // DataAdapter->ConvertToFinancialData(ohlc,_binSize);

    ohlc->setWidth(0.5*3600);
    ohlc->setTwoColored(true);
    rescaleAxes();
}

void CandlestickChart::SetXYAxis()
{
    //legend->setVisible(true);
    setInteraction(QCP::iRangeZoom,true);
    setInteraction(QCP::iRangeDrag, true);
    axisRect()->setRangeDragAxes(xAxis,yAxis2);
    axisRect()->setRangeZoomAxes(xAxis,yAxis2);

    xAxis->setBasePen(QPen(QColor(255,0,0,255)));
    xAxis->setTickLabels(true);
    //xAxis->setTickStep(3600); // 1 day tickstep
    xAxis->scaleRange(1.1, xAxis->range().center());
    //xAxis->setTickLabelType(QCPAxis::ltDateTime);
    //xAxis->setDateTimeSpec(Qt::UTC);
    //xAxis->setAutoTicks(true);
   // xAxis->setDateTimeFormat("dd. MMM. yy");
    xAxis->setTickLabelRotation(15);

    yAxis2->setVisible(true);
    yAxis2->axisRect()->setRangeZoom(Qt::Vertical);
    yAxis2->scaleRange(1.1, yAxis2->range().center());
    yAxis2->setBasePen(QPen(QColor(255,0,0,255)));
    //yAxis2->setScaleType(QCPAxis::stLogarithmic);

    // make axis rects' left side line up:
    QCPMarginGroup *group = new QCPMarginGroup(this);
    axisRect()->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    //volumeAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);
}

void CandlestickChart::SetupGraph()
{

    //bPaintBar= true;
    //bOverlay= true;

    SetOhlc();

   // SetIndicator();

   // SetPaintBar();

   // AddSubPlot();

    SetTicksToWhite();

    SetBackgroundToBlack();

    SetSubGrid();

    SetXYAxis();

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    /*connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(ctxMenu,SIGNAL(comboChanged(QString)),this,SLOT(on_cbStockSymbol_currentIndexChanged(QString)));
    connect(ctxMenu,SIGNAL(paintBarChanged(int)), this, SLOT(on_checkBoxPaintBar_stateChanged(int)));
    connect(ctxMenu,SIGNAL(overLayChanged(int)), this, SLOT(on_checkBoxOverlay_stateChanged(int)));
    dataTimer->start(0); // Interval 0 means to refresh as fast as possible*/
}
