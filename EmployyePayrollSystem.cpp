/* =============================================================================
   PAYROLL MANAGEMENT SYSTEM
   Object Oriented Programming
   Atiqa Lohani
   Features: Role-Based Access | File Storage | CNIC Validation | ANSI Colors
============================================================================= */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <algorithm>
using namespace std;

//  ANSI COLOR CODES
#define RESET "\033[0m"
#define BOLD "\033[1m"

// Role Colors
#define BLUE "\033[94m"      // Project name / Welcome / Thank you
#define PURPLE "\033[95m"      // Admin
#define GREEN "\033[92m"      // HR Manager
#define ORANGE "\033[93m"      // Employee  (bright yellow = closest to orange in ANSI)
#define RED "\033[91m"      // Errors
#define CYAN "\033[96m"      // General UI accents
#define WHITE "\033[97m"      // Normal text

// Background helpers
#define BG_BLUE "\033[44m"
#define BG_PURPLE "\033[45m"
#define BG_GREEN "\033[42m"
#define BG_ORANGE "\033[43m"

// File names for persistent storage
#define ADMIN_FILE "admin_records.txt"
#define HR_FILE "hr_records.txt"
#define EMPLOYEE_FILE "employee_records.txt"

//  GLOBAL DISPLAY UTILITIES
void clrscr() { system("cls"); }

void line(char c = '=', int n = 65, const char* color = WHITE) {
    cout << color;
    for (int i = 0; i < n; i++) cout << c;
    cout << RESET << "\n";
}

void center(const string& text, int width = 65, const char* color = WHITE) {
    int pad = (width - (int)text.length()) / 2;
    if (pad < 0) pad = 0;
    cout << color;
    for (int i = 0; i < pad; i++) cout << " ";
    cout << text << RESET << "\n";
}

void pause_msg() {
    cout << CYAN << "\nPress ENTER to continue..." << RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Returns color string based on role
const char* roleColor(const string& role) {
    if (role == "Admin")      return PURPLE;
    if (role == "HR Manager") return GREEN;
    return ORANGE;
}

//  CNIC VALIDATION
//  Valid formats: 1234567890123 (13 digits) OR 12345-6789012-3 (with dashes)
bool validateAndFormatCNIC(const string& input, string& formatted) {
    string digits = "";
    for (char c : input) {
        if (isdigit(c)) digits += c;
        else if (c != '-') return false;  // invalid character
    }
    if (digits.length() != 13) return false;

    // Format: XXXXX-XXXXXXX-X
    formatted = digits.substr(0, 5) + "-" +
                digits.substr(5, 7) + "-" +
                digits.substr(12, 1);
    return true;
}

//  CLASS: TaxCalculator  —  FUNCTOR using operator()
class TaxCalculator {
private:
    double rate1;
    double rate2;
    double threshold;

public:
    TaxCalculator() : rate1(0.05), rate2(0.15), threshold(50000.0) {}
    TaxCalculator(double r1, double r2, double t) : rate1(r1), rate2(r2), threshold(t) {}
    TaxCalculator(const TaxCalculator& o) : rate1(o.rate1), rate2(o.rate2), threshold(o.threshold) {}
    ~TaxCalculator() {}

    double operator()(double salary) const {
        if (salary < threshold) return salary * rate1;
        return salary * rate2;
    }

    double netAfterTax(double gross) const { return gross - (*this)(gross); }

    double getRate1()     const { return rate1;     }
    double getRate2()     const { return rate2;     }
    double getThreshold() const { return threshold; }
};

//  CLASS: MUser  -  Authentication with CNIC
class MUser {
private:
    char* uName;
    char* uPassword;
    string uRole;
    string uCNIC;

public:
    MUser() {
        uName = new char[50]; uPassword = new char[50];
        strcpy(uName, "Unknown"); strcpy(uPassword, "0000");
        uRole = "Employee"; uCNIC = "00000-0000000-0";
    }

    MUser(const char* n, const char* p, const string& r, const string& cnic) {
        uName = new char[strlen(n) + 1];
        uPassword = new char[strlen(p) + 1];
        strcpy(uName, n); strcpy(uPassword, p);
        uRole = r; uCNIC = cnic;
    }

    MUser(const MUser& other) {
        uName = new char[strlen(other.uName) + 1];
        uPassword = new char[strlen(other.uPassword) + 1];
        strcpy(uName, other.uName); strcpy(uPassword, other.uPassword);
        uRole = other.uRole; uCNIC = other.uCNIC;
    }

    MUser& operator=(const MUser& other) {
        if (this != &other) {
            delete[] uName; delete[] uPassword;
            uName = new char[strlen(other.uName) + 1];
            uPassword = new char[strlen(other.uPassword) + 1];
            strcpy(uName, other.uName); strcpy(uPassword, other.uPassword);
            uRole = other.uRole; uCNIC = other.uCNIC;
        }
        return *this;
    }

    ~MUser() { delete[] uName; delete[] uPassword; }

    string getName()     const { return string(uName);     }
    string getPassword() const { return string(uPassword); }
    string getRole()     const { return uRole;             }
    string getCNIC()     const { return uCNIC;             }

    bool signIn(const char* n, const char* p) const {
        return (strcmp(uName, n) == 0 && strcmp(uPassword, p) == 0);
    }

    void showData() const {
        const char* col = roleColor(uRole);
        cout << col << BOLD;
        cout << "  Username : " << uName  << "\n";
        cout << "  Role     : " << uRole  << "\n";
        cout << "  CNIC     : " << uCNIC  << "\n";
        cout << RESET;
    }

    bool operator==(const MUser& other) const {
        return strcmp(uName, other.uName) == 0;
    }
    bool operator!=(const MUser& other) const { return !(*this == other); }

    friend ostream& operator<<(ostream& os, const MUser& u) {
        os << "User[" << u.uName << " | " << u.uRole << " | CNIC: " << u.uCNIC << "]";
        return os;
    }
};

//  FILE I/O UTILITIES
//  Format per line: username|password|role|cnic
const char* getFileForRole(const string& role) {
    if (role == "Admin")      return ADMIN_FILE;
    if (role == "HR Manager") return HR_FILE;
    return EMPLOYEE_FILE;
}

// Append a new user record to the appropriate file
void saveUserToFile(const MUser& u) {
    const char* fname = getFileForRole(u.getRole());
    ofstream f(fname, ios::app);
    if (f.is_open()) {
        f << u.getName() << "|" << u.getPassword() << "|"
          << u.getRole() << "|" << u.getCNIC() << "\n";
        f.close();
    }
}

// Check if username already exists in ANY file
bool usernameExistsInFile(const string& username) {
    const char* files[] = { ADMIN_FILE, HR_FILE, EMPLOYEE_FILE };
    for (int f = 0; f < 3; f++) {
        ifstream file(files[f]);
        if (!file.is_open()) continue;
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string uname;
            getline(ss, uname, '|');
            if (uname == username) { file.close(); return true; }
        }
        file.close();
    }
    return false;
}

