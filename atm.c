#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

/* =========================================================
   SMART ATM SYSTEM
   Projet educatif realise en langage C.
   Les donnees sont stockees localement dans accounts.dat.
   ========================================================= */

/* -------------------- CONSTANTES -------------------- */

#define MAX_ACCOUNTS 100
#define MAX_TRANSACTIONS 100
#define NAME_SIZE 50
#define TYPE_SIZE 30
#define FILE_NAME "accounts.dat"
#define TEMP_FILE "accounts.tmp"
#define BACKUP_FILE "accounts.bak"
#define FILE_VERSION 1

/* Identifiants de demonstration : a modifier avant publication. */
#define MAX_BALANCE 1000000000000.0

/* -------------------- STRUCTURES -------------------- */

typedef struct
{
    char type[TYPE_SIZE];
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

/* -------------------- PROTOTYPES -------------------- */

/* Fonctions de saisie */
int readInt(const char *message, int min, int max);
double readDouble(const char *message);
void readName(const char *message, char name[], int size);
void pauseScreen(void);

/* Navigation */
void mainMenu(Account accounts[], int *count);
void userDashboard(Account accounts[], int count, int userIndex);
void ownerDashboard(Account accounts[], int *count);

/* Comptes et authentification */
void createAccount(Account accounts[], int *count);
int login(Account accounts[], int count);
int ownerLogin(void);
int findAccount(Account accounts[], int count, int accountNumber);
int generateAccountNumber(Account accounts[], int count);
void resetAccountPIN(Account accounts[], int count);

/* Operations */
void checkBalance(Account accounts[], int userIndex);
void deposit(Account accounts[], int userIndex);
void withdraw(Account accounts[], int userIndex);
void transfer(Account accounts[], int count, int userIndex);

/* Historique et informations */
void addTransaction(Account accounts[], int userIndex,
                    const char type[], double amount);
void showHistory(Account accounts[], int userIndex);
void showAccountInfo(Account accounts[], int userIndex);

/* Administration */
void showAllAccounts(Account accounts[], int count);
void showTotalMoney(Account accounts[], int count);
void deleteAccount(Account accounts[], int *count);
void showAllTransactions(Account accounts[], int count);
void systemSettings(void);

/* Fichiers */
bool saveAccounts(Account accounts[], int count);
void loadAccounts(Account accounts[], int *count);
bool validateAccount(const Account *account);
bool validateAllAccounts(Account accounts[], int count);

/* =========================================================
   1. FONCTIONS DE SAISIE
   ========================================================= */

/* Lit un entier et verifie sa validite. */
int readInt(const char *message, int min, int max)
{
    char input[128];
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

        /* Si la ligne est trop longue, vider le reste. */
        if (strchr(input, '\n') == NULL && !feof(stdin))
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Saisie trop longue. Reessayez.\n");
            continue;
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
            printf("Valeur invalide. Entrez un nombre entre %d et %d.\n",
                   min, max);
            continue;
        }

        return (int)value;
    }
}

/* Lit un montant positif ou nul et refuse les valeurs non finies. */
double readDouble(const char *message)
{
    char input[128];
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

        if (strchr(input, '\n') == NULL && !feof(stdin))
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Saisie trop longue. Reessayez.\n");
            continue;
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
            value < 0 || value > MAX_BALANCE)
        {
            printf("Montant invalide. Reessayez.\n");
            continue;
        }

        return value;
    }
}

/* Lit un nom non vide et vide le reste de la ligne si necessaire. */
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
        else if (!feof(stdin))
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Nom trop long. Maximum : %d caracteres.\n", size - 1);
            continue;
        }

        if (name[0] == '\0' || strspn(name, " \t") == strlen(name))
        {
            printf("Le nom ne peut pas etre vide.\n");
            continue;
        }

        return;
    }
}

/* Attend Entree avant de revenir au menu. */
void pauseScreen(void)
{
    printf("\nAppuyez sur Entree pour continuer...");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
}

/* =========================================================
   2. GESTION DES COMPTES
   ========================================================= */

/* Recherche un compte par son numero. Retourne son indice ou -1. */
int findAccount(Account accounts[], int count, int accountNumber)
{
    for (int i = 0; i < count; i++)
    {
        if (accounts[i].accountNumber == accountNumber)
            return i;
    }

    return -1;
}

