#include <iostream>
#include <Windows.h>
#include <string>

using namespace std;

void displayRegistry(HKEY hKey, const string &subKey);
void verifyCH(char *argv[]);
void verifyL(char *argv[]);

int main(int argc, char *argv[])
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

void displayRegistry(HKEY hKey, const string &subKey)
{
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
    {
        // Display subkeys
        DWORD subkeyCount, maxSubkeyLen;
        if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &subkeyCount, &maxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            for (DWORD i = 0; i < subkeyCount; i++)
            {
                char subkeyName[256];
                DWORD subkeyNameLen = sizeof(subkeyName);
                if (RegEnumKeyExA(hSubKey, i, subkeyName, &subkeyNameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                    cout << "Subkey: " << subkeyName << endl;
                }
            }
        }

        // Display values
        DWORD valueCount, maxValueNameLen, maxValueLen;
        if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, NULL, NULL, NULL, &valueCount, &maxValueNameLen, &maxValueLen, NULL, NULL) == ERROR_SUCCESS)
        {
            for (DWORD i = 0; i < valueCount; i++)
            {
                char valueName[256];
                DWORD valueNameLen = sizeof(valueName);
                BYTE *valueData = new BYTE[maxValueLen]; // merge !!!!!
                DWORD valueDataLen = sizeof(valueData);
                DWORD valueType;

                if (RegEnumValueA(hSubKey, i, valueName, &valueNameLen, NULL, &valueType, valueData, &valueDataLen) == ERROR_SUCCESS)
                {
                    cout << "Value: " << valueName << " = ";
                    switch (valueType)
                    {
                    case REG_SZ:
                        cout << string((char *)valueData);
                        break;
                    case REG_DWORD:
                        cout << *((DWORD *)valueData);
                        break;
                    default:
                        cout << "(unknown data type)";
                        break;
                    }
                    cout << endl;
                }
            }
        }

        RegCloseKey(hSubKey);
    }
}

bool setRegistryValue(HKEY hKey, const string &subKey, const string &valueName, DWORD valueType, const BYTE *valueData, DWORD valueSize)
{
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_WRITE, &hSubKey) == ERROR_SUCCESS)
    {
        if (RegSetValueExA(hSubKey, valueName.c_str(), 0, valueType, valueData, valueSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
            return true;
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
    return false;
}

bool resetRegistryKey(HKEY hKey, const string &subKey)
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

void verifyCH(char *argv[])
{
    // verificam daca exista registrul ...
}

void verifyL(char *argv[])
{
    // verificam daca exista registrul ...

    HKEY test;

    if (strcmp(argv[2], "HKEY_LOCAL_MACHINE") == 0)
    {
        test = HKEY_LOCAL_MACHINE;
    }
    displayRegistry(test, argv[3]);
}
