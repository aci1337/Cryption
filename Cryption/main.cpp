#include "importants.h"

int main()
{
    if (!CryptionStart()) {
        return 1;
    }

    if (protection()) {
        return 1;
    }

    system("cls");
    std::string folder;
    std::string password;
    std::string hashedPassword;


    if (!FileExists("C:/ProgramData/files.crypt"))
    {
        std::cout << "Enter a folder to encrypt: ";
        std::cin >> folder;
        std::ifstream fileFolder("C:/ProgramData/files.crypt");
        fileFolder >> folder;
        fileFolder.close();


        std::cout << "Set your password: ";
        std::cin >> password;
        hashedPassword = GetHashedPassword(password);

        iterateDirectoryAndXOR(folder, password);
    }
    else
    {
        std::cout << "Enter your password: ";
        std::cin >> password;
        hashedPassword = GetHashedPassword(password);
    }

    std::string enteredPassword;
    if (!FileExists("C:/ProgramData/files.crypt"))
    {
        HideConsole();

        std::cout << "Enter your password to decrypt: ";
        std::cin >> enteredPassword;
        std::string hashedEnteredPassword = GetHashedPassword(enteredPassword);

        if (hashedEnteredPassword == hashedPassword)
        {
            if (protection()) {
                return 1;
            }

            unencryptFiles(folder, password);
            ShowSuccessMessageBox();
        }
        else
        {
            std::cout << "Incorrect password.";
            return 1;
        }
    }

    ShowSuccessMessageBox();

    std::cin.ignore();
}


