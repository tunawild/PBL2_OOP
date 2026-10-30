#include "Models.h"

// =====================================================================
// CAI DAT LOP USER
// =====================================================================

User::User(const string& id, const string& user, const string& pass, UserRole r)
    : id(id), username(user), password(pass), role(r) {}

string User::getId() const { return id; }
string User::getUsername() const { return username; }
string User::getPassword() const { return password; }
UserRole User::getRole() const { return role; }

bool User::checkPassword(const string& pass) const {
    return password == pass;
}

// =====================================================================
// CAI DAT LOP ADMIN
// =====================================================================

Admin::Admin(const string& id, const string& user, const string& pass)
    : User(id, user, pass, ROLE_ADMIN) {}

string Admin::getRoleName() const {
    return "Admin";
}

// =====================================================================
// CAI DAT LOP CUSTOMER
// =====================================================================

Customer::Customer(const string& id, const string& user, const string& pass,
                   const string& n, const string& p, const string& lp,
                   const string& cm, float cap, float bal)
    : User(id, user, pass, ROLE_CUSTOMER),
      name(n), phone(p), licensePlate(lp),
      carModel(cm), batteryCapacity(cap), balance(bal) {}

string Customer::getName() const { return name; }
string Customer::getPhone() const { return phone; }
string Customer::getLicensePlate() const { return licensePlate; }
string Customer::getCarModel() const { return carModel; }
float Customer::getBatteryCapacity() const { return batteryCapacity; }
float Customer::getBalance() const { return balance; }

void Customer::deductBalance(float amount) {
    if (balance >= amount) {
        balance -= amount;
    } else {
        balance = 0; // Tranh so du bi am
    }
}

void Customer::addBalance(float amount) {
    if (amount > 0) {
        balance += amount;
    }
}

string Customer::getRoleName() const {
    return "Customer";
}

// =====================================================================
// CAI DAT LOP CHARGING PORT
// =====================================================================

ChargingPort::ChargingPort(const string& id, PortType t, float pwr, float price,
                           PortStatus st, const string& sessId)
    : portId(id), type(t), powerKW(pwr), status(st), pricePerKWh(price), currentSessionId(sessId) {}

string ChargingPort::getPortId() const { return portId; }
PortType ChargingPort::getType() const { return type; }
float ChargingPort::getPowerKW() const { return powerKW; }
PortStatus ChargingPort::getStatus() const { return status; }
float ChargingPort::getPricePerKWh() const { return pricePerKWh; }
string ChargingPort::getCurrentSessionId() const { return currentSessionId; }

void ChargingPort::setStatus(PortStatus newStatus) { status = newStatus; }
void ChargingPort::setPricePerKWh(float newPrice) { pricePerKWh = newPrice; }
void ChargingPort::setCurrentSessionId(const string& sessId) { currentSessionId = sessId; }

// =====================================================================
// CAI DAT LOP CHARGING STATION
// =====================================================================

ChargingStation::ChargingStation(const string& id, const string& n, const string& addr)
    : stationId(id), name(n), address(addr) {}

string ChargingStation::getStationId() const { return stationId; }
string ChargingStation::getName() const { return name; }
string ChargingStation::getAddress() const { return address; }
const vector<ChargingPort>& ChargingStation::getPorts() const { return ports; }
vector<ChargingPort>& ChargingStation::getPorts() { return ports; }

void ChargingStation::setName(const string& n) { name = n; }
void ChargingStation::setAddress(const string& addr) { address = addr; }

void ChargingStation::addPort(const ChargingPort& port) {
    ports.push_back(port);
}

bool ChargingStation::removePort(const string& portId) {
    for (auto it = ports.begin(); it != ports.end(); ++it) {
        if (it->getPortId() == portId) {
            ports.erase(it);
            return true;
        }
    }
    return false;
}

ChargingPort* ChargingStation::getPortById(const string& portId) {
    for (auto& p : ports) {
        if (p.getPortId() == portId) return &p;
    }
    return nullptr;
}

// =====================================================================
// CAI DAT LOP CHARGING SESSION
// =====================================================================

ChargingSession::ChargingSession(const string& sId, const string& cId, const string& stId,
                                 const string& pId, long long start)
    : sessionId(sId), customerId(cId), stationId(stId), portId(pId),
      startTime(start), endTime(0), energyKWh(0.0f), cost(0.0f), isActive(true) {}

string ChargingSession::getSessionId() const { return sessionId; }
string ChargingSession::getCustomerId() const { return customerId; }
string ChargingSession::getStationId() const { return stationId; }
string ChargingSession::getPortId() const { return portId; }
long long ChargingSession::getStartTime() const { return startTime; }
long long ChargingSession::getEndTime() const { return endTime; }
float ChargingSession::getEnergyKWh() const { return energyKWh; }
float ChargingSession::getCost() const { return cost; }
bool ChargingSession::getIsActive() const { return isActive; }

void ChargingSession::setEndTime(long long endT) { endTime = endT; }
void ChargingSession::setEnergyKWh(float energy) { energyKWh = energy; }
void ChargingSession::setCost(float c) { cost = c; }
void ChargingSession::setIsActive(bool active) { isActive = active; }

void ChargingSession::endSession(long long endT, float energy, float totalCost) {
    endTime = endT;
    energyKWh = energy;
    cost = totalCost;
    isActive = false;
}

// =====================================================================
// CAI DAT LOP INVOICE
// =====================================================================

Invoice::Invoice(const string& id, const string& sId, const string& cId, float amount, long long d)
    : invoiceId(id), sessionId(sId), customerId(cId), totalAmount(amount), date(d) {}

string Invoice::getInvoiceId() const { return invoiceId; }
string Invoice::getSessionId() const { return sessionId; }
string Invoice::getCustomerId() const { return customerId; }
float Invoice::getTotalAmount() const { return totalAmount; }
long long Invoice::getDate() const { return date; }
