# Test loop constructs
print("=== Loop Tests ===")

# While loop
print("While loop:")
i = 0
while i < 5:
    print("i =", i)
    i = i + 1

# For loop with list
print("For loop with list:")
numbers = [1, 2, 3, 4, 5]
for num in numbers:
    print("num =", num)

# For loop with range-like behavior
print("For loop counting:")
count = [0, 1, 2, 3, 4]
for c in count:
    print("count:", c)

# Break statement
print("Break test:")
i = 0
while i < 10:
    if i == 3:
        break
    print("i =", i)
    i = i + 1
print("Broke at i = 3")

# Continue statement
print("Continue test:")
i = 0
while i < 5:
    i = i + 1
    if i == 3:
        continue
    print("i =", i)
