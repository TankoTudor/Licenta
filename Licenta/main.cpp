﻿#include <iostream>
#include <Windows.h>
#include <string>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <vector>
#include <omp.h>

using namespace std;

void displayRegistry(HKEY hKey, const string& subKey, int level);
bool setRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, BYTE* valueData, DWORD valueDataSize);
bool registryKeyExists(HKEY hKey, const string& subKey);
bool resetRegistryKey(HKEY hKey, const string& subKey, const string& valueName);
bool createRegistryKeyWithValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, const string& valueData);
string readFile(const string& fileName);
void verifyCH(char* argv[]);
void verifyL(char* argv[]);
void verifyCRT(char* argv[]);
void PrintIndent(int level);

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
    else if (strcmp(argv[1], "-help") == 0)
    {
        if (argc == 2)
        {
            string fileName = "Help.txt";
            string fileContent = readFile(fileName);

            if (!fileContent.empty())
            {
                cout << fileContent;
            }
            return 0;
        }
        else
        {
            cout << "File doesn't exist!";
        }
    }
    else
    {
        cout << "Invalid argument";
    }
    return 0;
}

void displayRegistry(HKEY hKey, const string& subKey, int level = 0)
{
    // Open the registry key
    HKEY hSubKey;

    DWORD errorCode = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey);
    if (errorCode != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key! Error code: " << errorCode << endl;
        return;
    }

    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key!" << endl;
        return;
    }

    DWORD subkeyCount;
    DWORD valueCount;
    DWORD maxValueNameLength;
    DWORD maxValueDataLength;

    // Get information about the registry key
    if (RegQueryInfoKeyA(hSubKey, NULL, NULL, NULL, &subkeyCount, NULL, NULL, &valueCount, &maxValueNameLength, &maxValueDataLength, NULL, NULL) != ERROR_SUCCESS)
    {
        cerr << "Error retrieving key information!" << endl;
        RegCloseKey(hSubKey);
        return;
    }

    // Allocate buffers for value names and value data
    char* valueName = new char[maxValueNameLength + 1];
    BYTE* valueData = new BYTE[maxValueDataLength];
    DWORD valueNameLength;
    DWORD valueDataLength;
    DWORD valueType;

    PrintIndent(level);
    cout << "Subkey: " << subKey << endl;
    PrintIndent(level);
    cout << "Subkeys: " << subkeyCount << " Values: " << valueCount << endl;

    // Iterate over each value in the registry key
    for (DWORD i = 0; i < valueCount; i++)
    {
        valueNameLength = maxValueNameLength + 1;
        valueDataLength = maxValueDataLength;

        // Get the name and data of the value
        if (RegEnumValueA(hSubKey, i, valueName, &valueNameLength, NULL, &valueType, valueData, &valueDataLength) != ERROR_SUCCESS)
        {
            cerr << "Error retrieving value with index " << i << "!" << endl;
            continue;
        }

        string valueNameStr(valueName);
        string valueDataStr;

        // Convert the value type to the corresponding string
        string valueTypeStr;
        switch (valueType)
        {
        case REG_DWORD:
            valueTypeStr = "REG_DWORD";
            break;
        case REG_QWORD:
            valueTypeStr = "REG_QWORD";
            break;
        case REG_SZ:
            valueTypeStr = "REG_SZ";
            break;
        case REG_EXPAND_SZ:
            valueTypeStr = "REG_EXPAND_SZ";
            break;
        case REG_BINARY:
            valueTypeStr = "REG_BINARY";
            break;
        default:
            valueTypeStr = "Unknown Type";
            break;
        }

        PrintIndent(level);
        cout << "---------------------------------------------------" << endl;
        PrintIndent(level);
        cout << "Value Name: " << valueNameStr << endl;
        PrintIndent(level);
        cout << "Value Type: " << valueTypeStr << endl;

        // Display values based on their type
        if (valueType == REG_DWORD)
        {
            DWORD value = *reinterpret_cast<DWORD*>(valueData);
            PrintIndent(level);
            cout << "Value: " << value << " (0x" << hex << value << ")" << endl;
            PrintIndent(level);
            cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            cout << endl;
        }
        else if (valueType == REG_QWORD)
        {
            DWORDLONG value = *reinterpret_cast<DWORDLONG*>(valueData);
            PrintIndent(level);
            cout << "Value: " << value << endl;
            PrintIndent(level);
            cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            cout << endl;
        }
        else if (valueType == REG_SZ || valueType == REG_EXPAND_SZ)
        {
            valueDataStr = string(reinterpret_cast<const char*>(valueData));
            PrintIndent(level);
            cout << "Value: " << valueDataStr << endl;
            PrintIndent(level);
            cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            cout << endl;
        }
        else if (valueType == REG_BINARY)
        {
            PrintIndent(level);
            cout << "Value: ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            cout << endl;
            PrintIndent(level);
            cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            cout << endl;
        }
        else
        {
            PrintIndent(level);
            cout << "Value: N/A" << endl;
            PrintIndent(level);
            cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            cout << endl;
        }

        if (valueType == REG_SZ || valueType == REG_EXPAND_SZ)
        {
            string nextSubKey = subKey + "\\" + valueDataStr;
            displayRegistry(hKey, nextSubKey, level + 1);
        }
    }

    delete[] valueName;
    delete[] valueData;
    RegCloseKey(hSubKey);

}

