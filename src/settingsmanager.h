#ifndef SETTINGS_MANAGER_H_
#define SETTINGS_MANAGER_H_

class SettingsManager {
public:
    static void readCurrentSymbolIndex(int &index);
    static void writeCurrentSymbolIndex(int index);
};

#endif
