#include "mainwindow.h"
#include <QApplication>
#include <QMap>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QtMath>

double randomValue(double limit)
{
    return (qrand()/(double)RAND_MAX)*limit;
}

int randomValue(int min, int max)
{
    return (int)(min + (max-min)*(qrand()/(double)RAND_MAX));
}

struct MarketData
{
    MarketData() : open(0), high(0), low(0), close(0), volume(0) {}
    double open;
    double high;
    double low;
    double close;
    int volume;
};

//to simplify the data is already generated in slices of 30 minutes
int generateRandomData(QMap<QDateTime, MarketData> &data, int maxLineNo = 13)
{
    QString filename = "/Users/bogdan/projects/draw_financial_data/DUK 30 Minutes.txt";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return EXIT_FAILURE;
    }
    data.clear();
    int lineNo = 0;
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (0 == lineNo++) {
            continue;
        }
        QList<QByteArray> tok = line.split(',');
        if (7 != tok.length()) {
            qWarning() << "Invalid line" << line;
            continue;
        }
        QDateTime dateTime = QDateTime::fromString(tok[0]+" "+tok[1], "dd.MM.yyyy hh:mm:ss");
        MarketData item;
        item.open = tok[2].toDouble();
        item.high = tok[3].toDouble();
        item.low = tok[4].toDouble();
        item.close = tok[5].toDouble();
        item.volume = tok[6].toInt();
        data[dateTime] = item;
        if (maxLineNo < lineNo) {
            break;
        }
    }
    return EXIT_SUCCESS;
}

void showMarketData(const QMap<QDateTime, MarketData> &data)
{
    QMap<QDateTime, MarketData>::const_iterator i = data.constBegin();
    int n = 0;
    for (; i != data.constEnd(); ++i) {
        MarketData md = i.value();
        qInfo() << n++ << i.key() << md.open << md.high << md.low << md.close << md.volume;
    }
}

class MarketDataProcessor {
public:
    MarketDataProcessor() : _letterHeight(0), _currentLiteral('A') {}
    void operator()(const QVector<double> &upper, const QVector<double> &lower) {
        computeLiteralHeight(upper, lower);
        initLiteralMatrix(upper, lower);
        processCurrentDay();
    }
private:
    void computeLiteralHeight(const QVector<double> &upper, const QVector<double> &lower);
    void initLiteralMatrix(const QVector<double> &upper, const QVector<double> &lower);
    void processCurrentDay();
    enum {MAP_RESOLUTION = 10};
    void findMinMax(double &min, double &max, const QVector<double> &upper,
                    const QVector<double> &lower);
    char getLiteralAtIndex(int index) {
        char out = _currentLiteral+index;
        if (out > 'Z') {
            out = 'A';
        }
        return out;
    }
    void incrementCurrentLiteral() {
        ++_currentLiteral;
        if (_currentLiteral > 'Z') {
            _currentLiteral = 'A';
        }
    }
    double _letterHeight;
    char _currentLiteral;
    static const char _emptyChar;
    QVector<QVector<char> > _literalMatrix;
};

const char MarketDataProcessor::_emptyChar = ' ';

//compute the height of the literal as the average daily range divided by 10
void MarketDataProcessor::computeLiteralHeight(const QVector<double> &upper, const QVector<double> &lower)
{
    _letterHeight = 0;
    for (int i = 0; i < upper.length(); ++i) {
        _letterHeight += (upper.at(i)-lower.at(i));
    }
    _letterHeight /= (upper.length()*MAP_RESOLUTION);
    _currentLiteral = 'A';
}

//generate matrix, each matrix column is a price interval
void MarketDataProcessor::initLiteralMatrix(const QVector<double> &upper, const QVector<double> &lower)
{
    double min = 0;
    double max = 0;
    findMinMax(min, max, upper, lower);
    int rows = qRound((max-min)/_letterHeight);
    int cols = upper.length();

    _literalMatrix.resize(cols);
    for (int c = 0; c < cols; ++c) {
        _literalMatrix[c].resize(rows);
        int begin = qRound((lower.at(c)-min)/_letterHeight);
        int end = qRound((upper.at(c)-min)/_letterHeight);
        for (int r = 0; r < rows; ++r) {
            if ((r >= begin) && (r <= end)) {
                _literalMatrix[c][r] = getLiteralAtIndex(c);
            } else {
                _literalMatrix[c][r] = _emptyChar;
            }
        }
    }
}

void MarketDataProcessor::processCurrentDay()
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

    qInfo() << _literalMatrix.front();
    _literalMatrix.pop_front();
    incrementCurrentLiteral();
    processCurrentDay();
}

void MarketDataProcessor::findMinMax(double &min, double &max, const QVector<double> &upper,
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

void displayMarketProfile(const QMap<QDateTime, MarketData> &data)
{
    QMap<QDateTime, MarketData>::const_iterator i = data.constBegin();
    QVector<double> upper;
    QVector<double> lower;
    QDate currentDate;
    MarketDataProcessor processor;
    for (; i != data.constEnd(); ++i) {
        QDateTime dateTime = i.key();
        if ((currentDate != dateTime.date()) && !upper.isEmpty()) {
            //if the day has changed, process current day
            processor(upper, lower);
            upper.clear();
            lower.clear();
        }
        MarketData md = i.value();
        upper.push_back(md.high);
        lower.push_back(md.low);
        currentDate = dateTime.date();
    }
    if (!upper.isEmpty()) {
        processor(upper, lower);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QMap<QDateTime, MarketData> data;
    if (EXIT_SUCCESS == generateRandomData(data)) {
        //showMarketData(data);
        displayMarketProfile(data);
    }

    return a.exec();
}
