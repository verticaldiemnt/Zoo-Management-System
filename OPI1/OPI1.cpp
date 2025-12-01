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

/**
 * @brief Видаляє пробіли зліва (з початку) рядка.
 * @param s Вхідний рядок.
 * @return Рядок без пробілів на початку.
 */
static inline string ltrim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    return (start == string::npos) ? "" : s.substr(start);
}

/**
 * @brief Видаляє пробіли справа (з кінця) рядка.
 * @param s Вхідний рядок.
 * @return Рядок без пробілів у кінці.
 */
static inline string rtrim(const string& s) {
    size_t end = s.find_last_not_of(" \t\r\n");
    return (end == string::npos) ? "" : s.substr(0, end + 1);
}

/**
 * @brief Видаляє пробіли з обох кінців рядка.
 * @param s Вхідний рядок.
 * @return Очищений рядок.
 */
static inline string trim(const string& s) {
    return rtrim(ltrim(s));
}

// =================================================================================
// ПОЧАТОК БЛОКУ УЧАСНИКА 1
// МОДУЛЬ 1: АВТОРИЗАЦІЯ КОРИСТУВАЧА
// (Функції: loadUsersFromFile, saveUsersToFile, createDefaultUsersFile,
// usernameExists, authenticate, registerUser, runUserModule)
// =================================================================================

/*
Модуль “Авторизація користувача”: Забезпечує вхід до системи,
перевірку логіна і пароля користувача, визначає його роль (інспектор,
ветеринар, адміністратор, директор) і надає відповідний рівень
доступу. Також дозволяє створювати нових користувачів.
*/

/**
 * @brief Завантажує список користувачів з файлу USERS_FILE у масив.
 * @param users Масив для заповнення користувачами.
 * @param count Посилання на змінну, куди буде записано кількість завантажених користувачів.
 */
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

/**
 * @brief Зберігає поточний список користувачів з масиву у файл.
 * @param path Шлях до файлу (зазвичай USERS_FILE).
 * @param users Масив користувачів для збереження.
 * @param count Кількість користувачів у масиві.
 */
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

/**
 * @brief Створює файл users.txt за замовчуванням, якщо він не існує.
 * @param path Шлях до файлу (зазвичай USERS_FILE).
 */
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

/**
 * @brief Перевіряє, чи існує користувач з таким іменем.
 * @param users Масив користувачів.
 * @param count Кількість користувачів.
 * @param username Ім'я, яке потрібно перевірити.
 * @return true, якщо ім'я вже зайняте, інакше false.
 */
bool usernameExists(const User users[], int count, const string& username) {
    for (int i = 0; i < count; ++i) {
        if (users[i].username == username) return true;
    }
    return false;
}

/**
 * @brief Перевіряє логін та пароль користувача.
 * @param users Масив користувачів.
 * @param count Кількість користувачів.
 * @param login Логін, що перевіряється.
 * @param password Пароль, що перевіряється.
 * @return Вказівник на об'єкт User у разі успіху, або nullptr у разі невдачі.
 */
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
 * (Викликається лише адміністратором або директором).
 * @param users Масив користувачів.
 * @param count Посилання на лічильник користувачів (буде збільшено).
 */
void registerUser(User users[], int& count) {
    if (count >= MAX_USERS) {
        cout << "Error: User limit reached. Cannot register new user.\n";
        return;
    }

    string username, password, role;
    cout << "\n=== User Registration (Admin) ===\n";
    cout << "Enter new username: ";
    getline(cin, username);
    username = trim(username);

    if (username.empty()) {
        cout << "Error: Username cannot be empty.\n";
        return;
    }
    if (usernameExists(users, count, username)) {
        cout << "Error: Username already exists.\n";
        return;
    }
    cout << "Enter password: ";
    getline(cin, password);
    password = trim(password);
    if (password.empty()) {
        cout << "Error: Password cannot be empty.\n";
        return;
    }

    // --- Ось де адмін призначає роль ---
    cout << "Enter role (inspector / veterinarian / administrator / director): ";
    getline(cin, role);
    role = trim(role);

    if (role != "inspector" && role != "veterinarian" && role != "administrator" && role != "director") {
        cout << "Invalid role. Defaulting to 'inspector'.\n";
        role = "inspector";
    }
    // --- Кінець призначення ролі ---

    users[count].username = username;
    users[count].password = password;
    users[count].role = role;
    count++;

    saveUsersToFile(USERS_FILE, users, count);
    cout << "User registered successfully!\n";
}

