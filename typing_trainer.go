// typing_trainer.go
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"strings"
	"time"
)

const (
	reset  = "\033[0m"
	green  = "\033[92m"
	red    = "\033[91m"
	yellow = "\033[93m"
	blue   = "\033[94m"
	bold   = "\033[1m"
)

func colorize(text, color string) string {
	return color + text + reset
}

var wordsMap = map[string]map[string][]string{
	"ru": {
		"easy":   {"кот", "нос", "рот", "дом", "лес", "море", "вода", "рука", "нога", "мир"},
		"medium": {"компьютер", "программа", "алгоритм", "интернет", "сервер", "сайт", "код", "данные"},
		"hard":   {"программирование", "искусственный интеллект", "коммуникация", "информация", "технология"},
	},
	"en": {
		"easy":   {"cat", "dog", "sun", "run", "jump", "tree", "book", "fish", "bird", "hand"},
		"medium": {"computer", "program", "algorithm", "internet", "server", "website", "code", "data"},
		"hard":   {"programming", "artificial intelligence", "communication", "information", "technology"},
	},
}

type Stats struct {
	BestWPM       float64 `json:"best_wpm"`
	Games         int     `json:"games"`
	TotalAccuracy float64 `json:"total_accuracy"`
}

type Trainer struct {
	lang   string
	level  string
	text   string
	start  time.Time
	end    time.Time
	input  string
	stats  Stats
	statsFile string
}

func NewTrainer(lang, level string) *Trainer {
	t := &Trainer{lang: lang, level: level}
	t.statsFile = filepath.Join(os.Getenv("HOME"), fmt.Sprintf(".typing_trainer_%s.json", lang))
	t.loadStats()
	t.generateText()
	return t
}

func (t *Trainer) loadStats() {
	data, err := os.ReadFile(t.statsFile)
	if err != nil {
		t.stats = Stats{}
		return
	}
	json.Unmarshal(data, &t.stats)
}

func (t *Trainer) saveStats() {
	data, _ := json.MarshalIndent(t.stats, "", "  ")
	os.WriteFile(t.statsFile, data, 0644)
}

func (t *Trainer) generateText() {
	words := wordsMap[t.lang][t.level]
	rand.Seed(time.Now().UnixNano())
	count := rand.Intn(6) + 5 // 5-10 слов
	if t.level == "hard" {
		count = rand.Intn(8) + 8 // 8-15
	}
	var builder strings.Builder
	for i := 0; i < count; i++ {
		if i > 0 {
			if t.level == "hard" && rand.Float32() < 0.3 {
				builder.WriteString(", ")
			} else {
				builder.WriteString(" ")
			}
		}
		word := words[rand.Intn(len(words))]
		builder.WriteString(word)
		if t.level == "hard" && (i == count-1 || rand.Float32() < 0.2) {
			builder.WriteString(". ")
		}
	}
	t.text = strings.TrimSpace(builder.String())
}

func (t *Trainer) displayText() {
	fmt.Println(colorize("\nНапечатайте следующий текст:", bold))
	fmt.Println(colorize(t.text, blue))
	fmt.Println("\nНажмите Enter после завершения ввода. Время начнётся с первой буквы.")
	fmt.Println("Для выхода нажмите Ctrl+C.\n")
}

func (t *Trainer) getUserInput() {
	reader := bufio.NewReader(os.Stdin)
	t.start = time.Now()
	input, _ := reader.ReadString('\n')
	t.input = strings.TrimRight(input, "\n")
	t.end = time.Now()
}

func (t *Trainer) calculateAndDisplay() {
	original := t.text
	typed := t.input
	total := len(original)
	typedLen := len(typed)
	correct := 0
	errors := 0
	for i := 0; i < total; i++ {
		if i < typedLen && original[i] == typed[i] {
			correct++
		} else {
			errors++
		}
	}
	if typedLen < total {
		errors += total - typedLen
	}
	elapsed := t.end.Sub(t.start).Seconds()
	minutes := elapsed / 60.0
	wpm := (float64(correct) / 5.0) / minutes
	if minutes == 0 {
		wpm = 0
	}
	accuracy := (float64(correct) / float64(total)) * 100.0

	// Цветной вывод
	fmt.Println(colorize("\nРезультат:", bold))
	for i, ch := range original {
		if i < typedLen && ch == rune(typed[i]) {
			fmt.Print(colorize(string(ch), green))
		} else {
			fmt.Print(colorize(string(ch), red))
		}
	}
	fmt.Println()

	fmt.Printf("%s: %.1f WPM\n", colorize("Скорость", yellow), wpm)
	fmt.Printf("Точность: %.1f%%\n", accuracy)
	fmt.Printf("Правильных символов: %d/%d\n", correct, total)
	fmt.Printf("Ошибок: %d\n", errors)
	fmt.Printf("Время: %.2f сек\n", elapsed)

	t.stats.Games++
	if wpm > t.stats.BestWPM {
		t.stats.BestWPM = wpm
		fmt.Println(colorize("🏆 Новый рекорд!", green))
	}
	t.stats.TotalAccuracy = (t.stats.TotalAccuracy*float64(t.stats.Games-1) + accuracy) / float64(t.stats.Games)
	t.saveStats()
	fmt.Printf("%s: %.1f WPM\n", colorize("Лучший результат", blue), t.stats.BestWPM)
}

func (t *Trainer) run() {
	fmt.Println(colorize("⌨️  Тренажёр слепой печати", bold))
	fmt.Printf("Язык: %s, Уровень: %s\n", t.lang, t.level)
	t.displayText()
	t.getUserInput()
	t.calculateAndDisplay()
}

func main() {
	lang := "ru"
	level := "medium"
	showStats := false
	resetStats := false
	args := os.Args[1:]
	for i := 0; i < len(args); i++ {
		arg := args[i]
		switch arg {
		case "ru", "en":
			lang = arg
		case "easy", "medium", "hard":
			level = arg
		case "-s", "--stats":
			showStats = true
		case "-r", "--reset":
			resetStats = true
		case "-h", "--help":
			fmt.Println("Usage: typing_trainer [ru|en] [easy|medium|hard] [-s] [-r]")
			return
		}
	}
	if resetStats {
		for _, l := range []string{"ru", "en"} {
			f := filepath.Join(os.Getenv("HOME"), fmt.Sprintf(".typing_trainer_%s.json", l))
			os.Remove(f)
		}
		fmt.Println("Статистика сброшена.")
		return
	}
	if showStats {
		for _, l := range []string{"ru", "en"} {
			f := filepath.Join(os.Getenv("HOME"), fmt.Sprintf(".typing_trainer_%s.json", l))
			data, err := os.ReadFile(f)
			if err != nil {
				fmt.Printf("Статистика для %s пуста.\n", l)
				continue
			}
			var stats Stats
			json.Unmarshal(data, &stats)
			fmt.Printf(colorize("📊 Статистика для %s:\n", bold), l)
			fmt.Printf("  Лучший WPM: %.1f\n", stats.BestWPM)
			fmt.Printf("  Сыграно игр: %d\n", stats.Games)
			fmt.Printf("  Средняя точность: %.1f%%\n", stats.TotalAccuracy)
		}
		return
	}
	trainer := NewTrainer(lang, level)
	trainer.run()
}
