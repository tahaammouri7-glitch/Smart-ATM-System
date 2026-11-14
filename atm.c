

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

#define MAX_ACCOUNTS 100
#define MAX_TRANSACTIONS 100
#define NAME_SIZE 50
#define FILE_NAME "accounts.dat"
#define FILE_VERSION 1
#define OWNER_USERNAME "taha"
#define OWNER_PIN 2468

/* ================= STRUCTURES ================= */

typedef struct
{
    char type[30];
    double amount;
    double balanceAfter;
} Transaction;

typedef struct
{
    int accountNumber;
    char name[NAME_SIZE];
    int pin;
    double balance;

    Transaction history[MAX_TRANSACTIONS];
    int transactionCount;

} Account;

typedef struct
{
    char magic[8];
    unsigned int version;
    int accountCount;
} FileHeader;

/* ================= FUNCTION PROTOTYPES ================= */

int readInt(const char *message, int min, int max);
double readDouble(const char *message);
void readName(const char *message, char name[], int size);

void mainMenu(Account accounts[], int *count);
void userDashboard(Account accounts[], int count, int userIndex);

int ownerLogin(void);

void ownerDashboard(Account accounts[], int *count);

void showAllAccounts(Account accounts[], int count);
void showTotalMoney(Account accounts[], int count);
void deleteAccount(Account accounts[], int *count);
void showAllTransactions(Account accounts[], int count);
void systemSettings(void);

void createAccount(Account accounts[], int *count);
int login(Account accounts[], int count);

void checkBalance(Account accounts[], int userIndex);
void deposit(Account accounts[], int userIndex);
void withdraw(Account accounts[], int userIndex);
void transfer(Account accounts[], int count, int userIndex);

void showHistory(Account accounts[], int userIndex);
void showAccountInfo(Account accounts[], int userIndex);

int findAccount(Account accounts[], int count, int accountNumber);
int generateAccountNumber(Account accounts[], int count);

void addTransaction(Account accounts[], int userIndex,
                    const char type[], double amount);

bool saveAccounts(Account accounts[], int count);
void loadAccounts(Account accounts[], int *count);

/* ================= INPUT FUNCTIONS ================= */

int readInt(const char *message, int min, int max)
{
    char input[100];
    char *end;
    long value;

    while (true)
    {
        printf("%s", message);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\nErreur de lecture.\n");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        value = strtol(input, &end, 10);

        while (*end == ' ' || *end == '\t' ||
               *end == '\n' || *end == '\r')
        {
            end++;
        }

        if (end == input || *end != '\0' ||
            errno == ERANGE || value < min || value > max)
        {
            printf("Valeur invalide. Reessayez.\n");
            continue;
        }

        return (int)value;
    }
}

double readDouble(const char *message)
{
    char input[100];
    char *end;
    double value;

    while (true)
    {
        printf("%s", message);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\nErreur de lecture.\n");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        value = strtod(input, &end);

        while (*end == ' ' || *end == '\t' ||
               *end == '\n' || *end == '\r')
        {
            end++;
        }

        if (end == input || *end != '\0' ||
            errno == ERANGE || !isfinite(value) ||
            value < 0 || value > 1000000000000.0)
        {
            printf("Montant invalide. Reessayez.\n");
            continue;
        }

        return value;
    }
}

void readName(const char *message, char name[], int size)
{
    while (true)
    {
        printf("%s", message);

        if (fgets(name, size, stdin) == NULL)
        {
            printf("\nErreur de lecture.\n");
            exit(EXIT_FAILURE);
        }

        size_t length = strlen(name);

        if (length > 0 && name[length - 1] == '\n')
        {
            name[length - 1] = '\0';
        }
        else
        {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
            {
                /* Vider le reste de la ligne */
            }
        }

        if (strlen(name) == 0)
        {
            printf("Le nom ne peut pas etre vide.\n");
            continue;
        }

        if (strspn(name, " \t") == strlen(name))
        {
            printf("Veuillez entrer un nom valide.\n");
            continue;
        }

        return;
    }
}

/* ================= ACCOUNT NUMBER ================= */

int generateAccountNumber(Account accounts[], int count)
{
    int maxNumber = 1000;

    for (int i = 0; i < count; i++)
    {
        if (accounts[i].accountNumber > maxNumber)
        {
            maxNumber = accounts[i].accountNumber;
        }
    }

    return maxNumber + 1;
}

