# 🏦 Smart ATM System – Functions Documentation

## 1. Project Overview

Smart ATM System is a console-based ATM management application developed using the C programming language.

It simulates essential ATM operations, including account creation, authentication, deposits, withdrawals, money transfers, and transaction history management.

The application also provides an administrator dashboard for account management and system monitoring.

## 2. Data Structures

### `Transaction`
Stores information about each financial operation:
- `type`: Type of transaction.
- `amount`: Transaction amount.
- `balanceAfter`: Account balance after the operation.

### `Account`
Stores customer account information:
- `accountNumber`: Unique account identifier.
- `name`: Account holder's name.
- `pin`: Account authentication PIN.
- `balance`: Current account balance.
- `history`: Array of transaction records.
- `transactionCount`: Number of stored transactions.

### `FileHeader`
Stores metadata about the account data file:
- `magic`: File format identifier.
- `version`: File format version.
- `accountCount`: Number of saved accounts.

## 3. Input Validation Functions

| Function | Description |
|---|---|
| `readInt()` | Reads and validates an integer within a specified range. |
| `readDouble()` | Reads and validates a numeric amount. |
| `readName()` | Reads and validates a non-empty text input. |

## 4. Account Management Functions

| Function | Description |
|---|---|
| `generateAccountNumber()` | Generates a new account number. |
| `findAccount()` | Searches for an account using its account number. |
| `createAccount()` | Creates a new customer account. |
| `login()` | Authenticates a customer account. |
| `showAccountInfo()` | Displays customer account information. |

## 5. Banking Operations

| Function | Description |
|---|---|
| `checkBalance()` | Displays the current account balance. |
| `deposit()` | Adds money to an account. |
| `withdraw()` | Withdraws money if sufficient funds are available. |
| `transfer()` | Transfers money between two accounts. |

## 6. Transaction History

| Function | Description |
|---|---|
| `addTransaction()` | Records a transaction and its resulting balance. |
| `showHistory()` | Displays a customer's transaction history. |
| `showAllTransactions()` | Displays transaction records from all accounts. |

Each account can store up to 100 transactions.

## 7. Administrator Functions

| Function | Description |
|---|---|
| `ownerLogin()` | Authenticates the administrator using credentials stored in `admin.txt`. |
| `ownerDashboard()` | Displays the administrator dashboard. |
| `showAllAccounts()` | Displays all registered accounts. |
| `showTotalMoney()` | Calculates the total balance of all accounts. |
| `deleteAccount()` | Deletes a selected account after confirmation. |
| `resetAccountPIN()` | Resets a customer's account PIN. |
| `systemSettings()` | Displays basic system configuration information. |

## 8. File Handling Functions

| Function | Description |
|---|---|
| `saveAccounts()` | Saves account information to `accounts.dat`. |
| `loadAccounts()` | Loads and validates saved account data at startup. |

### Local Configuration Files

- `admin.txt`: Contains the administrator username and PIN in a local configuration file.
- `accounts.dat`: Stores customer account information and transaction history.

The project uses binary file handling with `fwrite()` and `fread()` to preserve account information between program executions.

Sensitive local files are excluded from version control through `.gitignore`.

## 9. Menu Navigation

- `mainMenu()` – Displays the main application menu.
- `userDashboard()` – Provides access to customer operations.
- `ownerDashboard()` – Provides access to administrator features.
- `main()` – Initializes the application, loads saved data, and starts the main menu.

## 10. Technologies and Libraries

- **Programming Language:** C
- **Compiler:** GCC
- **Standard Libraries:** `stdio.h`, `stdlib.h`, `string.h`, `stdbool.h`, `errno.h`, `math.h`
- **Data Storage:** Binary files
- **Interface:** Console / Terminal
- **Version Control:** Git and GitHub

## 11. Educational Objectives

This project demonstrates practical applications of:

- Functions and modular programming.
- Structures and arrays.
- Pointers and memory management.
- Input validation.
- File handling and data persistence.
- Searching and data management.
- Conditions and loops.

## 12. Security Notice

This application is an educational ATM simulation and is not intended for real banking operations.

Local credential storage is not encrypted and does not provide production-level security.

## 13. Author

**Taha Ammouri**

GitHub: [@tahaammouri7-glitch](https://github.com/tahaammouri7-glitch)

---

*Developed for educational purposes as part of a C programming learning journey.*