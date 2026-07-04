// typing_trainer.java
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.stream.*;

public class typing_trainer {
    private static final String RESET = "\u001B[0m";
    private static final String GREEN = "\u001B[92m";
    private static final String RED = "\u001B[91m";
    private static final String YELLOW = "\u001B[93m";
    private static final String BLUE = "\u001B[94m";
    private static final String BOLD = "\u001B[1m";

    private static String colorize(String text, String color) {
        return color + text + RESET;
    }

    private static final Map<String, Map<String, List<String>>> WORDS = new HashMap<>();
    static {
        Map<String, List<String>> ru = new HashMap<>();
        ru.put("easy", Arrays.asList("кот","нос","рот","дом","лес","море","вода","рука","нога","мир"));
        ru.put("medium", Arrays.asList("компьютер","программа","алгоритм","интернет","сервер","сайт","код","данные"));
        ru.put("hard", Arrays.asList("программирование","искусственный интеллект","коммуникация","информация","технология"));
        Map<String, List<String>> en = new HashMap<>();
        en.put("easy", Arrays.asList("cat","dog","sun","run","jump","tree","book","fish","bird","hand"));
        en.put("medium", Arrays.asList("computer","program","algorithm","internet","server","website","code","data"));
        en.put("hard", Arrays.asList("programming","artificial intelligence","communication","information","technology"));
        WORDS.put("ru", ru);
        WORDS.put("en", en);
    }

    private static class Stats {
        double best_wpm = 0;
        int games = 0;
        double total_accuracy = 0;
    }

    private String lang, level;
    private String text, userInput;
    private long start, end;
    private Stats stats;
    private String statsFile;
    private Scanner scanner;

    public typing_trainer(String lang, String level) {
        this.lang = lang;
        this.level = level;
        statsFile = System.getProperty("user.home") + "/.typing_trainer_" + lang + ".json";
        loadStats();
        generateText();
        scanner = new Scanner(System.in);
    }

    private void loadStats() {
        stats = new Stats();
        try {
            String json = new String(Files.readAllBytes(Paths.get(statsFile)));
            // простой парсинг
            stats.best_wpm = extractDouble(json, "best_wpm");
            stats.games = (int)extractDouble(json, "games");
            stats.total_accuracy = extractDouble(json, "total_accuracy");
        } catch (Exception e) {}
    }

    private double extractDouble(String json, String key) {
        int idx = json.indexOf("\"" + key + "\"");
        if (idx == -1) return 0;
        int start = json.indexOf(":", idx) + 1;
        int end = json.indexOf(",", start);
        if (end == -1) end = json.indexOf("}", start);
        String val = json.substring(start, end).trim();
        try { return Double.parseDouble(val); } catch (Exception e) { return 0; }
    }

    private void saveStats() {
        try {
            String json = "{\"best_wpm\":" + stats.best_wpm + ",\"games\":" + stats.games +
                          ",\"total_accuracy\":" + stats.total_accuracy + "}";
            Files.write(Paths.get(statsFile), json.getBytes());
        } catch (IOException e) {}
    }

    private void generateText() {
        List<String> words = WORDS.get(lang).get(level);
        Random rnd = new Random();
        int count = level.equals("hard") ? rnd.nextInt(8) + 8 : rnd.nextInt(6) + 5;
        StringBuilder sb = new StringBuilder();
        for (int i=0; i<count; i++) {
            if (i>0) {
                if (level.equals("hard") && rnd.nextDouble() < 0.3) {
                    sb.append(", ");
                } else {
                    sb.append(" ");
                }
            }
            sb.append(words.get(rnd.nextInt(words.size())));
            if (level.equals("hard") && (i==count-1 || rnd.nextDouble() < 0.2)) {
                sb.append(". ");
            }
        }
        text = sb.toString().trim();
    }

    private void displayText() {
        System.out.println(colorize("\nНапечатайте следующий текст:", BOLD));
        System.out.println(colorize(text, BLUE));
        System.out.println("\nНажмите Enter после завершения ввода. Время начнётся с первой буквы.");
        System.out.println("Для выхода нажмите Ctrl+C.\n");
    }

    private void getUserInput() {
        start = System.currentTimeMillis();
        userInput = scanner.nextLine();
        end = System.currentTimeMillis();
    }