bool setRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType,  BYTE* valueData, DWORD valueDataSize)
{
    // Open the registry key for modification
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_SET_VALUE, &hSubKey) != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key for modification!" << endl;
        return false;
    }

    // Modify the value in the registry key
    if (RegSetValueExA(hSubKey, valueName.c_str(), 0, valueType, valueData, valueDataSize) != ERROR_SUCCESS)
    {
        cerr << "Error modifying registry value!" << endl;
        RegCloseKey(hSubKey);
        return false;
    }

    cout << "Value modified successfully!" << endl;

    // Close the registry key
    RegCloseKey(hSubKey);

    return true;
}

bool resetRegistryKey(HKEY hKey, const string& subKey, const string& valueName = "")
{
    // Open the registry key for resetting
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_SET_VALUE, &hSubKey) != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key for resetting!" << endl;
        return false;
    }

    // Reset a specific value
    if (!valueName.empty())
    {
        if (RegDeleteValueA(hSubKey, valueName.c_str()) != ERROR_SUCCESS)
        {
            cerr << "Error resetting registry value!" << endl;
            RegCloseKey(hSubKey);
            return false;
        }
    }
    // Reset all values within the subkey
    else
    {
        DWORD valueIndex = 0;
        char valueNameBuffer[256];
        DWORD valueNameSize = sizeof(valueNameBuffer);

        // Enumerate all values and delete them
        while (RegEnumValueA(hSubKey, valueIndex, valueNameBuffer, &valueNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            if (RegDeleteValueA(hSubKey, valueNameBuffer) != ERROR_SUCCESS)
            {
                cerr << "Error resetting registry value!" << endl;
                RegCloseKey(hSubKey);
                return false;
            }

            // Reset value index and buffer size for next iteration
            valueIndex++;
            valueNameSize = sizeof(valueNameBuffer);
        }
    }

    cout << "Registry value(s) reset successfully!" << endl;

    // Close the registry key
    RegCloseKey(hSubKey);

    return true;
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

string readFile(const string& fileName)
{
    ifstream inputFile(fileName);
    string fileContent;

    if (inputFile.is_open())
    {
        string line;
        while (getline(inputFile, line))
        {
            fileContent += line + "\n";
        }
        inputFile.close();
    }
    else
    {
        cout << "Failed to open the file: " << fileName << endl;
    }

    return fileContent;
}

void verifyCH(char* argv[])
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

void PrintIndent(int level)
{
    for (int i = 0; i < level; i++)
    {
        cout << "\t";
    }
}
