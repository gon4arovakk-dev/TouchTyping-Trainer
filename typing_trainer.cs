// typing_trainer.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Diagnostics;

class TypingTrainer
{
    static string Colorize(string text, string color)
    {
        string col = color switch
        {
            "green" => "\x1b[92m",
            "red" => "\x1b[91m",
            "yellow" => "\x1b[93m",
            "blue" => "\x1b[94m",
            "bold" => "\x1b[1m",
            _ => "\x1b[0m"
        };
        return col + text + "\x1b[0m";
    }

    static Dictionary<string, Dictionary<string, List<string>>> WORDS = new Dictionary<string, Dictionary<string, List<string>>>()
    {
        {"ru", new Dictionary<string, List<string>>()
        {
            {"easy", new List<string>{"кот","нос","рот","дом","лес","море","вода","рука","нога","мир"}},
            {"medium", new List<string>{"компьютер","программа","алгоритм","интернет","сервер","сайт","код","данные"}},
            {"hard", new List<string>{"программирование","искусственный интеллект","коммуникация","информация","технология"}}
        }},
        {"en", new Dictionary<string, List<string>>()
        {
            {"easy", new List<string>{"cat","dog","sun","run","jump","tree","book","fish","bird","hand"}},
            {"medium", new List<string>{"computer","program","algorithm","internet","server","website","code","data"}},
            {"hard", new List<string>{"programming","artificial intelligence","communication","information","technology"}}
        }}
    };

    class Stats
    {
        public double best_wpm { get; set; }
        public int games { get; set; }
        public double total_accuracy { get; set; }
    }

    private string lang;
    private string level;
    private string text;
    private string userInput;
    private DateTime start;
    private DateTime end;
    private Stats stats;
    private string statsFile;

    public TypingTrainer(string lang, string level)
    {
        this.lang = lang;
        this.level = level;
        statsFile = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), $".typing_trainer_{lang}.json");
        LoadStats();
        GenerateText();
    }

    void LoadStats()
    {
        if (File.Exists(statsFile))
        {
            try
            {
                string json = File.ReadAllText(statsFile);
                stats = JsonSerializer.Deserialize<Stats>(json);
            }
            catch { stats = new Stats(); }
        }
        else stats = new Stats();
    }

    void SaveStats()
    {
        string json = JsonSerializer.Serialize(stats);
        File.WriteAllText(statsFile, json);
    }

    void GenerateText()
    {
        var words = WORDS[lang][level];
        Random rnd = new Random();
        int count = level == "hard" ? rnd.Next(8, 16) : rnd.Next(5, 11);
        var textParts = new List<string>();
        for (int i = 0; i < count; i++)
        {
            if (i > 0)
            {
                if (level == "hard" && rnd.NextDouble() < 0.3)
                    textParts.Add(", ");
                else
                    textParts.Add(" ");
            }
            textParts.Add(words[rnd.Next(words.Count)]);
            if (level == "hard" && (i == count-1 || rnd.NextDouble() < 0.2))
                textParts.Add(". ");
        }
        text = string.Concat(textParts).Trim();
    }

    void DisplayText()
    {
        Console.WriteLine(Colorize("\nНапечатайте следующий текст:", "bold"));
        Console.WriteLine(Colorize(text, "blue"));
        Console.WriteLine("\nНажмите Enter после завершения ввода. Время начнётся с первой буквы.");
        Console.WriteLine("Для выхода нажмите Ctrl+C.\n");
    }

    void GetUserInput()
    {
        start = DateTime.Now;
        userInput = Console.ReadLine();
        end = DateTime.Now;
    }

    void CalculateAndDisplay()
    {
        string original = text;
        string typed = userInput ?? "";
        int total = original.Length;
        int typedLen = typed.Length;
        int correct = 0;
        int errors = 0;
        for (int i = 0; i < total; i++)
        {
            if (i < typedLen && original[i] == typed[i])
                correct++;
            else
                errors++;
        }
        if (typedLen < total) errors += total - typedLen;
        double elapsed = (end - start).TotalSeconds;
        double minutes = elapsed / 60.0;
        double wpm = (correct / 5.0) / minutes;
        double accuracy = (correct / (double)total) * 100.0;

        Console.WriteLine(Colorize("\nРезультат:", "bold"));
        for (int i = 0; i < total; i++)
        {
            if (i < typedLen && original[i] == typed[i])
                Console.Write(Colorize(original[i].ToString(), "green"));
            else
                Console.Write(Colorize(original[i].ToString(), "red"));
        }
        Console.WriteLine();

        Console.WriteLine(Colorize($"Скорость: {wpm:F1} WPM", "yellow"));
        Console.WriteLine($"Точность: {accuracy:F1}%");
        Console.WriteLine($"Правильных символов: {correct}/{total}");
        Console.WriteLine($"Ошибок: {errors}");
        Console.WriteLine($"Время: {elapsed:F2} сек");

        stats.games++;
        if (wpm > stats.best_wpm)
        {
            stats.best_wpm = wpm;
            Console.WriteLine(Colorize("🏆 Новый рекорд!", "green"));
        }
        stats.total_accuracy = (stats.total_accuracy * (stats.games - 1) + accuracy) / stats.games;
        SaveStats();
        Console.WriteLine(Colorize($"Лучший результат: {stats.best_wpm:F1} WPM", "blue"));
    }

    public void Run()
    {
        Console.WriteLine(Colorize("⌨️  Тренажёр слепой печати", "bold"));
        Console.WriteLine($"Язык: {lang}, Уровень: {level}");
        DisplayText();
        GetUserInput();
        CalculateAndDisplay();
    }

    static void Main(string[] args)
    {
        string lang = "ru", level = "medium";
        bool showStats = false, resetStats = false;
        for (int i = 0; i < args.Length; i++)
        {
            string arg = args[i];
            if (arg == "ru" || arg == "en") lang = arg;
            else if (arg == "easy" || arg == "medium" || arg == "hard") level = arg;
            else if (arg == "-s" || arg == "--stats") showStats = true;
            else if (arg == "-r" || arg == "--reset") resetStats = true;
            else if (arg == "-h" || arg == "--help")
            {
                Console.WriteLine("Usage: typing_trainer [ru|en] [easy|medium|hard] [-s] [-r]");
                return;
            }
        }
        if (resetStats)
        {
            foreach (var l in new[]{"ru","en"})
            {
                string f = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), $".typing_trainer_{l}.json");
                if (File.Exists(f)) File.Delete(f);
            }
            Console.WriteLine("Статистика сброшена.");
            return;
        }
        if (showStats)
        {
            foreach (var l in new[]{"ru","en"})
            {
                string f = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), $".typing_trainer_{l}.json");
                if (File.Exists(f))
                {
                    try
                    {
                        string json = File.ReadAllText(f);
                        var stats = JsonSerializer.Deserialize<Stats>(json);
                        Console.WriteLine(Colorize($"📊 Статистика для {l}:", "bold"));
                        Console.WriteLine($"  Лучший WPM: {stats.best_wpm:F1}");
                        Console.WriteLine($"  Сыграно игр: {stats.games}");
                        Console.WriteLine($"  Средняя точность: {stats.total_accuracy:F1}%");
                    }
                    catch { Console.WriteLine($"Статистика для {l} повреждена."); }
                }
                else
                {
                    Console.WriteLine($"Статистика для {l} пуста.");
                }
            }
            return;
        }
        TypingTrainer trainer = new TypingTrainer(lang, level);
        trainer.Run();
    }
}
