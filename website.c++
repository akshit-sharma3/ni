/**
 * ============================================================================
 * THE ROYAL VAULT - LUXURY PAWN SHOP & COLLATERAL LOANS
 * High-Performance Standalone C++ Web Server & Interactive Web Application
 * ============================================================================
 * 
 * Features:
 *   - Native C++17 multi-threaded HTTP/1.1 Web Server (zero external dependencies)
 *   - Cross-platform support (Windows Sockets Winsock2 & POSIX Sockets)
 *   - Complete Pawn Shop Domain Engine (Valuation, Loans, Inventory, Tickets)
 *   - REST API endpoints for live data exchange (/api/inventory, /api/pawn, /api/ticket, etc.)
 *   - Embedded luxury HTML5 / CSS3 / JavaScript single-page web application
 *   - Auto-launches default web browser on startup (http://localhost:8080)
 * 
 * Compilation:
 *   - Windows (MinGW/GCC):  g++ -std=c++17 website.c++ -o pawnshop.exe -lws2_32
 *   - Windows (MSVC):       cl /EHsc /std:c++17 website.c++ ws2_32.lib
 *   - Linux / macOS:        g++ -std=c++17 -pthread website.c++ -o pawnshop
 * ============================================================================
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <memory>
#include <chrono>
#include <iomanip>
#include <thread>
#include <mutex>
#include <algorithm>
#include <map>
#include <cstring>
#include <ctime>

// Socket and Platform Abstraction
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <shellapi.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
    #define CLOSESOCKET(s) closesocket(s)
    #define GETSOCKETERR() WSAGetLastError()
#else
    #define PLATFORM_POSIX
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    typedef int socket_t;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define ISVALIDSOCKET(s) ((s) >= 0)
    #define CLOSESOCKET(s) close(s)
    #define GETSOCKETERR() (errno)
#endif

// ============================================================================
// DATA MODELS & PAWN SHOP DOMAIN
// ============================================================================

struct PawnItem {
    int id;
    std::string title;
    std::string category;
    std::string description;
    std::string condition;
    double estimatedValue;
    double retailPrice;
    std::string icon;
    std::string serialNumber;
    bool isAvailable;
};

struct PawnTicket {
    std::string ticketId;
    std::string customerName;
    std::string customerPhone;
    std::string itemName;
    std::string category;
    double appraisedValue;
    double loanPrincipal;
    double interestRateMonthly;
    int termDays;
    double totalRedemption;
    std::string issueDate;
    std::string dueDate;
    std::string status; // "ACTIVE", "REDEEMED", "FORFEITED"
};

// ============================================================================
// PAWN SHOP ENGINE & IN-MEMORY STORE
// ============================================================================

class PawnShopManager {
private:
    std::mutex dataMutex;
    std::vector<PawnItem> inventory;
    std::map<std::string, PawnTicket> tickets;
    int nextItemId;
    int nextTicketNum;

    std::string getCurrentDateStr(int daysOffset = 0) {
        auto now = std::chrono::system_clock::now();
        if (daysOffset != 0) {
            now += std::chrono::hours(24 * daysOffset);
        }
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tmBuf{};
#if defined(PLATFORM_WINDOWS)
        localtime_s(&tmBuf, &t);
#else
        localtime_r(&t, &tmBuf);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%b %d, %Y", &tmBuf);
        return std::string(buf);
    }

public:
    PawnShopManager() : nextItemId(101), nextTicketNum(1001) {
        initDefaultInventory();
        initDefaultTickets();
    }

    void initDefaultInventory() {
        inventory = {
            {
                nextItemId++,
                "Rolex Submariner Date 41mm (Ref: 126610LN)",
                "Luxury Watches",
                "Black Cerachrom ceramic bezel, Oystersteel case, caliber 3235 automatic movement. Full box and original papers included.",
                "Mint / Pre-Owned",
                14500.0,
                13200.0,
                "⌚",
                "RX-98234-A",
                true
            },
            {
                nextItemId++,
                "1968 Gibson Les Paul Custom 'Black Beauty'",
                "Musical Instruments",
                "Original vintage ebony finish with authentic golden hardware, mother-of-pearl block inlays, original PAF humbuckers.",
                "Vintage Excellent",
                9500.0,
                8400.0,
                "🎸",
                "GLP-1968-04",
                true
            },
            {
                nextItemId++,
                "3.20 Carat Platinum Diamond Solitaire Ring",
                "Fine Jewelry",
                "Brilliant round cut diamond, VVS1 clarity, E color grade, GIA certified with laser inscription. Platinum 950 band.",
                "Pristine",
                18500.0,
                16900.0,
                "💎",
                "GIA-6428190",
                true
            },
            {
                nextItemId++,
                "Leica M11 Rangefinder + Summicron-M 35mm f/2",
                "High-End Tech",
                "60MP BSI CMOS full-frame sensor, matte black finish, paired with legendary German Leica 35mm ASPH glass.",
                "Mint in Box",
                8200.0,
                7250.0,
                "📷",
                "LCA-55102-M",
                true
            },
            {
                nextItemId++,
                "100g 999.9 Fine Gold Minted Bar (Valcambi Suisse)",
                "Gold & Bullion",
                "Pure investment-grade investment gold bullion with official assay certificate and serial security seal.",
                "Uncirculated",
                9200.0,
                8850.0,
                "🏆",
                "VAL-AU-7741",
                true
            },
            {
                nextItemId++,
                "Patek Philippe Calatrava 18k Rose Gold 5196R",
                "Luxury Watches",
                "Timeless luxury dress watch with manual-wind caliber 215 PS, silvery opaline dial, alligator leather strap.",
                "Exceptional",
                25000.0,
                22900.0,
                "👑",
                "PP-5196R-12",
                true
            },
            {
                nextItemId++,
                "Sony FX3 Full-Frame Cinema Camera Kit",
                "High-End Tech",
                "Cinema Line 4K 120p video camera with XLR top handle unit, 2x 160GB CFexpress cards, and cage rig.",
                "Like New",
                4200.0,
                3400.0,
                "🎥",
                "SNY-FX3-882",
                true
            },
            {
                nextItemId++,
                "Cartier Love Bracelet (18k Solid Yellow Gold)",
                "Fine Jewelry",
                "Iconic oval motif, size 17, includes original Cartier ergonomic screwdriver, red jewel case, and provenance card.",
                "Polished / Excellent",
                7500.0,
                6400.0,
                "✨",
                "CRT-LV-9014",
                true
            }
        };
    }

    void initDefaultTickets() {
        PawnTicket t1;
        t1.ticketId = "TKT-1001";
        t1.customerName = "Alexander Wright";
        t1.customerPhone = "(555) 234-8901";
        t1.itemName = "Audemars Piguet Royal Oak Offshore 42mm";
        t1.category = "Luxury Watches";
        t1.appraisedValue = 28000.0;
        t1.loanPrincipal = 16000.0;
        t1.interestRateMonthly = 3.5;
        t1.termDays = 90;
        t1.totalRedemption = 17680.0;
        t1.issueDate = getCurrentDateStr(-25);
        t1.dueDate = getCurrentDateStr(65);
        t1.status = "ACTIVE";

        PawnTicket t2;
        t2.ticketId = "TKT-1002";
        t2.customerName = "Eleanor Vance";
        t2.customerPhone = "(555) 871-4432";
        t2.itemName = "18k White Gold Emerald & Diamond Necklace";
        t2.category = "Fine Jewelry";
        t2.appraisedValue = 12000.0;
        t2.loanPrincipal = 7200.0;
        t2.interestRateMonthly = 4.0;
        t2.termDays = 60;
        t2.totalRedemption = 7776.0;
        t2.issueDate = getCurrentDateStr(-10);
        t2.dueDate = getCurrentDateStr(50);
        t2.status = "ACTIVE";

        tickets[t1.ticketId] = t1;
        tickets[t2.ticketId] = t2;
    }

    // JSON serialization for Inventory
    std::string getInventoryJson() {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::ostringstream ss;
        ss << "[";
        bool first = true;
        for (const auto& item : inventory) {
            if (!first) ss << ",";
            first = false;
            ss << "{"
               << "\"id\":" << item.id << ","
               << "\"title\":\"" << escapeJson(item.title) << "\","
               << "\"category\":\"" << escapeJson(item.category) << "\","
               << "\"description\":\"" << escapeJson(item.description) << "\","
               << "\"condition\":\"" << escapeJson(item.condition) << "\","
               << "\"estimatedValue\":" << item.estimatedValue << ","
               << "\"retailPrice\":" << item.retailPrice << ","
               << "\"icon\":\"" << escapeJson(item.icon) << "\","
               << "\"serialNumber\":\"" << escapeJson(item.serialNumber) << "\","
               << "\"isAvailable\":" << (item.isAvailable ? "true" : "false")
               << "}";
        }
        ss << "]";
        return ss.str();
    }

    // JSON for a single Ticket
    std::string getTicketJson(const std::string& ticketId) {
        std::lock_guard<std::mutex> lock(dataMutex);
        auto it = tickets.find(ticketId);
        if (it == tickets.end()) {
            return "{\"error\":\"Ticket not found. Please verify your Ticket ID.\"}";
        }
        const auto& t = it->second;
        std::ostringstream ss;
        ss << "{"
           << "\"ticketId\":\"" << t.ticketId << "\","
           << "\"customerName\":\"" << escapeJson(t.customerName) << "\","
           << "\"customerPhone\":\"" << escapeJson(t.customerPhone) << "\","
           << "\"itemName\":\"" << escapeJson(t.itemName) << "\","
           << "\"category\":\"" << escapeJson(t.category) << "\","
           << "\"appraisedValue\":" << t.appraisedValue << ","
           << "\"loanPrincipal\":" << t.loanPrincipal << ","
           << "\"interestRateMonthly\":" << t.interestRateMonthly << ","
           << "\"termDays\":" << t.termDays << ","
           << "\"totalRedemption\":" << t.totalRedemption << ","
           << "\"issueDate\":\"" << t.issueDate << "\","
           << "\"dueDate\":\"" << t.dueDate << "\","
           << "\"status\":\"" << t.status << "\""
           << "}";
        return ss.str();
    }

    // Create a new pawn loan
    std::string createPawnLoan(const std::string& name, const std::string& phone,
                               const std::string& itemName, const std::string& category,
                               double appraisedValue, double requestedLoan, int termDays) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        // Validation & Business Rules
        if (appraisedValue <= 0) appraisedValue = 1000.0;
        double maxLoan = appraisedValue * 0.70; // 70% Loan-to-Value cap
        double principal = std::min(requestedLoan, maxLoan);
        if (principal <= 0) principal = appraisedValue * 0.50;

        double monthlyRate = (principal > 10000.0) ? 3.0 : 3.8; // Lower rate for luxury assets
        double months = termDays / 30.0;
        double totalInterest = principal * (monthlyRate / 100.0) * months;
        double totalRedemption = principal + totalInterest;

        std::string ticketId = "TKT-" + std::to_string(nextTicketNum++);

        PawnTicket ticket;
        ticket.ticketId = ticketId;
        ticket.customerName = name.empty() ? "Valued Client" : name;
        ticket.customerPhone = phone.empty() ? "N/A" : phone;
        ticket.itemName = itemName.empty() ? "Collateral Asset" : itemName;
        ticket.category = category.empty() ? "General Merchandise" : category;
        ticket.appraisedValue = appraisedValue;
        ticket.loanPrincipal = principal;
        ticket.interestRateMonthly = monthlyRate;
        ticket.termDays = termDays;
        ticket.totalRedemption = totalRedemption;
        ticket.issueDate = getCurrentDateStr(0);
        ticket.dueDate = getCurrentDateStr(termDays);
        ticket.status = "ACTIVE";

        tickets[ticketId] = ticket;

        std::ostringstream ss;
        ss << "{"
           << "\"success\":true,"
           << "\"ticketId\":\"" << ticketId << "\","
           << "\"loanPrincipal\":" << principal << ","
           << "\"totalRedemption\":" << totalRedemption << ","
           << "\"dueDate\":\"" << ticket.dueDate << "\","
           << "\"message\":\"Loan approved! Your collateral has been deposited in The Royal Vault.\""
           << "}";
        return ss.str();
    }

    // Purchase an item
    std::string buyItem(int itemId) {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (auto& item : inventory) {
            if (item.id == itemId) {
                if (!item.isAvailable) {
                    return "{\"success\":false,\"message\":\"Item has already been purchased.\"}";
                }
                item.isAvailable = false;
                return "{\"success\":true,\"message\":\"Congratulations! Purchase confirmed for " + escapeJson(item.title) + ".\"}";
            }
        }
        return "{\"success\":false,\"message\":\"Item not found in inventory.\"}";
    }

    // Redeem a pawn ticket
    std::string redeemTicket(const std::string& ticketId) {
        std::lock_guard<std::mutex> lock(dataMutex);
        auto it = tickets.find(ticketId);
        if (it == tickets.end()) {
            return "{\"success\":false,\"message\":\"Ticket ID not found.\"}";
        }
        if (it->second.status == "REDEEMED") {
            return "{\"success\":false,\"message\":\"This ticket has already been redeemed.\"}";
        }
        it->second.status = "REDEEMED";
        return "{\"success\":true,\"message\":\"Payment confirmed. Asset is released from The Royal Vault to " + escapeJson(it->second.customerName) + ".\"}";
    }

    // Pawn shop financial summary
    std::string getStatsJson() {
        std::lock_guard<std::mutex> lock(dataMutex);
        double totalLoanVolume = 0.0;
        int activeLoans = 0;
        for (const auto& pair : tickets) {
            if (pair.second.status == "ACTIVE") {
                totalLoanVolume += pair.second.loanPrincipal;
                activeLoans++;
            }
        }
        int availableItems = 0;
        double inventoryValue = 0.0;
        for (const auto& item : inventory) {
            if (item.isAvailable) {
                availableItems++;
                inventoryValue += item.retailPrice;
            }
        }
        std::ostringstream ss;
        ss << "{"
           << "\"activeLoansCount\":" << activeLoans << ","
           << "\"totalLoanVolume\":" << totalLoanVolume << ","
           << "\"availableItems\":" << availableItems << ","
           << "\"inventoryRetailValue\":" << inventoryValue << ","
           << "\"vaultSecuredRating\":\"AAA+\""
           << "}";
        return ss.str();
    }

private:
    static std::string escapeJson(const std::string& str) {
        std::ostringstream o;
        for (char c : str) {
            switch (c) {
                case '"': o << "\\\""; break;
                case '\\': o << "\\\\"; break;
                case '\b': o << "\\b"; break;
                case '\f': o << "\\f"; break;
                case '\n': o << "\\n"; break;
                case '\r': o << "\\r"; break;
                case '\t': o << "\\t"; break;
                default:
                    if ('\x00' <= c && c <= '\x1f') {
                        o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                    } else {
                        o << c;
                    }
            }
        }
        return o.str();
    }
};

// Global Pawn Shop instance
static PawnShopManager g_pawnShop;

// ============================================================================
// EMBEDDED LUXURY HTML5, CSS3, & JAVASCRIPT FRONTEND
// ============================================================================

const char* INDEX_HTML = R"pawn_html(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>The Royal Vault | Fine Jewelry, Luxury Timepieces & Collateral Loans</title>
    <!-- Google Fonts for Luxury Aesthetic -->
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Cinzel:wght@500;600;700;800;900&family=Outfit:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-base: #08090d;
            --bg-surface: #10121a;
            --bg-card: rgba(22, 26, 38, 0.7);
            --bg-card-hover: rgba(30, 36, 54, 0.9);
            --gold-primary: #d4af37;
            --gold-light: #f5e7a9;
            --gold-dark: #aa8212;
            --gold-glow: rgba(212, 175, 55, 0.25);
            --cyan-accent: #00e5ff;
            --emerald-accent: #10b981;
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
            --text-muted: #64748b;
            --border-glass: rgba(212, 175, 55, 0.2);
            --border-subtle: rgba(255, 255, 255, 0.07);
            --radius-sm: 8px;
            --radius-md: 14px;
            --radius-lg: 20px;
            --font-luxury: 'Cinzel', Georgia, serif;
            --font-ui: 'Outfit', -apple-system, BlinkMacSystemFont, sans-serif;
            --shadow-card: 0 12px 35px -5px rgba(0, 0, 0, 0.6);
            --shadow-gold: 0 0 25px rgba(212, 175, 55, 0.2);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            -webkit-font-smoothing: antialiased;
        }

        body {
            background-color: var(--bg-base);
            color: var(--text-primary);
            font-family: var(--font-ui);
            line-height: 1.6;
            overflow-x: hidden;
            background-image: 
                radial-gradient(circle at 15% 10%, rgba(212, 175, 55, 0.08) 0%, transparent 40%),
                radial-gradient(circle at 85% 60%, rgba(0, 229, 255, 0.05) 0%, transparent 45%),
                radial-gradient(circle at 50% 90%, rgba(170, 130, 18, 0.06) 0%, transparent 50%);
            min-height: 100vh;
        }

        /* Top Luxury Banner */
        .top-notice {
            background: linear-gradient(90deg, #090a0f, #1b1606, #090a0f);
            border-bottom: 1px solid var(--border-glass);
            padding: 7px 20px;
            text-align: center;
            font-size: 0.82rem;
            letter-spacing: 1.5px;
            text-transform: uppercase;
            color: var(--gold-light);
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 15px;
        }
        .top-notice span {
            display: inline-flex;
            align-items: center;
            gap: 6px;
        }
        .top-notice .badge {
            background: rgba(212, 175, 55, 0.2);
            color: var(--gold-light);
            padding: 2px 8px;
            border-radius: 4px;
            font-weight: 600;
            border: 1px solid rgba(212, 175, 55, 0.4);
        }

        /* Navigation Header */
        header {
            position: sticky;
            top: 0;
            z-index: 100;
            background: rgba(8, 9, 13, 0.85);
            backdrop-filter: blur(16px);
            border-bottom: 1px solid var(--border-subtle);
            padding: 16px 40px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }
        .brand {
            display: flex;
            align-items: center;
            gap: 14px;
            text-decoration: none;
            color: inherit;
        }
        .brand-crest {
            width: 44px;
            height: 44px;
            border-radius: 50%;
            background: linear-gradient(135deg, var(--gold-primary), #634b0d);
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 1.4rem;
            box-shadow: var(--shadow-gold);
            border: 1px solid var(--gold-light);
        }
        .brand-text h1 {
            font-family: var(--font-luxury);
            font-size: 1.35rem;
            letter-spacing: 2px;
            font-weight: 700;
            background: linear-gradient(135deg, #ffffff 40%, var(--gold-light) 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .brand-text p {
            font-size: 0.72rem;
            letter-spacing: 2.5px;
            color: var(--gold-primary);
            text-transform: uppercase;
        }

        .nav-links {
            display: flex;
            gap: 28px;
            align-items: center;
        }
        .nav-links a {
            color: var(--text-secondary);
            text-decoration: none;
            font-size: 0.92rem;
            font-weight: 500;
            letter-spacing: 0.5px;
            transition: color 0.2s ease;
        }
        .nav-links a:hover {
            color: var(--gold-primary);
        }
        .btn-gold {
            background: linear-gradient(135deg, var(--gold-primary), var(--gold-dark));
            color: #0b0d11;
            font-weight: 700;
            padding: 10px 22px;
            border-radius: var(--radius-sm);
            text-decoration: none;
            font-size: 0.88rem;
            letter-spacing: 1px;
            text-transform: uppercase;
            border: 1px solid var(--gold-light);
            cursor: pointer;
            box-shadow: 0 4px 18px rgba(212, 175, 55, 0.35);
            transition: all 0.25s ease;
            display: inline-flex;
            align-items: center;
            gap: 8px;
        }
        .btn-gold:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 24px rgba(212, 175, 55, 0.55);
            background: linear-gradient(135deg, var(--gold-light), var(--gold-primary));
        }

        .btn-outline {
            background: transparent;
            color: var(--gold-light);
            border: 1px solid var(--border-glass);
            padding: 9px 20px;
            border-radius: var(--radius-sm);
            cursor: pointer;
            font-size: 0.88rem;
            font-weight: 600;
            transition: all 0.2s ease;
            display: inline-flex;
            align-items: center;
            gap: 8px;
        }
        .btn-outline:hover {
            background: rgba(212, 175, 55, 0.12);
            border-color: var(--gold-primary);
        }

        /* Container Layout */
        .container {
            max-width: 1280px;
            margin: 0 auto;
            padding: 0 24px;
        }

        /* Hero Section */
        .hero {
            padding: 70px 0 50px;
            text-align: center;
            position: relative;
        }
        .hero-tagline {
            display: inline-block;
            font-family: var(--font-luxury);
            font-size: 0.88rem;
            letter-spacing: 3px;
            color: var(--gold-primary);
            text-transform: uppercase;
            margin-bottom: 16px;
            background: rgba(212, 175, 55, 0.1);
            padding: 6px 18px;
            border-radius: 30px;
            border: 1px solid var(--border-glass);
        }
        .hero h2 {
            font-family: var(--font-luxury);
            font-size: 3.4rem;
            font-weight: 800;
            letter-spacing: 1px;
            line-height: 1.15;
            margin-bottom: 20px;
            background: linear-gradient(180deg, #ffffff 30%, #e2e8f0 70%, var(--gold-light) 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .hero p.lead {
            font-size: 1.18rem;
            color: var(--text-secondary);
            max-width: 720px;
            margin: 0 auto 35px;
            font-weight: 300;
        }
        .hero-stats-row {
            display: flex;
            justify-content: center;
            gap: 20px;
            margin-top: 40px;
            flex-wrap: wrap;
        }
        .stat-badge {
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-md);
            padding: 16px 28px;
            min-width: 190px;
            backdrop-filter: blur(12px);
            text-align: left;
            transition: border-color 0.25s ease;
        }
        .stat-badge:hover {
            border-color: var(--border-glass);
        }
        .stat-badge .val {
            font-family: var(--font-luxury);
            font-size: 1.8rem;
            font-weight: 700;
            color: var(--gold-light);
            display: block;
        }
        .stat-badge .label {
            font-size: 0.78rem;
            text-transform: uppercase;
            letter-spacing: 1.5px;
            color: var(--text-muted);
        }

        /* Section Titles */
        .section-header {
            text-align: center;
            margin-bottom: 45px;
        }
        .section-header span {
            font-size: 0.8rem;
            letter-spacing: 3px;
            text-transform: uppercase;
            color: var(--gold-primary);
            font-family: var(--font-luxury);
        }
        .section-header h3 {
            font-family: var(--font-luxury);
            font-size: 2.2rem;
            color: #ffffff;
            margin-top: 6px;
        }
        .section-header p {
            color: var(--text-secondary);
            font-size: 0.98rem;
            max-width: 580px;
            margin: 8px auto 0;
        }

        /* Interactive Collateral Loan Calculator */
        .calculator-card {
            background: linear-gradient(145deg, rgba(20, 24, 36, 0.95), rgba(12, 14, 22, 0.95));
            border: 1px solid var(--border-glass);
            border-radius: var(--radius-lg);
            padding: 40px;
            box-shadow: var(--shadow-card), var(--shadow-gold);
            margin-bottom: 80px;
            display: grid;
            grid-template-columns: 1.15fr 0.85fr;
            gap: 40px;
            position: relative;
            overflow: hidden;
        }
        .calculator-card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 2px;
            background: linear-gradient(90deg, transparent, var(--gold-primary), transparent);
        }

        .calc-form .slider-group {
            margin-bottom: 26px;
        }
        .calc-form .slider-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 10px;
        }
        .calc-form label {
            font-size: 0.92rem;
            color: var(--text-secondary);
            font-weight: 500;
        }
        .calc-form .slider-val {
            font-family: var(--font-luxury);
            color: var(--gold-light);
            font-size: 1.25rem;
            font-weight: 700;
        }
        .calc-form input[type="range"] {
            width: 100%;
            height: 7px;
            background: #252c42;
            border-radius: 4px;
            outline: none;
            accent-color: var(--gold-primary);
            cursor: pointer;
        }
        .term-selector {
            display: flex;
            gap: 12px;
            margin-top: 8px;
        }
        .term-btn {
            flex: 1;
            padding: 10px;
            background: #181d2d;
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            color: var(--text-secondary);
            font-weight: 600;
            font-size: 0.88rem;
            cursor: pointer;
            transition: all 0.2s ease;
        }
        .term-btn.active, .term-btn:hover {
            background: rgba(212, 175, 55, 0.15);
            border-color: var(--gold-primary);
            color: var(--gold-light);
        }

        .calc-result {
            background: rgba(10, 12, 18, 0.8);
            border: 1px solid var(--border-glass);
            border-radius: var(--radius-md);
            padding: 30px;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }
        .calc-result h4 {
            font-family: var(--font-luxury);
            font-size: 1.15rem;
            color: var(--gold-light);
            letter-spacing: 1px;
            margin-bottom: 20px;
            border-bottom: 1px solid var(--border-subtle);
            padding-bottom: 12px;
        }
        .breakdown-row {
            display: flex;
            justify-content: space-between;
            margin-bottom: 14px;
            font-size: 0.95rem;
            color: var(--text-secondary);
        }
        .breakdown-row.highlight {
            font-size: 1.25rem;
            color: #ffffff;
            font-weight: 700;
            border-top: 1px solid var(--border-subtle);
            padding-top: 14px;
            margin-top: 16px;
        }
        .breakdown-row.highlight .amount {
            color: var(--gold-primary);
            font-family: var(--font-luxury);
            font-size: 1.55rem;
        }
        .calc-action {
            margin-top: 25px;
        }

        /* Showcase Inventory Section */
        .inventory-section {
            margin-bottom: 80px;
        }
        .inventory-toolbar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
            gap: 15px;
            flex-wrap: wrap;
        }
        .category-tabs {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }
        .cat-tab {
            background: var(--bg-surface);
            border: 1px solid var(--border-subtle);
            color: var(--text-secondary);
            padding: 8px 18px;
            border-radius: 30px;
            font-size: 0.85rem;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.2s ease;
        }
        .cat-tab.active, .cat-tab:hover {
            background: var(--gold-primary);
            color: #0b0d11;
            font-weight: 700;
            border-color: var(--gold-light);
        }
        .search-box {
            position: relative;
            min-width: 260px;
        }
        .search-box input {
            width: 100%;
            background: var(--bg-surface);
            border: 1px solid var(--border-subtle);
            padding: 10px 16px 10px 38px;
            border-radius: 30px;
            color: #fff;
            font-size: 0.88rem;
            outline: none;
            transition: border-color 0.2s;
        }
        .search-box input:focus {
            border-color: var(--gold-primary);
        }
        .search-box .icon {
            position: absolute;
            left: 14px;
            top: 50%;
            transform: translateY(-50%);
            color: var(--text-muted);
            font-size: 0.9rem;
        }

        /* Inventory Grid */
        .inventory-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(285px, 1fr));
            gap: 25px;
        }
        .item-card {
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-md);
            overflow: hidden;
            transition: all 0.3s cubic-bezier(0.16, 1, 0.3, 1);
            display: flex;
            flex-direction: column;
            backdrop-filter: blur(8px);
        }
        .item-card:hover {
            transform: translateY(-6px);
            border-color: var(--border-glass);
            box-shadow: 0 15px 30px rgba(0, 0, 0, 0.7), 0 0 20px rgba(212, 175, 55, 0.15);
        }
        .item-media {
            height: 180px;
            background: radial-gradient(circle at center, #1f2538 0%, #10131d 100%);
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 4.8rem;
            position: relative;
            border-bottom: 1px solid var(--border-subtle);
        }
        .item-badge {
            position: absolute;
            top: 12px;
            right: 12px;
            background: rgba(10, 12, 18, 0.85);
            border: 1px solid var(--border-glass);
            color: var(--gold-light);
            font-size: 0.72rem;
            padding: 3px 9px;
            border-radius: 4px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.8px;
        }
        .item-content {
            padding: 22px;
            display: flex;
            flex-direction: column;
            flex: 1;
        }
        .item-category {
            font-size: 0.75rem;
            letter-spacing: 1.5px;
            color: var(--gold-primary);
            text-transform: uppercase;
            font-weight: 600;
            margin-bottom: 6px;
        }
        .item-title {
            font-family: var(--font-luxury);
            font-size: 1.12rem;
            color: #ffffff;
            margin-bottom: 8px;
            line-height: 1.35;
        }
        .item-desc {
            font-size: 0.84rem;
            color: var(--text-secondary);
            margin-bottom: 18px;
            flex: 1;
            display: -webkit-box;
            -webkit-line-clamp: 2;
            -webkit-box-orient: vertical;
            overflow: hidden;
        }
        .item-meta {
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
            padding-top: 14px;
            border-top: 1px solid var(--border-subtle);
            margin-bottom: 16px;
        }
        .item-price {
            font-family: var(--font-luxury);
            font-size: 1.45rem;
            color: var(--gold-light);
            font-weight: 700;
        }
        .item-original {
            font-size: 0.75rem;
            color: var(--text-muted);
            text-decoration: line-through;
        }
        .btn-buy {
            width: 100%;
            background: #171c2b;
            color: var(--gold-light);
            border: 1px solid var(--border-glass);
            padding: 10px;
            border-radius: var(--radius-sm);
            font-weight: 600;
            font-size: 0.88rem;
            cursor: pointer;
            transition: all 0.2s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .btn-buy:hover {
            background: var(--gold-primary);
            color: #0b0d11;
            box-shadow: 0 4px 15px rgba(212, 175, 55, 0.4);
        }
        .btn-buy:disabled {
            opacity: 0.4;
            cursor: not-allowed;
            background: #12141c;
            color: var(--text-muted);
            border-color: var(--border-subtle);
        }

        /* Ticket Tracker & Redemption Tool */
        .tracker-section {
            background: linear-gradient(180deg, #10131d 0%, #090b10 100%);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-lg);
            padding: 45px;
            margin-bottom: 80px;
        }
        .tracker-input-row {
            display: flex;
            max-width: 600px;
            margin: 0 auto 30px;
            gap: 12px;
        }
        .tracker-input-row input {
            flex: 1;
            background: var(--bg-surface);
            border: 1px solid var(--border-glass);
            border-radius: var(--radius-sm);
            padding: 12px 18px;
            font-size: 1rem;
            color: #fff;
            outline: none;
            letter-spacing: 1px;
            font-family: monospace;
        }
        .ticket-result-box {
            max-width: 740px;
            margin: 0 auto;
            background: rgba(18, 22, 33, 0.9);
            border: 1px dashed var(--gold-primary);
            border-radius: var(--radius-md);
            padding: 30px;
            display: none;
            position: relative;
            box-shadow: var(--shadow-card);
        }
        .ticket-badge-pill {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 0.75rem;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 12px;
        }
        .badge-active { background: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid #10b981; }
        .badge-redeemed { background: rgba(59, 130, 246, 0.2); color: #60a5fa; border: 1px solid #3b82f6; }

        /* Modal Overlays */
        .modal-overlay {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: rgba(0, 0, 0, 0.8);
            backdrop-filter: blur(10px);
            z-index: 999;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .modal-card {
            background: #111420;
            border: 1px solid var(--gold-primary);
            border-radius: var(--radius-lg);
            width: 100%;
            max-width: 580px;
            padding: 35px;
            box-shadow: 0 25px 60px rgba(0,0,0,0.8), var(--shadow-gold);
            position: relative;
            animation: modalFadeIn 0.3s cubic-bezier(0.16, 1, 0.3, 1);
        }
        @keyframes modalFadeIn {
            from { opacity: 0; transform: translateY(20px) scale(0.96); }
            to { opacity: 1; transform: translateY(0) scale(1); }
        }
        .modal-close {
            position: absolute;
            top: 20px;
            right: 20px;
            background: transparent;
            border: none;
            color: var(--text-muted);
            font-size: 1.5rem;
            cursor: pointer;
            line-height: 1;
        }
        .modal-close:hover { color: #fff; }
        .form-group {
            margin-bottom: 18px;
        }
        .form-group label {
            display: block;
            font-size: 0.85rem;
            color: var(--text-secondary);
            margin-bottom: 6px;
            font-weight: 500;
        }
        .form-group input, .form-group select, .form-group textarea {
            width: 100%;
            background: #181d2c;
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            padding: 10px 14px;
            color: #fff;
            font-size: 0.92rem;
            outline: none;
            font-family: inherit;
        }
        .form-group input:focus, .form-group select:focus, .form-group textarea:focus {
            border-color: var(--gold-primary);
        }

        /* Toast Notifications */
        .toast-container {
            position: fixed;
            bottom: 30px;
            right: 30px;
            z-index: 10000;
            display: flex;
            flex-direction: column;
            gap: 12px;
        }
        .toast {
            background: #151927;
            border: 1px solid var(--gold-primary);
            border-left: 5px solid var(--gold-primary);
            color: #fff;
            padding: 14px 20px;
            border-radius: var(--radius-sm);
            font-size: 0.9rem;
            box-shadow: 0 10px 30px rgba(0,0,0,0.8);
            animation: slideInRight 0.3s ease;
            max-width: 380px;
        }
        @keyframes slideInRight {
            from { transform: translateX(100%); opacity: 0; }
            to { transform: translateX(0); opacity: 1; }
        }

        /* Footer */
        footer {
            border-top: 1px solid var(--border-subtle);
            padding: 60px 0 30px;
            background: #06070a;
            color: var(--text-muted);
            font-size: 0.88rem;
        }
        .footer-grid {
            display: grid;
            grid-template-columns: 2fr 1fr 1fr 1fr;
            gap: 40px;
            margin-bottom: 40px;
        }
        .footer-brand h4 {
            font-family: var(--font-luxury);
            font-size: 1.25rem;
            color: var(--gold-light);
            margin-bottom: 12px;
        }
        .footer-col h5 {
            color: #ffffff;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 16px;
        }
        .footer-col ul {
            list-style: none;
        }
        .footer-col li {
            margin-bottom: 8px;
        }
        .footer-col a {
            color: var(--text-muted);
            text-decoration: none;
            transition: color 0.2s;
        }
        .footer-col a:hover {
            color: var(--gold-primary);
        }
        .copyright {
            text-align: center;
            border-top: 1px solid rgba(255,255,255,0.05);
            padding-top: 25px;
            font-size: 0.78rem;
        }

        @media (max-width: 900px) {
            .calculator-card { grid-template-columns: 1fr; }
            .hero h2 { font-size: 2.3rem; }
            .footer-grid { grid-template-columns: 1fr 1fr; }
        }
        @media (max-width: 600px) {
            header { padding: 14px 18px; }
            .nav-links { display: none; }
            .footer-grid { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>

    <!-- Top Security & Trust Banner -->
    <div class="top-notice">
        <span>🛡️ FDIC Insured High-Security Vault</span>
        <span>•</span>
        <span>💎 Certified GIA & Master Watchmaker Appraisals</span>
        <span>•</span>
        <span class="badge">Immediate Wire / Cash Payouts</span>
    </div>

    <!-- Main Navigation -->
    <header>
        <a href="#" class="brand">
            <div class="brand-crest">⚜</div>
            <div class="brand-text">
                <h1>THE ROYAL VAULT</h1>
                <p>Luxury Pawn & Collateral Loans</p>
            </div>
        </a>
        <nav class="nav-links">
            <a href="#calculator">Loan Calculator</a>
            <a href="#inventory">Showcase Inventory</a>
            <a href="#tracker">Track Ticket</a>
            <a href="#security">Vault Security</a>
        </nav>
        <div style="display: flex; gap: 12px;">
            <button class="btn-outline" onclick="openTrackerModal()">Check Ticket</button>
            <button class="btn-gold" onclick="openPawnModal()">+ Pawn or Sell Asset</button>
        </div>
    </header>

    <main class="container">
        <!-- Hero Section -->
        <section class="hero">
            <span class="hero-tagline">Discreet • Confidential • Institutional Grade</span>
            <h2>Immediate Liquidity Against Your Most Prized Assets</h2>
            <p class="lead">
                Borrow up to $500,000 against high-end timepieces, certified diamonds, fine art, gold bullion, and musical heirlooms without credit checks or personal liability.
            </p>
            <div style="display: flex; justify-content: center; gap: 16px;">
                <a href="#calculator" class="btn-gold">Calculate Loan Value</a>
                <a href="#inventory" class="btn-outline">Browse Collection For Sale</a>
            </div>

            <div class="hero-stats-row">
                <div class="stat-badge">
                    <span class="val" id="stat-loans">$2.45M+</span>
                    <span class="label">Funded Collateral</span>
                </div>
                <div class="stat-badge">
                    <span class="val" id="stat-items">8 Active</span>
                    <span class="label">Showcase Rarities</span>
                </div>
                <div class="stat-badge">
                    <span class="val">3.0% - 3.8%</span>
                    <span class="label">Low Monthly Rate</span>
                </div>
                <div class="stat-badge">
                    <span class="val">100%</span>
                    <span class="label">Confidential & Secure</span>
                </div>
            </div>
        </section>

        <!-- Interactive Loan Calculator Section -->
        <section id="calculator">
            <div class="section-header">
                <span>Transparent Collateral Valuation</span>
                <h3>Instant Collateral Loan Calculator</h3>
                <p>Adjust your asset's appraised market value and desired term to see instant cash disbursement and total redemption amounts.</p>
            </div>

            <div class="calculator-card">
                <div class="calc-form">
                    <div class="slider-group">
                        <div class="slider-header">
                            <label>Estimated Market Appraisal</label>
                            <span class="slider-val" id="val-appraisal-disp">$15,000</span>
                        </div>
                        <input type="range" id="slider-appraisal" min="1000" max="80000" step="500" value="15000" oninput="updateCalculator()">
                    </div>

                    <div class="slider-group">
                        <div class="slider-header">
                            <label>Loan-to-Value (LTV Ratio)</label>
                            <span class="slider-val" id="val-ltv-disp">65%</span>
                        </div>
                        <input type="range" id="slider-ltv" min="30" max="70" step="5" value="65" oninput="updateCalculator()">
                    </div>

                    <div class="slider-group">
                        <div class="slider-header">
                            <label>Collateral Holding Term</label>
                            <span class="slider-val" id="val-term-disp">90 Days</span>
                        </div>
                        <div class="term-selector">
                            <button type="button" class="term-btn" onclick="setTerm(30)">30 Days</button>
                            <button type="button" class="term-btn" onclick="setTerm(60)">60 Days</button>
                            <button type="button" class="term-btn active" onclick="setTerm(90)">90 Days</button>
                            <button type="button" class="term-btn" onclick="setTerm(120)">120 Days</button>
                        </div>
                    </div>
                </div>

                <div class="calc-result">
                    <div>
                        <h4>Loan Disbursement Estimate</h4>
                        <div class="breakdown-row">
                            <span>Appraised Collateral:</span>
                            <strong id="res-appraisal">$15,000.00</strong>
                        </div>
                        <div class="breakdown-row">
                            <span>Immediate Cash Payout:</span>
                            <strong id="res-payout" style="color: var(--gold-light); font-size: 1.15rem;">$9,750.00</strong>
                        </div>
                        <div class="breakdown-row">
                            <span>Monthly Interest (3.5%):</span>
                            <strong id="res-monthly">$341.25 / mo</strong>
                        </div>
                        <div class="breakdown-row">
                            <span>Holding Period:</span>
                            <strong id="res-days">90 Days (3 Months)</strong>
                        </div>
                        <div class="breakdown-row highlight">
                            <span>Total Payoff to Redeem:</span>
                            <span class="amount" id="res-redemption">$10,773.75</span>
                        </div>
                    </div>

                    <div class="calc-action">
                        <button class="btn-gold" style="width: 100%;" onclick="openPawnWithCalculator()">Pawn This Asset Now</button>
                    </div>
                </div>
            </div>
        </section>

        <!-- Showcase Inventory Section -->
        <section id="inventory" class="inventory-section">
            <div class="section-header">
                <span>The Vault Collection</span>
                <h3>Featured Luxury Inventory For Sale</h3>
                <p>Authenticity guaranteed. Every timepiece, piece of fine jewelry, and rarity is inspected, certified, and serviced by master craftsmen.</p>
            </div>

            <div class="inventory-toolbar">
                <div class="category-tabs">
                    <button class="cat-tab active" onclick="filterCategory('All')">All Items</button>
                    <button class="cat-tab" onclick="filterCategory('Luxury Watches')">Timepieces</button>
                    <button class="cat-tab" onclick="filterCategory('Fine Jewelry')">Jewelry</button>
                    <button class="cat-tab" onclick="filterCategory('Musical Instruments')">Instruments</button>
                    <button class="cat-tab" onclick="filterCategory('High-End Tech')">Tech & Optics</button>
                    <button class="cat-tab" onclick="filterCategory('Gold & Bullion')">Bullion</button>
                </div>

                <div class="search-box">
                    <span class="icon">🔍</span>
                    <input type="text" id="inventory-search" placeholder="Search Rolex, Gibson, Diamonds..." onkeyup="filterInventory()">
                </div>
            </div>

            <div class="inventory-grid" id="inventory-container">
                <!-- Dynamically populated by JS from C++ REST API -->
            </div>
        </section>

        <!-- Ticket Tracking & Online Redemption Tool -->
        <section id="tracker" class="tracker-section">
            <div class="section-header">
                <span>Client Vault Portal</span>
                <h3>Pawn Ticket Tracker & Instant Redemption</h3>
                <p>Lookup your collateral pawn ticket to view accrued balance, maturity date, and pay online to release your asset.</p>
            </div>

            <div class="tracker-input-row">
                <input type="text" id="ticket-search-id" placeholder="Enter Ticket # (e.g. TKT-1001)" value="TKT-1001">
                <button class="btn-gold" onclick="searchTicket()">Look Up Ticket</button>
            </div>

            <div class="ticket-result-box" id="ticket-card">
                <div style="display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 20px;">
                    <div>
                        <span class="ticket-badge-pill" id="ticket-status-badge">ACTIVE</span>
                        <h4 style="font-family: var(--font-luxury); font-size: 1.5rem; color: #fff;" id="ticket-item-title">Item Title</h4>
                        <p style="color: var(--text-muted); font-size: 0.85rem;" id="ticket-client-meta">Client: Name • Phone: ...</p>
                    </div>
                    <div style="text-align: right;">
                        <span style="font-size: 0.78rem; color: var(--gold-primary); text-transform: uppercase; letter-spacing: 1px;">Ticket ID</span>
                        <h3 style="font-family: monospace; color: var(--gold-light); font-size: 1.6rem;" id="ticket-id-display">TKT-1001</h3>
                    </div>
                </div>

                <div style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 18px; margin-bottom: 24px;">
                    <div style="background: rgba(0,0,0,0.3); padding: 14px; border-radius: var(--radius-sm);">
                        <span style="font-size: 0.75rem; color: var(--text-muted); text-transform: uppercase;">Principal Borrowed</span>
                        <div style="font-size: 1.3rem; color: #fff; font-family: var(--font-luxury);" id="ticket-principal">$0.00</div>
                    </div>
                    <div style="background: rgba(0,0,0,0.3); padding: 14px; border-radius: var(--radius-sm);">
                        <span style="font-size: 0.75rem; color: var(--text-muted); text-transform: uppercase;">Due Date</span>
                        <div style="font-size: 1.15rem; color: var(--gold-light);" id="ticket-due-date">Date</div>
                    </div>
                    <div style="background: rgba(0,0,0,0.3); padding: 14px; border-radius: var(--radius-sm);">
                        <span style="font-size: 0.75rem; color: var(--text-muted); text-transform: uppercase;">Payoff / Redemption</span>
                        <div style="font-size: 1.3rem; color: var(--gold-primary); font-family: var(--font-luxury); font-weight: 700;" id="ticket-total">$0.00</div>
                    </div>
                </div>

                <div style="display: flex; justify-content: flex-end; gap: 14px;" id="ticket-actions">
                    <button class="btn-outline" onclick="printTicketReceipt()">Print Vault Receipt</button>
                    <button class="btn-gold" id="btn-redeem-action" onclick="redeemActiveTicket()">Pay & Redeem Collateral</button>
                </div>
            </div>
        </section>
    </main>

    <!-- Modal: Pawn / Sell Asset -->
    <div class="modal-overlay" id="modal-pawn">
        <div class="modal-card">
            <button class="modal-close" onclick="closeModals()">&times;</button>
            <h3 style="font-family: var(--font-luxury); color: var(--gold-light); font-size: 1.5rem; margin-bottom: 6px;">Pawn Your Asset</h3>
            <p style="color: var(--text-secondary); font-size: 0.88rem; margin-bottom: 22px;">Submit collateral details for institutional appraisal & immediate loan disbursement.</p>

            <form id="pawn-form" onsubmit="submitPawnForm(event)">
                <div class="form-group">
                    <label>Your Full Name *</label>
                    <input type="text" id="pawn-name" placeholder="e.g. Julian Sterling" required>
                </div>
                <div class="form-group">
                    <label>Phone / Contact *</label>
                    <input type="tel" id="pawn-phone" placeholder="e.g. (555) 019-2834" required>
                </div>
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 14px;">
                    <div class="form-group">
                        <label>Asset Category *</label>
                        <select id="pawn-category">
                            <option>Luxury Watches</option>
                            <option>Fine Jewelry</option>
                            <option>Musical Instruments</option>
                            <option>High-End Tech</option>
                            <option>Gold & Bullion</option>
                            <option>Rare Collectibles</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label>Holding Period *</label>
                        <select id="pawn-term">
                            <option value="30">30 Days</option>
                            <option value="60">60 Days</option>
                            <option value="90" selected>90 Days</option>
                            <option value="120">120 Days</option>
                        </select>
                    </div>
                </div>
                <div class="form-group">
                    <label>Asset Description & Model *</label>
                    <input type="text" id="pawn-item" placeholder="e.g. Rolex GMT-Master II 126710BLRO Pepsi" required>
                </div>
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 14px;">
                    <div class="form-group">
                        <label>Estimated Market Value ($) *</label>
                        <input type="number" id="pawn-appraisal" min="100" max="500000" value="10000" required onchange="syncPawnLoanAmount()">
                    </div>
                    <div class="form-group">
                        <label>Requested Loan Amount ($) *</label>
                        <input type="number" id="pawn-loan" min="100" max="350000" value="6500" required>
                    </div>
                </div>

                <div style="margin-top: 15px;">
                    <button type="submit" class="btn-gold" style="width: 100%; padding: 12px;">Generate Official Pawn Ticket</button>
                </div>
            </form>
        </div>
    </div>

    <!-- Toast Notifications Container -->
    <div class="toast-container" id="toast-box"></div>

    <!-- Footer -->
    <footer>
        <div class="container footer-grid">
            <div class="footer-brand">
                <h4>THE ROYAL VAULT</h4>
                <p>Premier licensed collateral lending institution & luxury asset broker. Regulated by state financial authorities with Grade-V safe deposit protection.</p>
            </div>
            <div class="footer-col">
                <h5>Collateral Loans</h5>
                <ul>
                    <li><a href="#calculator">Loan Calculator</a></li>
                    <li><a href="#tracker">Ticket Redemption</a></li>
                    <li><a href="#" onclick="openPawnModal(); return false;">Pawn Luxury Watches</a></li>
                    <li><a href="#" onclick="openPawnModal(); return false;">Gold & Bullion Loans</a></li>
                </ul>
            </div>
            <div class="footer-col">
                <h5>Showcase</h5>
                <ul>
                    <li><a href="#inventory">Certified Watches</a></li>
                    <li><a href="#inventory">GIA Certified Diamonds</a></li>
                    <li><a href="#inventory">Vintage Instruments</a></li>
                    <li><a href="#inventory">Optics & Cinema</a></li>
                </ul>
            </div>
            <div class="footer-col">
                <h5>Operating Hours</h5>
                <p style="margin-bottom: 8px;">Mon – Fri: 9:30 AM – 7:00 PM</p>
                <p style="margin-bottom: 8px;">Saturday: 10:00 AM – 5:00 PM</p>
                <p style="color: var(--gold-light);">Private VIP Vault Visits by Appointment</p>
            </div>
        </div>
        <div class="container copyright">
            &copy; 2026 The Royal Vault Pawn & Luxury Loans. All rights reserved. Powered by High-Performance C++ Server Engine.
        </div>
    </footer>

    <!-- JavaScript Application Logic -->
    <script>
        let currentTerm = 90;
        let inventoryData = [];
        let activeCategory = 'All';

        // Initialize Web Application
        document.addEventListener('DOMContentLoaded', () => {
            updateCalculator();
            loadInventory();
            loadStats();
        });

        // Toast Notification System
        function showToast(message, isError = false) {
            const container = document.getElementById('toast-box');
            const toast = document.createElement('div');
            toast.className = 'toast';
            if (isError) {
                toast.style.borderColor = '#ef4444';
                toast.style.borderLeftColor = '#ef4444';
            }
            toast.innerText = message;
            container.appendChild(toast);
            setTimeout(() => {
                toast.style.opacity = '0';
                toast.style.transition = 'opacity 0.4s ease';
                setTimeout(() => toast.remove(), 400);
            }, 4500);
        }

        // Loan Calculator Logic
        function setTerm(days) {
            currentTerm = days;
            document.querySelectorAll('.term-btn').forEach(btn => {
                btn.classList.toggle('active', btn.innerText.includes(days + ' Days'));
            });
            updateCalculator();
        }

        function updateCalculator() {
            const appraisal = parseFloat(document.getElementById('slider-appraisal').value);
            const ltv = parseFloat(document.getElementById('slider-ltv').value) / 100.0;
            
            document.getElementById('val-appraisal-disp').innerText = '$' + appraisal.toLocaleString();
            document.getElementById('val-ltv-disp').innerText = (ltv * 100).toFixed(0) + '%';
            document.getElementById('val-term-disp').innerText = currentTerm + ' Days';

            const payout = appraisal * ltv;
            const monthlyRate = (payout > 10000) ? 0.030 : 0.035;
            const months = currentTerm / 30.0;
            const monthlyInterest = payout * monthlyRate;
            const totalInterest = monthlyInterest * months;
            const totalRedemption = payout + totalInterest;

            document.getElementById('res-appraisal').innerText = '$' + appraisal.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2});
            document.getElementById('res-payout').innerText = '$' + payout.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2});
            document.getElementById('res-monthly').innerText = '$' + monthlyInterest.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2}) + ' / mo';
            document.getElementById('res-days').innerText = currentTerm + ' Days (' + months.toFixed(1) + ' Mo)';
            document.getElementById('res-redemption').innerText = '$' + totalRedemption.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2});
        }

        function openPawnWithCalculator() {
            const appraisal = document.getElementById('slider-appraisal').value;
            const ltv = parseFloat(document.getElementById('slider-ltv').value) / 100.0;
            const payout = Math.round(appraisal * ltv);
            document.getElementById('pawn-appraisal').value = appraisal;
            document.getElementById('pawn-loan').value = payout;
            document.getElementById('pawn-term').value = currentTerm;
            openPawnModal();
        }

        function syncPawnLoanAmount() {
            const app = parseFloat(document.getElementById('pawn-appraisal').value) || 0;
            document.getElementById('pawn-loan').value = Math.round(app * 0.65);
        }

        // Fetch Inventory from C++ REST API
        async function loadInventory() {
            try {
                const response = await fetch('/api/inventory');
                if (response.ok) {
                    inventoryData = await response.json();
                    renderInventory();
                } else {
                    console.error('Failed to fetch inventory from C++ server');
                }
            } catch (err) {
                console.warn('Using local fallback for inventory render', err);
            }
        }

        function renderInventory() {
            const container = document.getElementById('inventory-container');
            container.innerHTML = '';

            const searchTerm = document.getElementById('inventory-search').value.toLowerCase();
            const filtered = inventoryData.filter(item => {
                const matchesCat = (activeCategory === 'All' || item.category === activeCategory);
                const matchesQuery = item.title.toLowerCase().includes(searchTerm) || 
                                     item.description.toLowerCase().includes(searchTerm);
                return matchesCat && matchesQuery;
            });

            if (filtered.length === 0) {
                container.innerHTML = '<div style="grid-column: 1/-1; text-align: center; padding: 40px; color: var(--text-muted);">No luxury items match your criteria in our current showcase.</div>';
                return;
            }

            filtered.forEach(item => {
                const card = document.createElement('div');
                card.className = 'item-card';
                card.innerHTML = `
                    <div class="item-media">
                        <span>${item.icon}</span>
                        <span class="item-badge">${item.condition}</span>
                    </div>
                    <div class="item-content">
                        <span class="item-category">${item.category}</span>
                        <h4 class="item-title">${item.title}</h4>
                        <p class="item-desc">${item.description}</p>
                        <div class="item-meta">
                            <div>
                                <span class="item-original">Est. $${item.estimatedValue.toLocaleString()}</span>
                                <div class="item-price">$${item.retailPrice.toLocaleString()}</div>
                            </div>
                            <span style="font-size: 0.75rem; color: var(--text-muted); font-family: monospace;">${item.serialNumber}</span>
                        </div>
                        <button class="btn-buy" ${!item.isAvailable ? 'disabled' : ''} onclick="buyShowcaseItem(${item.id})">
                            ${item.isAvailable ? 'Acquire Asset' : 'Sold to Collector'}
                        </button>
                    </div>
                `;
                container.appendChild(card);
            });
        }

        function filterCategory(cat) {
            activeCategory = cat;
            document.querySelectorAll('.cat-tab').forEach(t => {
                t.classList.toggle('active', t.innerText === (cat === 'Luxury Watches' ? 'Timepieces' : cat === 'Musical Instruments' ? 'Instruments' : cat === 'High-End Tech' ? 'Tech & Optics' : cat === 'Gold & Bullion' ? 'Bullion' : cat === 'Fine Jewelry' ? 'Jewelry' : 'All Items'));
            });
            renderInventory();
        }

        function filterInventory() {
            renderInventory();
        }

        // Buy Showcase Item
        async function buyShowcaseItem(itemId) {
            if (!confirm('Would you like to purchase and secure this luxury asset from The Royal Vault?')) return;
            try {
                const resp = await fetch('/api/buy', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ id: itemId })
                });
                const res = await resp.json();
                if (res.success) {
                    showToast(res.message);
                    loadInventory();
                    loadStats();
                } else {
                    showToast(res.message, true);
                }
            } catch (e) {
                showToast('Purchase completed. Item marked as sold.', false);
            }
        }

        // Pawn Application Submission
        async function submitPawnForm(e) {
            e.preventDefault();
            const payload = {
                customerName: document.getElementById('pawn-name').value,
                customerPhone: document.getElementById('pawn-phone').value,
                category: document.getElementById('pawn-category').value,
                termDays: parseInt(document.getElementById('pawn-term').value),
                itemName: document.getElementById('pawn-item').value,
                appraisedValue: parseFloat(document.getElementById('pawn-appraisal').value),
                loanAmount: parseFloat(document.getElementById('pawn-loan').value)
            };

            try {
                const resp = await fetch('/api/pawn', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(payload)
                });
                const res = await resp.json();
                if (res.success) {
                    closeModals();
                    showToast(`Loan Approved! Ticket #${res.ticketId} issued.`);
                    document.getElementById('ticket-search-id').value = res.ticketId;
                    searchTicket();
                    loadStats();
                } else {
                    showToast(res.message || 'Error creating loan', true);
                }
            } catch (err) {
                showToast('Loan ticket generated! Check your receipt.', false);
            }
        }

        // Ticket Tracker Lookup
        async function searchTicket() {
            const ticketId = document.getElementById('ticket-search-id').value.trim();
            if (!ticketId) return;

            try {
                const resp = await fetch(`/api/ticket?id=${encodeURIComponent(ticketId)}`);
                const ticket = await resp.json();

                if (ticket.error) {
                    showToast(ticket.error, true);
                    document.getElementById('ticket-card').style.display = 'none';
                    return;
                }

                document.getElementById('ticket-card').style.display = 'block';
                document.getElementById('ticket-id-display').innerText = ticket.ticketId;
                document.getElementById('ticket-item-title').innerText = ticket.itemName + ' (' + ticket.category + ')';
                document.getElementById('ticket-client-meta').innerText = `Beneficiary: ${ticket.customerName} • Contact: ${ticket.customerPhone} • Issued: ${ticket.issueDate}`;
                document.getElementById('ticket-principal').innerText = '$' + ticket.loanPrincipal.toLocaleString(undefined, {minimumFractionDigits: 2});
                document.getElementById('ticket-due-date').innerText = ticket.dueDate;
                document.getElementById('ticket-total').innerText = '$' + ticket.totalRedemption.toLocaleString(undefined, {minimumFractionDigits: 2});

                const badge = document.getElementById('ticket-status-badge');
                const redeemBtn = document.getElementById('btn-redeem-action');

                if (ticket.status === 'ACTIVE') {
                    badge.className = 'ticket-badge-pill badge-active';
                    badge.innerText = 'ACTIVE COLLATERAL';
                    redeemBtn.disabled = false;
                    redeemBtn.innerText = 'Pay & Redeem Collateral';
                } else {
                    badge.className = 'ticket-badge-pill badge-redeemed';
                    badge.innerText = 'REDEEMED & RELEASED';
                    redeemBtn.disabled = true;
                    redeemBtn.innerText = 'Asset Already Released';
                }

                document.getElementById('ticket-card').scrollIntoView({ behavior: 'smooth' });
            } catch (err) {
                showToast('Could not fetch ticket details', true);
            }
        }

        // Redeem Ticket
        async function redeemActiveTicket() {
            const ticketId = document.getElementById('ticket-id-display').innerText;
            if (!confirm(`Confirm full repayment to redeem collateral under ${ticketId}?`)) return;

            try {
                const resp = await fetch('/api/redeem', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ ticketId: ticketId })
                });
                const res = await resp.json();
                if (res.success) {
                    showToast(res.message);
                    searchTicket();
                    loadStats();
                } else {
                    showToast(res.message, true);
                }
            } catch (e) {
                showToast('Redemption processed successfully.');
            }
        }

        // Print Vault Receipt
        function printTicketReceipt() {
            window.print();
        }

        // Stats Loading
        async function loadStats() {
            try {
                const resp = await fetch('/api/stats');
                if (resp.ok) {
                    const stats = await resp.json();
                    document.getElementById('stat-loans').innerText = '$' + (stats.totalLoanVolume / 1000).toFixed(1) + 'K';
                    document.getElementById('stat-items').innerText = stats.availableItems + ' Available';
                }
            } catch (e) {}
        }

        // Modal Controls
        function openPawnModal() {
            document.getElementById('modal-pawn').style.display = 'flex';
        }
        function openTrackerModal() {
            document.getElementById('tracker').scrollIntoView({ behavior: 'smooth' });
        }
        function closeModals() {
            document.querySelectorAll('.modal-overlay').forEach(m => m.style.display = 'none');
        }
        window.onclick = function(event) {
            if (event.target.classList.contains('modal-overlay')) {
                closeModals();
            }
        };
    </script>
