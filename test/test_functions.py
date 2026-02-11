# Test function definitions and calls
print("=== Function Tests ===")

# Simple function with no parameters
def greet():
    print("Hello, World!")

greet()

# Function with parameters
def add(a, b):
    return a + b

result = add(5, 3)
print("add(5, 3) =", result)

# Function with multiple parameters
def multiply(x, y, z):
    return x * y * z

result = multiply(2, 3, 4)
print("multiply(2, 3, 4) =", result)

# Function calling another function
def square(n):
    return n * n

def sum_of_squares(a, b):
    return square(a) + square(b)

result = sum_of_squares(3, 4)
print("sum_of_squares(3, 4) =", result)

# Recursive function
def factorial(n):
    if n == 0:
        return 1
    else:
        return n * factorial(n - 1)

result = factorial(5)
print("factorial(5) =", result)

# Function with no return (implicit None)
def no_return():
    x = 42

result = no_return()
print("no_return() returned:", result)
