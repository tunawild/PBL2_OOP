#pragma once
#include "Models.h"
#include <vector>
#include <string>

using namespace std;

// =====================================================================
// LOP QUAN LY NGUOI DUNG (USER MANAGER)
// =====================================================================

/**
 * @brief Lop UserManager quan ly toan bo danh sach nguoi dung gom Admin va Customer.
 * Xu ly logic dang nhap, dang xuat, tim kiem, them moi khach hang va nap tien.
 */
class UserManager {
private:
    vector<Admin> admins;           // Danh sach quan tri vien
    vector<Customer> customers;     // Danh sach khach hang
    User* loggedInUser;             // Con tro toi nguoi dung hien dang dang nhap

public:
    /**
     * @brief Constructor khoi tao UserManager
     */
    UserManager();

    /**
     * @brief Doc du lieu tai khoan Admin va Customer tu cac file CSV.
     * Chi doc tu file, khong khoi tao du lieu mau neu file trong.
     */
    void loadData();

    /**
     * @brief Ghi toan bo du lieu Admin va Customer ra cac file CSV.
     */
    void saveData();

    /**
     * @brief Xu ly dang nhap vao he thong
     * @param username Ten dang nhap
     * @param password Mat khau
     * @return true neu dang nhap thanh cong, false neu sai thong tin
     */
    bool login(const string& username, const string& password);

    /**
     * @brief Dang xuat khoi he thong, dat loggedInUser ve nullptr
     */
    void logout();

    /**
     * @brief Lay con tro toi nguoi dung dang dang nhap
     * @return Con tro User* hoac nullptr neu chua dang nhap
     */
    User* getLoggedInUser();

    /**
     * @brief Lay danh sach tham chieu den toan bo khach hang
     */
    vector<Customer>& getCustomers();

    /**
     * @brief Tim kiem khach hang theo ma dinh danh (ID)
     * @param id Ma khach hang (vi du: C01)
     * @return Con tro Customer* neu tim thay, nguoc lai nullptr
     */
    Customer* getCustomerById(const string& id);

    /**
     * @brief Them mot khach hang moi vao he thong
     * @param c Doi tuong Customer can them
     */
    void addCustomer(const Customer& c);

    /**
     * @brief Kiem tra ten dang nhap da ton tai trong he thong hay chua
     * @param username Ten dang nhap can kiem tra
     * @return true neu da ton tai, false neu chua
     */
    bool usernameExists(const string& username);

    /**
     * @brief Tu dong sinh ma khach hang tiep theo (C01, C02, C03...)
     */
    string generateNextCustomerId();

    /**
     * @brief Admin nap tien vao tai khoan cho khach hang
     * @param customerId Ma khach hang can nap
     * @param amount So tien can nap (VND)
     * @return true neu nap thanh cong, false neu khong tim thay khach
     */
    bool topupCustomerBalance(const string& customerId, float amount);
};

// =====================================================================
// LOP QUAN LY TRAM SAC VA CONG SAC (STATION MANAGER)
// =====================================================================

/**
 * @brief Lop StationManager quan ly toan bo danh sach tram sac va cac cong sac.
 * Xu ly them/sua/xoa tram, them/xoa cong, cap nhat don gia, chuyen trang thai bao tri,
 * va tinh toan cac so lieu thong ke cho Admin Dashboard.
 */
class StationManager {
private:
    vector<ChargingStation> stations;   // Danh sach cac tram sac trong he thong

public:
    /**
     * @brief Constructor khoi tao StationManager
     */
    StationManager();

    /**
     * @brief Doc du lieu tram sac va cong sac tu stations.csv va ports.csv.
     * Chi doc tu file, khong khoi tao du lieu mau neu file trong.
     */
    void loadData();

    /**
     * @brief Ghi toan bo du lieu tram sac va cong sac ra stations.csv va ports.csv.
     */
    void saveData();

    /**
     * @brief Lay danh sach tham chieu den toan bo tram sac
     */
    vector<ChargingStation>& getStations();

    /**
     * @brief Tim kiem tram sac theo stationId
     * @param id Ma tram sac (vi du: ST01)
     * @return Con tro ChargingStation* neu tim thay, nguoc lai nullptr
     */
    ChargingStation* getStationById(const string& id);

    /**
     * @brief Them mot tram sac moi vao he thong
     * @param st Doi tuong ChargingStation can them
     */
    void addStation(const ChargingStation& st);

    /**
     * @brief Xoa mot tram sac khoi he thong
     * @note Dieu kien rang buoc: Chi xoa khi tram khong co cong nao dang sac (Occupied).
     * @param id Ma tram sac can xoa
     * @return true neu xoa thanh cong, false neu co cong dang sac hoac khong tim thay
     */
    bool deleteStation(const string& id);

    /**
     * @brief Cap nhat thong tin ten va dia chi cua tram sac
     */
    void updateStation(const string& id, const string& name, const string& address);

    /**
     * @brief Them mot cong sac moi vao mot tram cu the
     * @param stationId Ma tram nhan cong sac
     * @param port Doi tuong ChargingPort can them
     */
    void addPortToStation(const string& stationId, const ChargingPort& port);

