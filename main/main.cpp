#include "SdCardUtils.h"
#include "todo_db.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
//#include "esp_heap_caps.h"


static const char* TAG = "[ESP-SQLITE-TEST]";

extern "C" void app_main()
{

  ESP_LOGI(TAG, "=== STARTING SD + SQLite SAFE TEST ===");

  //--------------------------------------------------------------------------
  // STEP 1: Mount SPI SD card
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 1] Mounting SPI SD card...");
  esp_err_t ret = SdCardUtils::mountSPI("/sdcard", 23, 19, 18, 5);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(ret));
    return;
  }
  ESP_LOGI(TAG, "[STEP 1] SD card mounted successfully.");


  //--------------------------------------------------------------------------
  // STEP 2: Wait for filesystem stability
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 2] Waiting 500ms for FS stabilization...");
  vTaskDelay(pdMS_TO_TICKS(500));


  //--------------------------------------------------------------------------
  // STEP 3: Test FS is writable
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 3] Testing filesystem write...");
  FILE* f = fopen("/sdcard/fs_test.txt", "w");
  if (!f) {
    ESP_LOGE(TAG, "Filesystem is not writable!");
    SdCardUtils::unmount("/sdcard");
    return;
  }
  fprintf(f, "FS test ok\n");
  fclose(f);

  ESP_LOGI(TAG, "[STEP 3] Filesystem is writable.");


  //--------------------------------------------------------------------------
  // STEP 3a: List all files on SD card
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 3a] Listing files in /sdcard...");
  auto files = SdCardUtils::listFiles("/sdcard");
  if (files.empty()) {
    ESP_LOGI(TAG, "No files found on SD card.");
  } else {
    for (auto& file : files) {
      ESP_LOGI(TAG, "Found: %s", file.c_str());
    }
  }


  //--------------------------------------------------------------------------
  // STEP 4: Initialize SQLite database
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 4] SQLite Init");
  sqlite3_initialize();

  ESP_LOGI(TAG, "[STEP 4] SQLite open/create database at /sdcard/todo.db...");
  TodoDB db("/sdcard/todo.db");
  if (!db.init()) {
    ESP_LOGE(TAG, "Failed to initialize database!");
    SdCardUtils::unmount("/sdcard");
    return;
  }
  ESP_LOGI(TAG, "[STEP 4] Database initialized successfully.");


  //--------------------------------------------------------------------------
  // STEP 5: Insert test TODOs
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 5] Inserting test TODO items...");
  db.create("Buy milk");
  db.create("Buy car");
  db.create("Go to gym");
  db.create("Write ESP32 SQLite app");
  ESP_LOGI(TAG, "[STEP 5] Test TODOs inserted.");


  //--------------------------------------------------------------------------
  // STEP 6: Read all TODOs
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 6] Reading all TODO items...");
  auto todos = db.readAll();
  if (todos.empty()) {
    ESP_LOGI(TAG, "No TODO items found!");
  } else {
    for (auto& t : todos) {
      ESP_LOGI(TAG, "TODO %d | %s | %d", t.id, t.title.c_str(), t.completed);
    }
  }


  //--------------------------------------------------------------------------
  // STEP 7: Update first TODO
  //--------------------------------------------------------------------------
  if (!todos.empty()) {
    ESP_LOGI(TAG, "[STEP 7] Updating first TODO...");
    db.update(todos[0].id, "Buy soya milk", true);
    ESP_LOGI(TAG, "[STEP 7] First TODO updated.");
  }


  //--------------------------------------------------------------------------
  // STEP 8: Delete second TODO (if exists)
  //--------------------------------------------------------------------------
  if (todos.size() > 1) {
    ESP_LOGI(TAG, "[STEP 8] Deleting second TODO...");
    db.remove(todos[1].id);
    ESP_LOGI(TAG, "[STEP 8] Second TODO deleted.");
  }


  //--------------------------------------------------------------------------
  // STEP 9: Read all TODOs again
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 9] Reading all TODOs after update/delete...");
  todos = db.readAll();
  for (auto& t : todos) {
    ESP_LOGI(TAG, "UPDATED %d | %s | %d", t.id, t.title.c_str(), t.completed);
  }


  //--------------------------------------------------------------------------
  // STEP 10: Unmount SD card
  //--------------------------------------------------------------------------
  ESP_LOGI(TAG, "[STEP 10] Unmounting SD card...");
  SdCardUtils::unmount("/sdcard");
  ESP_LOGI(TAG, "[STEP 10] SD card unmounted.");

  ESP_LOGI(TAG, "=== SD + SQLite SAFE TEST COMPLETED ===");
}
