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
#include <vector>

using namespace std;

// =====================================================================
// BẢNG MÃ UNICODE TIẾNG VIỆT ĐẦY ĐỦ CHO PHÔNG CHỮ
// =====================================================================

static std::vector<int> getVietnameseCodepoints() {
    std::vector<int> cp;
    for (int i = 32; i <= 126; i++) cp.push_back(i);        // Ký tự ASCII cơ bản (32 - 126)
    for (int i = 160; i <= 255; i++) cp.push_back(i);       // Latin-1 Supplement (À, Á, Â,...)
    for (int i = 0x0100; i <= 0x024F; i++) cp.push_back(i); // Latin Extended-A và Latin Extended-B (Toàn bộ Ơ, ơ, Ư, ư, Ă, ă, Đ, đ...)
    for (int i = 0x1EA0; i <= 0x1EF9; i++) cp.push_back(i); // Latin Extended Additional (Toàn bộ âm tiết tiếng Việt có dấu)
    // Các ký hiệu dấu câu và biểu tượng hỗ trợ
    cp.push_back(0x2010); // gạch nối ngắn
    cp.push_back(0x2013); // gạch nối ngắn –
    cp.push_back(0x2014); // gạch ngang dài —
    cp.push_back(0x2018); // mở nháy đơn ‘
    cp.push_back(0x2019); // đóng nháy đơn ’
    cp.push_back(0x201C); // mở nháy kép “
    cp.push_back(0x201D); // đóng nháy kép ”
    cp.push_back(0x2022); // dấu chấm tròn •
    cp.push_back(0x2026); // dấu ba chấm …
    cp.push_back(0x20AB); // ký hiệu đồng ₫
    cp.push_back(0x2212); // dấu trừ toán học
    return cp;
}

// =====================================================================
// CAC HAM HO TRO DINH DANG (FORMAT HELPERS)
// =====================================================================