// Load all users from all 3 files into userList
// Returns count of users loaded; caller must delete[] each MUser*
int loadAllUsersFromFiles(MUser** userList, int maxUsers) {
    int count = 0;
    const char* files[] = { ADMIN_FILE, HR_FILE, EMPLOYEE_FILE };
    for (int f = 0; f < 3 && count < maxUsers; f++) {
        ifstream file(files[f]);
        if (!file.is_open()) continue;
        string lineStr;
        while (getline(file, lineStr) && count < maxUsers) {
            if (lineStr.empty()) continue;
            stringstream ss(lineStr);
            string uname, upass, urole, ucnic;
            if (getline(ss, uname, '|') &&
                getline(ss, upass, '|') &&
                getline(ss, urole, '|') &&
                getline(ss, ucnic)) {
                userList[count++] = new MUser(uname.c_str(), upass.c_str(), urole, ucnic);
            }
        }
        file.close();
    }
    return count;
}

// Rewrite a single role's file after any modification (e.g., future edit feature)
void rewriteFileForRole(const string& role, MUser** userList, int count) {
    const char* fname = getFileForRole(role);
    ofstream f(fname, ios::trunc);
    for (int i = 0; i < count; i++) {
        if (userList[i]->getRole() == role) {
            f << userList[i]->getName()     << "|"
              << userList[i]->getPassword() << "|"
              << userList[i]->getRole()     << "|"
              << userList[i]->getCNIC()     << "\n";
        }
    }
    f.close();
}

//  CLASS: Department
class Department {
private:
    string deptName;
    string deptHead;
    double budget;
    int headcount;

public:
    Department() : deptName("General"), deptHead("TBD"), budget(0.0), headcount(0) {}
    Department(const string& nm, const string& hd, double bgt)
        : deptName(nm), deptHead(hd), budget(bgt), headcount(0) {}
    Department(const Department& o)
        : deptName(o.deptName), deptHead(o.deptHead), budget(o.budget), headcount(o.headcount) {}
    ~Department() {}

    string getName()     const { return deptName;  }
    string getHead()     const { return deptHead;  }
    double getBudget()   const { return budget;    }
    int getHeadcount()   const { return headcount; }
    void addEmployee()         { headcount++;      }
    void setBudget(double b)   { if (b > 0) budget = b; }

    Department& operator+=(double amount) { budget += amount; return *this; }
    Department& operator-=(double amount) {
        budget = (budget >= amount) ? budget - amount : 0.0;
        return *this;
    }
    bool operator==(const Department& other) const { return deptName == other.deptName; }

    friend ostream& operator<<(ostream& os, const Department& d) {
        os << fixed << setprecision(2)
           << d.deptName << "  (Head: " << d.deptHead
           << " | Budget: " << d.budget
           << " | Staff: " << d.headcount << ")";
        return os;
    }
};

//  CLASS: Employee  —  Abstract Base
class Employee {
protected:
    int empId;
    string empName;
    string empDept;
    string empEmail;
    string empPhone;
    string empCNIC;
    double baseSalary;
    string empType;

public:
    static int totalEmployees;

    Employee() : empId(0), empName("Unknown"), empDept("N/A"),
        empEmail("N/A"), empPhone("N/A"), empCNIC("00000-0000000-0"),
        baseSalary(0.0), empType("None") {
        totalEmployees++;
    }

    Employee(int id, const string& nm, const string& dept,
             const string& em, const string& ph, const string& cnic,
             double sal, const string& type)
        : empId(id), empName(nm), empDept(dept), empEmail(em),
          empPhone(ph), empCNIC(cnic), baseSalary(sal), empType(type) {
        totalEmployees++;
    }

    Employee(const Employee& o)
        : empId(o.empId), empName(o.empName), empDept(o.empDept),
          empEmail(o.empEmail), empPhone(o.empPhone), empCNIC(o.empCNIC),
          baseSalary(o.baseSalary), empType(o.empType) {
        totalEmployees++;
    }

    virtual ~Employee() { totalEmployees--; }

    virtual double calculateSalary() const = 0;
    virtual void displayInfo()       const = 0;
    virtual void printSlip(const TaxCalculator& tc) const = 0;

    int    getId()          const { return empId;       }
    string getName()        const { return empName;     }
    string getDepartment()  const { return empDept;     }
    string getEmail()       const { return empEmail;    }
    string getPhone()       const { return empPhone;    }
    string getCNIC()        const { return empCNIC;     }
    double getBaseSalary()  const { return baseSalary;  }
    string getType()        const { return empType;     }

    void setName(const string& n)  { empName  = n; }
    void setDept(const string& d)  { empDept  = d; }
    void setEmail(const string& e) { empEmail = e; }
    void setPhone(const string& p) { empPhone = p; }
    void setBaseSalary(double s)   {
        if (s > 0) baseSalary = s;
        else cout << RED << "  [!] Salary must be > 0.\n" << RESET;
    }

    bool operator>(const Employee& o) const { return calculateSalary() > o.calculateSalary(); }
    bool operator<(const Employee& o) const { return calculateSalary() < o.calculateSalary(); }
    bool operator==(const Employee& o) const { return empId == o.empId; }

