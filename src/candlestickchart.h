#ifndef CANDLESTICKCHART_H
#define CANDLESTICKCHART_H
#include "qcustomplot.h"

class CandlestickChart : public QCustomPlot
{
public:
    CandlestickChart(QWidget *parent);

    void SetFinancialMap(QSharedPointer<QCPFinancialDataContainer> finMap);

private:

    QCPFinancial *ohlc;
    void SetupGraph();
    void SetBackgroundToBlack();
    void SetTicksToWhite();
    void SetXYAxis();
    void SetOhlc();
    void AddSubPlot();
    void SetSubGrid();    
};

#endif // CANDLESTICKCHART_H
