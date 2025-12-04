#ifndef SD_CARD_UTILS_H
#define SD_CARD_UTILS_H

#include "esp_err.h"
#include <vector>
#include <string>

class SdCardUtils {
public:
    // Mount SPI SD card at given path, returns ESP_OK or error
    static esp_err_t mountSPI(
        const char* mountPoint = "/sdcard",
        int mosi = 23,
        int miso = 19,
        int sclk = 18,
        int cs = 5
    );

    // Unmount SD card
    static void unmount(const char* mountPoint = "/sdcard");

    // List all files in directory (non-recursive)
    static std::vector<std::string> listFiles(const std::string& path = "/sdcard");
};

#endif // SD_CARD_UTILS_H
