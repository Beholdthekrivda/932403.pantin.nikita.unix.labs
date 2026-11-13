#include <filesystem>
#include <iostream>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <cstdio>

using namespace std;
namespace fs = std::filesystem;

string to_hex(const string &raw)
{
  string out(raw.size() * 2, '\0');
  for (size_t i = 0; i < raw.size(); i++)
  {
    sprintf(&out[i * 2], "%02x", (unsigned char)raw[i]);
  }

  return out;
}

string sha1_calc(const string &path) 
{
  unsigned char hash[SHA_DIGEST_LENGTH];

  FILE *fp = fopen(path.c_str(), "rb");

  if (!fp)
  {
    cout << "Ошибка открытия файла " << path << endl;
    return string();
  }

  SHA_CTX ctx;
  SHA1_Init(&ctx);

  unsigned char buf[64 * 1024];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
  {
    SHA1_Update(&ctx, buf, n);
  }

  fclose(fp);
  SHA1_Final(hash, &ctx);

  return string((char *)hash, SHA_DIGEST_LENGTH);
}

int main(int argc, char *argv[]) 
{
  if (argc < 2) 
  {
    cout << "Недостаточно аргументов" << endl;
    cout << "Использование: ./project <path_to_dir>";
    return 1;
  }

  unordered_map<string, string> list;

  fs::path DIR = argv[1];

  try 
  {
    auto options = fs::directory_options::skip_permission_denied;

    for (auto it = fs::recursive_directory_iterator(DIR, options);
         it != fs::recursive_directory_iterator(); ++it) 
    {

      string filename = it->path().filename().string();

      if (fs::is_regular_file(it->status())) 
      {
        // cout << "Файл: " << it->path().string() << endl;

        string sha1_hash = sha1_calc(it->path().string());
        string hex = to_hex(sha1_hash);

        if (list.count(hex) > 0)
        {
          error_code err;

          fs::remove(it->path().c_str(), err);

          if (err)
          {
            cout << "Ошибка: " << err.message() << " " << err.value() << endl;
            return 1;
          }
          
          fs::create_hard_link(list[hex].c_str(), it->path().c_str());
        }
        else
        {
          list[hex] = it->path().string();
        }
      } 
      else if (fs::is_directory(it->status())) 
      {
        // cout << "Директория: " << it->path().string() << endl;
      }
    }

    for (const auto &[key, value] : list)
    {
      cout << key << ": " << value << endl;
    }
  } 
  catch (const fs::filesystem_error &e) 
  {
    cout << "Ошибка: " << e.what() << endl;
  }

  return 0;
}
