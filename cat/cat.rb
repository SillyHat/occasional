class Integer
  def non_negative
    self < 0 ? 0 : self
  end

  def not_bigger_than(limit)
    self > limit ? limit : self
  end
end

$cats = Hash.new
$cats_mutex = Mutex.new

$male_names = ["Albert", "Boris", "Caesar", "Dennis", "Elon", "Fedor", "George", "Hugo"]
$female_names = ["Anna", "Beatrice", "Clara", "Dorothy", "Eve", "Felicia", "Georgia", "Hilda"]

module RegularCatActions
  def wave
    puts "#{self.name} waves to you!"
    sleep(rand(1..4))
    state_change(0, 1, 2)
  end

  def greet
    another_cat = $cats.keys.reject { |key| key == self or key.alive == false }.sample
    return if another_cat.nil?
    puts "#{self.name}: Hello, #{another_cat.name}!"
    sleep(rand(1..4))
    state_change(1, 1, 1)
  end

  def eat
    puts "*#{self.name} eats*"
    sleep(rand(5..8))
    state_change(-4, 2, 2)
  end

  def rest
    puts "*#{self.name} goes to sleep*"
    sleep(rand(15..25))
    puts "*#{self.name} wakes up!*"
    sleep(rand(1..4))
    state_change(4, 0, -4)
  end

  def play
    puts "*#{self.name} plays*"
    sleep(rand(6..9))
    state_change(3, 3, 3)
  end

  def climb_curtains
    puts "*#{self.name} climbs curtains!*"
    sleep(rand(6..9))
    state_change(3, 5, 4)
  end

  def throw_up
    puts "*#{self.name} throws up!*"
    sleep(rand(3..5))
    state_change(5, -5, 2)
  end

  def poop
    puts "*#{self.name} poops*"
    sleep(rand(3..5))
    state_change(2, 0, 2)
  end

  def look_at_smth
    puts "*#{self.name} looks at something*"
    sleep(rand(1..4))
    state_change(0, 1, 1)
  end

  def punch
    another_cat = $cats.keys.reject { |key| key == self or key.alive == false }.sample
    return if another_cat.nil?
    puts "#{self.name} fights #{another_cat.name}!"
    another_cat.react_to_punch
    sleep(rand(1..4))
    state_change(2, 2, 2)
  end
end

module CatReactions
  def speak
    puts "#{self.name}: meow"
    sleep(rand(1..4))
  end

  def purr
    puts "*#{self.name} purrs*"
    sleep(rand(1..4))
  end

  def hiss
    puts "#{self.name} hisses! Looks like the kitty's angry."
    sleep(rand(1..4))
  end

  def react_to_punch
    self.punch_mutex.synchronize do
      self.punch_count += 1
    end
  end
end

module MaleCatActions
  def mate
    female_cat = $cats.keys.reject { |key| key.instance_of? MaleCat or key.alive == false or key == self.mother }.sample
    return if female_cat.nil?
    puts "#{self.name} mates with #{female_cat.name}!"
    self.react_to_mate
    female_cat.react_to_mate(self)
    sleep(rand(1..4))
  end
end

module FemaleCatActions
  def mate
    male_cat = $cats.keys.reject { |key| key.instance_of? FemaleCat or key.alive == false or key == self.father }.sample
    return if male_cat.nil?
    puts "#{self.name} mates with #{male_cat.name}!"
    self.react_to_mate(male_cat)
    male_cat.react_to_mate
    sleep(rand(1..4))
  end
end

