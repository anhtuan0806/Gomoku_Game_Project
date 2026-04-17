#include "SaveLoadSystem.h"
#include "../ApplicationTypes/GameConstants.h"
#include <filesystem>
#include <fstream>
#include <windows.h>
#include <ctime>
#include <iomanip>
#include <sstream>

// Magic number de kiem tra file hop le (tranh load file rac)
static const uint32_t SAVE_MAGIC   = 0xCA05A1E2;
static const uint32_t SAVE_VERSION = 5; // v5: Thêm matchWins + totalTimePossessed

// ---------- Helpers: Write/Read cac kieu dong ----------

static void WriteWStr(std::ofstream& f, const std::wstring& s) {
    uint32_t len = (uint32_t)s.size();
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        f.write(reinterpret_cast<const char*>(s.data()), len * sizeof(wchar_t));
    }
}

static bool ReadWStr(std::ifstream& f, std::wstring& s) {
    uint32_t len = 0;
    if (!f.read(reinterpret_cast<char*>(&len), sizeof(len))) {
        return false;
    }
    if (len > 4096) {
        return false; // Gioi han bao ve chong file loi
    }
    s.resize(len);
    if (len > 0 && !f.read(reinterpret_cast<char*>(&s[0]), len * sizeof(wchar_t))) {
        return false;
    }
    return true;
}

static void WriteStr(std::ofstream& f, const std::string& s) {
    uint32_t len = (uint32_t)s.size();
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        f.write(s.data(), len);
    }
}

static bool ReadStr(std::ifstream& f, std::string& s) {
    uint32_t len = 0;
    if (!f.read(reinterpret_cast<char*>(&len), sizeof(len))) {
        return false;
    }
    if (len > 4096) {
        return false;
    }
    s.resize(len);
    if (len > 0 && !f.read(&s[0], len)) {
        return false;
    }
    return true;
}

template<typename T>
static void WriteVec(std::ofstream& f, const std::vector<T>& v) {
    uint32_t len = (uint32_t)v.size();
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        f.write(reinterpret_cast<const char*>(v.data()), len * sizeof(T));
    }
}

template<typename T>
static bool ReadVec(std::ifstream& f, std::vector<T>& v) {
    uint32_t len = 0;
    if (!f.read(reinterpret_cast<char*>(&len), sizeof(len))) {
        return false;
    }
    if (len > 100000) {
        return false; // Gioi han bao ve
    }
    v.resize(len);
    if (len > 0 && !f.read(reinterpret_cast<char*>(v.data()), len * sizeof(T))) {
        return false;
    }
    return true;
}

// ---------- Serialize PlayerInfo2 ----------

static void WritePlayer(std::ofstream& f, const PlayerInfo2& p) {
    WriteWStr(f, p.name);
    WriteStr(f, p.avatarPath);
    f.write(&p.piece, sizeof(p.piece));
    f.write(reinterpret_cast<const char*>(&p.totalWins), sizeof(p.totalWins));
    f.write(reinterpret_cast<const char*>(&p.matchWins), sizeof(p.matchWins));
    f.write(reinterpret_cast<const char*>(&p.movesCount), sizeof(p.movesCount));
    f.write(reinterpret_cast<const char*>(&p.maxTurnTime), sizeof(p.maxTurnTime));
    f.write(reinterpret_cast<const char*>(&p.totalTimePossessed), sizeof(p.totalTimePossessed));
}

