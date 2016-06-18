#include <QApplication>
#include <QFile>
#include <QDebug>

#include "mainwindow.h"

//to simplify the data is already generated in slices of 30 minutes
static
int generateData(QMap<QDateTime, MarketProfile::Data> &data, int maxLineNo = 14)
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
        if (7 > tok.length()) {
            qWarning() << "Invalid line" << line;
            continue;
        }
        QDateTime dateTime = QDateTime::fromString(tok[0]+" "+tok[1], "dd.MM.yyyy hh:mm:ss");
        MarketProfile::Data item;
        item.open = tok[2].toDouble();
        item.high = tok[3].toDouble();
        item.low = tok[4].toDouble();
        item.close = tok[5].toDouble();
        item.volume = tok[6].toInt();
        data[dateTime] = item;
        if ((0 <= maxLineNo) && ((maxLineNo-1) < lineNo)) {
            break;
        }
    }
    return EXIT_SUCCESS;
}

static
void showMarketData(const QMap<QDateTime, MarketProfile::Data> &data)
{
    QMap<QDateTime, MarketProfile::Data>::const_iterator i = data.constBegin();
    int n = 0;
    for (; i != data.constEnd(); ++i) {
        MarketProfile::Data md = i.value();
        qInfo() << n++ << i.key() << md.open << md.high << md.low << md.close << md.volume;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QMap<QDateTime, MarketProfile::Data> data;
    if (EXIT_SUCCESS == generateData(data)) {
        //showMarketData(data);
        w.loadTimeSeries(data);
    }

    w.show();

    return a.exec();
}
