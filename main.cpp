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
    void computeLiteralHeight(const QVector<double> &upper, const QVector<double> &lower);
    void processCurrentDay(QVector<double> &upper, QVector<double> &lower);
private:
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
};

//compute the height of the literal as the average daily range divided by 10
void MarketDataProcessor::computeLiteralHeight(const QVector<double> &upper, const QVector<double> &lower)
{
    _letterHeight = 0;
    for (int i = 0; i < upper.length(); ++i) {
        _letterHeight += (upper.at(i)-lower.at(i));
    }
    _letterHeight /= (upper.length()*10);
    _currentLiteral = 'A';
}

void MarketDataProcessor::processCurrentDay(QVector<double> &upper, QVector<double> &lower)
{
    if (upper.isEmpty()) {
        return;
    }

    QVector<char> charBar;
    int numberOfLiterals = (int)((upper.at(0)-lower.at(0))/_letterHeight);
    for (int i = 0; i < numberOfLiterals; ++i) {
        charBar.push_back(_currentLiteral);
    }
    for (int i = 1; i < upper.length(); ++i) {
        double diff = upper.at(i)-upper.at(0);
        if (0 < diff) {
            numberOfLiterals = qCeil(diff/_letterHeight);
            for (int j = 0; j < numberOfLiterals; ++j) {
                charBar.push_front(getLiteralAtIndex(i));
            }
            upper[0] = upper.at(i);
            upper[i] -= diff;
        }
        diff = lower.at(0)-lower.at(i);
        if (0 < diff) {
            numberOfLiterals = qCeil(diff/_letterHeight);
            for (int j = 0; j < numberOfLiterals; ++j) {
                charBar.push_back(getLiteralAtIndex(i));
            }
            lower[0] = lower.at(i);
            lower[i] += diff;
        }
    }
    upper.pop_front();
    lower.pop_front();
    incrementCurrentLiteral();
    qInfo() << charBar;
    processCurrentDay(upper, lower);
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
            processor.computeLiteralHeight(upper, lower);
            processor.processCurrentDay(upper, lower);
        }
        MarketData md = i.value();
        upper.push_back(md.high);
        lower.push_back(md.low);
        currentDate = dateTime.date();
    }
    if (!upper.isEmpty()) {
        processor.computeLiteralHeight(upper, lower);
        processor.processCurrentDay(upper, lower);
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