/* Genere un numero superieur au plus grand numero existant. */
int generateAccountNumber(Account accounts[], int count)
{
    int maxNumber = 1000;

    for (int i = 0; i < count; i++)
    {
        if (accounts[i].accountNumber > maxNumber)
            maxNumber = accounts[i].accountNumber;
    }

    return maxNumber + 1;
}

/* Ajoute une operation dans l'historique du compte. */
void addTransaction(Account accounts[], int userIndex,
                    const char type[], double amount)
{
    Account *account = &accounts[userIndex];

    /* Si l'historique est plein, supprimer la transaction la plus ancienne. */
    if (account->transactionCount >= MAX_TRANSACTIONS)
    {
        memmove(&account->history[0],
                &account->history[1],
                sizeof(Transaction) * (MAX_TRANSACTIONS - 1));

        account->transactionCount = MAX_TRANSACTIONS - 1;
    }

    Transaction *transaction =
        &account->history[account->transactionCount];

    memset(transaction, 0, sizeof(*transaction));
    snprintf(transaction->type, sizeof(transaction->type), "%s", type);
    transaction->amount = amount;
    transaction->balanceAfter = account->balance;

    account->transactionCount++;
}

/* Cree un compte et sauvegarde les nouvelles donnees. */
void createAccount(Account accounts[], int *count)
{
    if (*count >= MAX_ACCOUNTS)
    {
        printf("\nNombre maximum de comptes atteint.\n");
        return;
    }

    printf("\n========== CREATION DE COMPTE ==========\n");

    Account newAccount = {0};

    newAccount.accountNumber = generateAccountNumber(accounts, *count);
    readName("Nom du titulaire : ", newAccount.name, NAME_SIZE);

    newAccount.pin = readInt("Choisissez un PIN a 4 chiffres : ", 1000, 9999);

    int confirmPIN = readInt("Confirmez le PIN : ", 1000, 9999);
    if (newAccount.pin != confirmPIN)
    {
        printf("Les PIN ne correspondent pas. Creation annulee.\n");
        return;
    }

    newAccount.balance = readDouble("Depot initial (0 si aucun) : ");
    newAccount.transactionCount = 0;

    if (newAccount.balance > 0)
    {
        snprintf(newAccount.history[0].type,
                 sizeof(newAccount.history[0].type), "%s", "Depot initial");
        newAccount.history[0].amount = newAccount.balance;
        newAccount.history[0].balanceAfter = newAccount.balance;
        newAccount.transactionCount = 1;
    }

    accounts[*count] = newAccount;
    (*count)++;

    if (!saveAccounts(accounts, *count))
    {
        /* Annuler la creation en memoire si la sauvegarde echoue. */
        (*count)--;
        memset(&accounts[*count], 0, sizeof(Account));
        printf("Erreur : le compte n'a pas pu etre sauvegarde.\n");
        return;
    }

    printf("\nCompte cree et sauvegarde avec succes !\n");
    printf("Nom : %s\n", newAccount.name);
    printf("Numero de compte : %d\n", newAccount.accountNumber);
    printf("Solde initial : %.2f DH\n", newAccount.balance);
    printf("Conservez votre numero de compte.\n");
}

/* Connexion d'un utilisateur avec trois tentatives maximum. */
int login(Account accounts[], int count)
{
    if (count == 0)
    {
        printf("\nAucun compte disponible.\n");
        return -1;
    }

    printf("\n========== CONNEXION CLIENT ==========\n");

    int accountNumber = readInt("Numero de compte : ", 1, 1000000000);
    int index = findAccount(accounts, count, accountNumber);

    if (index == -1)
    {
        printf("Compte introuvable.\n");
        return -1;
    }

    for (int attempt = 1; attempt <= 3; attempt++)
    {
        int pin = readInt("Code PIN : ", 1000, 9999);

        if (accounts[index].pin == pin)
        {
            printf("\nBienvenue, %s !\n", accounts[index].name);
            return index;
        }

        printf("PIN incorrect. Tentatives restantes : %d\n", 3 - attempt);
    }

    printf("\nNombre maximum de tentatives atteint. Connexion refusee.\n");
    return -1;
}

/* Connexion de l'administrateur. */

