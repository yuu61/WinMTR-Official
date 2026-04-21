//*****************************************************************************
// FILE:            LRUStore.cpp
//*****************************************************************************

#include "Global.h"
#include "LRUStore.h"

#include <string_view>

namespace {

constexpr size_t kHostBufferLen = 255;

// Duplicated with Settings.cpp's OpenWinMTRSubKey; candidate for a shared
// RegistryHelpers translation unit.
LONG OpenLRUKey(REGSAM access, HKEY& outKey)
{
	HKEY  hSoftware = nullptr;
	DWORD disp      = 0;
	LONG  r         = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software", 0, nullptr,
	                                  REG_OPTION_NON_VOLATILE, access, nullptr,
	                                  &hSoftware, &disp);
	if (r != ERROR_SUCCESS) {
		return r;
	}

	HKEY hWinMTR = nullptr;
	r = RegCreateKeyExW(hSoftware, L"WinMTR", 0, nullptr,
	                    REG_OPTION_NON_VOLATILE, access, nullptr,
	                    &hWinMTR, &disp);
	RegCloseKey(hSoftware);
	if (r != ERROR_SUCCESS) {
		return r;
	}

	r = RegCreateKeyExW(hWinMTR, L"LRU", 0, nullptr,
	                    REG_OPTION_NON_VOLATILE, access, nullptr,
	                    &outKey, &disp);
	RegCloseKey(hWinMTR);
	return r;
}

void WriteDwordValue(HKEY hKey, LPCWSTR name, DWORD value)
{
	RegSetValueExW(hKey, name, 0, REG_DWORD,
	               reinterpret_cast<const BYTE*>(&value), sizeof(DWORD));
}

} // namespace


//*****************************************************************************
// LRUStore::Load
//*****************************************************************************
bool LRUStore::Load(std::vector<CString>& outHosts)
{
	HKEY hLRU = nullptr;
	if (OpenLRUKey(KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS) {
		return false;
	}

	DWORD tmp = 0;
	DWORD sz  = sizeof(DWORD);
	if (RegQueryValueExW(hLRU, L"NrLRU", nullptr, nullptr,
	                     reinterpret_cast<BYTE*>(&tmp), &sz) != ERROR_SUCCESS) {
		WriteDwordValue(hLRU, L"NrLRU", static_cast<DWORD>(nr_));
	} else {
		wchar_t hostBuf[kHostBufferLen]{};
		nr_ = static_cast<int>(tmp);
		outHosts.reserve(static_cast<size_t>(max_));
		for (int i = 0; i < max_; ++i) {
			const auto keyName = std::format(L"Host{}", i + 1);
			DWORD hostSize = sizeof(hostBuf);
			if (RegQueryValueExW(hLRU, keyName.c_str(), nullptr, nullptr,
			                     reinterpret_cast<BYTE*>(hostBuf), &hostSize) == ERROR_SUCCESS) {
				const DWORD chars = hostSize / sizeof(wchar_t);
				if (chars == 0 || chars > _countof(hostBuf)) {
					continue;
				}
				hostBuf[_countof(hostBuf) - 1] = L'\0';
				if (hostBuf[0] == L'\0') {
					continue;
				}
				outHosts.emplace_back(hostBuf);
			}
		}
	}
	RegCloseKey(hLRU);
	return true;
}


//*****************************************************************************
// LRUStore::Append
//*****************************************************************************
void LRUStore::Append(LPCWSTR host)
{
	HKEY hLRU = nullptr;
	if (OpenLRUKey(KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS) {
		return;
	}

	if (nr_ >= max_) {
		nr_ = 0;
	}
	nr_++;
	const auto              keyName  = std::format(L"Host{}", nr_);
	const std::wstring_view hostView{host};
	RegSetValueExW(hLRU, keyName.c_str(), 0, REG_SZ,
	               reinterpret_cast<const BYTE*>(host),
	               static_cast<DWORD>((hostView.size() + 1) * sizeof(wchar_t)));
	WriteDwordValue(hLRU, L"NrLRU", static_cast<DWORD>(nr_));
	RegCloseKey(hLRU);
}


//*****************************************************************************
// LRUStore::Trim
//
// Mirrors legacy off-by-one in OnOptions: deletes Host{max_}..Host{nr_}.
//*****************************************************************************
void LRUStore::Trim()
{
	HKEY hLRU = nullptr;
	if (OpenLRUKey(KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS) {
		return;
	}

	for (int i = max_; i <= nr_; ++i) {
		RegDeleteValueW(hLRU, std::format(L"Host{}", i).c_str());
	}
	nr_ = max_;
	WriteDwordValue(hLRU, L"NrLRU", static_cast<DWORD>(nr_));
	RegCloseKey(hLRU);
}


//*****************************************************************************
// LRUStore::Clear
//
// Mirrors legacy ClearHistory: starts at Host0 (which never exists) and
// continues through Host{nr_}.
//*****************************************************************************
void LRUStore::Clear()
{
	HKEY hLRU = nullptr;
	if (OpenLRUKey(KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS) {
		return;
	}

	for (int i = 0; i <= nr_; ++i) {
		RegDeleteValueW(hLRU, std::format(L"Host{}", i).c_str());
	}
	nr_ = 0;
	WriteDwordValue(hLRU, L"NrLRU", 0);
	RegCloseKey(hLRU);
}