    /**
     * @brief Xoa mot cong sac khoi mot tram sac
     * @note Khong cho phep xoa cong khi dang co xe sac.
     */
    bool deletePort(const string& stationId, const string& portId);

    /**
     * @brief Chuyen doi trang thai cong sac thu cong (Available <-> Maintenance)
     */
    void updatePortStatus(const string& stationId, const string& portId, PortStatus newStatus);

    /**
     * @brief Cap nhat don gia dien (VND/kWh) cho mot cong sac
     */
    void updatePortPrice(const string& stationId, const string& portId, float newPrice);

    /**
     * @brief Tu dong sinh ma tram sac tiep theo (ST01, ST02, ST03...)
     */
    string generateNextStationId();

    /**
     * @brief Tu dong sinh ma cong sac tiep theo cho mot tram (P01, P02...)
     */
    string generateNextPortId(const string& stationId);

    // --- Cac ham thong ke cho Admin Dashboard ---
    /**
     * @brief Lay tong so tram sac hien co
     */
    int getTotalStations() const;

    /**
     * @brief Lay tong so cong sac tren toan he thong
     */
    int getTotalPorts() const;

    /**
     * @brief Dem so luong cong sac theo trang thai (Available / Occupied / Maintenance)
     */
    int getPortsByStatus(PortStatus status) const;
};

// =====================================================================
// LOP QUAN LY PHIEN SAC VA HOA DON (SESSION MANAGER)
// =====================================================================

/**
 * @brief Lop SessionManager quan ly toan bo lich su va cac phien sac truc tiep (Live Sessions),
 * xu ly bat dau sac, dung sac, tinh toan chi phi, tao hoa don va thong ke doanh thu/dien nang.
 */
class SessionManager {
private:
    vector<ChargingSession> sessions;   // Danh sach toan bo phien sac
    vector<Invoice> invoices;           // Danh sach toan bo hoa don

public:
    /**
     * @brief Constructor khoi tao SessionManager
     */
    SessionManager();

    /**
     * @brief Doc du lieu phien sac va hoa don tu sessions.csv va invoices.csv.
     * Chi doc tu file, khong khoi tao du lieu mau neu file trong.
     */
    void loadData();

    /**
     * @brief Ghi toan bo du lieu phien sac va hoa don ra sessions.csv va invoices.csv.
     */
    void saveData();

    /**
     * @brief Lay danh sach tham chieu den toan bo phien sac
     */
    vector<ChargingSession>& getSessions();

    /**
     * @brief Lay danh sach tham chieu den toan bo hoa don
     */
    vector<Invoice>& getInvoices();

    /**
     * @brief Bat dau mot phien sac moi cho khach hang
     * @param customerId Ma khach hang bat dau sac
     * @param stationId Ma tram sac
     * @param portId Ma cong sac
     * @param sm Tham chieu StationManager de cap nhat trang thai cong sang OCCUPIED
     * @return true neu bat dau thanh cong, false neu cong khong kha dung
     */
    bool startSession(const string& customerId, const string& stationId,
                      const string& portId, StationManager& sm);

    /**
     * @brief Dung phien sac, tinh toan dien nang tieu thu, tru tien vi va sinh hoa don
     * @param customerId Ma khach hang dang sac
     * @param sm Tham chieu StationManager de tra cong sac ve AVAILABLE
     * @param um Tham chieu UserManager de tru tien so du vi khach hang
     * @return true neu ket thuc thanh cong
     */
    bool endSession(const string& customerId, StationManager& sm, UserManager& um);

    /**
     * @brief Lay phien sac hien dang hoat dong (isActive == true) cua mot khach hang
     */
    ChargingSession* getActiveSession(const string& customerId);

    /**
     * @brief Lay hoa don gan nhat cua mot khach hang
     */
    Invoice* getLastInvoice(const string& customerId);

    /**
     * @brief Lay toan bo danh sach hoa don cua mot khach hang cu the
     */
    vector<Invoice> getInvoicesByCustomer(const string& customerId);

    // --- Cac ham thong ke cho Admin Dashboard ---
    /**
     * @brief Tinh tong doanh thu toan he thong (VND)
     */
    float getTotalRevenue() const;

    /**
     * @brief Tinh tong dien nang da tieu thu toan he thong (kWh)
     */
    float getTotalEnergyConsumed() const;
};

// =====================================================================
// LOP DIEU PHOI DU LIEU TONG THE (DATA MANAGER)
// =====================================================================

/**
 * @brief Lop DataManager cung cap phuong thuc tinh dong bo load va save
 * cho tat ca cac Manager trong he thong.
 */
class DataManager {
public:
    /**
     * @brief Nạp toàn bộ dữ liệu từ các file CSV
     */
    static void loadAll(UserManager& um, StationManager& sm, SessionManager& sem);

    /**
     * @brief Lưu toàn bộ dữ liệu vào các file CSV
     */
    static void saveAll(UserManager& um, StationManager& sm, SessionManager& sem);
};
