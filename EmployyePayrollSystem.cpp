/* =============================================================================
        EMPLOYEE PAYROLL MANAGEMENT SYSTEM
        Object-Oriented Programming - C++ Project
        Atiqa Lohani 
============================================================================= */

#include <iostream>
#include <string>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <limits>     

using namespace std;

// GLOBAL DISPLAY UTILITIES

void clrscr() { system("cls"); }

void line(char c = '=', int n = 62) {
    for (int i = 0; i < n; i++) cout << c;
    cout << "\n";
}

void center(const string& text, int width = 62) {
    int pad = (width - (int)text.length()) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) cout << " ";
    cout << text << "\n";
}

void pause_msg() {
    cout << "\nPress ENTER to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

//  CLASS: MUser  —  Sign Up / Sign In authentication
//  COVERS: Encapsulation, Constructors, Destructor,
//  Copy Constructor (Deep), this Pointer,
//  Operator Overloading (== and <<)
class MUser {
private:
    char*  uName;
    char*  uPassword;
    string uRole;

public:

    // Default Constructor 
    MUser() {
        uName     = new char[50];
        uPassword = new char[50];
        strcpy(uName,     "Unknown");
        strcpy(uPassword, "0000");
        uRole = "User";
        cout << " [Default Constructor  -> MUser created]\n";
    }

    // Parameterized Constructor
    MUser(const char* n, const char* p, const string& r) {
        uName     = new char[strlen(n) + 1];
        uPassword = new char[strlen(p) + 1];
        strcpy(uName, n);
        strcpy(uPassword, p);
        uRole = r;
        cout << " [Param Constructor    -> MUser: " << this->uName << "]\n";
    }

    // Deep Copy Constructor 
    MUser(const MUser& other) {
        uName     = new char[strlen(other.uName) + 1];
        uPassword = new char[strlen(other.uPassword) + 1];
        strcpy(this->uName,     other.uName);
        strcpy(this->uPassword, other.uPassword);
        this->uRole = other.uRole;
        cout << " [Deep Copy Constructor -> MUser: " << this->uName << "]\n";
    }

    // Assignment Operator 
    MUser& operator=(const MUser& other) {
        if (this != &other) {
            delete[] uName;
            delete[] uPassword;
            uName     = new char[strlen(other.uName) + 1];
            uPassword = new char[strlen(other.uPassword) + 1];
            strcpy(uName, other.uName);
            strcpy(uPassword, other.uPassword);
            uRole = other.uRole;
        }
        return *this;
    }

    // Destructor
    ~MUser() {
        cout << " [Destructor Called    -> MUser: " << uName << "]\n";
        delete[] uName;
        delete[] uPassword;
    }

    // Accessor Methods (Abstraction)
    string getName()     const { return string(uName);     }
    string getPassword() const { return string(uPassword); }
    string getRole()     const { return uRole;             }

    // signIn(): returns true if credentials match 
    bool signIn(const char* n, const char* p) const {
        return (strcmp(uName, n) == 0 && strcmp(uPassword, p) == 0);
    }

    // showData(): displays name and role 
    void showData() const {
        cout << " Name : " << uName << "\n";
        cout << " Role : " << uRole << "\n";
    }

    // Operator Overloading: == 
    bool operator==(const MUser& other) const {
        return strcmp(uName, other.uName) == 0;
    }

    // Friend Operator Overloading: << 
    friend ostream& operator<<(ostream& os, const MUser& u) {
        os << "User[ " << u.uName << " | " << u.uRole << " ]";
        return os;
    }
};

//  CLASS: Employee  —  Abstract Base Class
//  COVERS: Abstraction (pure virtual methods), Encapsulation,
//  Operator Overloading, this Pointer, static member
class Employee {
protected:
    int    empId;
    string empName;
    string empDept;
    double baseSalary;
    string empType;

public:
    static int totalEmployees;

    // Default Constructor 
    Employee()
        : empId(0), empName("Unknown"), empDept("N/A"),
          baseSalary(0.0), empType("None") {
        totalEmployees++;
    }

    // Parameterized Constructor 
    Employee(int id, const string& nm, const string& dept,
             double sal, const string& type)
        : empId(id), empName(nm), empDept(dept),
          baseSalary(sal), empType(type) {
        totalEmployees++;
    }

    // Copy Constructor 
    Employee(const Employee& o)
        : empId(o.empId), empName(o.empName), empDept(o.empDept),
          baseSalary(o.baseSalary), empType(o.empType) {
        totalEmployees++;
    }

    // Virtual Destructor
    virtual ~Employee() {
        totalEmployees--;
    }
    
    virtual double calculateSalary() const = 0;
    virtual void   displayInfo()     const = 0;

    // Getters 
    int    getId()         const { return empId;      }
    string getName()       const { return empName;    }
    string getDepartment() const { return empDept;    }
    double getBaseSalary() const { return baseSalary; }
    string getType()       const { return empType;    }

    // Setters with Validation 
    void setName(const string& n) { this->empName = n; }
    void setDept(const string& d) { this->empDept = d; }
    void setBaseSalary(double s) {
        if (s > 0) this->baseSalary = s;
        else cout << " [!] Invalid salary value!\n";
    }

    bool operator>(const Employee& o) const {
        return calculateSalary() > o.calculateSalary();
    }
    bool operator<(const Employee& o) const {
        return calculateSalary() < o.calculateSalary();
    }

    // Friend Operator << 
    friend ostream& operator<<(ostream& os, const Employee& e) {
        os << "Employee[ ID:" << e.empId
           << " | " << e.empName
           << " | " << e.empType << " ]";
        return os;
    }
};

int Employee::totalEmployees = 0;

//  CLASS: FullTimeEmployee

class FullTimeEmployee : public Employee {
private:
    double bonus;
    double deductions;
    int    leaveDays;

public:
    FullTimeEmployee()
        : Employee(), bonus(0), deductions(0), leaveDays(0) {}

    FullTimeEmployee(int id, const string& nm, const string& dept,
                     double sal, double bon, double ded, int leave)
        : Employee(id, nm, dept, sal, "Full-Time"),
          bonus(bon), deductions(ded), leaveDays(leave) {}

    FullTimeEmployee(const FullTimeEmployee& o)
        : Employee(o), bonus(o.bonus),
          deductions(o.deductions), leaveDays(o.leaveDays) {}

    ~FullTimeEmployee() {}

    double calculateSalary() const override {
        double perDay      = baseSalary / 30.0;
        double leaveDeduct = perDay * leaveDays;
        return baseSalary + bonus - deductions - leaveDeduct;
    }

    void displayInfo() const override {
        line('-', 62);
        cout << " Type            : Full-Time Employee\n";
        cout << " ID              : " << empId   << "\n";
        cout << " Name            : " << empName << "\n";
        cout << " Department      : " << empDept << "\n";
        cout << fixed << setprecision(2);
        cout << " Base Salary     : " << baseSalary        << "\n";
        cout << " Bonus           : " << bonus             << "\n";
        cout << " Deductions      : " << deductions        << "\n";
        cout << " Leave Days      : "  << leaveDays         << " day(s)\n";
        cout << " NET SALARY      : " << calculateSalary() << "\n";
        line('-', 62);
    }

    double getBonus()      const { return bonus;      }
    double getDeductions() const { return deductions; }
    int    getLeaveDays()  const { return leaveDays;  }
};

//  CLASS: PartTimeEmployee

class PartTimeEmployee : public Employee {
private:
    double hourlyRate;
    int    hoursWorked;

public:
    PartTimeEmployee()
        : Employee(), hourlyRate(0), hoursWorked(0) {}

    PartTimeEmployee(int id, const string& nm, const string& dept,
                     double rate, int hours)
        : Employee(id, nm, dept, rate * hours, "Part-Time"),
          hourlyRate(rate), hoursWorked(hours) {}

    PartTimeEmployee(const PartTimeEmployee& o)
        : Employee(o), hourlyRate(o.hourlyRate), hoursWorked(o.hoursWorked) {}

    ~PartTimeEmployee() {}

    double calculateSalary() const override {
        return hourlyRate * hoursWorked;
    }

    void displayInfo() const override {
        line('-', 62);
        cout << " Type            : Part-Time Employee\n";
        cout << " ID              : " << empId       << "\n";
        cout << " Name            : " << empName     << "\n";
        cout << " Department      : " << empDept     << "\n";
        cout << fixed << setprecision(2);
        cout << " Hourly Rate     : " << hourlyRate        << "\n";
        cout << " Hours Worked    : "  << hoursWorked       << " hrs\n";
        cout << " NET SALARY      : " << calculateSalary() << "\n";
        line('-', 62);
    }
};

// CLASS: ContractEmployee

class ContractEmployee : public Employee {
private:
    double contractAmount;
    int    contractMonths;
    double taxRate;

public:
    ContractEmployee()
        : Employee(), contractAmount(0), contractMonths(1), taxRate(0.1) {}

    ContractEmployee(int id, const string& nm, const string& dept,
                     double amount, int months, double tax)
        : Employee(id, nm, dept, amount, "Contract"),
          contractAmount(amount), contractMonths(months), taxRate(tax) {}

    ContractEmployee(const ContractEmployee& o)
        : Employee(o), contractAmount(o.contractAmount),
          contractMonths(o.contractMonths), taxRate(o.taxRate) {}

    ~ContractEmployee() {}

    double calculateSalary() const override {
        double monthly = contractAmount / contractMonths;
        return monthly - (monthly * taxRate);
    }

    void displayInfo() const override {
        line('-', 62);
        cout << " Type            : Contract Employee\n";
        cout << " ID              : " << empId           << "\n";
        cout << " Name            : " << empName         << "\n";
        cout << " Department      : " << empDept         << "\n";
        cout << fixed << setprecision(2);
        cout << " Contract Amount : " << contractAmount      << "\n";
        cout << " Contract Months : "  << contractMonths      << " month(s)\n";
        cout << " Tax Rate        : "  << (taxRate * 100)     << "%\n";
        cout << " Monthly Net Pay : " << calculateSalary()   << "\n";
        line('-', 62);
    }
};

//  CLASS: PayrollSystem
//  COVERS: Dynamic Memory Allocation, Arrow Operator,
//  Operator Overloading (+=), Destructor

class PayrollSystem {
private:
    Employee** empList;
    int        count;
    int        capacity;
    string     companyName;

    void resize() {
        capacity *= 2;
        Employee** tmp = new Employee*[capacity];
        for (int i = 0; i < count; i++) tmp[i] = empList[i];
        delete[] empList;
        empList = tmp;
    }

public:
    PayrollSystem(const string& company = "TechCorp Pvt Ltd", int cap = 10)
        : count(0), capacity(cap), companyName(company) {
        empList = new Employee*[capacity];
    }

    ~PayrollSystem() {
        cout << "\n [PayrollSystem Destructor: releasing "
             << count << " employee record(s)]\n";
        for (int i = 0; i < count; i++) delete empList[i];
        delete[] empList;
    }

    void addEmployee(Employee* e) {
        if (count >= capacity) resize();
        empList[count++] = e;
        cout << "  >> Record added! Total employees: " << count << "\n";
    }

    // Overloaded += for cascaded style
    PayrollSystem& operator+=(Employee* e) {
        addEmployee(e);
        return *this;
    }

    void viewAll() const {
        if (count == 0) { cout << " No records found.\n"; return; }
        for (int i = 0; i < count; i++) empList[i]->displayInfo();
    }

    Employee* searchById(int id) const {
        for (int i = 0; i < count; i++)
            if (empList[i]->getId() == id) return empList[i];
        return nullptr;
    }

    Employee* searchByName(const string& nm) const {
        for (int i = 0; i < count; i++)
            if (empList[i]->getName() == nm) return empList[i];
        return nullptr;
    }

    bool updateSalary(int id, double newSal) {
        Employee* e = searchById(id);
        if (e) { e->setBaseSalary(newSal); return true; }
        return false;
    }

    bool deleteEmployee(int id) {
        for (int i = 0; i < count; i++) {
            if (empList[i]->getId() == id) {
                delete empList[i];
                for (int j = i; j < count - 1; j++)
                    empList[j] = empList[j + 1];
                count--;
                return true;
            }
        }
        return false;
    }

    double totalPayroll() const {
        double total = 0;
        for (int i = 0; i < count; i++)
            total += empList[i]->calculateSalary();
        return total;
    }

    Employee* highestPaid() const {
        if (!count) return nullptr;
        Employee* top = empList[0];
        for (int i = 1; i < count; i++)
            if (*empList[i] > *top) top = empList[i];
        return top;
    }

    Employee* lowestPaid() const {
        if (!count) return nullptr;
        Employee* low = empList[0];
        for (int i = 1; i < count; i++)
            if (*empList[i] < *low) low = empList[i];
        return low;
    }

    void summaryReport() const {
        clrscr();
        line('=', 62);
        center("PAYROLL SUMMARY REPORT", 62);
        center(companyName, 62);
        line('=', 62);

        if (!count) { cout << " No employee records found.\n"; return; }

        cout << "  " << left
             << setw(6)  << "ID"
             << setw(18) << "Name"
             << setw(14) << "Type"
             << setw(12) << "Net Salary"
             << "\n";
        line('-', 62);

        for (int i = 0; i < count; i++) {
            cout << "  " << left
                 << setw(6)  << empList[i]->getId()
                 << setw(18) << empList[i]->getName()
                 << setw(14) << empList[i]->getType()
                 << " " << fixed << setprecision(2)
                 << empList[i]->calculateSalary() << "\n";
        }

        line('-', 62);
        cout << fixed << setprecision(2);
        cout << " Total Employees : " << count << "\n";
        cout << " Total Payroll   : " << totalPayroll() << "\n";

        Employee* hp = highestPaid();
        Employee* lp = lowestPaid();
        if (hp)
            cout << " Highest Paid    : " << hp->getName()
                 << "  (" << hp->calculateSalary() << ")\n";
        if (lp)
            cout << "  Lowest Paid     : " << lp->getName()
                 << "  (" << lp->calculateSalary() << ")\n";
        line('=', 62);
    }

    int    getCount()       const { return count;       }
    string getCompanyName() const { return companyName; }
};

// SHALLOW COPY DEMO — intentional dangerous copy for teaching

class ShallowDemo {
public:
    char* data;

    ShallowDemo(const char* d) {
        data = new char[strlen(d) + 1];
        strcpy(data, d);
    }

    // Shallow: copies pointer, not data
    ShallowDemo(const ShallowDemo& o) {
        data = o.data;   // same address — intentional for demo
        cout << " [Shallow Copy] Both point to: " << (void*)data << "\n";
    }

    void show(const string& lbl) {
        cout << "  " << lbl << " = " << data
             << "  (addr: " << (void*)data << ")\n";
    }

    // Destructor deliberately does NOT free to avoid double-free in demo
    ~ShallowDemo() {}
};

// DEEP COPY DEMO — safe independent copy

class DeepDemo {
public:
    char* data;

    DeepDemo(const char* d) {
        data = new char[strlen(d) + 1];
        strcpy(data, d);
    }

    // Deep: new allocation — independent memory
    DeepDemo(const DeepDemo& o) {
        data = new char[strlen(o.data) + 1];
        strcpy(data, o.data);
        cout << " [Deep Copy]    New memory at: " << (void*)data << "\n";
    }

    void show(const string& lbl) {
        cout << "  " << lbl << " = " << data
             << "  (addr: " << (void*)data << ")\n";
    }

    ~DeepDemo() { delete[] data; }
};

//   SIGN-UP / SIGN-IN GLOBALS

const int MAX_USERS = 20;
MUser* userList[MAX_USERS];
int    userCount = 0;

//  signUpUser() — Dynamic memory, parameterized constructor,
//  validation, this pointer

bool signUpUser() {
    clrscr();
    line('=', 62);
    center("S I G N   U P - Create New Account", 62);
    line('=', 62);

    if (userCount >= MAX_USERS) {
        cout << " [!] Maximum user limit reached!\n";
        pause_msg();
        return false;
    }

    char   name[50], pass[50], confirm[50];
    string role;

    cout << "\n Enter Username   : ";
    cin  >> name;

    // Duplicate username check
    for (int i = 0; i < userCount; i++) {
        if (userList[i]->getName() == string(name)) {
            cout << "\n [!] Username already exists! Try another.\n";
            pause_msg();
            return false;
        }
    }

    cout << " Enter Password   : "; cin >> pass;
    cout << " Confirm Password : "; cin >> confirm;

    if (strcmp(pass, confirm) != 0) {
        cout << "\n [!] Passwords do not match!\n";
        pause_msg();
        return false;
    }

    cout << "\nSelect Role:\n";
    cout << "   [1] Admin\n";
    cout << "   [2] HR Manager\n";
    cout << "   [3] Employee\n";
    cout << "Choice: ";
    int rc; cin >> rc;

    switch (rc) {
        case 1:  role = "Admin";      break;
        case 2:  role = "HR Manager"; break;
        case 3:  role = "Employee";   break;
        default: role = "Employee";
    }

    cout << "\n [Allocating user object with 'new'...]\n";
    userList[userCount] = new MUser(name, pass, role);
    userCount++;

    cout << "\n";
    line('-', 62);
    center("ACCOUNT CREATED SUCCESSFULLY!", 62);
    line('-', 62);
    cout << "Name : " << name << "   Role : " << role << "\n";
    pause_msg();
    return true;
}

//  signInUser() — Arrow operator, signIn(), validation
//  FIX 6: Restructured to remove unreachable return at end

MUser* signInUser() {
    clrscr();
    line('=', 62);
    center("S I G N   I N - Login to Your Account", 62);
    line('=', 62);

    if (userCount == 0) {
        cout << "\n [!] No accounts found. Please sign up first.\n";
        pause_msg();
        return nullptr;
    }

    char name[50], pass[50];
    int  attempts = 3;

    while (attempts > 0) {
        cout << "\n Username : "; cin >> name;
        cout << " Password : "; cin >> pass;

        for (int i = 0; i < userCount; i++) {
            if (userList[i]->signIn(name, pass)) {   // arrow operator ->
                cout << "\n";
                line('+', 62);
                center("LOGIN SUCCESSFUL!", 62);
                line('+', 62);
                cout << " Welcome, ";
                userList[i]->showData();       
                pause_msg();
                return userList[i];
            }
        }

        attempts--;
        if (attempts > 0) {
            cout << "\n[!] Invalid Credentials! Attempts left: "
                 << attempts << "\n";
        } else {
            cout << "\n[!] Too many failed attempts. Access denied.\n";
            pause_msg();
        }
    }

    return nullptr;   
}

//   EMPLOYEE INPUT HELPERS

int nextId = 1001;

void addFullTime(PayrollSystem& sys) {
    clrscr();
    line('=', 62); center("ADD  FULL-TIME  EMPLOYEE", 62); line('=', 62);
    string name, dept;
    double sal, bon, ded;
    int    leave;

    cout << "\n Auto ID         : " << nextId << "\n";
    cout << " Name            : "; cin >> name;
    cout << " Department      : "; cin >> dept;
    cout << " Base Salary     : "; cin >> sal;
    while (sal <= 0) {
        cout << "  [!] Must be > 0. Re-enter: "; cin >> sal;
    }
    cout << " Bonus           : "; cin >> bon;
    cout << " Deductions      : "; cin >> ded;
    cout << " Leave Days      : "; cin >> leave;
    while (leave < 0 || leave > 30) {
        cout << " [!] 0-30 only. Re-enter: "; cin >> leave;
    }

    sys += new FullTimeEmployee(nextId++, name, dept, sal, bon, ded, leave);
    pause_msg();
}

void addPartTime(PayrollSystem& sys) {
    clrscr();
    line('=', 62); center("ADD  PART-TIME  EMPLOYEE", 62); line('=', 62);
    string name, dept;
    double rate;
    int    hours;

    cout << "\n Auto ID         : " << nextId << "\n";
    cout << " Name            : "; cin >> name;
    cout << " Department      : "; cin >> dept;
    cout << " Hourly Rate     : "; cin >> rate;
    while (rate <= 0) {
        cout << " [!] Must be > 0. Re-enter: "; cin >> rate;
    }
    cout << " Hours Worked    : "; cin >> hours;
    while (hours <= 0 || hours > 744) {
        cout << " [!] 1-744. Re-enter: "; cin >> hours;
    }

    sys += new PartTimeEmployee(nextId++, name, dept, rate, hours);
    pause_msg();
}

void addContract(PayrollSystem& sys) {
    clrscr();
    line('=', 62); center("ADD  CONTRACT  EMPLOYEE", 62); line('=', 62);
    string name, dept;
    double amount, tax;
    int    months;

    cout << "\n Auto ID            : " << nextId << "\n";
    cout << " Name               : "; cin >> name;
    cout << " Department         : "; cin >> dept;
    cout << " Contract Amount    : "; cin >> amount;
    while (amount <= 0) {
        cout << " [!] Must be > 0. Re-enter: "; cin >> amount;
    }
    cout << " Contract Months    : "; cin >> months;
    while (months <= 0 || months > 60) {
        cout << " [!] 1-60. Re-enter: "; cin >> months;
    }
    cout << " Tax Rate (0.0-1.0) : "; cin >> tax;
    while (tax < 0.0 || tax > 1.0) {
        cout << " [!] 0.0-1.0 only. Re-enter: "; cin >> tax;
    }

    sys += new ContractEmployee(nextId++, name, dept, amount, months, tax);
    pause_msg();
}

void searchMenu(const PayrollSystem& sys) {
    clrscr();
    line('=', 62); center("SEARCH  EMPLOYEE", 62); line('=', 62);
    cout << "\n [1] Search by ID\n  [2] Search by Name\n  Choice: ";
    int ch; cin >> ch;
    Employee* res = nullptr;

    if (ch == 1) {
        int id; cout << " Employee ID   : "; cin >> id;
        res = sys.searchById(id);
    } else {
        string nm; cout << " Employee Name : "; cin >> nm;
        res = sys.searchByName(nm);
    }

    cout << "\n";
    if (res) { cout << " Record Found:\n"; res->displayInfo(); }
    else      cout << " [!] Employee not found!\n";
    pause_msg();
}

//  DEMO: Copy Constructor — MUser deep copy walkthrough

void demoCopyConstructor() {
    clrscr();
    line('=', 62);
    center("COPY CONSTRUCTOR DEMO  (MUser)", 62);
    line('=', 62);

    cout << "\n Step 1: Create original with 'new' (Param Constructor)\n";
    MUser* orig = new MUser("DemoAdmin", "secret99", "Admin");

    cout << "\n Step 2: showData() via ARROW OPERATOR (->)\n";
    cout << "  Original:\n";
    orig->showData();

    cout << "\n Step 3: Create copy using Copy Constructor (deep copy)\n";
    MUser* cpy = new MUser(*orig);

    cout << "\n Step 4: showData() via DOT OPERATOR (dereference first)\n";
    (*cpy).showData();

    cout << "\n Step 5: Operator == test:\n";
    cout << "  orig == cpy : " << (*orig == *cpy ? "TRUE" : "FALSE") << "\n";

    cout << "\n Step 6: Stream Operator << test:\n";
    cout << "  " << *orig << "\n";
    cout << "  " << *cpy  << "\n";

    cout << "\n Step 7: delete orig → destructor fires\n";
    delete orig;

    cout << "\n Step 8: copy still works (deep copy — independent memory)\n";
    cpy->showData();

    cout << "\n Step 9: delete copy\n";
    delete cpy;

    pause_msg();
}

//  DEMO: Shallow vs Deep Copy side-by-side

void demoShallowDeep() {
    clrscr();
    line('=', 62);
    center("SHALLOW  vs  DEEP  COPY  DEMO", 62);
    line('=', 62);

    cout << "\n SHALLOW COPY — both share SAME address\n\n";

    ShallowDemo s1("PayrollData");
    s1.show("s1 (original)");
    ShallowDemo s2(s1);
    s2.show("s2 (copy)   ");

    cout << "\n Modifying s1 → strcpy(s1.data, \"HACKED!\")\n";
    strcpy(s1.data, "HACKED!");
    s1.show("s1 after    ");
    s2.show("s2 after    ");
    cout << " [!] s2 ALSO changed — DANGEROUS!\n";

    cout << "\n DEEP COPY — each has OWN separate address\n\n";

    DeepDemo d1("PayrollData");
    d1.show("d1 (original)");
    DeepDemo d2(d1);
    d2.show("d2 (copy)   ");

    cout << "\n Modifying d1 → strcpy(d1.data, \"CHANGED!\")\n";
    strcpy(d1.data, "CHANGED!");
    d1.show("d1 after    ");
    d2.show("d2 after    ");
    cout << " [OK] d2 UNAFFECTED — SAFE!\n";

    pause_msg();
}

//  MAIN MENU display

void showMainMenu(const string& userName, const string& userRole) {
    line('=', 62);
    center("EMPLOYEE  PAYROLL  MANAGEMENT  SYSTEM", 62);
    center("Atiqa Lohani", 62);
    line('-', 62);
    center("Logged in: " + userName + "  [" + userRole + "]", 62);
    line('=', 62);
    cout << "\n";
    cout << "  EMPLOYEE RECORDS\n";
    line('-', 62);
    cout << "  [1]  Add Full-Time Employee\n";
    cout << "  [2]  Add Part-Time Employee\n";
    cout << "  [3]  Add Contract Employee\n";
    cout << "  [4]  View All Employees\n";
    cout << "  [5]  Search Employee\n";
    cout << "  [6]  Update Employee Salary\n";
    cout << "  [7]  Delete Employee Record\n";
    cout << "  [8]  Payroll Summary Report\n";
    cout << "\n";
    cout << "  OOP DEMONSTRATIONS\n";
    line('-', 62);
    cout << "  [9]  Copy Constructor Demo (MUser)\n";
    cout << "  [10] Shallow vs Deep Copy Demo\n";
    cout << "\n";
    cout << "  [0]  Logout\n";
    line('=', 62);
    cout << "Choice: ";
}

//  BANNER
void banner() {
    clrscr();
    cout << "\n";
    line('=', 62);
    center("=    EMPLOYEE PAYROLL MANAGEMENT SYSTEM     =", 62);
    center("=     Object-Oriented Programming C++       =", 62);
    center("=               Atiqa Lohani                =", 62);
    line('=', 62);
    cout << "\n";
}

// MAIN — SignUp → SignIn → Menu → Logout → Cleanup

int main() {

    PayrollSystem* payroll = new PayrollSystem("Atiqa Lohani");

    bool appRunning = true;

    while (appRunning) {
        banner();
        line('-', 62);
        center("WELCOME", 62);
        line('-', 62);
        cout << " [1]  Sign Up  (Create Account)\n";
        cout << " [2]  Sign In  (Login)\n";
        cout << " [0]  Exit Application\n";
        line('-', 62);
        cout << "Choice: ";

        int startChoice;
        cin >> startChoice;

        if (startChoice == 0) {
            appRunning = false;

        } else if (startChoice == 1) {
            signUpUser();

        } else if (startChoice == 2) {
            MUser* currentUser = signInUser();

            if (currentUser != nullptr) {
                bool loggedIn = true;

                while (loggedIn) {
                    clrscr();
                    showMainMenu(currentUser->getName(),   // -> operator
                                 currentUser->getRole());

                    int choice;
                    cin >> choice;

                    switch (choice) {
                        case 1:  addFullTime(*payroll); break;
                        case 2:  addPartTime(*payroll); break;
                        case 3:  addContract(*payroll); break;

                        case 4:
                            clrscr();
                            line('=', 62);
                            center("ALL  EMPLOYEE  RECORDS", 62);
                            line('=', 62);
                            payroll->viewAll();
                            pause_msg();
                            break;

                        case 5:  searchMenu(*payroll); break;

                        case 6: {
                            clrscr();
                            line('=', 62);
                            center("UPDATE  EMPLOYEE  SALARY", 62);
                            line('=', 62);
                            int id; double ns;
                            cout << "\n  Employee ID        : "; cin >> id;
                            cout << " New Base Salary ($): "; cin >> ns;
                            if (payroll->updateSalary(id, ns))
                                cout << "\n [OK] Salary updated!\n";
                            else
                                cout << "\n [!] Employee not found!\n";
                            pause_msg();
                            break;
                        }

                        case 7: {
                            clrscr();
                            line('=', 62);
                            center("DELETE  EMPLOYEE  RECORD", 62);
                            line('=', 62);
                            int id;
                            cout << "\n Enter Employee ID to delete: ";
                            cin >> id;
                            if (payroll->deleteEmployee(id))
                                cout << "\n [OK] Employee deleted!\n";
                            else
                                cout << "\n [!] Employee not found!\n";
                            pause_msg();
                            break;
                        }

                        case 8:
                            payroll->summaryReport();
                            pause_msg();
                            break;

                        case 9:  demoCopyConstructor(); break;
                        case 10: demoShallowDeep();     break;

                        case 0:
                            loggedIn = false;
                            cout << "\n Logging out...\n";
                            pause_msg();
                            break;

                        default:
                            cout << "\n [!] Invalid choice. Please try again.\n";
                            pause_msg();
                    }
                }
            }

        } else {
            cout << "\n [!] Invalid choice.\n";
            pause_msg();
        }
    }

    // Release all memory 
    cout << "\n";
    line('=', 62);
    center("SHUTTING DOWN — Releasing Memory", 62);
    line('=', 62);
    cout << "\n [Deleting user objects...]\n";
    for (int i = 0; i < userCount; i++)
        delete userList[i];

    cout << "\n [Deleting PayrollSystem...]\n";
    delete payroll;

    cout << "\n";
    line('=', 62);
    center("Thank you for using the Payroll System!", 62);
    center("All dynamically allocated memory freed.", 62);
    line('=', 62);
    cout << "\n";

    return 0;
}