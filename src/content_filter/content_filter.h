#ifndef CONTENT_FILTER_H
#define CONTENT_FILTER_H

#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>

// ---------------- Filter Interfaces ----------------

class BlacklistFilter {
public:
    BlacklistFilter() = default;
    BlacklistFilter(const std::unordered_set<std::string>& blacklistItems);
    bool applyFilter(const std::string& request);
    void addToBlacklist(const std::string& host);
    void removeFromBlacklist(const std::string& host);
    void clear(); // Thêm hàm clear
private:
    std::unordered_set<std::string> blacklist;
};

class WhitelistFilter {
public:
    WhitelistFilter() = default;
    WhitelistFilter(const std::unordered_set<std::string>& whitelistItems);
    bool applyFilter(const std::string& request);
    void addToWhitelist(const std::string& host);
    void removeFromWhitelist(const std::string& host);
    void clear(); // Thêm hàm clear
private:
    std::unordered_set<std::string> whitelist;
};


#endif // CONTENT_FILTER_H
