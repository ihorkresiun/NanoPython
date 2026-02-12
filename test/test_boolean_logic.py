print("=== Boolean Logic Tests ===")

# Basic comparisons
print("5 > 3:", 5 > 3)
print("5 < 3:", 5 < 3)
print("5 == 5:", 5 == 5)
print("5 != 3:", 5 != 3)

# Logical operators in conditionals
if 5 > 3:
    print("5 is greater than 3")

if not 5 < 3:
    print("5 is not less than 3")

# Truthiness
if 1:
    print("1 is truthy")
if 0:
    print("This should not print")
else:
    print("0 is falsy")