    friend ostream& operator<<(ostream& os, const Employee& e) {
        os << "EMP[" << right << setw(4) << e.empId << "] "
           << left << setw(18) << e.empName
           << setw(13) << e.empType
           << " " << fixed << setprecision(2) << e.baseSalary;
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
    FullTimeEmployee() : Employee(), bonus(0.0), deductions(0.0), leaveDays(0) {}

    FullTimeEmployee(int id, const string& nm, const string& dept,
                     const string& em, const string& ph, const string& cnic,
                     double sal, double bon, double ded, int leave)
        : Employee(id, nm, dept, em, ph, cnic, sal, "Full-Time"),
          bonus(bon), deductions(ded), leaveDays(leave) {}

    FullTimeEmployee(const FullTimeEmployee& o)
        : Employee(o), bonus(o.bonus), deductions(o.deductions), leaveDays(o.leaveDays) {}

    ~FullTimeEmployee() {}

    double calculateSalary() const override {
        double perDay = baseSalary / 30.0;
        return baseSalary + bonus - deductions - (perDay * leaveDays);
    }

    void displayInfo() const override {
        line('-', 65, ORANGE);
        cout << ORANGE << BOLD;
        cout << "  Type         : Full-Time Employee\n";
        cout << "  ID           : " << empId    << "\n";
        cout << "  Name         : " << empName  << "\n";
        cout << "  CNIC         : " << empCNIC  << "\n";
        cout << "  Department   : " << empDept  << "\n";
        cout << "  Email        : " << empEmail << "\n";
        cout << "  Phone        : " << empPhone << "\n";
        cout << fixed << setprecision(2);
        cout << "  Base Salary  :  " << baseSalary        << "\n";
        cout << "  Bonus        :  " << bonus             << "\n";
        cout << "  Deductions   :  " << deductions        << "\n";
        cout << "  Leave Days   : "  << leaveDays         << " day(s)\n";
        cout << "  NET SALARY   :  " << calculateSalary() << "\n";
        cout << RESET;
        line('-', 65, ORANGE);
    }

    void printSlip(const TaxCalculator& tc) const override {
        double gross = calculateSalary();
        double tax   = tc(gross);
        double net   = gross - tax;
        line('=', 65, ORANGE);
        center("- SALARY SLIP | FULL-TIME EMPLOYEE -", 65, ORANGE);
        line('-', 65, ORANGE);
        cout << ORANGE;
        cout << "  Employee     : " << empName << "  (ID: " << empId << ")\n";
        cout << "  CNIC         : " << empCNIC  << "\n";
        cout << "  Department   : " << empDept  << "\n";
        cout << "  Email        : " << empEmail << "\n";
        cout << fixed << setprecision(2);
        cout << "  Gross Pay    :  " << gross << "\n";
        cout << "  Tax Deducted :  " << tax
             << "  (" << (gross < tc.getThreshold() ? tc.getRate1() : tc.getRate2()) * 100
             << "%)\n";
        cout << "  Net Pay      :  " << net << "\n";
        cout << RESET;
        line('=', 65, ORANGE);
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
    PartTimeEmployee() : Employee(), hourlyRate(0.0), hoursWorked(0) {}

    PartTimeEmployee(int id, const string& nm, const string& dept,
                     const string& em, const string& ph, const string& cnic,
                     double rate, int hours)
        : Employee(id, nm, dept, em, ph, cnic, rate * hours, "Part-Time"),
          hourlyRate(rate), hoursWorked(hours) {}

    PartTimeEmployee(const PartTimeEmployee& o)
        : Employee(o), hourlyRate(o.hourlyRate), hoursWorked(o.hoursWorked) {}

    ~PartTimeEmployee() {}

    double calculateSalary() const override { return hourlyRate * hoursWorked; }

    void displayInfo() const override {
        line('-', 65, ORANGE);
        cout << ORANGE << BOLD;
        cout << "  Type         : Part-Time Employee\n";
        cout << "  ID           : " << empId       << "\n";
        cout << "  Name         : " << empName     << "\n";
        cout << "  CNIC         : " << empCNIC     << "\n";
        cout << "  Department   : " << empDept     << "\n";
        cout << "  Email        : " << empEmail    << "\n";
        cout << "  Phone        : " << empPhone    << "\n";
        cout << fixed << setprecision(2);
        cout << "  Hourly Rate  :  " << hourlyRate        << "\n";
        cout << "  Hours Worked : "  << hoursWorked       << " hrs\n";
        cout << "  NET SALARY   :  " << calculateSalary() << "\n";
        cout << RESET;
        line('-', 65, ORANGE);
    }

    void printSlip(const TaxCalculator& tc) const override {
        double gross = calculateSalary();
        double tax   = tc(gross);
        line('=', 65, ORANGE);
        center("- SALARY SLIP | PART-TIME EMPLOYEE -", 65, ORANGE);
        line('-', 65, ORANGE);
        cout << ORANGE;
        cout << "  Employee     : " << empName << "  (ID: " << empId << ")\n";
        cout << "  CNIC         : " << empCNIC  << "\n";
        cout << "  Department   : " << empDept  << "\n";
        cout << fixed << setprecision(2);
        cout << "  Hours x Rate : " << hoursWorked << " x $" << hourlyRate << "\n";
        cout << "  Gross Pay    :  " << gross << "\n";
        cout << "  Tax Deducted :  " << tax   << "\n";
        cout << "  Net Pay      :  " << gross - tax << "\n";
        cout << RESET;
        line('=', 65, ORANGE);
    }
};

//  CLASS: ContractEmployee
class ContractEmployee : public Employee {
private:
    double contractAmount;
    int    contractMonths;
    double taxRate;

public:
    ContractEmployee() : Employee(), contractAmount(0.0), contractMonths(1), taxRate(0.10) {}

    ContractEmployee(int id, const string& nm, const string& dept,
                     const string& em, const string& ph, const string& cnic,
                     double amount, int months, double tax)
        : Employee(id, nm, dept, em, ph, cnic, amount, "Contract"),
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
        line('-', 65, ORANGE);
        cout << ORANGE << BOLD;
        cout << "  Type           : Contract Employee\n";
        cout << "  ID             : " << empId         << "\n";
        cout << "  Name           : " << empName       << "\n";
        cout << "  CNIC           : " << empCNIC       << "\n";
        cout << "  Department     : " << empDept       << "\n";
        cout << "  Email          : " << empEmail      << "\n";
        cout << "  Phone          : " << empPhone      << "\n";
        cout << fixed << setprecision(2);
        cout << "  Contract Amount:  " << contractAmount    << "\n";
        cout << "  Contract Months: "  << contractMonths    << "\n";
        cout << "  Tax Rate       : "  << (taxRate * 100)   << "%\n";
        cout << "  Monthly Net    :  " << calculateSalary() << "\n";
        cout << RESET;
        line('-', 65, ORANGE);
    }

    void printSlip(const TaxCalculator& tc) const override {
        double monthly     = contractAmount / contractMonths;
        double contractTax = monthly * taxRate;
        double sysTax      = tc(monthly);
        line('=', 65, ORANGE);
        center("- SALARY SLIP | CONTRACT EMPLOYEE -", 65, ORANGE);
        line('-', 65, ORANGE);
        cout << ORANGE;
        cout << "  Employee       : " << empName << "  (ID: " << empId << ")\n";
        cout << "  CNIC           : " << empCNIC  << "\n";
        cout << "  Department     : " << empDept  << "\n";
        cout << fixed << setprecision(2);
        cout << "  Monthly Gross  :  " << monthly     << "\n";
        cout << "  Contract Tax   :  " << contractTax << "  (" << (taxRate * 100) << "%)\n";
        cout << "  System Tax     :  " << sysTax      << "  (via TaxCalculator functor)\n";
        cout << "  Monthly Net    :  " << calculateSalary() << "\n";
        cout << RESET;
        line('=', 65, ORANGE);
    }
};

//  CLASS: EmployeeHandle  —  Smart pointer wrapper
class EmployeeHandle {
private:
    Employee* ptr;
public:
    explicit EmployeeHandle(Employee* p = nullptr) : ptr(p) {}
    Employee*  operator->()  const { return ptr;  }
    Employee&  operator*()   const { return *ptr; }
    explicit   operator bool() const { return ptr != nullptr; }
    bool       isValid()     const { return ptr != nullptr; }
    Employee*  get()         const { return ptr; }
};

//  CLASS: PayrollSystem  —  Core manager
class PayrollSystem {
private:
    Employee** empList;
    int count;
    int capacity;
    string companyName;