/**
 * @brief Функція-запускач для Модуля 1.
 * (ОНОВЛЕНО: тепер вимагає вхід для доступу до реєстрації)
 */
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

    // --- Крок 1: Вимагаємо вхід для демонстрації модуля ---
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

    // --- Крок 2: Меню на основі ролі ---
    while (true) {
        cout << "\n--- User Module Menu ---\n";

        // *Тільки* адмін (або директор) бачить опцію реєстрації
        if (loggedInUser->role == "administrator" || loggedInUser->role == "director") {
            cout << "1. Register New User\n";
        }
        cout << "2. Exit to Main Menu\n";
        cout << "Select option: ";

        string choice;
        getline(cin, choice);

        if (choice == "1" && (loggedInUser->role == "administrator" || loggedInUser->role == "director")) {
            // Адмін, який увійшов, тепер запускає функцію реєстрації
            // і сам призначає роль новому користувачу
            registerUser(users, userCount);
        }
        else if (choice == "2") {
            cout << "Returning to main menu...\n";
            break;
        }
        else {
            // Сюди потрапить не-адмін, якщо введе "1"
            cout << "Invalid choice or insufficient permissions. Please try again.\n";
        }
    }
}
// =================================================================================
// КІНЕЦЬ 1 модуля
// =================================================================================


// --- Спільні функції для Модулів 2 та 3 ---
// (Ці функції можуть бути представлені Учасником 2 або 3)

/**
 * @brief Завантажує список тварин з файлу ANIMALS_FILE у масив.
 * @param animals Масив для заповнення тваринами.
 * @param count Посилання на змінну, куди буде записано кількість завантажених тварин.
 */
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

/**
 * @brief Зберігає поточний список тварин з масиву у файл ANIMALS_FILE.
 * @param animals Масив тварин для збереження.
 * @param count Кількість тварин у масиві.
 */
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

/**
 * @brief Перевіряє, чи існує тварина з таким ID.
 * @param animals Масив тварин.
 * @param count Кількість тварин.
 * @param id ID, яке потрібно перевірити.
 * @return true, якщо ID вже зайнятий, інакше false.
 */
bool idExists(const Animal animals[], int count, int id) {
    for (int i = 0; i < count; ++i) {
        if (animals[i].id == id) return true;
    }
    return false;
}

/**
 * @brief Знаходить тварину за її ID.
 * @param animals Масив тварин.
 * @param count Кількість тварин.
 * @param id ID тварини для пошуку.
 * @return Вказівник на об'єкт Animal у разі успіху, або nullptr, якщо тварину не знайдено.
 */
Animal* findAnimalById(Animal animals[], int count, int id) {
    for (int i = 0; i < count; ++i) {
        if (animals[i].id == id) return &animals[i];
    }
    return nullptr;
}

/**
 * @brief Виводить у консоль список усіх тварин у вигляді таблиці.
 * (Використовується Модулями 2 та 3 для демонстрації)
 */
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
// (Функції: addNewAnimal, runAddAnimalModule)
// =================================================================================

/*
Модуль “Додавання нової тварини”: Призначений для внесення в
базу нових тварин. Користувач вводить ID, назву, вид, вік
і стан здоров’я.
*/

/**
 * @brief Додає нову тварину до масиву та зберігає у файл.
 * @param animals Масив тварин.
 * @param count Посилання на лічильник тварин.
 */
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
    cin.ignore(10000, '\n'); // Очистити буфер після cin

    if (idExists(animals, count, a.id)) {
        cout << "Error: Animal with this ID already exists.\n";
        return;
    }

    cout << "Enter animal name: ";
    getline(cin, a.name);
    cout << "Enter species (e.g., Lion, Tiger, Elephant): ";
    getline(cin, a.species);
    cout << "Enter age (in years): ";
    while (!(cin >> a.age)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cin.ignore(10000, '\n'); // Очистити буфер після cin

    cout << "Enter health status (e.g., Healthy, Sick, Injured): ";
    getline(cin, a.healthStatus);

    animals[count] = a;
    count++;

    saveAnimals(animals, count);
    cout << "\nAnimal added successfully!\n";
}

