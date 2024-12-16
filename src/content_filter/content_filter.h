#ifndef CONTENT_FILTER_H
#define CONTENT_FILTER_H

#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <fstream>

// ---------------- Filter Interfaces ----------------

class BlacklistFilter {
public:
    BlacklistFilter() = default;
    BlacklistFilter(const std::unordered_set<std::string>& blacklistItems);
    bool applyFilter(const std::string& request);
    void addToBlacklist(const std::string& host);
    void removeFromBlacklist(const std::string& host);
    void saveData(std::ofstream& file) const;
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
    void saveData(std::ofstream& file) const;
    void clear(); // Thêm hàm clear
private:
    std::unordered_set<std::string> whitelist;
};

class KeywordFilter {
public:
    KeywordFilter() = default;
    KeywordFilter(const std::vector<std::string>& keywordList);
    bool applyFilter(const std::string& request);
    void addKeyword(const std::string& keyword);
    void removeKeyword(const std::string& keyword);
    void saveData(std::ofstream& file) const;
    void clear(); // Thêm hàm clear
private:
    std::vector<std::string> keywords;
};


#endif // CONTENT_FILTER_H