    void resize() {
        capacity *= 2;
        Employee** tmp = new Employee*[capacity];
        for (int i = 0; i < count; i++) tmp[i] = empList[i];
        delete[] empList;
        empList = tmp;
    }

public:
    PayrollSystem(const string& co = "TechCorp Pvt Ltd", int cap = 10)
        : count(0), capacity(cap), companyName(co) {
        empList = new Employee*[capacity];
    }

    ~PayrollSystem() {
        for (int i = 0; i < count; i++) delete empList[i];
        delete[] empList;
    }

    void addEmployee(Employee* e) {
        if (count >= capacity) resize();
        empList[count++] = e;
        cout << GREEN << "  >> Record added. Total: " << count << RESET << "\n";
    }

    PayrollSystem& operator+=(Employee* e) { addEmployee(e); return *this; }

    Employee*       operator[](int index)       {
        if (index >= 0 && index < count) return empList[index];
        return nullptr;
    }
    const Employee* operator[](int index) const {
        if (index >= 0 && index < count) return empList[index];
        return nullptr;
    }

    int operator()(const string& dept) const {
        int cnt = 0;
        for (int i = 0; i < count; i++)
            if (empList[i]->getDepartment() == dept) cnt++;
        return cnt;
    }

    PayrollSystem& operator--() {
        if (count > 0) delete empList[--count];
        return *this;
    }

    friend ostream& operator<<(ostream& os, const PayrollSystem& ps) {
        os << "PayrollSystem[ " << ps.companyName << " | Staff: " << ps.count << " ]";
        return os;
    }

    PayrollSystem& sortBySalary() {
        for (int i = 0; i < count - 1; i++)
            for (int j = 0; j < count - i - 1; j++)
                if (*empList[j] > *empList[j + 1])
                    swap(empList[j], empList[j + 1]);
        return *this;
    }

