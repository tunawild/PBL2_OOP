#pragma once
#include <string>
#include <vector>

using namespace std;

enum UserRole {
    ROLE_ADMIN,
    ROLE_CUSTOMER
};

class User {
protected:
    string id;
    string username;
    string password;
    UserRole role;
public:
    User(string id, string user, string pass, UserRole r);
    virtual ~User() = default;

    string getId() const;
    string getUsername() const;
    string getPassword() const;
    bool checkPassword(const string& pass) const;
    UserRole getRole() const;
};

class Admin : public User {
public:
    Admin(string id, string user, string pass);
};

class Customer : public User {
private:
    string name;
    string phone;
    string licensePlate;
    string carModel;
    float batteryCapacity;
    float balance;
    bool isLocked;
public:
    Customer(string id, string user, string pass, string name, string phone, string license, string carModel, float capacity, float bal, bool locked = false);
    
    string getName() const;
    string getPhone() const;
    string getLicensePlate() const;
    string getCarModel() const;
    float getBatteryCapacity() const;
    float getBalance() const;
    bool getIsLocked() const;
    
    void setLocked(bool l);
    void deductBalance(float amount);
    void addBalance(float amount);
};

enum PortType { AC_STANDARD, DC_FAST, DC_SUPERCHARGE };
enum PortStatus { AVAILABLE, OCCUPIED, MAINTENANCE };

class ChargingPort {
public:
    string portId;
    PortType type;
    float powerKW;
    PortStatus status;
    float pricePerKWh;
    string currentSessionId; 

    ChargingPort(string id, PortType type, float pwr, float price, PortStatus st = AVAILABLE, string sessId = "");
};

class ChargingStation {
public:
    string stationId;
    string name;
    string address;
    vector<ChargingPort> ports;

    ChargingStation(string id, string n, string addr);
    void addPort(ChargingPort port);
};

class ChargingSession {
public:
    string sessionId;
    string customerId;
    string stationId;
    string portId;
    long long startTime;
    long long endTime;
    float energyKWh;
    float cost;
    bool isActive;

    ChargingSession(string sId, string cId, string stId, string pId, long long start);
    void endSession(long long endT, float energy, float totalCost);
};

class Invoice {
public:
    string invoiceId;
    string sessionId;
    string customerId;
    float totalAmount;
    long long date;

    Invoice(string id, string sId, string cId, float amount, long long d);
};
