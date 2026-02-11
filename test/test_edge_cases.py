# Test edge cases and special scenarios
print("=== Edge Case Tests ===")

# Empty list
empty = []
print("Empty list:", empty)

# Single element
single = [42]
print("Single element list:", single)

# Zero
zero = 0
print("Zero:", zero)

# Negative numbers
neg = -5
print("Negative number:", neg)

result = neg + 10
print("-5 + 10 =", result)

# Boolean-like values
true_val = 1
false_val = 0

if true_val:
    print("true_val is truthy")

if false_val:
    print("This should not print")
else:
    print("false_val is falsy")

# Function with no parameters
def no_params():
    return 42

result = no_params()
print("no_params() =", result)

# Empty function body with pass
def empty_func():
    pass

empty_func()
print("empty_func completed")

# Chained assignments
a = b = c = 10
print("a =", a)
print("b =", b)
print("c =", c)
