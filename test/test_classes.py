# Test class definitions and objects
print("=== Class Tests ===")

# Simple class
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    
    def display(self):
        print("Point(", self.x, ",", self.y, ")")

p1 = Point(10, 20)
p1.display()
print("p1.x =", p1.x)
print("p1.y =", p1.y)

# Modify attributes
p1.x = 15
p1.display()

# Multiple instances
p2 = Point(5, 7)
p2.display()

# Class with methods
class Calculator:
    def add(self, a, b):
        return a + b
    
    def multiply(self, a, b):
        return a * b

calc = Calculator()
print("calc.add(5, 3) =", calc.add(5, 3))
print("calc.multiply(4, 6) =", calc.multiply(4, 6))

# Class inheritance
class Animal:
    def __init__(self, name):
        self.name = name
    
    def speak(self):
        print("Animal speaks")

class Dog(Animal):
    def speak(self):
        print(self.name, "says: Woof!")

dog = Dog("Buddy")
dog.speak()
print("Dog's name:", dog.name)
