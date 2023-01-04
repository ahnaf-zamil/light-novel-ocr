#include <main.h>

std::map<std::string, std::vector<std::string>> get_files(std::string img_folder_path)
{
    // Gets all files in the respective folders of each image

    std::map<std::string, std::vector<std::string>> file_map = {};

    DIR* d;
    struct dirent* img_dir;
    d = opendir(img_folder_path.c_str());
    if (d)
    {
        int i = 0;
        while ((img_dir = readdir(d)) != NULL)
        {
            if (i > 1)
            {
                std::string section_dir_fpath = img_folder_path + "\\" + img_dir->d_name;

                DWORD folder_attr = GetFileAttributesA(section_dir_fpath.c_str());
                if (folder_attr & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // If entry is a directory, do OCR for all images in that img_dir
                    // (loop again)
                    // Looping over all files in each section folder. At this point idk
                    // how to name the vars
                    DIR* s_d;
                    struct dirent* section_dir;
                    s_d = opendir(section_dir_fpath.c_str());
                    int j = 0;
                    while ((section_dir = readdir(s_d)) != NULL)
                    {
                        if (j > 1)
                        {
                            std::string section_idx_fpath
                                = section_dir_fpath + "\\" + section_dir->d_name;

                            if (file_map.find(img_dir->d_name) == file_map.end())
                            {
                                // entry not found
                                file_map[img_dir->d_name] = {};
                            }
                            file_map[img_dir->d_name].push_back(section_idx_fpath);
                        }
                        j++;
                    }
                    closedir(s_d);
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

    return file_map;
}

std::string exec(std::string cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

void do_ocr_for_folder(std::string out_file, std::string img_folder_path)
{
    std::map<std::string, std::vector<wchar_t*>> text_map = {};

    for (const auto& section : get_files(img_folder_path))
    {
        std::vector<std::string> section_files = section.second;
        for (unsigned int i = 0; i < section_files.size(); i++)
        {
            std::string s_file = section_files[i];

            // Running tesseract for OCR
            std::string cmd = "tesseract \"" + s_file + "\" stdout -l jpn_vert --psm 5";
            std::string output = exec(cmd);

            // Converting UTF-8 to UTF-16 cuz that's what windows supports
            int size = MultiByteToWideChar(CP_UTF8, 0, output.c_str(), -1, NULL, 0);
            wchar_t* utf16 = (wchar_t*)malloc((size + 1) * sizeof(wchar_t));
            MultiByteToWideChar(CP_UTF8, 0, output.c_str(), -1, utf16, size);
            text_map[section.first].push_back(utf16);
        }
    }

    // Writing to file
    FILE* fs = fopen(out_file.c_str(), "w+,ccs=UTF-8");
    for (const auto& elem : text_map)
    {
        for (unsigned int i = 0; i < elem.second.size(); i++)
        {
            wchar_t* line = elem.second[i];
            fwrite(line, wcslen(line) * sizeof(wchar_t), 1, fs);
        }
    }
    fclose(fs);

    std::cout << "Finished OCR for " + out_file << std::endl;
}
