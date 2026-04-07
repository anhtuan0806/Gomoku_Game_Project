#include "SaveLoadSystem.h"
#include "../ApplicationTypes/GameConstants.h"
#include <filesystem>
#include <fstream>
#include <windows.h>

// Magic number de kiem tra file hop le (tranh load file rac)
static const uint32_t SAVE_MAGIC   = 0xCA05A1E2;
static const uint32_t SAVE_VERSION = 2; // Tang version bat cu khi nao struct thay doi

// ---------- Helpers: Write/Read cac kieu dong ----------

static void WriteWStr(std::ofstream& f, const std::wstring& s) {
    uint32_t len = (uint32_t)s.size();
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) f.write(reinterpret_cast<const char*>(s.data()), len * sizeof(wchar_t));
}

static bool ReadWStr(std::ifstream& f, std::wstring& s) {
    uint32_t len = 0;
    if (!f.read(reinterpret_cast<char*>(&len), sizeof(len))) return false;
    if (len > 4096) return false; // Gioi han bao ve chong file loi
    s.resize(len);
    if (len > 0 && !f.read(reinterpret_cast<char*>(&s[0]), len * sizeof(wchar_t))) return false;
    return true;
}

static void WriteStr(std::ofstream& f, const std::string& s) {
    uint32_t len = (uint32_t)s.size();
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) f.write(s.data(), len);
}

static bool ReadStr(std::ifstream& f, std::string& s) {
    uint32_t len = 0;
    if (!f.read(reinterpret_cast<char*>(&len), sizeof(len))) return false;
    if (len > 4096) return false;
    s.resize(len);
    if (len > 0 && !f.read(&s[0], len)) return false;
    return true;
}

template<typename T>
static void WriteVec(std::ofstream& f, const std::vector<T>& v) {
    uint32_t len = (uint32_t)v.size();
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) f.write(reinterpret_cast<const char*>(v.data()), len * sizeof(T));
}

template<typename T>
static bool ReadVec(std::ifstream& f, std::vector<T>& v) {
    uint32_t len = 0;
    if (!f.read(reinterpret_cast<char*>(&len), sizeof(len))) return false;
    if (len > 100000) return false; // Gioi han bao ve
    v.resize(len);
    if (len > 0 && !f.read(reinterpret_cast<char*>(v.data()), len * sizeof(T))) return false;
    return true;
}

// ---------- Serialize PlayerInfo2 ----------

static void WritePlayer(std::ofstream& f, const PlayerInfo2& p) {
    WriteWStr(f, p.name);
    WriteStr(f, p.avatarPath);
    f.write(&p.piece,                                               sizeof(p.piece));
    f.write(reinterpret_cast<const char*>(&p.totalWins),           sizeof(p.totalWins));
    f.write(reinterpret_cast<const char*>(&p.movesCount),          sizeof(p.movesCount));
    f.write(reinterpret_cast<const char*>(&p.maxTurnTime),         sizeof(p.maxTurnTime));
}

static bool ReadPlayer(std::ifstream& f, PlayerInfo2& p) {
    if (!ReadWStr(f, p.name))      return false;
    if (!ReadStr(f, p.avatarPath)) return false;
    if (!f.read(&p.piece, sizeof(p.piece)))                                          return false;
    if (!f.read(reinterpret_cast<char*>(&p.totalWins),    sizeof(p.totalWins)))     return false;
    if (!f.read(reinterpret_cast<char*>(&p.movesCount),   sizeof(p.movesCount)))    return false;
    if (!f.read(reinterpret_cast<char*>(&p.maxTurnTime),  sizeof(p.maxTurnTime)))   return false;
    return true;
}

// ---------- Public API ----------

bool SaveMatchData(const PlayState* state, const std::wstring& filename) {
    CreateDirectoryW(L"Asset", NULL);
    CreateDirectoryW(L"Asset/save", NULL);

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Header magic + version de kiem tra tinh toan ven
    file.write(reinterpret_cast<const char*>(&SAVE_MAGIC),   sizeof(SAVE_MAGIC));
    file.write(reinterpret_cast<const char*>(&SAVE_VERSION), sizeof(SAVE_VERSION));

    // Cac truong POD (Plain Old Data -- an toan raw copy)
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

    // Cac truong dong (co con tro heap -- PHAI serialize rieng)
    WritePlayer(file, state->p1);
    WritePlayer(file, state->p2);
    WriteVec(file, state->winningCells);
    WriteVec(file, state->matchHistory);
    WriteVec(file, state->redoStack);

    file.close();
    return true;
}

bool LoadMatchData(PlayState* state, const std::wstring& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Kiem tra magic + version -- neu sai thi tu choi load, tranh crash
    uint32_t magic = 0, version = 0;
    file.read(reinterpret_cast<char*>(&magic),   sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (magic != SAVE_MAGIC || version != SAVE_VERSION) {
        file.close();
        return false; // File cu (luu kieu raw dump) hoac file bi hong
    }

    // Doc POD fields
    if (!file.read(reinterpret_cast<char*>(&state->gameMode),        sizeof(state->gameMode)))        return false;
    if (!file.read(reinterpret_cast<char*>(&state->matchType),       sizeof(state->matchType)))       return false;
    if (!file.read(reinterpret_cast<char*>(&state->isP1Turn),        sizeof(state->isP1Turn)))        return false;
    if (!file.read(reinterpret_cast<char*>(&state->countdownTime),   sizeof(state->countdownTime)))   return false;
    if (!file.read(reinterpret_cast<char*>(&state->timeRemaining),   sizeof(state->timeRemaining)))   return false;
    if (!file.read(reinterpret_cast<char*>(&state->boardSize),       sizeof(state->boardSize)))       return false;
    if (!file.read(reinterpret_cast<char*>(state->board),            sizeof(state->board)))           return false;
    if (!file.read(reinterpret_cast<char*>(&state->cursorRow),       sizeof(state->cursorRow)))       return false;
    if (!file.read(reinterpret_cast<char*>(&state->cursorCol),       sizeof(state->cursorCol)))       return false;
    if (!file.read(reinterpret_cast<char*>(&state->lastMoveRow),     sizeof(state->lastMoveRow)))     return false;
    if (!file.read(reinterpret_cast<char*>(&state->lastMoveCol),     sizeof(state->lastMoveCol)))     return false;
    if (!file.read(reinterpret_cast<char*>(&state->status),          sizeof(state->status)))          return false;
    if (!file.read(reinterpret_cast<char*>(&state->winner),          sizeof(state->winner)))          return false;
    if (!file.read(reinterpret_cast<char*>(&state->difficulty),      sizeof(state->difficulty)))      return false;
    if (!file.read(reinterpret_cast<char*>(&state->targetScore),     sizeof(state->targetScore)))     return false;
    if (!file.read(reinterpret_cast<char*>(&state->matchDuration),   sizeof(state->matchDuration)))   return false;
    if (!file.read(reinterpret_cast<char*>(&state->p1TotalTimeLeft), sizeof(state->p1TotalTimeLeft))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->p2TotalTimeLeft), sizeof(state->p2TotalTimeLeft))) return false;
    if (!file.read(reinterpret_cast<char*>(&state->isMatchTimed),    sizeof(state->isMatchTimed)))    return false;

    // Doc cac truong dong
    if (!ReadPlayer(file, state->p1))        return false;
    if (!ReadPlayer(file, state->p2))        return false;
    if (!ReadVec(file, state->winningCells)) return false;
    if (!ReadVec(file, state->matchHistory)) return false;
    if (!ReadVec(file, state->redoStack))    return false;

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