static bool ReadPlayer(std::ifstream& f, PlayerInfo2& p, uint32_t version) {
    if (!ReadWStr(f, p.name)) {
        return false;
    }
    if (!ReadStr(f, p.avatarPath)) {
        return false;
    }
    if (!f.read(&p.piece, sizeof(p.piece))) {
        return false;
    }
    if (!f.read(reinterpret_cast<char*>(&p.totalWins), sizeof(p.totalWins))) {
        return false;
    }
    if (version >= 5) {
        if (!f.read(reinterpret_cast<char*>(&p.matchWins), sizeof(p.matchWins))) return false;
    } else {
        p.matchWins = 0;
    }
    if (!f.read(reinterpret_cast<char*>(&p.movesCount), sizeof(p.movesCount))) {
        return false;
    }
    if (!f.read(reinterpret_cast<char*>(&p.maxTurnTime), sizeof(p.maxTurnTime))) {
        return false;
    }
    if (version >= 5) {
        if (!f.read(reinterpret_cast<char*>(&p.totalTimePossessed), sizeof(p.totalTimePossessed))) return false;
    } else {
        p.totalTimePossessed = 0.0f;
    }
    return true;
}

// ---------- Public API ----------

bool SaveMatchData(const PlayState* state, const std::wstring& filename) {
    CreateDirectoryW(L"Asset", NULL);
    CreateDirectoryW(L"Asset/save", NULL);

    std::filesystem::path savePath(filename);
    std::ofstream file(savePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(&SAVE_MAGIC),   sizeof(SAVE_MAGIC));
    file.write(reinterpret_cast<const char*>(&SAVE_VERSION), sizeof(SAVE_VERSION));
    
    // Serialise Save Name
    WriteWStr(file, state->saveName);

    // Capture and write Timestamp (v4)
    std::wstring ts = state->saveTimestamp;
    if (ts.empty()) {
        auto t = std::time(nullptr);
        struct tm tm_buf;
        localtime_s(&tm_buf, &t);
        std::wstringstream wss;
        // Format: Day/Month/Year Hour:Minute
        wss << std::setfill(L'0') << std::setw(2) << tm_buf.tm_mday << L"/"
            << std::setfill(L'0') << std::setw(2) << (tm_buf.tm_mon + 1) << L"/"
            << (tm_buf.tm_year + 1900) << L" "
            << std::setfill(L'0') << std::setw(2) << tm_buf.tm_hour << L":"
            << std::setfill(L'0') << std::setw(2) << tm_buf.tm_min;
        ts = wss.str();
    }
    WriteWStr(file, ts);

    // POD fields
    file.write(reinterpret_cast<const char*>(&state->gameMode),        sizeof(state->gameMode));
    file.write(reinterpret_cast<const char*>(&state->matchType),       sizeof(state->matchType));
    file.write(reinterpret_cast<const char*>(&state->isP1Turn),        sizeof(state->isP1Turn));
    file.write(reinterpret_cast<const char*>(&state->countdownTime),   sizeof(state->countdownTime));
    file.write(reinterpret_cast<const char*>(&state->timeRemaining),   sizeof(state->timeRemaining));
    file.write(reinterpret_cast<const char*>(&state->boardSize),       sizeof(state->boardSize));
    file.write(reinterpret_cast<const char*>(state->board),            sizeof(state->board));
    file.write(reinterpret_cast<const char*>(&state->cursorRow),       sizeof(state->cursorRow));
    file.write(reinterpret_cast<const char*>(&state->cursorCol),       sizeof(state->cursorCol));
    file.write(reinterpret_cast<const char*>(&state->lastMoveRow),     sizeof(state->lastMoveRow));
    file.write(reinterpret_cast<const char*>(&state->lastMoveCol),     sizeof(state->lastMoveCol));
    file.write(reinterpret_cast<const char*>(&state->status),          sizeof(state->status));
    file.write(reinterpret_cast<const char*>(&state->winner),          sizeof(state->winner));
    file.write(reinterpret_cast<const char*>(&state->difficulty),      sizeof(state->difficulty));
    file.write(reinterpret_cast<const char*>(&state->targetScore),     sizeof(state->targetScore));
    file.write(reinterpret_cast<const char*>(&state->matchDuration),   sizeof(state->matchDuration));
    file.write(reinterpret_cast<const char*>(&state->p1TotalTimeLeft), sizeof(state->p1TotalTimeLeft));
    file.write(reinterpret_cast<const char*>(&state->p2TotalTimeLeft), sizeof(state->p2TotalTimeLeft));
    file.write(reinterpret_cast<const char*>(&state->isMatchTimed),    sizeof(state->isMatchTimed));

    // Dynamic fields
    WritePlayer(file, state->p1);
    WritePlayer(file, state->p2);
    WriteVec(file, state->winningCells);
    WriteVec(file, state->matchHistory);
    WriteVec(file, state->redoStack);

    file.close();
    return true;
}