    private void calculateAndDisplay() {
        String original = text;
        String typed = userInput != null ? userInput : "";
        int total = original.length();
        int typedLen = typed.length();
        int correct = 0;
        int errors = 0;
        for (int i=0; i<total; i++) {
            if (i < typedLen && original.charAt(i) == typed.charAt(i)) {
                correct++;
            } else {
                errors++;
            }
        }
        if (typedLen < total) errors += total - typedLen;
        double elapsed = (end - start) / 1000.0;
        double minutes = elapsed / 60.0;
        double wpm = (correct / 5.0) / minutes;
        double accuracy = (correct / (double) total) * 100.0;

        System.out.println(colorize("\nРезультат:", BOLD));
        for (int i=0; i<total; i++) {
            if (i < typedLen && original.charAt(i) == typed.charAt(i)) {
                System.out.print(colorize(String.valueOf(original.charAt(i)), GREEN));
            } else {
                System.out.print(colorize(String.valueOf(original.charAt(i)), RED));
            }
        }
        System.out.println();

        System.out.printf("%s: %.1f WPM\n", colorize("Скорость", YELLOW), wpm);
        System.out.printf("Точность: %.1f%%\n", accuracy);
        System.out.printf("Правильных символов: %d/%d\n", correct, total);
        System.out.printf("Ошибок: %d\n", errors);
        System.out.printf("Время: %.2f сек\n", elapsed);

        stats.games++;
        if (wpm > stats.best_wpm) {
            stats.best_wpm = wpm;
            System.out.println(colorize("🏆 Новый рекорд!", GREEN));
        }
        stats.total_accuracy = (stats.total_accuracy * (stats.games-1) + accuracy) / stats.games;
        saveStats();
        System.out.printf("%s: %.1f WPM\n", colorize("Лучший результат", BLUE), stats.best_wpm);
    }

    public void run() {
        System.out.println(colorize("⌨️  Тренажёр слепой печати", BOLD));
        System.out.printf("Язык: %s, Уровень: %s\n", lang, level);
        displayText();
        getUserInput();
        calculateAndDisplay();
        scanner.close();
    }

    public static void main(String[] args) {
        String lang = "ru", level = "medium";
        boolean showStats = false, resetStats = false;
        for (int i=0; i<args.length; i++) {
            String arg = args[i];
            if (arg.equals("ru") || arg.equals("en")) lang = arg;
            else if (arg.equals("easy") || arg.equals("medium") || arg.equals("hard")) level = arg;
            else if (arg.equals("-s") || arg.equals("--stats")) showStats = true;
            else if (arg.equals("-r") || arg.equals("--reset")) resetStats = true;
            else if (arg.equals("-h") || arg.equals("--help")) {
                System.out.println("Usage: typing_trainer [ru|en] [easy|medium|hard] [-s] [-r]");
                return;
            }
        }
        if (resetStats) {
            for (String l : new String[]{"ru","en"}) {
                String f = System.getProperty("user.home") + "/.typing_trainer_" + l + ".json";
                try { Files.deleteIfExists(Paths.get(f)); } catch (Exception e) {}
            }
            System.out.println("Статистика сброшена.");
            return;
        }
        if (showStats) {
            for (String l : new String[]{"ru","en"}) {
                String f = System.getProperty("user.home") + "/.typing_trainer_" + l + ".json";
                try {
                    String json = new String(Files.readAllBytes(Paths.get(f)));
                    typing_trainer trainer = new typing_trainer(l, "medium"); // just to parse
                    double best = trainer.extractDouble(json, "best_wpm");
                    int games = (int)trainer.extractDouble(json, "games");
                    double acc = trainer.extractDouble(json, "total_accuracy");
                    System.out.println(colorize("📊 Статистика для " + l + ":", BOLD));
                    System.out.printf("  Лучший WPM: %.1f\n", best);
                    System.out.printf("  Сыграно игр: %d\n", games);
                    System.out.printf("  Средняя точность: %.1f%%\n", acc);
                } catch (Exception e) {
                    System.out.println("Статистика для " + l + " пуста.");
                }
            }
            return;
        }
        typing_trainer trainer = new typing_trainer(lang, level);
        trainer.run();
    }
}
