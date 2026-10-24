#include "Managers.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <direct.h>   // _mkdir trên Windows

using namespace std;

// =====================================================================
// Helper: Tách chuỗi theo ký tự phân cách
// =====================================================================
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream ss(s);
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Đảm bảo thư mục Data/ tồn tại trước khi ghi file
static void ensureDataDir() {
    _mkdir("Data");
}

// =====================================================================
// UserManager
// =====================================================================
UserManager::UserManager() : loggedInUser(nullptr) {}

void UserManager::loadData() {
    admins.clear();
    customers.clear();

    // --- Đọc admins.csv ---
    // Header: id|username|password
    ifstream fa("Data/admins.csv");
    if (fa.is_open()) {
        string line;
        getline(fa, line); // bỏ dòng header
        while (getline(fa, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 3)
                admins.push_back(Admin(t[0], t[1], t[2]));
        }
    }

    // --- Đọc customers.csv ---
    // Header: id|username|password|name|phone|licensePlate|carModel|batteryCapacity|balance|isLocked
    ifstream fc("Data/customers.csv");
    if (fc.is_open()) {
        string line;
        getline(fc, line); // bỏ dòng header
        while (getline(fc, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 10)
                customers.push_back(Customer(
                    t[0], t[1], t[2], t[3], t[4],
                    t[5], t[6], stof(t[7]), stof(t[8]), stoi(t[9])
                ));
        }
    }

    // Tạo admin mặc định nếu chưa có
    if (admins.empty()) {
        admins.push_back(Admin("A01", "admin", "admin123"));
    }
}

void UserManager::saveData() {
    ensureDataDir();

    // --- Ghi admins.csv ---
    ofstream fa("Data/admins.csv");
    fa << "id|username|password\n";
    for (auto& a : admins)
        fa << a.getId() << "|" << a.getUsername() << "|" << a.getPassword() << "\n";

    // --- Ghi customers.csv ---
    ofstream fc("Data/customers.csv");
    fc << "id|username|password|name|phone|licensePlate|carModel|batteryCapacity|balance|isLocked\n";
    for (auto& c : customers)
        fc << c.getId()          << "|"
           << c.getUsername()    << "|"
           << c.getPassword()    << "|"
           << c.getName()        << "|"
           << c.getPhone()       << "|"
           << c.getLicensePlate()<< "|"
           << c.getCarModel()    << "|"
           << c.getBatteryCapacity() << "|"
           << fixed << setprecision(0) << c.getBalance() << "|"
           << c.getIsLocked()    << "\n";
}

bool UserManager::login(const string& username, const string& password) {
    for (auto& a : admins)
        if (a.getUsername() == username && a.checkPassword(password)) {
            loggedInUser = &a; return true;
        }
    for (auto& c : customers)
        if (c.getUsername() == username && c.checkPassword(password)) {
            if (c.getIsLocked()) return false;
            loggedInUser = &c; return true;
        }
    return false;
}

void UserManager::logout() { loggedInUser = nullptr; }
User* UserManager::getLoggedInUser() { return loggedInUser; }
vector<Customer>& UserManager::getCustomers() { return customers; }

Customer* UserManager::getCustomerById(const string& id) {
    for (auto& c : customers) if (c.getId() == id) return &c;
    return nullptr;
}

void UserManager::toggleCustomerLock(const string& id) {
    Customer* c = getCustomerById(id);
    if (c) c->setLocked(!c->getIsLocked());
}

void UserManager::addCustomer(const Customer& c) { customers.push_back(c); }

bool UserManager::usernameExists(const string& username) {
    for (auto& a : admins) if (a.getUsername() == username) return true;
    for (auto& c : customers) if (c.getUsername() == username) return true;
    return false;
}

string UserManager::generateNextCustomerId() {
    int maxId = 0;
    for (auto& c : customers) {
        string cid = c.getId();
        if (cid.size() > 1 && cid[0] == 'C') {
            try {
                int num = stoi(cid.substr(1));
                if (num > maxId) maxId = num;
            } catch (...) {}
        }
    }
    maxId++;
    char buf[16];
    sprintf(buf, "C%02d", maxId);
    return string(buf);
}

// =====================================================================
// StationManager
// =====================================================================
StationManager::StationManager() {}

void StationManager::loadData() {
    stations.clear();

    // --- Đọc stations.csv ---
    // Header: stationId|name|address
    ifstream fs("Data/stations.csv");
    if (fs.is_open()) {
        string line;
        getline(fs, line); // bỏ header
        while (getline(fs, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 3)
                stations.push_back(ChargingStation(t[0], t[1], t[2]));
        }
    }

    // --- Đọc ports.csv ---
    // Header: stationId|portId|type|powerKW|status|pricePerKWh|currentSessionId
    ifstream fp("Data/ports.csv");
    if (fp.is_open()) {
        string line;
        getline(fp, line); // bỏ header
        while (getline(fp, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 7) {
                ChargingStation* st = getStationById(t[0]);
                if (st)
                    st->addPort(ChargingPort(
                        t[1], (PortType)stoi(t[2]),
                        stof(t[3]), stof(t[5]),
                        (PortStatus)stoi(t[4]), t[6]
                    ));
            }
        }
    }
}

