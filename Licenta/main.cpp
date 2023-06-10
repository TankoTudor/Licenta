#include <iostream>
#include <Windows.h>
#include <string>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <vector>
#include <omp.h>
#include <winreg.h>

using namespace std;

#ifndef KEY_WOW64_32KEY
#define KEY_WOW64_32KEY 0x0200
#endif

#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_64KEY 0x0100
#endif

void displayKeysAndvalues(HKEY hKey, const string& subKey, int level);
void setRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, BYTE* valueData, DWORD valueDataSize);
bool registryKeyExists(HKEY hKey, const string& subKey);
void createRegistryKey(HKEY hKey, const string& subKey);
void createRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, const string& valueData);
void deleteRegistryKey(HKEY hKey, const string& subKey);
void deleteRegistryValue(HKEY hKey, const string& subKey, const string& valueName);
void verifyL(char* argv[]);
void verifyCRTKey(char* argv[]);
void verifyCRTVal(char* argv[]);
void verifySetVal(char* argv[]);
void verifyDelKey(char* argv[]);
void verifyDelVal(char* argv[]);
void printIndent(int level);
string valueTypeToString(DWORD valueType);
DWORD stringToValueType(const string& valueTypeString);
string readFile(const string& fileName);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Nu s-a afisat nici un argument";
        return 0;
    }
    if (strcmp(argv[1], "-l") == 0)
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
    else if (strcmp(argv[1], "-crtkey") == 0)
    {
        if (argc == 4)
        {
            verifyCRTKey(argv);
        }
        else
        {
            cout << "Numar argumente invalid";
        }
    }
    else if (strcmp(argv[1], "-crtval") == 0)
    {
        if (argc == 7)
        {
            verifyCRTVal(argv);
        }
        else
        {
            cout << "Numar argumente invalid";
        }
    }
    else if (strcmp(argv[1], "-setval") == 0)
    {
        if (argc == 7)
        {
            verifySetVal(argv);
        }
        else
        {
            cout << "Numar argumente invalid";
        }

    }
    else if (strcmp(argv[1], "-delkey") == 0)
    {
        if (argc == 4)
        {
            verifyDelKey(argv);
        }
        else
        {
            cout << "Numar argumente invalid";
        }
    }
    else if (strcmp(argv[1], "-delval") == 0)
    {
        if (argc == 5)
        {
            verifyDelVal(argv);
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

void displayKeysAndvalues(HKEY hKey, const string& subKey, int level = 0)
{
    HKEY hSubKey;
    DWORD errorCode = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey);
    if (errorCode != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key! Error code: " << errorCode << endl;
        return;
    }

    DWORD subkeyCount;
    DWORD valueCount;
    DWORD maxValueNameLength;
    DWORD maxValueDataLength;

    if (RegQueryInfoKeyA(hSubKey, NULL, NULL, NULL, &subkeyCount, NULL, NULL, &valueCount, &maxValueNameLength, &maxValueDataLength, NULL, NULL) != ERROR_SUCCESS)
    {
        cerr << "Error retrieving key information!" << endl;
        RegCloseKey(hSubKey);
        return;
    }

    char* valueName = new char[maxValueNameLength + 1];
    BYTE* valueData = new BYTE[maxValueDataLength];
    DWORD valueNameLength;
    DWORD valueDataLength;
    DWORD valueType;

    cout << "---------------------------------------------------" << endl;
    cout << "Subkey: " << subKey << endl;
    cout << "Subkeys: " << subkeyCount << " Values: " << valueCount << endl;

    if (subkeyCount > 0)
    {
        cout << "---------------------------------------------------" << endl;
        cout << "Subkeys:" << endl;

        for (DWORD i = 0; i < subkeyCount; i++)
        {
            valueNameLength = maxValueNameLength + 1;

            if (i >= subkeyCount)
            {
                cerr << "Subkey index out of bounds!" << endl;
                continue;
            }

            if (RegEnumKeyExA(hSubKey, i, valueName, &valueNameLength, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            {
                cerr << "Error retrieving subkey with index " << i << "!" << endl;
                continue;
            }

            string subkeyName(valueName);
            string nextSubKey = subKey + "\\" + subkeyName;

            // Verifică dacă subcheia curentă există în registru
            HKEY hNextSubKey;
            DWORD subKeyErrorCode = RegOpenKeyExA(hKey, nextSubKey.c_str(), 0, KEY_READ, &hNextSubKey);
            if (subKeyErrorCode != ERROR_SUCCESS)
            {
                cerr << "Error opening subkey: " << nextSubKey << " Error code: " << subKeyErrorCode << endl;
                continue;
            }
            RegCloseKey(hNextSubKey);

            displayKeysAndvalues(hKey, nextSubKey, level + 1);
        }
    }
    else if (valueCount > 0)
    {
        cout << "Values:" << endl;

        for (DWORD i = 0; i < valueCount; i++)
        {
            valueNameLength = maxValueNameLength + 1;
            valueDataLength = maxValueDataLength;

            if (i >= valueCount)
            {
                cerr << "Value index out of bounds!" << endl;
                continue;
            }

            if (RegEnumValueA(hSubKey, i, valueName, &valueNameLength, NULL, &valueType, valueData, &valueDataLength) != ERROR_SUCCESS)
            {
                cerr << "Error retrieving value with index " << i << "!" << endl;
                continue;
            }

            string valueNameStr(valueName);
            string valueDataStr;

            string valueTypeStr = valueTypeToString(valueType);

            printIndent(level + 1);
            cout << valueNameStr;
            printIndent(level + 3);
            cout << valueTypeStr;

            // Display value data based on its type
            if (valueType == REG_DWORD)
            {
                printIndent(level + 5);
                cout << "(   " << valueDataLength << " B) ";
                for (DWORD j = 0; j < valueDataLength; j++)
                {
                    printf("%02X ", valueData[j]);
                }
                cout << endl;
            }
            else if (valueType == REG_QWORD)
            {
                printIndent(level + 5);
                cout << "(   " << valueDataLength << " B) ";
                for (DWORD j = 0; j < valueDataLength; j++)
                {
                    printf("%02X ", valueData[j]);
                }
                cout << endl;
            }
            else if (valueType == REG_SZ || valueType == REG_EXPAND_SZ)
            {
                printIndent(level + 5);
                cout << "(   " << valueDataLength << " B) ";
                for (DWORD j = 0; j < valueDataLength; j++)
                {
                    printf("%02X ", valueData[j]);
                }
                cout << endl;
            }
            else if (valueType == REG_BINARY)
            {
                printIndent(level + 5);
                cout << "(   " << valueDataLength << " B) ";
                for (DWORD j = 0; j < valueDataLength; j++)
                {
                    printf("%02X ", valueData[j]);
                }
                cout << endl;
            }
            else
            {
                printIndent(level + 5);
                cout << "(   " << valueDataLength << " B) ";
                for (DWORD j = 0; j < valueDataLength; j++)
                {
                    printf("%02X ", valueData[j]);
                }
                cout << endl;
            }
        }
    }

    delete[] valueName;
    delete[] valueData;
    RegCloseKey(hSubKey);
}

void createRegistryKey(HKEY hKey, const string& subKey)
{
    HKEY hKey32, hKey64;
    DWORD dwDisposition;

    LONG result32 = RegCreateKeyExA(hKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_32KEY, NULL, &hKey32, &dwDisposition);
    LONG result64 = RegCreateKeyExA(hKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey64, &dwDisposition);

    if (result32 == ERROR_SUCCESS && result64 == ERROR_SUCCESS)
    {
        cout << "Registry key created successfully!" << endl;
        RegCloseKey(hKey32);
        RegCloseKey(hKey64);
    }
    else
    {
        cout << "Failed to create registry key. Error code: " << GetLastError() << endl;
    }
}

void createRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, const string& valueData)
{
    HKEY hKey32, hKey64;
    DWORD dwDisposition;

    LONG result32 = RegCreateKeyExA(hKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_32KEY, NULL, &hKey32, &dwDisposition);
    LONG result64 = RegCreateKeyExA(hKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey64, &dwDisposition);

    if (result32 == ERROR_SUCCESS && result64 == ERROR_SUCCESS)
    {
        DWORD dataSize = 0;
        LPBYTE dataBuffer = nullptr;

        if (valueType == REG_SZ || valueType == REG_EXPAND_SZ || valueType == REG_MULTI_SZ)
        {
            dataSize = valueData.length() + 1;
            dataBuffer = reinterpret_cast<LPBYTE>(const_cast<char*>(valueData.c_str()));
        }
        else if (valueType == REG_DWORD)
        {
            DWORD dwValueData;
            try
            {
                dwValueData = stoul(valueData, nullptr, 0);
                dataSize = sizeof(DWORD);
                dataBuffer = reinterpret_cast<LPBYTE>(&dwValueData);
            }
            catch (const exception& e)
            {
                cout << "Failed to convert valueData to DWORD: " << e.what() << endl;
                RegCloseKey(hKey32);
                RegCloseKey(hKey64);
                return;
            }
        }
        else if (valueType == REG_BINARY)
        {
            dataSize = valueData.length() / 2;
            dataBuffer = new BYTE[dataSize];

            for (size_t i = 0; i < dataSize; ++i)
            {
                string byteString = valueData.substr(i * 2, 2);
                dataBuffer[i] = static_cast<BYTE>(stoi(byteString, nullptr, 16));
            }
        }
        else
        {
            cout << "Invalid value type." << endl;
            RegCloseKey(hKey32);
            RegCloseKey(hKey64);
            return;
        }

        LONG setResult32 = RegSetValueExA(hKey32, valueName.c_str(), 0, valueType, dataBuffer, dataSize);
        LONG setResult64 = RegSetValueExA(hKey64, valueName.c_str(), 0, valueType, dataBuffer, dataSize);

        if (setResult32 == ERROR_SUCCESS && setResult64 == ERROR_SUCCESS)
        {
            cout << "Registry value created successfully!" << endl;
        }
        else
        {
            cout << "Failed to set value in the registry key. Error code (32-bit): " << setResult32 << ", Error code (64-bit): " << setResult64 << endl;
        }

        if (valueType == REG_BINARY)
        {
            delete[] dataBuffer;
        }

        RegCloseKey(hKey32);
        RegCloseKey(hKey64);
    }
    else
    {
        cout << "Failed to create registry key. Error code (32-bit): " << result32 << ", Error code (64-bit): " << result64 << endl;
    }
}

void setRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType,const BYTE* valueData, DWORD valueDataSize)
{
    // Open the registry key for modification in the 32-bit view
    HKEY hSubKey32;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_32KEY, &hSubKey32) != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key for modification (32-bit view)!" << endl;
        return;
    }

    // Modify the value in the registry key (32-bit view)
    if (RegSetValueExA(hSubKey32, valueName.c_str(), 0, valueType, valueData, valueDataSize) != ERROR_SUCCESS)
    {
        cerr << "Error modifying registry value (32-bit view)!" << endl;
        RegCloseKey(hSubKey32);
        return;
    }

    cout << "Value modified successfully (32-bit view)!" << endl;

    // Close the registry key (32-bit view)
    RegCloseKey(hSubKey32);

    // Open the registry key for modification in the 64-bit view
    HKEY hSubKey64;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &hSubKey64) != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key for modification (64-bit view)!" << endl;
        return;
    }

    // Modify the value in the registry key (64-bit view)
    if (RegSetValueExA(hSubKey64, valueName.c_str(), 0, valueType, valueData, valueDataSize) != ERROR_SUCCESS)
    {
        cerr << "Error modifying registry value (64-bit view)!" << endl;
        RegCloseKey(hSubKey64);
        return;
    }

    cout << "Value modified successfully (64-bit view)!" << endl;

    // Close the registry key (64-bit view)
    RegCloseKey(hSubKey64);
}

