#ifndef CONFIG_H_
#define CONFIG_H_

#define APP_NAME "MarketProfile"

#define OANDA_URL "https://api-fxtrade.oanda.com/v1/candles?instrument=%1&start=%2&granularity=%3&candleFormat=midpoint"
#define CANDLE_GRANULARITY "M30"

#define CANDLES_NAME "candles"
#define TIME_NAME "time"
#define OPEN_NAME "openMid"
#define HIGH_NAME "highMid"
#define LOW_NAME "lowMid"
#define CLOSE_NAME "closeMid"
#define VOLUME_NAME "volume"
#define COMPLETE_NAME "complete"

#define OBSOLETE_DATA_THRESHOLD_DAYS 7

#endif