/* ================= FIND ACCOUNT ================= */

int findAccount(Account accounts[], int count, int accountNumber)
{
    for (int i = 0; i < count; i++)
    {
        if (accounts[i].accountNumber == accountNumber)
        {
            return i;
        }
    }

    return -1;
}

/* ================= TRANSACTION HISTORY ================= */

void addTransaction(Account accounts[], int userIndex,
                    const char type[], double amount)
{
    Account *account = &accounts[userIndex];

    /* Garder les 100 transactions les plus recentes */

    if (account->transactionCount >= MAX_TRANSACTIONS)
    {
        memmove(
            &account->history[0],
            &account->history[1],
            sizeof(Transaction) * (MAX_TRANSACTIONS - 1));

        account->transactionCount = MAX_TRANSACTIONS - 1;
    }

    Transaction *transaction =
        &account->history[account->transactionCount];

    snprintf(transaction->type, sizeof(transaction->type),
             "%s", type);

    transaction->amount = amount;
    transaction->balanceAfter = account->balance;

    account->transactionCount++;
}

/* ================= SAVE ACCOUNTS ================= */

bool saveAccounts(Account accounts[], int count)
{
    FILE *file = fopen(FILE_NAME, "wb");

    if (file == NULL)
    {
        printf("Erreur : impossible de sauvegarder les donnees.\n");
        return false;
    }

    FileHeader header = {0};

    snprintf(header.magic, sizeof(header.magic), "ATMFILE");
    header.version = FILE_VERSION;
    header.accountCount = count;

    bool success = true;

    if (fwrite(&header, sizeof(header), 1, file) != 1)
    {
        success = false;
    }

    if (success && count > 0)
    {
        if (fwrite(accounts, sizeof(Account), count, file) != (size_t)count)
        {
            success = false;
        }
    }

    if (fclose(file) != 0)
    {
        success = false;
    }

    if (!success)
    {
        printf("Erreur pendant la sauvegarde.\n");
        return false;
    }

    return true;
}

/* ================= LOAD ACCOUNTS ================= */

void loadAccounts(Account accounts[], int *count)
{
    FILE *file = fopen(FILE_NAME, "rb");

    if (file == NULL)
    {
        *count = 0;
        printf("Aucune sauvegarde precedente trouvee.\n");
        return;
    }

    FileHeader header;

    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        printf("Fichier de sauvegarde invalide.\n");
        fclose(file);
        *count = 0;
        return;
    }

    if (strcmp(header.magic, "ATMFILE") != 0 ||
        header.version != FILE_VERSION ||
        header.accountCount < 0 ||
        header.accountCount > MAX_ACCOUNTS)
    {
        printf("Format de sauvegarde non valide.\n");
        fclose(file);
        *count = 0;
        return;
    }

    int loaded = header.accountCount;

    if (loaded > 0)
    {
        if (fread(accounts, sizeof(Account), loaded, file) != (size_t)loaded)
        {
            printf("Donnees incompletes dans le fichier.\n");
            fclose(file);
            *count = 0;
            return;
        }
    }

    fclose(file);

    /* Verification des donnees chargees */

    for (int i = 0; i < loaded; i++)
    {
        if (accounts[i].accountNumber <= 0 ||
            accounts[i].pin < 1000 ||
            accounts[i].pin > 9999 ||
            !isfinite(accounts[i].balance) ||
            accounts[i].balance < 0 ||
            accounts[i].transactionCount < 0 ||
            accounts[i].transactionCount > MAX_TRANSACTIONS ||
            accounts[i].name[NAME_SIZE - 1] != '\0')
        {
            printf("Donnees de compte invalides.\n");
            *count = 0;
            return;
        }

        for (int j = 0; j < accounts[i].transactionCount; j++)
        {
            if (accounts[i].history[j].type[29] != '\0' ||
                !isfinite(accounts[i].history[j].amount) ||
                !isfinite(accounts[i].history[j].balanceAfter))
            {
                printf("Historique invalide.\n");
                *count = 0;
                return;
            }
        }
    }

    *count = loaded;

    printf("%d compte(s) charge(s) avec succes.\n", *count);
}

/* ================= CREATE ACCOUNT ================= */

