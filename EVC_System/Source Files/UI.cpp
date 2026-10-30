#include "UI.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <string.h>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace std;

// =====================================================================
// CAC HAM HO TRO DINH DANG (FORMAT HELPERS)
// =====================================================================

/**
 * @brief Dinh dang so tien sang chuoi tien te Viet Nam Dong (VND) co dau phay ngan cach.
 * Vi du: 500000 -> "500,000 VND"
 */
static string formatMoney(float amount) {
    long long val = (long long)amount;
    string s = to_string(val);
    int n = (int)s.length();
    string res = "";
    int count = 0;
    for (int i = n - 1; i >= 0; --i) {
        res = s[i] + res;
        count++;
        if (count == 3 && i != 0) {
            res = "," + res;
            count = 0;
        }
    }
    return res + " VND";
}

/**
 * @brief Chuyen doi gia tri thoi gian timestamp sang chuoi ngay gio de doc
 * Vi du: 1792830315 -> "24/10/2026 14:45"
 */
static string formatTime(long long ts) {
    if (ts <= 0) return "--:--";
    time_t t = (time_t)ts;
    struct tm* tm_info = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", tm_info);
    return string(buf);
}

/**
 * @brief Chuyen doi toan bo chuoi sang chu thuong de phuc vu tim kiem
 */
static string toLower(const string& str) {
    string res = str;
    for (char &c : res) c = (char)tolower((unsigned char)c);
    return res;
}

// =====================================================================
// CAI DAT CONSTRUCTOR VA KHOI TAO APP UI
// =====================================================================

AppUI::AppUI(UserManager& u, StationManager& s, SessionManager& se)
    : um(u), sm(s), sem(se), currentState(STATE_LOGIN),
      loginUsernameActive(false), loginPasswordActive(false), loginError(false), loginErrorMsg(""),
      adminMenuSelected(0),
      activeStationName(false), activeStationAddress(false), stationStatusMsg(""), showAddStationForm(false),
      targetStationIdForPort(""), selectedPortType(0), activePortPower(false), activePortPrice(false), showAddPortForm(false),
      activeCustomerSearch(false), showAddCustomerForm(false), activeCustField(0), customerStatusMsg(""),
      showTopupModal(false), topupTargetCustomerId(""), activeAdminTopup(false), adminTopupMsg(""),
      customerTab(0), customerErrorMsg(""),
      receiptInvoiceId(""), receiptStationName(""), receiptPortId(""), receiptKWh(0.0f), receiptCost(0.0f), receiptRemainingBal(0.0f)
{
    // Khoi tao bo dem bo nho cho cac chuoi nhap lieu
    memset(loginUsername, 0, sizeof(loginUsername));
    memset(loginPassword, 0, sizeof(loginPassword));
    memset(newStationName, 0, sizeof(newStationName));
    memset(newStationAddress, 0, sizeof(newStationAddress));
    memset(newPortPower, 0, sizeof(newPortPower));
    memset(newPortPrice, 0, sizeof(newPortPrice));
    memset(customerSearch, 0, sizeof(customerSearch));
    memset(newCustUsername, 0, sizeof(newCustUsername));
    memset(newCustPassword, 0, sizeof(newCustPassword));
    memset(newCustName, 0, sizeof(newCustName));
    memset(newCustPhone, 0, sizeof(newCustPhone));
    memset(newCustPlate, 0, sizeof(newCustPlate));
    memset(newCustCarModel, 0, sizeof(newCustCarModel));
    memset(newCustBattery, 0, sizeof(newCustBattery));
    memset(newCustBalance, 0, sizeof(newCustBalance));
    memset(adminTopupAmount, 0, sizeof(adminTopupAmount));

    // Gia tri mac dinh tien loi khi them moi
    strcpy(newPortPower, "22");
    strcpy(newPortPrice, "3500");
    strcpy(newCustBattery, "80");
    strcpy(newCustBalance, "500000");
    strcpy(adminTopupAmount, "100000");
}

void AppUI::Init() {
    GuiLoadStyleDefault();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
}

void AppUI::UpdateDrawFrame() {
    BeginDrawing();
    ClearBackground((Color){ 245, 247, 250, 255 }); // Mau nen xam trang trang nha

    // Dieu phoi ve man hinh theo State hien tai
    switch (currentState) {
        case STATE_LOGIN:             DrawLogin(); break;
        case STATE_ADMIN_DASHBOARD:   DrawAdminDashboard(); break;
        case STATE_ADMIN_STATIONS:    DrawAdminStations(); break;
        case STATE_ADMIN_CUSTOMERS:   DrawAdminCustomers(); break;
        case STATE_ADMIN_INVOICES:    DrawAdminInvoices(); break;
        case STATE_CUSTOMER_MENU:     DrawCustomerMenu(); break;
        case STATE_CUSTOMER_CHARGING: DrawCustomerCharging(); break;
        case STATE_CUSTOMER_RECEIPT:  DrawCustomerReceipt(); break;
    }

    // Neu Admin dang mo hop thoai nap tien cho khach thi ve de len tren
    if (showTopupModal) {
        DrawAdminTopupModal();
    }

    EndDrawing();
}

// =====================================================================
// 1. MAN HINH DANG NHAP (LOGIN SCREEN)
// =====================================================================

void AppUI::DrawLogin() {
    // Header Banner
    DrawRectangle(0, 0, GetScreenWidth(), 120, (Color){ 30, 58, 138, 255 });
    DrawText("EV CHARGING STATION MANAGEMENT", 240, 35, 30, WHITE);
    DrawText("HE THONG QUAN LY TRAM SAC XE DIEN (PBL2 - OOP)", 310, 75, 18, (Color){ 191, 219, 254, 255 });

    // Login Card (hop dang nhap bo goc)
    DrawRectangleRounded((Rectangle){ 280, 180, 464, 430 }, 0.05f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ 280, 180, 464, 430 }, 0.05f, 4, (Color){ 203, 213, 225, 255 });
    
    DrawText("DANG NHAP HE THONG", 380, 210, 22, (Color){ 15, 23, 42, 255 });

    // O nhap Username
    DrawText("Ten dang nhap (Username):", 330, 260, 16, (Color){ 71, 85, 105, 255 });
    if (GuiTextBox((Rectangle){ 330, 285, 364, 38 }, loginUsername, 64, loginUsernameActive)) {
        loginUsernameActive = !loginUsernameActive;
        if (loginUsernameActive) loginPasswordActive = false;
    }

    // O nhap Password
    DrawText("Mat khau (Password):", 330, 340, 16, (Color){ 71, 85, 105, 255 });
    if (GuiTextBox((Rectangle){ 330, 365, 364, 38 }, loginPassword, 64, loginPasswordActive)) {
        loginPasswordActive = !loginPasswordActive;
        if (loginPasswordActive) loginUsernameActive = false;
    }

    // Nut Bam Dang Nhap
    if (GuiButton((Rectangle){ 330, 435, 364, 45 }, "DANG NHAP")) {
        loginUsernameActive = false;
        loginPasswordActive = false;
        if (um.login(loginUsername, loginPassword)) {
            loginError = false;
            loginErrorMsg = "";
            if (um.getLoggedInUser()->getRole() == ROLE_ADMIN) {
                currentState = STATE_ADMIN_DASHBOARD;
            } else {
                // Kiem tra neu khach hang dang co phien sac do dang thi vao thang man hinh sac
                Customer* c = (Customer*)um.getLoggedInUser();
                if (c && sem.getActiveSession(c->getId()) != nullptr) {
                    currentState = STATE_CUSTOMER_CHARGING;
                } else {
                    currentState = STATE_CUSTOMER_MENU;
                }
            }
        } else {
            loginError = true;
            loginErrorMsg = "Sai tai khoan hoac mat khau!";
        }
    }

    if (loginError) {
        DrawText(loginErrorMsg.c_str(), 360, 495, 15, RED);
    }

    // Goi y tai khoan kiem thu
    DrawRectangle(310, 525, 404, 60, (Color){ 241, 245, 249, 255 });
    DrawText("Tai khoan Admin: admin / admin123", 330, 535, 14, (Color){ 100, 116, 139, 255 });
    DrawText("Tai khoan Khach: khach1 / 123", 330, 555, 14, (Color){ 100, 116, 139, 255 });
}

