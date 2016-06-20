#include <QDebug>
#include <iostream>
#include <QtMath>
#include "marketprofile.h"

const char MarketProfile::_emptyChar = ' ';

MarketProfile::MarketProfile(QWidget *parent) :
  QCustomPlot(parent)
{
    clear();
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                    QCP::iSelectLegend | QCP::iSelectPlottables);
    xAxis->setSubTickCount(0);
    xAxis->setTickLength(0, 4);
    xAxis->setTickLabelRotation(20);
    xAxis->setAutoTicks(false);
    xAxis->setAutoTickLabels(false);
    _currentFont.setLetterSpacing(QFont::PercentageSpacing, 0);
    connect(this, &QCustomPlot::beforeReplot, this, &MarketProfile::updateItems);
}

//compute the height of the literal as the average daily range divided by MAP_RESOLUTION
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
    _currentYMax = max;

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

bool MarketProfile::loadTimeSeries(const QMap<QDateTime, MarketProfile::Data> &data,
                                   bool update)
{
    if (data.isEmpty()) {
        return false;
    }
    if (!update) {
        QCustomPlot::clearItems();
        clear();
    }
    QMap<QDateTime, MarketProfile::Data>::const_iterator i = data.constBegin();
    QVector<double> upper;
    QVector<double> lower;
    QDate currentDate;
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
    replot();
    return true;
}

void MarketProfile::displayItem()
{
    yAxis->setRange(_yMin-_letterHeight, _yMax+_letterHeight);

    int oldPointSize = _currentFont.pointSize();
    int pointSize = computeFontPointSize(_letterHeight);
    _currentFont.setPointSize(pointSize);

    int nbChars = 0;
    QVector<QCPItemText*> bars;
    for (int n = 0; n < _item.size(); ++n) {
        QString row = _item.at(n);
        QCPItemText *barText = new QCPItemText(this);
        setupItemText(barText, row, _xPos, _currentYMin+(n+1)*_letterHeight);
        if (nbChars < row.size()) {
            nbChars = row.size();
        }
        bars.push_back(barText);
    }
    _items[_xPos].bars = bars;
    _items[_xPos].letterHeight = _letterHeight;
    _items[_xPos].currentYMin = _currentYMin;
    _currentFont.setPointSize(oldPointSize);
    _tickVector.push_back(_xPos);
    xAxis->setTickVector(_tickVector);
    xAxis->setTickVectorLabels(_tickVectorLabels);
    //new x position
    _xPos += nbChars*_letterHeight;
    xAxis->setRange(0, _xPos);
    _item.clear();
}

bool MarketProfile::setupItemText(QCPItemText *itemText, const QString &text,
                                  double x, double y)
{
    itemText->setPositionAlignment(Qt::AlignLeft);
    itemText->position->setType(QCPItemPosition::ptPlotCoords);
    itemText->position->setCoords(x, y);
    itemText->setFont(_currentFont);
    itemText->setColor(_literalColor);
    itemText->setText(text);
    return addItem(itemText);
}

bool MarketProfile::setBackgroudColor(int red, int green, int blue)
{
    if (!isValidColor(red) || !isValidColor(green) || !isValidColor(blue)) {
        return false;
    }
    QBrush brush(Qt::SolidPattern);
    brush.setColor(QColor(red, green, blue));
    QCustomPlot::setBackground(brush);
    replot();
    return true;
}

bool MarketProfile::setLiteralColor(int red, int green, int blue)
{
    if (!isValidColor(red) || !isValidColor(green) || !isValidColor(blue)) {
        return false;
    }
    _literalColor.setRgb(red, green, blue);
    replot();
    return true;
}

bool MarketProfile::setXLabel(const QString &label)
{
    if (label.isEmpty()) {
        return false;
    }
    xAxis->setLabelColor(_labelColor);
    xAxis->setTickLabelColor(_labelColor);
    xAxis->setLabel(label);
    replot();
    return true;
}

bool MarketProfile::setYLabel(const QString &label)
{
    if (label.isEmpty()) {
        return false;
    }
    yAxis->setLabelColor(_labelColor);
    yAxis->setTickLabelColor(_labelColor);
    yAxis->setLabel(label);
    replot();
    return true;
}

