#include <cctype>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "Lexer.hpp"
#include "Token.hpp"

using namespace std;

namespace {
class DoubleBufferedReader {
public:
    DoubleBufferedReader(const string& filePath, size_t bufferSize)
        : file_(filePath, ios::binary), bufferSize_(bufferSize) {
        if (!file_) {
            throw runtime_error("Failed to open input file.");
        }

        buffers_[0].reserve(bufferSize_);
        buffers_[1].reserve(bufferSize_);

        activeIndex_ = 0;
        activePos_ = 0;
        activeLen_ = 0;
        nextLen_ = 0;
        eof_ = false;
        stop_ = false;
        requestLoad_ = false;
        nextReady_ = false;

        loadBuffer(activeIndex_);
        loader_ = thread(&DoubleBufferedReader::loaderLoop, this);

        if (!eof_) {
            requestLoad_ = true;
            cv_.notify_all();
        }
    }

    ~DoubleBufferedReader() {
        {
            lock_guard<mutex> lock(mutex_);
            stop_ = true;
            requestLoad_ = true;
        }
        cv_.notify_all();
        if (loader_.joinable()) {
            loader_.join();
        }
    }

    bool getChar(char& ch) {
        if (activePos_ >= activeLen_) {
            if (!switchBuffer()) {
                return false;
            }
        }

        ch = buffers_[activeIndex_][activePos_++];
        return true;
    }

private:
    void loadBuffer(int index) {
        string temp;
        temp.resize(bufferSize_);
        file_.read(&temp[0], static_cast<streamsize>(bufferSize_));
        size_t bytes = static_cast<size_t>(file_.gcount());
        temp.resize(bytes);

        if (index == activeIndex_) {
            buffers_[index] = move(temp);
            activeLen_ = bytes;
            activePos_ = 0;
        } else {
            buffers_[index] = move(temp);
            nextLen_ = bytes;
        }

        if (bytes < bufferSize_) {
            eof_ = true;
        }
    }

    void loaderLoop() {
        unique_lock<mutex> lock(mutex_);
        while (true) {
            cv_.wait(lock, [&] { return requestLoad_ || stop_; });
            if (stop_) {
                break;
            }

            int target = 1 - activeIndex_;
            requestLoad_ = false;
            lock.unlock();
            loadBuffer(target);
            lock.lock();
            nextReady_ = true;
            cv_.notify_all();
        }
    }

    bool switchBuffer() {
        unique_lock<mutex> lock(mutex_);
        if (!nextReady_) {
            if (eof_) {
                return false;
            }
            requestLoad_ = true;
            cv_.notify_all();
            cv_.wait(lock, [&] { return nextReady_ || stop_; });
            if (stop_) {
                return false;
            }
        }

        activeIndex_ = 1 - activeIndex_;
        activePos_ = 0;
        activeLen_ = nextLen_;
        nextReady_ = false;

        if (activeLen_ == 0) {
            return false;
        }

        if (!eof_) {
            requestLoad_ = true;
            cv_.notify_all();
        }

        return true;
    }

    ifstream file_;
    size_t bufferSize_;
    string buffers_[2];
    int activeIndex_;
    size_t activePos_;
    size_t activeLen_;
    size_t nextLen_;
    bool eof_;
    bool stop_;
    bool requestLoad_;
    bool nextReady_;
    mutex mutex_;
    condition_variable cv_;
    thread loader_;
};
} // namespace

Lexer::Lexer(const string& filePath, size_t bufferSize)
    : filePath_(filePath), bufferSize_(bufferSize) {}