/**
 * @brief Định dạng số tiền sang chuỗi tiền tệ Việt Nam Đồng (VNĐ) có dấu phẩy ngăn cách.
 * Ví dụ: 500000 -> "500,000 VNĐ"
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
    return res + " VNĐ";
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
    uiFont.texture.id = 0;
    loginBgTexture.id = 0;

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

AppUI::~AppUI() {
    if (uiFont.texture.id != 0) {
        UnloadFont(uiFont);
        uiFont.texture.id = 0;
    }
    if (loginBgTexture.id != 0) {
        UnloadTexture(loginBgTexture);
        loginBgTexture.id = 0;
    }
}

void AppUI::Init() {
    GuiLoadStyleDefault();

    // Lấy bảng mã ký tự tiếng Việt đầy đủ (bao gồm toàn bộ Latin-1, Latin-A, Latin-B Ơ ơ Ư ư, Latin Extended Additional)
    std::vector<int> vnCodepoints = getVietnameseCodepoints();

    // Nạp phông chữ hệ thống Segoe UI hiển thị tiếng Việt sắc nét, rõ ràng
    if (FileExists("C:\\Windows\\Fonts\\segoeui.ttf")) {
        uiFont = LoadFontEx("C:\\Windows\\Fonts\\segoeui.ttf", 36, vnCodepoints.data(), (int)vnCodepoints.size());
        if (uiFont.texture.id != 0) {
            GenTextureMipmaps(&uiFont.texture);
            SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
            GuiSetFont(uiFont);
        }
    } else if (FileExists("C:\\Windows\\Fonts\\arial.ttf")) {
        uiFont = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 36, vnCodepoints.data(), (int)vnCodepoints.size());
        if (uiFont.texture.id != 0) {
            GenTextureMipmaps(&uiFont.texture);
            SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
            GuiSetFont(uiFont);
        }
    }

    // Nạp ảnh nền tòa nhà Bách Khoa cho màn hình đăng nhập (hỗ trợ PNG và JPG)
    if (FileExists("assets/login_bg.png")) {
        loginBgTexture = LoadTexture("assets/login_bg.png");
        if (loginBgTexture.id != 0) {
            SetTextureFilter(loginBgTexture, TEXTURE_FILTER_BILINEAR);
        }
    } else if (FileExists("assets/login_bg.jpg")) {
        loginBgTexture = LoadTexture("assets/login_bg.jpg");
        if (loginBgTexture.id != 0) {
            SetTextureFilter(loginBgTexture, TEXTURE_FILTER_BILINEAR);
        }
    }

    GuiSetStyle(DEFAULT, TEXT_SIZE, 19);
}

// =====================================================================
// CAC HAM HO TRO VE CHU VOI PHONG CHU SEGOE UI CHUAN
// =====================================================================

void AppUI::DrawTextUI(const string& text, float posX, float posY, float fontSize, Color color) {
    if (uiFont.texture.id != 0) {
        float spacing = (fontSize <= 15.0f) ? 0.0f : 0.5f;
        DrawTextEx(uiFont, text.c_str(), (Vector2){ posX, posY }, fontSize, spacing, color);
    } else {
        DrawText(text.c_str(), (int)posX, (int)posY, (int)fontSize, color);
    }
}

void AppUI::DrawTextUICentered(const string& text, float posY, float fontSize, Color color) {
    float textW = 0.0f;
    float spacing = (fontSize <= 15.0f) ? 0.0f : 0.5f;
    if (uiFont.texture.id != 0) {
        Vector2 sz = MeasureTextEx(uiFont, text.c_str(), fontSize, spacing);
        textW = sz.x;
    } else {
        textW = (float)MeasureText(text.c_str(), (int)fontSize);
    }
    float posX = ((float)GetScreenWidth() - textW) / 2.0f;
    DrawTextUI(text, posX, posY, fontSize, color);
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

    // Modal nap tien cho khach
    if (showTopupModal) {
        DrawAdminTopupModal();
    }

    EndDrawing();
}

// =====================================================================
// 1. MAN HINH DANG NHAP (LOGIN SCREEN) - CAN GIUA TOAN MAN HINH CHUAN XAC
// =====================================================================

void AppUI::DrawLogin() {
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Điểm chia đôi màn hình: 50% bên trái (Ảnh tòa nhà) và 50% bên phải (Khung đăng nhập)
    float splitX = screenW * 0.50f;

    // Màu nền chủ đạo bên phải: Xanh biển sương nhạt (Soft Ocean Mist)
    Color rightBg = (Color){ 241, 246, 254, 255 };

    // =====================================================================
    // 1. BÊN TRÁI: ẢNH NỀN TÒA NHÀ & VÙNG CHUYỂN TIẾP (HORIZONTAL FADE)
    // =====================================================================
    // Vùng chuyển tiếp mượt mà: bắt đầu mờ từ splitX - 160px và tan biến hoàn toàn tại splitX + 130px
    float fadeW = 290.0f;
    float fadeStart = splitX - 160.0f;
    float fadeEnd = fadeStart + fadeW; // splitX + 130.0f

    if (loginBgTexture.id != 0) {
        float tw = (float)loginBgTexture.width;
        float th = (float)loginBgTexture.height;
        float scale = fmaxf(fadeEnd / tw, screenH / th);
        float sw = fadeEnd / scale;
        float sh = screenH / scale;
        float sx = (tw - sw) / 2.0f;
        float sy = (th - sh) / 2.0f;

        Rectangle sourceRec = { sx, sy, sw, sh };
        Rectangle destRec = { 0, 0, fadeEnd, screenH };
        DrawTexturePro(loginBgTexture, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // Lớp phủ gradient tối ở chân ảnh và góc trên để tôn chữ trắng và vàng
        DrawRectangleGradientV(0, (int)(screenH * 0.45f), (int)fadeEnd, (int)(screenH * 0.55f), (Color){ 0, 0, 0, 0 }, (Color){ 15, 30, 65, 245 });
        DrawRectangleGradientV(0, 0, (int)fadeEnd, 130, (Color){ 15, 30, 65, 180 }, (Color){ 0, 0, 0, 0 });
    } else {
        DrawRectangleGradientV(0, 0, (int)fadeEnd, (int)screenH, (Color){ 30, 64, 175, 255 }, (Color){ 15, 30, 65, 255 });
    }

    // HIỆU ỨNG CHUYỂN TIẾP MỀM MẠI (HORIZONTAL FADE TRANSITION)
    // Phủ dải gradient trong suốt từ fadeStart tới fadeEnd, làm mờ dần ảnh và tan biến tự nhiên vào nền rightBg
    DrawRectangleGradientH((int)fadeStart, 0, (int)fadeW, (int)screenH, (Color){ rightBg.r, rightBg.g, rightBg.b, 0 }, rightBg);
    // Lớp bổ sung thứ hai làm dịu đường cong alpha, xóa bỏ hoàn toàn bất kỳ cảm giác gờ cạnh
    DrawRectangleGradientH((int)(fadeStart + fadeW * 0.45f), 0, (int)(fadeW * 0.55f), (int)screenH, (Color){ rightBg.r, rightBg.g, rightBg.b, 0 }, (Color){ rightBg.r, rightBg.g, rightBg.b, 200 });

    // TOÀN BỘ NỬA BÊN PHẢI ĐƯỢC PHỦ NỀN ĐỒNG NHẤT KHÔNG CÓ BẤT KỲ ĐƯỜNG RÃNH HAY GẠCH NÀO
    DrawRectangle((int)fadeEnd - 1, 0, (int)(screenW - fadeEnd + 2), (int)screenH, rightBg);

    // Text thông tin trường & khoa ở góc trên bên trái
    DrawTextUI("TRƯỜNG ĐẠI HỌC BÁCH KHOA – ĐẠI HỌC ĐÀ NẴNG", 35, 30, 16, WHITE);
    DrawTextUI("KHOA CÔNG NGHỆ THÔNG TIN", 35, 56, 15, (Color){ 250, 204, 21, 255 }); // Màu Vàng Hoàng Gia

    // Text thông tin đồ án ở chân ảnh bên trái
    float bottomTextY = screenH - 135.0f;
    // Thẻ nhỏ màu Vàng kim
    DrawRectangleRounded((Rectangle){ 35, bottomTextY - 34, 250, 28 }, 0.3f, 4, (Color){ 234, 179, 8, 240 });
    DrawTextUI("ĐỒ ÁN CƠ SỞ LẬP TRÌNH (PBL2)", 48, bottomTextY - 28, 14, (Color){ 15, 23, 42, 255 });

    DrawTextUI("HỆ THỐNG QUẢN LÝ TRẠM SẠC XE ĐIỆN", 35, bottomTextY, 26, WHITE);
    DrawTextUI("Mô hình điều phối trạm sạc, cổng sạc và thanh toán thông minh", 35, bottomTextY + 38, 15, (Color){ 191, 219, 254, 255 });
    DrawTextUI("Ngôn ngữ lập trình C++ • Đồ họa Raylib • Kiến trúc Hướng đối tượng OOP", 35, bottomTextY + 66, 14, (Color){ 147, 197, 253, 255 });

    // =====================================================================
    // 2. BÊN PHẢI: KHUNG ĐĂNG NHẬP VỚI TÔNG XANH BIỂN & VÀNG KIM
    // =====================================================================
    float rightW = screenW - splitX;

    // Kích thước Card Đăng nhập
    float formW = 390.0f;
    float cardW = formW + 70.0f;
    float cardH = 460.0f;
    float cardX = splitX + (rightW - cardW) / 2.0f;
    float cardY = (screenH - cardH) / 2.0f;
    if (cardY < 30.0f) cardY = 30.0f;

    // Vẽ Card nền Trắng sang trọng với góc bo tròn
    DrawRectangleRounded((Rectangle){ cardX, cardY, cardW, cardH }, 0.04f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ cardX, cardY, cardW, cardH }, 0.04f, 4, (Color){ 219, 234, 254, 255 });

    // DẢI VIỀN VÀNG HOÀNG GIA (GOLD ACCENT RIBBON) Ở ĐỈNH CARD
    DrawRectangleRounded((Rectangle){ cardX + 25, cardY, cardW - 50, 5 }, 0.5f, 4, (Color){ 234, 179, 8, 255 });

    float formX = cardX + 35.0f;

    // Huy hiệu PBL2: Nền vàng nhạt, viền vàng kim, chữ vàng hổ phách
    DrawRectangleRounded((Rectangle){ formX, cardY + 30, 265, 30 }, 0.3f, 4, (Color){ 254, 243, 199, 255 });
    DrawRectangleRoundedLines((Rectangle){ formX, cardY + 30, 265, 30 }, 0.3f, 4, (Color){ 245, 158, 11, 255 });
    DrawTextUI("PBL2 • DỰ ÁN HỆ THỐNG TRẠM SẠC", formX + 14, cardY + 36, 14, (Color){ 180, 83, 9, 255 });

    // Tiêu đề chính màn hình Đăng nhập (Màu Xanh Biển Đậm)
    DrawTextUI("ĐĂNG NHẬP HỆ THỐNG", formX, cardY + 72, 28, (Color){ 30, 58, 138, 255 });
    DrawTextUI("Quản lý và vận hành trạm sạc xe điện thông minh", formX, cardY + 106, 14, (Color){ 100, 116, 139, 255 });

    // Tọa độ các thành phần nhập liệu
    Rectangle userBox = { formX, cardY + 164, formW, 44 };
    Rectangle passBox = { formX, cardY + 248, formW, 44 };
    Rectangle loginBtn = { formX, cardY + 316, formW, 48 };

    // Xử lý click chuột trực tiếp
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, userBox)) {
            loginUsernameActive = true;
            loginPasswordActive = false;
        } else if (CheckCollisionPointRec(mouse, passBox)) {
            loginPasswordActive = true;
            loginUsernameActive = false;
        } else if (!CheckCollisionPointRec(mouse, loginBtn)) {
            loginUsernameActive = false;
            loginPasswordActive = false;
        }
    }

    // Hỗ trợ phím TAB và ENTER
    if (IsKeyPressed(KEY_TAB)) {
        if (loginUsernameActive) {
            loginUsernameActive = false;
            loginPasswordActive = true;
        } else if (loginPasswordActive) {
            loginPasswordActive = false;
            loginUsernameActive = true;
        }
    }

    bool enterTriggeredLogin = false;
    if (IsKeyPressed(KEY_ENTER)) {
        if (loginUsernameActive) {
            loginUsernameActive = false;
            loginPasswordActive = true;
        } else if (loginPasswordActive) {
            enterTriggeredLogin = true;
        }
    }

    // Ô nhập Username
    DrawTextUI("Tên đăng nhập (Username):", formX, cardY + 140, 15, (Color){ 30, 41, 59, 255 });
    GuiTextBox(userBox, loginUsername, 64, loginUsernameActive);
    if (loginUsernameActive) {
        // Viền ánh Vàng Kim nổi bật khi đang nhập Username
        DrawRectangleRoundedLines((Rectangle){ userBox.x - 2, userBox.y - 2, userBox.width + 4, userBox.height + 4 }, 0.1f, 4, (Color){ 234, 179, 8, 255 });
    }

    // Ô nhập Password
    DrawTextUI("Mật khẩu (Password):", formX, cardY + 224, 15, (Color){ 30, 41, 59, 255 });
    GuiTextBox(passBox, loginPassword, 64, loginPasswordActive);
    if (loginPasswordActive) {
        // Viền ánh Vàng Kim nổi bật khi đang nhập Password
        DrawRectangleRoundedLines((Rectangle){ passBox.x - 2, passBox.y - 2, passBox.width + 4, passBox.height + 4 }, 0.1f, 4, (Color){ 234, 179, 8, 255 });
    }

    // Nút Bấm Đăng Nhập: Màu Xanh Biển Hoàng Gia & Viền Vàng Kim
    Vector2 mousePos = GetMousePosition();
    bool btnHover = CheckCollisionPointRec(mousePos, loginBtn);
    bool btnClick = btnHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Color btnBg = btnHover ? (Color){ 37, 99, 235, 255 } : (Color){ 30, 64, 175, 255 }; // Xanh biển
    Color btnBorder = btnHover ? (Color){ 250, 204, 21, 255 } : (Color){ 234, 179, 8, 255 }; // Vàng kim
    DrawRectangleRounded(loginBtn, 0.15f, 4, btnBg);
    DrawRectangleRoundedLines(loginBtn, 0.15f, 4, btnBorder);

    // Chữ ĐĂNG NHẬP căn giữa nút
    float btnTxtW = 120.0f;
    if (uiFont.texture.id != 0) {
        btnTxtW = MeasureTextEx(uiFont, "ĐĂNG NHẬP", 18, 1.0f).x;
    }
    DrawTextUI("ĐĂNG NHẬP", loginBtn.x + (loginBtn.width - btnTxtW) / 2.0f, loginBtn.y + 14, 18, (Color){ 254, 240, 138, 255 }); // Chữ Vàng sáng

    if (btnClick || enterTriggeredLogin) {
        loginUsernameActive = false;
        loginPasswordActive = false;
        if (um.login(loginUsername, loginPassword)) {
            loginError = false;
            loginErrorMsg = "";
            if (um.getLoggedInUser()->getRole() == ROLE_ADMIN) {
                currentState = STATE_ADMIN_DASHBOARD;
            } else {
                Customer* c = (Customer*)um.getLoggedInUser();
                if (c && sem.getActiveSession(c->getId()) != nullptr) {
                    currentState = STATE_CUSTOMER_CHARGING;
                } else {
                    currentState = STATE_CUSTOMER_MENU;
                }
            }
        } else {
            loginError = true;
            loginErrorMsg = "Sai tên đăng nhập hoặc mật khẩu!";
        }
    }

    // Thông báo lỗi nếu đăng nhập sai
    if (loginError) {
        float errW = 240.0f;
        if (uiFont.texture.id != 0) errW = MeasureTextEx(uiFont, loginErrorMsg.c_str(), 15, 1.0f).x;
        DrawTextUI(loginErrorMsg, formX + (formW - errW) / 2.0f, cardY + 382.0f, 15, (Color){ 220, 38, 38, 255 });
    }

    // Dòng giới thiệu nhỏ ở chân màn hình
    DrawTextUI("Dự án PBL2: Lập trình hướng đối tượng (OOP) • Khoa Công nghệ Thông tin", formX - 10, cardY + cardH + 18.0f, 13, (Color){ 100, 116, 139, 255 });
}

// =====================================================================
// THANH SIDEBAR DIEU HUONG CHO ADMIN
// =====================================================================

void AppUI::DrawAdminSidebar() {
    float sideW = 240.0f;
    float screenH = (float)GetScreenHeight();

    DrawRectangle(0, 0, (int)sideW, (int)screenH, (Color){ 30, 41, 59, 255 });
    
    // Logo Admin
    DrawRectangle(15, 25, (int)sideW - 30, 60, (Color){ 15, 23, 42, 255 });
    DrawTextUI("EVC ADMIN", 50, 40, 24, (Color){ 56, 189, 248, 255 });

    // Cac nut chuyen tab quan ly to ro, de bam
    if (GuiButton((Rectangle){ 15, 115, sideW - 30, 48 }, "Bảng điều khiển")) {
        currentState = STATE_ADMIN_DASHBOARD;
    }
    if (GuiButton((Rectangle){ 15, 175, sideW - 30, 48 }, "Trạm & Cổng sạc")) {
        currentState = STATE_ADMIN_STATIONS;
    }
    if (GuiButton((Rectangle){ 15, 235, sideW - 30, 48 }, "Khách hàng")) {
        currentState = STATE_ADMIN_CUSTOMERS;
    }
    if (GuiButton((Rectangle){ 15, 295, sideW - 30, 48 }, "Hóa đơn & Phiên sạc")) {
        currentState = STATE_ADMIN_INVOICES;
    }
    
    // Nut Dang Xuat o cuoi sidebar
    if (GuiButton((Rectangle){ 15, screenH - 70, sideW - 30, 48 }, "ĐĂNG XUẤT")) {
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
// 2. ADMIN DASHBOARD (THONG KE TONG QUAN TOAN MAN HINH)
// =====================================================================

void AppUI::DrawAdminDashboard() {
    DrawAdminSidebar();

    float startX = 265.0f;
    float contentW = (float)GetScreenWidth() - startX - 30.0f;

    DrawTextUI("BẢNG THỐNG KÊ TỔNG QUAN HỆ THỐNG", startX, 25, 28, (Color){ 15, 23, 42, 255 });

    int totalStations = sm.getTotalStations();
    int totalPorts = sm.getTotalPorts();
    int avail = sm.getPortsByStatus(AVAILABLE);
    int occ = sm.getPortsByStatus(OCCUPIED);
    int maint = sm.getPortsByStatus(MAINTENANCE);
    float revenue = sem.getTotalRevenue();
    float energy = sem.getTotalEnergyConsumed();

    // 4 The Thong ke trang thai cong sac tu dong co dan deu toan man hinh
    float cardSpacing = 20.0f;
    float cardW = (contentW - 3.0f * cardSpacing) / 4.0f;
    float cardH = 115.0f;
    float cardY = 75.0f;

    // The 1: Tong so tram
    DrawRectangleRounded((Rectangle){ startX, cardY, cardW, cardH }, 0.1f, 4, (Color){ 224, 242, 254, 255 });
    DrawTextUI("Tổng số trạm sạc", startX + 18, cardY + 16, 16, (Color){ 3, 105, 161, 255 });
    DrawTextUI(TextFormat("%d", totalStations), startX + 18, cardY + 45, 42, (Color){ 2, 132, 199, 255 });

    // The 2: Cong trong kha dung (Available)
    float c2X = startX + (cardW + cardSpacing);
    DrawRectangleRounded((Rectangle){ c2X, cardY, cardW, cardH }, 0.1f, 4, (Color){ 220, 252, 231, 255 });
    DrawTextUI("Cổng trống (Available)", c2X + 18, cardY + 16, 16, (Color){ 21, 128, 61, 255 });
    DrawTextUI(TextFormat("%d / %d", avail, totalPorts), c2X + 18, cardY + 45, 42, (Color){ 22, 163, 74, 255 });

    // The 3: Cong dang duoc sac (Occupied)
    float c3X = startX + 2.0f * (cardW + cardSpacing);
    DrawRectangleRounded((Rectangle){ c3X, cardY, cardW, cardH }, 0.1f, 4, (Color){ 254, 243, 199, 255 });
    DrawTextUI("Đang sạc (Occupied)", c3X + 18, cardY + 16, 16, (Color){ 180, 83, 9, 255 });
    DrawTextUI(TextFormat("%d", occ), c3X + 18, cardY + 45, 42, (Color){ 217, 119, 6, 255 });

    // The 4: Cong dang bao tri (Maintenance)
    float c4X = startX + 3.0f * (cardW + cardSpacing);
    DrawRectangleRounded((Rectangle){ c4X, cardY, cardW, cardH }, 0.1f, 4, (Color){ 254, 226, 226, 255 });
    DrawTextUI("Bảo trì (Maintenance)", c4X + 18, cardY + 16, 16, (Color){ 185, 28, 28, 255 });
    DrawTextUI(TextFormat("%d", maint), c4X + 18, cardY + 45, 42, (Color){ 220, 38, 38, 255 });

    // Khung Doanh thu & Dien nang tieu thu co dan theo 2 nua
    float boxW = (contentW - 20.0f) / 2.0f;
    float boxH = 95.0f;
    float boxY = cardY + cardH + 18.0f;

    DrawRectangleRounded((Rectangle){ startX, boxY, boxW, boxH }, 0.1f, 4, (Color){ 238, 242, 255, 255 });
    DrawTextUI("Tổng doanh thu hệ thống", startX + 22, boxY + 15, 17, (Color){ 67, 56, 202, 255 });
    DrawTextUI(formatMoney(revenue), startX + 22, boxY + 45, 28, (Color){ 79, 70, 229, 255 });

    DrawRectangleRounded((Rectangle){ startX + boxW + 20.0f, boxY, boxW, boxH }, 0.1f, 4, (Color){ 243, 232, 255, 255 });
    DrawTextUI("Tổng điện năng tiêu thụ", startX + boxW + 42, boxY + 15, 17, (Color){ 126, 34, 206, 255 });
    DrawTextUI(TextFormat("%.2f kWh", energy), startX + boxW + 42, boxY + 45, 28, (Color){ 147, 51, 234, 255 });

    // Bang danh sach cac phien sac truc tiep (Live Sessions)
    float tableY = boxY + boxH + 25.0f;
    float tableH = (float)GetScreenHeight() - tableY - 25.0f;

    DrawTextUI("CÁC PHIÊN SẠC ĐANG DIỄN RA (LIVE SESSIONS)", startX, tableY - 28.0f, 20, (Color){ 30, 41, 59, 255 });
    DrawRectangle((int)startX, (int)tableY, (int)contentW, (int)tableH, WHITE);
    DrawRectangleLines((int)startX, (int)tableY, (int)contentW, (int)tableH, (Color){ 203, 213, 225, 255 });

    DrawRectangle((int)startX, (int)tableY, (int)contentW, 40, (Color){ 241, 245, 249, 255 });
    DrawTextUI("Mã Phiên", startX + 20, tableY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Khách Hàng", startX + contentW * 0.22f, tableY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Trạm Sạc", startX + contentW * 0.44f, tableY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Cổng", startX + contentW * 0.68f, tableY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Thời Gian Bắt Đầu", startX + contentW * 0.80f, tableY + 10, 16, (Color){ 71, 85, 105, 255 });

    float py = tableY + 52.0f;
    int liveCount = 0;
    for (auto& s : sem.getSessions()) {
        if (s.getIsActive()) {
            liveCount++;
            Customer* c = um.getCustomerById(s.getCustomerId());
            string custName = c ? c->getName() : s.getCustomerId();
            ChargingStation* st = sm.getStationById(s.getStationId());
            string stName = st ? st->getName() : s.getStationId();

            DrawTextUI(s.getSessionId(), startX + 20, py, 16, (Color){ 15, 23, 42, 255 });
            DrawTextUI(custName, startX + contentW * 0.22f, py, 16, (Color){ 15, 23, 42, 255 });
            DrawTextUI(stName, startX + contentW * 0.44f, py, 16, (Color){ 15, 23, 42, 255 });
            DrawTextUI(s.getPortId(), startX + contentW * 0.68f, py, 16, (Color){ 217, 119, 6, 255 });
            DrawTextUI(formatTime(s.getStartTime()), startX + contentW * 0.80f, py, 16, (Color){ 100, 116, 139, 255 });

            py += 36.0f;
            if (py > tableY + tableH - 30.0f) break;
        }
    }
    if (liveCount == 0) {
        DrawTextUI("Hiện không có phiên sạc nào đang diễn ra trên hệ thống.", startX + contentW * 0.30f, tableY + tableH * 0.45f, 18, (Color){ 148, 163, 184, 255 });
    }
}

// =====================================================================
// 3. ADMIN: QUAN LY TRAM SAC & CONG SAC TOAN MAN HINH
// =====================================================================

void AppUI::DrawAdminStations() {
    DrawAdminSidebar();

    float startX = 265.0f;
    float contentW = (float)GetScreenWidth() - startX - 30.0f;

    DrawTextUI("QUẢN LÝ TRẠM SẠC & CỔNG SẠC", startX, 25, 28, (Color){ 15, 23, 42, 255 });

    // Nut Mo Form Them Tram Moi (Can phai)
    float addBtnW = 230.0f;
    if (GuiButton((Rectangle){ startX + contentW - addBtnW, 20, addBtnW, 40 }, "+ THÊM TRẠM SẠC MỚI")) {
        showAddStationForm = !showAddStationForm;
        showAddPortForm = false;
        stationStatusMsg = "";
    }

    if (!stationStatusMsg.empty()) {
        DrawTextUI(stationStatusMsg, startX, 62, 16, (stationStatusMsg.find("thành công") != string::npos) ? GREEN : RED);
    }

    // FORM THÊM TRẠM SẠC MỚI
    if (showAddStationForm) {
        float formY = 85.0f;
        float formH = 125.0f;
        DrawRectangleRounded((Rectangle){ startX, formY, contentW, formH }, 0.05f, 4, (Color){ 241, 245, 249, 255 });
        DrawRectangleRoundedLines((Rectangle){ startX, formY, contentW, formH }, 0.05f, 4, (Color){ 148, 163, 184, 255 });
        DrawTextUI("NHẬP THÔNG TIN TRẠM SẠC MỚI:", startX + 20, formY + 12, 17, (Color){ 30, 41, 59, 255 });

        Rectangle nameBox = { startX + 105, formY + 42, 260, 34 };
        Rectangle addrBox = { startX + 460, formY + 42, contentW - 480, 34 };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, nameBox)) {
                activeStationName = true;
                activeStationAddress = false;
            } else if (CheckCollisionPointRec(m, addrBox)) {
                activeStationAddress = true;
                activeStationName = false;
            }
        }

        DrawTextUI("Tên trạm:", startX + 20, formY + 48, 16, DARKGRAY);
        GuiTextBox(nameBox, newStationName, 128, activeStationName);

        DrawTextUI("Địa chỉ:", startX + 390, formY + 48, 16, DARKGRAY);
        GuiTextBox(addrBox, newStationAddress, 128, activeStationAddress);

        if (GuiButton((Rectangle){ startX + contentW - 270, formY + 82, 145, 34 }, "Xác Nhận Thêm")) {
            if (strlen(newStationName) == 0 || strlen(newStationAddress) == 0) {
                stationStatusMsg = "Vui lòng nhập đầy đủ tên và địa chỉ trạm!";
            } else {
                string newId = sm.generateNextStationId();
                ChargingStation st(newId, newStationName, newStationAddress);
                sm.addStation(st);
                autoSave();
                stationStatusMsg = "Thêm trạm sạc " + newId + " thành công!";
                memset(newStationName, 0, sizeof(newStationName));
                memset(newStationAddress, 0, sizeof(newStationAddress));
                showAddStationForm = false;
            }
        }
        if (GuiButton((Rectangle){ startX + contentW - 115, formY + 82, 95, 34 }, "Hủy")) {
            showAddStationForm = false;
        }
    }

    // FORM THÊM CỔNG SẠC MỚI VÀO TRẠM CỤ THỂ
    if (showAddPortForm) {
        float formY = 85.0f;
        float formH = 135.0f;
        DrawRectangleRounded((Rectangle){ startX, formY, contentW, formH }, 0.05f, 4, (Color){ 254, 243, 199, 255 });
        DrawRectangleRoundedLines((Rectangle){ startX, formY, contentW, formH }, 0.05f, 4, (Color){ 217, 119, 6, 255 });
        DrawTextUI(TextFormat("THÊM CỔNG SẠC CHO TRẠM: %s", targetStationIdForPort.c_str()), startX + 20, formY + 12, 17, (Color){ 180, 83, 9, 255 });

        DrawTextUI("Loại cổng:", startX + 20, formY + 48, 16, DARKGRAY);
        if (GuiButton((Rectangle){ startX + 110, formY + 44, 125, 32 }, selectedPortType == 0 ? "[AC Standard]" : "AC Standard")) selectedPortType = 0;
        if (GuiButton((Rectangle){ startX + 245, formY + 44, 110, 32 }, selectedPortType == 1 ? "[DC Fast]" : "DC Fast")) selectedPortType = 1;
        if (GuiButton((Rectangle){ startX + 365, formY + 44, 145, 32 }, selectedPortType == 2 ? "[Supercharge]" : "Supercharge")) selectedPortType = 2;

        Rectangle pwrBox = { startX + 660, formY + 44, 80, 32 };
        Rectangle prcBox = { startX + 840, formY + 44, 90, 32 };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, pwrBox)) {
                activePortPower = true;
                activePortPrice = false;
            } else if (CheckCollisionPointRec(m, prcBox)) {
                activePortPrice = true;
                activePortPower = false;
            }
        }

        DrawTextUI("Công suất (kW):", startX + 530, formY + 48, 16, DARKGRAY);
        GuiTextBox(pwrBox, newPortPower, 32, activePortPower);

        DrawTextUI("Giá/kWh:", startX + 760, formY + 48, 16, DARKGRAY);
        GuiTextBox(prcBox, newPortPrice, 32, activePortPrice);

        if (GuiButton((Rectangle){ startX + contentW - 270, formY + 90, 145, 34 }, "Xác Nhận Thêm")) {
            float pwr = (float)atof(newPortPower);
            float prc = (float)atof(newPortPrice);
            if (pwr <= 0) pwr = 22.0f;
            if (prc <= 0) prc = 3500.0f;

            string portId = sm.generateNextPortId(targetStationIdForPort);
            ChargingPort cp(portId, (PortType)selectedPortType, pwr, prc, AVAILABLE, "");
            sm.addPortToStation(targetStationIdForPort, cp);
            autoSave();
            stationStatusMsg = "Thêm cổng " + portId + " vào trạm " + targetStationIdForPort + " thành công!";
            showAddPortForm = false;
        }
        if (GuiButton((Rectangle){ startX + contentW - 115, formY + 90, 95, 34 }, "Hủy")) {
            showAddPortForm = false;
        }
    }

    // DANH SÁCH BẢNG CÁC TRẠM SẠC VÀ CÁC CỔNG
    float listStartY = (showAddStationForm || showAddPortForm) ? 235.0f : 90.0f;
    float y = listStartY;

    for (auto& st : sm.getStations()) {
        if (y > (float)GetScreenHeight() - 60.0f) break;
        // Hop tieu de tram sac
        DrawRectangleRounded((Rectangle){ startX, y, contentW, 44 }, 0.05f, 4, (Color){ 226, 232, 240, 255 });
        DrawRectangleRoundedLines((Rectangle){ startX, y, contentW, 44 }, 0.05f, 4, (Color){ 203, 213, 225, 255 });

        DrawTextUI(TextFormat("[%s] %s", st.getStationId().c_str(), st.getName().c_str()), startX + 18, y + 10, 18, (Color){ 15, 23, 42, 255 });
        DrawTextUI(TextFormat("Địa chỉ: %s", st.getAddress().c_str()), startX + 270, y + 12, 16, (Color){ 71, 85, 105, 255 });

        // Nut Them cong sac vao tram
        if (GuiButton((Rectangle){ startX + contentW - 240, y + 6, 125, 32 }, "+ Thêm Cổng")) {
            targetStationIdForPort = st.getStationId();
            showAddPortForm = true;
            showAddStationForm = false;
            stationStatusMsg = "";
        }

        // Nut Xoa tram
        if (GuiButton((Rectangle){ startX + contentW - 105, y + 6, 95, 32 }, "Xóa Trạm")) {
            if (sm.deleteStation(st.getStationId())) {
                autoSave();
                stationStatusMsg = "Xóa trạm " + st.getStationId() + " thành công!";
                break;
            } else {
                stationStatusMsg = "Không thể xóa trạm vì đang có cổng đang được sạc!";
            }
        }
        y += 50.0f;

        // Danh sach cac cong sac cua tram
        if (st.getPorts().empty()) {
            DrawTextUI("  (Trạm chưa có cổng sạc nào. Nhấn '+ Thêm Cổng' để bổ sung)", startX + 30, y + 5, 15, (Color){ 148, 163, 184, 255 });
            y += 35.0f;
        } else {
            for (auto& p : st.getPorts()) {
                if (y > (float)GetScreenHeight() - 50.0f) break;
                DrawRectangleRounded((Rectangle){ startX + 20, y, contentW - 20, 38 }, 0.05f, 4, WHITE);
                DrawRectangleRoundedLines((Rectangle){ startX + 20, y, contentW - 20, 38 }, 0.05f, 4, (Color){ 241, 245, 249, 255 });

                string typeStr = (p.getType() == AC_STANDARD) ? "AC Standard" : ((p.getType() == DC_FAST) ? "DC Fast" : "DC Supercharge");
                string statusStr = (p.getStatus() == AVAILABLE) ? "TRỐNG (Available)" : ((p.getStatus() == OCCUPIED) ? "ĐANG SẠC" : "BẢO TRÌ (Maintenance)");
                Color stColor = (p.getStatus() == AVAILABLE) ? (Color){ 22, 163, 74, 255 } : ((p.getStatus() == OCCUPIED) ? (Color){ 217, 119, 6, 255 } : (Color){ 220, 38, 38, 255 });

                DrawTextUI(TextFormat("Cổng %s: %s | %.0f kW | %.0f đ/kWh", p.getPortId().c_str(), typeStr.c_str(), p.getPowerKW(), p.getPricePerKWh()), startX + 40, y + 9, 16, (Color){ 30, 41, 59, 255 });
                DrawTextUI(statusStr, startX + contentW * 0.52f, y + 9, 16, stColor);

                // Nut Chuyen doi trang thai Bao tri / Hoat dong
                if (p.getStatus() != OCCUPIED) {
                    const char* btnLabel = (p.getStatus() == AVAILABLE) ? "Đặt Bảo Trì" : "Mở Hoạt Động";
                    if (GuiButton((Rectangle){ startX + contentW - 220, y + 4, 125, 30 }, btnLabel)) {
                        PortStatus newSt = (p.getStatus() == AVAILABLE) ? MAINTENANCE : AVAILABLE;
                        sm.updatePortStatus(st.getStationId(), p.getPortId(), newSt);
                        autoSave();
                    }
                    if (GuiButton((Rectangle){ startX + contentW - 85, y + 4, 75, 30 }, "Xóa")) {
                        sm.deletePort(st.getStationId(), p.getPortId());
                        autoSave();
                        break;
                    }
                } else {
                    DrawTextUI("[Đang phục vụ xe sạc]", startX + contentW - 200, y + 9, 15, (Color){ 217, 119, 6, 255 });
                }
                y += 44.0f;
            }
        }
        y += 10.0f;
    }
}

// =====================================================================
// 4. ADMIN: QUAN LY KHACH HANG & NAP TIEN TOAN MAN HINH
// =====================================================================

void AppUI::DrawAdminCustomers() {
    DrawAdminSidebar();

    float startX = 265.0f;
    float contentW = (float)GetScreenWidth() - startX - 30.0f;

    DrawTextUI("QUẢN LÝ KHÁCH HÀNG & TÀI KHOẢN", startX, 25, 28, (Color){ 15, 23, 42, 255 });

    // Thanh Tim kiem
    Rectangle searchBox = { startX + 270, 65, 320, 36 };
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        activeCustomerSearch = CheckCollisionPointRec(m, searchBox);
    }

    DrawTextUI("Tìm kiếm (Tên / SĐT / Biển số):", startX, 72, 16, DARKGRAY);
    GuiTextBox(searchBox, customerSearch, 128, activeCustomerSearch);

    // Nut Mo Form Them Khach Hang Moi (Can phai)
    float addCustBtnW = 245.0f;
    if (GuiButton((Rectangle){ startX + contentW - addCustBtnW, 63, addCustBtnW, 38 }, "+ THÊM TÀI KHOẢN KHÁCH")) {
        showAddCustomerForm = !showAddCustomerForm;
        customerStatusMsg = "";
    }

    if (!customerStatusMsg.empty()) {
        DrawTextUI(customerStatusMsg, startX, 108, 16, (customerStatusMsg.find("thành công") != string::npos) ? GREEN : RED);
    }

    // FORM THÊM TÀI KHOẢN KHÁCH HÀNG MỚI
    if (showAddCustomerForm) {
        float formY = 130.0f;
        float formH = 190.0f;
        DrawRectangleRounded((Rectangle){ startX, formY, contentW, formH }, 0.05f, 4, (Color){ 241, 245, 249, 255 });
        DrawRectangleRoundedLines((Rectangle){ startX, formY, contentW, formH }, 0.05f, 4, (Color){ 148, 163, 184, 255 });
        DrawTextUI("THÔNG TIN TÀI KHOẢN KHÁCH HÀNG MỚI:", startX + 20, formY + 12, 17, (Color){ 30, 41, 59, 255 });

        Rectangle bUser = { startX + 110, formY + 42, 140, 32 };
        Rectangle bPass = { startX + 355, formY + 42, 140, 32 };
        Rectangle bName = { startX + 585, formY + 42, contentW - 605, 32 };
        Rectangle bPhone = { startX + 110, formY + 89, 140, 32 };
        Rectangle bPlate = { startX + 355, formY + 89, 140, 32 };
        Rectangle bCar = { startX + 585, formY + 89, 130, 32 };
        Rectangle bBat = { startX + 810, formY + 89, 85, 32 };
        Rectangle bBal = { startX + 185, formY + 136, 180, 32 };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, bUser)) activeCustField = 1;
            else if (CheckCollisionPointRec(m, bPass)) activeCustField = 2;
            else if (CheckCollisionPointRec(m, bName)) activeCustField = 3;
            else if (CheckCollisionPointRec(m, bPhone)) activeCustField = 4;
            else if (CheckCollisionPointRec(m, bPlate)) activeCustField = 5;
            else if (CheckCollisionPointRec(m, bCar)) activeCustField = 6;
            else if (CheckCollisionPointRec(m, bBat)) activeCustField = 7;
            else if (CheckCollisionPointRec(m, bBal)) activeCustField = 8;
        }

        // Hang 1: Username, Password, Ho Ten
        DrawTextUI("Tài khoản:", startX + 20, formY + 48, 15, DARKGRAY);
        GuiTextBox(bUser, newCustUsername, 64, activeCustField == 1);

        DrawTextUI("Mật khẩu:", startX + 270, formY + 48, 15, DARKGRAY);
        GuiTextBox(bPass, newCustPassword, 64, activeCustField == 2);

        DrawTextUI("Họ Tên:", startX + 515, formY + 48, 15, DARKGRAY);
        GuiTextBox(bName, newCustName, 64, activeCustField == 3);

        // Hang 2: SDT, Bien so, Dong xe, Dung luong pin
        DrawTextUI("SĐT:", startX + 20, formY + 95, 15, DARKGRAY);
        GuiTextBox(bPhone, newCustPhone, 32, activeCustField == 4);

        DrawTextUI("Biển số xe:", startX + 270, formY + 95, 15, DARKGRAY);
        GuiTextBox(bPlate, newCustPlate, 32, activeCustField == 5);

        DrawTextUI("Dòng xe:", startX + 515, formY + 95, 15, DARKGRAY);
        GuiTextBox(bCar, newCustCarModel, 32, activeCustField == 6);

        DrawTextUI("Pin kWh:", startX + 735, formY + 95, 15, DARKGRAY);
        GuiTextBox(bBat, newCustBattery, 32, activeCustField == 7);

        // Hang 3: So du vi khoi tao & Nut bam
        DrawTextUI("Số dư ban đầu (VNĐ):", startX + 20, formY + 142, 15, DARKGRAY);
        GuiTextBox(bBal, newCustBalance, 32, activeCustField == 8);

        if (GuiButton((Rectangle){ startX + contentW - 270, formY + 136, 150, 36 }, "Xác Nhận Tạo")) {
            if (strlen(newCustUsername) == 0 || strlen(newCustPassword) == 0 || strlen(newCustName) == 0) {
                customerStatusMsg = "Vui lòng nhập ít nhất Tên đăng nhập, Mật khẩu và Họ Tên!";
            } else if (um.usernameExists(newCustUsername)) {
                customerStatusMsg = "Tên đăng nhập này đã tồn tại trong hệ thống!";
            } else {
                string nextId = um.generateNextCustomerId();
                float bat = (float)atof(newCustBattery);
                if (bat <= 0) bat = 75.0f;
                float bal = (float)atof(newCustBalance);
                if (bal < 0) bal = 0.0f;

                Customer c(nextId, newCustUsername, newCustPassword, newCustName,
                           newCustPhone, newCustPlate, newCustCarModel, bat, bal);
                um.addCustomer(c);
                autoSave();
                customerStatusMsg = "Tạo tài khoản " + string(newCustUsername) + " (Mã: " + nextId + ") thành công!";
                showAddCustomerForm = false;
                memset(newCustUsername, 0, sizeof(newCustUsername));
                memset(newCustPassword, 0, sizeof(newCustPassword));
                memset(newCustName, 0, sizeof(newCustName));
            }
        }
        if (GuiButton((Rectangle){ startX + contentW - 110, formY + 136, 95, 36 }, "Hủy")) {
            showAddCustomerForm = false;
        }
    }

    // DANH SÁCH BẢNG KHÁCH HÀNG TOÀN MÀN HÌNH
    float tableStartY = showAddCustomerForm ? 335.0f : 120.0f;
    float tableH = (float)GetScreenHeight() - tableStartY - 30.0f;

    DrawRectangle((int)startX, (int)tableStartY, (int)contentW, (int)tableH, WHITE);
    DrawRectangleLines((int)startX, (int)tableStartY, (int)contentW, (int)tableH, (Color){ 203, 213, 225, 255 });

    // Header Bang
    DrawRectangle((int)startX, (int)tableStartY, (int)contentW, 40, (Color){ 241, 245, 249, 255 });
    DrawTextUI("Mã KH", startX + 15, tableStartY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Họ Tên", startX + contentW * 0.10f, tableStartY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("SĐT", startX + contentW * 0.32f, tableStartY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Biển Số", startX + contentW * 0.47f, tableStartY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Dòng Xe", startX + contentW * 0.62f, tableStartY + 10, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Số Dư Ví", startX + contentW * 0.74f, tableStartY + 10, 16, (Color){ 22, 163, 74, 255 });
    DrawTextUI("Thao Tác", startX + contentW - 120, tableStartY + 10, 16, (Color){ 71, 85, 105, 255 });

    string kw = toLower(customerSearch);
    float rowY = tableStartY + 50.0f;
    for (auto& c : um.getCustomers()) {
        if (rowY > tableStartY + tableH - 35.0f) break;
        // Bo loc tim kiem theo Tu khoa
        if (!kw.empty()) {
            if (toLower(c.getName()).find(kw) == string::npos &&
                toLower(c.getPhone()).find(kw) == string::npos &&
                toLower(c.getLicensePlate()).find(kw) == string::npos) {
                continue;
            }
        }

        DrawTextUI(c.getId(), startX + 15, rowY, 16, (Color){ 15, 23, 42, 255 });
        DrawTextUI(c.getName(), startX + contentW * 0.10f, rowY, 16, (Color){ 15, 23, 42, 255 });
        DrawTextUI(c.getPhone(), startX + contentW * 0.32f, rowY, 16, (Color){ 100, 116, 139, 255 });
        DrawTextUI(c.getLicensePlate(), startX + contentW * 0.47f, rowY, 16, (Color){ 15, 23, 42, 255 });
        DrawTextUI(c.getCarModel(), startX + contentW * 0.62f, rowY, 16, (Color){ 71, 85, 105, 255 });
        DrawTextUI(formatMoney(c.getBalance()), startX + contentW * 0.74f, rowY, 16, (Color){ 22, 163, 74, 255 });

        // Nut Nạp tiền cho khách (Da thay the nut Khoa tai khoan theo yeu cau)
        if (GuiButton((Rectangle){ startX + contentW - 125, rowY - 4, 105, 28 }, "+ Nạp Tiền")) {
            showTopupModal = true;
            topupTargetCustomerId = c.getId();
            adminTopupMsg = "";
            strcpy(adminTopupAmount, "100000");
        }

        rowY += 38.0f;
    }
}

// =====================================================================
// MODAL HOP THOAI NAP TIEN CHO KHACH HANG (CAN GIUA TOAN MAN HINH)
// =====================================================================

void AppUI::DrawAdminTopupModal() {
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Lop phu mo toi toan man hinh
    DrawRectangle(0, 0, (int)screenW, (int)screenH, (Color){ 0, 0, 0, 160 });

    Customer* c = um.getCustomerById(topupTargetCustomerId);
    if (!c) {
        showTopupModal = false;
        return;
    }

    // Khung modal trung tam
    float modalW = 540.0f;
    float modalH = 410.0f;
    float modalX = (screenW - modalW) / 2.0f;
    float modalY = (screenH - modalH) / 2.0f;

    DrawRectangleRounded((Rectangle){ modalX, modalY, modalW, modalH }, 0.05f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ modalX, modalY, modalW, modalH }, 0.05f, 4, (Color){ 203, 213, 225, 255 });

    // Tieu de hop thoai (Tông Xanh Biển và Viền Vàng Kim)
    DrawRectangleRounded((Rectangle){ modalX, modalY, modalW, 60 }, 0.05f, 4, (Color){ 30, 58, 138, 255 });
    DrawRectangleRounded((Rectangle){ modalX + 20, modalY, modalW - 40, 4 }, 0.5f, 4, (Color){ 234, 179, 8, 255 });
    
    float tW = 270.0f;
    if (uiFont.texture.id != 0) tW = MeasureTextEx(uiFont, "NẠP TIỀN CHO KHÁCH HÀNG", 22, 1.0f).x;
    DrawTextUI("NẠP TIỀN CHO KHÁCH HÀNG", modalX + (modalW - tW) / 2.0f, modalY + 18, 22, WHITE);

    // Thong tin khach hang
    DrawTextUI(TextFormat("Khách hàng: %s  (Mã: %s)", c->getName().c_str(), c->getId().c_str()), modalX + 35, modalY + 80, 17, (Color){ 15, 23, 42, 255 });
    DrawTextUI(TextFormat("Số dư hiện tại: %s", formatMoney(c->getBalance()).c_str()), modalX + 35, modalY + 115, 18, (Color){ 22, 163, 74, 255 });

    // Cac nut chon nhanh menh gia
    DrawTextUI("Chọn nhanh mệnh giá:", modalX + 35, modalY + 155, 15, DARKGRAY);
    if (GuiButton((Rectangle){ modalX + 35, modalY + 180, 105, 38 }, "+ 50,000 đ")) {
        um.topupCustomerBalance(c->getId(), 50000.0f);
        autoSave();
        adminTopupMsg = "Nạp thành công 50,000 VNĐ!";
    }
    if (GuiButton((Rectangle){ modalX + 150, modalY + 180, 105, 38 }, "+ 100,000 đ")) {
        um.topupCustomerBalance(c->getId(), 100000.0f);
        autoSave();
        adminTopupMsg = "Nạp thành công 100,000 VNĐ!";
    }
    if (GuiButton((Rectangle){ modalX + 265, modalY + 180, 105, 38 }, "+ 200,000 đ")) {
        um.topupCustomerBalance(c->getId(), 200000.0f);
        autoSave();
        adminTopupMsg = "Nạp thành công 200,000 VNĐ!";
    }
    if (GuiButton((Rectangle){ modalX + 380, modalY + 180, 115, 38 }, "+ 500,000 đ")) {
        um.topupCustomerBalance(c->getId(), 500000.0f);
        autoSave();
        adminTopupMsg = "Nạp thành công 500,000 VNĐ!";
    }

    // Nhap so tien tuy y
    Rectangle topupBox = { modalX + 35, modalY + 260, 280, 38 };
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        activeAdminTopup = CheckCollisionPointRec(m, topupBox);
    }

    DrawTextUI("Hoặc nhập số tiền tùy ý (VNĐ):", modalX + 35, modalY + 235, 15, DARKGRAY);
    GuiTextBox(topupBox, adminTopupAmount, 32, activeAdminTopup);

    if (GuiButton((Rectangle){ modalX + 330, modalY + 260, 165, 38 }, "Xác Nhận Nạp")) {
        float val = (float)atof(adminTopupAmount);
        if (val > 0) {
            um.topupCustomerBalance(c->getId(), val);
            autoSave();
            adminTopupMsg = "Nạp thành công " + formatMoney(val) + "!";
        } else {
            adminTopupMsg = "Vui lòng nhập số tiền hợp lệ!";
        }
    }

    if (!adminTopupMsg.empty()) {
        DrawTextUI(adminTopupMsg, modalX + 35, modalY + 315, 16, (adminTopupMsg.find("thành công") != string::npos) ? GREEN : RED);
    }

    // Nut Dong Modal
    if (GuiButton((Rectangle){ modalX + 180, modalY + 348, 180, 42 }, "Hoàn Tất / Đóng")) {
        showTopupModal = false;
        adminTopupMsg = "";
    }
}

// =====================================================================
// 5. ADMIN: LICH SU HOA DON TOAN HE THONG (TOAN MAN HINH)
// =====================================================================

void AppUI::DrawAdminInvoices() {
    DrawAdminSidebar();

    float startX = 265.0f;
    float contentW = (float)GetScreenWidth() - startX - 30.0f;
    float tableH = (float)GetScreenHeight() - 110.0f;

    DrawTextUI("LỊCH SỬ HÓA ĐƠN TOÀN HỆ THỐNG", startX, 25, 28, (Color){ 15, 23, 42, 255 });

    DrawRectangle((int)startX, 75, (int)contentW, (int)tableH, WHITE);
    DrawRectangleLines((int)startX, 75, (int)contentW, (int)tableH, (Color){ 203, 213, 225, 255 });

    DrawRectangle((int)startX, 75, (int)contentW, 40, (Color){ 241, 245, 249, 255 });
    DrawTextUI("Mã Hóa Đơn", startX + 20, 85, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Mã Phiên", startX + contentW * 0.22f, 85, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Mã Khách Hàng", startX + contentW * 0.44f, 85, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Tổng Tiền (VNĐ)", startX + contentW * 0.65f, 85, 16, (Color){ 71, 85, 105, 255 });
    DrawTextUI("Thời Gian Lập", startX + contentW * 0.82f, 85, 16, (Color){ 71, 85, 105, 255 });

    float y = 130.0f;
    auto& invs = sem.getInvoices();
    if (invs.empty()) {
        DrawTextUI("Chưa có hóa đơn nào được ghi nhận trong hệ thống.", startX + contentW * 0.30f, 75.0f + tableH * 0.45f, 18, (Color){ 148, 163, 184, 255 });
    } else {
        for (int i = (int)invs.size() - 1; i >= 0; --i) {
            auto& inv = invs[i];
            if (y > 75.0f + tableH - 30.0f) break;

            DrawTextUI(inv.getInvoiceId(), startX + 20, y, 16, (Color){ 15, 23, 42, 255 });
            DrawTextUI(inv.getSessionId(), startX + contentW * 0.22f, y, 16, (Color){ 100, 116, 139, 255 });
            DrawTextUI(inv.getCustomerId(), startX + contentW * 0.44f, y, 16, (Color){ 15, 23, 42, 255 });
            DrawTextUI(formatMoney(inv.getTotalAmount()), startX + contentW * 0.65f, y, 16, (Color){ 22, 163, 74, 255 });
            DrawTextUI(formatTime(inv.getDate()), startX + contentW * 0.82f, y, 16, (Color){ 100, 116, 139, 255 });

            y += 36.0f;
        }
    }
}

// =====================================================================
// 6. GIAO DIEN MENU KHACH HANG (CUSTOMER MENU) - PHONG TO TOAN MAN HINH
// =====================================================================

void AppUI::DrawCustomerMenu() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Header hien thi: ID, Ten, Xe, Bien so va So du o goc tren cung
    DrawRectangle(0, 0, (int)screenW, 80, (Color){ 30, 58, 138, 255 });
    // Dải viền Vàng Kim ở đáy header
    DrawRectangle(0, 77, (int)screenW, 3, (Color){ 234, 179, 8, 255 });

    DrawTextUI("EVC CHARGING", 30, 26, 26, (Color){ 56, 189, 248, 255 });

    DrawTextUI(TextFormat("%s  |  Mã: %s  |  Xe: %s (%s)", c->getName().c_str(), c->getId().c_str(), c->getCarModel().c_str(), c->getLicensePlate().c_str()), 240, 28, 17, WHITE);

    // O hien thi so du tai khoan noi bat ben phai
    float balBoxW = 250.0f;
    float balBoxX = screenW - balBoxW - 140.0f;
    DrawRectangleRounded((Rectangle){ balBoxX, 18, balBoxW, 44 }, 0.2f, 4, (Color){ 22, 101, 52, 255 });
    DrawTextUI(TextFormat("Số dư: %s", formatMoney(c->getBalance()).c_str()), balBoxX + 15, 28, 17, (Color){ 187, 247, 208, 255 });

    // Nut Dang Xuat o goc ngoai cung ben phai
    float logoutBtnW = 110.0f;
    float logoutBtnX = screenW - logoutBtnW - 20.0f;
    if (GuiButton((Rectangle){ logoutBtnX, 18, logoutBtnW, 44 }, "Đăng Xuất")) {
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
    DrawRectangle(0, 80, (int)screenW, 50, (Color){ 241, 245, 249, 255 });
    if (GuiButton((Rectangle){ 30, 86, 300, 38 }, customerTab == 0 ? "[ TRA CỨU & SẠC ĐIỆN ]" : "Tra Cứu & Sạc Điện")) {
        customerTab = 0;
        customerErrorMsg = "";
    }
    if (GuiButton((Rectangle){ 345, 86, 300, 38 }, customerTab == 1 ? "[ LỊCH SỬ HÓA ĐƠN ]" : "Lịch Sử Hóa Đơn")) {
        customerTab = 1;
        customerErrorMsg = "";
    }

    // Thong bao loi neu co
    if (!customerErrorMsg.empty()) {
        DrawRectangle(30, 138, (int)screenW - 60, 34, (Color){ 254, 226, 226, 255 });
        DrawTextUI(customerErrorMsg, 45, 145, 16, RED);
    }

    // -------------------------------------------------------------
    // TAB 0: TRA CỨU TRẠM SẠC & BẮT ĐẦU SẠC
    // -------------------------------------------------------------
    if (customerTab == 0) {
        float y = customerErrorMsg.empty() ? 150.0f : 185.0f;
        float fullBoxW = screenW - 60.0f;

        DrawTextUI("DANH SÁCH TRA CỨU TRẠM SẠC & CỔNG SẠC KHẢ DỤNG:", 30, y, 19, (Color){ 30, 41, 59, 255 });
        y += 35.0f;

        for (auto& st : sm.getStations()) {
            if (y > screenH - 60.0f) break;
            DrawRectangleRounded((Rectangle){ 30, y, fullBoxW, 42 }, 0.05f, 4, (Color){ 226, 232, 240, 255 });
            DrawRectangleRoundedLines((Rectangle){ 30, y, fullBoxW, 42 }, 0.05f, 4, (Color){ 203, 213, 225, 255 });

            DrawTextUI(TextFormat("Trạm: %s  –  Địa chỉ: %s", st.getName().c_str(), st.getAddress().c_str()), 45, y + 10, 17, (Color){ 15, 23, 42, 255 });
            y += 48.0f;

            if (st.getPorts().empty()) {
                DrawTextUI("  (Trạm hiện chưa có cổng sạc nào)", 55, y + 5, 15, DARKGRAY);
                y += 32.0f;
            } else {
                for (auto& p : st.getPorts()) {
                    if (y > screenH - 50.0f) break;
                    float portBoxW = fullBoxW - 30.0f;
                    DrawRectangleRounded((Rectangle){ 50, y, portBoxW, 42 }, 0.05f, 4, WHITE);
                    DrawRectangleRoundedLines((Rectangle){ 50, y, portBoxW, 42 }, 0.05f, 4, (Color){ 241, 245, 249, 255 });

                    string typeStr = (p.getType() == AC_STANDARD) ? "AC Standard" : ((p.getType() == DC_FAST) ? "DC Fast" : "DC Supercharge");
                    string stStr = (p.getStatus() == AVAILABLE) ? "TRỐNG (Khả dụng)" : ((p.getStatus() == OCCUPIED) ? "ĐANG SẠC" : "BẢO TRÌ (Maintenance)");
                    Color stColor = (p.getStatus() == AVAILABLE) ? (Color){ 22, 163, 74, 255 } : ((p.getStatus() == OCCUPIED) ? (Color){ 217, 119, 6, 255 } : (Color){ 220, 38, 38, 255 });

                    DrawTextUI(TextFormat("Trụ %s: %s (%.0f kW)  –  Đơn giá: %.0f đ/kWh", p.getPortId().c_str(), typeStr.c_str(), p.getPowerKW(), p.getPricePerKWh()), 65, y + 11, 16, (Color){ 15, 23, 42, 255 });
                    DrawTextUI(stStr, 50.0f + portBoxW * 0.55f, y + 11, 16, stColor);

                    // Nut Bat Dau Sac (Chi kha dung khi cong AVAILABLE)
                    if (p.getStatus() == AVAILABLE) {
                        float chargeBtnW = 180.0f;
                        if (GuiButton((Rectangle){ 50.0f + portBoxW - chargeBtnW - 15.0f, y + 5, chargeBtnW, 32 }, "BẮT ĐẦU SẠC")) {
                            if (c->getBalance() < 20000.0f) {
                                customerErrorMsg = "Số dư tài khoản không đủ (Tối thiểu 20.000 VNĐ để bắt đầu sạc)! Vui lòng liên hệ Quản trị viên để nạp tiền.";
                            } else {
                                if (sem.startSession(c->getId(), st.getStationId(), p.getPortId(), sm)) {
                                    customerErrorMsg = "";
                                    autoSave();
                                    currentState = STATE_CUSTOMER_CHARGING;
                                    return;
                                }
                            }
                        }
                    } else if (p.getStatus() == OCCUPIED) {
                        DrawTextUI("(Đang có xe sạc)", 50.0f + portBoxW - 190.0f, y + 11, 15, (Color){ 217, 119, 6, 255 });
                    } else {
                        DrawTextUI("(Tạm ngưng bảo trì)", 50.0f + portBoxW - 190.0f, y + 11, 15, (Color){ 220, 38, 38, 255 });
                    }

                    y += 48.0f;
                }
            }
            y += 12.0f;
        }
    }

    // -------------------------------------------------------------
    // TAB 1: LỊCH SỬ HÓA ĐƠN CỦA KHÁCH HÀNG
    // -------------------------------------------------------------
    else if (customerTab == 1) {
        float tableW = screenW - 60.0f;
        float tableH = screenH - 190.0f;

        DrawRectangle(30, 150, (int)tableW, (int)tableH, WHITE);
        DrawRectangleLines(30, 150, (int)tableW, (int)tableH, (Color){ 203, 213, 225, 255 });

        DrawTextUI("LỊCH SỬ CÁC HÓA ĐƠN SẠC CỦA BẠN", 50, 168, 22, (Color){ 30, 41, 59, 255 });

        DrawRectangle(45, 210, (int)tableW - 30, 40, (Color){ 241, 245, 249, 255 });
        DrawTextUI("Mã Hóa Đơn", 65, 220, 16, (Color){ 71, 85, 105, 255 });
        DrawTextUI("Mã Phiên Sạc", 45.0f + tableW * 0.28f, 220, 16, (Color){ 71, 85, 105, 255 });
        DrawTextUI("Số Tiền Thanh Toán", 45.0f + tableW * 0.55f, 220, 16, (Color){ 71, 85, 105, 255 });
        DrawTextUI("Thời Gian Lập", 45.0f + tableW * 0.78f, 220, 16, (Color){ 71, 85, 105, 255 });

        auto myInvs = sem.getInvoicesByCustomer(c->getId());
        if (myInvs.empty()) {
            DrawTextUI("Bạn chưa có hóa đơn sạc nào trong hệ thống.", tableW * 0.35f, 350, 18, (Color){ 148, 163, 184, 255 });
        } else {
            float y = 265.0f;
            for (int i = (int)myInvs.size() - 1; i >= 0; --i) {
                if (y > 150.0f + tableH - 35.0f) break;
                auto& inv = myInvs[i];
                DrawTextUI(inv.getInvoiceId(), 65, y, 16, (Color){ 15, 23, 42, 255 });
                DrawTextUI(inv.getSessionId(), 45.0f + tableW * 0.28f, y, 16, (Color){ 100, 116, 139, 255 });
                DrawTextUI(formatMoney(inv.getTotalAmount()), 45.0f + tableW * 0.55f, y, 16, (Color){ 22, 163, 74, 255 });
                DrawTextUI(formatTime(inv.getDate()), 45.0f + tableW * 0.78f, y, 16, (Color){ 100, 116, 139, 255 });
                y += 38.0f;
            }
        }
    }
}

// =====================================================================
// 7. MAN HINH DANG SAC TRUC TIEP (CAN GIUA TOAN MAN HINH)
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

    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    // Nen toi hien dai cho man hinh dang sac
    DrawRectangle(0, 0, (int)screenW, (int)screenH, (Color){ 15, 23, 42, 255 });

    // Tieu de
    DrawTextUICentered("XE CỦA BẠN ĐANG ĐƯỢC SẠC ĐIỆN", 45, 32, (Color){ 56, 189, 248, 255 });
    DrawTextUICentered(TextFormat("Trạm: %s  |  Địa chỉ: %s  |  Cổng sạc: %s (%.0f kW)", stName.c_str(), stAddr.c_str(), ses->getPortId().c_str(), pwr), 95, 17, (Color){ 203, 213, 225, 255 });

    // Khung dong ho hien thi thong so can giua
    float boxW = 800.0f;
    float boxH = 540.0f;
    float boxX = (screenW - boxW) / 2.0f;
    float boxY = (screenH - boxH) / 2.0f + 25.0f;

    DrawRectangleRounded((Rectangle){ boxX, boxY, boxW, boxH }, 0.05f, 4, (Color){ 30, 41, 59, 255 });

    // THANH TRẠNG THÁI ĐANG SẠC (Charging Status Animation)
    float barW = boxW - 120.0f;
    float barH = 52.0f;
    float barX = boxX + 60.0f;
    float barY = boxY + 40.0f;

    Rectangle barRect = { barX, barY, barW, barH };
    DrawRectangleRounded(barRect, 0.2f, 4, (Color){ 15, 23, 42, 255 });
    DrawRectangleRoundedLines(barRect, 0.2f, 4, (Color){ 56, 189, 248, 255 });

    // Hieu ung luong dien chay nhe nhang (pulsing wave)
    float waveOffset = fmodf((float)GetTime() * 140.0f, barW);
    DrawRectangleRounded((Rectangle){ barX + waveOffset * 0.5f, barY + 2.0f, 150.0f, barH - 4.0f }, 0.2f, 4, (Color){ 34, 197, 94, 120 });

    // Chu trang thai sac can giua trong thanh
    string statusText = "ĐANG SẠC ĐIỆN (CHARGING IN PROGRESS)...";
    float stTxtW = 340.0f;
    if (uiFont.texture.id != 0) stTxtW = MeasureTextEx(uiFont, statusText.c_str(), 19, 1.0f).x;
    DrawTextUI(statusText, barX + (barW - stTxtW) / 2.0f, barY + 16, 19, (Color){ 74, 222, 128, 255 });

    // Thong so 1: Thoi gian da sac
    float col1X = boxX + 80.0f;
    float col2X = boxX + boxW / 2.0f + 40.0f;

    DrawTextUI("Thời gian đã sạc:", col1X, boxY + 130, 17, (Color){ 148, 163, 184, 255 });
    DrawTextUI(TextFormat("%02d phút %02d giây", mins, secs), col1X, boxY + 160, 32, WHITE);

    // Thong so 2: Dien nang da nap (kWh)
    DrawTextUI("Điện năng đã nạp:", col2X, boxY + 130, 17, (Color){ 148, 163, 184, 255 });
    DrawTextUI(TextFormat("%.2f kWh", currentKWh), col2X, boxY + 160, 32, (Color){ 56, 189, 248, 255 });

    // Thong so 3: Chi phi tam tinh
    DrawTextUI("Chi phí tạm tính:", col1X, boxY + 240, 17, (Color){ 148, 163, 184, 255 });
    DrawTextUI(formatMoney(currentCost), col1X, boxY + 270, 32, (Color){ 250, 204, 21, 255 });

    // Thong so 4: So du vi hien tai
    DrawTextUI("Số dư ví hiện tại:", col2X, boxY + 240, 17, (Color){ 148, 163, 184, 255 });
    DrawTextUI(formatMoney(c->getBalance()), col2X, boxY + 270, 32, (Color){ 74, 222, 128, 255 });

    // Canh bao neu chi phi vuot qua so du
    if (currentCost > c->getBalance()) {
        DrawTextUI("Lưu ý: Chi phí đã vượt quá số dư! Vui lòng dừng sạc để thanh toán.", col1X, boxY + 345, 16, RED);
    } else {
        DrawTextUI("Giữ an toàn – Không ngắt dây sạc khi chưa bấm dừng sạc trên màn hình.", col1X, boxY + 345, 15, (Color){ 148, 163, 184, 255 });
    }

    // Nut Dung Sac & Thanh Toan can giua
    float stopBtnW = 440.0f;
    float stopBtnH = 60.0f;
    float stopBtnX = boxX + (boxW - stopBtnW) / 2.0f;
    if (GuiButton((Rectangle){ stopBtnX, boxY + 410, stopBtnW, stopBtnH }, "DỪNG SẠC & THANH TOÁN")) {
        receiptStationName = stName;
        receiptPortId = ses->getPortId();
        receiptKWh = currentKWh;
        receiptCost = currentCost;

        if (sem.endSession(c->getId(), sm, um)) {
            receiptRemainingBal = c->getBalance();
            Invoice* lastInv = sem.getLastInvoice(c->getId());
            receiptInvoiceId = lastInv ? lastInv->getInvoiceId() : "INV_SUCCESS";
            autoSave();
            currentState = STATE_CUSTOMER_RECEIPT;
        }
    }
}

// =====================================================================
// 8. MAN HINH BIEN LAI HOA DON (CAN GIUA TOAN MAN HINH)
// =====================================================================

void AppUI::DrawCustomerReceipt() {
    Customer* c = (Customer*)um.getLoggedInUser();
    if (!c) return;

    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    DrawRectangle(0, 0, (int)screenW, (int)screenH, (Color){ 241, 245, 249, 255 });

    // Card Hoa don thanh toan can giua
    float cardW = 560.0f;
    float cardH = 570.0f;
    float cardX = (screenW - cardW) / 2.0f;
    float cardY = (screenH - cardH) / 2.0f;

    DrawRectangleRounded((Rectangle){ cardX, cardY, cardW, cardH }, 0.05f, 4, WHITE);
    DrawRectangleRoundedLines((Rectangle){ cardX, cardY, cardW, cardH }, 0.05f, 4, (Color){ 203, 213, 225, 255 });

    // Header Hoa don (Xanh lá thành công kết hợp dải vàng)
    DrawRectangleRounded((Rectangle){ cardX, cardY, cardW, 90 }, 0.05f, 4, (Color){ 22, 101, 52, 255 });
    DrawRectangleRounded((Rectangle){ cardX + 20, cardY, cardW - 40, 4 }, 0.5f, 4, (Color){ 234, 179, 8, 255 });
    
    string successTxt = "THANH TOÁN THÀNH CÔNG!";
    float sTxtW = 260.0f;
    if (uiFont.texture.id != 0) sTxtW = MeasureTextEx(uiFont, successTxt.c_str(), 24, 1.0f).x;
    DrawTextUI(successTxt, cardX + (cardW - sTxtW) / 2.0f, cardY + 22, 24, WHITE);
    
    string subTxt = "Hóa Đơn Điện Tử – PBL2 EV Station";
    float subW = 240.0f;
    if (uiFont.texture.id != 0) subW = MeasureTextEx(uiFont, subTxt.c_str(), 16, 1.0f).x;
    DrawTextUI(subTxt, cardX + (cardW - subW) / 2.0f, cardY + 54, 16, (Color){ 187, 247, 208, 255 });

    float y = cardY + 120.0f;
    float textX = cardX + 40.0f;

    DrawTextUI(TextFormat("Mã hóa đơn: %s", receiptInvoiceId.c_str()), textX, y, 17, (Color){ 15, 23, 42, 255 }); y += 40.0f;
    DrawTextUI(TextFormat("Khách hàng: %s (%s)", c->getName().c_str(), c->getId().c_str()), textX, y, 17, (Color){ 15, 23, 42, 255 }); y += 40.0f;
    DrawTextUI(TextFormat("Biển số xe: %s  –  Dòng xe: %s", c->getLicensePlate().c_str(), c->getCarModel().c_str()), textX, y, 17, (Color){ 15, 23, 42, 255 }); y += 40.0f;
    DrawTextUI(TextFormat("Trạm sạc: %s  (Cổng %s)", receiptStationName.c_str(), receiptPortId.c_str()), textX, y, 17, (Color){ 15, 23, 42, 255 }); y += 40.0f;
    DrawTextUI(TextFormat("Điện năng nạp: %.2f kWh", receiptKWh), textX, y, 17, (Color){ 15, 23, 42, 255 }); y += 40.0f;

    DrawLine((int)textX, (int)y, (int)(cardX + cardW - 40.0f), (int)y, (Color){ 203, 213, 225, 255 }); y += 25.0f;

    DrawTextUI("TỔNG TIỀN ĐÃ TRỪ:", textX, y, 18, (Color){ 185, 28, 28, 255 });
    DrawTextUI(formatMoney(receiptCost), cardX + cardW - 220.0f, y, 20, (Color){ 185, 28, 28, 255 }); y += 42.0f;

    DrawTextUI("Số dư ví còn lại:", textX, y, 18, (Color){ 21, 128, 61, 255 });
    DrawTextUI(formatMoney(receiptRemainingBal), cardX + cardW - 220.0f, y, 20, (Color){ 21, 128, 61, 255 }); y += 55.0f;

    float backBtnW = 340.0f;
    float backBtnX = cardX + (cardW - backBtnW) / 2.0f;
    if (GuiButton((Rectangle){ backBtnX, y, backBtnW, 48 }, "QUAY LẠI MENU CHÍNH")) {
        currentState = STATE_CUSTOMER_MENU;
    }
}
