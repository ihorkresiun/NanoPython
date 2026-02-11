# Test variable scope
print("=== Scope Tests ===")

# Global variables
global_var = 100
print("Global var:", global_var)

def test_function():
    local_var = 50
    print("Local var:", local_var)
    print("Global var in function:", global_var)

test_function()
print("Global var after function:", global_var)

# Function parameters
def modify_param(x):
    x = x + 10
    print("Inside function, x =", x)
    return x

value = 5
result = modify_param(value)
print("After function call, value =", value)
print("Returned result =", result)

# Nested functions
def outer():
    outer_var = "outer"
    print("In outer:", outer_var)
    
    def inner():
        inner_var = "inner"
        print("In inner:", inner_var)
        print("Accessing outer var:", outer_var)
    
    inner()

outer()
