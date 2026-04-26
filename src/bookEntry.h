#include <string>

// Book Entry
struct BookEntry {
    std::string id;
    long long timestamp;

    BookEntry() = default;
    ~BookEntry() = default;

    BookEntry(const std::string& id, long long ts) : id(id), timestamp(ts) {}
};