// typing_trainer.cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cctype>
#include <filesystem>
#include <algorithm>

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

const string RESET = "\033[0m";
const string GREEN = "\033[92m";
const string RED = "\033[91m";
const string YELLOW = "\033[93m";
const string BLUE = "\033[94m";
const string BOLD = "\033[1m";

string colorize(const string& text, const string& color) {
    return color + text + RESET;
}

string getHomeDir() {
    const char* home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE");
    return string(home);
}

map<string, map<string, vector<string>>> WORDS = {
    {"ru", {
        {"easy", {"кот","нос","рот","дом","лес","море","вода","рука","нога","мир"}},
        {"medium", {"компьютер","программа","алгоритм","интернет","сервер","сайт","код","данные"}},
        {"hard", {"программирование","искусственный интеллект","коммуникация","информация","технология"}}
    }},
    {"en", {
        {"easy", {"cat","dog","sun","run","jump","tree","book","fish","bird","hand"}},
        {"medium", {"computer","program","algorithm","internet","server","website","code","data"}},
        {"hard", {"programming","artificial intelligence","communication","information","technology"}}
    }}
};

struct Stats {
    double best_wpm = 0;
    int games = 0;
    double total_accuracy = 0;
};

class Trainer {
public:
    string lang, level;
    string text;
    string userInput;
    high_resolution_clock::time_point start, end;
    Stats stats;
    string statsFile;

    Trainer(string l, string lvl) : lang(l), level(lvl) {
        statsFile = getHomeDir() + "/.typing_trainer_" + lang + ".json";
        loadStats();
        generateText();
    }

    void loadStats() {
        ifstream f(statsFile);
        if (!f) return;
        string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
        // Простой парсинг JSON (без библиотеки)
        auto extract = [&](const string& key) -> double {
            size_t pos = content.find("\"" + key + "\"");
            if (pos == string::npos) return 0;
            pos = content.find(":", pos) + 1;
            size_t end = content.find(",", pos);
            if (end == string::npos) end = content.find("}", pos);
            string val = content.substr(pos, end-pos);
            val.erase(remove_if(val.begin(), val.end(), ::isspace), val.end());
            if (val.front() == '"') val.erase(0,1);
            if (val.back() == '"') val.pop_back();
            return stod(val);
        };
        stats.best_wpm = extract("best_wpm");
        stats.games = (int)extract("games");
        stats.total_accuracy = extract("total_accuracy");
    }

    void saveStats() {
        ofstream f(statsFile);
        if (f) {
            f << "{\"best_wpm\":" << stats.best_wpm
              << ",\"games\":" << stats.games
              << ",\"total_accuracy\":" << stats.total_accuracy << "}";
        }
    }

