#ifndef MARKET_PROFILE_H_
#define MARKET_PROFILE_H_

#include <QVector>
#include <QMap>
#include <QDateTime>
#include "qcustomplot.h"

class MarketProfile : public QCustomPlot {
public:
    struct Data
    {
        Data() : open(0), high(0), low(0), close(0), volume(0) {}
        double open;
        double high;
        double low;
        double close;
        int volume;
    };
    explicit MarketProfile(QWidget *parent);
    void display(const QMap<QDateTime, MarketProfile::Data> &data);
private:
    enum {MAP_RESOLUTION = 10};
    void process(const QVector<double> &upper, const QVector<double> &lower,
                    const QDate &currentDate, bool dump = false) {
        _tickVectorLabels.push_back(currentDate.toString("MMM d yyyy"));
        computeLiteralHeight(upper, lower);
        initLiteralMatrix(upper, lower);
        if (dump) {
            dumpLiteralMatrix();
        }
        processCurrentDay();
    }
    void dumpLiteralMatrix();
    void computeLiteralHeight(const QVector<double> &upper, const QVector<double> &lower);
    void initLiteralMatrix(const QVector<double> &upper, const QVector<double> &lower);
    void processCurrentDay();
    void findMinMax(double &min, double &max, const QVector<double> &upper,
                    const QVector<double> &lower);
    char getLiteralAtIndex(int index) {
        char out = _currentLiteral;
        for (int n = 1; n <= index; ++n) {
            ++out;
            if (out > 'Z') {
                out = 'A';
            }
        }
        return out;
    }
    void incrementCurrentLiteral() {
        ++_currentLiteral;
        if (_currentLiteral > 'Z') {
            _currentLiteral = 'A';
        }
    }
    void displayItem();
    double _letterHeight;
    char _currentLiteral;
    static const char _emptyChar;
    QVector<QVector<char> > _literalMatrix;
    double _yMin;
    double _yMax;
    QVector<QString> _item;
    double _xPos;
    QFont _currentFont;
    double _currentYMin;
    QVector<double> _tickVector;
    QVector<QString> _tickVectorLabels;
};

#endif