// =====================================================================
// THANH SIDEBAR DIEU HUONG CHO ADMIN
// =====================================================================

void AppUI::DrawAdminSidebar() {
    DrawRectangle(0, 0, 220, GetScreenHeight(), (Color){ 30, 41, 59, 255 });
    
    // Logo Admin
    DrawRectangle(15, 20, 190, 50, (Color){ 15, 23, 42, 255 });
    DrawText("⚡ EVC ADMIN", 35, 32, 22, (Color){ 56, 189, 248, 255 });

    // Cac nut chuyen tab quan ly
    if (GuiButton((Rectangle){ 15, 100, 190, 42 }, "📊 Dashboard")) {
        currentState = STATE_ADMIN_DASHBOARD;
    }
    if (GuiButton((Rectangle){ 15, 155, 190, 42 }, "🔌 Tram & Cong sac")) {
        currentState = STATE_ADMIN_STATIONS;
    }
    if (GuiButton((Rectangle){ 15, 210, 190, 42 }, "👥 Khach hang")) {
        currentState = STATE_ADMIN_CUSTOMERS;
    }
    if (GuiButton((Rectangle){ 15, 265, 190, 42 }, "🧾 Hoa don & Phien sac")) {
        currentState = STATE_ADMIN_INVOICES;
    }
    
    // Nut Dang Xuat
    if (GuiButton((Rectangle){ 15, (float)(GetScreenHeight() - 60), 190, 42 }, "🚪 DANG XUAT")) {
        um.logout();
        currentState = STATE_LOGIN;
        memset(loginUsername, 0, sizeof(loginUsername));
        memset(loginPassword, 0, sizeof(loginPassword));
        loginUsernameActive = false;
        loginPasswordActive = false;
        autoSave();
    }
}

// =====================================================================
// 2. ADMIN DASHBOARD (THONG KE TONG QUAN)
// =====================================================================

void AppUI::DrawAdminDashboard() {
    DrawAdminSidebar();

    int startX = 245;
    DrawText("THONG KE TONG QUAN HE THONG", startX, 25, 26, (Color){ 15, 23, 42, 255 });

    int totalStations = sm.getTotalStations();
    int totalPorts = sm.getTotalPorts();
    int avail = sm.getPortsByStatus(AVAILABLE);
    int occ = sm.getPortsByStatus(OCCUPIED);
    int maint = sm.getPortsByStatus(MAINTENANCE);
    float revenue = sem.getTotalRevenue();
    float energy = sem.getTotalEnergyConsumed();

    // 4 The Thong ke trang thai cong sac
    // The 1: Tong so tram
    DrawRectangleRounded((Rectangle){ (float)startX, 75, 175, 100 }, 0.1f, 4, (Color){ 224, 242, 254, 255 });
    DrawText("Tong so tram", startX + 15, 90, 15, (Color){ 3, 105, 161, 255 });
    DrawText(TextFormat("%d", totalStations), startX + 15, 115, 36, (Color){ 2, 132, 199, 255 });

    // The 2: Cong trong kha dung (Available)
    DrawRectangleRounded((Rectangle){ (float)startX + 190, 75, 175, 100 }, 0.1f, 4, (Color){ 220, 252, 231, 255 });
    DrawText("Cong trong (Available)", startX + 205, 90, 15, (Color){ 21, 128, 61, 255 });
    DrawText(TextFormat("%d / %d", avail, totalPorts), startX + 205, 115, 36, (Color){ 22, 163, 74, 255 });

    // The 3: Cong dang duoc sac (Occupied)
    DrawRectangleRounded((Rectangle){ (float)startX + 380, 75, 175, 100 }, 0.1f, 4, (Color){ 254, 243, 199, 255 });
    DrawText("Dang sac (Occupied)", startX + 395, 90, 15, (Color){ 180, 83, 9, 255 });
    DrawText(TextFormat("%d", occ), startX + 395, 115, 36, (Color){ 217, 119, 6, 255 });

    // The 4: Cong dang bao tri (Maintenance)
    DrawRectangleRounded((Rectangle){ (float)startX + 570, 75, 175, 100 }, 0.1f, 4, (Color){ 254, 226, 226, 255 });
    DrawText("Bao tri (Maintenance)", startX + 585, 90, 15, (Color){ 185, 28, 28, 255 });
    DrawText(TextFormat("%d", maint), startX + 585, 115, 36, (Color){ 220, 38, 38, 255 });

    // Khung Doanh thu & Dien nang tieu thu
    DrawRectangleRounded((Rectangle){ (float)startX, 195, 365, 90 }, 0.1f, 4, (Color){ 238, 242, 255, 255 });
    DrawText("Tong doanh thu he thong", startX + 20, 210, 16, (Color){ 67, 56, 202, 255 });
    DrawText(formatMoney(revenue).c_str(), startX + 20, 235, 26, (Color){ 79, 70, 229, 255 });

    DrawRectangleRounded((Rectangle){ (float)startX + 380, 195, 365, 90 }, 0.1f, 4, (Color){ 243, 232, 255, 255 });
    DrawText("Tong dien nang tieu thu", startX + 400, 210, 16, (Color){ 126, 34, 206, 255 });
    DrawText(TextFormat("%.2f kWh", energy), startX + 400, 235, 26, (Color){ 147, 51, 234, 255 });

    // Bang danh sach cac phien sac dang truc tiep hoat dong (Live Sessions)
    DrawText("CAC PHIEN SAC DANG DIEN RA (LIVE SESSIONS)", startX, 310, 18, (Color){ 30, 41, 59, 255 });
    DrawRectangle(startX, 340, 745, 380, WHITE);
    DrawRectangleLines(startX, 340, 745, 380, (Color){ 203, 213, 225, 255 });

    DrawRectangle(startX, 340, 745, 35, (Color){ 241, 245, 249, 255 });
    DrawText("Ma Phien", startX + 15, 350, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Khach Hang", startX + 160, 350, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Tram Sac", startX + 310, 350, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Cong", startX + 460, 350, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Thoi Gian Bat Dau", startX + 540, 350, 15, (Color){ 71, 85, 105, 255 });

    int py = 385;
    int liveCount = 0;
    for (auto& s : sem.getSessions()) {
        if (s.getIsActive()) {
            liveCount++;
            Customer* c = um.getCustomerById(s.getCustomerId());
            string custName = c ? c->getName() : s.getCustomerId();
            ChargingStation* st = sm.getStationById(s.getStationId());
            string stName = st ? st->getName() : s.getStationId();

            DrawText(s.getSessionId().c_str(), startX + 15, py, 14, (Color){ 15, 23, 42, 255 });
            DrawText(custName.c_str(), startX + 160, py, 14, (Color){ 15, 23, 42, 255 });
            DrawText(stName.c_str(), startX + 310, py, 14, (Color){ 15, 23, 42, 255 });
            DrawText(s.getPortId().c_str(), startX + 460, py, 14, (Color){ 217, 119, 6, 255 });
            DrawText(formatTime(s.getStartTime()).c_str(), startX + 540, py, 14, (Color){ 100, 116, 139, 255 });

            py += 30;
            if (py > 690) break;
        }
    }
    if (liveCount == 0) {
        DrawText("Hien khong co phien sac nao dang dien ra.", startX + 220, 480, 16, (Color){ 148, 163, 184, 255 });
    }
}

