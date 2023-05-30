#include <iostream>
#include <Windows.h>
#include <string>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <vector>
#include <omp.h>

using namespace std;

void displayRegistry(HKEY hKey, const string& subKey, int level);
void setRegistryValue(HKEY hKey, const string& subKey, const string& valueName, DWORD valueType, BYTE* valueData, DWORD valueSize);
bool registryKeyExists(HKEY hKey, const string& subKey);
bool resetRegistryKey(HKEY hKey, const string& subKey);
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
    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
    {
        std::cerr << "Error opening registry key!" << std::endl;
        return;
    }

    DWORD subkeyCount;
    DWORD valueCount;
    DWORD maxValueNameLength;
    DWORD maxValueDataLength;

    // Get information about the registry key
    if (RegQueryInfoKeyA(hSubKey, NULL, NULL, NULL, &subkeyCount, NULL, NULL, &valueCount, &maxValueNameLength, &maxValueDataLength, NULL, NULL) != ERROR_SUCCESS)
    {
        std::cerr << "Error retrieving key information!" << std::endl;
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
    std::cout << "Subkey: " << subKey << std::endl;
    PrintIndent(level);
    std::cout << "Subkeys: " << subkeyCount << " Values: " << valueCount << std::endl;

    // Iterate over each value in the registry key
    for (DWORD i = 0; i < valueCount; i++)
    {
        valueNameLength = maxValueNameLength + 1;
        valueDataLength = maxValueDataLength;

        // Get the name and data of the value
        if (RegEnumValueA(hSubKey, i, valueName, &valueNameLength, NULL, &valueType, valueData, &valueDataLength) != ERROR_SUCCESS)
        {
            std::cerr << "Error retrieving value with index " << i << "!" << std::endl;
            continue;
        }

        std::string valueNameStr(valueName);
        std::string valueDataStr;

        // Convert the value type to the corresponding string
        std::string valueTypeStr;
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
        std::cout << "---------------------------------------------------" << std::endl;
        PrintIndent(level);
        std::cout << "Value Name: " << valueNameStr << std::endl;
        PrintIndent(level);
        std::cout << "Value Type: " << valueTypeStr << std::endl;

        // Display values based on their type
        if (valueType == REG_DWORD)
        {
            DWORD value = *reinterpret_cast<DWORD*>(valueData);
            PrintIndent(level);
            std::cout << "Value: " << value << " (0x" << std::hex << value << ")" << std::endl;
            PrintIndent(level);
            std::cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            std::cout << std::endl;
        }
        else if (valueType == REG_QWORD)
        {
            DWORDLONG value = *reinterpret_cast<DWORDLONG*>(valueData);
            PrintIndent(level);
            std::cout << "Value: " << value << std::endl;
            PrintIndent(level);
            std::cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            std::cout << std::endl;
        }
        else if (valueType == REG_SZ || valueType == REG_EXPAND_SZ)
        {
            valueDataStr = std::string(reinterpret_cast<const char*>(valueData));
            PrintIndent(level);
            std::cout << "Value: " << valueDataStr << std::endl;
            PrintIndent(level);
            std::cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            std::cout << std::endl;
        }
        else if (valueType == REG_BINARY)
        {
            PrintIndent(level);
            std::cout << "Value: ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            std::cout << std::endl;
            PrintIndent(level);
            std::cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            std::cout << std::endl;
        }
        else
        {
            PrintIndent(level);
            std::cout << "Value: N/A" << std::endl;
            PrintIndent(level);
            std::cout << "Space Occupied: " << valueDataLength << " B) ";
            for (DWORD j = 0; j < valueDataLength; j++)
            {
                printf("%02X ", valueData[j]);
            }
            std::cout << std::endl;
        }

        if (valueType == REG_SZ || valueType == REG_EXPAND_SZ)
        {
            std::string nextSubKey = subKey + "\\" + valueDataStr;
            displayRegistry(hKey, nextSubKey, level + 1);
        }
    }

    delete[] valueName;
    delete[] valueData;
    RegCloseKey(hSubKey);

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
        LONG result = RegRestoreKeyA(hSubKey, nullptr, REG_REFRESH_HIVE);
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
        std::cout << "\t";
    }
}
