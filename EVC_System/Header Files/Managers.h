#pragma once
#include "Models.h"
#include <vector>
#include <string>

using namespace std;

class UserManager {
private:
    vector<Admin> admins;
    vector<Customer> customers;
    User* loggedInUser;
public:
    UserManager();
    void loadData();
    void saveData();

    bool login(const string& username, const string& password);
    void logout();
    User* getLoggedInUser();

    vector<Customer>& getCustomers();
    Customer* getCustomerById(const string& id);
    void toggleCustomerLock(const string& id);
    
    // Thêm khách hàng mới
    void addCustomer(const Customer& c);
    bool usernameExists(const string& username);
    string generateNextCustomerId();
};

class StationManager {
private:
    vector<ChargingStation> stations;
public:
    StationManager();
    void loadData();
    void saveData();

    vector<ChargingStation>& getStations();
    ChargingStation* getStationById(const string& id);
    void addStation(const ChargingStation& st);
    bool deleteStation(const string& id); // chỉ xóa khi không có cổng nào đang sạc
    void updateStation(const string& id, const string& name, const string& address);

    void addPortToStation(const string& stationId, const ChargingPort& port);
    bool deletePort(const string& stationId, const string& portId);
    void updatePortStatus(const string& stationId, const string& portId, PortStatus newStatus);
    void updatePortPrice(const string& stationId, const string& portId, float newPrice);

    string generateNextStationId();
    string generateNextPortId(const string& stationId);

    // Dashboard stats
    int getTotalStations();
    int getTotalPorts();
    int getPortsByStatus(PortStatus status);
};

class SessionManager {
private:
    vector<ChargingSession> sessions;
    vector<Invoice> invoices;
public:
    SessionManager();
    void loadData();
    void saveData();

    vector<ChargingSession>& getSessions();
    vector<Invoice>& getInvoices();

    // Customer actions
    bool startSession(const string& customerId, const string& stationId, const string& portId, StationManager& sm);
    bool endSession(const string& customerId, StationManager& sm, UserManager& um);
    ChargingSession* getActiveSession(const string& customerId);
    Invoice* getLastInvoice(const string& customerId);
    vector<Invoice> getInvoicesByCustomer(const string& customerId);

    // Dashboard stats
    float getTotalRevenue();
    float getTotalEnergyConsumed();
};

class DataManager {
public:
    static void loadAll(UserManager& um, StationManager& sm, SessionManager& sem);
    static void saveAll(UserManager& um, StationManager& sm, SessionManager& sem);
};
