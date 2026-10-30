#pragma once
#include "Managers.h"
#include "raylib.h"
#include <string>

using namespace std;

// =====================================================================
// ENUM TRANG THAI MAN HINH GIAO DIEN (APP STATE)
// =====================================================================

/**
 * @brief Enum quan ly cac trang thai man hinh trong ung dung (State Pattern)
 */
enum AppState {
    STATE_LOGIN,                // Man hinh Dang nhap
    STATE_ADMIN_DASHBOARD,      // Man hinh Tong quan (Dashboard) cho Admin
    STATE_ADMIN_STATIONS,       // Man hinh Quan ly tram va cong sac cho Admin
    STATE_ADMIN_CUSTOMERS,      // Man hinh Quan ly khach hang & nap tien cho Admin
    STATE_ADMIN_INVOICES,       // Man hinh Quan ly hoa don toan he thong cho Admin
    STATE_CUSTOMER_MENU,        // Man hinh Menu chinh cho Khach hang
    STATE_CUSTOMER_CHARGING,    // Man hinh Dang thuc hien sac xe cho Khach hang
    STATE_CUSTOMER_RECEIPT      // Man hinh Bien lai hoa don sau khi dung sac
};

// =====================================================================
// LOP QUAN LY GIAO DIEN NGUOI DUNG (APP UI)
// =====================================================================

/**
 * @brief Lop AppUI quan ly toan bo giao dien do hoa Raylib/Raygui,
 * ho tro phong to toan man hinh (Fullscreen/Maximized Responsive)
 * va su dung phong chu Segoe UI ro rang, sac net, de doc.
 */
class AppUI {
private:
    // --- Tham chieu den cac lop quan ly nghiep vu (OOP: Dependency Injection) ---
    UserManager& um;
    StationManager& sm;
    SessionManager& sem;

    AppState currentState;      // Trang thai man hinh hien tai
    Font uiFont;                // Phong chu he thong Segoe UI giup chu to ro, min mang

    /**
     * @brief Ham tien ich tu dong ghi luu toan bo du lieu ra tep CSV
     */
    void autoSave() {
        DataManager::saveAll(um, sm, sem);
    }

    // --- Trang thai Man hinh Dang nhap ---
    char loginUsername[64];
    char loginPassword[64];
    bool loginUsernameActive;
    bool loginPasswordActive;
    bool loginError;
    string loginErrorMsg;

    // --- Trang thai Sidebar Admin ---
    int adminMenuSelected;
    
    // --- Trang thai Form Them Tram sac (Admin) ---
    char newStationName[128];
    char newStationAddress[128];
    bool activeStationName;
    bool activeStationAddress;
    string stationStatusMsg;
    bool showAddStationForm;

    // --- Trang thai Form Them Cong sac vao Tram (Admin) ---
    string targetStationIdForPort;
    int selectedPortType;       // 0: AC Standard, 1: DC Fast, 2: Supercharge
    char newPortPower[32];
    char newPortPrice[32];
    bool activePortPower;
    bool activePortPrice;
    bool showAddPortForm;

    // --- Trang thai Quan ly & Them Khach hang (Admin) ---
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
    int activeCustField;        // Chi so o text dang focus
    string customerStatusMsg;

    // --- Trang thai Admin Nap Tien cho Khach hang ---
    bool showTopupModal;
    string topupTargetCustomerId;
    char adminTopupAmount[32];
    bool activeAdminTopup;
    string adminTopupMsg;

    // --- Trang thai Giao dien Khach hang ---
    int customerTab;            // 0: Tra cuu & Sac dien, 1: Lich su hoa don
    string customerErrorMsg;

    // --- Du lieu Bien lai Hoa don (Receipt) sau khi dung sac ---
    string receiptInvoiceId;
    string receiptStationName;
    string receiptPortId;
    float receiptKWh;
    float receiptCost;
    float receiptRemainingBal;

public:
    /**
     * @brief Constructor khoi tao AppUI voi cac Manager tuong ung
     */
    AppUI(UserManager& u, StationManager& s, SessionManager& se);

    /**
     * @brief Destructor giai phong phong chu neu co
     */
    ~AppUI();

    /**
     * @brief Khoi tao style va font cho thu vien do hoa Raygui
     */
    void Init();

    /**
     * @brief Ham cap nhat logic va ve khung hinh (duoc goi lien tuc trong vong lap chinh)
     */
    void UpdateDrawFrame();

    /**
     * @brief Ham tien ich ve chu voi phong chu Segoe UI mượt mà sắc nét
     */
    void DrawTextUI(const string& text, float posX, float posY, float fontSize, Color color);

    /**
     * @brief Ham tien ich ve chu can giua man hinh theo chieu ngang
     */
    void DrawTextUICentered(const string& text, float posY, float fontSize, Color color);

private:
    // --- Cac ham ve giao dien theo tung man hinh ---
    void DrawLogin();
    void DrawAdminDashboard();
    void DrawAdminStations();
    void DrawAdminCustomers();
    void DrawAdminInvoices();
    
    void DrawCustomerMenu();
    void DrawCustomerCharging();
    void DrawCustomerReceipt();

    void DrawAdminSidebar();
    void DrawAdminTopupModal(); // Hop thoai nap tien cho khach
};