bool LoadMatchData(PlayState* state, const std::wstring& filename) {
    std::filesystem::path loadPath(filename);
    std::ifstream file(loadPath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    uint32_t magic = 0, version = 0;
    file.read(reinterpret_cast<char*>(&magic),   sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    
    // Support version >= 3
    if (magic != SAVE_MAGIC || version < 3) {
        file.close();
        return false;
    }

    // Read Save Name
    if (!ReadWStr(file, state->saveName)) {
        return false;
    }

    // Read Timestamp (v4+)
    if (version >= 4) {
        if (!ReadWStr(file, state->saveTimestamp)) return false;
    } else {
        state->saveTimestamp = L"N/A (Bản cũ)";
    }

    // POD fields
    if (!file.read(reinterpret_cast<char*>(&state->gameMode), sizeof(state->gameMode))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->matchType), sizeof(state->matchType))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->isP1Turn), sizeof(state->isP1Turn))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->countdownTime), sizeof(state->countdownTime))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->timeRemaining), sizeof(state->timeRemaining))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->boardSize), sizeof(state->boardSize))) return false;
    if (!file.read(reinterpret_cast<char*>(state->board), sizeof(state->board))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->cursorRow), sizeof(state->cursorRow))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->cursorCol), sizeof(state->cursorCol))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->lastMoveRow), sizeof(state->lastMoveRow))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->lastMoveCol), sizeof(state->lastMoveCol))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->status), sizeof(state->status))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->winner), sizeof(state->winner))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->difficulty), sizeof(state->difficulty))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->targetScore), sizeof(state->targetScore))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->matchDuration), sizeof(state->matchDuration))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->p1TotalTimeLeft), sizeof(state->p1TotalTimeLeft))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->p2TotalTimeLeft), sizeof(state->p2TotalTimeLeft))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->isMatchTimed), sizeof(state->isMatchTimed))) return false;

    // Dynamic fields
    if (!ReadPlayer(file, state->p1, version)) return false;
    if (!ReadPlayer(file, state->p2, version)) return false;
    if (!ReadVec(file, state->winningCells)) return false;
    if (!ReadVec(file, state->matchHistory)) return false;
    if (!ReadVec(file, state->redoStack)) return false;

    file.close();
    return true;
}

std::wstring GetSavePath(int slot) {
    return L"Asset/save/slot_" + std::to_wstring(slot) + L".bin";
}

