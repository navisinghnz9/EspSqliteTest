#include "SdCardUtils.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "driver/gpio.h"
#include <dirent.h>
#include <sys/stat.h>

static const char* TAG = "SdCardUtils";

esp_err_t SdCardUtils::mountSPI(
    const char* mountPoint,
    int mosi, int miso, int sclk, int cs
)
{
    ESP_LOGI(TAG, "Mounting SPI SD card at %s", mountPoint);

    //----------------------------------------
    // Initialize SPI host
    //----------------------------------------
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = mosi;
    bus_cfg.miso_io_num = miso;
    bus_cfg.sclk_io_num = sclk;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;

    esp_err_t ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //----------------------------------------
    // Configure SD SPI device
    //----------------------------------------
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)cs;
    slot_config.host_id = (spi_host_device_t)host.slot;

    //----------------------------------------
    // Mount FATFS
    //----------------------------------------
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;

    sdmmc_card_t* card;
    ret = esp_vfs_fat_sdspi_mount(mountPoint, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SD card mounted at %s", mountPoint);
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

void SdCardUtils::unmount(const char* mountPoint)
{
    ESP_LOGI(TAG, "Unmounting SD card at %s", mountPoint);
    esp_vfs_fat_sdcard_unmount(mountPoint, nullptr);
}

std::vector<std::string> SdCardUtils::listFiles(const std::string& path)
{
    std::vector<std::string> files;
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", path.c_str());
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fullPath = path + "/" + entry->d_name;

        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
            continue;

        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                files.push_back(fullPath);
                ESP_LOGI(TAG, "FILE : %s", fullPath.c_str());
            } else if (S_ISDIR(st.st_mode)) {
                ESP_LOGI(TAG, "DIR  : %s", fullPath.c_str());
            }
        }
    }

    closedir(dir);
    return files;
}