void StationManager::saveData() {
    ensureDataDir();

    // --- Ghi stations.csv ---
    ofstream fs("Data/stations.csv");
    fs << "stationId|name|address\n";
    for (auto& st : stations)
        fs << st.stationId << "|" << st.name << "|" << st.address << "\n";

    // --- Ghi ports.csv ---
    ofstream fp("Data/ports.csv");
    fp << "stationId|portId|type|powerKW|status|pricePerKWh|currentSessionId\n";
    for (auto& st : stations)
        for (auto& p : st.ports)
            fp << st.stationId         << "|"
               << p.portId             << "|"
               << (int)p.type          << "|"
               << p.powerKW            << "|"
               << (int)p.status        << "|"
               << fixed << setprecision(0) << p.pricePerKWh << "|"
               << p.currentSessionId   << "\n";
}

vector<ChargingStation>& StationManager::getStations() { return stations; }

ChargingStation* StationManager::getStationById(const string& id) {
    for (auto& s : stations) if (s.stationId == id) return &s;
    return nullptr;
}

void StationManager::addStation(const ChargingStation& st) { stations.push_back(st); }

bool StationManager::deleteStation(const string& id) {
    for (auto it = stations.begin(); it != stations.end(); ++it) {
        if (it->stationId == id) {
            for (auto& p : it->ports)
                if (p.status == OCCUPIED) return false; // Chỉ xóa khi không có cổng nào đang sạc
            stations.erase(it);
            return true;
        }
    }
    return false;
}

void StationManager::updateStation(const string& id, const string& name, const string& address) {
    ChargingStation* st = getStationById(id);
    if (st) { st->name = name; st->address = address; }
}

void StationManager::addPortToStation(const string& stationId, const ChargingPort& port) {
    ChargingStation* st = getStationById(stationId);
    if (st) st->addPort(port);
}

bool StationManager::deletePort(const string& stationId, const string& portId) {
    ChargingStation* st = getStationById(stationId);
    if (!st) return false;
    for (auto it = st->ports.begin(); it != st->ports.end(); ++it) {
        if (it->portId == portId) {
            if (it->status == OCCUPIED) return false;
            st->ports.erase(it);
            return true;
        }
    }
    return false;
}

void StationManager::updatePortStatus(const string& stationId, const string& portId, PortStatus newStatus) {
    ChargingStation* st = getStationById(stationId);
    if (st) {
        for (auto& p : st->ports) {
            if (p.portId == portId) p.status = newStatus;
        }
    }
}

void StationManager::updatePortPrice(const string& stationId, const string& portId, float newPrice) {
    ChargingStation* st = getStationById(stationId);
    if (st) {
        for (auto& p : st->ports) {
            if (p.portId == portId) p.pricePerKWh = newPrice;
        }
    }
}

string StationManager::generateNextStationId() {
    int maxId = 0;
    for (auto& s : stations) {
        string sid = s.stationId;
        if (sid.size() > 2 && sid.substr(0, 2) == "ST") {
            try {
                int num = stoi(sid.substr(2));
                if (num > maxId) maxId = num;
            } catch (...) {}
        }
    }
    maxId++;
    char buf[16];
    sprintf(buf, "ST%02d", maxId);
    return string(buf);
}