bool MarketProfile::setLabelColor(int red, int green, int blue)
{
    if (!isValidColor(red) || !isValidColor(green) || !isValidColor(blue)) {
        return false;
    }
    _labelColor.setRgb(red, green, blue);
    setXLabel(xAxis->label());
    setYLabel(yAxis->label());
    replot();
    return true;
}

void MarketProfile::clear()
{
    _letterHeight = 0;
    _currentLiteral = 'A';
    _literalMatrix.clear();
    _yMin = -1;
    _yMax = -1;
    _item.clear();
    _xPos = 0;
    _currentYMin = -1;
    _currentYMax = -1;
    _tickVector.clear();
    _tickVectorLabels.clear();
    _tickVectorDates.clear();
    _indicators.clear();
    _items.clear();
}

bool MarketProfile::addIndicator(const QString &indicatorName,
                                 const QMap<QDateTime, double> &position)
{
    if (indicatorName.isEmpty() || position.isEmpty()) {
        return false;
    }
    if (_indicators.contains(indicatorName)) {
        return false;
    }
    //search input data in existing tick dates
    QDateTime key = position.firstKey();
    const QDate currentDate = key.date();
    int pos = 0;
    bool rc = findTickPosition(pos, currentDate);
    if (!rc) {
        return false;
    }
    //the position has been found
    QCPItemText *ind = new QCPItemText(this);
    _indicators[indicatorName] = ind;
    rc = setupItemText(ind, indicatorName, _tickVector.at(pos),
                         position[key]);
    if (rc) {
        replot();
    }
    return rc;
}

bool MarketProfile::updateIndicator(const QString &indicatorName,
                                    const QMap<QDateTime, double> &position)
{
    if (indicatorName.isEmpty() || position.isEmpty()) {
        return false;
    }
    //search for the indicator
    if (!_indicators.contains(indicatorName)) {
        return false;
    }
    //search input data in existing tick dates
    QDateTime key = position.firstKey();
    const QDate currentDate = key.date();
    int pos = 0;
    bool rc = findTickPosition(pos, currentDate);
    if (!rc) {
        return false;
    }
    //update indicator
    QCPItemText *ind = _indicators[indicatorName];
    ind->position->setCoords(_tickVector.at(pos), position[key]);
    replot();
    return true;
}

bool MarketProfile::updateIndicator(const QString &indicatorName, bool show)
{
    if (indicatorName.isEmpty()) {
        return false;
    }
    //search for the indicator
    if (!_indicators.contains(indicatorName)) {
        return false;
    }
    //show/hide indicator
    QCPItemText *ind = _indicators[indicatorName];
    ind->setVisible(show);
    replot();
    return true;
}

bool MarketProfile::removeIndicator(const QString &indicatorName)
{
    if (indicatorName.isEmpty()) {
        return false;
    }
    //search for the indicator
    if (!_indicators.contains(indicatorName)) {
        return false;
    }
    //remove indicator
    QCPItemText *ind = _indicators[indicatorName];
    bool rc = false;
    if (hasItem(ind)) {
        rc = removeItem(ind);
    }
    _indicators.remove(indicatorName);
    replot();
    return rc;
}

bool MarketProfile::findTickPosition(int &pos, const QDate &currentDate)
{
    pos = 0;
    for (; pos < _tickVectorDates.size(); ++pos) {
        if (currentDate == _tickVectorDates.at(pos)) {
            break;
        }
    }
    return (pos < _tickVectorDates.size());
}

void MarketProfile::updateItems()
{
    static int oldHeight = 0;
    if ((oldHeight != height()) && (0 < _yMin) && (0 < _yMax)) {
        oldHeight = height();
        //update the font for each item from current market profile
        QMap<double,Item>::const_iterator it = _items.cbegin();
        for (; it != _items.cend(); ++it) {
            double xPos = it.key();
            Item item = it.value();
            int pointSize = computeFontPointSize(item.letterHeight);
            QFont font(_currentFont);
            font.setPointSize(pointSize);
            for (int n = 0; n < item.bars.size(); ++n) {
                item.bars[n]->position->setCoords(xPos, item.currentYMin+(n+1)*item.letterHeight);
                item.bars[n]->setFont(font);
            }
        }
    }
}
