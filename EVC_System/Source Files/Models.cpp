#include "Models.h"

User::User(string id, string user, string pass, UserRole r) : id(id), username(user), password(pass), role(r) {}
string User::getId() const { return id; }
string User::getUsername() const { return username; }
string User::getPassword() const { return password; }
bool User::checkPassword(const string& pass) const { return password == pass; }
UserRole User::getRole() const { return role; }

Admin::Admin(string id, string user, string pass) : User(id, user, pass, ROLE_ADMIN) {}

Customer::Customer(string id, string user, string pass, string n, string p, string lp, string cm, float cap, float bal, bool locked)
    : User(id, user, pass, ROLE_CUSTOMER), name(n), phone(p), licensePlate(lp), carModel(cm), batteryCapacity(cap), balance(bal), isLocked(locked) {}

string Customer::getName() const { return name; }
string Customer::getPhone() const { return phone; }
string Customer::getLicensePlate() const { return licensePlate; }
string Customer::getCarModel() const { return carModel; }
float Customer::getBatteryCapacity() const { return batteryCapacity; }
float Customer::getBalance() const { return balance; }
bool Customer::getIsLocked() const { return isLocked; }

void Customer::setLocked(bool l) { isLocked = l; }
void Customer::deductBalance(float amount) { if(balance >= amount) balance -= amount; }
void Customer::addBalance(float amount) { balance += amount; }

ChargingPort::ChargingPort(string id, PortType t, float pwr, float price, PortStatus st, string sessId)
    : portId(id), type(t), powerKW(pwr), status(st), pricePerKWh(price), currentSessionId(sessId) {}

ChargingStation::ChargingStation(string id, string n, string addr) : stationId(id), name(n), address(addr) {}
void ChargingStation::addPort(ChargingPort port) { ports.push_back(port); }

ChargingSession::ChargingSession(string sId, string cId, string stId, string pId, long long start)
    : sessionId(sId), customerId(cId), stationId(stId), portId(pId), startTime(start), endTime(0), energyKWh(0), cost(0), isActive(true) {}

void ChargingSession::endSession(long long endT, float energy, float totalCost) {
    endTime = endT;
    energyKWh = energy;
    cost = totalCost;
    isActive = false;
}

Invoice::Invoice(string id, string sId, string cId, float amount, long long d)
    : invoiceId(id), sessionId(sId), customerId(cId), totalAmount(amount), date(d) {}
