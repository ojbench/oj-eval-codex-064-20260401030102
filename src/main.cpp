#include <bits/stdc++.h>
using namespace std;

static inline bool is_integer_token(const string &s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0]=='+' || s[0]=='-') i = 1;
    if (i>=s.size()) return false;
    for (; i<s.size(); ++i) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

static inline bool is_uinteger_token(const string &s) {
    if (s.empty()) return false;
    if (s[0]=='-') return false; // negative cannot be unsigned literal here
    size_t i = 0;
    if (s[0]=='+') i = 1;
    if (i>=s.size()) return false;
    for (; i<s.size(); ++i) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

static inline bool is_vector_literal(const string &s) {
    return s.size()>=2 && s.front()=='[' && s.back()==']';
}

static inline string default_format_token(const string &tok) {
    if (is_vector_literal(tok)) {
        // assume already comma-separated w/o spaces per problem spec
        return tok;
    }
    // If looks like integer, normalize canonical decimal
    if (is_integer_token(tok)) {
        try {
            long long v = stoll(tok);
            return to_string(v);
        } catch (...) {
            return tok;
        }
    }
    if (is_uinteger_token(tok)) {
        try {
            unsigned long long v = stoull(tok);
            return to_string(v);
        } catch (...) {
            return tok;
        }
    }
    // Treat otherwise as string
    return tok;
}

static inline string format_once(const string &fmt, deque<string> &args) {
    string out;
    out.reserve(fmt.size());
    for (size_t i=0; i<fmt.size(); ++i) {
        char c = fmt[i];
        if (c=='%') {
            if (i+1 >= fmt.size()) {
                // dangling %, ignore
                continue;
            }
            char sp = fmt[i+1];
            if (sp=='%') {
                out.push_back('%');
                ++i; // consume spec
                continue;
            }
            if (args.empty()) {
                // Not enough arguments; leave literal
                // Consume spec char to avoid infinite loop
                ++i;
                continue;
            }
            string tok = args.front();
            args.pop_front();
            switch (sp) {
                case 's':
                    out += tok;
                    break;
                case 'd': {
                    // parse as signed 64-bit if possible
                    // fall back to original token
                    try {
                        long long v = stoll(tok);
                        out += to_string(v);
                    } catch (...) {
                        out += tok;
                    }
                    } break;
                case 'u': {
                    try {
                        unsigned long long v = stoull(tok);
                        out += to_string(v);
                    } catch (...) {
                        try {
                            long long sv = stoll(tok);
                            unsigned long long v = static_cast<unsigned long long>(sv);
                            out += to_string(v);
                        } catch (...) {
                            out += tok;
                        }
                    }
                    } break;
                case '_': {
                    out += default_format_token(tok);
                    } break;
                default:
                    // invalid specifier; keep as literal: "%" + sp and push token back
                    out.push_back('%');
                    out.push_back(sp);
                    // put back token for potential following specifiers
                    args.push_front(tok);
                    break;
            }
            ++i; // skip specifier char
        } else {
            out.push_back(c);
        }
    }
    return out;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Read input as multiple cases: each case starts with a line = format string.
    // Then read as many tokens as there are specifiers (excluding escaped %%).
    // Output one formatted line per case.

    string fmt;
    bool firstOutput = true;
    while (true) {
        if (!std::getline(cin, fmt)) break;
        // skip empty lines
        if (fmt.size()==0) {
            // If there are blank lines between cases, just print blank line to match input
            if (!firstOutput) cout << '\n';
            firstOutput = false;
            continue;
        }
        // If line is a pure integer, treat it as number of cases
        auto trim = [](const string &s){
            size_t i=0; while (i<s.size() && isspace((unsigned char)s[i])) ++i;
            size_t j=s.size(); while (j>i && isspace((unsigned char)s[j-1])) --j;
            return s.substr(i, j-i);
        };
        string tline = trim(fmt);
        bool all_digits = !tline.empty() && all_of(tline.begin(), tline.end(), [](unsigned char ch){return isdigit(ch);} );
        if (all_digits) {
            int T = stoi(tline);
            for (int c=0; c<T; ++c) {
                string fl;
                // read next non-empty format line
                while (true) {
                    if (!getline(cin, fl)) { fl.clear(); break; }
                    if (!fl.empty()) break;
                }
                if (fl.empty()) break;
                // process this case as below
                // Count recognized specifiers (exclude "%%")
                size_t need2 = 0;
                for (size_t i=0; i<fl.size(); ++i) {
                    if (fl[i]=='%') {
                        if (i+1<fl.size()) {
                            char sp = fl[i+1];
                            if (sp=='%') { ++i; continue; }
                            if (sp=='s' || sp=='d' || sp=='u' || sp=='_') ++need2;
                            ++i;
                        }
                    }
                }
                deque<string> args2;
                string tok2;
                auto read_token2 = [&]() -> bool {
                    tok2.clear();
                    int ch;
                    while ((ch = cin.peek()) != EOF && isspace(ch)) cin.get();
                    if (ch == EOF) return false;
                    if (ch == '"') {
                        cin.get();
                        while ((ch = cin.get()) != EOF) {
                            if (ch == '"') break;
                            tok2.push_back(static_cast<char>(ch));
                        }
                        return true;
                    }
                    while ((ch = cin.peek()) != EOF && !isspace(ch)) tok2.push_back(static_cast<char>(cin.get()));
                    return !tok2.empty();
                };
                while (args2.size() < need2 && read_token2()) args2.push_back(tok2);
                string dummy2; getline(cin, dummy2);
                string res2 = format_once(fl, args2);
                if (!firstOutput) cout << '\n';
                cout << res2;
                firstOutput = false;
            }
            continue; // go read next block
        }
        // Count specifiers (exclude "%%") for single case line
        size_t need = 0;
        for (size_t i=0; i<fmt.size(); ++i) {
            if (fmt[i]=='%') {
                if (i+1<fmt.size() && fmt[i+1]=='%') { ++i; continue; }
                // count only s, d, u, _
                if (i+1<fmt.size()) {
                    char sp = fmt[i+1];
                    if (sp=='s' || sp=='d' || sp=='u' || sp=='_') ++need;
                }
                ++i; // skip next char if any
            }
        }
        // Read tokens until we have 'need' tokens.
        // Support quoted strings with spaces: tokens may be
        // - bare: no spaces, separated by whitespace
        // - quoted with "..." allowing spaces; quotes are stripped
        deque<string> args;
        string tok;
        auto read_token = [&]() -> bool {
            tok.clear();
            int ch;
            // skip leading spaces and newlines
            while ((ch = cin.peek()) != EOF && isspace(ch)) {
                cin.get();
            }
            if (ch == EOF) return false;
            if (ch == '"') {
                cin.get(); // consume opening quote
                // read until closing quote
                while ((ch = cin.get()) != EOF) {
                    if (ch == '"') break;
                    tok.push_back(static_cast<char>(ch));
                }
                return true;
            }
            // read until next whitespace
            while ((ch = cin.peek()) != EOF && !isspace(ch)) {
                tok.push_back(static_cast<char>(cin.get()));
            }
            return !tok.empty();
        };
        while (args.size() < need && read_token()) {
            args.push_back(tok);
        }
        // consume rest of the line after tokens to newline
        string dummy;
        getline(cin, dummy);

        string res = format_once(fmt, args);
        if (!firstOutput) cout << '\n';
        cout << res;
        firstOutput = false;
    }

    return 0;
}
