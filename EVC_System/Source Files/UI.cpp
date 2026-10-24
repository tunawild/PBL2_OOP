#include "UI.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <string.h>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

// =====================================================================
// Helper formatting functions
// =====================================================================
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

static string formatTime(long long ts) {
    if (ts <= 0) return "--:--";
    time_t t = (time_t)ts;
    struct tm* tm_info = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", tm_info);
    return string(buf);
}

static string toLower(const string& str) {
    string res = str;
    for (char &c : res) c = (char)tolower((unsigned char)c);
    return res;
}

// =====================================================================
// AppUI Implementation
// =====================================================================
AppUI::AppUI(UserManager& u, StationManager& s, SessionManager& se)
    : um(u), sm(s), sem(se), currentState(STATE_LOGIN),
      loginUsernameActive(false), loginPasswordActive(false), loginError(false), loginErrorMsg(""),
      adminMenuSelected(0),
      activeStationName(false), activeStationAddress(false), stationStatusMsg(""), showAddStationForm(false),
      targetStationIdForPort(""), selectedPortType(0), activePortPower(false), activePortPrice(false), showAddPortForm(false),
      activeCustomerSearch(false), showAddCustomerForm(false), activeCustField(0), customerStatusMsg(""),
      customerTab(0), customerErrorMsg(""), activeTopupAmount(false), topupStatusMsg(""),
      receiptInvoiceId(""), receiptStationName(""), receiptPortId(""), receiptKWh(0.0f), receiptCost(0.0f), receiptRemainingBal(0.0f)
{
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
    memset(topupAmountStr, 0, sizeof(topupAmountStr));

    strcpy(newPortPower, "22");
    strcpy(newPortPrice, "3500");
    strcpy(newCustBattery, "80");
    strcpy(newCustBalance, "500000");
    strcpy(topupAmountStr, "100000");
}

void AppUI::Init() {
    GuiLoadStyleDefault();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
}

void AppUI::UpdateDrawFrame() {
    BeginDrawing();
    ClearBackground((Color){ 245, 247, 250, 255 });

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

    EndDrawing();
}

// =====================================================================
// MÀN HÌNH ĐĂNG NHẬP (LOGIN)
// =====================================================================
void AppUI::DrawLogin() {
    // Header Banner
    DrawRectangle(0, 0, GetScreenWidth(), 120, (Color){ 30, 58, 138, 255 });
    DrawText("EV CHARGING STATION MANAGEMENT", 240, 35, 30, WHITE);
    DrawText("HE THONG QUAN LY TRAM SAC XE DIEN (PBL2 - OOP)", 310, 75, 18, (Color){ 191, 219, 254, 255 });

    // Login Card
    DrawRectangleRounded((Rectangle){ 280, 180, 464, 430 }, 0.05f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ 280, 180, 464, 430 }, 0.05f, 4, (Color){ 203, 213, 225, 255 });
    
    DrawText("DANG NHAP HE THONG", 380, 210, 22, (Color){ 15, 23, 42, 255 });

    // Username input
    DrawText("Ten dang nhap (Username):", 330, 260, 16, (Color){ 71, 85, 105, 255 });
    if (GuiTextBox((Rectangle){ 330, 285, 364, 38 }, loginUsername, 64, loginUsernameActive)) {
        loginUsernameActive = !loginUsernameActive;
        if (loginUsernameActive) loginPasswordActive = false;
    }

    // Password input
    DrawText("Mat khau (Password):", 330, 340, 16, (Color){ 71, 85, 105, 255 });
    if (GuiTextBox((Rectangle){ 330, 365, 364, 38 }, loginPassword, 64, loginPasswordActive)) {
        loginPasswordActive = !loginPasswordActive;
        if (loginPasswordActive) loginUsernameActive = false;
    }

    // Login Button
    if (GuiButton((Rectangle){ 330, 435, 364, 45 }, "DANG NHAP")) {
        loginUsernameActive = false;
        loginPasswordActive = false;
        if (um.login(loginUsername, loginPassword)) {
            loginError = false;
            loginErrorMsg = "";
            if (um.getLoggedInUser()->getRole() == ROLE_ADMIN) {
                currentState = STATE_ADMIN_DASHBOARD;
            } else {
                // Kiểm tra nếu khách này đang có phiên sạc dở dang thì chuyển thẳng vào màn hình sạc
                Customer* c = (Customer*)um.getLoggedInUser();
                if (c && sem.getActiveSession(c->getId()) != nullptr) {
                    currentState = STATE_CUSTOMER_CHARGING;
                } else {
                    currentState = STATE_CUSTOMER_MENU;
                }
            }
        } else {
            loginError = true;
            loginErrorMsg = "Sai tai khoan hoac mat khau (hoac tai khoan bi khoa)!";
        }
    }

    if (loginError) {
        DrawText(loginErrorMsg.c_str(), 310, 495, 15, RED);
    }

    // Account hints
    DrawRectangle(310, 525, 404, 60, (Color){ 241, 245, 249, 255 });
    DrawText("Tai khoan Admin: admin / admin123", 330, 535, 14, (Color){ 100, 116, 139, 255 });
    DrawText("Tai khoan Khach: khach1 / 123", 330, 555, 14, (Color){ 100, 116, 139, 255 });
}