vector<Token> Lexer::tokenize() {
    DoubleBufferedReader reader(filePath_, bufferSize_);
    vector<Token> tokens;

    int line = 1;
    int column = 0;
    bool hasPeek = false;
    char peekChar = '\0';

    auto advancePosition = [&](char ch) {
        if (ch == '\n') {
            line += 1;
            column = 0;
        } else {
            column += 1;
        }
    };

    auto readChar = [&](char& ch) -> bool {
        if (hasPeek) {
            ch = peekChar;
            hasPeek = false;
        } else if (!reader.getChar(ch)) {
            return false;
        }
        advancePosition(ch);
        return true;
    };

    auto peek = [&](char& ch) -> bool {
        if (!hasPeek) {
            if (!reader.getChar(peekChar)) {
                return false;
            }
            hasPeek = true;
        }
        ch = peekChar;
        return true;
    };

    const regex keywordRegex("^(int|float|bool|void|if|else|while|for|return|input|output)$");
    const regex boolRegex("^(true|false)$");
    const regex identifierRegex("^[A-Za-z_][A-Za-z0-9_]*$");
    const regex intRegex("^[0-9]+$");
    const regex floatRegex("^[0-9]+\\.[0-9]+$");

    auto makeToken = [&](TokenType type, const string& lexeme, int startLine, int startCol) {
        tokens.push_back(Token{type, lexeme, startLine, startCol});
    };

    while (true) {
        char ch = '\0';
        if (!peek(ch)) {
            break;
        }

        if (isspace(static_cast<unsigned char>(ch))) {
            readChar(ch);
            continue;
        }

        int startLine = line;
        int startCol = column + 1;

        if (ch == '/') {
            readChar(ch);
            char next = '\0';
            if (peek(next) && next == '/') {
                readChar(next);
                while (readChar(ch)) {
                    if (ch == '\n') {
                        break;
                    }
                }
                continue;
            }
            makeToken(TokenType::OPERATOR, "/", startLine, startCol);
            continue;
        }

        const string separators = ";,(){}";
        if (separators.find(ch) != string::npos) {
            readChar(ch);
            makeToken(TokenType::SEPARATOR, string(1, ch), startLine, startCol);
            continue;
        }

        const string oneCharOps = "+-*=/<>!";
        const string twoCharStart = "=!<>&|";
        if (oneCharOps.find(ch) != string::npos || twoCharStart.find(ch) != string::npos) {
            readChar(ch);
            char next = '\0';
            string op(1, ch);
            if (peek(next)) {
                string twoChar = op + next;
                if (twoChar == "==" || twoChar == "!=" || twoChar == "<=" || twoChar == ">=" ||
                    twoChar == "&&" || twoChar == "||") {
                    readChar(next);
                    op = twoChar;
                }
            }
            makeToken(TokenType::OPERATOR, op, startLine, startCol);
            continue;
        }

        if (isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
            string lexeme;
            while (peek(ch) && (isalnum(static_cast<unsigned char>(ch)) || ch == '_')) {
                readChar(ch);
                lexeme.push_back(ch);
            }

            if (regex_match(lexeme, keywordRegex)) {
                makeToken(TokenType::KEYWORD, lexeme, startLine, startCol);
            } else if (regex_match(lexeme, boolRegex)) {
                makeToken(TokenType::BOOLEAN_LITERAL, lexeme, startLine, startCol);
            } else if (regex_match(lexeme, identifierRegex)) {
                makeToken(TokenType::IDENTIFIER, lexeme, startLine, startCol);
            } else {
                makeToken(TokenType::ERROR, lexeme, startLine, startCol);
            }
            continue;
        }

        if (isdigit(static_cast<unsigned char>(ch))) {
            string lexeme;
            bool sawDot = false;
            while (peek(ch) && (isdigit(static_cast<unsigned char>(ch)) || ch == '.')) {
                if (ch == '.') {
                    if (sawDot) {
                        break;
                    }
                    sawDot = true;
                }
                readChar(ch);
                lexeme.push_back(ch);
            }

            if (regex_match(lexeme, intRegex)) {
                makeToken(TokenType::INT_LITERAL, lexeme, startLine, startCol);
            } else if (regex_match(lexeme, floatRegex)) {
                makeToken(TokenType::FLOAT_LITERAL, lexeme, startLine, startCol);
            } else {
                makeToken(TokenType::ERROR, lexeme, startLine, startCol);
            }
            continue;
        }

        readChar(ch);
        makeToken(TokenType::ERROR, string(1, ch), startLine, startCol);
    }

    tokens.push_back(Token{TokenType::END_OF_FILE, "", line, column + 1});
    return tokens;
}