// =====================================================================
// 3. ADMIN: QUAN LY TRAM SAC & CONG SAC
// =====================================================================

void AppUI::DrawAdminStations() {
    DrawAdminSidebar();

    int startX = 245;
    DrawText("QUAN LY TRAM SAC & CONG SAC", startX, 25, 24, (Color){ 15, 23, 42, 255 });

    // Nut Mo Form Them Tram Moi
    if (GuiButton((Rectangle){ (float)startX + 560, 20, 185, 35 }, "+ THEM TRAM SAC MOI")) {
        showAddStationForm = !showAddStationForm;
        showAddPortForm = false;
        stationStatusMsg = "";
    }

    if (!stationStatusMsg.empty()) {
        DrawText(stationStatusMsg.c_str(), startX, 58, 14, (stationStatusMsg.find("thanh cong") != string::npos) ? GREEN : RED);
    }

    // FORM THÊM TRẠM SẠC MỚI
    if (showAddStationForm) {
        DrawRectangle(startX, 80, 745, 120, (Color){ 241, 245, 249, 255 });
        DrawRectangleLines(startX, 80, 745, 120, (Color){ 148, 163, 184, 255 });
        DrawText("NHAP THONG TIN TRAM SAC MOI:", startX + 15, 90, 15, (Color){ 30, 41, 59, 255 });

        DrawText("Ten tram:", startX + 15, 115, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 85, 110, 220, 28 }, newStationName, 128, activeStationName)) {
            activeStationName = !activeStationName;
            if (activeStationName) activeStationAddress = false;
        }

        DrawText("Dia chi:", startX + 325, 115, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 385, 110, 340, 28 }, newStationAddress, 128, activeStationAddress)) {
            activeStationAddress = !activeStationAddress;
            if (activeStationAddress) activeStationName = false;
        }

        if (GuiButton((Rectangle){ (float)startX + 480, 150, 130, 32 }, "Xac Nhan Them")) {
            if (strlen(newStationName) == 0 || strlen(newStationAddress) == 0) {
                stationStatusMsg = "Vui long nhap day du ten va dia chi tram!";
            } else {
                string newId = sm.generateNextStationId();
                ChargingStation st(newId, newStationName, newStationAddress);
                sm.addStation(st);
                autoSave(); // Luu ngay lap tuc ra tep CSV
                stationStatusMsg = "Them tram sac " + newId + " thanh cong!";
                memset(newStationName, 0, sizeof(newStationName));
                memset(newStationAddress, 0, sizeof(newStationAddress));
                showAddStationForm = false;
            }
        }
        if (GuiButton((Rectangle){ (float)startX + 625, 150, 100, 32 }, "Huy")) {
            showAddStationForm = false;
        }
    }

    // FORM THÊM CỔNG SẠC MỚI VÀO TRẠM CỤ THỂ
    if (showAddPortForm) {
        DrawRectangle(startX, 80, 745, 130, (Color){ 254, 243, 199, 255 });
        DrawRectangleLines(startX, 80, 745, 130, (Color){ 217, 119, 6, 255 });
        DrawText(TextFormat("THEM CONG SAC CHO TRAM: %s", targetStationIdForPort.c_str()), startX + 15, 90, 15, (Color){ 180, 83, 9, 255 });

        DrawText("Loai cong:", startX + 15, 118, 14, DARKGRAY);
        if (GuiButton((Rectangle){ (float)startX + 90, 115, 105, 26 }, selectedPortType == 0 ? "[AC Standard]" : "AC Standard")) selectedPortType = 0;
        if (GuiButton((Rectangle){ (float)startX + 200, 115, 90, 26 }, selectedPortType == 1 ? "[DC Fast]" : "DC Fast")) selectedPortType = 1;
        if (GuiButton((Rectangle){ (float)startX + 295, 115, 125, 26 }, selectedPortType == 2 ? "[Supercharge]" : "Supercharge")) selectedPortType = 2;

        DrawText("Cong suat (kW):", startX + 435, 118, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 550, 115, 60, 26 }, newPortPower, 32, activePortPower)) {
            activePortPower = !activePortPower;
            if (activePortPower) activePortPrice = false;
        }

        DrawText("Gia/kWh:", startX + 620, 118, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 680, 115, 55, 26 }, newPortPrice, 32, activePortPrice)) {
            activePortPrice = !activePortPrice;
            if (activePortPrice) activePortPower = false;
        }

        if (GuiButton((Rectangle){ (float)startX + 480, 160, 130, 32 }, "Xac Nhan Them")) {
            float pwr = (float)atof(newPortPower);
            float prc = (float)atof(newPortPrice);
            if (pwr <= 0) pwr = 22.0f;
            if (prc <= 0) prc = 3500.0f;

            string portId = sm.generateNextPortId(targetStationIdForPort);
            ChargingPort cp(portId, (PortType)selectedPortType, pwr, prc, AVAILABLE, "");
            sm.addPortToStation(targetStationIdForPort, cp);
            autoSave(); // Luu ngay lap tuc ra ports.csv de nho trang thai cho lan sau
            stationStatusMsg = "Them cong " + portId + " vao tram " + targetStationIdForPort + " thanh cong!";
            showAddPortForm = false;
        }
        if (GuiButton((Rectangle){ (float)startX + 625, 160, 100, 32 }, "Huy")) {
            showAddPortForm = false;
        }
    }

    // DANH SÁCH BẢNG CÁC TRẠM SẠC VÀ CÁC CỔNG
    int listStartY = (showAddStationForm || showAddPortForm) ? 225 : 85;
    int y = listStartY;

    for (auto& st : sm.getStations()) {
        if (y > 710) break;
        // Hop tieu de tram sac
        DrawRectangle(startX, y, 745, 36, (Color){ 226, 232, 240, 255 });
        DrawRectangleLines(startX, y, 745, 36, (Color){ 203, 213, 225, 255 });

        DrawText(TextFormat("[%s] %s", st.getStationId().c_str(), st.getName().c_str()), startX + 15, y + 9, 16, (Color){ 15, 23, 42, 255 });
        DrawText(TextFormat("Dia chi: %s", st.getAddress().c_str()), startX + 220, y + 10, 14, (Color){ 71, 85, 105, 255 });

        // Nut Them cong sac vao tram
        if (GuiButton((Rectangle){ (float)startX + 520, (float)y + 5, 105, 26 }, "+ Them Cong")) {
            targetStationIdForPort = st.getStationId();
            showAddPortForm = true;
            showAddStationForm = false;
            stationStatusMsg = "";
        }

        // Nut Xoa tram (Chi xoa duoc khi khong co cong nao dang sac)
        if (GuiButton((Rectangle){ (float)startX + 635, (float)y + 5, 95, 26 }, "Xoa Tram")) {
            if (sm.deleteStation(st.getStationId())) {
                autoSave();
                stationStatusMsg = "Xoa tram " + st.getStationId() + " thanh cong!";
                break;
            } else {
                stationStatusMsg = "Khong the xoa tram vi dang co cong dang duoc sac!";
            }
        }
        y += 42;

        // Danh sach cac cong sac cua tram
        if (st.getPorts().empty()) {
            DrawText("  (Tram chua co cong sac nao. Nhan '+ Them Cong' de bo sung)", startX + 25, y + 5, 14, (Color){ 148, 163, 184, 255 });
            y += 30;
        } else {
            for (auto& p : st.getPorts()) {
                if (y > 720) break;
                DrawRectangle(startX + 20, y, 725, 32, WHITE);
                DrawRectangleLines(startX + 20, y, 725, 32, (Color){ 241, 245, 249, 255 });

                string typeStr = (p.getType() == AC_STANDARD) ? "AC Standard" : ((p.getType() == DC_FAST) ? "DC Fast" : "DC Supercharge");
                string statusStr = (p.getStatus() == AVAILABLE) ? "TRONG (Available)" : ((p.getStatus() == OCCUPIED) ? "DANG SAC" : "BAO TRI (Maintenance)");
                Color stColor = (p.getStatus() == AVAILABLE) ? (Color){ 22, 163, 74, 255 } : ((p.getStatus() == OCCUPIED) ? (Color){ 217, 119, 6, 255 } : (Color){ 220, 38, 38, 255 });

                DrawText(TextFormat("Cong %s: %s | %.0f kW | %.0f d/kWh", p.getPortId().c_str(), typeStr.c_str(), p.getPowerKW(), p.getPricePerKWh()), startX + 35, y + 8, 14, (Color){ 30, 41, 59, 255 });
                DrawText(statusStr.c_str(), startX + 380, y + 8, 14, stColor);

                // Nut Chuyen doi trang thai Bao tri / Hoat dong
                if (p.getStatus() != OCCUPIED) {
                    const char* btnLabel = (p.getStatus() == AVAILABLE) ? "Dat Bao Tri" : "Mo Hoat Dong";
                    if (GuiButton((Rectangle){ (float)startX + 550, (float)y + 3, 100, 24 }, btnLabel)) {
                        PortStatus newSt = (p.getStatus() == AVAILABLE) ? MAINTENANCE : AVAILABLE;
                        sm.updatePortStatus(st.getStationId(), p.getPortId(), newSt);
                        autoSave();
                    }
                    if (GuiButton((Rectangle){ (float)startX + 660, (float)y + 3, 70, 24 }, "Xoa")) {
                        sm.deletePort(st.getStationId(), p.getPortId());
                        autoSave();
                        break;
                    }
                } else {
                    DrawText("[Dang phuc vu xe sac]", startX + 560, y + 8, 13, (Color){ 217, 119, 6, 255 });
                }
                y += 34;
            }
        }
        y += 8;
    }
}

