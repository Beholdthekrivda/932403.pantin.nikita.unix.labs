#include <experimental/filesystem>
#include <filesystem>
#include <iostream>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>

using namespace std;

unsigned char *sha1_calc(const char *data) {
  unsigned char hash[SHA_DIGEST_LENGTH];

  SHA1((unsigned char *)data, strlen(data), hash);

  return hash;
}

int main(int argc, char *argv[]) {
  if (argc < 1) {
    cout << "Недостаточно аргументов" << endl;
    cout << "Использование: ./project <path_to_dir>";
    return 1;
  }

  unordered_map<unsigned char hash, char file_path>;

  std::filesystem::path DIR = argv[1];

  try {
    auto options = std::filesystem::directory_options::skip_permission_denied;

    for (auto it = std::filesystem::recursive_directory_iterator(DIR, options);
         it != std::filesystem::recursive_directory_iterator(); ++it) {

      string filename = it->path().filename().string();

      if (!filename.empty() && filename[0] == '.') {
        if (std::filesystem::is_directory(it->status())) {
          it.disable_recursion_pending();
        }
        continue;
      }

      if (std::filesystem::is_regular_file(it->status())) {
        cout << "Файл: " << it->path().c_str() << endl;
        unsigned char *sha1_hash = sha1_calc(it->path().c_str());

      } else if (std::filesystem::is_directory(it->status())) {
        cout << "Директория: " << it->path().c_str() << endl;
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    cout << "Ошибка: " << e.what() << endl;
  }

  return 0;
}
