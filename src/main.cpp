#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unistd.h>

#include "../include/nlohmann/json.hpp"


using json = nlohmann::json;


std::string
url_encode(const std::string& s) {
    static const char* hex = "0123456789ABCDEF";
    std::string out;
    out.reserve(s.size());

    for (unsigned char c : s) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += c;
        } else {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 0xf];
        }
    }
    return out;
}


struct
Bang {
    std::string tag;
    std::string domain;
    std::string url_template;
};


std::unordered_map<std::string, Bang>
load_bangs(const std::string& path) {
    std::unordered_map<std::string, Bang> bangs;
    std::ifstream f(path);
    if (!f) {
        std::cerr << "error: " << path << " not found.\n";
        return bangs;
    }

    json data;
    try {
        f >> data;
    } catch (const json::parse_error& e) {
        std::cerr << "error: could not decode json: " << e.what() << "\n";
        return bangs;
    }

    for (auto& b : data) {
        std::string t = b.value("t", "");
        if (t.empty()) continue;

        for (char& c : t) c = (char)std::tolower((unsigned char)c);

        bangs[t] = Bang{
            t,
            b.value("d", ""),
            b.value("u", ""),
        };
    }
    return bangs;
}


struct
ParsedQuery {
    std::string bang;
    std::string query;
};


ParsedQuery
parse_query(const std::string& input) {
    if (input.size() > 1 && input[0] == '!') {
        auto space = input.find(' ');
        std::string tag = input.substr(1, space == std::string::npos ? std::string::npos : space - 1);
        std::string rest = space == std::string::npos ? "" : input.substr(space + 1);

        for (char& c : tag) c = (char)std::tolower((unsigned char)c);

        return {tag, rest};
    }
    return {"", input};
}


std::string
get_redirect_url(
    const std::unordered_map<std::string, Bang>& bangs,
    const std::string& input
) {
    auto parsed = parse_query(input);
    std::string clean_query = parsed.query;

    const Bang* selected = nullptr;

    if (!parsed.bang.empty()) {
        auto it = bangs.find(parsed.bang);
        if (it != bangs.end())
            selected = &it->second;
    }

    if (!selected) {
        auto it = bangs.find("ddg");
        if (it != bangs.end()) {
            selected = &it->second;
            clean_query = input;
        }
    }

    if (!selected) return "";

    if (clean_query.empty())
        return "https://" + selected->domain;

    std::string url = selected->url_template;
    std::string placeholder = "{{{s}}}";
    auto pos = url.find(placeholder);
    if (pos != std::string::npos)
        url.replace(pos, placeholder.size(), url_encode(clean_query));

    return url;
}


int
main(int argc, char* argv[]) {
    bool do_open = false;
    std::string query;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--open" || arg == "-o") {
            do_open = true;
        } else {
            if (!query.empty()) query += ' ';
            query += arg;
        }
    }

    if (query.empty()) {
        std::cerr << "usage: bang [--open|-o] <query>\n";
        return 1;
    }

    std::string exe_dir;
    {
        char buf[4096];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len != -1) {
            buf[len] = '\0';
            exe_dir = buf;
        } else {
            char* path = realpath(argv[0], nullptr);
            if (path) {
                exe_dir = path;
                free(path);
            }
        }
        auto slash = exe_dir.rfind('/');
        if (slash != std::string::npos) exe_dir = exe_dir.substr(0, slash);
    }

    auto bangs = load_bangs(exe_dir + "/source.json");
    if (bangs.empty()) return 1;

    std::string url = get_redirect_url(bangs, query);
    if (url.empty()) {
        std::cerr << "could not determine a redirect url.\n";
        return 1;
    }

    if (do_open) {
        int ret = std::system(("xdg-open " + url).c_str());
        if (ret != 0) {
            std::cerr << "error opening url with xdg-open\n";
            return 1;
        }
    } else {
        std::cout << url << "\n";
    }

    return 0;
}
