#include <dirent.h>
#include <errno.h>
#include <analyser.h>

std::string get_cwd()
{
    char* cwd = _getcwd(0, 0); // ms specific
    std::string working_directory(cwd);
    std::free(cwd);
    return working_directory;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Err: enter the name of the folder containing all pages during command invocation" << std::endl;
        return 1;
    }

    std::string page_folder(argv[1]);
    std::string pwd = get_cwd();

    // Looping over all images in pages directory
    DIR *d;
    struct dirent *dir;

    std::string page_path = pwd + "\\" + page_folder;
    d = opendir(page_path.c_str());
    if (d) {
        if (CreateDirectory("tmp", NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
            int i = 0;
            while ((dir = readdir(d)) != NULL) {
                if (i > 1) { // To ignore . and ..
                    analyse_image(page_path + "\\" + dir->d_name, dir->d_name);
                }
                i++;
            }
            closedir(d);
        }
    } else if (ENOENT == errno) {
        std::cout << "Err: Directory " << page_folder << " does not exist" << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}