/**
 * @brief Функція-запускач для Модуля 2.
 */
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
// КІНЕЦЬ 2 модуля
// =================================================================================


// =================================================================================
// МОДУЛЬ 3: ОНОВЛЕННЯ ІНФОРМАЦІЇ
// (Функції: updateAnimal, runUpdateAnimalModule)
// =================================================================================

/*
Модуль “Оновлення інформації”: Дозволяє редагувати дані про
існуючих тварин. Можна змінювати вік або стан здоров’я за ID
тварини. Зміни зберігаються у файл бази.
*/

/**
 * @brief Оновлює інформацію (вік, здоров'я) про існуючу тварину за ID.
 * @param animals Масив тварин.
 * @param count Кількість тварин.
 */
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
    cin.ignore(10000, '\n'); // Очистити буфер після cin

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
    cin.ignore(10000, '\n'); // Очистити буфер після cin

    cout << "Enter new health status (current: " << animal->healthStatus << "): ";
    getline(cin, animal->healthStatus);
    animal->healthStatus = trim(animal->healthStatus);

    saveAnimals(animals, count);
    cout << "\nAnimal information updated successfully!\n";
}

/**
 * @brief Функція-запускач для Модуля 3.
 */
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
// Кінець 3 модуля
// =================================================================================


// =================================================================================
// МОДУЛЬ 4: ФОРМУВАННЯ ЗВІТІВ
// (Функції: generateReport, runReportModule)
// =================================================================================

/*
Модуль “Формування звітів”: Здійснює аналіз усіх наявних даних про
тварин і формує статистичний звіт — кількість тварин за видами,
середній вік, загальну кількість.
*/

/**
 * @brief Генерує звіт за видами тварин (кількість, середній вік).
 * Виводить звіт у консоль та зберігає у файл REPORT_FILE.
 * @param animals Масив тварин.
 * @param count Кількість тварин.
 */
void generateReport(const Animal animals[], int count) {
    if (count == 0) {
        cout << "No animals found to generate a report.\n";
        return;
    }

    // Використовуємо MAX_ANIMALS, оскільки видів не може бути більше, ніж тварин
    SpeciesReport reportData[MAX_ANIMALS];
    int reportSize = 0;

    for (int i = 0; i < count; ++i) {
        bool found = false;
        for (int j = 0; j < reportSize; ++j) {
            if (reportData[j].species == animals[i].species) {
                reportData[j].count++;
                reportData[j].totalAge += animals[i].age;
                found = true;
                break;
            }
        }
        if (!found) {
            if (reportSize < MAX_ANIMALS) { // Переконайтеся, що не виходимо за межі масиву
                reportData[reportSize].species = animals[i].species;
                reportData[reportSize].count = 1;
                reportData[reportSize].totalAge = animals[i].age;
                reportSize++;
            }
        }
    }

    ostringstream reportText;
    reportText << "=== Zoo Animal Report ===\n";
    reportText << left << setw(20) << "Species"
        << setw(10) << "Count"
        << setw(15) << "Average Age" << "\n";
    reportText << string(45, '-') << "\n";

    int totalAnimals = 0;
    for (int i = 0; i < reportSize; ++i) {
        double avgAge = (reportData[i].count > 0) ?
            static_cast<double>(reportData[i].totalAge) / reportData[i].count : 0.0;

        reportText << left << setw(20) << reportData[i].species
            << setw(10) << reportData[i].count
            << setw(15) << fixed << setprecision(1) << avgAge << "\n";

        totalAnimals += reportData[i].count;
    }

    reportText << string(45, '-') << "\n";
    reportText << "Total animals: " << totalAnimals << "\n";

    cout << "\n" << reportText.str() << "\n";

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

/**
 * @brief Функція-запускач для Модуля 4.
 */
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
// Кінець 4 модуля
// =================================================================================


// =================================================================================
// ГОЛОВНА ФУНКЦІЯ (МЕНЮ ДЛЯ ПРЕЗЕНТАЦІЇ)
// =================================================================================

/**
 * @brief Головна функція програми.
 * Показує меню вибору модуля для демонстрації.
 * @return 0 у разі успішного завершення.
 */
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