#include "Managers.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <direct.h>   // _mkdir tren Windows de dam bao thu muc Data ton tai

using namespace std;

// =====================================================================
// HELPER: TACH CHUOI CHUAN XAC (KHONG BO SOT TOKEN RONG)
// =====================================================================

/**
 * @brief Ham tach chuoi dua tren ky tu phan cach delimiter.
 * Khac voi istringstream >> getline, ham nay su dung s.find()
 * nen dam bao giu dung moi token ke ca khi token cuoi cung la chuoi rong.
 * Vi du: "ST01|P01|0|22|0|3500|" se tach duoc dung 7 tokens.
 */
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);
    while (end != string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delimiter, start);
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

/**
 * @brief Tao thu muc Data neu chua ton tai tren he thong
 */
static void ensureDataDir() {
    _mkdir("Data");
}

// =====================================================================
// CAI DAT LOP USER MANAGER
// =====================================================================

UserManager::UserManager() : loggedInUser(nullptr) {}

void UserManager::loadData() {
    admins.clear();
    customers.clear();

    // --- Doc tep admins.csv ---
    // Cau truc CSDL: id|username|password
    ifstream fa("Data/admins.csv");
    if (fa.is_open()) {
        string line;
        getline(fa, line); // Bo qua dong tieu de (Header)
        while (getline(fa, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 3) {
                admins.push_back(Admin(t[0], t[1], t[2]));
            }
        }
        fa.close();
    }

    // --- Doc tep customers.csv ---
    // Cau truc CSDL: id|username|password|name|phone|licensePlate|carModel|batteryCapacity|balance
    ifstream fc("Data/customers.csv");
    if (fc.is_open()) {
        string line;
        getline(fc, line); // Bo qua dong tieu de (Header)
        while (getline(fc, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 9) {
                customers.push_back(Customer(
                    t[0], t[1], t[2], t[3], t[4],
                    t[5], t[6], stof(t[7]), stof(t[8])
                ));
            }
        }
        fc.close();
    }

    // Khong khoi tao du lieu mau neu file trong - hoan toan doc tu file CSV
}

void UserManager::saveData() {
    ensureDataDir();

    // --- Ghi tep admins.csv ---
    ofstream fa("Data/admins.csv");
    fa << "id|username|password\n";
    for (auto& a : admins) {
        fa << a.getId() << "|"
           << a.getUsername() << "|"
           << a.getPassword() << "\n";
    }
    fa.close();

    // --- Ghi tep customers.csv ---
    ofstream fc("Data/customers.csv");
    fc << "id|username|password|name|phone|licensePlate|carModel|batteryCapacity|balance\n";
    for (auto& c : customers) {
        fc << c.getId()               << "|"
           << c.getUsername()         << "|"
           << c.getPassword()         << "|"
           << c.getName()             << "|"
           << c.getPhone()            << "|"
           << c.getLicensePlate()     << "|"
           << c.getCarModel()         << "|"
           << c.getBatteryCapacity()  << "|"
           << fixed << setprecision(0)<< c.getBalance() << "\n";
    }
    fc.close();
}

bool UserManager::login(const string& username, const string& password) {
    // 1. Kiem tra trong danh sach Quan tri vien
    for (auto& a : admins) {
        if (a.getUsername() == username && a.checkPassword(password)) {
            loggedInUser = &a;
            return true;
        }
    }

    // 2. Kiem tra trong danh sach Khach hang (da bo phan isLocked)
    for (auto& c : customers) {
        if (c.getUsername() == username && c.checkPassword(password)) {
            loggedInUser = &c;
            return true;
        }
    }

    return false;
}

void UserManager::logout() {
    loggedInUser = nullptr;
}

User* UserManager::getLoggedInUser() {
    return loggedInUser;
}

vector<Customer>& UserManager::getCustomers() {
    return customers;
}

Customer* UserManager::getCustomerById(const string& id) {
    for (auto& c : customers) {
        if (c.getId() == id) return &c;
    }
    return nullptr;
}

void UserManager::addCustomer(const Customer& c) {
    customers.push_back(c);
}

