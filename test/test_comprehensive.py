# Comprehensive test combining multiple features
print("=== Comprehensive Test ===")

# Define a class with multiple methods
class BankAccount:
    def __init__(self, name, balance):
        self.name = name
        self.balance = balance
    
    def deposit(self, amount):
        self.balance = self.balance + amount
        print("Deposited:", amount)
        return self.balance
    
    def withdraw(self, amount):
        if amount > self.balance:
            print("Insufficient funds")
            return self.balance
        else:
            self.balance = self.balance - amount
            print("Withdrew:", amount)
            return self.balance
    
    def get_balance(self):
        return self.balance

# Create account
account = BankAccount("Alice", 1000)
print("Account holder:", account.name)
print("Initial balance:", account.get_balance())

# Perform transactions
account.deposit(500)
print("Balance after deposit:", account.get_balance())

account.withdraw(200)
print("Balance after withdrawal:", account.get_balance())

# Try to overdraw
account.withdraw(2000)
print("Balance after failed withdrawal:", account.get_balance())

# Use loops with objects
accounts = [
    BankAccount("Bob", 500),
    BankAccount("Charlie", 750),
    BankAccount("Diana", 1200)
]

print("All account balances:")
for acc in accounts:
    print(acc.name, ":", acc.get_balance())

# Calculate total
total = 0
for acc in accounts:
    total = total + acc.get_balance()

print("Total across all accounts:", total)
