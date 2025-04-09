#pragma once

#include <istream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>
#include <stdexcept>

#define VERBOSE_PARSE_LOGGING
#ifdef VERBOSE_PARSE_LOGGING
#include "logging.h"
#endif

template <typename T>
class Parser
{
protected:
    std::istream &input;
    std::string begin_token;
    std::string version_header;

    using Handler = std::function<bool(T &, const std::string &, std::istream &)>;
    std::unordered_map<std::string, Handler> keyword_handlers;

    size_t parse_count = 0; // Count of current entry being parsed

    virtual void registerKeywords() = 0;
    virtual T makeDefault() = 0;
    virtual bool validate(const T &) = 0;

public:
    Parser(std::istream &in, std::string begin, std::string version)
        : input(in), begin_token(std::move(begin)), version_header(std::move(version)) {}

    std::vector<T> parseAll()
    {
        std::vector<T> entries;
        std::string line;
        parse_count = 0;

        std::getline(input, line);
        if (line != version_header)
            throw std::runtime_error("Invalid file header: " + line);

        while (std::getline(input, line))
        {
            if (line == begin_token)
            {
                ++parse_count;
                T entry = makeDefault();
                registerKeywords();

                if (!parseOne(entry))
                {
#ifdef VERBOSE_PARSE_LOGGING
                    Log::write("Parse error in %s #%zu", begin_token.c_str(), parse_count);
#endif
                    continue;
                }

                if (!validate(entry))
                {
#ifdef VERBOSE_PARSE_LOGGING
                    Log::write("Validation failed in %s #%zu", begin_token.c_str(), parse_count);
#endif
                    continue;
                }

                entries.push_back(std::move(entry));
            }
        }

        return entries;
    }

protected:
    bool parseOne(T &entry)
    {
        std::unordered_set<std::string> seen;
        std::string line;

        while (std::getline(input, line))
        {
            if (line == "END")
                break;

            std::istringstream iss(line);
            std::string keyword;
            iss >> keyword;

            std::string rest;
            std::getline(iss, rest); // everything after the keyword
            if (!rest.empty() && rest.front() == ' ')
                rest.erase(0, 1); // trim leading space

            if (keyword.empty())
                continue;

            if (seen.count(keyword))
            {
#ifdef VERBOSE_PARSE_LOGGING
                Log::write("Duplicate field \"%s\" in %s #%zu", keyword.c_str(), begin_token.c_str(), parse_count);
#endif
                return false;
            }

            auto handler = keyword_handlers.find(keyword);
            if (handler == keyword_handlers.end())
            {
#ifdef VERBOSE_PARSE_LOGGING
                Log::write("Unknown keyword \"%s\" in %s #%zu", keyword.c_str(), begin_token.c_str(), parse_count);
#endif
                return false;
            }

            seen.insert(keyword);

            if (!handler->second(entry, rest, input))
            {
#ifdef VERBOSE_PARSE_LOGGING
                Log::write("Handler failed for \"%s\" in %s #%zu", keyword.c_str(), begin_token.c_str(), parse_count);
#endif
                return false;
            }
        }

        return true;
    }
};