void deleteRegistryKey(HKEY hKey, const string& subKey)
{
    // Open the registry key in the 32-bit view
    HKEY hSubKey32;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hSubKey32) != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key (32-bit view)! Error code: " << GetLastError() << endl;
        return;
    }

    // Delete the subkey in the 32-bit view
    LONG result32 = RegDeleteKeyA(hSubKey32, "");
    if (result32 != ERROR_SUCCESS)
    {
        cerr << "Error deleting registry key (32-bit view)! Error code: " << result32 << endl;
        RegCloseKey(hSubKey32);
        return;
    }

    cout << "Registry key deleted successfully (32-bit view)!" << endl;

    // Close the registry key in the 32-bit view
    RegCloseKey(hSubKey32);

    // Open the registry key in the 64-bit view
    HKEY hSubKey64;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hSubKey64) != ERROR_SUCCESS)
    {
        cerr << "Error opening registry key (64-bit view)! Error code: " << GetLastError() << endl;
        return;
    }

    // Delete the subkey in the 64-bit view
    LONG result64 = RegDeleteKeyA(hSubKey64, "");
    if (result64 != ERROR_SUCCESS)
    {
        cerr << "Error deleting registry key (64-bit view)! Error code: " << result64 << endl;
        RegCloseKey(hSubKey64);
        return;
    }

    cout << "Registry key deleted successfully (64-bit view)!" << endl;

    // Close the registry key in the 64-bit view
    RegCloseKey(hSubKey64);
}