void createAccount(Account accounts[], int *count)
{
    if (*count >= MAX_ACCOUNTS)
    {
        printf("\nNombre maximum de comptes atteint.\n");
        return;
    }

    printf("\n========== CREATION DE COMPTE ==========\n");

    Account newAccount = {0};

    newAccount.accountNumber =
        generateAccountNumber(accounts, *count);

    readName("Nom du titulaire : ",
             newAccount.name, NAME_SIZE);

    newAccount.pin = readInt("Choisissez un PIN (4 chiffres) : ",
                             1000, 9999);

    newAccount.balance = readDouble("Depot initial (0 si aucun) : ");

    newAccount.transactionCount = 0;

    if (newAccount.balance > 0)
    {
        addTransaction(
            (Account[]){newAccount},
            0,
            "Depot initial",
            newAccount.balance);
    }

    accounts[*count] = newAccount;
    (*count)++;

    printf("\nCompte cree avec succes !\n");
    printf("Numero de compte : %d\n",
           newAccount.accountNumber);

    if (!saveAccounts(accounts, *count))
    {
        printf("Attention : le compte existe en memoire, "
               "mais sa sauvegarde a echoue.\n");
    }
}

/* ================= LOGIN ================= */

int login(Account accounts[], int count)
{
    if (count == 0)
    {
        printf("\nAucun compte disponible.\n");
        return -1;
    }

    printf("\n========== CONNEXION ==========\n");

    int accountNumber =
        readInt("Numero de compte : ", 1, 1000000000);

    int index = findAccount(accounts, count, accountNumber);

    if (index == -1)
    {
        printf("Compte introuvable.\n");
        return -1;
    }

    int pin = readInt("Code PIN : ", 1000, 9999);

    if (accounts[index].pin != pin)
    {
        printf("Code PIN incorrect.\n");
        return -1;
    }

    printf("\nBienvenue, %s !\n", accounts[index].name);

    return index;
}

/* ================= OWNER LOGIN ================= */

int ownerLogin(void)
{
    char username[NAME_SIZE];
    int pin;

    printf("\n========== OWNER LOGIN ==========\n");

    readName("Nom d'utilisateur : ", username, NAME_SIZE);
    pin = readInt("Code PIN : ", 1000, 9999);

    if (strcmp(username, OWNER_USERNAME) == 0 &&
        pin == OWNER_PIN)
    {
        printf("\nBienvenue, Administrateur !\n");
        return 1;
    }

    printf("\nIdentifiants incorrects.\n");
    return 0;
}

/* ================= CHECK BALANCE ================= */

void checkBalance(Account accounts[], int userIndex)
{
    printf("\n========== SOLDE ==========\n");

    printf("Solde actuel : %.2f DH\n",
           accounts[userIndex].balance);
}

/* ================= DEPOSIT ================= */

void deposit(Account accounts[], int userIndex)
{
    printf("\n========== DEPOT ==========\n");

    double amount = readDouble("Montant a deposer : ");

    if (amount <= 0)
    {
        printf("Le montant doit etre superieur a zero.\n");
        return;
    }

    if (amount > 1000000000000.0 -
                     accounts[userIndex].balance)
    {
        printf("Montant trop eleve.\n");
        return;
    }

    accounts[userIndex].balance += amount;

    addTransaction(accounts, userIndex, "Depot", amount);

    printf("Depot effectue avec succes !\n");
    printf("Nouveau solde : %.2f DH\n",
           accounts[userIndex].balance);
}

/* ================= WITHDRAW ================= */

void withdraw(Account accounts[], int userIndex)
{
    printf("\n========== RETRAIT ==========\n");

    double amount = readDouble("Montant a retirer : ");

    if (amount <= 0)
    {
        printf("Le montant doit etre superieur a zero.\n");
        return;
    }

    if (amount > accounts[userIndex].balance)
    {
        printf("Solde insuffisant.\n");
        return;
    }

    accounts[userIndex].balance -= amount;

    addTransaction(accounts, userIndex, "Retrait", amount);

    printf("Retrait effectue avec succes !\n");
    printf("Nouveau solde : %.2f DH\n",
           accounts[userIndex].balance);
}

/* ================= TRANSFER ================= */

