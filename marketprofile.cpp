#include <QDebug>
#include <iostream>
#include <QtMath>
#include "marketprofile.h"

const char MarketProfile::_emptyChar = ' ';

MarketProfile::MarketProfile(QCustomPlot *customPlot, const QFont &currentFont) : _letterHeight(0),
  _currentLiteral('A'), _customPlot(customPlot),
  _yMin(-1), _yMax(-1), _xPos(0), _currentFont(currentFont),
  _currentYMin(-1)
{
    _customPlot->xAxis->setSubTickCount(0);
    _customPlot->xAxis->setTickLength(0, 4);
    _customPlot->xAxis->setTickLabelRotation(20);
    _customPlot->xAxis->setAutoTicks(false);
    _customPlot->xAxis->setAutoTickLabels(false);
    _currentFont.setLetterSpacing(QFont::PercentageSpacing, 0);
}

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
        _yMax = max;
    } else if (_yMax < max) {
        _yMax = max;
    }
    _currentYMin = min;

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
        displayItem();
        return;
    }

    QVector<char> bar = _literalMatrix.front();
    qDebug() << bar;

    if (_item.isEmpty()) {
        _item.resize(bar.size());
    }
    for (int n = 0; n < bar.size(); ++n) {
        if (_emptyChar != bar.at(n)) {
            _item[n].append(bar.at(n));
        }
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
    _tickVector.clear();
    _tickVectorLabels.clear();
    for (; i != data.constEnd(); ++i) {
        QDateTime dateTime = i.key();
        if ((currentDate != dateTime.date()) && !upper.isEmpty()) {
            //if the day has changed, process current day
            process(upper, lower, currentDate);
            upper.clear();
            lower.clear();
        }
        MarketProfile::Data md = i.value();
        upper.push_back(md.high);
        lower.push_back(md.low);
        currentDate = dateTime.date();
    }
    if (!upper.isEmpty()) {
        process(upper, lower, currentDate);
    }
}

void MarketProfile::displayItem()
{
    if (NULL != _customPlot) {
        _customPlot->yAxis->setRange(_yMin, _yMax);
        qDebug() << "y min" << _yMin << "y max" << _yMax;
        qDebug() << "current y min" << _currentYMin;

        int nbChars = 0;
        for (int n = 0; n < _item.size(); ++n) {
            QCPItemText *barText = new QCPItemText(_customPlot);
            _customPlot->addItem(barText);
            barText->setPositionAlignment(Qt::AlignLeft);
            barText->position->setType(QCPItemPosition::ptPlotCoords);
            barText->position->setCoords(_xPos, _currentYMin+n*_letterHeight);
            //_currentFont.setPixelSize(pixelSize);
            barText->setFont(_currentFont);
            QString row = _item.at(n);
            barText->setText(row);
            if (nbChars < row.size()) {
                nbChars = row.size();
            }
        }
        _tickVector.push_back(_xPos);
        _customPlot->xAxis->setTickVector(_tickVector);
        _customPlot->xAxis->setTickVectorLabels(_tickVectorLabels);
        //new x position
        _xPos += nbChars*5*_letterHeight;
        _customPlot->xAxis->setRange(0, _xPos);
    }
    _item.clear();
}