void deleteRegistryValue(HKEY hKey, const string& subKey, const string& valueName)
{
    HKEY hSubKey32;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_32KEY, &hSubKey32) == ERROR_SUCCESS)
    {
        if (RegDeleteValueA(hSubKey32, valueName.c_str()) == ERROR_SUCCESS)
        {
            cout << "Registry value deleted successfully (32-bit view)!" << endl;
        }
        else
        {
            cerr << "Error deleting registry value (32-bit view)! Error code: " << GetLastError() << endl;
        }
        RegCloseKey(hSubKey32);
    }
    else
    {
        cerr << "Error opening registry key (32-bit view)! Error code: " << GetLastError() << endl;
    }

    HKEY hSubKey64;
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &hSubKey64) == ERROR_SUCCESS)
    {
        if (RegDeleteValueA(hSubKey64, valueName.c_str()) == ERROR_SUCCESS)
        {
            cout << "Registry value deleted successfully (64-bit view)!" << endl;
        }
        else
        {
            cerr << "Error deleting registry value (64-bit view)! Error code: " << GetLastError() << endl;
        }
        RegCloseKey(hSubKey64);
    }
    else
    {
        cerr << "Error opening registry key (64-bit view)! Error code: " << GetLastError() << endl;
    }
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
        displayKeysAndvalues(verifKey, argv[3]);
    }
    else {
        cout << "The registry does not exist";
    }
}

