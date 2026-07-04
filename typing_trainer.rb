#!/usr/bin/env ruby
# typing_trainer.rb
# encoding: UTF-8

require 'json'
require 'fileutils'
require 'time'

COLORS = {
  reset: "\e[0m",
  green: "\e[92m",
  red: "\e[91m",
  yellow: "\e[93m",
  blue: "\e[94m",
  bold: "\e[1m"
}

def colorize(text, color)
  "#{COLORS[color]}#{text}#{COLORS[:reset]}"
end

WORDS = {
  'ru' => {
    'easy' => %w[кот нос рот дом лес море вода рука нога мир],
    'medium' => %w[компьютер программа алгоритм интернет сервер сайт код данные],
    'hard' => %w[программирование искусственный\ интеллект коммуникация информация технология]
  },
  'en' => {
    'easy' => %w[cat dog sun run jump tree book fish bird hand],
    'medium' => %w[computer program algorithm internet server website code data],
    'hard' => %w[programming artificial\ intelligence communication information technology]
  }
}

class Trainer
  attr_reader :lang, :level, :text, :user_input, :start_time, :end_time, :stats_file

  def initialize(lang, level)
    @lang = lang
    @level = level
    @stats_file = File.join(Dir.home, ".typing_trainer_#{lang}.json")
    load_stats
    generate_text
  end

  def load_stats
    if File.exist?(@stats_file)
      @stats = JSON.parse(File.read(@stats_file))
    else
      @stats = { 'best_wpm' => 0, 'games' => 0, 'total_accuracy' => 0 }
    end
  end

  def save_stats
    File.write(@stats_file, JSON.pretty_generate(@stats))
  end

  def generate_text
    words = WORDS[@lang][@level]
    count = @level == 'hard' ? rand(8..15) : rand(5..10)
    parts = []
    count.times do |i|
      parts << ' ' if i > 0 && (@level != 'hard' || rand < 0.3 ? ' ' : ', ')
      parts << words.sample
      parts << '. ' if @level == 'hard' && (i == count-1 || rand < 0.2)
    end
    @text = parts.join.strip
  end

  def display_text
    puts colorize("\nНапечатайте следующий текст:", :bold)
    puts colorize(@text, :blue)
    puts "\nНажмите Enter после завершения ввода. Время начнётся с первой буквы."
    puts "Для выхода нажмите Ctrl+C.\n"
  end

  def get_user_input
    @start_time = Time.now
    @user_input = gets.chomp
    @end_time = Time.now
  rescue Interrupt
    puts colorize("\nВыход.", :yellow)
    exit
  end

  def calculate_and_display
    original = @text
    typed = @user_input || ''
    total = original.length
    typed_len = typed.length
    correct = 0
    errors = 0
    original.chars.each_with_index do |ch, i|
      if i < typed_len && ch == typed[i]
        correct += 1
      else
        errors += 1
      end
    end
    if typed_len < total
      errors += total - typed_len
    end
    elapsed = @end_time - @start_time
    minutes = elapsed / 60.0
    wpm = (correct / 5.0) / minutes
    accuracy = (correct / total.to_f) * 100.0

    puts colorize("\nРезультат:", :bold)
    original.chars.each_with_index do |ch, i|
      if i < typed_len && ch == typed[i]
        print colorize(ch, :green)
      else
        print colorize(ch, :red)
      end
    end
    puts

    puts colorize("Скорость: #{wpm.round(1)} WPM", :yellow)
    puts "Точность: #{accuracy.round(1)}%"
    puts "Правильных символов: #{correct}/#{total}"
    puts "Ошибок: #{errors}"
    puts "Время: #{elapsed.round(2)} сек"

    @stats['games'] += 1
    if wpm > @stats['best_wpm']
      @stats['best_wpm'] = wpm
      puts colorize("🏆 Новый рекорд!", :green)
    end
    @stats['total_accuracy'] = (@stats['total_accuracy'] * (@stats['games']-1) + accuracy) / @stats['games']
    save_stats
    puts colorize("Лучший результат: #{@stats['best_wpm'].round(1)} WPM", :blue)
  end

  def run
    puts colorize("⌨️  Тренажёр слепой печати", :bold)
    puts "Язык: #{@lang}, Уровень: #{@level}"
    display_text
    get_user_input
    calculate_and_display
  end
end

def main
  lang = 'ru'
  level = 'medium'
  show_stats = false
  reset_stats = false
  i = 0
  while i < ARGV.size
    arg = ARGV[i]
    case arg
    when 'ru', 'en' then lang = arg
    when 'easy', 'medium', 'hard' then level = arg
    when '-s', '--stats' then show_stats = true
    when '-r', '--reset' then reset_stats = true
    when '-h', '--help'
      puts "Usage: ruby typing_trainer.rb [ru|en] [easy|medium|hard] [-s] [-r]"
      return
    end
    i += 1
  end
  if reset_stats
    ['ru', 'en'].each do |l|
      f = File.join(Dir.home, ".typing_trainer_#{l}.json")
      File.delete(f) if File.exist?(f)
    end
    puts 'Статистика сброшена.'
    return
  end
  if show_stats
    ['ru', 'en'].each do |l|
      f = File.join(Dir.home, ".typing_trainer_#{l}.json")
      if File.exist?(f)
        stats = JSON.parse(File.read(f))
        puts colorize("📊 Статистика для #{l}:", :bold)
        puts "  Лучший WPM: #{stats['best_wpm'].round(1)}"
        puts "  Сыграно игр: #{stats['games']}"
        puts "  Средняя точность: #{stats['total_accuracy'].round(1)}%"
      else
        puts "Статистика для #{l} пуста."
      end
    end
    return
  end
  trainer = Trainer.new(lang, level)
  trainer.run
end

main if __FILE__ == $0