int ownerLogin(void)
{
    char username[NAME_SIZE];
    char savedUsername[NAME_SIZE];
    char savedPin[20];
    char enteredPin[20];

    FILE *file = fopen("admin.txt", "r");

    if (file == NULL)
    {
        printf("\nErreur : fichier admin.txt introuvable.\n");
        return 0;
    }

    if (fgets(savedUsername, NAME_SIZE, file) == NULL ||
        fgets(savedPin, sizeof(savedPin), file) == NULL)
    {
        fclose(file);
        printf("\nErreur : fichier admin.txt invalide.\n");
        return 0;
    }

    fclose(file);

    savedUsername[strcspn(savedUsername, "\r\n")] = '\0';
    savedPin[strcspn(savedPin, "\r\n")] = '\0';

    printf("\n========== CONNEXION ADMINISTRATEUR ==========\n");

    readName("Nom d'utilisateur : ", username, NAME_SIZE);
    readName("Code PIN administrateur : ", enteredPin, sizeof(enteredPin));

    if (strcmp(username, savedUsername) == 0 &&
        strcmp(enteredPin, savedPin) == 0)
    {
        printf("\nBienvenue, Administrateur !\n");
        return 1;
    }

    printf("\nIdentifiants incorrects.\n");
    return 0;
}


/* Permet a l'administrateur de remplacer un PIN oublie. */
void resetAccountPIN(Account accounts[], int count)
{
    if (count == 0)
    {
        printf("\nAucun compte disponible.\n");
        return;
    }

    printf("\n========== REINITIALISATION DU PIN ==========\n");

    int number = readInt("Numero du compte : ", 1, 1000000000);
    int index = findAccount(accounts, count, number);

    if (index == -1)
    {
        printf("Compte introuvable.\n");
        return;
    }

    printf("Titulaire : %s\n", accounts[index].name);

    int confirm = readInt("Confirmer ? (1=Oui / 0=Non) : ", 0, 1);
    if (confirm == 0)
    {
        printf("Operation annulee.\n");
        return;
    }

    int newPIN = readInt("Nouveau PIN : ", 1000, 9999);
    int repeatedPIN = readInt("Confirmer le nouveau PIN : ", 1000, 9999);

    if (newPIN != repeatedPIN)
    {
        printf("Les PIN ne correspondent pas.\n");
        return;
    }

    int oldPIN = accounts[index].pin;
    accounts[index].pin = newPIN;

    if (!saveAccounts(accounts, count))
    {
        accounts[index].pin = oldPIN;
        printf("Erreur de sauvegarde. Ancien PIN conserve en memoire.\n");
        return;
    }

    printf("PIN reinitialise avec succes.\n");
}

/* =========================================================
   3. OPERATIONS BANCAIRES
   ========================================================= */

void checkBalance(Account accounts[], int userIndex)
{
    printf("\n========== CONSULTATION DU SOLDE ==========\n");
    printf("Solde actuel : %.2f DH\n", accounts[userIndex].balance);
}

void deposit(Account accounts[], int userIndex)
{
    printf("\n========== DEPOT ==========\n");

    double amount = readDouble("Montant a deposer : ");

    if (amount <= 0)
    {
        printf("Le montant doit etre superieur a zero.\n");
        return;
    }

    if (amount > MAX_BALANCE - accounts[userIndex].balance)
    {
        printf("Montant trop eleve.\n");
        return;
    }

    accounts[userIndex].balance += amount;
    addTransaction(accounts, userIndex, "Depot", amount);

    printf("Depot effectue. Nouveau solde : %.2f DH\n",
           accounts[userIndex].balance);
}

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

    printf("Retrait effectue. Nouveau solde : %.2f DH\n",
           accounts[userIndex].balance);
}