string StationManager::generateNextPortId(const string& stationId) {
    ChargingStation* st = getStationById(stationId);
    int maxId = 0;
    if (st) {
        for (auto& p : st->ports) {
            string pid = p.portId;
            if (pid.size() > 1 && pid[0] == 'P') {
                try {
                    int num = stoi(pid.substr(1));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
        }
    }
    maxId++;
    char buf[16];
    sprintf(buf, "P%02d", maxId);
    return string(buf);
}

int StationManager::getTotalStations() { return (int)stations.size(); }
int StationManager::getTotalPorts() {
    int sum = 0;
    for (auto& st : stations) sum += (int)st.ports.size();
    return sum;
}
int StationManager::getPortsByStatus(PortStatus status) {
    int sum = 0;
    for (auto& st : stations)
        for (auto& p : st.ports)
            if (p.status == status) sum++;
    return sum;
}

// =====================================================================
// SessionManager
// =====================================================================
SessionManager::SessionManager() {}

void SessionManager::loadData() {
    sessions.clear();
    invoices.clear();

    // --- Đọc sessions.csv ---
    // Header: sessionId|customerId|stationId|portId|startTime|endTime|energyKWh|cost|isActive
    ifstream fs("Data/sessions.csv");
    if (fs.is_open()) {
        string line;
        getline(fs, line); // bỏ header
        while (getline(fs, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 9) {
                ChargingSession ses(t[0], t[1], t[2], t[3], stoll(t[4]));
                ses.endTime    = stoll(t[5]);
                ses.energyKWh  = stof(t[6]);
                ses.cost       = stof(t[7]);
                ses.isActive   = stoi(t[8]);
                sessions.push_back(ses);
            }
        }
    }

    // --- Đọc invoices.csv ---
    // Header: invoiceId|sessionId|customerId|totalAmount|date
    ifstream fi("Data/invoices.csv");
    if (fi.is_open()) {
        string line;
        getline(fi, line); // bỏ header
        while (getline(fi, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 5)
                invoices.push_back(Invoice(t[0], t[1], t[2], stof(t[3]), stoll(t[4])));
        }
    }
}

void SessionManager::saveData() {
    ensureDataDir();

    // --- Ghi sessions.csv ---
    ofstream fs("Data/sessions.csv");
    fs << "sessionId|customerId|stationId|portId|startTime|endTime|energyKWh|cost|isActive\n";
    for (auto& s : sessions)
        fs << s.sessionId   << "|"
           << s.customerId  << "|"
           << s.stationId   << "|"
           << s.portId      << "|"
           << s.startTime   << "|"
           << s.endTime     << "|"
           << fixed << setprecision(2) << s.energyKWh   << "|"
           << fixed << setprecision(0) << s.cost        << "|"
           << s.isActive    << "\n";

    // --- Ghi invoices.csv ---
    ofstream fi("Data/invoices.csv");
    fi << "invoiceId|sessionId|customerId|totalAmount|date\n";
    for (auto& i : invoices)
        fi << i.invoiceId   << "|"
           << i.sessionId   << "|"
           << i.customerId  << "|"
           << fixed << setprecision(0) << i.totalAmount << "|"
           << i.date        << "\n";
}

vector<ChargingSession>& SessionManager::getSessions() { return sessions; }
vector<Invoice>& SessionManager::getInvoices() { return invoices; }

ChargingSession* SessionManager::getActiveSession(const string& customerId) {
    for (auto& s : sessions) {
        if (s.isActive && s.customerId == customerId) return &s;
    }
    return nullptr;
}

Invoice* SessionManager::getLastInvoice(const string& customerId) {
    for (int i = (int)invoices.size() - 1; i >= 0; --i) {
        if (invoices[i].customerId == customerId) return &invoices[i];
    }
    return nullptr;
}

vector<Invoice> SessionManager::getInvoicesByCustomer(const string& customerId) {
    vector<Invoice> res;
    for (auto& inv : invoices) {
        if (inv.customerId == customerId) res.push_back(inv);
    }
    return res;
}

bool SessionManager::startSession(const string& customerId, const string& stationId,
                                   const string& portId, StationManager& sm) {
    // Nếu khách đã có session đang sạc thì không bắt đầu cái mới
    if (getActiveSession(customerId) != nullptr) return false;

    ChargingStation* st = sm.getStationById(stationId);
    if (!st) return false;
    for (auto& p : st->ports) {
        if (p.portId == portId && p.status == AVAILABLE) {
            string sId = "SES_" + to_string(time(0));
            ChargingSession ses(sId, customerId, stationId, portId, time(0));
            sessions.push_back(ses);
            p.status = OCCUPIED;
            p.currentSessionId = sId;
            return true;
        }
    }
    return false;
}

bool SessionManager::endSession(const string& customerId, StationManager& sm, UserManager& um) {
    for (auto& s : sessions) {
        if (s.isActive && s.customerId == customerId) {
            long long endT    = time(0);
            long long duration = endT - s.startTime;
            if (duration <= 0) duration = 1;

            ChargingStation* st = sm.getStationById(s.stationId);
            if (st) {
                for (auto& p : st->ports) {
                    if (p.portId == s.portId) {
                        // Tính điện năng tiêu thụ (Mô phỏng 1 giây thực = 1 phút sạc)
                        float simulatedHours = (duration * 60.0f) / 3600.0f;
                        float energy = p.powerKW * simulatedHours;
                        if (energy < 0.5f) energy = 0.5f; // tối thiểu 0.5 kWh
                        float totalCost = energy * p.pricePerKWh;

                        Customer* c = um.getCustomerById(customerId);
                        if (c) {
                            c->deductBalance(totalCost);
                        }
                        
                        s.endSession(endT, energy, totalCost);
                        p.status = AVAILABLE;
                        p.currentSessionId = "";

                        string invId = "INV_" + to_string(time(0));
                        invoices.push_back(Invoice(invId, s.sessionId, customerId, totalCost, endT));
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

float SessionManager::getTotalRevenue() {
    float sum = 0;
    for (auto& i : invoices) sum += i.totalAmount;
    return sum;
}

float SessionManager::getTotalEnergyConsumed() {
    float sum = 0;
    for (auto& s : sessions) sum += s.energyKWh;
    return sum;
}

// =====================================================================
// DataManager
// =====================================================================
void DataManager::loadAll(UserManager& um, StationManager& sm, SessionManager& sem) {
    um.loadData();
    sm.loadData();
    sem.loadData();
}

void DataManager::saveAll(UserManager& um, StationManager& sm, SessionManager& sem) {
    um.saveData();
    sm.saveData();
    sem.saveData();
}
