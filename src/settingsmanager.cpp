#include <QSettings>
#include "settingsmanager.h"
#include "config.h"

#define CURRENT_SYMBOL_INDEX "CURRENT_SYMBOL_INDEX"

void SettingsManager::readCurrentSymbolIndex(int &index)
{
    QSettings settings(ORG_NAME, APP_NAME);
    index = settings.value(CURRENT_SYMBOL_INDEX, 0).toInt();
}

void SettingsManager::writeCurrentSymbolIndex(int index)
{
    QSettings settings(ORG_NAME, APP_NAME);
    settings.setValue(CURRENT_SYMBOL_INDEX, index);
}