bool UserManager::usernameExists(const string& username) {
    for (auto& a : admins) {
        if (a.getUsername() == username) return true;
    }
    for (auto& c : customers) {
        if (c.getUsername() == username) return true;
    }
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

bool UserManager::topupCustomerBalance(const string& customerId, float amount) {
    Customer* c = getCustomerById(customerId);
    if (c && amount > 0) {
        c->addBalance(amount);
        return true;
    }
    return false;
}

// =====================================================================
// CAI DAT LOP STATION MANAGER
// =====================================================================

StationManager::StationManager() {}

void StationManager::loadData() {
    stations.clear();

    // --- 1. Doc tep stations.csv ---
    // Cau truc CSDL: stationId|name|address
    ifstream fs("Data/stations.csv");
    if (fs.is_open()) {
        string line;
        getline(fs, line); // Bo qua dong Header
        while (getline(fs, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 3) {
                stations.push_back(ChargingStation(t[0], t[1], t[2]));
            }
        }
        fs.close();
    }

    // --- 2. Doc tep ports.csv ---
    // Cau truc CSDL: stationId|portId|type|powerKW|status|pricePerKWh|currentSessionId
    ifstream fp("Data/ports.csv");
    if (fp.is_open()) {
        string line;
        getline(fp, line); // Bo qua dong Header
        while (getline(fp, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            // Can it nhat 6 truong bat buoc, truong 7 la currentSessionId (co the rong)
            if (t.size() >= 6) {
                ChargingStation* st = getStationById(t[0]);
                if (st) {
                    string sessId = (t.size() >= 7) ? t[6] : "";
                    st->addPort(ChargingPort(
                        t[1],
                        (PortType)stoi(t[2]),
                        stof(t[3]),
                        stof(t[5]),
                        (PortStatus)stoi(t[4]),
                        sessId
                    ));
                }
            }
        }
        fp.close();
    }

    // Khong khoi tao du lieu mau neu file trong - hoan toan doc tu file CSV
}

void StationManager::saveData() {
    ensureDataDir();

    // --- 1. Ghi tep stations.csv ---
    ofstream fs("Data/stations.csv");
    fs << "stationId|name|address\n";
    for (auto& st : stations) {
        fs << st.getStationId() << "|"
           << st.getName()      << "|"
           << st.getAddress()   << "\n";
    }
    fs.close();

    // --- 2. Ghi tep ports.csv ---
    ofstream fp("Data/ports.csv");
    fp << "stationId|portId|type|powerKW|status|pricePerKWh|currentSessionId\n";
    for (auto& st : stations) {
        for (auto& p : st.getPorts()) {
            fp << st.getStationId()          << "|"
               << p.getPortId()              << "|"
               << (int)p.getType()           << "|"
               << p.getPowerKW()             << "|"
               << (int)p.getStatus()         << "|"
               << fixed << setprecision(0)   << p.getPricePerKWh() << "|"
               << p.getCurrentSessionId()    << "\n";
        }
    }
    fp.close();
}

vector<ChargingStation>& StationManager::getStations() {
    return stations;
}

ChargingStation* StationManager::getStationById(const string& id) {
    for (auto& s : stations) {
        if (s.getStationId() == id) return &s;
    }
    return nullptr;
}

void StationManager::addStation(const ChargingStation& st) {
    stations.push_back(st);
}

bool StationManager::deleteStation(const string& id) {
    for (auto it = stations.begin(); it != stations.end(); ++it) {
        if (it->getStationId() == id) {
            // Rang buoc: Chi xoa khi tat ca cac cong cua tram khong o trang thai OCCUPIED
            for (auto& p : it->getPorts()) {
                if (p.getStatus() == OCCUPIED) return false;
            }
            stations.erase(it);
            return true;
        }
    }
    return false;
}

void StationManager::updateStation(const string& id, const string& name, const string& address) {
    ChargingStation* st = getStationById(id);
    if (st) {
        st->setName(name);
        st->setAddress(address);
    }
}

void StationManager::addPortToStation(const string& stationId, const ChargingPort& port) {
    ChargingStation* st = getStationById(stationId);
    if (st) {
        st->addPort(port);
    }
}

bool StationManager::deletePort(const string& stationId, const string& portId) {
    ChargingStation* st = getStationById(stationId);
    if (!st) return false;
    ChargingPort* p = st->getPortById(portId);
    if (p && p->getStatus() == OCCUPIED) {
        return false; // Khong cho phep xoa cong dang duoc sac
    }
    return st->removePort(portId);
}

void StationManager::updatePortStatus(const string& stationId, const string& portId, PortStatus newStatus) {
    ChargingStation* st = getStationById(stationId);
    if (st) {
        ChargingPort* p = st->getPortById(portId);
        if (p) p->setStatus(newStatus);
    }
}

void StationManager::updatePortPrice(const string& stationId, const string& portId, float newPrice) {
    ChargingStation* st = getStationById(stationId);
    if (st) {
        ChargingPort* p = st->getPortById(portId);
        if (p) p->setPricePerKWh(newPrice);
    }
}

string StationManager::generateNextStationId() {
    int maxId = 0;
    for (auto& s : stations) {
        string sid = s.getStationId();
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
        for (auto& p : st->getPorts()) {
            string pid = p.getPortId();
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

int StationManager::getTotalStations() const {
    return (int)stations.size();
}

int StationManager::getTotalPorts() const {
    int sum = 0;
    for (const auto& st : stations) {
        sum += (int)st.getPorts().size();
    }
    return sum;
}

int StationManager::getPortsByStatus(PortStatus status) const {
    int sum = 0;
    for (const auto& st : stations) {
        for (const auto& p : st.getPorts()) {
            if (p.getStatus() == status) sum++;
        }
    }
    return sum;
}

// =====================================================================
// CAI DAT LOP SESSION MANAGER
// =====================================================================

SessionManager::SessionManager() {}

void SessionManager::loadData() {
    sessions.clear();
    invoices.clear();

    // --- 1. Doc tep sessions.csv ---
    // Cau truc CSDL: sessionId|customerId|stationId|portId|startTime|endTime|energyKWh|cost|isActive
    ifstream fs("Data/sessions.csv");
    if (fs.is_open()) {
        string line;
        getline(fs, line); // Bo qua Header
        while (getline(fs, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 9) {
                ChargingSession ses(t[0], t[1], t[2], t[3], stoll(t[4]));
                ses.setEndTime(stoll(t[5]));
                ses.setEnergyKWh(stof(t[6]));
                ses.setCost(stof(t[7]));
                ses.setIsActive(stoi(t[8]) != 0);
                sessions.push_back(ses);
            }
        }
        fs.close();
    }

    // --- 2. Doc tep invoices.csv ---
    // Cau truc CSDL: invoiceId|sessionId|customerId|totalAmount|date
    ifstream fi("Data/invoices.csv");
    if (fi.is_open()) {
        string line;
        getline(fi, line); // Bo qua Header
        while (getline(fi, line)) {
            if (line.empty()) continue;
            vector<string> t = split(line, '|');
            if (t.size() >= 5) {
                invoices.push_back(Invoice(t[0], t[1], t[2], stof(t[3]), stoll(t[4])));
            }
        }
        fi.close();
    }

    // Khong khoi tao du lieu mau neu file trong - hoan toan doc tu file CSV
}

void SessionManager::saveData() {
    ensureDataDir();

    // --- 1. Ghi tep sessions.csv ---
    ofstream fs("Data/sessions.csv");
    fs << "sessionId|customerId|stationId|portId|startTime|endTime|energyKWh|cost|isActive\n";
    for (auto& s : sessions) {
        fs << s.getSessionId()          << "|"
           << s.getCustomerId()         << "|"
           << s.getStationId()          << "|"
           << s.getPortId()             << "|"
           << s.getStartTime()          << "|"
           << s.getEndTime()            << "|"
           << fixed << setprecision(2)  << s.getEnergyKWh() << "|"
           << fixed << setprecision(0)  << s.getCost()      << "|"
           << (s.getIsActive() ? 1 : 0) << "\n";
    }
    fs.close();

    // --- 2. Ghi tep invoices.csv ---
    ofstream fi("Data/invoices.csv");
    fi << "invoiceId|sessionId|customerId|totalAmount|date\n";
    for (auto& i : invoices) {
        fi << i.getInvoiceId()          << "|"
           << i.getSessionId()          << "|"
           << i.getCustomerId()         << "|"
           << fixed << setprecision(0)  << i.getTotalAmount() << "|"
           << i.getDate()               << "\n";
    }
    fi.close();
}

vector<ChargingSession>& SessionManager::getSessions() {
    return sessions;
}

vector<Invoice>& SessionManager::getInvoices() {
    return invoices;
}

ChargingSession* SessionManager::getActiveSession(const string& customerId) {
    for (auto& s : sessions) {
        if (s.getIsActive() && s.getCustomerId() == customerId) return &s;
    }
    return nullptr;
}

Invoice* SessionManager::getLastInvoice(const string& customerId) {
    for (int i = (int)invoices.size() - 1; i >= 0; --i) {
        if (invoices[i].getCustomerId() == customerId) return &invoices[i];
    }
    return nullptr;
}

vector<Invoice> SessionManager::getInvoicesByCustomer(const string& customerId) {
    vector<Invoice> res;
    for (auto& inv : invoices) {
        if (inv.getCustomerId() == customerId) res.push_back(inv);
    }
    return res;
}

bool SessionManager::startSession(const string& customerId, const string& stationId,
                                  const string& portId, StationManager& sm) {
    // 1. Kiem tra xem khach hang da co phien sac nao dang dien ra hay chua
    if (getActiveSession(customerId) != nullptr) return false;

    // 2. Kiem tra tinh kha dung cua tram va cong sac
    ChargingStation* st = sm.getStationById(stationId);
    if (!st) return false;

    ChargingPort* p = st->getPortById(portId);
    if (p && p->getStatus() == AVAILABLE) {
        // Sinh ma phien sac tuong ung theo timestamp
        string sId = "SES_" + to_string(time(0));
        ChargingSession ses(sId, customerId, stationId, portId, time(0));
        sessions.push_back(ses);

        // Cap nhat trang thai cong sang OCCUPIED
        p->setStatus(OCCUPIED);
        p->setCurrentSessionId(sId);
        return true;
    }
    return false;
}

bool SessionManager::endSession(const string& customerId, StationManager& sm, UserManager& um) {
    for (auto& s : sessions) {
        if (s.getIsActive() && s.getCustomerId() == customerId) {
            long long endT = time(0);
            long long duration = endT - s.getStartTime();
            if (duration <= 0) duration = 1;

            ChargingStation* st = sm.getStationById(s.getStationId());
            if (st) {
                ChargingPort* p = st->getPortById(s.getPortId());
                if (p) {
                    // Tinh dien nang tieu thu (Mo phong 1 giay thuc = 1 phut sac)
                    float simulatedHours = (duration * 60.0f) / 3600.0f;
                    float energy = p->getPowerKW() * simulatedHours;
                    if (energy < 0.5f) energy = 0.5f; // Toi thieu 0.5 kWh cho demo
                    float totalCost = energy * p->getPricePerKWh();

                    // Tru tien vao tai khoan cua khach
                    Customer* c = um.getCustomerById(customerId);
                    if (c) {
                        c->deductBalance(totalCost);
                    }

                    // Ket thuc phien sac
                    s.endSession(endT, energy, totalCost);

                    // Tra cong sac ve trang thai AVAILABLE
                    p->setStatus(AVAILABLE);
                    p->setCurrentSessionId("");

                    // Sinh hoa don thanh toan
                    string invId = "INV_" + to_string(time(0));
                    invoices.push_back(Invoice(invId, s.getSessionId(), customerId, totalCost, endT));
                    return true;
                }
            }
        }
    }
    return false;
}

float SessionManager::getTotalRevenue() const {
    float sum = 0.0f;
    for (const auto& i : invoices) {
        sum += i.getTotalAmount();
    }
    return sum;
}

float SessionManager::getTotalEnergyConsumed() const {
    float sum = 0.0f;
    for (const auto& s : sessions) {
        sum += s.getEnergyKWh();
    }
    return sum;
}

// =====================================================================
// CAI DAT LOP DATA MANAGER
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
