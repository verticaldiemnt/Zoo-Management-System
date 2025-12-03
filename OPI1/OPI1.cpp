#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip> // Для setw, setprecision, fixed, left

using namespace std;

// --- Структури ---

/**
 * @struct User
 * @brief Зберігає інформацію про одного користувача системи.
 */
struct User {
    string username; ///< Логін користувача
    string password; ///< Пароль користувача
    string role;     ///< Роль (наприклад, "administrator", "inspector")
};

/**
 * @struct Animal
 * @brief Зберігає інформацію про одну тварину в зоопарку.
 */
struct Animal {
    int id = 0;          ///< Унікальний ідентифікатор тварини
    string name;         ///< Ім'я (кличка) тварини
    string species;      ///< Вид тварини (наприклад, "Lion")
    int age = 0;         ///< Вік тварини в роках
    string healthStatus; ///< Поточний стан здоров'я (наприклад, "Healthy")
};

/**
 * @struct SpeciesReport
 * @brief Допоміжна структура для агрегації даних при генерації звіту.
 */
struct SpeciesReport {
    string species;  ///< Назва виду
    int count = 0;   ///< Кількість тварин цього виду
    int totalAge = 0; ///< Сумарний вік тварин цього виду
};

// --- Константи ---
const string USERS_FILE = "users.txt";     ///< Файл для зберігання даних користувачів
const string ANIMALS_FILE = "animals.txt"; ///< Файл для зберігання даних тварин
const string REPORT_FILE = "report.txt";   ///< Файл для збереження згенерованого звіту

const int MAX_USERS = 100;   ///< Максимальна кількість користувачів у системі
const int MAX_ANIMALS = 500; ///< Максимальна кількість тварин у системі

// --- Допоміжні функції (Trim) ---

static inline string ltrim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    return (start == string::npos) ? "" : s.substr(start);
}

static inline string rtrim(const string& s) {
    size_t end = s.find_last_not_of(" \t\r\n");
    return (end == string::npos) ? "" : s.substr(0, end + 1);
}

static inline string trim(const string& s) {
    return rtrim(ltrim(s));
}

// =================================================================================
// МОДУЛЬ 1: АВТОРИЗАЦІЯ КОРИСТУВАЧА
// =================================================================================

void loadUsersFromFile(User users[], int& count) {
    count = 0;
    ifstream fin(USERS_FILE);
    if (!fin.is_open()) return;

    string line;
    while (getline(fin, line)) {
        if (count >= MAX_USERS) {
            cerr << "Warning: Max user limit reached. Some users not loaded.\n";
            break;
        }
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        string part;
        string parts[3];
        int partCount = 0;

        while (partCount < 3 && getline(iss, part, ':')) {
            parts[partCount++] = trim(part);
        }

        if (partCount == 3) {
            users[count].username = parts[0];
            users[count].password = parts[1];
            users[count].role = parts[2];
            count++;
        }
    }
    fin.close();
}

void saveUsersToFile(const string& path, const User users[], int count) {
    ofstream fout(path);
    if (!fout.is_open()) {
        cerr << "Error: cannot save user file.\n";
        return;
    }
    fout << "# Format: username:password:role\n";
    for (int i = 0; i < count; ++i) {
        fout << users[i].username << ":" << users[i].password << ":" << users[i].role << "\n";
    }
    fout.close();
}

void createDefaultUsersFile(const string& path) {
    ofstream fout(path);
    if (!fout.is_open()) {
        cerr << "Error: cannot create user file: " << path << "\n";
        return;
    }
    fout << "# Format: username:password:role\n";
    fout << "inspector1:inspectorpass:inspector\n";
    fout << "vet1:vetpass:veterinarian\n";
    fout << "admin:adminpass:administrator\n";
    fout << "director:directorpass:director\n";
    fout.close();
    cout << "Created default user file: " << path << "\n";
}

bool usernameExists(const User users[], int count, const string& username) {
    for (int i = 0; i < count; ++i) {
        if (users[i].username == username) return true;
    }
    return false;
}

const User* authenticate(const User users[], int count, const string& login, const string& password) {
    for (int i = 0; i < count; ++i) {
        if (users[i].username == login && users[i].password == password) {
            return &users[i];
        }
    }
    return nullptr;
}

/**
 * @brief Реєструє нового користувача в системі.
 * ВИПРАВЛЕНО: Додано перевірку на спецсимволи та валідацію ролі.
 */
