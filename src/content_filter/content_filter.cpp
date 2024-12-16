#include "content_filter.h"

// ---------------- BlacklistFilter Implementation ----------------
BlacklistFilter::BlacklistFilter(const std::unordered_set<std::string>& blacklistItems)
    : blacklist(blacklistItems) {}

bool BlacklistFilter::applyFilter(const std::string& request) {
    for (const auto& host : blacklist) {
        if (request.find(host) != std::string::npos) {
            return true; 
        }
    }
    return false; 
}

void BlacklistFilter::addToBlacklist(const std::string& host) {
    blacklist.insert(host);
}

void BlacklistFilter::removeFromBlacklist(const std::string& host) {
    blacklist.erase(host);
}

void BlacklistFilter::saveData(std::ofstream& file) const {
    for (const auto& host : blacklist) {
        file << host << '\n';
    }
}

void BlacklistFilter::clear() {
    blacklist.clear();
}

// ---------------- WhitelistFilter Implementation ----------------
WhitelistFilter::WhitelistFilter(const std::unordered_set<std::string>& whitelistItems)
    : whitelist(whitelistItems) {}

bool WhitelistFilter::applyFilter(const std::string& request) {
   for (const auto& host : whitelist) {
        if (request.find(host) != std::string::npos) {
            return true; 
        }
    }
    return false; 
}

void WhitelistFilter::addToWhitelist(const std::string& host) {
    whitelist.insert(host);
}

void WhitelistFilter::removeFromWhitelist(const std::string& host) {
    whitelist.erase(host);
}

void WhitelistFilter::saveData(std::ofstream& file) const {
    for (const auto& host : whitelist) {
        file << host << '\n';
    }
}

void WhitelistFilter::clear() {
    whitelist.clear();
}

// ---------------- KeywordFilter Implementation ----------------
KeywordFilter::KeywordFilter(const std::vector<std::string>& keywordList)
    : keywords(keywordList) {}

bool KeywordFilter::applyFilter(const std::string& request) {
    for (const auto& keyword : keywords) {
        if (request.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void KeywordFilter::addKeyword(const std::string& keyword) {
    keywords.push_back(keyword);
}

void KeywordFilter::removeKeyword(const std::string& keyword) {
    keywords.erase(std::remove(keywords.begin(), keywords.end(), keyword), keywords.end());
}

void KeywordFilter::saveData(std::ofstream& file) const {
    for (const auto& keyword : keywords) {
        file << keyword << '\n';
    }
}

void KeywordFilter::clear() {
    keywords.clear();
}
