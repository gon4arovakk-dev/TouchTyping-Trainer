# typing_trainer.py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import random
import json
import time
from pathlib import Path

# ANSI-цвета
COLORS = {
    'reset': '\033[0m',
    'green': '\033[92m',
    'red': '\033[91m',
    'yellow': '\033[93m',
    'blue': '\033[94m',
    'bold': '\033[1m'
}

def colorize(text, color):
    return f"{COLORS.get(color, '')}{text}{COLORS['reset']}"

# Словари по языкам и уровням
WORDS = {
    'ru': {
        'easy': ['кот', 'нос', 'рот', 'дом', 'лес', 'море', 'вода', 'рука', 'нога', 'мир'],
        'medium': ['компьютер', 'программа', 'алгоритм', 'интернет', 'сервер', 'сайт', 'код', 'данные'],
        'hard': ['программирование', 'искусственный интеллект', 'коммуникация', 'информация', 'технология']
    },
    'en': {
        'easy': ['cat', 'dog', 'sun', 'run', 'jump', 'tree', 'book', 'fish', 'bird', 'hand'],
        'medium': ['computer', 'program', 'algorithm', 'internet', 'server', 'website', 'code', 'data'],
        'hard': ['programming', 'artificial intelligence', 'communication', 'information', 'technology']
    }
}

class TypingTrainer:
    def __init__(self, lang='ru', level='medium'):
        self.lang = lang
        self.level = level
        self.text = self.generate_text()
        self.start_time = None
        self.end_time = None
        self.user_input = ""
        self.stats_file = Path.home() / f'.typing_trainer_{lang}.json'
        self.load_stats()

    def generate_text(self):
        words = WORDS[self.lang][self.level]
        # Случайное количество слов
        count = random.randint(5, 10)
        if self.level == 'hard':
            count = random.randint(8, 15)
        # Добавляем запятые и точки для сложного уровня
        if self.level == 'hard':
            text = ''
            for i in range(count):
                word = random.choice(words)
                if i > 0 and random.random() < 0.3:
                    text += ', '
                elif i > 0:
                    text += ' '
                text += word
                if i == count - 1 or random.random() < 0.2:
                    text += '. '
            return text.strip()
        else:
            return ' '.join(random.choice(words) for _ in range(count))

    def load_stats(self):
        if self.stats_file.exists():
            with open(self.stats_file, 'r') as f:
                self.stats = json.load(f)
        else:
            self.stats = {'best_wpm': 0, 'games': 0, 'total_accuracy': 0}

    def save_stats(self):
        with open(self.stats_file, 'w') as f:
            json.dump(self.stats, f, indent=2)

    def display_text(self):
        print(colorize("\nНапечатайте следующий текст:", 'bold'))
        print(colorize(self.text, 'blue'))
        print("\nНажмите Enter после завершения ввода. Время начнётся с первой буквы.")
        print("Для выхода нажмите Ctrl+C.\n")

    def get_user_input(self):
        self.start_time = time.time()
        try:
            self.user_input = input()
        except KeyboardInterrupt:
            print(colorize("\nВыход.", 'yellow'))
            sys.exit(0)
        self.end_time = time.time()

    def calculate_and_display(self):
        original = self.text
        typed = self.user_input
        total = len(original)
        typed_len = len(typed)
        correct = 0
        errors = 0
        # Посимвольное сравнение
        for i in range(total):
            if i < typed_len and original[i] == typed[i]:
                correct += 1
            else:
                errors += 1
        if typed_len < total:
            errors += total - typed_len

        elapsed = self.end_time - self.start_time
        minutes = elapsed / 60.0
        # WPM: правильные символы / 5 / минуты
        wpm = (correct / 5.0) / minutes if minutes > 0 else 0
        accuracy = (correct / total) * 100 if total > 0 else 0

        # Цветной вывод текста с ошибками
        print(colorize("\nРезультат:", 'bold'))
        for i, ch in enumerate(original):
            if i < typed_len and ch == typed[i]:
                print(colorize(ch, 'green'), end='')
            else:
                print(colorize(ch, 'red'), end='')
        print()

        print(colorize(f"Скорость: {wpm:.1f} WPM", 'yellow'))
        print(f"Точность: {accuracy:.1f}%")
        print(f"Правильных символов: {correct}/{total}")
        print(f"Ошибок: {errors}")
        print(f"Время: {elapsed:.2f} сек")

        # Обновление статистики
        self.stats['games'] += 1
        if wpm > self.stats['best_wpm']:
            self.stats['best_wpm'] = wpm
            print(colorize("🏆 Новый рекорд!", 'green'))
        self.stats['total_accuracy'] = (self.stats['total_accuracy'] * (self.stats['games']-1) + accuracy) / self.stats['games']
        self.save_stats()

        print(colorize(f"Лучший результат: {self.stats['best_wpm']:.1f} WPM", 'blue'))

    def run(self):
        print(colorize("⌨️  Тренажёр слепой печати", 'bold'))
        print(f"Язык: {self.lang}, Уровень: {self.level}")
        self.display_text()
        self.get_user_input()
        self.calculate_and_display()

def main():
    lang = 'ru'
    level = 'medium'
    show_stats = False
    reset_stats = False
    args = sys.argv[1:]
    for i, arg in enumerate(args):
        if arg in ['ru', 'en']:
            lang = arg
        elif arg in ['easy', 'medium', 'hard']:
            level = arg
        elif arg == '-s' or arg == '--stats':
            show_stats = True
        elif arg == '-r' or arg == '--reset':
            reset_stats = True
        elif arg == '-h' or arg == '--help':
            print("Usage: typing_trainer.py [ru|en] [easy|medium|hard] [-s] [-r]")
            return
    if reset_stats:
        for l in ['ru', 'en']:
            f = Path.home() / f'.typing_trainer_{l}.json'
            if f.exists():
                f.unlink()
        print("Статистика сброшена.")
        return
    if show_stats:
        for l in ['ru', 'en']:
            f = Path.home() / f'.typing_trainer_{l}.json'
            if f.exists():
                with open(f, 'r') as fp:
                    stats = json.load(fp)
                    print(colorize(f"📊 Статистика для {l}:", 'bold'))
                    print(f"  Лучший WPM: {stats['best_wpm']:.1f}")
                    print(f"  Сыграно игр: {stats['games']}")
                    print(f"  Средняя точность: {stats['total_accuracy']:.1f}%")
            else:
                print(f"Статистика для {l} пуста.")
        return
    trainer = TypingTrainer(lang, level)
    trainer.run()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print(colorize("\nВыход.", 'yellow'))
        sys.exit(0)