void transfer(Account accounts[], int count, int userIndex)
{
    printf("\n========== TRANSFERT ==========\n");

    int destinationNumber =
        readInt("Numero du compte destinataire : ",
                1, 1000000000);

    int destinationIndex =
        findAccount(accounts, count, destinationNumber);

    if (destinationIndex == -1)
    {
        printf("Compte destinataire introuvable.\n");
        return;
    }

    if (destinationIndex == userIndex)
    {
        printf("Vous ne pouvez pas transferer vers votre propre compte.\n");
        return;
    }

    double amount = readDouble("Montant a transferer : ");

    if (amount <= 0)
    {
        printf("Le montant doit etre superieur a zero.\n");
        return;
    }

    if (amount > accounts[userIndex].balance)
    {
        printf("Solde insuffisant.\n");
        return;
    }

    if (amount > 1000000000000.0 -
                     accounts[destinationIndex].balance)
    {
        printf("Le solde du destinataire ne peut pas recevoir "
               "ce montant.\n");
        return;
    }

    accounts[userIndex].balance -= amount;
    accounts[destinationIndex].balance += amount;

    addTransaction(accounts, userIndex,
                   "Transfert envoye", amount);

    addTransaction(accounts, destinationIndex,
                   "Transfert recu", amount);

    printf("\nTransfert effectue avec succes !\n");
    printf("Destinataire : %s\n",
           accounts[destinationIndex].name);
    printf("Montant : %.2f DH\n", amount);
    printf("Votre nouveau solde : %.2f DH\n",
           accounts[userIndex].balance);
}

/* ================= TRANSACTION HISTORY ================= */

void showHistory(Account accounts[], int userIndex)
{
    Account *account = &accounts[userIndex];

    printf("\n========== HISTORIQUE ==========\n");

    if (account->transactionCount == 0)
    {
        printf("Aucune transaction disponible.\n");
        return;
    }

    printf("%-5s %-22s %-15s %-15s\n",
           "N", "Type", "Montant", "Solde");

    printf("------------------------------------------------------\n");

    for (int i = 0; i < account->transactionCount; i++)
    {
        Transaction transaction = account->history[i];

        printf("%-5d %-22s %-15.2f %-15.2f\n",
               i + 1,
               transaction.type,
               transaction.amount,
               transaction.balanceAfter);
    }
}

/* ================= ACCOUNT INFORMATION ================= */

void showAccountInfo(Account accounts[], int userIndex)
{
    Account account = accounts[userIndex];

    printf("\n========== INFORMATIONS DU COMPTE ==========\n");

    printf("Nom du titulaire : %s\n", account.name);
    printf("Numero de compte : %d\n", account.accountNumber);
    printf("Solde actuel     : %.2f DH\n", account.balance);
    printf("Nombre de transactions : %d\n",
           account.transactionCount);
}

/* ================= OWNER FUNCTIONS ================= */

void showAllAccounts(Account accounts[], int count)
{
    printf("\n========== TOUS LES COMPTES ==========\n");

    if (count == 0)
    {
        printf("Aucun compte disponible.\n");
        return;
    }

    printf("%-12s %-25s %-15s %-15s\n",
           "Numero", "Titulaire", "Solde", "       Pin");

    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++)
    {
        printf("%-12d %-25s %-15.2fDH      %-15d\n",
               accounts[i].accountNumber,
               accounts[i].name,
               accounts[i].balance,
               accounts[i].pin);
    }
}

void showTotalMoney(Account accounts[], int count)
{
    double total = 0;

    for (int i = 0; i < count; i++)
    {
        total += accounts[i].balance;
    }

    printf("\n========== TOTAL DES FONDS ==========\n");
    printf("Nombre de comptes : %d\n", count);
    printf("Total des soldes : %.2f DH\n", total);
}

void deleteAccount(Account accounts[], int *count)
{
    if (*count == 0)
    {
        printf("\nAucun compte a supprimer.\n");
        return;
    }

    showAllAccounts(accounts, *count);

    int number = readInt(
        "\nNumero du compte a supprimer : ",
        1, 1000000000);

    int index = findAccount(accounts, *count, number);

    if (index == -1)
    {
        printf("Compte introuvable.\n");
        return;
    }

    printf("\nTitulaire : %s\n", accounts[index].name);

    int confirm = readInt(
        "Confirmer la suppression ? (1=Oui / 0=Non) : ",
        0, 1);

    if (confirm == 0)
    {
        printf("Suppression annulee.\n");
        return;
    }

    for (int i = index; i < *count - 1; i++)
    {
        accounts[i] = accounts[i + 1];
    }

    (*count)--;

    if (saveAccounts(accounts, *count))
    {
        printf("Compte supprime avec succes.\n");
    }
    else
    {
        printf("Erreur lors de la sauvegarde.\n");
    }
}

