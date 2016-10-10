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
    bool setBackgroudColor(int red, int green, int blue);
    bool setLiteralColor(int red, int green, int blue);
    bool setXLabel(const QString &label);
    bool setYLabel(const QString &label);
    bool setLabelColor(int red, int green, int blue);
    bool loadTimeSeries(const QMap<QDateTime, MarketProfile::Data> &data, bool update = false);
    bool updateTimeSeries(const QMap<QDateTime, MarketProfile::Data> &data) {
        return loadTimeSeries(data, true);
    }
    // all methods for indicator manipulation use indicator name for identification
    bool addIndicator(const QString &indicatorName, const QMap<QDateTime, double> &position);
    bool removeIndicator(const QString &indicatorName);
    bool updateIndicator(const QString &indicatorName, const QMap<QDateTime, double> &position);
    bool showIndicator(const QString &indicatorName) {
        return updateIndicator(indicatorName, true);
    }
    bool hideIndicator(const QString &indicatorName) {
        return updateIndicator(indicatorName, false);
    }
    void setMapResolution(int mapResolution) {
        _mapResolution = mapResolution;
    }
private slots:
    void updateItems();
    void onMouseWheel(QWheelEvent *event);
private:
    enum {MAP_RESOLUTION = 5};
    void process(const QVector<double> &upper, const QVector<double> &lower,
                    const QDate &currentDate, bool dump = false) {
        _tickVectorLabels.push_back(currentDate.toString("MMM d yyyy"));
        _tickVectorDates.push_back(currentDate);
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
    bool isValidColor(int val) {
        return ((0 <= val) && (255 >= val));
    }
    void displayItem();
    void clear();
    bool setupItemText(QCPItemText *itemText, const QString &text, double x, double y);
    bool updateIndicator(const QString &indicatorName, bool show = true);
    bool findTickPosition(int &pos, const QDate &currentDate);
    int computeFontPointSize(double letterHeight, double lower, double upper) const {
        const int widgetHeight = height();
        const double out = letterHeight*widgetHeight/(upper-lower);
        return qFloor(out);
    }
    void updateItemsInternal(double lower, double upper);

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
    double _currentYMax;
    QVector<double> _tickVector;
    QVector<QString> _tickVectorLabels;
    QVector<QDate> _tickVectorDates;
    QMap<QString, QCPItemText*> _indicators;
    QColor _literalColor;
    QColor _labelColor;
    struct Item {
        double letterHeight;
        double currentYMin;
        QVector<QCPItemText*> bars;
    };
    QMap<double,Item> _items;
    int _mapResolution;
    int _oldHeight;
};

#endif
