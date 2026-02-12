# Stress test for garbage collection
print("=== GC Stress Test ===")

# Stress 1: Deep nesting
print("Stress 1: Deep nested structures")
def create_deep_list(depth):
    if depth == 0:
        return 1
    return [create_deep_list(depth - 1)]

i = 0
while i < 10:
    deep = create_deep_list(5)
    i = i + 1
print("Created 10 deep nested structures")

# Stress 2: Many small allocations
print("Stress 2: Many small allocations")
count = 0
while count < 200:
    tiny = [count]
    count = count + 1
print("Created 200 small allocations")

# Stress 3: Alternating large and small
print("Stress 3: Alternating allocation sizes")
j = 0
while j < 50:
    if j > 25:
        large = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
    else:
        small = [j]
    j = j + 1
print("Alternating allocations complete")

# Stress 4: Rapid object creation in function
print("Stress 4: Rapid function allocations")
def allocate_many():
    k = 0
    while k < 30:
        local = [k, k * 2, k * 3, k * 4]
        k = k + 1
    return k

iterations = 0
while iterations < 10:
    result = allocate_many()
    iterations = iterations + 1
print("Completed 10 iterations of rapid allocations")

# Stress 5: Class instance churn
print("Stress 5: Class instance churn")
class Data:
    def __init__(self, value):
        self.value = value
        self.data = [value, value * 2, value * 3]
    
    def get_value(self):
        return self.value

n = 0
last_value = 0
while n < 50:
    obj = Data(n)
    last_value = obj.get_value()
    n = n + 1
print("Created and churned 50 instances, last value:", last_value)

# Stress 6: Dictionary churn
print("Stress 6: Dictionary churn")
m = 0
while m < 60:
    temp_dict = {"a": m, "b": m + 1, "c": m + 2, "d": m + 3}
    m = m + 1
print("Created 60 temporary dictionaries")

# Stress 7: Mixed collection types
print("Stress 7: Mixed collection types")
p = 0
while p < 40:
    list_obj = [p, p + 1]
    dict_obj = {"x": p}
    tuple_obj = (p, p * 2)
    p = p + 1
print("Created 40 mixed collections")

# Stress 8: Nested function allocations
print("Stress 8: Nested function allocations")
def outer():
    def inner():
        return [1, 2, 3, 4, 5]
    q = 0
    while q < 20:
        temp = inner()
        q = q + 1
    return q

r = 0
while r < 5:
    result = outer()
    r = r + 1
print("Nested function allocations complete")

print("=== GC Stress Test Complete ===")
print("Memory management is working!")