void showAllTransactions(Account accounts[], int count)
{
    printf("\n========== TOUTES LES TRANSACTIONS ==========\n");

    int found = 0;

    for (int i = 0; i < count; i++)
    {
        Account *account = &accounts[i];

        for (int j = 0; j < account->transactionCount; j++)
        {
            Transaction *t = &account->history[j];

            printf("\nCompte : %d\n",
                   account->accountNumber);

            printf("Titulaire : %s\n", account->name);

            printf("Operation : %s\n", t->type);

            printf("Montant : %.2f DH\n", t->amount);

            printf("Solde apres operation : %.2f DH\n",
                   t->balanceAfter);

            printf("------------------------------------\n");

            found = 1;
        }
    }

    if (!found)
    {
        printf("Aucune transaction disponible.\n");
    }
}

void systemSettings(void)
{
    printf("\n========== PARAMETRES DU SYSTEME ==========\n");

    printf("Nom du systeme : SMART ATM SYSTEM\n");
    printf("Version : 1.0\n");
    printf("Nombre maximum de comptes : %d\n", MAX_ACCOUNTS);
    printf("Historique maximum par compte : %d\n",
           MAX_TRANSACTIONS);
    printf("Fichier de sauvegarde : %s\n", FILE_NAME);

    printf("\nAppuyez sur Entree pour continuer...");
    getchar();
}

/* ================= OWNER DASHBOARD ================= */

void ownerDashboard(Account accounts[], int *count)
{
    int choice;

    do
    {
        printf("\n====================================\n");
        printf("          OWNER DASHBOARD           \n");
        printf("====================================\n");

        printf("1. View All Accounts\n");
        printf("2. View Total Money\n");
        printf("3. Delete Account\n");
        printf("4. View All Transactions\n");
        printf("5. System Settings\n");
        printf("0. Logout\n");

        printf("====================================\n");

        choice = readInt("Votre choix : ", 0, 5);

        switch (choice)
        {
        case 1:
            showAllAccounts(accounts, *count);
            break;

        case 2:
            showTotalMoney(accounts, *count);
            break;

        case 3:
            deleteAccount(accounts, count);
            break;

        case 4:
            showAllTransactions(accounts, *count);
            break;

        case 5:
            systemSettings();
            break;

        case 0:
            printf("Deconnexion Administrateur reussie.\n");
            break;
        }

    } while (choice != 0);
}

/* ================= USER DASHBOARD ================= */

void userDashboard(Account accounts[], int count, int userIndex)
{
    int choice;

    do
    {
        printf("\n====================================\n");
        printf("          SMART ATM SYSTEM          \n");
        printf("====================================\n");

        printf("1. Consulter le solde\n");
        printf("2. Deposer de l'argent\n");
        printf("3. Retirer de l'argent\n");
        printf("4. Transferer de l'argent\n");
        printf("5. Historique des transactions\n");
        printf("6. Informations du compte\n");
        printf("0. Deconnexion\n");

        printf("====================================\n");

        choice = readInt("Votre choix : ", 0, 6);

        switch (choice)
        {
        case 1:
            checkBalance(accounts, userIndex);
            break;

        case 2:
            deposit(accounts, userIndex);
            saveAccounts(accounts, count);
            break;

        case 3:
            withdraw(accounts, userIndex);
            saveAccounts(accounts, count);
            break;

        case 4:
            transfer(accounts, count, userIndex);
            saveAccounts(accounts, count);
            break;

        case 5:
            showHistory(accounts, userIndex);
            break;

        case 6:
            showAccountInfo(accounts, userIndex);
            break;

        case 0:
            printf("Deconnexion reussie.\n");
            break;
        }

    } while (choice != 0);
}

/* ================= MAIN FUNCTION ================= */

int main(void)
{
    Account accounts[MAX_ACCOUNTS] = {0};
    int count = 0;

    printf("Chargement de SMART ATM SYSTEM...\n");

    loadAccounts(accounts, &count);

    mainMenu(accounts, &count);

    return 0;
}