void registerUser(User users[], int& count) {
    if (count >= MAX_USERS) {
        cout << "Error: User limit reached. Cannot register new user.\n";
        return;
    }

    string username, password, role;
    cout << "\n=== User Registration (Admin) ===\n";

    // --- Введення логіна ---
    cout << "Enter new username: ";
    getline(cin, username);
    username = trim(username);

    if (username.empty()) {
        cout << "Error: Username cannot be empty.\n";
        return;
    }

    // ВИПРАВЛЕННЯ ПОМИЛКИ №1: Заборона символу ':' у логіні
    if (username.find(':') != string::npos) {
        cout << "Error: Username cannot contain the character ':' as it breaks file format.\n";
        return;
    }

    if (usernameExists(users, count, username)) {
        cout << "Error: Username already exists.\n";
        return;
    }

    // --- Введення пароля ---
    cout << "Enter password: ";
    getline(cin, password);
    password = trim(password);
    if (password.empty()) {
        cout << "Error: Password cannot be empty.\n";
        return;
    }
    // (Опціонально) Також перевіряємо пароль на ':' для надійності
    if (password.find(':') != string::npos) {
        cout << "Error: Password cannot contain the character ':'.\n";
        return;
    }

    // --- Введення ролі ---
    // ВИПРАВЛЕННЯ ПОМИЛКИ №2: Примусове введення коректної ролі
    bool validRole = false;
    while (!validRole) {
        cout << "Enter role (inspector / veterinarian / administrator / director): ";
        getline(cin, role);
        role = trim(role);

        if (role == "inspector" || role == "veterinarian" ||
            role == "administrator" || role == "director") {
            validRole = true;
        }
        else {
            cout << "Invalid role entered. Please try again.\n";
            // Ми більше не встановлюємо роль за замовчуванням мовчки
        }
    }

    users[count].username = username;
    users[count].password = password;
    users[count].role = role;
    count++;

    saveUsersToFile(USERS_FILE, users, count);
    cout << "User registered successfully!\n";
}

void runUserModule() {
    User users[MAX_USERS];
    int userCount = 0;

    loadUsersFromFile(users, userCount);

    if (userCount == 0) {
        cout << "User file not found or empty. Creating default file...\n";
        createDefaultUsersFile(USERS_FILE);
        loadUsersFromFile(users, userCount);
        if (userCount == 0) {
            cerr << "Failed to load users. Exiting module.\n";
            return;
        }
    }

    cout << "\n=== Zoo Management System ===\n";
    cout << "=== Authorization & Registration ===\n\n";

    cout << "--- Module Demo: Please login to proceed ---" << endl;
    string login, password;
    cout << "Login: ";
    getline(cin, login);
    cout << "Password: ";
    getline(cin, password);

    const User* loggedInUser = authenticate(users, userCount, trim(login), trim(password));

    if (loggedInUser == nullptr) {
        cout << "Invalid login. Returning to main menu.\n";
        return;
    }

    cout << "\nLogin successful! You are: " << loggedInUser->username
        << " (Role: " << loggedInUser->role << ")" << endl;

    while (true) {
        cout << "\n--- User Module Menu ---\n";
        if (loggedInUser->role == "administrator" || loggedInUser->role == "director") {
            cout << "1. Register New User\n";
        }
        cout << "2. Exit to Main Menu\n";
        cout << "Select option: ";

        string choice;
        getline(cin, choice);

        if (choice == "1" && (loggedInUser->role == "administrator" || loggedInUser->role == "director")) {
            registerUser(users, userCount);
        }
        else if (choice == "2") {
            cout << "Returning to main menu...\n";
            break;
        }
        else {
            cout << "Invalid choice or insufficient permissions. Please try again.\n";
        }
    }
}

// --- Спільні функції для Модулів 2 та 3 ---

void loadAnimals(Animal animals[], int& count) {
    count = 0;
    ifstream fin(ANIMALS_FILE);
    if (!fin.is_open()) return;

    string line;
    while (getline(fin, line)) {
        if (count >= MAX_ANIMALS) {
            cerr << "Warning: Max animal limit reached. Some animals not loaded.\n";
            break;
        }
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        Animal a;
        char sep;

        if (iss >> a.id >> sep && sep == ':') {
            getline(iss, a.name, ':');
            getline(iss, a.species, ':');
            iss >> a.age;
            if (iss.peek() == ':') iss.ignore();
            getline(iss, a.healthStatus);

            a.name = trim(a.name);
            a.species = trim(a.species);
            a.healthStatus = trim(a.healthStatus);

            animals[count] = a;
            count++;
        }
    }
    fin.close();
}

void saveAnimals(const Animal animals[], int count) {
    ofstream fout(ANIMALS_FILE);
    if (!fout.is_open()) {
        cerr << "Error: cannot save animal file.\n";
        return;
    }
    fout << "# Format: ID:Name:Species:Age:HealthStatus\n";
    for (int i = 0; i < count; ++i) {
        fout << animals[i].id << ":" << animals[i].name << ":" << animals[i].species
            << ":" << animals[i].age << ":" << animals[i].healthStatus << "\n";
    }
    fout.close();
}