class Cat
  include RegularCatActions, CatReactions

  def initialize(name, father, mother)
    @name = name
    @father = father
    @mother = mother

    @age = 0
    @lifespan = rand(15..20)

    @hunger = 0
    @mood = 5
    @fatigue = 0

    @alive = true

    @punch_count = 0
    @punch_mutex = Mutex.new
  end

  def self.make_cat(father, mother)
    c = nil
    case [0, 1].sample
    when 0
      c = MaleCat.new($male_names.sample, father, mother)
    else
      c = FemaleCat.new($female_names.sample, father, mother)
    end
    $cats_mutex.synchronize do
      $cats[c] = Thread.new {
        c.life_cycle
      }
    end
    puts "==== #{c.name} is born to #{father.name} and #{mother.name}! ===="
  end

  def inspect
    puts "This kitty's name is #{self.name}"
    puts "Hunger:  #{self.hunger}"
    puts "Mood:    #{self.mood}"
    puts "Fatigue: #{self.fatigue}"
  end

  def life_cycle
    (0..self.lifespan).each do
      self.send(RegularCatActions.instance_methods.sample)
      self.age += 1
    end
    sleep(2)
    self.alive = false
    puts "============"
    puts "#{self.name} (#{self.age}) is in a better place now. Cats remaining:"
    $cats.clone.keys.reject { |key| key.alive == false}.each do |cat|
      puts "#{cat.name} (#{cat.age})"
    end
    puts "============"
  end

  def reflect
    self.punch_mutex.synchronize do
      [self.mood, self.fatigue].map { |item| (item - 4 * self.punch_count).non_negative }
      self.punch_count = 0
    end

    self.rest if self.fatigue == 10
    (self.speak; self.eat) if self.hunger == 10
    (self.speak; self.purr) if self.mood == 10
    (self.speak; self.hiss) if self.mood.zero?
  end

  attr_reader :name, :father, :mother, :age, :lifespan, :hunger, :mood, :fatigue, :alive

  protected

  def state_change(hunger_diff, mood_diff, fatigue_diff)
      self.hunger = (self.hunger += hunger_diff).non_negative.not_bigger_than(10)
      self.mood = (self.mood += mood_diff).non_negative.not_bigger_than(10)
      self.fatigue = (self.fatigue += fatigue_diff).non_negative.not_bigger_than(10)
      self.reflect
  end

  attr_writer :age, :hunger, :mood, :fatigue, :alive
  attr_accessor :punch_count, :punch_mutex
end

class MaleCat < Cat
  include MaleCatActions

  def life_cycle
    (0..self.lifespan).each do
      self.send((RegularCatActions.instance_methods + MaleCatActions.instance_methods).sample)
      self.age += 1
    end
    sleep(2)
    self.alive = false
    puts "============"
    puts "#{self.name} (#{self.age}) is in a better place now. Cats remaining:"
    $cats.clone.keys.reject { |key| key.alive == false}.each do |cat|
      puts "#{cat.name} (#{cat.age})"
    end
    puts "============"
  end

  def react_to_mate
    state_change(7, 7, 4)
    self.reflect
  end
end

class FemaleCat < Cat
  include FemaleCatActions

  def initialize(name, father, mother)
    @name = name
    @father = father
    @mother = mother

    @age = 0
    @lifespan = rand(15..20)

    @hunger = 0
    @mood = 5
    @fatigue = 0

    @alive = true

    @punch_count = 0
    @punch_mutex = Mutex.new

    @mate_state = []
    @mate_mutex = Mutex.new
  end

  def life_cycle
    (0..self.lifespan).each do
      self.send((RegularCatActions.instance_methods + FemaleCatActions.instance_methods).sample)
      self.age += 1
    end
    sleep(2)
    self.alive = false
    puts "============"
    puts "#{self.name} (#{self.age}) is in a better place now. Cats remaining:"
    $cats.clone.keys.reject { |key| key.alive == false}.each do |cat|
      puts "#{cat.name} (#{cat.age})"
    end
    puts "============"
  end

  def react_to_mate(male_cat)
    self.mate_mutex.synchronize do
      self.mate_state.push(male_cat) if self.mate_state.empty? and (0..3).to_a.sample.nonzero?
    end
  end

  def reflect
    self.punch_mutex.synchronize do
      [self.mood, self.fatigue].map { |item| (item - 4 * self.punch_count).non_negative }
      self.punch_count = 0
    end

    self.mate_mutex.synchronize do
      (self.mood, self.hunger, self.fatigue = 2, 10, 10; Cat.make_cat(self.mate_state.pop, self)) if self.mate_state.any?
    end

    self.rest if self.fatigue == 10
    (self.speak; self.eat) if self.hunger == 10
    (self.speak; self.purr) if self.mood == 10
    (self.speak; self.hiss) if self.mood.zero?
  end

  protected

  attr_accessor :mate_state, :mate_mutex
end

[MaleCat.new("Adam", nil, nil), MaleCat.new("Boris", nil, nil), FemaleCat.new("Clara", nil, nil), FemaleCat.new("Dorothy", nil, nil)].each do |cat|
  $cats_mutex.synchronize do
    $cats[cat] = Thread.new {
      cat.life_cycle
    }
  end
end

while $cats.clone.any? { |key, _| key.alive == true } do
  sleep(2)
end