    const PayrollSystem& viewAll() const {
        if (!count) { cout << RED << "No records found.\n" << RESET; return *this; }
        for (int i = 0; i < count; i++) empList[i]->displayInfo();
        return *this;
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

    bool updateSalary(int id, double s) {
        Employee* e = searchById(id);
        if (e) { e->setBaseSalary(s); return true; }
        return false;
    }

    bool deleteEmployee(int id) {
        for (int i = 0; i < count; i++) {
            if (empList[i]->getId() == id) {
                delete empList[i];
                for (int j = i; j < count - 1; j++) empList[j] = empList[j + 1];
                count--;
                return true;
            }
        }
        return false;
    }

    void printAllSlips(const TaxCalculator& tc) const {
        if (!count) { cout << RED << "No records.\n" << RESET; return; }
        for (int i = 0; i < count; i++) empList[i]->printSlip(tc);
    }

    double totalPayroll() const {
        double t = 0;
        for (int i = 0; i < count; i++) t += empList[i]->calculateSalary();
        return t;
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

    void summaryReport(const TaxCalculator& tc) const {
        clrscr();
        line('=', 65, CYAN); center("PAYROLL SUMMARY REPORT", 65, CYAN);
        center(companyName, 65, WHITE); line('=', 65, CYAN);

        if (!count) { cout << RED << "No records found.\n" << RESET; return; }

        cout << CYAN;
        cout << "  " << left << setw(6)  << "ID" << setw(20) << "Name"
             << setw(14) << "Type" << setw(13) << "Gross" << "Tax\n";
        cout << RESET;
        line('-', 65, CYAN);

        for (int i = 0; i < count; i++) {
            double gross = empList[i]->calculateSalary();
            double tax   = tc(gross);
            cout << ORANGE;
            cout << "  " << left << setw(6)  << empList[i]->getId()
                 << setw(20) << empList[i]->getName()
                 << setw(14) << empList[i]->getType()
                 << " " << fixed << setprecision(2) << setw(12) << gross << " " << tax << "\n";
            cout << RESET;
        }

        line('-', 65, CYAN);
        double total = totalPayroll();
        cout << fixed << setprecision(2) << CYAN;
        cout << "Total Staff     : " << count                    << "\n";
        cout << "Total Employees : " << Employee::totalEmployees  << "\n";
        cout << "Gross Payroll   : " << total                    << "\n";
        cout << "Total Tax       : " << tc(total)                << "\n";
        cout << "Net Payroll     : " << tc.netAfterTax(total)    << "\n";
        Employee* hp = highestPaid();
        Employee* lp = lowestPaid();
        if (hp) cout << "Highest Paid    : " << hp->getName() << " (" << hp->calculateSalary() << ")\n";
        if (lp) cout << "Lowest Paid     : " << lp->getName() << " (" << lp->calculateSalary() << ")\n";
        cout << RESET;
        line('=', 65, CYAN);
    }

    int    getCount()       const { return count;       }
    string getCompanyName() const { return companyName; }
};

//  SHALLOW vs DEEP COPY DEMO CLASSES
class ShallowDemo {
public:
    char* data;
    ShallowDemo(const char* d) { data = new char[strlen(d)+1]; strcpy(data,d); }
    ShallowDemo(const ShallowDemo& o) {
        data = o.data;
        cout << PURPLE << "  [Shallow] shared address: " << (void*)data << RESET << "\n";
    }
    void show(const string& lbl) {
        cout << "  " << left << setw(16) << lbl << " = " << data << "  @" << (void*)data << "\n";
    }
    ~ShallowDemo() {}
};

class DeepDemo {
public:
    char* data;
    DeepDemo(const char* d) { data = new char[strlen(d)+1]; strcpy(data,d); }
    DeepDemo(const DeepDemo& o) {
        data = new char[strlen(o.data)+1];
        strcpy(data,o.data);
        cout << GREEN << "  [Deep]    new address:    " << (void*)data << RESET << "\n";
    }
    void show(const string& lbl) {
        cout << "  " << left << setw(16) << lbl << " = " << data << "  @" << (void*)data << "\n";
    }
    ~DeepDemo() { delete[] data; }
};

//  AUTH GLOBALS
const int MAX_USERS = 100;
MUser* userList[MAX_USERS];
int    userCount = 0;

// Load persisted users from files at startup
void loadUsersFromFiles() {
    userCount = loadAllUsersFromFiles(userList, MAX_USERS);
    if (userCount > 0)
        cout << GREEN << "  [OK] Loaded " << userCount << " user(s) from storage.\n" << RESET;
}

//  SIGN UP
bool signUpUser() {
    clrscr();
    line('=', 65, BLUE); center("S I G N  U P - Create Account", 65, BLUE); line('=', 65, BLUE);

    if (userCount >= MAX_USERS) {
        cout << RED << "[!] Max user limit reached.\n" << RESET;
        pause_msg(); return false;
    }

    char name[50], pass[50], confirm[50];
    string role, rawCNIC, formattedCNIC;

    cout << BLUE << "\nUsername        : " << RESET; cin >> name;

    // Check memory AND files for duplicates
    for (int i = 0; i < userCount; i++) {
        if (userList[i]->getName() == string(name)) {
            cout << RED << "\n[!] Username already taken!\n" << RESET;
            pause_msg(); return false;
        }
    }
    if (usernameExistsInFile(string(name))) {
        cout << RED << "\n[!] Username already taken!\n" << RESET;
        pause_msg(); return false;
    }

    cout << BLUE << "Password        : " << RESET; cin >> pass;
    cout << BLUE << "Confirm Password: " << RESET; cin >> confirm;

    if (strcmp(pass, confirm) != 0) {
        cout << RED << "\n[!] Passwords do not match!\n" << RESET;
        pause_msg(); return false;
    }

    // CNIC input and validation
    cout << BLUE << "CNIC            : " << RESET;
    cin >> rawCNIC;
    while (!validateAndFormatCNIC(rawCNIC, formattedCNIC)) {
        cout << RED << "[!] Invalid CNIC. Must be exactly 13 digits (dashes optional): " << RESET;
        cin >> rawCNIC;
    }

    cout << BLUE << "\nRole: [1] Admin  [2] HR Manager  [3] Employee\nChoice: " << RESET;
    int rc; cin >> rc;
    switch (rc) {
        case 1:  role = "Admin";      break;
        case 2:  role = "HR Manager"; break;
        default: role = "Employee";
    }

    const char* col = roleColor(role);
    userList[userCount] = new MUser(name, pass, role, formattedCNIC);

    // Persist to the appropriate file
    saveUserToFile(*userList[userCount]);
    userCount++;

    cout << "\n";
    line('-', 65, col);
    center("ACCOUNT CREATED SUCCESSFULLY!", 65, col);
    center("Data saved to " + string(getFileForRole(role)), 65, WHITE);
    line('-', 65, col);
    cout << col << "Record : " << *userList[userCount - 1] << "\n" << RESET;
    pause_msg();
    return true;
}

//  SIGN IN
MUser* signInUser() {
    clrscr();
    line('=', 65, BLUE); center("S I G N  I N - Login", 65, BLUE); line('=', 65, BLUE);

    if (userCount == 0) {
        cout << RED << "\n[!] No accounts found. Please sign up first.\n" << RESET;
        pause_msg(); return nullptr;
    }

    char name[50], pass[50];
    int attempts = 3;

    while (attempts > 0) {
        cout << BLUE << "\nUsername : " << RESET; cin >> name;
        cout << BLUE << "Password : " << RESET; cin >> pass;

        for (int i = 0; i < userCount; i++) {
            if (userList[i]->signIn(name, pass)) {
                const char* col = roleColor(userList[i]->getRole());
                cout << "\n";
                line('-', 65, col);
                center("LOGIN SUCCESSFUL!", 65, col);
                line('-', 65, col);
                cout << col << "Welcome, " << RESET << "\n";
                userList[i]->showData();
                pause_msg();
                return userList[i];
            }
        }

        attempts--;
        if (attempts > 0)
            cout << RED << "\n[!] Wrong credentials. Attempts left: " << attempts << "\n" << RESET;
        else {
            cout << RED << "\n[!] Too many failed attempts. Access denied.\n" << RESET;
            pause_msg();
        }
    }
    return nullptr;
}

//  EMPLOYEE INPUT HELPERS
int nextId = 1001;

string inputCNIC() {
    string rawCNIC, formattedCNIC;
    cout << ORANGE << "CNIC (13 digits, dashes optional): " << RESET;
    cin >> rawCNIC;
    while (!validateAndFormatCNIC(rawCNIC, formattedCNIC)) {
        cout << RED << "[!] Invalid CNIC. Try again: " << RESET;
        cin >> rawCNIC;
    }
    return formattedCNIC;
}

void addFullTime(PayrollSystem& sys) {
    clrscr();
    line('=', 65, ORANGE); center("ADD  FULL-TIME  EMPLOYEE", 65, ORANGE); line('=', 65, ORANGE);
    string name, dept, email, phone, cnic;
    double sal, bon, ded; int leave;

    cout << ORANGE << "\nAuto ID          : " << nextId << "\n" << RESET;
    cout << ORANGE << "Name             : " << RESET; cin >> name;
    cout << ORANGE << "Department       : " << RESET; cin >> dept;
    cout << ORANGE << "Email            : " << RESET; cin >> email;
    cout << ORANGE << "Phone            : " << RESET; cin >> phone;
    cnic = inputCNIC();
    cout << ORANGE << "Base Salary      : " << RESET; cin >> sal;
    while (sal <= 0) { cout << RED << "[!] Must be > 0: " << RESET; cin >> sal; }
    cout << ORANGE << "Bonus            : " << RESET; cin >> bon;
    cout << ORANGE << "Deductions       : " << RESET; cin >> ded;
    cout << ORANGE << "Leave Days (0-10): " << RESET; cin >> leave;
    while (leave < 0 || leave > 10) { cout << RED << "[!] 0-10 only: " << RESET; cin >> leave; }

    sys += new FullTimeEmployee(nextId++, name, dept, email, phone, cnic, sal, bon, ded, leave);
    int last = sys.getCount() - 1;
    if (sys[last]) { cout << "\nVerified via operator[]: " << ORANGE; cout << *sys[last] << RESET << "\n"; }
    pause_msg();
}

void addPartTime(PayrollSystem& sys) {
    clrscr();
    line('=', 65, ORANGE); center("ADD  PART-TIME  EMPLOYEE", 65, ORANGE); line('=', 65, ORANGE);
    string name, dept, email, phone, cnic;
    double rate; int hours;

    cout << ORANGE << "\nAuto ID         : " << nextId << "\n" << RESET;
    cout << ORANGE << "Name            : " << RESET; cin >> name;
    cout << ORANGE << "Department      : " << RESET; cin >> dept;
    cout << ORANGE << "Email           : " << RESET; cin >> email;
    cout << ORANGE << "Phone           : " << RESET; cin >> phone;
    cnic = inputCNIC();
    cout << ORANGE << "Hourly Rate     : " << RESET; cin >> rate;
    while (rate <= 0) { cout << RED << "[!] Must be > 0: " << RESET; cin >> rate; }
    cout << ORANGE << "Hours Worked    : " << RESET; cin >> hours;
    while (hours <= 0 || hours > 744) { cout << RED << "[!] 1-744: " << RESET; cin >> hours; }

    sys += new PartTimeEmployee(nextId++, name, dept, email, phone, cnic, rate, hours);
    int last = sys.getCount() - 1;
    if (sys[last]) { cout << "\nVerified: " << ORANGE << *sys[last] << RESET << "\n"; }
    pause_msg();
}

void addContract(PayrollSystem& sys) {
    clrscr();
    line('=', 65, ORANGE); center("ADD  CONTRACT  EMPLOYEE", 65, ORANGE); line('=', 65, ORANGE);
    string name, dept, email, phone, cnic;
    double amount, tax; int months;

    cout << ORANGE << "\nAuto ID             : " << nextId << "\n" << RESET;
    cout << ORANGE << "Name                : " << RESET; cin >> name;
    cout << ORANGE << "Department          : " << RESET; cin >> dept;
    cout << ORANGE << "Email               : " << RESET; cin >> email;
    cout << ORANGE << "Phone               : " << RESET; cin >> phone;
    cnic = inputCNIC();
    cout << ORANGE << "Contract Amount     : " << RESET; cin >> amount;
    while (amount <= 0) { cout << RED << "[!] Must be > 0: " << RESET; cin >> amount; }
    cout << ORANGE << "Contract Months     : " << RESET; cin >> months;
    while (months <= 0 || months > 60) { cout << RED << "[!] 1-60: " << RESET; cin >> months; }
    cout << ORANGE << "Tax Rate (0.0-1.0)  : " << RESET; cin >> tax;
    while (tax < 0.0 || tax > 1.0) { cout << RED << "[!] 0.0-1.0: " << RESET; cin >> tax; }

    sys += new ContractEmployee(nextId++, name, dept, email, phone, cnic, amount, months, tax);
    int last = sys.getCount() - 1;
    if (sys[last]) { cout << "\nVerified: " << ORANGE << *sys[last] << RESET << "\n"; }
    pause_msg();
}

void searchMenu(const PayrollSystem& sys) {
    clrscr();
    line('=', 65, CYAN); center("SEARCH  EMPLOYEE", 65, CYAN); line('=', 65, CYAN);
    cout << "\n1. By ID   2. By Name\n  Choice: ";
    int ch; cin >> ch;
    Employee* res = nullptr;
    if (ch == 1) {
        int id; cout << "Employee ID   : "; cin >> id;
        res = sys.searchById(id);
    } else {
        string nm; cout << "Employee Name : "; cin >> nm;
        res = sys.searchByName(nm);
    }
    cout << "\n";
    if (res) { cout << GREEN << "Record Found:\n" << RESET; res->displayInfo(); }
    else cout << RED << "[!] Employee not found.\n" << RESET;
    pause_msg();
}

//  ROLE-BASED ACCESS CHECK
bool hasAccess(const string& role, const string& feature) {
    // Admin: full access
    if (role == "Admin") return true;

    // HR Manager: can manage employees and view reports, cannot delete or view demos
    if (role == "HR Manager") {
        if (feature == "delete")       return false;
        if (feature == "demo")         return false;
        return true;
    }

    // Employee (logged-in): can only view their own slip and summary
    if (role == "Employee") {
        if (feature == "view_summary") return true;
        if (feature == "view_slip")    return true;
        return false;
    }
    return false;
}

void accessDenied() {
    cout << RED << BOLD << "\n[ACCESS DENIED] Your role does not permit this action.\n" << RESET;
    pause_msg();
}

//  DEMO FUNCTIONS
void demoOperators(PayrollSystem& sys, const TaxCalculator& tc) {
    clrscr();
    line('=', 65, PURPLE); center("OPERATOR OVERLOADING DEMO", 65, PURPLE); line('=', 65, PURPLE);

    cout << PURPLE << "\n-- FRIEND OPERATORS --\n" << RESET;
    line('-', 65, PURPLE);
    cout << "\n1. friend operator<< (PayrollSystem):\n";
    cout << "     " << CYAN << sys << RESET << "\n";

    cout << "\n2. friend operator<< (Employee) via operator[]:\n";
    if (sys.getCount() > 0 && sys[0])
        cout << "     " << ORANGE << *sys[0] << RESET << "\n";
    else cout << RED << "     (add employees first)\n" << RESET;

    cout << PURPLE << "\n-- MEMBER OPERATORS --\n" << RESET;
    line('-', 65, PURPLE);

    cout << "\n3. operator[] MEMBER:\n";
    if (sys.getCount() > 0 && sys[0])
        cout << "     sys[0]->getName() = " << ORANGE << sys[0]->getName() << RESET << "\n";
    else cout << RED << "     (no employees yet)\n" << RESET;

    cout << "\n4. operator() MEMBER functor on PayrollSystem:\n";
    cout << "     sys(\"Engineering\") = " << ORANGE << sys("Engineering") << RESET << " employee(s)\n";
    cout << "     sys(\"HR\")          = " << ORANGE << sys("HR")          << RESET << " employee(s)\n";

    cout << "\n5. operator() MEMBER functor on TaxCalculator:\n";
    cout << fixed << setprecision(2);
    cout << "     tc(30000) = " << GREEN << tc(30000) << RESET << " (below threshold)\n";
    cout << "     tc(80000) = " << RED   << tc(80000) << RESET << " (above threshold)\n";
    cout << "     tc.netAfterTax(80000) = " << CYAN << tc.netAfterTax(80000) << RESET << "\n";

    cout << "\n6. operator-> MEMBER via EmployeeHandle:\n";
    if (sys.getCount() > 0) {
        EmployeeHandle handle(sys[0]);
        if (handle) {
            cout << "     handle->getName()   = " << ORANGE << handle->getName() << RESET << "\n";
            cout << "     handle->getType()   = " << ORANGE << handle->getType() << RESET << "\n";
            cout << "     handle->calcSalary()= " << ORANGE << handle->calculateSalary() << RESET << "\n";
        }
    } else cout << RED << "     (add employees first)\n" << RESET;

    cout << "\n7. operator> and < MEMBER (salary comparison):\n";
    if (sys.getCount() >= 2) {
        cout << "     sys[0] > sys[1] : " << (*sys[0] > *sys[1] ? GREEN "TRUE" : RED "FALSE") << RESET << "\n";
        cout << "     sys[0] < sys[1] : " << (*sys[0] < *sys[1] ? GREEN "TRUE" : RED "FALSE") << RESET << "\n";
    } else cout << RED << "     (need 2+ employees)\n" << RESET;

    cout << PURPLE << "\n-- CASCADED OPERATORS --\n" << RESET;
    line('-', 65, PURPLE);

    cout << "\n8. CASCADED operator+= on Department:\n";
    Department hr("HR", "Maham", 100000.0);
    hr += 5000.0; hr += 3000.0; hr -= 1500.0;
    cout << "     After += 5000, += 3000, -= 1500:\n";
    cout << "     " << CYAN << hr << RESET << "\n";

    cout << "\n9. CASCADED operator<< :\n";
    cout << "     " << ORANGE << *sys[0] << RESET << " ... system: " << CYAN << sys << RESET << "\n";

    cout << "\n10. CASCADED sortBySalary().viewAll():\n";
    sys.sortBySalary().viewAll();

    pause_msg();
}

void demoCopyConstructor() {
    clrscr();
    line('=', 65, PURPLE); center("COPY CONSTRUCTOR DEMO (MUser — Deep Copy)", 65, PURPLE); line('=', 65, PURPLE);

    cout << PURPLE << "\nStep 1: Allocate with 'new' - parameterized constructor\n" << RESET;
    MUser* orig = new MUser("DemoAdmin", "pass123", "Admin", "12345-6789012-3");

    cout << PURPLE << "\nStep 2: Arrow operator (->) - calls method through pointer\n" << RESET;
    orig->showData();

    cout << PURPLE << "\nStep 3: Deep Copy Constructor - MUser(*orig)\n" << RESET;
    MUser* cpy = new MUser(*orig);

    cout << PURPLE << "\nStep 4: Dot operator - dereference then use dot\n" << RESET;
    (*cpy).showData();

    cout << PURPLE << "\nStep 5: operator== (MEMBER) test:\n" << RESET;
    cout << "orig == cpy : " << (*orig == *cpy ? GREEN "TRUE" : RED "FALSE") << RESET << "\n";
    cout << "orig != cpy : " << (*orig != *cpy ? GREEN "TRUE" : RED "FALSE") << RESET << "\n";

    cout << PURPLE << "\nStep 6: friend operator<< CASCADED:\n" << RESET;
    cout << "orig: " << PURPLE << *orig << RESET << "\n";
    cout << "cpy : " << PURPLE << *cpy  << RESET << "\n";

    cout << PURPLE << "\nStep 7: delete orig - destructor fires for original only\n" << RESET;
    delete orig;

    cout << GREEN << "\nStep 8: cpy still works - deep copy is independent!\n" << RESET;
    cpy->showData();

    cout << PURPLE << "\nStep 9: delete cpy\n" << RESET;
    delete cpy;

    pause_msg();
}

void demoShallowDeep() {
    clrscr();
    line('=', 65, PURPLE); center("SHALLOW vs DEEP COPY DEMO", 65, PURPLE); line('=', 65, PURPLE);

    cout << RED << "\n-- SHALLOW COPY -- both point to SAME memory address --\n\n" << RESET;
    ShallowDemo s1("PayrollData");
    s1.show("s1 (original)");
    ShallowDemo s2(s1);
    s2.show("s2 (copy)   ");
    cout << "\nModifying s1 -> strcpy(s1.data, \"HACKED!\")\n";
    strcpy(s1.data, "HACKED!");
    s1.show("s1 after    ");
    s2.show("s2 after    ");
    cout << RED << "[!!] s2 was ALSO changed - DANGEROUS!\n" << RESET;

    cout << GREEN << "\n-- DEEP COPY -- each has its OWN independent address --\n\n" << RESET;
    DeepDemo d1("PayrollData");
    d1.show("d1 (original)");
    DeepDemo d2(d1);
    d2.show("d2 (copy)   ");
    cout << "\nModifying d1 -> strcpy(d1.data, \"CHANGED!\")\n";
    strcpy(d1.data, "CHANGED!");
    d1.show("d1 after    ");
    d2.show("d2 after    ");
    cout << GREEN << "[OK] d2 is UNAFFECTED - SAFE!\n" << RESET;
    pause_msg();
}

//  MAIN MENU — role-based visibility
void showMainMenu(const string& user, const string& role) {
    const char* col = roleColor(role);
    line('=', 65, col);
    center("TECHCORP HR & PAYROLL MANAGEMENT SYSTEM", 65, BLUE);
    line('-', 65, col);
    center("Logged in: " + user + "  [" + role + "]", 65, col);
    line('=', 65, col);

    if (role == "Admin" || role == "HR Manager") {
        cout << col << "\n EMPLOYEE MANAGEMENT\n" << RESET;
        line('-', 65, col);
        cout << col << "1.  Add Full-Time Employee\n";
        cout << "2.  Add Part-Time Employee\n";
        cout << "3.  Add Contract Employee\n";
        cout << "4.  View All Employees\n";
        cout << "5.  Search Employee\n";
        cout << "6.  Update Employee Salary\n";
        if (role == "Admin") cout << "7.  Delete Employee\n";
        cout << RESET;

        cout << col << "\n PAYROLL & REPORTS\n" << RESET;
        line('-', 65, col);
        cout << col;
        cout << "8.  Payroll Summary Report\n";
        cout << "9.  Print All Salary Slips\n";
        cout << "10. Print Slip by Employee ID\n";
        cout << "11. Sort by Salary (Cascaded Call)\n";
        cout << RESET;

        if (role == "Admin") {
            cout << col << "\n OOP DEMONSTRATIONS\n" << RESET;
            line('-', 65, col);
            cout << col;
            cout << "12. Operator Overloading Demo\n";
            cout << "13. Copy Constructor Demo\n";
            cout << "14. Shallow vs Deep Copy Demo\n";
            cout << RESET;
        }
    } else {
        // Employee: limited menu
        cout << ORANGE << "\n YOUR PAYROLL\n" << RESET;
        line('-', 65, ORANGE);
        cout << ORANGE;
        cout << "8.  Payroll Summary Report\n";
        cout << "10. My Salary Slip\n";
        cout << RESET;
    }

    cout << col << "\n0.  Logout\n" << RESET;
    line('=', 65, col);
    cout << col << "Choice: " << RESET;
}

//  BANNER
void banner() {
    clrscr();
    cout << "\n";
    line('*', 65, BLUE);
    cout << BLUE << "* " << string(61, ' ') << " *\n" << RESET;
    center("*                   PAYROLL MANAGEMENT SYSTEM                   *", 65, BLUE);
    center("*                 Object-Oriented Programming C++               *", 65, BLUE);
    cout << BLUE << "* " << string(61, ' ') << " *\n" << RESET;
    line('*', 65, BLUE);
    cout << "\n";
    // Show file status
    cout << CYAN << "Data Files: "
         << ADMIN_FILE << " | " << HR_FILE << "\n| " << EMPLOYEE_FILE << "\n"
         << RESET;
}

//  MAIN
int main() {
    PayrollSystem* payroll = new PayrollSystem("UET,Lahore(ksk)");
    TaxCalculator taxCalc(0.05, 0.15, 50000.0);

    // Load persisted users from files
    loadUsersFromFiles();

    bool running = true;
    while (running) {
        banner();
        line('-', 65, BLUE);
        center("W E L C O M E", 65, BLUE);
        line('-', 65, BLUE);
        cout << BLUE << "[1] Sign Up   [2] Sign In   [0] Exit\n" << RESET;
        line('-', 65, BLUE);
        cout << BLUE << "Choice: " << RESET;
        int sc; cin >> sc;

        if (sc == 0) {
            running = false;

        } else if (sc == 1) {
            signUpUser();

        } else if (sc == 2) {
            MUser* cu = signInUser();

            if (cu != nullptr) {
                bool loggedIn = true;
                string role = cu->getRole();

                while (loggedIn) {
                    clrscr();
                    showMainMenu(cu->getName(), role);
                    int ch; cin >> ch;

                    switch (ch) {
                        case 1:
                            if (hasAccess(role, "add")) addFullTime(*payroll);
                            else accessDenied();
                            break;
                        case 2:
                            if (hasAccess(role, "add")) addPartTime(*payroll);
                            else accessDenied();
                            break;
                        case 3:
                            if (hasAccess(role, "add")) addContract(*payroll);
                            else accessDenied();
                            break;
                        case 4:
                            if (hasAccess(role, "view")) {
                                clrscr();
                                line('=', 65, CYAN); center("ALL EMPLOYEE RECORDS", 65, CYAN); line('=', 65, CYAN);
                                payroll->viewAll();
                                pause_msg();
                            } else accessDenied();
                            break;
                        case 5:
                            if (hasAccess(role, "search")) searchMenu(*payroll);
                            else accessDenied();
                            break;
                        case 6: {
                            if (!hasAccess(role, "update")) { accessDenied(); break; }
                            clrscr();
                            line('=', 65, CYAN); center("UPDATE SALARY", 65, CYAN); line('=', 65, CYAN);
                            int id; double ns;
                            cout << CYAN << "\nEmployee ID       : " << RESET; cin >> id;
                            cout << CYAN << "New Base Salary  : " << RESET; cin >> ns;
                            cout << (payroll->updateSalary(id, ns)
                                     ? (string(GREEN) + "\n[OK] Salary updated!\n" + RESET)
                                     : (string(RED)   + "\n[!] Employee not found.\n" + RESET));
                            pause_msg(); break;
                        }
                        case 7: {
                            if (!hasAccess(role, "delete")) { accessDenied(); break; }
                            clrscr();
                            line('=', 65, RED); center("DELETE EMPLOYEE", 65, RED); line('=', 65, RED);
                            int id;
                            cout << RED << "\nEmployee ID: " << RESET; cin >> id;
                            cout << (payroll->deleteEmployee(id)
                                     ? (string(GREEN) + "\n[OK] Employee deleted!\n" + RESET)
                                     : (string(RED)   + "\n[!] Not found.\n" + RESET));
                            pause_msg(); break;
                        }
                        case 8:
                            if (hasAccess(role, "view_summary")) {
                                payroll->summaryReport(taxCalc);
                                pause_msg();
                            } else accessDenied();
                            break;
                        case 9:
                            if (hasAccess(role, "slips")) {
                                clrscr();
                                payroll->printAllSlips(taxCalc);
                                pause_msg();
                            } else accessDenied();
                            break;
                        case 10: {
                            clrscr();
                            if (role == "Employee") {
                                // Employee can ONLY see their own slip — matched by their login username
                                line('=', 65, ORANGE);
                                center("MY SALARY SLIP", 65, ORANGE);
                                line('=', 65, ORANGE);
                                string loggedInName = cu->getName();
                                cout << ORANGE << "\n  Fetching slip for: " << loggedInName << "\n" << RESET;
                                Employee* e = payroll->searchByName(loggedInName);
                                if (e) {
                                    e->printSlip(taxCalc);
                                } else {
                                    cout << RED << "\n  [!] No employee record found matching your username.\n";
                                    cout << "      Please ask your HR/Admin to register your employee record\n";
                                    cout << "      with the name: " << ORANGE << loggedInName << RED << "\n" << RESET;
                                }
                            } else {
                                // Admin / HR Manager: search by ID
                                line('=', 65, ORANGE);
                                center("SALARY SLIP BY ID", 65, ORANGE);
                                line('=', 65, ORANGE);
                                int id;
                                cout << ORANGE << "\nEmployee ID: " << RESET; cin >> id;
                                Employee* e = payroll->searchById(id);
                                if (e) e->printSlip(taxCalc);
                                else   cout << RED << "\n[!] Not found.\n" << RESET;
                            }
                            pause_msg(); break;
                        }
                        case 11:
                            if (hasAccess(role, "sort")) {
                                clrscr();
                                line('=', 65, CYAN); center("SORTED BY SALARY (ASCENDING)", 65, CYAN); line('=', 65, CYAN);
                                payroll->sortBySalary().viewAll();
                                cout << "\nSystem: " << CYAN << *payroll << RESET << "\n";
                                pause_msg();
                            } else accessDenied();
                            break;
                        case 12:
                            if (hasAccess(role, "demo")) demoOperators(*payroll, taxCalc);
                            else accessDenied();
                            break;
                        case 13:
                            if (hasAccess(role, "demo")) demoCopyConstructor();
                            else accessDenied();
                            break;
                        case 14:
                            if (hasAccess(role, "demo")) demoShallowDeep();
                            else accessDenied();
                            break;
                        case 0:
                            loggedIn = false;
                            cout << roleColor(role) << "\nLogging out...\n" << RESET;
                            pause_msg(); break;
                        default:
                            cout << RED << "\n[!] Invalid option. Try again.\n" << RESET;
                            pause_msg();
                    }
                }
            }
        } else {
            cout << RED << "\n[!] Invalid choice.\n" << RESET;
            pause_msg();
        }
    }

    // Cleanup
    cout << "\n";
    line('=', 65, BLUE); center("SHUTDOWN - Freeing All Memory", 65, BLUE); line('=', 65, BLUE);
    cout << CYAN << "\nDeleting " << userCount << " user object(s).\n" << RESET;
    for (int i = 0; i < userCount; i++) delete userList[i];

    cout << CYAN << "\nDeleting PayrollSystem.\n" << RESET;
    delete payroll;

    cout << "\n";
    line('*', 65, BLUE);
    center("Thank you for using TechCorp Payroll System!", 65, BLUE);
    center("All dynamically allocated memory freed.", 65, BLUE);
    line('*', 65, BLUE);
    cout << RESET;
    return 0;
}
