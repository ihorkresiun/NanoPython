# Test garbage collection
print("=== Garbage Collection Tests ===")

# Test 1: Create many objects to trigger GC
print("Test 1: Creating many objects")
count = 0
while count < 100:
    temp_list = [1, 2, 3, 4, 5]
    temp_dict = {"a": 1, "b": 2, "c": 3}
    temp_str = "This is a temporary string"
    count = count + 1
print("Created 100 iterations of objects")

# Test 2: Objects going out of scope
print("Test 2: Objects going out of scope")
def create_objects():
    local_list = [10, 20, 30, 40, 50]
    local_dict = {"x": 100, "y": 200}
    local_str = "Local string that should be collected"
    return 42

result = create_objects()
print("Function returned:", result)

# Test 3: Reassigning variables
print("Test 3: Reassigning variables")
data = [1, 2, 3]
data = [4, 5, 6]
data = [7, 8, 9]
print("Final data:", data)

# Test 4: Nested structures
print("Test 4: Nested structures")
nested = [[1, 2], [3, 4], [5, 6]]
nested = [[7, 8, 9], [10, 11, 12]]
print("Nested list:", nested)

# Test 5: Large allocations
print("Test 5: Large allocations")
i = 0
while i < 50:
    large_list = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    large_dict = {"a": 1, "b": 2, "c": 3, "d": 4, "e": 5}
    i = i + 1
print("Created 50 large objects")

# Test 6: Object replacement in loops
print("Test 6: Object replacement in loops")
j = 0
obj = [0]
while j < 30:
    obj = [j, j + 1, j + 2]
    j = j + 1
print("Final obj:", obj)

# Test 7: Class instances (if classes trigger GC)
print("Test 7: Class instances")
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    
    def get_sum(self):
        return self.x + self.y

k = 0
while k < 20:
    p = Point(k, k + 1)
    sum_val = p.get_sum()
    k = k + 1
print("Created and destroyed 20 Point instances")

# Test 8: Mixed allocations
print("Test 8: Mixed allocations")
counter = 0
while counter < 40:
    if counter > 20:
        temp = {"index": counter, "value": counter * 2}
    else:
        temp = [counter, counter * 2, counter * 3]
    counter = counter + 1
print("Mixed allocations complete")

# Test 9: String concatenation (creates temporary strings)
print("Test 9: String concatenation")
base = "Start"
n = 0
while n < 15:
    base = base + " "
    n = n + 1
print("String length after concatenation:", len(base))

# Test 10: Persistent vs temporary objects
print("Test 10: Persistent vs temporary")
persistent = [1, 2, 3]
m = 0
while m < 25:
    temporary = [m, m + 1, m + 2, m + 3]
    m = m + 1
print("Persistent still exists:", persistent)

print("=== GC Tests Complete ===")
print("If this prints, GC is working correctly!")