// =====================================================================
// THANH ĐIỀU HƯỚNG ADMIN (SIDEBAR)
// =====================================================================
void AppUI::DrawAdminSidebar() {
    DrawRectangle(0, 0, 220, GetScreenHeight(), (Color){ 30, 41, 59, 255 });
    
    // Logo
    DrawRectangle(15, 20, 190, 50, (Color){ 15, 23, 42, 255 });
    DrawText("⚡ EVC ADMIN", 35, 32, 22, (Color){ 56, 189, 248, 255 });

    // Menu Buttons
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
    
    // Logout Button
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
// ADMIN DASHBOARD
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

    // 4 Stat Cards
    // Card 1: Tổng trạm
    DrawRectangleRounded((Rectangle){ (float)startX, 75, 175, 100 }, 0.1f, 4, (Color){ 224, 242, 254, 255 });
    DrawText("Tong so tram", startX + 15, 90, 15, (Color){ 3, 105, 161, 255 });
    DrawText(TextFormat("%d", totalStations), startX + 15, 115, 36, (Color){ 2, 132, 199, 255 });

    // Card 2: Cổng khả dụng (Available)
    DrawRectangleRounded((Rectangle){ (float)startX + 190, 75, 175, 100 }, 0.1f, 4, (Color){ 220, 252, 231, 255 });
    DrawText("Cong trong (Available)", startX + 205, 90, 15, (Color){ 21, 128, 61, 255 });
    DrawText(TextFormat("%d / %d", avail, totalPorts), startX + 205, 115, 36, (Color){ 22, 163, 74, 255 });

    // Card 3: Cổng đang sạc (Occupied)
    DrawRectangleRounded((Rectangle){ (float)startX + 380, 75, 175, 100 }, 0.1f, 4, (Color){ 254, 243, 199, 255 });
    DrawText("Dang sac (Occupied)", startX + 395, 90, 15, (Color){ 180, 83, 9, 255 });
    DrawText(TextFormat("%d", occ), startX + 395, 115, 36, (Color){ 217, 119, 6, 255 });

    // Card 4: Cổng bảo trì (Maintenance)
    DrawRectangleRounded((Rectangle){ (float)startX + 570, 75, 175, 100 }, 0.1f, 4, (Color){ 254, 226, 226, 255 });
    DrawText("Bao tri (Maintenance)", startX + 585, 90, 15, (Color){ 185, 28, 28, 255 });
    DrawText(TextFormat("%d", maint), startX + 585, 115, 36, (Color){ 220, 38, 38, 255 });

    // Revenue & Energy Banner
    DrawRectangleRounded((Rectangle){ (float)startX, 195, 365, 90 }, 0.1f, 4, (Color){ 238, 242, 255, 255 });
    DrawText("Tong doanh thu he thong", startX + 20, 210, 16, (Color){ 67, 56, 202, 255 });
    DrawText(formatMoney(revenue).c_str(), startX + 20, 235, 26, (Color){ 79, 70, 229, 255 });

    DrawRectangleRounded((Rectangle){ (float)startX + 380, 195, 365, 90 }, 0.1f, 4, (Color){ 243, 232, 255, 255 });
    DrawText("Tong dien nang tieu thu", startX + 400, 210, 16, (Color){ 126, 34, 206, 255 });
    DrawText(TextFormat("%.2f kWh", energy), startX + 400, 235, 26, (Color){ 147, 51, 234, 255 });

    // Danh sách các phiên sạc đang hoạt động (Live Sessions)
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
        if (s.isActive) {
            liveCount++;
            Customer* c = um.getCustomerById(s.customerId);
            string custName = c ? c->getName() : s.customerId;
            ChargingStation* st = sm.getStationById(s.stationId);
            string stName = st ? st->name : s.stationId;

            DrawText(s.sessionId.c_str(), startX + 15, py, 14, (Color){ 15, 23, 42, 255 });
            DrawText(custName.c_str(), startX + 160, py, 14, (Color){ 15, 23, 42, 255 });
            DrawText(stName.c_str(), startX + 310, py, 14, (Color){ 15, 23, 42, 255 });
            DrawText(s.portId.c_str(), startX + 460, py, 14, (Color){ 217, 119, 6, 255 });
            DrawText(formatTime(s.startTime).c_str(), startX + 540, py, 14, (Color){ 100, 116, 139, 255 });

            py += 30;
            if (py > 690) break;
        }
    }
    if (liveCount == 0) {
        DrawText("Hien khong co phien sac nao dang dien ra.", startX + 220, 480, 16, (Color){ 148, 163, 184, 255 });
    }
}

