# Test lists, tuples, sets, and dicts
print("=== Data Structure Tests ===")

# Lists
print("List tests:")
lst = [1, 2, 3, 4, 5]
print("List:", lst)
print("First element:", lst[0])
print("Last element:", lst[4])

# Modify list
lst[2] = 99
print("After lst[2] = 99:", lst)

# Tuples
print("Tuple tests:")
tpl = (10, 20, 30)
print("Tuple:", tpl)
print("First element:", tpl[0])
print("Second element:", tpl[1])

# Sets
print("Set tests:")
s1 = {1, 2, 3}
s2 = {3, 4, 5}
print("Set 1:", s1)
print("Set 2:", s2)

# Dictionaries
print("Dictionary tests:")
person = {"name": "Alice", "age": 30}
print("Dict:", person)
print("Name:", person["name"])
print("Age:", person["age"])

# Modify dict
person["age"] = 31
print("After age update:", person)

# Nested structures
nested = [[1, 2], [3, 4], [5, 6]]
print("Nested list:", nested)
print("nested[1][0] =", nested[1][0])
