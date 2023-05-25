#include <iostream>
#include <Windows.h>
#include <string>
#include <iomanip>
#include <algorithm>

using namespace std;

void displayRegistry(HKEY hKey, const string& subKey);
bool registryKeyExists(HKEY hKey, const string& subKey);
void verifyCH(char* argv[]);
void verifyL(char* argv[]);
void verifyCRT(char* argv[]);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Nu s-a afisat nici un argument";
        return 0;
    }
    if (strcmp(argv[1], "-ch") == 0)
    {
        if (argc == 7)
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
    else if (strcmp(argv[1], "-crt") == 0)
    {
        if (argc == 7)
        {
            verifyCRT(argv);
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
        DWORD subkeyCount, maxSubkeyLen;
        if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &subkeyCount, &maxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            cout << "Subkeys: " << subkeyCount << endl;
            cout << "Values:" << endl;
            for (DWORD i = 0; i < subkeyCount; i++)
            {
                char subkeyName[256];
                DWORD subkeyNameLen = sizeof(subkeyName);
                if (RegEnumKeyExA(hSubKey, i, subkeyName, &subkeyNameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                    cout << "--------" << endl;
                    cout << "Subkey: " << subkeyName << endl;
                    DWORD valueCount, maxValueNameLen, maxValueLen;
                    if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, NULL, NULL, NULL, &valueCount, &maxValueNameLen, &maxValueLen, NULL, NULL) == ERROR_SUCCESS)
                    {
                        cout << "Subkeys: " << subkeyCount << " Values: " << valueCount << endl;
                        cout << "values:" << endl;
                        for (DWORD j = 0; j < valueCount; j++) // Changed loop variable to 'j'
                        {
                            char valueName[256];
                            DWORD valueNameLen = sizeof(valueName);
                            BYTE* valueData = new BYTE[maxValueLen];
                            DWORD valueDataLen = maxValueLen;
                            DWORD valueType;

                            if (RegEnumValueA(hSubKey, j, valueName, &valueNameLen, NULL, &valueType, valueData, &valueDataLen) == ERROR_SUCCESS)
                            {
                                cout << valueName << " ";
                                switch (valueType)
                                {
                                case REG_SZ:
                                    cout << "REG_SZ";
                                    break;
                                case REG_DWORD:
                                    cout << "REG_DWORD";
                                    break;
                                case REG_BINARY:
                                    cout << "REG_BINARY";
                                    break;
                                default:
                                    cout << "(unknown data type)";
                                    break;
                                }
                                cout << " (" << valueDataLen << " B) ";
                                for (DWORD k = 0; k < valueDataLen; k++)
                                {
                                    cout << hex << setw(2) << setfill('0') << static_cast<int>(valueData[k]) << " ";
                                }
                                cout << endl;
                            }
                            delete[] valueData;
                        }
                    }
                }
            }
        }
        RegCloseKey(hSubKey);
    }
}

void setRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType,  BYTE* valueData, DWORD valueSize)
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

bool registryKeyExists(HKEY hKey, const string& subKey)
{
    HKEY hSubKey;
    LONG result = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey);
    if (result == ERROR_SUCCESS)
    {
        RegCloseKey(hSubKey);
        return true; 
    }
    else if (result == ERROR_FILE_NOT_FOUND)
    {
        return false; 
    }
    else
    {
        cout << "Error opening registry key: " << result << endl;
        return false;
    }
}

bool createRegistryKeyWithValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, const string& valueData)
{
    HKEY hKeyNew;
    DWORD dwDisposition;
    LONG result = RegCreateKeyExA(hKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyNew, &dwDisposition);
    if (result == ERROR_SUCCESS)
    {
        DWORD dwValueData;
        
        if (valueType == REG_DWORD && !valueData.empty())
        {
            try
            {
                dwValueData = stoul(valueData, nullptr, 0);
            }
            catch (const exception& e)
            {
                cout << "Failed to convert valueData to DWORD: " << e.what() << endl;
                RegCloseKey(hKeyNew);
                return false;
            }
        }
        else
        {
            cout << "Invalid value type or data." << endl;
            RegCloseKey(hKeyNew);
            return false;
        }

        result = RegSetValueExA(hKeyNew, valueName.c_str(), 0, valueType, reinterpret_cast<const BYTE*>(&dwValueData), sizeof(dwValueData));
        if (result == ERROR_SUCCESS)
        {
            RegCloseKey(hKeyNew);
            cout << "Registry key and value created successfully!" << endl;
            return true;
        }
        else
        {
            RegCloseKey(hKeyNew);
            cout << "Failed to set value in the registry key. Error code: " << result << endl;
            return false;
        }
    }
    else
    {
        cout << "Failed to create registry key. Error code: " << result << endl;
        return false;
    }
}

void verifyCH(char* argv[])
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

    if (registryKeyExists(verifKey, argv[3]) == true) {
        setRegistryValue(verifKey, argv[3], argv[4], (DWORD)argv[5], (BYTE*)argv[6], (DWORD)argv[7]);
    }
    else {
        cout << "The registry does not exist";
    }
}

void verifyL(char* argv[])
{
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

    if (registryKeyExists(verifKey, argv[3]) == true) {
        displayRegistry(verifKey, argv[3]);
    }
    else {
        cout << "The registry does not exist";
    }
}

void verifyCRT(char* argv[]) 
{
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
    else {
        cout << "The registry does not exist";
    }

    DWORD valueType;
    if (strcmp(argv[5], "REG_SZ") == 0)
    {
        valueType = REG_SZ;
    }else if (strcmp(argv[5], "REG_DWORD") == 0)
    {
        valueType = REG_DWORD;
    }
    else if (strcmp(argv[5], "REG_BINARY") == 0)
    {
        valueType = REG_BINARY;
    }
    if (createRegistryKeyWithValue(verifKey, argv[3], argv[4], valueType, argv[6]))
    {
        cout << "Succes!";
    }
}