// =====================================================================
// ADMIN: QUẢN LÝ TRẠM SẠC & CỔNG SẠC
// =====================================================================
void AppUI::DrawAdminStations() {
    DrawAdminSidebar();

    int startX = 245;
    DrawText("QUAN LY TRAM SAC & CONG SAC", startX, 25, 24, (Color){ 15, 23, 42, 255 });

    // Nút mở form Thêm Trạm
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
                autoSave();
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
            autoSave();
            stationStatusMsg = "Them cong " + portId + " vao tram " + targetStationIdForPort + " thanh cong!";
            showAddPortForm = false;
        }
        if (GuiButton((Rectangle){ (float)startX + 625, 160, 100, 32 }, "Huy")) {
            showAddPortForm = false;
        }
    }

    // DANH SÁCH CÁC TRẠM SẠC VÀ CÁC CỔNG
    int listStartY = (showAddStationForm || showAddPortForm) ? 225 : 85;
    int y = listStartY;

    for (auto& st : sm.getStations()) {
        if (y > 710) break;
        // Hộp thông tin trạm
        DrawRectangle(startX, y, 745, 36, (Color){ 226, 232, 240, 255 });
        DrawRectangleLines(startX, y, 745, 36, (Color){ 203, 213, 225, 255 });

        DrawText(TextFormat("[%s] %s", st.stationId.c_str(), st.name.c_str()), startX + 15, y + 9, 16, (Color){ 15, 23, 42, 255 });
        DrawText(TextFormat("Dia chi: %s", st.address.c_str()), startX + 220, y + 10, 14, (Color){ 71, 85, 105, 255 });

        // Nút thêm cổng cho trạm này
        if (GuiButton((Rectangle){ (float)startX + 520, (float)y + 5, 105, 26 }, "+ Them Cong")) {
            targetStationIdForPort = st.stationId;
            showAddPortForm = true;
            showAddStationForm = false;
            stationStatusMsg = "";
        }

        // Nút xóa trạm (chỉ xóa khi không có cổng nào đang sạc)
        if (GuiButton((Rectangle){ (float)startX + 635, (float)y + 5, 95, 26 }, "Xoa Tram")) {
            if (sm.deleteStation(st.stationId)) {
                autoSave();
                stationStatusMsg = "Xoa tram " + st.stationId + " thanh cong!";
                break;
            } else {
                stationStatusMsg = "Khong the xoa tram vi dang co cong dang duoc sac!";
            }
        }
        y += 42;

        // Danh sách các cổng của trạm
        if (st.ports.empty()) {
            DrawText("  (Tram chua co cong sac nao. Nhan '+ Them Cong' de bo sung)", startX + 25, y + 5, 14, (Color){ 148, 163, 184, 255 });
            y += 30;
        } else {
            for (auto& p : st.ports) {
                if (y > 720) break;
                DrawRectangle(startX + 20, y, 725, 32, WHITE);
                DrawRectangleLines(startX + 20, y, 725, 32, (Color){ 241, 245, 249, 255 });

                string typeStr = (p.type == AC_STANDARD) ? "AC Standard" : ((p.type == DC_FAST) ? "DC Fast" : "DC Supercharge");
                string statusStr = (p.status == AVAILABLE) ? "TRONG (Available)" : ((p.status == OCCUPIED) ? "DANG SAC" : "BAO TRI (Maintenance)");
                Color stColor = (p.status == AVAILABLE) ? (Color){ 22, 163, 74, 255 } : ((p.status == OCCUPIED) ? (Color){ 217, 119, 6, 255 } : (Color){ 220, 38, 38, 255 });

                DrawText(TextFormat("Cong %s: %s | %.0f kW | %.0f d/kWh", p.portId.c_str(), typeStr.c_str(), p.powerKW, p.pricePerKWh), startX + 35, y + 8, 14, (Color){ 30, 41, 59, 255 });
                DrawText(statusStr.c_str(), startX + 380, y + 8, 14, stColor);

                // Nút chuyển trạng thái Available <-> Maintenance
                if (p.status != OCCUPIED) {
                    const char* btnLabel = (p.status == AVAILABLE) ? "Dat Bao Tri" : "Mo Hoat Dong";
                    if (GuiButton((Rectangle){ (float)startX + 550, (float)y + 3, 100, 24 }, btnLabel)) {
                        PortStatus newSt = (p.status == AVAILABLE) ? MAINTENANCE : AVAILABLE;
                        sm.updatePortStatus(st.stationId, p.portId, newSt);
                        autoSave();
                    }
                    if (GuiButton((Rectangle){ (float)startX + 660, (float)y + 3, 70, 24 }, "Xoa")) {
                        sm.deletePort(st.stationId, p.portId);
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
// ADMIN: QUẢN LÝ KHÁCH HÀNG & TÀI KHOẢN
// =====================================================================
void AppUI::DrawAdminCustomers() {
    DrawAdminSidebar();

    int startX = 245;
    DrawText("QUAN LY KHACH HANG & TAI KHOAN", startX, 25, 24, (Color){ 15, 23, 42, 255 });

    // Thanh tìm kiếm
    DrawText("Tim kiem (Ten / SDT / Bien so):", startX, 68, 14, DARKGRAY);
    if (GuiTextBox((Rectangle){ (float)startX + 225, 62, 280, 30 }, customerSearch, 128, activeCustomerSearch)) {
        activeCustomerSearch = !activeCustomerSearch;
    }

    // Nút mở form Thêm Khách Hàng
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

        // Hàng 1: User & Pass
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

        // Hàng 2: SĐT, Biển số, Dòng xe
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

        // Hàng 3: Số dư khởi tạo & Nút
        DrawText("So du ban dau (VND):", startX + 15, 235, 14, DARKGRAY);
        if (GuiTextBox((Rectangle){ (float)startX + 165, 230, 140, 26 }, newCustBalance, 32, activeCustField == 8)) {
            activeCustField = (activeCustField == 8) ? 0 : 8;
        }

        if (GuiButton((Rectangle){ (float)startX + 480, 230, 140, 32 }, "Xac Nhan Tao")) {
            if (strlen(newCustUsername) == 0 || strlen(newCustPassword) == 0 || strlen(newCustName) == 0) {
                customerStatusMsg = "Vui long nhap it nhat Username, Password va Ho Ten!";
            } else if (um.usernameExists(newCustUsername)) {
                customerStatusMsg = "Ten dang nhap nay da ton tai!";
            } else {
                string nextId = um.generateNextCustomerId();
                float bat = (float)atof(newCustBattery);
                if (bat <= 0) bat = 75.0f;
                float bal = (float)atof(newCustBalance);
                if (bal < 0) bal = 0.0f;

                Customer c(nextId, newCustUsername, newCustPassword, newCustName,
                           newCustPhone, newCustPlate, newCustCarModel, bat, bal, false);
                um.addCustomer(c);
                autoSave();
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

    // Table Header
    DrawRectangle(startX, tableStartY, 745, 34, (Color){ 241, 245, 249, 255 });
    DrawText("Ma KH", startX + 10, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Ho Ten", startX + 70, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("SDT", startX + 210, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Bien So", startX + 310, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Dong Xe", startX + 410, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("So Du (VND)", startX + 490, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });
    DrawText("Thao Tac", startX + 635, tableStartY + 9, 14, (Color){ 71, 85, 105, 255 });

    string kw = toLower(customerSearch);
    int rowY = tableStartY + 42;
    for (auto& c : um.getCustomers()) {
        if (rowY > 730) break;
        // Filter by keyword
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

        // Nút Khóa / Mở Khóa
        const char* lockLabel = c.getIsLocked() ? "Mo Khoa" : "KHOA";
        if (GuiButton((Rectangle){ (float)startX + 635, (float)rowY - 3, 90, 24 }, lockLabel)) {
            um.toggleCustomerLock(c.getId());
            autoSave();
        }

        rowY += 32;
    }
}

// =====================================================================
// ADMIN: HÓA ĐƠN & PHIÊN SẠC
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

            DrawText(inv.invoiceId.c_str(), startX + 20, y, 14, (Color){ 15, 23, 42, 255 });
            DrawText(inv.sessionId.c_str(), startX + 180, y, 14, (Color){ 100, 116, 139, 255 });
            DrawText(inv.customerId.c_str(), startX + 330, y, 14, (Color){ 15, 23, 42, 255 });
            DrawText(formatMoney(inv.totalAmount).c_str(), startX + 470, y, 14, (Color){ 22, 163, 74, 255 });
            DrawText(formatTime(inv.date).c_str(), startX + 610, y, 14, (Color){ 100, 116, 139, 255 });

            y += 30;
        }
    }
}

// =====================================================================
// GIAO DIỆN KHÁCH HÀNG (CUSTOMER MENU)
// =====================================================================
void AppUI::DrawCustomerMenu() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    // Header hiển thị: ID, Tên, Số dư ở góc trên cùng
    DrawRectangle(0, 0, GetScreenWidth(), 70, (Color){ 30, 58, 138, 255 });
    DrawText("⚡ EVC CHARGING", 25, 22, 24, (Color){ 56, 189, 248, 255 });

    // Hiển thị thông tin người dùng góc trên
    DrawText(TextFormat("👤 %s  |  Ma KH: %s  |  Xe: %s (%s)", c->getName().c_str(), c->getId().c_str(), c->getCarModel().c_str(), c->getLicensePlate().c_str()), 220, 26, 15, WHITE);

    // Số dư tài khoản nổi bật
    DrawRectangleRounded((Rectangle){ 700, 15, 210, 40 }, 0.2f, 4, (Color){ 22, 101, 52, 255 });
    DrawText(TextFormat("So du: %s", formatMoney(c->getBalance()).c_str()), 710, 27, 15, (Color){ 187, 247, 208, 255 });

    // Nút Đăng xuất
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

    // Tabs chuyển đổi màn hình con cho khách
    DrawRectangle(0, 70, GetScreenWidth(), 45, (Color){ 241, 245, 249, 255 });
    if (GuiButton((Rectangle){ 25, 76, 220, 33 }, customerTab == 0 ? "[ 🔌 TRA CUU & SAC DIEN ]" : "🔌 Tra Cuu & Sac Dien")) {
        customerTab = 0;
        customerErrorMsg = "";
    }
    if (GuiButton((Rectangle){ 260, 76, 190, 33 }, customerTab == 1 ? "[ 💳 NAP TIEN VAO VI ]" : "💳 Nap Tien Vao Vi")) {
        customerTab = 1;
        customerErrorMsg = "";
    }
    if (GuiButton((Rectangle){ 465, 76, 210, 33 }, customerTab == 2 ? "[ 📜 LICH SU HOA DON ]" : "📜 Lich Su Hoa Don")) {
        customerTab = 2;
        customerErrorMsg = "";
    }

    // Thông báo lỗi nếu có
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

            DrawText(TextFormat("📍 Tram: %s  -  Dia chi: %s", st.name.c_str(), st.address.c_str()), 40, y + 9, 16, (Color){ 15, 23, 42, 255 });
            y += 42;

            if (st.ports.empty()) {
                DrawText("  (Tram hien chua co cong sac nao)", 50, y + 5, 14, DARKGRAY);
                y += 28;
            } else {
                for (auto& p : st.ports) {
                    if (y > 720) break;
                    DrawRectangle(45, y, 940, 36, WHITE);
                    DrawRectangleLines(45, y, 940, 36, (Color){ 241, 245, 249, 255 });

                    string typeStr = (p.type == AC_STANDARD) ? "AC Standard" : ((p.type == DC_FAST) ? "DC Fast" : "DC Supercharge");
                    string stStr = (p.status == AVAILABLE) ? "TRONG (Available)" : ((p.status == OCCUPIED) ? "DANG SAC" : "BAO TRI (Maintenance)");
                    Color stColor = (p.status == AVAILABLE) ? (Color){ 22, 163, 74, 255 } : ((p.status == OCCUPIED) ? (Color){ 217, 119, 6, 255 } : (Color){ 220, 38, 38, 255 });

                    DrawText(TextFormat("Trụ %s: %s (%.0f kW)  -  Don gia: %.0f d/kWh", p.portId.c_str(), typeStr.c_str(), p.powerKW, p.pricePerKWh), 60, y + 10, 14, (Color){ 15, 23, 42, 255 });
                    DrawText(stStr.c_str(), 520, y + 10, 14, stColor);

                    // Nút Bắt Đầu Sạc (Chỉ hiện khi cổng Available)
                    if (p.status == AVAILABLE) {
                        if (GuiButton((Rectangle){ 800, (float)y + 4, 160, 28 }, "BAT DAU SAC")) {
                            // 1. Kiểm tra số dư tài khoản
                            if (c->getBalance() < 20000.0f) {
                                customerErrorMsg = "So du tai khoan khong du (Toi thieu 20.000 VND de bat dau sac)! Vui long qua tab Nap Tien.";
                            } else {
                                // 2. Khởi tạo phiên sạc
                                if (sem.startSession(c->getId(), st.stationId, p.portId, sm)) {
                                    customerErrorMsg = "";
                                    autoSave();
                                    currentState = STATE_CUSTOMER_CHARGING;
                                    return;
                                }
                            }
                        }
                    } else if (p.status == OCCUPIED) {
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
    // TAB 1: NẠP TIỀN VÀO VÍ
    // -------------------------------------------------------------
    else if (customerTab == 1) {
        DrawRectangle(25, 140, 974, 550, WHITE);
        DrawRectangleLines(25, 140, 974, 550, (Color){ 203, 213, 225, 255 });

        DrawText("NAP TIEN VAO TAI KHOAN TRU SAC", 320, 170, 22, (Color){ 30, 41, 59, 255 });

        DrawText("So du hien tai cua ban:", 330, 220, 16, DARKGRAY);
        DrawText(formatMoney(c->getBalance()).c_str(), 520, 215, 24, (Color){ 22, 163, 74, 255 });

        DrawText("Chon nhanh menh gia nap:", 330, 275, 16, (Color){ 71, 85, 105, 255 });
        if (GuiButton((Rectangle){ 330, 310, 150, 45 }, "+ 50.000 VND")) {
            c->addBalance(50000.0f);
            autoSave();
            topupStatusMsg = "Nap thanh cong 50,000 VND!";
        }
        if (GuiButton((Rectangle){ 500, 310, 150, 45 }, "+ 100.000 VND")) {
            c->addBalance(100000.0f);
            autoSave();
            topupStatusMsg = "Nap thanh cong 100,000 VND!";
        }
        if (GuiButton((Rectangle){ 330, 370, 150, 45 }, "+ 200.000 VND")) {
            c->addBalance(200000.0f);
            autoSave();
            topupStatusMsg = "Nap thanh cong 200,000 VND!";
        }
        if (GuiButton((Rectangle){ 500, 370, 150, 45 }, "+ 500.000 VND")) {
            c->addBalance(500000.0f);
            autoSave();
            topupStatusMsg = "Nap thanh cong 500,000 VND!";
        }

        // Nhập số tiền tùy ý
        DrawText("Hoac nhap so tien tuy y (VND):", 330, 445, 16, DARKGRAY);
        if (GuiTextBox((Rectangle){ 330, 475, 200, 38 }, topupAmountStr, 32, activeTopupAmount)) {
            activeTopupAmount = !activeTopupAmount;
        }

        if (GuiButton((Rectangle){ 545, 475, 130, 38 }, "Xac Nhan Nap")) {
            float val = (float)atof(topupAmountStr);
            if (val > 0) {
                c->addBalance(val);
                autoSave();
                topupStatusMsg = "Nap thanh cong " + formatMoney(val) + "!";
            } else {
                topupStatusMsg = "Vui long nhap so tien hop le!";
            }
        }

        if (!topupStatusMsg.empty()) {
            DrawText(topupStatusMsg.c_str(), 360, 540, 18, (topupStatusMsg.find("thanh cong") != string::npos) ? GREEN : RED);
        }
    }

    // -------------------------------------------------------------
    // TAB 2: LỊCH SỬ HÓA ĐƠN CỦA TÔI
    // -------------------------------------------------------------
    else if (customerTab == 2) {
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
            DrawText("Ban chua co hoa don sac nao.", 380, 330, 16, (Color){ 148, 163, 184, 255 });
        } else {
            int y = 245;
            for (int i = (int)myInvs.size() - 1; i >= 0; --i) {
                if (y > 650) break;
                auto& inv = myInvs[i];
                DrawText(inv.invoiceId.c_str(), 60, y, 14, (Color){ 15, 23, 42, 255 });
                DrawText(inv.sessionId.c_str(), 280, y, 14, (Color){ 100, 116, 139, 255 });
                DrawText(formatMoney(inv.totalAmount).c_str(), 520, y, 14, (Color){ 22, 163, 74, 255 });
                DrawText(formatTime(inv.date).c_str(), 760, y, 14, (Color){ 100, 116, 139, 255 });
                y += 32;
            }
        }
    }
}

// =====================================================================
// MÀN HÌNH ĐANG TRONG PHIÊN SẠC (LIVE CHARGING SCREEN)
// =====================================================================
void AppUI::DrawCustomerCharging() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    ChargingSession* ses = sem.getActiveSession(c->getId());
    if (!ses) {
        currentState = STATE_CUSTOMER_MENU;
        return;
    }

    ChargingStation* st = sm.getStationById(ses->stationId);
    string stName = st ? st->name : ses->stationId;
    string stAddr = st ? st->address : "";

    float pwr = 22.0f;
    float prc = 3500.0f;
    if (st) {
        for (auto& p : st->ports) {
            if (p.portId == ses->portId) {
                pwr = p.powerKW;
                prc = p.pricePerKWh;
                break;
            }
        }
    }

    long long durationSec = time(0) - ses->startTime;
    if (durationSec < 0) durationSec = 0;
    int mins = (int)(durationSec / 60);
    int secs = (int)(durationSec % 60);

    // Tính điện năng mô phỏng (1s thực = 1 phút sạc)
    float simulatedHours = (durationSec * 60.0f) / 3600.0f;
    float currentKWh = pwr * simulatedHours;
    if (currentKWh < 0.2f) currentKWh = 0.2f;
    float currentCost = currentKWh * prc;

    // Tính tiến độ Pin %
    float batCap = c->getBatteryCapacity();
    if (batCap <= 0) batCap = 80.0f;
    float addedPercent = (currentKWh / batCap) * 100.0f;
    float currentPercent = 30.0f + addedPercent;
    if (currentPercent > 100.0f) currentPercent = 100.0f;

    // Nền tối hiện đại cho phiên sạc
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 15, 23, 42, 255 });

    // Tiêu đề
    DrawText("⚡ XE CUA BAN DANG DUOC SAC DIEN", 300, 45, 26, (Color){ 56, 189, 248, 255 });
    DrawText(TextFormat("Tram: %s  |  Dia chi: %s  |  Cong sac: %s (%.0f kW)", stName.c_str(), stAddr.c_str(), ses->portId.c_str(), pwr), 240, 85, 15, (Color){ 203, 213, 225, 255 });

    // Khung đồng hồ hiển thị
    DrawRectangleRounded((Rectangle){ 180, 130, 664, 460 }, 0.05f, 4, (Color){ 30, 41, 59, 255 });

    // Thanh PIN trực quan
    DrawRectangleRounded((Rectangle){ 240, 165, 544, 40 }, 0.2f, 4, (Color){ 51, 65, 85, 255 });
    float progressWidth = 544.0f * (currentPercent / 100.0f);
    DrawRectangleRounded((Rectangle){ 240, 165, progressWidth, 40 }, 0.2f, 4, (Color){ 34, 197, 94, 255 });
    DrawText(TextFormat("Dung luong pin: %.1f %%", currentPercent), 430, 175, 18, WHITE);

    // Thời gian đã sạc
    DrawText("Thoi gian da sac:", 240, 240, 16, (Color){ 148, 163, 184, 255 });
    DrawText(TextFormat("%02d phut %02d giay", mins, secs), 240, 265, 28, WHITE);

    // Điện năng tiêu thụ
    DrawText("Dien nang da nạp:", 540, 240, 16, (Color){ 148, 163, 184, 255 });
    DrawText(TextFormat("%.2f kWh", currentKWh), 540, 265, 28, (Color){ 56, 189, 248, 255 });

    // Chi phí ước tính
    DrawText("Chi phi tam tinh:", 240, 335, 16, (Color){ 148, 163, 184, 255 });
    DrawText(formatMoney(currentCost).c_str(), 240, 360, 28, (Color){ 250, 204, 21, 255 });

    // Số dư khả dụng
    DrawText("So du vi hien tai:", 540, 335, 16, (Color){ 148, 163, 184, 255 });
    DrawText(formatMoney(c->getBalance()).c_str(), 540, 360, 28, (Color){ 74, 222, 128, 255 });

    // Cảnh báo nếu chi phí sắp vượt quá số dư
    if (currentCost > c->getBalance()) {
        DrawText("⚠️ Chi phi da vuot qua so du! Vui long dung sac de thanh toan.", 240, 420, 15, RED);
    } else {
        DrawText("Giu an toan - Khong ngat day sac khi chua bam dung sac tren man hinh.", 250, 420, 14, (Color){ 148, 163, 184, 255 });
    }

    // Nút DỪNG SẠC & THANH TOÁN
    if (GuiButton((Rectangle){ 330, 480, 364, 55 }, "🛑 DUNG SAC & THANH TOAN")) {
        // Lưu thông tin để hiện hóa đơn
        receiptStationName = stName;
        receiptPortId = ses->portId;
        receiptKWh = currentKWh;
        receiptCost = currentCost;

        if (sem.endSession(c->getId(), sm, um)) {
            receiptRemainingBal = c->getBalance();
            Invoice* lastInv = sem.getLastInvoice(c->getId());
            receiptInvoiceId = lastInv ? lastInv->invoiceId : "INV_SUCCESS";
            autoSave();
            currentState = STATE_CUSTOMER_RECEIPT;
        }
    }
}

// =====================================================================
// MÀN HÌNH BIÊN LAI / HÓA ĐƠN HOÀN TẤT (RECEIPT)
// =====================================================================
void AppUI::DrawCustomerReceipt() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 241, 245, 249, 255 });

    // Card hóa đơn
    DrawRectangleRounded((Rectangle){ 262, 100, 500, 530 }, 0.05f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ 262, 100, 500, 530 }, 0.05f, 4, (Color){ 203, 213, 225, 255 });

    // Header hóa đơn
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