// =====================================================================
// 4. ADMIN: QUAN LY KHACH HANG & NAP TIEN
// =====================================================================

void AppUI::DrawAdminCustomers() {
    DrawAdminSidebar();

    int startX = 245;
    DrawText("QUAN LY KHACH HANG & TAI KHOAN", startX, 25, 24, (Color){ 15, 23, 42, 255 });

    // Thanh Tim kiem
    DrawText("Tim kiem (Ten / SDT / Bien so):", startX, 68, 14, DARKGRAY);
    if (GuiTextBox((Rectangle){ (float)startX + 225, 62, 280, 30 }, customerSearch, 128, activeCustomerSearch)) {
        activeCustomerSearch = !activeCustomerSearch;
    }

    // Nut Mo Form Them Khach Hang Moi
    if (GuiButton((Rectangle){ (float)startX + 520, 60, 225, 34 }, "+ THEM TAI KHOAN KHACH")) {
        showAddCustomerForm = !showAddCustomerForm;
        customerStatusMsg = "";
    }

    if (!customerStatusMsg.empty()) {
        DrawText(customerStatusMsg.c_str(), startX, 98, 14, (customerStatusMsg.find("thanh cong") != string::npos) ? GREEN : RED);
    }

    // FORM THÊM TÀI KHOẢN KHÁCH HÀNG MỚI
    if (showAddCustomerForm) {
        DrawRectangle(startX, 118, 745, 175, (Color){ 241, 245, 249, 255 });
        DrawRectangleLines(startX, 118, 745, 175, (Color){ 148, 163, 184, 255 });
        DrawText("THONG TIN TAI KHOAN KHACH HANG MOI:", startX + 15, 128, 15, (Color){ 30, 41, 59, 255 });

        // Hang 1: Username, Password, Ho Ten
        DrawText("Username:", startX + 15, 155, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 95, 150, 120, 26 }, newCustUsername, 64, activeCustField == 1)) {
            activeCustField = (activeCustField == 1) ? 0 : 1;
        }

        DrawText("Password:", startX + 230, 155, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 305, 150, 120, 26 }, newCustPassword, 64, activeCustField == 2)) {
            activeCustField = (activeCustField == 2) ? 0 : 2;
        }

        DrawText("Ho Ten:", startX + 440, 155, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 500, 150, 230, 26 }, newCustName, 64, activeCustField == 3)) {
            activeCustField = (activeCustField == 3) ? 0 : 3;
        }

        // Hang 2: SDT, Bien so, Dong xe, Dung luong pin
        DrawText("SDT:", startX + 15, 195, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 95, 190, 120, 26 }, newCustPhone, 32, activeCustField == 4)) {
            activeCustField = (activeCustField == 4) ? 0 : 4;
        }

        DrawText("Bien so xe:", startX + 230, 195, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 305, 190, 120, 26 }, newCustPlate, 32, activeCustField == 5)) {
            activeCustField = (activeCustField == 5) ? 0 : 5;
        }

        DrawText("Dong xe:", startX + 440, 195, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 500, 190, 85, 26 }, newCustCarModel, 32, activeCustField == 6)) {
            activeCustField = (activeCustField == 6) ? 0 : 6;
        }

        DrawText("Pin kWh:", startX + 595, 195, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 665, 190, 65, 26 }, newCustBattery, 32, activeCustField == 7)) {
            activeCustField = (activeCustField == 7) ? 0 : 7;
        }

        // Hang 3: So du vi khoi tao & Nut bam
        DrawText("So du ban dau (VND):", startX + 15, 235, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 165, 230, 140, 26 }, newCustBalance, 32, activeCustField == 8)) {
            activeCustField = (activeCustField == 8) ? 0 : 8;
        }

        if (GuiButton((Rectangle){ (float)startX + 480, 230, 140, 32 }, "Xac Nhan Tao")) {
            if (strlen(newCustUsername) == 0 || strlen(newCustPassword) == 0 || strlen(newCustName) == 0) {
                customerStatusMsg = "Vui long nhap it nhat Username, Password va Ho Ten!";
            } else if (um.usernameExists(newCustUsername)) {
                customerStatusMsg = "Ten dang nhap nay da ton tai trong he thong!";
            } else {
                string nextId = um.generateNextCustomerId();
                float bat = (float)atof(newCustBattery);
                if (bat <= 0) bat = 75.0f;
                float bal = (float)atof(newCustBalance);
                if (bal < 0) bal = 0.0f;

                Customer c(nextId, newCustUsername, newCustPassword, newCustName,
                           newCustPhone, newCustPlate, newCustCarModel, bat, bal);
                um.addCustomer(c);
                autoSave(); // Luu ngay vao customers.csv
                customerStatusMsg = "Tao tai khoan " + string(newCustUsername) + " (Ma: " + nextId + ") thanh cong!";
                showAddCustomerForm = false;
                memset(newCustUsername, 0, sizeof(newCustUsername));
                memset(newCustPassword, 0, sizeof(newCustPassword));
                memset(newCustName, 0, sizeof(newCustName));
            }
        }
        if (GuiButton((Rectangle){ (float)startX + 630, 230, 100, 32 }, "Huy")) {
            showAddCustomerForm = false;
        }
    }

    // DANH SÁCH BẢNG KHÁCH HÀNG
    int tableStartY = showAddCustomerForm ? 310 : 115;
    DrawRectangle(startX, tableStartY, 745, 430, WHITE);
    DrawRectangleLines(startX, tableStartY, 745, 430, (Color){ 203, 213, 225, 255 });

    // Header Bang
    DrawRectangle(startX, tableStartY, 745, 34, (Color){ 241, 245, 249, 255 });
    DrawText("Ma KH", startX + 10, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Ho Ten", startX + 70, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("SDT", startX + 210, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Bien So", startX + 310, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Dong Xe", startX + 410, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("So Du Vi", startX + 490, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Thao Tac", startX + 635, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });

    string kw = toLower(customerSearch);
    int rowY = tableStartY + 42;
    for (auto& c : um.getCustomers()) {
        if (rowY > 730) break;
        // Bo loc tim kiem theo Tu khoa
        if (!kw.empty()) {
            if (toLower(c.getName()).find(kw) == string::npos &&
                toLower(c.getPhone()).find(kw) == string::npos &&
                toLower(c.getLicensePlate()).find(kw) == string::npos) {
                continue;
            }
        }

        DrawText(c.getId().c_str(), startX + 10, rowY, 14, (Color){ 15, 23, 42, 255 });
        DrawText(c.getName().c_str(), startX + 70, rowY, 14, (Color){ 15, 23, 42, 255 });
        DrawText(c.getPhone().c_str(), startX + 210, rowY, 14, (Color){ 100, 116, 139, 255 });
        DrawText(c.getLicensePlate().c_str(), startX + 310, rowY, 14, (Color){ 15, 23, 42, 255 });
        DrawText(c.getCarModel().c_str(), startX + 410, rowY, 14, (Color){ 71, 85, 105, 255 });
        DrawText(formatMoney(c.getBalance()).c_str(), startX + 490, rowY, 14, (Color){ 22, 163, 74, 255 });

        // Nut Nạp tiền cho khách (Da thay the nut Khoa tai khoan theo yeu cau)
        if (GuiButton((Rectangle){ (float)startX + 635, (float)rowY - 3, 95, 24 }, "+ Nap Tien")) {
            showTopupModal = true;
            topupTargetCustomerId = c.getId();
            adminTopupMsg = "";
            strcpy(adminTopupAmount, "100000");
        }

        rowY += 32;
    }
}

