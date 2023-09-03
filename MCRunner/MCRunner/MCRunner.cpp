// MCRunner.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include <fstream>
#include <Urlmon.h>
#include <direct.h>
#include <string.h>
#include <zip.h>
#include <sys/stat.h>
#include <json.h>
#include <thread>
#include <filesystem>
#include <fcntl.h>

#pragma comment(lib, "urlmon.lib")

using namespace std;

string mcversion;
string mcdir;

vector<string> strings;

void main_downloader();
string assets_downloader();
void objects_downloader(string assetId, string key);
void mc_downloader();
void libraries_downloader();
bool file_download(string f_url, string f_path);
vector<string> splitWord(string str, char del);
string join(vector<string> vct, char sep);
int mkpath(std::string s);

int main(int argc, char** argv)
{
    if (argc > 3) {
        cout << "You need add more arguments\n";
        return 1;
    }

    mcversion = argv[1];
    mcdir = argv[2];

    

    ifstream jarfile, jsonfile;
    string jar = mcdir + "\\versions\\" + mcversion + "\\" + mcversion + ".jar";
    string json = mcdir + "\\versions\\" + mcversion + "\\" + mcversion + ".json";

    jarfile.open(jar);
    jsonfile.open(json);

    /*
    if (jarfile || jsonfile) {
        cout << "You have MC in " << mcversion;
        return 1;
    }
    */

    main_downloader();

    return 0;
}

void main_downloader()
{
    string url = "https://raw.githubusercontent.com/InventivetalentDev/minecraft-assets/" + mcversion + "/version.json";
    string path = std::string(mcdir + "\\versions\\" + mcversion + "\\");

    struct stat buffer;

    if (stat(path.c_str(), &buffer) != 0) {
        cout << "[+] Se creo el folder\n";
        _mkdir(path.c_str());
    }

    string vrsf = path + "version.json";

    file_download(url, vrsf);

    std::ifstream versionfile(vrsf, std::ifstream::binary);
    Json::Value f;
    versionfile >> f;

    string vrsurl = f["url"].asString();

    versionfile.close();

    file_download(vrsurl, path + mcversion + ".json");

    DeleteFileA(vrsf.c_str());

    string assetId = assets_downloader();

    string opath = mcdir + "\\assets\\objects\\";

    struct stat obuffer;

    if (stat(opath.c_str(), &obuffer) != 0) {
        cout << "[+] Se creo el folder\n";
        _mkdir(opath.c_str());
    }

    vector<string> keys = { "icons", "lang", "sounds/ui", "sounds/title", "sounds/step", "sounds/tile", "sounds/block", "sounds/step", "sounds/random", "sounds/ui", "sounds/records", "sounds/portal", "sounds/note", "sounds/music", "sounds/mob", "sounds/minecart", "sounds/liquid", "sounds/item", "sounds/fire", "sounds/fireworks", "sounds/event", "sounds/entity", "sounds/enchant", "sounds/dig", "sounds/ambient", "sounds/damage", "textures", "resourcepacks", "sounds.json", "pack" };

    vector<thread> threads;

    cout << "[+] Descargando objetos con: " << keys.size() << " Hilos" << "\n";
    for (int i = 0; i < keys.size(); i++)
    {
        threads.push_back(thread(objects_downloader, string(assetId), string(keys[i])));
    }

    for (auto& th : threads)
    {
        th.join();
    }

    mc_downloader();

    libraries_downloader();

    objects_downloader("5", "asd");
}

string assets_downloader()
{
    string path = mcdir + "\\assets\\indexes\\";

    struct stat buffer;

    if (stat(path.c_str(), &buffer) != 0) {
        cout << "[+] Se creo el folder\n";
        _mkdir(path.c_str());
    }

    string mn = mcdir + "\\versions\\" + mcversion + "\\" + mcversion + ".json";

    std::ifstream mainjson(mn, std::ifstream::binary);
    Json::Value f;
    mainjson >> f;

    string asseturl = f["assetIndex"]["url"].asString();
    string id = f["assetIndex"]["id"].asString();

    mainjson.close();

    file_download(asseturl, path + id + ".json");

    return id;
}

