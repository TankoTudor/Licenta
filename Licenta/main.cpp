#include <iostream>
#include <Windows.h>
#include <string>
#include <atltime.h>

using namespace std;

void displayRegistry(HKEY hKey, const string& subKey);
void verifyCH(char* argv[]);
void verifyL(char* argv[]);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Nu s-a afisat nici un argument";
        return 0;
    }
    if (strcmp(argv[1], "-ch") == 0)
    {
        if (argc == 5)
        {
            verifyCH(argv);
        }
        else
        {
            cout << "Numar argumente invalid";
        }
    }
    else if (strcmp(argv[1], "-l") == 0)
    {
        if (argc == 4 || argc == 3)
        {
            verifyL(argv);
        }
        else
        {
            cout << "Numar argumente invalid";
        }
    }
    else if (strcmp(argv[1], "-r") == 0)
    {
        if (argc == 3)
        {
            verifyCH(argv);
        }
        else
        {
            cout << "Numar argumente invalid";
        }
    }
    else
    {
        cout << "Invalid argument";
    }
    return 0;
}

void displayRegistry(HKEY hKey, const string& subKey)
{
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
    {
        // Display subkeys
        cout << "Subkey:" << endl;
        DWORD subkeyCount, maxSubkeyLen;
        if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &subkeyCount, &maxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            for (DWORD i = 0; i < subkeyCount; i++)
            {
                char subkeyName[256];
                DWORD subkeyNameLen = sizeof(subkeyName);
                if (RegEnumKeyExA(hSubKey, i, subkeyName, &subkeyNameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                    cout << "-" << subkeyName << endl;
                    DWORD valueCount, maxValueNameLen, maxValueLen;
                    if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, NULL, NULL, NULL, &valueCount, &maxValueNameLen, &maxValueLen, NULL, NULL) == ERROR_SUCCESS)
                    {
                        for (DWORD i = 0; i < valueCount; i++)
                        {
                            char valueName[256];
                            DWORD valueNameLen = sizeof(valueName);
                            BYTE* valueData = new BYTE[maxValueLen];
                            DWORD valueDataLen = maxValueLen;
                            DWORD valueType;

                            if (RegEnumValueA(hSubKey, i, valueName, &valueNameLen, NULL, &valueType, valueData, &valueDataLen) == ERROR_SUCCESS)
                            {
                                std::cout << "  - Name: " << valueName << std::endl;
                                std::cout << "    Type: ";
                                switch (valueType)
                                {
                                case REG_SZ:
                                    std::cout << "REG_SZ" << std::endl;
                                    std::cout << "    Data: " << reinterpret_cast<char*>(valueData) << std::endl;
                                    break;
                                case REG_DWORD:
                                    std::cout << "REG_DWORD" << std::endl;
                                    std::cout << "    Data: " << *reinterpret_cast<DWORD*>(valueData) << std::endl;
                                    break;
                                default:
                                    std::cout << "Unknown" << std::endl;
                                    break;
                                }
                            }
                            delete[] valueData;
                        }
                    }
                }
            }
            RegCloseKey(hSubKey);
        }
    }
}

void setRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, const BYTE* valueData, DWORD valueSize)
{
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_WRITE, &hSubKey) == ERROR_SUCCESS)
    {
        if (RegSetValueExA(hSubKey, valueName.c_str(), 0, valueType, valueData, valueSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
        }
        else
        {
            cout << "Failed to set registry value" << endl;
        }
        RegCloseKey(hSubKey);
    }
    else
    {
        cout << "Failed to open registry key" << endl;
    }
}

bool resetRegistryKey(HKEY hKey, const string& subKey)
{
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
    {
        LONG result = RegRestoreKeyA(hSubKey, NULL, REG_REFRESH_HIVE);
        if (result == ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
            return true;
        }
        else
        {
            cout << "Failed to reset registry key: " << result << endl;
        }
        RegCloseKey(hSubKey);
    }
    else
    {
        cout << "Failed to open registry key" << endl;
    }
    return false;
}