// =====================================================================
// MODAL HOP THOAI NAP TIEN CHO KHACH HANG (ADMIN TOPUP MODAL)
// =====================================================================

void AppUI::DrawAdminTopupModal() {
    // Lop phu mo toi toan man hinh
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 160 });

    Customer* c = um.getCustomerById(topupTargetCustomerId);
    if (!c) {
        showTopupModal = false;
        return;
    }

    // Khung modal trung tam
    int modalX = 280;
    int modalY = 180;
    int modalW = 480;
    int modalH = 380;

    DrawRectangleRounded((Rectangle){ (float)modalX, (float)modalY, (float)modalW, (float)modalH }, 0.05f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ (float)modalX, (float)modalY, (float)modalW, (float)modalH }, 0.05f, 4, (Color){ 203, 213, 225, 255 });

    // Tieu de
    DrawRectangleRounded((Rectangle){ (float)modalX, (float)modalY, (float)modalW, 55 }, 0.05f, 4, (Color){ 30, 58, 138, 255 });
    DrawText("💳 NAP TIEN CHO KHACH HANG", modalX + 90, modalY + 18, 20, WHITE);

    // Thong tin khach hang
    DrawText(TextFormat("Khach hang: %s  (Ma: %s)", c->getName().c_str(), c->getId().c_str()), modalX + 30, modalY + 75, 16, (Color){ 15, 23, 42, 255 });
    DrawText(TextFormat("So du hien tai: %s", formatMoney(c->getBalance()).c_str()), modalX + 30, modalY + 105, 16, (Color){ 22, 163, 74, 255 });

    // Cac nut chon nhanh menh gia
    DrawText("Chon nhanh menh gia:", modalX + 30, modalY + 140, 14, DARKGRAY);
    if (GuiButton((Rectangle){ (float)modalX + 30, (float)modalY + 165, 95, 35 }, "+ 50k")) {
        um.topupCustomerBalance(c->getId(), 50000.0f);
        autoSave();
        adminTopupMsg = "Nap thanh cong 50,000 VND!";
    }
    if (GuiButton((Rectangle){ (float)modalX + 135, (float)modalY + 165, 95, 35 }, "+ 100k")) {
        um.topupCustomerBalance(c->getId(), 100000.0f);
        autoSave();
        adminTopupMsg = "Nap thanh cong 100,000 VND!";
    }
    if (GuiButton((Rectangle){ (float)modalX + 240, (float)modalY + 165, 95, 35 }, "+ 200k")) {
        um.topupCustomerBalance(c->getId(), 200000.0f);
        autoSave();
        adminTopupMsg = "Nap thanh cong 200,000 VND!";
    }
    if (GuiButton((Rectangle){ (float)modalX + 345, (float)modalY + 165, 105, 35 }, "+ 500k")) {
        um.topupCustomerBalance(c->getId(), 500000.0f);
        autoSave();
        adminTopupMsg = "Nap thanh cong 500,000 VND!";
    }

    // Nhap so tien tuy y
    DrawText("Hoac nhap so tien tuy y (VND):", modalX + 30, modalY + 220, 14, DARKGRAY);
    if (GuiTextBox((Rectangle){ (float)modalX + 30, (float)modalY + 245, 250, 35 }, adminTopupAmount, 32, activeAdminTopup)) {
        activeAdminTopup = !activeAdminTopup;
    }

    if (GuiButton((Rectangle){ (float)modalX + 295, (float)modalY + 245, 155, 35 }, "Xac Nhan Nap")) {
        float val = (float)atof(adminTopupAmount);
        if (val > 0) {
            um.topupCustomerBalance(c->getId(), val);
            autoSave();
            adminTopupMsg = "Nap thanh cong " + formatMoney(val) + "!";
        } else {
            adminTopupMsg = "Vui long nhap so tien hop le!";
        }
    }

    if (!adminTopupMsg.empty()) {
        DrawText(adminTopupMsg.c_str(), modalX + 30, modalY + 295, 15, (adminTopupMsg.find("thanh cong") != string::npos) ? GREEN : RED);
    }

    // Nut Dong Modal
    if (GuiButton((Rectangle){ (float)modalX + 160, (float)modalY + 325, 160, 38 }, "Hoan Tat / Dong")) {
        showTopupModal = false;
        adminTopupMsg = "";
    }
}