void verifyCRTKey(char* argv[]) 
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

    createRegistryKey(verifKey,argv[3]);
}

void verifyCRTVal(char* argv[])
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
    createRegistryValue(verifKey, argv[3], argv[4], stringToValueType(argv[5]), argv[6]);
}

void verifySetVal(char* argv[])
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

    if (registryKeyExists(verifKey, argv[3]) == true) {
        setRegistryValue(verifKey, argv[3], argv[4], stringToValueType(argv[5]), reinterpret_cast<const BYTE*>(&argv[6]), sizeof(argv[6]));
    }
    else {
        cout << "The registry does not exist";
    }
}

void verifyDelKey(char* argv[])
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

    if (registryKeyExists(verifKey, argv[3]) == true) {
        deleteRegistryKey(verifKey, argv[3]);
    }
    else {
        cout << "The registry does not exist";
    }
}

void verifyDelVal(char* argv[])
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

    if (registryKeyExists(verifKey, argv[3]) == true) {
        deleteRegistryValue(verifKey, argv[3],argv[4]);
    }
    else {
        cout << "The registry does not exist";
    }
}

void printIndent(int level)
{
    for (int i = 0; i < level; i++)
    {
        cout << "  ";
    }
}

string valueTypeToString(DWORD valueType)
{
    switch (valueType)
    {
    case REG_NONE:
        return "REG_NONE";
    case REG_SZ:
        return "REG_SZ";
    case REG_EXPAND_SZ:
        return "REG_EXPAND_SZ";
    case REG_BINARY:
        return "REG_BINARY";
    case REG_DWORD:
        return "REG_DWORD";
    case REG_DWORD_BIG_ENDIAN:
        return "REG_DWORD_BIG_ENDIAN";
    case REG_LINK:
        return "REG_LINK";
    case REG_MULTI_SZ:
        return "REG_MULTI_SZ";
    case REG_RESOURCE_LIST:
        return "REG_RESOURCE_LIST";
    case REG_FULL_RESOURCE_DESCRIPTOR:
        return "REG_FULL_RESOURCE_DESCRIPTOR";
    case REG_RESOURCE_REQUIREMENTS_LIST:
        return "REG_RESOURCE_REQUIREMENTS_LIST";
    case REG_QWORD:
        return "REG_QWORD";
    default:
        return "Unknown Type"; // Unknown type, return a default value
    }
}

DWORD stringToValueType(const string& valueTypeString)
{
    if (valueTypeString == "REG_NONE")
        return REG_NONE;
    else if (valueTypeString == "REG_SZ")
        return REG_SZ;
    else if (valueTypeString == "REG_EXPAND_SZ")
        return REG_EXPAND_SZ;
    else if (valueTypeString == "REG_BINARY")
        return REG_BINARY;
    else if (valueTypeString == "REG_DWORD")
        return REG_DWORD;
    else if (valueTypeString == "REG_DWORD_BIG_ENDIAN")
        return REG_DWORD_BIG_ENDIAN;
    else if (valueTypeString == "REG_LINK")
        return REG_LINK;
    else if (valueTypeString == "REG_MULTI_SZ")
        return REG_MULTI_SZ;
    else if (valueTypeString == "REG_RESOURCE_LIST")
        return REG_RESOURCE_LIST;
    else if (valueTypeString == "REG_FULL_RESOURCE_DESCRIPTOR")
        return REG_FULL_RESOURCE_DESCRIPTOR;
    else if (valueTypeString == "REG_RESOURCE_REQUIREMENTS_LIST")
        return REG_RESOURCE_REQUIREMENTS_LIST;
    else if (valueTypeString == "REG_QWORD")
        return REG_QWORD;
    else
        return 0; // Unknown type, return a default value
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
