#ifndef _SAVELOADSYSTEM_H_
#define _SAVELOADSYSTEM_H_
#include "../ApplicationTypes/PlayState.h"
#include <string>

/** @file SaveLoadSystem.h
 *  @brief API lưu/tải ván chơi, quản lý slot lưu và metadata.
 */

/** @brief Lưu toàn bộ trạng thái `PlayState` vào file nhị phân.
 *  @param state Con trỏ tới trạng thái cần lưu.
 *  @param filename Đường dẫn file (wstring).
 *  @return true nếu lưu thành công.
 */
bool SaveMatchData(const PlayState *state, const std::wstring &filename);

/** @brief Tải trạng thái từ file vào `state`.
 *  @return true nếu tải thành công.
 */
bool LoadMatchData(PlayState *state, const std::wstring &filename);

/** @brief Trả về đường dẫn file lưu theo slot. */
std::wstring GetSavePath(int slot);

/** @brief Kiểm tra xem slot lưu có tồn tại file không. */
bool CheckSaveExists(int slot);

/** @brief Thông tin metadata tóm tắt của 1 slot lưu (dùng cho UI). */
struct SaveMetadata
{
    std::wstring name;
    std::wstring timestamp;
    int mode; /**< 0: Caro, 1: TTT */
    int type; /**< 0: PvP, 1: PvE */
    int p1Wins;
    int p2Wins;
    int difficulty;
    int boardSize;
    bool exists;
    uint32_t version;
};

/** @brief Xóa save tại `slot`. */
bool DeleteSave(int slot);

/** @brief Đổi tên save tại `slot` (không thay đổi timestamp).
 *  @return true nếu thành công.
 */
bool RenameSave(int slot, const std::wstring &newName);

/** @brief Trả về tên hiển thị của save tại `slot`. */
std::wstring GetSaveDisplayName(int slot);

/** @brief Lấy metadata tóm tắt cho `slot`. */
SaveMetadata GetSaveMetadata(int slot);

#endif // _SAVELOADSYSTEM_H_