// =====================================================================
// 5. ADMIN: LICH SU HOA DON TOAN HE THONG
// =====================================================================

void AppUI::DrawAdminInvoices() {
    DrawAdminSidebar();

    int startX = 245;
    DrawText("LICH SU HOA DON TOAN HE THONG", startX, 25, 24, (Color){ 15, 23, 42, 255 });

    DrawRectangle(startX, 75, 745, 650, WHITE);
    DrawRectangleLines(startX, 75, 745, 650, (Color){ 203, 213, 225, 255 });

    DrawRectangle(startX, 75, 745, 36, (Color){ 241, 245, 249, 255 });
    DrawText("Ma Hoa Don", startX + 20, 85, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Ma Phien", startX + 180, 85, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Ma Khach Hang", startX + 330, 85, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Tong Tien (VND)", startX + 470, 85, 15, (Color){ 71, 85, 105, 255 });
    DrawText("Thoi Gian Lap", startX + 610, 85, 15, (Color){ 71, 85, 105, 255 });

    int y = 120;
    auto& invs = sem.getInvoices();
    if (invs.empty()) {
        DrawText("Chua co hoa don nao duoc ghi nhan trong he thong.", startX + 200, 300, 16, (Color){ 148, 163, 184, 255 });
    } else {
        for (int i = (int)invs.size() - 1; i >= 0; --i) {
            auto& inv = invs[i];
            if (y > 700) break;

            DrawText(inv.getInvoiceId().c_str(), startX + 20, y, 14, (Color){ 15, 23, 42, 255 });
            DrawText(inv.getSessionId().c_str(), startX + 180, y, 14, (Color){ 100, 116, 139, 255 });
            DrawText(inv.getCustomerId().c_str(), startX + 330, y, 14, (Color){ 15, 23, 42, 255 });
            DrawText(formatMoney(inv.getTotalAmount()).c_str(), startX + 470, y, 14, (Color){ 22, 163, 74, 255 });
            DrawText(formatTime(inv.getDate()).c_str(), startX + 610, y, 14, (Color){ 100, 116, 139, 255 });

            y += 30;
        }
    }
}

// =====================================================================
// 6. GIAO DIEN MENU KHACH HANG (CUSTOMER MENU)
// =====================================================================

void AppUI::DrawCustomerMenu() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    // Header hien thi: ID, Ten, Xe, Bien so va So du o goc tren cung
    DrawRectangle(0, 0, GetScreenWidth(), 70, (Color){ 30, 58, 138, 255 });
    DrawText("⚡ EVC CHARGING", 25, 22, 24, (Color){ 56, 189, 248, 255 });

    DrawText(TextFormat("👤 %s  |  Ma: %s  |  Xe: %s (%s)", c->getName().c_str(), c->getId().c_str(), c->getCarModel().c_str(), c->getLicensePlate().c_str()), 220, 26, 15, WHITE);

    // O hien thi so du tai khoan noi bat
    DrawRectangleRounded((Rectangle){ 700, 15, 210, 40 }, 0.2f, 4, (Color){ 22, 101, 52, 255 });
    DrawText(TextFormat("So du: %s", formatMoney(c->getBalance()).c_str()), 710, 27, 15, (Color){ 187, 247, 208, 255 });

    // Nut Dang Xuat
    if (GuiButton((Rectangle){ 925, 15, 85, 40 }, "Dang Xuat")) {
        um.logout();
        currentState = STATE_LOGIN;
        memset(loginUsername, 0, sizeof(loginUsername));
        memset(loginPassword, 0, sizeof(loginPassword));
        loginUsernameActive = false;
        loginPasswordActive = false;
        autoSave();
        return;
    }

    // Tabs chuyen doi: CHI CON 2 TAB (Da bo tab nap tien o khach theo yeu cau)
    DrawRectangle(0, 70, GetScreenWidth(), 45, (Color){ 241, 245, 249, 255 });
    if (GuiButton((Rectangle){ 25, 76, 260, 33 }, customerTab == 0 ? "[ 🔌 TRA CUU & SAC DIEN ]" : "🔌 Tra Cuu & Sac Dien")) {
        customerTab = 0;
        customerErrorMsg = "";
    }
    if (GuiButton((Rectangle){ 300, 76, 260, 33 }, customerTab == 1 ? "[ 📜 LICH SU HOA DON ]" : "📜 Lich Su Hoa Don")) {
        customerTab = 1;
        customerErrorMsg = "";
    }

    // Thong bao loi neu co
    if (!customerErrorMsg.empty()) {
        DrawRectangle(25, 120, 974, 30, (Color){ 254, 226, 226, 255 });
        DrawText(customerErrorMsg.c_str(), 40, 127, 15, RED);
    }

    // -------------------------------------------------------------
    // TAB 0: TRA CỨU TRẠM SẠC & BẮT ĐẦU SẠC
    // -------------------------------------------------------------
    if (customerTab == 0) {
        int y = customerErrorMsg.empty() ? 130 : 160;
        DrawText("DANH SACH TRAC CUU TRAM SAC & CONG SAC KHA DUNG:", 25, y, 17, (Color){ 30, 41, 59, 255 });
        y += 30;

        for (auto& st : sm.getStations()) {
            if (y > 710) break;
            DrawRectangle(25, y, 974, 36, (Color){ 226, 232, 240, 255 });
            DrawRectangleLines(25, y, 974, 36, (Color){ 203, 213, 225, 255 });

            DrawText(TextFormat("📍 Tram: %s  -  Dia chi: %s", st.getName().c_str(), st.getAddress().c_str()), 40, y + 9, 16, (Color){ 15, 23, 42, 255 });
            y += 42;

            if (st.getPorts().empty()) {
                DrawText("  (Tram hien chua co cong sac nao)", 50, y + 5, 14, DARKGRAY);
                y += 28;
            } else {
                for (auto& p : st.getPorts()) {
                    if (y > 720) break;
                    DrawRectangle(45, y, 940, 36, WHITE);
                    DrawRectangleLines(45, y, 940, 36, (Color){ 241, 245, 249, 255 });

                    string typeStr = (p.getType() == AC_STANDARD) ? "AC Standard" : ((p.getType() == DC_FAST) ? "DC Fast" : "DC Supercharge");
                    string stStr = (p.getStatus() == AVAILABLE) ? "TRONG (Available)" : ((p.getStatus() == OCCUPIED) ? "DANG SAC" : "BAO TRI (Maintenance)");
                    Color stColor = (p.getStatus() == AVAILABLE) ? (Color){ 22, 163, 74, 255 } : ((p.getStatus() == OCCUPIED) ? (Color){ 217, 119, 6, 255 } : (Color){ 220, 38, 38, 255 });

                    DrawText(TextFormat("Tru %s: %s (%.0f kW)  -  Don gia: %.0f d/kWh", p.getPortId().c_str(), typeStr.c_str(), p.getPowerKW(), p.getPricePerKWh()), 60, y + 10, 14, (Color){ 15, 23, 42, 255 });
                    DrawText(stStr.c_str(), 520, y + 10, 14, stColor);

                    // Nut Bat Dau Sac (Chi kha dung khi cong AVAILABLE)
                    if (p.getStatus() == AVAILABLE) {
                        if (GuiButton((Rectangle){ 800, (float)y + 4, 160, 28 }, "BAT DAU SAC")) {
                            // 1. Kiem tra so du tai khoan
                            if (c->getBalance() < 20000.0f) {
                                customerErrorMsg = "So du tai khoan khong du (Toi thieu 20.000 VND de bat dau sac)! Vui long lien he Admin de nap tien.";
                            } else {
                                // 2. Khoi tao phien sac
                                if (sem.startSession(c->getId(), st.getStationId(), p.getPortId(), sm)) {
                                    customerErrorMsg = "";
                                    autoSave(); // Luu ngay trang thai dang sac
                                    currentState = STATE_CUSTOMER_CHARGING;
                                    return;
                                }
                            }
                        }
                    } else if (p.getStatus() == OCCUPIED) {
                        DrawText("(Dang co xe sac)", 820, y + 10, 13, (Color){ 217, 119, 6, 255 });
                    } else {
                        DrawText("(Tam ngung bao tri)", 820, y + 10, 13, (Color){ 220, 38, 38, 255 });
                    }

                    y += 40;
                }
            }
            y += 10;
        }
    }

    // -------------------------------------------------------------
    // TAB 1: LỊCH SỬ HÓA ĐƠN CỦA KHÁCH HÀNG
    // -------------------------------------------------------------
    else if (customerTab == 1) {
        DrawRectangle(25, 140, 974, 550, WHITE);
        DrawRectangleLines(25, 140, 974, 550, (Color){ 203, 213, 225, 255 });

        DrawText("LICH SU CAC HOA DON SAC CUA BAN", 40, 160, 20, (Color){ 30, 41, 59, 255 });

        DrawRectangle(40, 200, 940, 35, (Color){ 241, 245, 249, 255 });
        DrawText("Ma Hoa Don", 60, 210, 15, (Color){ 71, 85, 105, 255 });
        DrawText("Ma Phien Sac", 280, 210, 15, (Color){ 71, 85, 105, 255 });
        DrawText("So Tien Thanh Toan", 520, 210, 15, (Color){ 71, 85, 105, 255 });
        DrawText("Thoi Gian Lap", 760, 210, 15, (Color){ 71, 85, 105, 255 });

        auto myInvs = sem.getInvoicesByCustomer(c->getId());
        if (myInvs.empty()) {
            DrawText("Ban chua co hoa don sac nao trong he thong.", 380, 330, 16, (Color){ 148, 163, 184, 255 });
        } else {
            int y = 245;
            for (int i = (int)myInvs.size() - 1; i >= 0; --i) {
                if (y > 650) break;
                auto& inv = myInvs[i];
                DrawText(inv.getInvoiceId().c_str(), 60, y, 14, (Color){ 15, 23, 42, 255 });
                DrawText(inv.getSessionId().c_str(), 280, y, 14, (Color){ 100, 116, 139, 255 });
                DrawText(formatMoney(inv.getTotalAmount()).c_str(), 520, y, 14, (Color){ 22, 163, 74, 255 });
                DrawText(formatTime(inv.getDate()).c_str(), 760, y, 14, (Color){ 100, 116, 139, 255 });
                y += 32;
            }
        }
    }
}

