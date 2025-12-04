#ifndef TODO_DB_H
#define TODO_DB_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct TodoItem {
    int id;
    std::string title;
    int completed;
};

class TodoDB {
public:
    TodoDB(const std::string& path);
    ~TodoDB();

    bool init();
    bool create(const std::string& title);
    std::vector<TodoItem> readAll();
    bool update(int id, const std::string& title, bool completed);
    bool remove(int id);

private:
    sqlite3* db_;
    bool exec(const std::string& sql);
};

#endif // TODO_DB_H