bool idExists(const Animal animals[], int count, int id) {
    for (int i = 0; i < count; ++i) {
        if (animals[i].id == id) return true;
    }
    return false;
}

Animal* findAnimalById(Animal animals[], int count, int id) {
    for (int i = 0; i < count; ++i) {
        if (animals[i].id == id) return &animals[i];
    }
    return nullptr;
}

void showAnimals(const Animal animals[], int count) {
    cout << "\n=== Animal List ===\n";
    if (count == 0) {
        cout << "No animals found.\n";
        return;
    }

    cout << left << setw(5) << "ID"
        << setw(20) << "Name"
        << setw(20) << "Species"
        << setw(7) << "Age"
        << "Health Status" << "\n";
    cout << string(70, '-') << "\n";

    for (int i = 0; i < count; ++i) {
        cout << left << setw(5) << animals[i].id
            << setw(20) << animals[i].name
            << setw(20) << animals[i].species
            << setw(7) << animals[i].age
            << animals[i].healthStatus << "\n";
    }
}

// =================================================================================
// МОДУЛЬ 2: ДОДАВАННЯ НОВОЇ ТВАРИНИ
// =================================================================================

void addNewAnimal(Animal animals[], int& count) {
    if (count >= MAX_ANIMALS) {
        cout << "Error: Animal limit reached. Cannot add new animal.\n";
        return;
    }

    Animal a;
    cout << "\n=== Add New Animal ===\n";
    cout << "Enter animal ID (number): ";
    while (!(cin >> a.id)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cin.ignore(10000, '\n');

    if (idExists(animals, count, a.id)) {
        cout << "Error: Animal with this ID already exists.\n";
        return;
    }

    cout << "Enter animal name: ";
    getline(cin, a.name);
    // Для тварин також бажано уникати ':', оскільки це роздільник в animals.txt
    if (a.name.find(':') != string::npos) {
        cout << "Warning: Removing ':' from name to prevent database corruption.\n";
        // Простий fix: замінити ':' на пробіл або видалити
        size_t pos;
        while ((pos = a.name.find(':')) != string::npos) a.name[pos] = ' ';
    }

    cout << "Enter species (e.g., Lion, Tiger, Elephant): ";
    getline(cin, a.species);
    cout << "Enter age (in years): ";
    while (!(cin >> a.age)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cin.ignore(10000, '\n');

    cout << "Enter health status (e.g., Healthy, Sick, Injured): ";
    getline(cin, a.healthStatus);

    animals[count] = a;
    count++;

    saveAnimals(animals, count);
    cout << "\nAnimal added successfully!\n";
}

void runAddAnimalModule() {
    Animal animals[MAX_ANIMALS];
    int animalCount = 0;
    loadAnimals(animals, animalCount);

    cout << "\n=== Zoo Management System ===\n";
    cout << "=== Add New Animal Module ===\n\n";
    while (true) {
        cout << "1. Add new animal\n";
        cout << "2. Show all animals\n";
        cout << "3. Exit to Main Menu\n";
        cout << "Select option: ";
        string choice;
        getline(cin, choice);

        if (choice == "1") {
            addNewAnimal(animals, animalCount);
        }
        else if (choice == "2") {
            showAnimals(animals, animalCount);
        }
        else if (choice == "3") {
            cout << "Returning to main menu...\n";
            break;
        }
        else {
            cout << "Invalid option. Try again.\n";
        }
    }
}

// =================================================================================
// МОДУЛЬ 3: ОНОВЛЕННЯ ІНФОРМАЦІЇ
// =================================================================================

void updateAnimal(Animal animals[], int count) {
    cout << "\n=== Update Animal Information ===\n";
    if (count == 0) {
        cout << "No animals found to update.\n";
        return;
    }

    int id;
    cout << "Enter animal ID to update: ";
    while (!(cin >> id)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cin.ignore(10000, '\n');

    Animal* animal = findAnimalById(animals, count, id);
    if (!animal) {
        cout << "Error: Animal with ID " << id << " not found.\n";
        return;
    }

    cout << "\nCurrent Information:\n";
    cout << "Name: " << animal->name << "\n";
    cout << "Species: " << animal->species << "\n";
    cout << "Age: " << animal->age << "\n";
    cout << "Health: " << animal->healthStatus << "\n\n";

    cout << "Enter new age (current " << animal->age << "): ";
    while (!(cin >> animal->age)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cin.ignore(10000, '\n');

    cout << "Enter new health status (current: " << animal->healthStatus << "): ";
    getline(cin, animal->healthStatus);
    animal->healthStatus = trim(animal->healthStatus);

    saveAnimals(animals, count);
    cout << "\nAnimal information updated successfully!\n";
}

void runUpdateAnimalModule() {
    Animal animals[MAX_ANIMALS];
    int animalCount = 0;
    loadAnimals(animals, animalCount);

    cout << "\n=== Zoo Management System ===\n";
    cout << "=== Update Animal Information ===\n\n";
    if (animalCount == 0) {
        cout << "No animals found in the file. Please add some first using Add New Animal module.\n";
        cout << "Returning to main menu...\n";
        return;
    }

    while (true) {
        cout << "\n1. Update animal info\n";
        cout << "2. Show all animals\n";
        cout << "3. Exit to Main Menu\n";
        cout << "Select option: ";
        string choice;
        getline(cin, choice);

        if (choice == "1") {
            updateAnimal(animals, animalCount);
        }
        else if (choice == "2") {
            showAnimals(animals, animalCount);
        }
        else if (choice == "3") {
            cout << "Returning to main menu...\n";
            break;
        }
        else {
            cout << "Invalid option. Try again.\n";
        }
    }
}

// =================================================================================
// МОДУЛЬ 4: ФОРМУВАННЯ ЗВІТІВ
// =================================================================================

void generateReport(const Animal animals[], int count) {
    if (count == 0) {
        cout << "No animals found to generate a report.\n";
        return;
    }

    SpeciesReport reportData[MAX_ANIMALS];
    int reportSize = 0;

    // === FIX #1: Case-insensitive grouping ===
    auto toLower = [](string s) {
        for (char& c : s) c = tolower(c);
        return s;
        };

    for (int i = 0; i < count; ++i) {
        string normalized = toLower(animals[i].species);
        bool found = false;

        for (int j = 0; j < reportSize; ++j) {
            if (toLower(reportData[j].species) == normalized) {
                reportData[j].count++;
                reportData[j].totalAge += animals[i].age;
                found = true;
                break;
            }
        }

        if (!found) {
            reportData[reportSize].species = animals[i].species; // зберігаємо перший варіант написання
            reportData[reportSize].count = 1;
            reportData[reportSize].totalAge = animals[i].age;
            reportSize++;
        }
    }

    // === FIX #2: Dynamic column width ===
    int speciesColumnWidth = 20;
    for (int i = 0; i < reportSize; ++i) {
        if (reportData[i].species.length() + 2 > speciesColumnWidth)
            speciesColumnWidth = reportData[i].species.length() + 2;
    }

    ostringstream reportText;

    reportText << "=== Zoo Animal Report ===\n";
    reportText << left << setw(speciesColumnWidth) << "Species"
        << setw(10) << "Count"
        << setw(15) << "Average Age" << "\n";

    reportText << string(speciesColumnWidth + 25, '-') << "\n";

    int totalAnimals = 0;

    for (int i = 0; i < reportSize; ++i) {
        double avgAge = (reportData[i].count > 0)
            ? static_cast<double>(reportData[i].totalAge) / reportData[i].count
            : 0.0;

        reportText << left << setw(speciesColumnWidth) << reportData[i].species
            << setw(10) << reportData[i].count
            << setw(15) << fixed << setprecision(1) << avgAge << "\n";

        totalAnimals += reportData[i].count;
    }

    reportText << string(speciesColumnWidth + 25, '-') << "\n";
    reportText << "Total animals: " << totalAnimals << "\n";

    // Вивід у консоль
    cout << "\n" << reportText.str() << "\n";

    // Запис у файл
    ofstream fout(REPORT_FILE);
    if (fout.is_open()) {
        fout << reportText.str();
        fout.close();
        cout << "Report saved to " << REPORT_FILE << "\n";
    }
    else {
        cerr << "Error: Unable to save report file.\n";
    }
}

void runReportModule() {
    Animal animals[MAX_ANIMALS];
    int animalCount = 0;

    loadAnimals(animals, animalCount);

    cout << "\n=== Report Animals Module ===\n";
    cout << "Loaded " << animalCount << " animals from " << ANIMALS_FILE << "\n";

    generateReport(animals, animalCount);

    cout << "\nReport generated. Returning to main menu...\n";
}

// =================================================================================
// ГОЛОВНА ФУНКЦІЯ
// =================================================================================

int main() {
    string choice;
    while (true) {
        cout << "\n--- ZOO MANAGEMENT SYSTEM (MAIN MENU) ---\n";
        cout << "1. User Management (Login)\n";
        cout << "2. Add Animal Module\n";
        cout << "3. Update Animal Module\n";
        cout << "4. Generate Report Module\n";
        cout << "5. Exit Program\n";
        cout << "Select module: ";

        getline(cin, choice);

        if (choice == "1") {
            runUserModule();
        }
        else if (choice == "2") {
            runAddAnimalModule();
        }
        else if (choice == "3") {
            runUpdateAnimalModule();
        }
        else if (choice == "4") {
            runReportModule();
        }
        else if (choice == "5") {
            cout << "Goodbye!\n";
            break;
        }
        else {
            cout << "Invalid choice. Please select from 1 to 5.\n";
        }
    }
    return 0;
}