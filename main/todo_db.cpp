#include "todo_db.h"
#include <iostream>

TodoDB::TodoDB(const std::string& path) : db_(nullptr) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "SQLite open failed\n";
        db_ = nullptr;
    }
}

TodoDB::~TodoDB() {
    if (db_) sqlite3_close(db_);
}

bool TodoDB::exec(const std::string& sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool TodoDB::init() {
    return exec(
        "CREATE TABLE IF NOT EXISTS todo ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "completed INTEGER NOT NULL);"
    );
}

bool TodoDB::create(const std::string& title) {
    return exec("INSERT INTO todo (title, completed) VALUES ('" + title + "', 0);");
}

std::vector<TodoItem> TodoDB::readAll() {
    std::vector<TodoItem> list;
    const char* sql = "SELECT id, title, completed FROM todo;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            TodoItem item;
            item.id = sqlite3_column_int(stmt, 0);
            item.title = (const char*)sqlite3_column_text(stmt, 1);
            item.completed = sqlite3_column_int(stmt, 2);
            list.push_back(item);
        }
    }

    sqlite3_finalize(stmt);
    return list;
}

bool TodoDB::update(int id, const std::string& title, bool completed) {
    return exec(
        "UPDATE todo SET title='" + title +
        "', completed=" + std::to_string(completed) +
        " WHERE id=" + std::to_string(id) + ";"
    );
}

bool TodoDB::remove(int id) {
    return exec("DELETE FROM todo WHERE id=" + std::to_string(id) + ";");
}