void objects_downloader(string id, string key)
{
    cout << "[+] Start: " + key + " \n";
    string path = mcdir + "\\assets\\objects\\";
    string mn = mcdir + "\\assets\\indexes\\" + id + ".json";

    std::ifstream indexjson(mn, std::ifstream::binary);
    Json::Value f;
    Json::Reader reader;
    indexjson >> f;

    reader.parse(indexjson, f);

    int count = 0;
    for (auto& ob : f["objects"].getMemberNames())
    {
            string hatwo = f["objects"][ob]["hash"].asString().substr(0, 2);
            string hash = f["objects"][ob]["hash"].asString();

            string url = string("https://resources.download.minecraft.net/" + hatwo + "/" + hash);

            struct stat buffer;
            if (stat((path + hatwo + "\\").c_str(), &buffer) != 0) {
                cout << "[+] Se creo el folder\n";
                _mkdir((path + hatwo + "\\").c_str());
            }

            struct stat rebuffer;
            if (stat((path + hatwo + "\\" + hash).c_str(), &rebuffer) != 0)
            {
                thread(file_download, url, string(path + hatwo + "\\" + hash)).join();
                cout << count << "\n";
            }
            count = count + 1;
    }

    indexjson.close();
}

void mc_downloader()
{
    string vpath = mcdir + "\\versions\\" + mcversion;
    string mn = mcdir + "\\versions\\" + mcversion + "\\" + mcversion + ".json";

    std::ifstream mainjson(mn, std::ifstream::binary);
    Json::Value f;
    mainjson >> f;

    string jarurl = f["downloads"]["client"]["url"].asString();

    file_download(jarurl, vpath + "\\client.jar");

    mainjson.close();

    rename((vpath + "\\client.jar").c_str(), (vpath + "\\" + mcversion + ".jar").c_str());
}

void libraries_downloader()
{
    string mn = mcdir + "\\versions\\" + mcversion + "\\" + mcversion + ".json";

    std::ifstream mainjson(mn, std::ifstream::binary);
    Json::Value f;
    mainjson >> f;

    int counte = f["libraries"].size();

    cout << counte << "\n";

    for (auto& lib : f["libraries"])
    {
        string lpath = "";
        string url = "";

        if (lib["downloads"]["artifact"]["url"])
        {
            url = lib["downloads"]["artifact"]["url"].asString();
            lpath = mcdir + "\\libraries" + url.substr(31, 200);
        }

        else
        {
            if (lib["downloads"]["classifiers"]["natives-windows"].isNull())
            {
                url = lib["downloads"]["classifiers"]["natives-windows-64"]["url"].asString();
                lpath = mcdir + "\\libraries" + url.substr(31, 200);
            }

            else
            {
                url = lib["downloads"]["classifiers"]["natives-windows"]["url"].asString();
                lpath = mcdir + "\\libraries" + url.substr(31, 200);
            }
        }

        vector<string> pp = splitWord(lpath, '/');
        pp.pop_back();

        string dirpath = join(pp, '/');

        struct stat buffer;
        if (stat(dirpath.c_str(), &buffer) != 0) {
            cout << "[+] Se creo el folder " << dirpath << "\n";
            mkpath(dirpath);
        }

        struct stat sebuffer;
        if (stat(lpath.c_str(), &sebuffer) != 0) {
            file_download(url, lpath);
        }

    }

    mainjson.close();
}

bool file_download(string f_url, string f_path)
{
    wstring tempUrl = wstring(f_url.begin(), f_url.end());
    wstring tempPath = wstring(f_path.begin(), f_path.end());

    LPCWSTR wideStringUrl = tempUrl.c_str();
    LPCWSTR wideStringPath = tempPath.c_str();

    if (S_OK == URLDownloadToFile(NULL, wideStringUrl, wideStringPath, 0, NULL))
    {
        cout << "[+] Download Success\n";
        return true;
    }

    else
    {
        cout << "[-] Download Failed\n" << f_url << " into " << f_path << "\n";
        return false;
    }
}

vector<string> splitWord(string s, char del)
{

    vector<string> complete;

        stringstream ss(s);
        string word;
        while (!ss.eof()) {
            getline(ss, word, del);
            complete.push_back(word);
        }

    return complete;
}

string join(vector<string> vct, char sep)
{
    string full = "";

    for (auto& word : vct)
    {
        full += word + sep;
    }

    full.pop_back();

    return full;
}

int mkpath(std::string s)
{
    size_t pos = 0;
    std::string dir;
    int mdret;

    if (s[s.size() - 1] != '/') {
        s += '/';
    }

    while ((pos = s.find_first_of('/', pos)) != std::string::npos) {
        dir = s.substr(0, pos++);
        if (dir.size() == 0) continue;
        if ((mdret = _mkdir(dir.c_str())) && errno != EEXIST) {
            return mdret;
        }
    }
    return mdret;
}