bool CheckSaveExists(int slot) {
    DWORD dwAttrib = GetFileAttributesW(GetSavePath(slot).c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool DeleteSave(int slot) {
    if (!CheckSaveExists(slot)) return false;
    std::filesystem::path p(GetSavePath(slot));
    return std::filesystem::remove(p);
}

bool RenameSave(int slot, const std::wstring& newName) {
    if (!CheckSaveExists(slot)) return false;
    
    PlayState temp;
    if (!LoadMatchData(&temp, GetSavePath(slot))) {
        return false;
    }
    
    temp.saveName = newName.substr(0, 15); 
    // Khi đổi tên, ta dùng lại saveTimestamp cũ chứ không cập nhật mới
    return SaveMatchData(&temp, GetSavePath(slot));
}

std::wstring GetSaveDisplayName(int slot) {
    if (!CheckSaveExists(slot)) return L"";
    
    std::filesystem::path loadPath(GetSavePath(slot));
    std::ifstream file(loadPath, std::ios::binary);
    if (!file.is_open()) return L"";

    uint32_t magic = 0, version = 0;
    file.read(reinterpret_cast<char*>(&magic),   sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    
    if (magic != SAVE_MAGIC || version < 3) {
        file.close();
        return L"Bản lưu cũ (v" + std::to_wstring(version) + L")";
    }

    std::wstring name;
    if (ReadWStr(file, name)) {
        file.close();
        return name;
    }
    
    file.close();
    return L"File lỗi";
}

SaveMetadata GetSaveMetadata(int slot) {
    SaveMetadata meta;
    meta.exists = false;
    meta.version = 0;

    if (!CheckSaveExists(slot)) return meta;
    
    std::filesystem::path loadPath(GetSavePath(slot));
    std::ifstream file(loadPath, std::ios::binary);
    if (!file.is_open()) return meta;

    uint32_t magic = 0, version = 0;
    file.read(reinterpret_cast<char*>(&magic),   sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    
    if (magic != SAVE_MAGIC) {
        file.close();
        return meta;
    }

    meta.exists = true;
    meta.version = version;

    if (version < 3) {
        meta.name = L"Bản cũ (v" + std::to_wstring(version) + L")";
        meta.timestamp = L"N/A";
        file.close();
        return meta;
    }

    // Version >= 3
    ReadWStr(file, meta.name);
    
    if (version >= 4) {
        ReadWStr(file, meta.timestamp);
    } else {
        meta.timestamp = L"N/A (Bản cũ)";
    }

    // Read basic meta for v3/v4
    file.read(reinterpret_cast<char*>(&meta.mode),       sizeof(meta.mode));
    file.read(reinterpret_cast<char*>(&meta.type),       sizeof(meta.type));
    
    // Skip POD fields to get wins
    // p1Turn(1) + countdownTime(4) + timeRemaining(4) + boardSize(4) + board(max*max*4) + cursorRow(4) + cursorCol(4) + lastMoveRow(4) + lastMoveCol(4) + status(4) + winner(4) + difficulty(4) + targetScore(4) + matchDuration(4) + p1TotalTimeLeft(4) + p2TotalTimeLeft(4) + isMatchTimed(1)
    // Board size depends on constant MAX_BOARD_SIZE (likely 20x20 = 400 * 4 = 1600 bytes)
    // Better strategy: Read them properly or seek
    // For safety, let's just read them
    int dummyI; bool dummyB; int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE]; float dummyF;
    file.read(reinterpret_cast<char*>(&dummyB), sizeof(dummyB));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&meta.boardSize), sizeof(meta.boardSize));
    file.read(reinterpret_cast<char*>(board), sizeof(board));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&meta.difficulty), sizeof(meta.difficulty));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyF), sizeof(dummyF));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyB), sizeof(dummyB));

    // Player 1
    std::wstring p1Name; std::string p1Ava; char p1Piece; PlayerInfo2 tempP1{};
    ReadWStr(file, p1Name);
    ReadStr(file, p1Ava);
    file.read(&p1Piece, sizeof(p1Piece));
    file.read(reinterpret_cast<char*>(&meta.p1Wins), sizeof(meta.p1Wins));
    // v5: skip matchWins
    if (version >= 5) { int dummy = 0; file.read(reinterpret_cast<char*>(&dummy), sizeof(dummy)); }
    // movesCount + maxTurnTime
    file.read(reinterpret_cast<char*>(&dummyI), sizeof(dummyI));
    file.read(reinterpret_cast<char*>(&dummyF), sizeof(dummyF));
    // v5: skip totalTimePossessed
    if (version >= 5) { float dummyP = 0; file.read(reinterpret_cast<char*>(&dummyP), sizeof(dummyP)); }
    
    std::wstring p2Name; std::string p2Ava; char p2Piece;
    ReadWStr(file, p2Name);
    ReadStr(file, p2Ava);
    file.read(&p2Piece, sizeof(p2Piece));
    file.read(reinterpret_cast<char*>(&meta.p2Wins), sizeof(meta.p2Wins));

    file.close();
    return meta;
}
