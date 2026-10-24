#pragma once
#include "Managers.h"
#include "raylib.h"
#include <string>

using namespace std;

enum AppState {
    STATE_LOGIN,
    STATE_ADMIN_DASHBOARD,
    STATE_ADMIN_STATIONS,
    STATE_ADMIN_CUSTOMERS,
    STATE_ADMIN_INVOICES,
    STATE_CUSTOMER_MENU,
    STATE_CUSTOMER_CHARGING,
    STATE_CUSTOMER_RECEIPT
};

class AppUI {
private:
    UserManager& um;
    StationManager& sm;
    SessionManager& sem;
    AppState currentState;

    void autoSave() {
        DataManager::saveAll(um, sm, sem);
    }

    // --- Login screen state ---
    char loginUsername[64];
    char loginPassword[64];
    bool loginUsernameActive;
    bool loginPasswordActive;
    bool loginError;
    string loginErrorMsg;

    // --- Admin state ---
    int adminMenuSelected;
    
    // Admin: Form thêm trạm sạc
    char newStationName[128];
    char newStationAddress[128];
    bool activeStationName;
    bool activeStationAddress;
    string stationStatusMsg;
    bool showAddStationForm;

    // Admin: Form thêm cổng sạc
    string targetStationIdForPort;
    int selectedPortType; // 0: AC, 1: DC Fast, 2: Supercharge
    char newPortPower[32];
    char newPortPrice[32];
    bool activePortPower;
    bool activePortPrice;
    bool showAddPortForm;

    // Admin: Tìm kiếm & Thêm khách hàng
    char customerSearch[128];
    bool activeCustomerSearch;
    bool showAddCustomerForm;
    char newCustUsername[64];
    char newCustPassword[64];
    char newCustName[64];
    char newCustPhone[32];
    char newCustPlate[32];
    char newCustCarModel[32];
    char newCustBattery[32];
    char newCustBalance[32];
    int activeCustField; // 0: None, 1: User, 2: Pass, 3: Name, 4: Phone, 5: Plate, 6: Car, 7: Bat, 8: Bal
    string customerStatusMsg;

    // --- Customer state ---
    int customerTab; // 0: Trạm & Cổng, 1: Nạp tiền, 2: Lịch sử hóa đơn
    string customerErrorMsg;
    char topupAmountStr[32];
    bool activeTopupAmount;
    string topupStatusMsg;

    // Last invoice data for Receipt screen
    string receiptInvoiceId;
    string receiptStationName;
    string receiptPortId;
    float receiptKWh;
    float receiptCost;
    float receiptRemainingBal;

public:
    AppUI(UserManager& u, StationManager& s, SessionManager& se);
    void Init();
    void UpdateDrawFrame();

private:
    void DrawLogin();
    void DrawAdminDashboard();
    void DrawAdminStations();
    void DrawAdminCustomers();
    void DrawAdminInvoices();
    
    void DrawCustomerMenu();
    void DrawCustomerCharging();
    void DrawCustomerReceipt();

    void DrawAdminSidebar();
};
