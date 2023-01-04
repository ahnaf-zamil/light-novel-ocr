#include <main.h>

std::string get_cwd()
{
    char* cwd = _getcwd(0, 0); // ms specific
    std::string working_directory(cwd);
    std::free(cwd);
    return working_directory;
}

std::vector<std::string> splitString(const std::string& str)
{
    std::vector<std::string> tokens;

    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, '\n'))
    {
        tokens.push_back(token);
    }

    return tokens;
}

void print_banner()
{
    std::string tess_version = exec("tesseract --version");
    std::vector<std::string> tokens = splitString(tess_version);

    std::time_t t = std::time(nullptr);
    std::tm* const pTInfo = std::localtime(&t);

    const char* banner =
        R"(
   _      _       _     _     _   _                _    ____   _____ _____
  | |    (_)     | |   | |   | \ | |              | |  / __ \ / ____|  __ \
  | |     _  __ _| |__ | |_  |  \| | _____   _____| | | |  | | |    | |__) |
  | |    | |/ _` | '_ \| __| | . ` |/ _ \ \ / / _ \ | | |  | | |    |  _  /
  | |____| | (_| | | | | |_  | |\  | (_) \ V /  __/ | | |__| | |____| | \ \
  |______|_|\__, |_| |_|\__| |_| \_|\___/ \_/ \___|_|  \____/ \_____|_|  \_\
            __/ |
           |___/                                                           )";

    std::cout << banner << std::endl << std::endl;
    std::cout << "Copyright DevGuyAhnaf " << 1900 + pTInfo->tm_year << " | All rights reserved"
              << std::endl;
    std::cout << "OpenCV " << CV_VERSION << std::endl;
    std::cout << tokens[0] << std::endl << std::endl;
}

void start_analyser(std::string pwd, std::string page_folder)
{
    // Looping over all images in pages directory
    DIR* d;
    struct dirent* dir;

    std::string page_path = pwd + "\\" + page_folder;
    d = opendir(page_path.c_str());
    if (d)
    {
        if (CreateDirectory("tmp", NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            auto full_start = std::chrono::high_resolution_clock::now();
            int i = 0;
            while ((dir = readdir(d)) != NULL)
            {
                if (i > 1)
                { // To ignore . and ..
                    auto start = std::chrono::high_resolution_clock::now();
                    analyse_image(page_path + "\\" + dir->d_name, dir->d_name);
                    auto stop = std::chrono::high_resolution_clock::now();
                    std::cout << "Analysed " << dir->d_name << ". Time taken: "
                              << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)
                                     .count()
                              << " ms" << std::endl;
                }
                i++;
            }
            closedir(d);

            auto full_stop = std::chrono::high_resolution_clock::now();
            std::cout << std::endl
                      << "Finished analysing " << i << " images in "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(
                             full_stop - full_start)
                             .count()
                      << " ms" << std::endl;
        }
    }
    else if (ENOENT == errno)
    {
        std::cout << "Err: Directory " << page_folder << " does not exist" << std::endl;
        exit(EXIT_FAILURE);
    }
}


void start_ocr(std::string pwd)
{
    DIR* d;
    struct dirent* dir;

    std::string tmp_path = pwd + "\\" + "tmp";
    std::string out_dir = pwd + "\\" + "ocr_out";
    std::vector<std::thread> threads;

    if (CreateDirectory(out_dir.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
    {
        d = opendir(tmp_path.c_str());
        if (d)
        {
            int i = 0;
            while ((dir = readdir(d)) != NULL)
            {
                if (i > 1)
                {
                    std::string img_folder_path = tmp_path + "\\" + dir->d_name;
                    DWORD file_attr = GetFileAttributesA(img_folder_path.c_str());
                    if (file_attr & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // If entry is a directory, do OCR for all images in that dir (loop
                        // again)
                        // Storing thread in vector to join later
                        threads.emplace_back(std::thread(do_ocr_for_folder, out_dir + "\\" + dir->d_name + ".txt", img_folder_path));
                    }
                }
                i++;
            }

            closedir(d);
        }
        else if (ENOENT == errno)
        {
            std::cout << "Err: Directory "
                      << "tmp"
                      << " does not exist in pwd" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Joining threads to keep program running till tasks are done
        for (std::thread& th : threads)
        {
            th.join();
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Err: Not enough arguments provided during invocation" << std::endl;
        return 1;
    }

    print_banner();

    std::string pwd = get_cwd();

    if (strcmp(argv[1], "--ocr") == 0)
    { // If OCR flag is passed
        std::cout << "Starting OCR" << std::endl;
        // Runs the ocr
        start_ocr(pwd);
    }
    else
    {
        // If argv[1] is a folder
        std::string page_folder(argv[1]);

        std::cout << "Starting image analyser" << std::endl << std::endl;
        // Runs the analyser
        start_analyser(pwd, page_folder);
    }

    system("pause");
    return 0;
}