void verifyCH(char* argv[])
{
    // verificam daca exista registrul ...
    HKEY verifKey, verifySubKey;

    if (strcmp(argv[2], "HKEY_CLASSES_ROOT") == 0)
    {

        verifKey = HKEY_CLASSES_ROOT;
    }
    else if (strcmp(argv[2], "HKEY_CURRENT_USER") == 0)
    {

        verifKey = HKEY_CURRENT_USER;
    }
    else if (strcmp(argv[2], "HKEY_LOCAL_MACHINE") == 0)
    {

        verifKey = HKEY_LOCAL_MACHINE;
    }
    else if (strcmp(argv[2], "HKEY_USERS") == 0)
    {

        verifKey = HKEY_USERS;
    }
    else if (strcmp(argv[2], "HKEY_CURRENT_CONFIG") == 0)
    {

        verifKey = HKEY_CURRENT_CONFIG;
    }

    LONG result = RegOpenKeyExA(verifKey, argv[3], 0, KEY_READ, &verifySubKey);

    if (result == ERROR_SUCCESS)
    {

        // setRegistryValue(verifKey,argv[3],);
    }
}

void verifyL(char* argv[])
{
    // verificam daca exista registrul ...

    HKEY verifKey;

    if (strcmp(argv[2], "HKEY_CLASSES_ROOT") == 0)
    {

        verifKey = HKEY_CLASSES_ROOT;
    }
    else if (strcmp(argv[2], "HKEY_CURRENT_USER") == 0)
    {

        verifKey = HKEY_CURRENT_USER;
    }
    else if (strcmp(argv[2], "HKEY_LOCAL_MACHINE") == 0)
    {

        verifKey = HKEY_LOCAL_MACHINE;
    }
    else if (strcmp(argv[2], "HKEY_USERS") == 0)
    {

        verifKey = HKEY_USERS;
    }
    else if (strcmp(argv[2], "HKEY_CURRENT_CONFIG") == 0)
    {

        verifKey = HKEY_CURRENT_CONFIG;
    }
    // displayRegistry(verifKey, argv[3]);
    DumpKey(verifKey);
}

void DumpKey(HKEY hKey)
{
    bool dumpKeys = true, bool dumpValues = true, bool recurse = true;
    DWORD nsubkeys, nvalues;
    DWORD maxValueSize;
    DWORD maxValueNameLen;
    FILETIME modified;
    if (ERROR_SUCCESS != ::RegQueryInfoKey(hKey, nullptr, nullptr, nullptr,
        &nsubkeys, nullptr, nullptr, &nvalues, &maxValueNameLen,
        &maxValueSize, nullptr, &modified))
        return;
    printf("Subkeys: %u Values: %u\n", nsubkeys, nvalues);
    if (dumpValues)
    {
        DWORD type;
        auto value = std::make_unique<BYTE[]>(maxValueSize);
        auto name = std::make_unique<WCHAR[]>(maxValueNameLen + 1);
        printf("values:\n");
        for (DWORD i = 0;; i++)
        {
            DWORD cname = maxValueNameLen + 1;
            DWORD size = maxValueSize;
            auto error = ::RegEnumValue(hKey, i, name.get(), &cname, nullptr,
                &type, value.get(), &size);
            if (error == ERROR_NO_MORE_ITEMS)
                break;
            auto display = GetValueAsString(value.get(), min(64, size), type);
            printf(" %-30ws %-12ws (%5u B) %ws\n", name.get(),
                (PCWSTR)display.first, size, (PCWSTR)display.second);
        }
    }
}

std::pair<CString, CString>
GetValueAsString(const BYTE* data, DWORD size, DWORD type)
{
    CString value, stype;
    switch (type)
    {
    case REG_DWORD:
        stype = L"REG_DWORD";
        value.Format(L"%u (0x%X)", *(DWORD*)data, *(DWORD*)data);
        break;
    case REG_QWORD:
        stype = L"REG_QWORD";
        value.Format(L"%llu (0x%llX)", *(DWORD64*)data, *(DWORD64*)data);
        break;
    case REG_SZ:
        stype = L"REG_SZ";
        value = (PCWSTR)data;
        break;
    case REG_EXPAND_SZ:
        stype = L"REG_EXPAND_SZ";
        value = (PCWSTR)data;
        break;
    case REG_BINARY:
        stype = L"REG_BINARY";
        for (DWORD i = 0; i < size; i++)
            value.Format(L"%s%02X ", value, data[i]);
        break;
    default:
        stype.Format(L"%u", type);
        value = L"(Unsupported)";
        break;
    }
    return { stype, value };
}