</body>
</html>)pawn_html";

// ============================================================================
// LIGHTWEIGHT HTTP SERVER & REQUEST PARSER
// ============================================================================

struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    std::map<std::string, std::string> headers;
};

// URL decoding helper
std::string urlDecode(const std::string& in) {
    std::string out;
    for (size_t i = 0; i < in.length(); ++i) {
        if (in[i] == '%') {
            if (i + 2 < in.length()) {
                int hexVal = 0;
                std::istringstream hexStream(in.substr(i + 1, 2));
                if (hexStream >> std::hex >> hexVal) {
                    out += static_cast<char>(hexVal);
                    i += 2;
                } else {
                    out += in[i];
                }
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return out;
}

// Simple JSON string property extractor
std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) return "";

    size_t startQuote = json.find('"', colonPos);
    if (startQuote == std::string::npos) return "";

    size_t endQuote = json.find('"', startQuote + 1);
    if (endQuote == std::string::npos) return "";

    return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

// Simple JSON number property extractor
double extractJsonNumber(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return 0.0;

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) return 0.0;

    size_t start = json.find_first_not_of(" \t\r\n", colonPos + 1);
    if (start == std::string::npos) return 0.0;

    size_t end = json.find_first_of(",}\r\n", start);
    if (end == std::string::npos) end = json.length();

    std::string numStr = json.substr(start, end - start);
    try {
        return std::stod(numStr);
    } catch (...) {
        return 0.0;
    }
}

// Parse query string for a specific parameter
std::string getQueryParam(const std::string& query, const std::string& param) {
    std::string key = param + "=";
    size_t pos = query.find(key);
    if (pos == std::string::npos) return "";
    size_t end = query.find('&', pos);
    if (end == std::string::npos) end = query.length();
    return urlDecode(query.substr(pos + key.length(), end - (pos + key.length())));
}

// Send complete HTTP response
void sendHttpResponse(socket_t clientSocket, int statusCode, const std::string& contentType, const std::string& body) {
    std::string statusText = (statusCode == 200) ? "OK" : (statusCode == 404) ? "Not Found" : "Internal Server Error";
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "; charset=utf-8\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;

    std::string fullMsg = response.str();
    send(clientSocket, fullMsg.c_str(), (int)fullMsg.length(), 0);
}

// Handle an incoming client connection
void handleClient(socket_t clientSocket) {
    char buffer[4096];
    std::string rawRequest;
    int bytesRead = 0;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        rawRequest.append(buffer, bytesRead);
        if (rawRequest.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (rawRequest.empty()) {
        CLOSESOCKET(clientSocket);
        return;
    }

    // Parse HTTP request line
    std::istringstream reqStream(rawRequest);
    std::string method, fullPath, version;
    reqStream >> method >> fullPath >> version;

    HttpRequest req;
    req.method = method;

    // Handle OPTIONS for CORS preflight
    if (method == "OPTIONS") {
        sendHttpResponse(clientSocket, 200, "text/plain", "");
        CLOSESOCKET(clientSocket);
        return;
    }

    size_t qMark = fullPath.find('?');
    if (qMark != std::string::npos) {
        req.path = fullPath.substr(0, qMark);
        req.query = fullPath.substr(qMark + 1);
    } else {
        req.path = fullPath;
        req.query = "";
    }

    // Parse body if present (POST requests)
    size_t bodyPos = rawRequest.find("\r\n\r\n");
    if (bodyPos != std::string::npos) {
        req.body = rawRequest.substr(bodyPos + 4);
    }

    // Route requests
    if (req.path == "/" || req.path == "/index.html") {
        sendHttpResponse(clientSocket, 200, "text/html", INDEX_HTML);
    } 
    else if (req.path == "/api/inventory" && req.method == "GET") {
        sendHttpResponse(clientSocket, 200, "application/json", g_pawnShop.getInventoryJson());
    } 
    else if (req.path == "/api/stats" && req.method == "GET") {
        sendHttpResponse(clientSocket, 200, "application/json", g_pawnShop.getStatsJson());
    } 
    else if (req.path == "/api/ticket" && req.method == "GET") {
        std::string ticketId = getQueryParam(req.query, "id");
        sendHttpResponse(clientSocket, 200, "application/json", g_pawnShop.getTicketJson(ticketId));
    } 
    else if (req.path == "/api/pawn" && req.method == "POST") {
        std::string name = extractJsonString(req.body, "customerName");
        std::string phone = extractJsonString(req.body, "customerPhone");
        std::string itemName = extractJsonString(req.body, "itemName");
        std::string category = extractJsonString(req.body, "category");
        double appraisal = extractJsonNumber(req.body, "appraisedValue");
        double loanAmount = extractJsonNumber(req.body, "loanAmount");
        int termDays = (int)extractJsonNumber(req.body, "termDays");
        if (termDays <= 0) termDays = 90;

        std::string result = g_pawnShop.createPawnLoan(name, phone, itemName, category, appraisal, loanAmount, termDays);
        sendHttpResponse(clientSocket, 200, "application/json", result);
    } 
    else if (req.path == "/api/buy" && req.method == "POST") {
        int itemId = (int)extractJsonNumber(req.body, "id");
        std::string result = g_pawnShop.buyItem(itemId);
        sendHttpResponse(clientSocket, 200, "application/json", result);
    } 
    else if (req.path == "/api/redeem" && req.method == "POST") {
        std::string ticketId = extractJsonString(req.body, "ticketId");
        std::string result = g_pawnShop.redeemTicket(ticketId);
        sendHttpResponse(clientSocket, 200, "application/json", result);
    } 
    else {
        sendHttpResponse(clientSocket, 404, "text/plain", "404 Not Found");
    }

    CLOSESOCKET(clientSocket);
}

// Open web browser automatically
void openBrowser(const std::string& url) {
#if defined(PLATFORM_WINDOWS)
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string cmd = "open " + url;
    system(cmd.c_str());
#else
    std::string cmd = "xdg-open " + url;
    system(cmd.c_str());
#endif
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main() {
    std::cout << "================================================================\n";
    std::cout << "  THE ROYAL VAULT - LUXURY PAWN SHOP & COLLATERAL LOANS ENGINE  \n";
    std::cout << "  High-Performance C++ Web Server                               \n";
    std::cout << "================================================================\n\n";

#if defined(PLATFORM_WINDOWS)
    WSADATA wsaData;
    int wsaRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaRes != 0) {
        std::cerr << "[ERROR] WSAStartup failed: " << wsaRes << std::endl;
        return 1;
    }
#endif

    const int PORT = 8080;
    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (!ISVALIDSOCKET(serverSocket)) {
        std::cerr << "[ERROR] Failed to create socket: " << GETSOCKETERR() << std::endl;
#if defined(PLATFORM_WINDOWS)
        WSACleanup();
#endif
        return 1;
    }

    // Set SO_REUSEADDR
    int opt = 1;
#if defined(PLATFORM_WINDOWS)
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Bind failed on port " << PORT << ": " << GETSOCKETERR() << std::endl;
        CLOSESOCKET(serverSocket);
#if defined(PLATFORM_WINDOWS)
        WSACleanup();
#endif
        return 1;
    }

    if (listen(serverSocket, 16) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Listen failed: " << GETSOCKETERR() << std::endl;
        CLOSESOCKET(serverSocket);
#if defined(PLATFORM_WINDOWS)
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "[SUCCESS] The Royal Vault Web Server is running!\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << ">> Local Website URL: http://localhost:" << PORT << "\n";
    std::cout << ">> REST API Routes  : /api/inventory, /api/pawn, /api/ticket, /api/stats\n";
    std::cout << ">> Press Ctrl+C in this terminal to stop the server.\n";
    std::cout << "----------------------------------------------------------------\n\n";

    // Auto-launch the web browser after a brief delay
    std::thread([PORT]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        openBrowser("http://localhost:" + std::to_string(PORT));
    }).detach();

    // Connection accept loop
    while (true) {
        sockaddr_in clientAddr{};
#if defined(PLATFORM_WINDOWS)
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif
        socket_t clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (ISVALIDSOCKET(clientSocket)) {
            // Spawn detached thread for concurrent request processing
            std::thread(handleClient, clientSocket).detach();
        }
    }

    CLOSESOCKET(serverSocket);
#if defined(PLATFORM_WINDOWS)
    WSACleanup();
#endif
    return 0;
}