    void generateText() {
        auto& words = WORDS[lang][level];
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, words.size()-1);
        int count = (level == "hard") ? rand() % 8 + 8 : rand() % 6 + 5;
        stringstream ss;
        for (int i=0; i<count; ++i) {
            if (i>0) {
                if (level == "hard" && (rand() % 10 < 3)) ss << ", ";
                else ss << " ";
            }
            ss << words[dis(gen)];
            if (level == "hard" && (i==count-1 || (rand() % 10 < 2))) {
                ss << ". ";
            }
        }
        text = ss.str();
        // Удаляем лишние пробелы в конце
        while (!text.empty() && text.back() == ' ') text.pop_back();
    }

    void displayText() {
        cout << colorize("\nНапечатайте следующий текст:", BOLD) << endl;
        cout << colorize(text, BLUE) << endl;
        cout << "\nНажмите Enter после завершения ввода. Время начнётся с первой буквы." << endl;
        cout << "Для выхода нажмите Ctrl+C.\n" << endl;
    }

    void getUserInput() {
        cin.ignore(); // очистка буфера
        start = high_resolution_clock::now();
        getline(cin, userInput);
        end = high_resolution_clock::now();
    }

    void calculateAndDisplay() {
        string original = text;
        string typed = userInput;
        int total = original.size();
        int typedLen = typed.size();
        int correct = 0;
        int errors = 0;
        for (int i=0; i<total; ++i) {
            if (i < typedLen && original[i] == typed[i]) {
                correct++;
            } else {
                errors++;
            }
        }
        if (typedLen < total) errors += total - typedLen;
        auto duration = duration_cast<milliseconds>(end - start).count();
        double elapsed = duration / 1000.0;
        double minutes = elapsed / 60.0;
        double wpm = (correct / 5.0) / minutes;
        double accuracy = (correct / (double)total) * 100.0;

        // Цветной вывод
        cout << colorize("\nРезультат:", BOLD) << endl;
        for (int i=0; i<total; ++i) {
            if (i < typedLen && original[i] == typed[i])
                cout << colorize(string(1, original[i]), GREEN);
            else
                cout << colorize(string(1, original[i]), RED);
        }
        cout << endl;

        cout << colorize("Скорость: " + to_string(wpm) + " WPM", YELLOW) << endl;
        cout << "Точность: " << accuracy << "%" << endl;
        cout << "Правильных символов: " << correct << "/" << total << endl;
        cout << "Ошибок: " << errors << endl;
        cout << "Время: " << elapsed << " сек" << endl;

        stats.games++;
        if (wpm > stats.best_wpm) {
            stats.best_wpm = wpm;
            cout << colorize("🏆 Новый рекорд!", GREEN) << endl;
        }
        stats.total_accuracy = (stats.total_accuracy * (stats.games-1) + accuracy) / stats.games;
        saveStats();
        cout << colorize("Лучший результат: " + to_string(stats.best_wpm) + " WPM", BLUE) << endl;
    }

    void run() {
        cout << colorize("⌨️  Тренажёр слепой печати", BOLD) << endl;
        cout << "Язык: " << lang << ", Уровень: " << level << endl;
        displayText();
        getUserInput();
        calculateAndDisplay();
    }
};

int main(int argc, char* argv[]) {
    string lang = "ru", level = "medium";
    bool showStats = false, resetStats = false;
    for (int i=1; i<argc; ++i) {
        string arg = argv[i];
        if (arg == "ru" || arg == "en") lang = arg;
        else if (arg == "easy" || arg == "medium" || arg == "hard") level = arg;
        else if (arg == "-s" || arg == "--stats") showStats = true;
        else if (arg == "-r" || arg == "--reset") resetStats = true;
        else if (arg == "-h" || arg == "--help") {
            cout << "Usage: typing_trainer [ru|en] [easy|medium|hard] [-s] [-r]" << endl;
            return 0;
        }
    }
    if (resetStats) {
        for (string l : {"ru", "en"}) {
            string f = getHomeDir() + "/.typing_trainer_" + l + ".json";
            if (fs::exists(f)) fs::remove(f);
        }
        cout << "Статистика сброшена." << endl;
        return 0;
    }
    if (showStats) {
        for (string l : {"ru", "en"}) {
            string f = getHomeDir() + "/.typing_trainer_" + l + ".json";
            ifstream file(f);
            if (file) {
                string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                auto extract = [&](const string& key) -> double {
                    size_t pos = content.find("\"" + key + "\"");
                    if (pos == string::npos) return 0;
                    pos = content.find(":", pos) + 1;
                    size_t end = content.find(",", pos);
                    if (end == string::npos) end = content.find("}", pos);
                    string val = content.substr(pos, end-pos);
                    val.erase(remove_if(val.begin(), val.end(), ::isspace), val.end());
                    if (val.front() == '"') val.erase(0,1);
                    if (val.back() == '"') val.pop_back();
                    return stod(val);
                };
                double best = extract("best_wpm");
                int games = (int)extract("games");
                double acc = extract("total_accuracy");
                cout << colorize("📊 Статистика для " + l + ":", BOLD) << endl;
                cout << "  Лучший WPM: " << best << endl;
                cout << "  Сыграно игр: " << games << endl;
                cout << "  Средняя точность: " << acc << "%" << endl;
            } else {
                cout << "Статистика для " + l + " пуста." << endl;
            }
        }
        return 0;
    }
    Trainer trainer(lang, level);
    trainer.run();
    return 0;
}
