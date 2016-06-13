#include <QDebug>
#include <iostream>
#include <QtMath>
#include "marketprofile.h"

const char MarketProfile::_emptyChar = ' ';

MarketProfile::MarketProfile(QCustomPlot *customPlot) : _letterHeight(0),
        _currentLiteral('A'), _customPlot(customPlot),
        _yMin(-1), _yMax(-1) {}

//compute the height of the literal as the average daily range divided by 10
void MarketProfile::computeLiteralHeight(const QVector<double> &upper,
                                         const QVector<double> &lower)
{
    _letterHeight = 0;
    for (int i = 0; i < upper.length(); ++i) {
        _letterHeight += (upper.at(i)-lower.at(i));
    }
    _letterHeight /= (upper.length()*MAP_RESOLUTION);
    _currentLiteral = 'A';
}

//generate matrix, each matrix column is a price interval
void MarketProfile::initLiteralMatrix(const QVector<double> &upper,
                                      const QVector<double> &lower)
{
    double min = 0;
    double max = 0;
    findMinMax(min, max, upper, lower);
    int rows = qRound((max-min)/_letterHeight);
    int cols = upper.length();

    if (-1 == _yMin) {
        _yMin = min;
    } else if (_yMin > min) {
        _yMin = min;
    }
    if (-1 == _yMax) {
        _yMax = min;
    } else if (_yMax < max) {
        _yMax = max;
    }
    _lower = lower.at(0);

    _literalMatrix.resize(cols);
    for (int c = 0; c < cols; ++c) {
        _literalMatrix[c].resize(rows);
        int begin = qRound((lower.at(c)-min)/_letterHeight);
        int end = qRound((upper.at(c)-min)/_letterHeight)-1;
        for (int r = 0; r < rows; ++r) {
            if ((r >= begin) && (r <= end)) {
                _literalMatrix[c][r] = getLiteralAtIndex(c);
            } else {
                _literalMatrix[c][r] = _emptyChar;
            }
        }
    }
}

void MarketProfile::processCurrentDay()
{
    if (_literalMatrix.isEmpty() || _literalMatrix.at(0).isEmpty()) {
        return;
    }

    //project all other columns on the first column
    //and mark the cells that are projected
    bool projectionSuccessful = false;
    for (int c = 1; c < _literalMatrix.length(); ++c) {
        for (int r = 0; r < _literalMatrix.at(0).length(); ++r) {
            if ((_emptyChar != _literalMatrix.at(c).at(r)) && (_emptyChar == _literalMatrix.at(0).at(r))) {
                _literalMatrix[0][r] = _literalMatrix[c][r];
                _literalMatrix[c][r] = _emptyChar;
                projectionSuccessful = true;
            }
        }
    }
    if (!projectionSuccessful) {
        return;
    }

    QVector<char> bar = _literalMatrix.front();
    qInfo() << bar;

    if (NULL != _customPlot) {
        _customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                     QCP::iSelectLegend | QCP::iSelectPlottables);
        _customPlot->yAxis->setRange(_yMin, _yMax);
    }

    _literalMatrix.pop_front();
    incrementCurrentLiteral();
    processCurrentDay();
}

void MarketProfile::findMinMax(double &min, double &max, const QVector<double> &upper,
                const QVector<double> &lower)
{
    min = lower.at(0);
    for (int i = 1; i < lower.length(); ++i) {
        if (min > lower.at(i)) {
            min = lower.at(i);
        }
    }
    max = upper.at(0);
    for (int i = 1; i < upper.length(); ++i) {
        if (max < upper.at(i)) {
            max = upper.at(i);
        }
    }
}

void MarketProfile::dumpLiteralMatrix()
{
    for (int r = _literalMatrix.at(0).length()-1; r >= 0 ; --r) {
        for (int c = 0; c < _literalMatrix.length(); ++c) {
            std::cout << _literalMatrix.at(c).at(r) << " ";
        }
        std::cout << std::endl;
    }
}

void MarketProfile::display(const QMap<QDateTime, MarketProfile::Data> &data)
{
    QMap<QDateTime, MarketProfile::Data>::const_iterator i = data.constBegin();
    QVector<double> upper;
    QVector<double> lower;
    QDate currentDate;
    for (; i != data.constEnd(); ++i) {
        QDateTime dateTime = i.key();
        if ((currentDate != dateTime.date()) && !upper.isEmpty()) {
            //if the day has changed, process current day
            qInfo() << currentDate;
            process(upper, lower);
            upper.clear();
            lower.clear();
        }
        MarketProfile::Data md = i.value();
        upper.push_back(md.high);
        lower.push_back(md.low);
        currentDate = dateTime.date();
    }
    if (!upper.isEmpty()) {
        qInfo() << currentDate;
        process(upper, lower);
    }
}
