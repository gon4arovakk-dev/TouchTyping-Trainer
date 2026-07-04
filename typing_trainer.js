// typing_trainer.js
#!/usr/bin/env node
'use strict';

const fs = require('fs');
const path = require('path');
const os = require('os');
const readline = require('readline');

const COLORS = {
    reset: '\x1b[0m',
    green: '\x1b[92m',
    red: '\x1b[91m',
    yellow: '\x1b[93m',
    blue: '\x1b[94m',
    bold: '\x1b[1m'
};

function colorize(text, color) {
    return COLORS[color] + text + COLORS.reset;
}

const WORDS = {
    ru: {
        easy: ['кот','нос','рот','дом','лес','море','вода','рука','нога','мир'],
        medium: ['компьютер','программа','алгоритм','интернет','сервер','сайт','код','данные'],
        hard: ['программирование','искусственный интеллект','коммуникация','информация','технология']
    },
    en: {
        easy: ['cat','dog','sun','run','jump','tree','book','fish','bird','hand'],
        medium: ['computer','program','algorithm','internet','server','website','code','data'],
        hard: ['programming','artificial intelligence','communication','information','technology']
    }
};

class Trainer {
    constructor(lang, level) {
        this.lang = lang;
        this.level = level;
        this.statsFile = path.join(os.homedir(), `.typing_trainer_${lang}.json`);
        this.loadStats();
        this.generateText();
    }

    loadStats() {
        try {
            this.stats = JSON.parse(fs.readFileSync(this.statsFile, 'utf8'));
        } catch {
            this.stats = { best_wpm: 0, games: 0, total_accuracy: 0 };
        }
    }

    saveStats() {
        fs.writeFileSync(this.statsFile, JSON.stringify(this.stats, null, 2));
    }

    generateText() {
        const words = WORDS[this.lang][this.level];
        const count = this.level === 'hard' ? Math.floor(Math.random() * 8) + 8 : Math.floor(Math.random() * 6) + 5;
        let text = '';
        for (let i = 0; i < count; i++) {
            if (i > 0) {
                if (this.level === 'hard' && Math.random() < 0.3) {
                    text += ', ';
                } else {
                    text += ' ';
                }
            }
            const word = words[Math.floor(Math.random() * words.length)];
            text += word;
            if (this.level === 'hard' && (i === count-1 || Math.random() < 0.2)) {
                text += '. ';
            }
        }
        this.text = text.trim();
    }

    displayText() {
        console.log(colorize('\nНапечатайте следующий текст:', 'bold'));
        console.log(colorize(this.text, 'blue'));
        console.log('\nНажмите Enter после завершения ввода. Время начнётся с первой буквы.');
        console.log('Для выхода нажмите Ctrl+C.\n');
    }

    async getUserInput() {
        const rl = readline.createInterface({
            input: process.stdin,
            output: process.stdout
        });
        this.start = Date.now();
        const input = await new Promise(resolve => {
            rl.question('', answer => {
                resolve(answer);
                rl.close();
            });
        });
        this.userInput = input;
        this.end = Date.now();
    }

    calculateAndDisplay() {
        const original = this.text;
        const typed = this.userInput || '';
        const total = original.length;
        const typedLen = typed.length;
        let correct = 0;
        let errors = 0;
        for (let i = 0; i < total; i++) {
            if (i < typedLen && original[i] === typed[i]) {
                correct++;
            } else {
                errors++;
            }
        }
        if (typedLen < total) errors += total - typedLen;
        const elapsed = (this.end - this.start) / 1000;
        const minutes = elapsed / 60;
        const wpm = (correct / 5) / minutes;
        const accuracy = (correct / total) * 100;

        console.log(colorize('\nРезультат:', 'bold'));
        for (let i = 0; i < total; i++) {
            if (i < typedLen && original[i] === typed[i]) {
                process.stdout.write(colorize(original[i], 'green'));
            } else {
                process.stdout.write(colorize(original[i], 'red'));
            }
        }
        console.log();

        console.log(colorize(`Скорость: ${wpm.toFixed(1)} WPM`, 'yellow'));
        console.log(`Точность: ${accuracy.toFixed(1)}%`);
        console.log(`Правильных символов: ${correct}/${total}`);
        console.log(`Ошибок: ${errors}`);
        console.log(`Время: ${elapsed.toFixed(2)} сек`);

        this.stats.games++;
        if (wpm > this.stats.best_wpm) {
            this.stats.best_wpm = wpm;
            console.log(colorize('🏆 Новый рекорд!', 'green'));
        }
        this.stats.total_accuracy = (this.stats.total_accuracy * (this.stats.games-1) + accuracy) / this.stats.games;
        this.saveStats();
        console.log(colorize(`Лучший результат: ${this.stats.best_wpm.toFixed(1)} WPM`, 'blue'));
    }

    async run() {
        console.log(colorize('⌨️  Тренажёр слепой печати', 'bold'));
        console.log(`Язык: ${this.lang}, Уровень: ${this.level}`);
        this.displayText();
        await this.getUserInput();
        this.calculateAndDisplay();
    }
}

async function main() {
    let lang = 'ru', level = 'medium';
    let showStats = false, resetStats = false;
    const args = process.argv.slice(2);
    for (let i = 0; i < args.length; i++) {
        const arg = args[i];
        if (arg === 'ru' || arg === 'en') lang = arg;
        else if (arg === 'easy' || arg === 'medium' || arg === 'hard') level = arg;
        else if (arg === '-s' || arg === '--stats') showStats = true;
        else if (arg === '-r' || arg === '--reset') resetStats = true;
        else if (arg === '-h' || arg === '--help') {
            console.log('Usage: node typing_trainer.js [ru|en] [easy|medium|hard] [-s] [-r]');
            return;
        }
    }
    if (resetStats) {
        ['ru', 'en'].forEach(l => {
            const f = path.join(os.homedir(), `.typing_trainer_${l}.json`);
            if (fs.existsSync(f)) fs.unlinkSync(f);
        });
        console.log('Статистика сброшена.');
        return;
    }
    if (showStats) {
        ['ru', 'en'].forEach(l => {
            const f = path.join(os.homedir(), `.typing_trainer_${l}.json`);
            if (fs.existsSync(f)) {
                const stats = JSON.parse(fs.readFileSync(f, 'utf8'));
                console.log(colorize(`📊 Статистика для ${l}:`, 'bold'));
                console.log(`  Лучший WPM: ${stats.best_wpm.toFixed(1)}`);
                console.log(`  Сыграно игр: ${stats.games}`);
                console.log(`  Средняя точность: ${stats.total_accuracy.toFixed(1)}%`);
            } else {
                console.log(`Статистика для ${l} пуста.`);
            }
        });
        return;
    }
    const trainer = new Trainer(lang, level);
    await trainer.run();
}

main().catch(console.error);
