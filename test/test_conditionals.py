# Test if/else statements
print("=== Conditional Tests ===")

x = 10
print("x =", x)

if x > 5:
    print("x is greater than 5")

if x < 5:
    print("x is less than 5")
else:
    print("x is not less than 5")

# Nested if
if x > 0:
    if x > 5:
        print("x is positive and greater than 5")
    else:
        print("x is positive but not greater than 5")
else:
    print("x is not positive")

# Comparison operators
a = 10
b = 10
c = 20

if a == b:
    print("a equals b")

if a < c:
    print("a is less than c")

if c > a:
    print("c is greater than a")
