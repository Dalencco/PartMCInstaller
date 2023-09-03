#include <iostream>
#include <Windows.h>
#include <fstream>
#include <Urlmon.h>
#include <direct.h>
#include <string>
#include <sys/stat.h>
#include <json/json.h>
#include <json/value.h>

using namespace std;

string mcversion;
string mcdir;

void main_downloader();
bool file_download(string u, string p);

int main(int argc, char** argv) {
	
	if (argc < 3) 
	{
		cout << "[-] You need more arguments.";	
		return 1;
	}
	
	mcversion = argv[1];
	mcdir = argv[2];
	
	ifstream jarfile, jsonfile;
    string jar = mcdir + "\\versions\\" + mcversion + "\\" + mcversion + ".jar";
    string json = mcdir + "\\versions\\" + mcversion + "\\" + mcversion + ".json";

    jarfile.open(jar.c_str());
    jsonfile.open(json.c_str());

    if (jarfile || jsonfile) {
        cout << "You have MC in " << mcversion;
        return 1;
    }
    
    main_downloader();
	
	return 0;
}

void main_downloader()
{
	string url = "https://raw.githubusercontent.com/InventivetalentDev/minecraft-assets/" + mcversion + "/version.json";
    string path = std::string(mcdir + "\\versions\\" + mcversion);

    struct stat buffer;

    if (stat(path.c_str(), &buffer) != 0) 
	{
        CreateDirectory(path.c_str(), NULL);
    }
    
    string vrspath = path + "\\version.json";
    
    file_download(url, vrspath);
    
    ifstream file(vrspath.c_str());
	Json::Reader reader;
	Json::Value completeJsonData;
	reader.parse(file, completeJsonData);

}

bool file_download(string f_url, string f_path)
{
	if (S_OK == URLDownloadToFile(NULL, f_url.c_str(), f_path.c_str(), 0, NULL))
	{
		cout << "[+] Success Download ";
		return true;
	}
	
	else 
	{
		cout << "[-] Failed Download";
		return false;
	}
}
