idf_component_register(
    SRCS "main.cpp" "todo_db.cpp" "SdCardUtils.cpp"
    INCLUDE_DIRS "."
    REQUIRES fatfs sdmmc driver esp32-idf-sqlite3)

