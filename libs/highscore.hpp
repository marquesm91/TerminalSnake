#ifndef HIGHSCORE_H_
#define HIGHSCORE_H_

#include <fstream>
#include <string>
#include <cstdlib>

class Highscore {

private:
    int score;
    std::string filePath;

    std::string getConfigPath() {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/.tsnake_highscore";
        }
        return ".tsnake_highscore";
    }

public:
    Highscore() : score(0) {
        filePath = getConfigPath();
        load();
    }

    void load() {
        std::ifstream file(filePath);
        if (file.is_open()) {
            file >> score;
            file.close();
        }
    }

    void save(int newScore) {
        if (newScore > score) {
            score = newScore;
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << score;
                file.close();
            }
        }
    }

    int get() const { return score; }

    void set(int newScore) {
        if (newScore > score) {
            score = newScore;
            save(score);
        }
    }
};

#endif