// =====================================================================
// 7. MAN HINH DANG SAC TRUC TIEP (LIVE CHARGING SCREEN)
// =====================================================================

void AppUI::DrawCustomerCharging() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    ChargingSession* ses = sem.getActiveSession(c->getId());
    if (!ses) {
        currentState = STATE_CUSTOMER_MENU;
        return;
    }

    ChargingStation* st = sm.getStationById(ses->getStationId());
    string stName = st ? st->getName() : ses->getStationId();
    string stAddr = st ? st->getAddress() : "";

    float pwr = 22.0f;
    float prc = 3500.0f;
    if (st) {
        ChargingPort* p = st->getPortById(ses->getPortId());
        if (p) {
            pwr = p->getPowerKW();
            prc = p->getPricePerKWh();
        }
    }

    long long durationSec = time(0) - ses->getStartTime();
    if (durationSec < 0) durationSec = 0;
    int mins = (int)(durationSec / 60);
    int secs = (int)(durationSec % 60);

    // Tinh dien nang tieu thu (Mo phong 1 giay thuc = 1 phut sac)
    float simulatedHours = (durationSec * 60.0f) / 3600.0f;
    float currentKWh = pwr * simulatedHours;
    if (currentKWh < 0.2f) currentKWh = 0.2f;
    float currentCost = currentKWh * prc;

    // Nen toi hien dai cho man hinh dang sac
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 15, 23, 42, 255 });

    // Tieu de
    DrawText("⚡ XE CUA BAN DANG DUOC SAC DIEN", 300, 45, 26, (Color){ 56, 189, 248, 255 });
    DrawText(TextFormat("Tram: %s  |  Dia chi: %s  |  Cong sac: %s (%.0f kW)", stName.c_str(), stAddr.c_str(), ses->getPortId().c_str(), pwr), 240, 85, 15, (Color){ 203, 213, 225, 255 });

    // Khung dong ho hien thi thong so
    DrawRectangleRounded((Rectangle){ 180, 130, 664, 460 }, 0.05f, 4, (Color){ 30, 41, 59, 255 });

    // THANH TRẠNG THÁI ĐANG SẠC (Charging Status Animation)
    // (Da loai bo toan bo phan tinh toan % pin theo yeu cau, thay bang thanh hieu ung dang sac)
    Rectangle barRect = { 240, 165, 544, 45 };
    DrawRectangleRounded(barRect, 0.2f, 4, (Color){ 15, 23, 42, 255 });
    DrawRectangleRoundedLines(barRect, 0.2f, 4, (Color){ 56, 189, 248, 255 });

    // Hieu ung luong dien chay nhe nhang (pulsing wave)
    float waveOffset = fmodf((float)GetTime() * 120.0f, 544.0f);
    DrawRectangleRounded((Rectangle){ 240 + waveOffset * 0.5f, 167, 120, 41 }, 0.2f, 4, (Color){ 34, 197, 94, 120 });

    // Chu trang thai sac
    DrawText("⚡ DANG SAC DIEN (CHARGING IN PROGRESS)...", 310, 178, 18, (Color){ 74, 222, 128, 255 });

    // Thoi gian da sac
    DrawText("Thoi gian da sac:", 240, 240, 16, (Color){ 148, 163, 184, 255 });
    DrawText(TextFormat("%02d phut %02d giay", mins, secs), 240, 265, 28, WHITE);

    // Dien nang da nap (kWh)
    DrawText("Dien nang da nap:", 540, 240, 16, (Color){ 148, 163, 184, 255 });
    DrawText(TextFormat("%.2f kWh", currentKWh), 540, 265, 28, (Color){ 56, 189, 248, 255 });

    // Chi phi tam tinh
    DrawText("Chi phi tam tinh:", 240, 335, 16, (Color){ 148, 163, 184, 255 });
    DrawText(formatMoney(currentCost).c_str(), 240, 360, 28, (Color){ 250, 204, 21, 255 });

    // So du vi hien tai
    DrawText("So du vi hien tai:", 540, 335, 16, (Color){ 148, 163, 184, 255 });
    DrawText(formatMoney(c->getBalance()).c_str(), 540, 360, 28, (Color){ 74, 222, 128, 255 });

    // Canh bao neu chi phi vuot qua so du
    if (currentCost > c->getBalance()) {
        DrawText("⚠️ Chi phi da vuot qua so du! Vui long dung sac de thanh toan.", 240, 420, 15, RED);
    } else {
        DrawText("Giu an toan - Khong ngat day sac khi chua bam dung sac tren man hinh.", 250, 420, 14, (Color){ 148, 163, 184, 255 });
    }

    // Nut Dung Sac & Thanh Toan
    if (GuiButton((Rectangle){ 330, 480, 364, 55 }, "🛑 DUNG SAC & THANH TOAN")) {
        // Luu thong tin de hien thi tren hoa don
        receiptStationName = stName;
        receiptPortId = ses->getPortId();
        receiptKWh = currentKWh;
        receiptCost = currentCost;

        if (sem.endSession(c->getId(), sm, um)) {
            receiptRemainingBal = c->getBalance();
            Invoice* lastInv = sem.getLastInvoice(c->getId());
            receiptInvoiceId = lastInv ? lastInv->getInvoiceId() : "INV_SUCCESS";
            autoSave(); // Luu ngay lap tuc sau khi dung sac va tru tien
            currentState = STATE_CUSTOMER_RECEIPT;
        }
    }
}