void transfer(Account accounts[], int count, int userIndex)
{
    printf("\n========== TRANSFERT ==========\n");

    int destinationNumber =
        readInt("Numero du compte destinataire : ", 1, 1000000000);

    int destinationIndex =
        findAccount(accounts, count, destinationNumber);

    if (destinationIndex == -1)
    {
        printf("Compte destinataire introuvable.\n");
        return;
    }

    if (destinationIndex == userIndex)
    {
        printf("Le transfert vers votre propre compte est impossible.\n");
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

    if (amount > MAX_BALANCE - accounts[destinationIndex].balance)
    {
        printf("Le compte destinataire ne peut pas recevoir ce montant.\n");
        return;
    }

    accounts[userIndex].balance -= amount;
    accounts[destinationIndex].balance += amount;

    addTransaction(accounts, userIndex, "Transfert envoye", amount);
    addTransaction(accounts, destinationIndex, "Transfert recu", amount);

    printf("\nTransfert effectue !\n");
    printf("Destinataire : %s\n", accounts[destinationIndex].name);
    printf("Montant : %.2f DH\n", amount);
    printf("Votre nouveau solde : %.2f DH\n",
           accounts[userIndex].balance);
}

/* =========================================================
   4. HISTORIQUE ET INFORMATIONS
   ========================================================= */

void showHistory(Account accounts[], int userIndex)
{
    Account *account = &accounts[userIndex];

    printf("\n========== HISTORIQUE DES TRANSACTIONS ==========\n");

    if (account->transactionCount == 0)
    {
        printf("Aucune transaction disponible.\n");
        return;
    }

    printf("%-5s %-22s %-15s %-15s\n",
           "N", "Type", "Montant", "Solde");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < account->transactionCount; i++)
    {
        Transaction *t = &account->history[i];

        printf("%-5d %-22s %-15.2f %-15.2f\n",
               i + 1, t->type, t->amount, t->balanceAfter);
    }
}

void showAccountInfo(Account accounts[], int userIndex)
{
    Account *account = &accounts[userIndex];

    printf("\n========== INFORMATIONS DU COMPTE ==========\n");
    printf("Titulaire : %s\n", account->name);
    printf("Numero de compte : %d\n", account->accountNumber);
    printf("Solde : %.2f DH\n", account->balance);
    printf("Transactions enregistrees : %d\n",
           account->transactionCount);
}

/* =========================================================
   5. FONCTIONS D'ADMINISTRATION
   ========================================================= */

void showAllAccounts(Account accounts[], int count)
{
    printf("\n========== LISTE DES COMPTES ==========\n");

    if (count == 0)
    {
        printf("Aucun compte disponible.\n");
        return;
    }

    printf("%-12s %-25s %-15s\n", "Numero", "Titulaire", "Solde");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < count; i++)
    {
        /* Le PIN n'est jamais affiche dans la liste. */
        printf("%-12d %-25s %-15.2f DH\n",
               accounts[i].accountNumber,
               accounts[i].name,
               accounts[i].balance);
    }
}

void showTotalMoney(Account accounts[], int count)
{
    double total = 0.0;

    for (int i = 0; i < count; i++)
        total += accounts[i].balance;

    printf("\n========== STATISTIQUES ==========\n");
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

    int number = readInt("\nNumero du compte a supprimer : ",
                         1, 1000000000);
    int index = findAccount(accounts, *count, number);

    if (index == -1)
    {
        printf("Compte introuvable.\n");
        return;
    }

    int confirm = readInt("Confirmer la suppression ? (1=Oui / 0=Non) : ",
                          0, 1);
    if (confirm == 0)
    {
        printf("Suppression annulee.\n");
        return;
    }

    Account backup[MAX_ACCOUNTS];
    int oldCount = *count;
    memcpy(backup, accounts, sizeof(Account) * oldCount);

    for (int i = index; i < *count - 1; i++)
        accounts[i] = accounts[i + 1];

    (*count)--;

    if (!saveAccounts(accounts, *count))
    {
        memcpy(accounts, backup, sizeof(Account) * oldCount);
        *count = oldCount;
        printf("Erreur de sauvegarde. Suppression annulee.\n");
        return;
    }

    printf("Compte supprime avec succes.\n");
}

void showAllTransactions(Account accounts[], int count)
{
    int found = 0;

    printf("\n========== TOUTES LES TRANSACTIONS ==========\n");

    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < accounts[i].transactionCount; j++)
        {
            Transaction *t = &accounts[i].history[j];

            printf("\nCompte : %d | Titulaire : %s\n",
                   accounts[i].accountNumber, accounts[i].name);
            printf("Operation : %s\n", t->type);
            printf("Montant : %.2f DH\n", t->amount);
            printf("Solde apres operation : %.2f DH\n", t->balanceAfter);
            printf("-----------------------------------------------\n");

            found = 1;
        }
    }

    if (!found)
        printf("Aucune transaction disponible.\n");
}

void systemSettings(void)
{
    printf("\n========== PARAMETRES DU SYSTEME ==========\n");
    printf("Nom : SMART ATM SYSTEM\n");
    printf("Version : 2.0\n");
    printf("Langage : C\n");
    printf("Nombre maximum de comptes : %d\n", MAX_ACCOUNTS);
    printf("Historique maximum par compte : %d\n", MAX_TRANSACTIONS);
    printf("Fichier de donnees : %s\n", FILE_NAME);
    printf("\nNote : projet educatif, pas un systeme bancaire reel.\n");
}

/* =========================================================
   6. VALIDATION ET SAUVEGARDE DES DONNEES
   ========================================================= */

/* Verifie la structure et les champs d'un compte. */
bool validateAccount(const Account *account)
{
    if (account->accountNumber <= 0 ||
        account->pin < 1000 || account->pin > 9999 ||
        !isfinite(account->balance) ||
        account->balance < 0 || account->balance > MAX_BALANCE ||
        account->transactionCount < 0 ||
        account->transactionCount > MAX_TRANSACTIONS)
    {
        return false;
    }

    if (memchr(account->name, '\0', NAME_SIZE) == NULL ||
        account->name[0] == '\0' ||
        strspn(account->name, " \t") == strlen(account->name))
    {
        return false;
    }

    for (int i = 0; i < account->transactionCount; i++)
    {
        const Transaction *t = &account->history[i];

        if (memchr(t->type, '\0', TYPE_SIZE) == NULL ||
            t->type[0] == '\0' ||
            !isfinite(t->amount) || t->amount < 0 ||
            !isfinite(t->balanceAfter) ||
            t->balanceAfter < 0 || t->balanceAfter > MAX_BALANCE)
        {
            return false;
        }
    }

    return true;
}

/* Verifie tous les comptes et l'unicite des numeros. */
bool validateAllAccounts(Account accounts[], int count)
{
    if (count < 0 || count > MAX_ACCOUNTS)
        return false;

    for (int i = 0; i < count; i++)
    {
        if (!validateAccount(&accounts[i]))
            return false;

        for (int j = 0; j < i; j++)
        {
            if (accounts[i].accountNumber == accounts[j].accountNumber)
                return false;
        }
    }

    return true;
}

/* Sauvegarde les donnees dans un fichier temporaire avant remplacement. */
bool saveAccounts(Account accounts[], int count)
{
    if (!validateAllAccounts(accounts, count))
    {
        printf("Erreur : donnees invalides, sauvegarde refusee.\n");
        return false;
    }

    FILE *file = fopen(TEMP_FILE, "wb");
    if (file == NULL)
    {
        printf("Erreur : impossible de creer le fichier temporaire.\n");
        return false;
    }

    FileHeader header = {0};
    memcpy(header.magic, "ATMFILE", 7);
    header.version = FILE_VERSION;
    header.accountCount = count;

    bool success = true;

    if (fwrite(&header, sizeof(header), 1, file) != 1)
        success = false;

    if (success && count > 0 &&
        fwrite(accounts, sizeof(Account), count, file) != (size_t)count)
    {
        success = false;
    }

    if (fclose(file) != 0)
        success = false;

    if (!success)
    {
        remove(TEMP_FILE);
        printf("Erreur pendant l'ecriture du fichier temporaire.\n");
        return false;
    }

    /*
       Conserver l'ancienne sauvegarde comme fichier de secours pendant
       le remplacement. Cela limite le risque de perdre les donnees.
    */
    bool hadOldFile = false;
    FILE *existing = fopen(FILE_NAME, "rb");

    if (existing != NULL)
    {
        fclose(existing);
        remove(BACKUP_FILE);

        if (rename(FILE_NAME, BACKUP_FILE) != 0)
        {
            printf("Impossible de proteger l'ancienne sauvegarde.\n");
            remove(TEMP_FILE);
            return false;
        }

        hadOldFile = true;
    }

    if (rename(TEMP_FILE, FILE_NAME) != 0)
    {
        printf("Erreur : impossible de finaliser la sauvegarde.\n");

        if (hadOldFile)
            rename(BACKUP_FILE, FILE_NAME);

        return false;
    }

    if (hadOldFile)
        remove(BACKUP_FILE);

    return true;
}