// =====================================================================
// 8. MAN HINH BIEN LAI HOA DON (RECEIPT SCREEN)
// =====================================================================

void AppUI::DrawCustomerReceipt() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 241, 245, 249, 255 });

    // Card Hoa don thanh toan
    DrawRectangleRounded((Rectangle){ 262, 100, 500, 530 }, 0.05f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ 262, 100, 500, 530 }, 0.05f, 4, (Color){ 203, 213, 225, 255 });

    // Header Hoa don
    DrawRectangle(262, 100, 500, 80, (Color){ 22, 101, 52, 255 });
    DrawText("✅ THANH TOAN THANH CONG!", 350, 125, 22, WHITE);
    DrawText("Hoa Don Dien Tu - PBL2 EV Station", 380, 152, 14, (Color){ 187, 247, 208, 255 });

    int y = 210;
    DrawText(TextFormat("Ma hoa don: %s", receiptInvoiceId.c_str()), 290, y, 16, (Color){ 15, 23, 42, 255 }); y += 35;
    DrawText(TextFormat("Khach hang: %s (%s)", c->getName().c_str(), c->getId().c_str()), 290, y, 16, (Color){ 15, 23, 42, 255 }); y += 35;
    DrawText(TextFormat("Bien so xe: %s  -  Xe: %s", c->getLicensePlate().c_str(), c->getCarModel().c_str()), 290, y, 16, (Color){ 15, 23, 42, 255 }); y += 35;
    DrawText(TextFormat("Tram sac: %s  (Cong %s)", receiptStationName.c_str(), receiptPortId.c_str()), 290, y, 16, (Color){ 15, 23, 42, 255 }); y += 35;
    DrawText(TextFormat("Dien nang nap: %.2f kWh", receiptKWh), 290, y, 16, (Color){ 15, 23, 42, 255 }); y += 35;

    DrawLine(290, y, 730, y, (Color){ 203, 213, 225, 255 }); y += 20;

    DrawText("TONG TIEN DA TRU:", 290, y, 16, (Color){ 185, 28, 28, 255 });
    DrawText(formatMoney(receiptCost).c_str(), 520, y, 18, (Color){ 185, 28, 28, 255 }); y += 35;

    DrawText("So du vi con lai:", 290, y, 16, (Color){ 21, 128, 61, 255 });
    DrawText(formatMoney(receiptRemainingBal).c_str(), 520, y, 18, (Color){ 21, 128, 61, 255 }); y += 50;

    if (GuiButton((Rectangle){ 362, (float)y, 300, 45 }, "QUAY LAI MENU CHINH")) {
        currentState = STATE_CUSTOMER_MENU;
    }
}