/* Charge le fichier et verifie son contenu avant utilisation. */
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

    if (memcmp(header.magic, "ATMFILE", 7) != 0 ||
        header.magic[7] != '\0' ||
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

    if (loaded > 0 &&
        fread(accounts, sizeof(Account), loaded, file) != (size_t)loaded)
    {
        printf("Donnees incompletes dans le fichier.\n");
        fclose(file);
        *count = 0;
        return;
    }

    /* Refuser les donnees supplementaires inattendues. */
    int extra = fgetc(file);
    if (extra != EOF || ferror(file))
    {
        printf("Le fichier contient des donnees inattendues.\n");
        fclose(file);
        *count = 0;
        return;
    }

    fclose(file);

    if (!validateAllAccounts(accounts, loaded))
    {
        printf("Les donnees sauvegardees sont invalides ou dupliquees.\n");
        *count = 0;
        return;
    }

    *count = loaded;
    printf("%d compte(s) charge(s) avec succes.\n", *count);
}

/* =========================================================
   7. TABLEAUX DE BORD
   ========================================================= */

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

        if (choice >= 2 && choice <= 4)
        {
            /* Copie de securite pour restaurer la memoire si l'enregistrement echoue. */
            Account backup[MAX_ACCOUNTS];
            memcpy(backup, accounts, sizeof(Account) * count);

            if (choice == 2)
                deposit(accounts, userIndex);
            else if (choice == 3)
                withdraw(accounts, userIndex);
            else
                transfer(accounts, count, userIndex);

            if (!saveAccounts(accounts, count))
            {
                memcpy(accounts, backup, sizeof(Account) * count);
                printf("Sauvegarde impossible : les modifications ont ete annulees en memoire.\n");
            }
            else
            {
                printf("Donnees sauvegardees avec succes.\n");
            }
        }
        else
        {
            switch (choice)
            {
            case 1:
                checkBalance(accounts, userIndex);
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
        }

    } while (choice != 0);
}

void ownerDashboard(Account accounts[], int *count)
{
    int choice;

    do
    {
        printf("\n====================================\n");
        printf("          OWNER DASHBOARD           \n");
        printf("====================================\n");
        printf("1. Voir tous les comptes\n");
        printf("2. Voir le total des soldes\n");
        printf("3. Supprimer un compte\n");
        printf("4. Voir toutes les transactions\n");
        printf("5. Parametres du systeme\n");
        printf("6. Reinitialiser un PIN\n");
        printf("0. Deconnexion\n");
        printf("====================================\n");

        choice = readInt("Votre choix : ", 0, 6);

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
        case 6:
            resetAccountPIN(accounts, *count);
            break;
        case 0:
            printf("Deconnexion administrateur reussie.\n");
            break;
        }

    } while (choice != 0);
}

/* Menu principal : point d'entree vers les espaces client et admin. */
void mainMenu(Account accounts[], int *count)
{
    int choice;

    do
    {
        printf("\n====================================\n");
        printf("          SMART ATM SYSTEM          \n");
        printf("====================================\n");
        printf("1. Creer un compte\n");
        printf("2. Se connecter\n");
        printf("3. Connexion administrateur\n");
        printf("0. Quitter\n");
        printf("====================================\n");

        choice = readInt("Votre choix : ", 0, 3);

        switch (choice)
        {
        case 1:
            createAccount(accounts, count);
            break;

        case 2:
        {
            int userIndex = login(accounts, *count);
            if (userIndex != -1)
                userDashboard(accounts, *count, userIndex);
            break;
        }

        case 3:
            if (ownerLogin())
                ownerDashboard(accounts, count);
            break;

        case 0:
            printf("\nMerci d'avoir utilise SMART ATM SYSTEM !\n");
            printf("Au revoir.\n");
            break;
        }

    } while (choice != 0);
}

/* =========================================================
   8. FONCTION PRINCIPALE
   ========================================================= */

int main(void)
{
    Account accounts[MAX_ACCOUNTS] = {0};
    int count = 0;

    printf("Chargement de SMART ATM SYSTEM...\n");
    loadAccounts(accounts, &count);

    mainMenu(accounts, &count);

    return 0;
}
