/*
	src/ApiSender.cpp
	Copyright (C) <2026 realjhen123

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published
	by the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <map>
#include <curl/curl.h>
#include "json/json.h"
#include <stdlib.h>
#include <fstream>
#define SUCCEED 1
namespace jsonfile {
	static void writeFileFromString(const std::string filename, const std::string body) {
		std::ofstream ofile(filename);
		ofile << body;
		ofile.close();
	}
	static Json::Value readJsonFile(const std::string filename) {
		std::ifstream ifile;
		ifile.open(filename);
		Json::CharReaderBuilder ReaderBuilder;
		ReaderBuilder["emitUTF8"] = true;
		Json::Value root;
		std::string strerr;
		bool ok = Json::parseFromStream(ReaderBuilder, ifile, &root, &strerr);
		return root;
	}
	static void writeJsonFile(const std::string& filename, const Json::Value& root) {
		Json::StreamWriterBuilder writebuild;
		writebuild["emitUTF8"] = true;
		std::string document = Json::writeString(writebuild, root);
		writeFileFromString(filename, document);
	}
	static Json::Value readJsonFromString(const std::string& mystr) {
		Json::CharReaderBuilder ReaderBuilder;
		ReaderBuilder["emitUTF8"] = true;
		std::unique_ptr<Json::CharReader> charread(ReaderBuilder.newCharReader());
		Json::Value root;
		std::string strerr;
		bool isok = charread->parse(mystr.c_str(), mystr.c_str() + mystr.size(), &root, &strerr);
		return root;
	}
	static std::string jsontoString(const Json::Value& json_val, std::string tap_ = "\t") {
		Json::StreamWriterBuilder builder;
		builder["emitUTF8"] = true;
		builder["indentation"] = tap_;
		// 设置错误处理（避免默认断言崩溃）
		builder["commentStyle"] = "None";
		builder["allowComments"] = false;
		std::string json_str;
		try {
			json_str = Json::writeString(builder, json_val);
		}
		catch (const Json::Exception& e) {
			return "";
		}

		return json_str;
	}
	static Json::Value parse(std::string str_) {
		return jsonfile::readJsonFromString(str_);
	}
	static std::string parse(Json::Value json_) {
		return jsonfile::jsontoString(json_);
	}
	//	static void writeJsonFile(const string filename)
};
class CurlClient {
public:   
	CURL* curl_;
	struct curl_slist* headers = nullptr;

    CurlClient() {
        curl_global_init(CURL_GLOBAL_ALL);
        this->curl_ = curl_easy_init();
    }
    ~CurlClient()
    {
        curl_easy_cleanup(this->curl_);
    }
	int addHeader(std::string header) {
		this->headers = curl_slist_append(this->headers, header.c_str());
		return SUCCEED;
	}
	bool Post(const std::string& url, const std::string& data, std::string& response) {
		if (!curl_) {
			return false;
		}
		curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, this->headers);
		curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_, CURLOPT_POST, 1L);
		curl_easy_setopt(curl_, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlClient::WriteCallback);
		curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
		CURLcode res = curl_easy_perform(curl_);
		return (res == CURLE_OK);
	}
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
		size_t realsize = size * nmemb;
		std::string* str = static_cast<std::string*>(userp);
		str->append(static_cast<char*>(contents), realsize);
		return realsize;
	}
	void OutputReqHeaders() const {
		struct curl_slist* p = this->headers;
		while (p != NULL) {
			std::cout << p->data;
			p = p->next;
		}
	}
};
static std::string getReadableTime() {
	std::time_t now = std::time(nullptr);
	std::tm localTime;
#ifdef _WIN32
	localtime_s(&localTime, &now);
#else
	localtime_r(&now, &localTime);
#endif
	char buf[64];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &localTime);
	return std::string(buf);
}
int main()
{
	system("mkdir ApiSender");
	std::string command_1, command_2, command_3, command_4;
	std::string workfile = ".\\ApiSender\\ApiSender.txt" , workname;
	std::string working;
	Json::Value basicconfig = jsonfile::readJsonFile(".\\ApiSender\\config.json");
	std::cout << ">";
	std::cin >> command_1;
	Json::Value config;
	CurlClient cc;
	while (command_1 != "q") {
		if (command_1 == "init" || command_1 == "i") {
			std::cin >> command_2 >> command_3;
			for (char c : command_2) {
				if ((c < 65 || c>122) && c != 46 )std::abort();
				else if (c > 90 && c < 97 && c != 95)std::abort();
			}
			config["."]["url"] = "";
			config["."]["request"]["header"] = Json::nullValue;
			config["."]["request"]["body"] = "";
			config["."]["cookies"] = Json::nullValue;
			config["."]["response"]["type"] = "commandline";
			basicconfig["ApiSender"]["introduction"] = "";
			working = ".";
			if (command_2 == ".") {
				workfile = ".\\ApiSender\\ApiSender.txt";
				workname = "ApiSender";
				basicconfig["ApiSender"]["introduction"] = command_3;
			}
			else {
				workfile = ".\\ApiSender\\" + command_2 + ".txt";
				workname = command_2;
				basicconfig[command_2]["introduction"] = command_3;
			}
			jsonfile::writeJsonFile(workfile, config);
			jsonfile::writeJsonFile(".\\ApiSender\\config.json", basicconfig);
		}
		else if (command_1 == "set") {
#ifdef _WIN32
			system((std::string("notepad ")+ workfile).c_str());
#elif __linux__
			system((std::string("vim ") + workfile).c_str());
#endif
			config = jsonfile::readJsonFile(workfile);
		}
		else if (command_1 == "switch" || command_1 == "sw") {
			config = jsonfile::readJsonFile(workfile);
			std::cin >> command_2;
			working = command_2;
			if (!config[working].isObject()) {
				config[command_2]["url"] = "";
				config[command_2]["request"]["header"] = Json::nullValue;
				config[command_2]["request"]["body"] = "";
				config[command_2]["cookies"] = Json::nullValue;
				config[command_2]["response"]["type"] = "commandline";
				basicconfig[workname][command_2] = "";
			}
			jsonfile::writeJsonFile(workfile,config);
			jsonfile::writeJsonFile(".\\ApiSender\\config.json", basicconfig);
		}
		else if (command_1 == "load" || command_1 == "l" || command_1 == "spaceload" || command_1 == "loadspace") {
			std::cin >> command_2;
			for (char c : command_2) {
				if ((c < 65 || c>122) && c != 46 )std::abort();
				else if (c > 90 && c < 97 && c != 95)std::abort();
			}
			if (command_2 == ".") {
				workfile = ".\\ApiSender\\ApiSender.txt";
				workname = "ApiSender";
			}
			else {
				workfile = ".\\ApiSender\\" + command_2 + ".txt";
				workname = command_2;
			}
			config = jsonfile::readJsonFile(workfile);
		}
		else if (command_1 == "reload" || command_1 == "rl") {
			config = jsonfile::readJsonFile(workfile);
		}
		else if (command_1 == "postgo" || command_1 == "send") {
			if (working != "") {
				cc.headers = nullptr;
				std::vector<std::string> k = config[working]["request"]["header"].getMemberNames();
				for (const auto& it : k) {
					cc.addHeader(it + ": " + config[working]["request"]["header"][it].asString());
				}
				std::string req = config[working]["request"]["body"].isObject() ? jsonfile::parse(config[working]["request"]["body"]) : config[working]["request"]["body"].asString();
				std::string res;
				std::cout << "Url:" << config[working]["url"].asString() << std::endl
					<< "Request Body:" << req << std::endl;
				std::cout << "Request Header: ";
				cc.OutputReqHeaders();
				std::cout << std::endl;
				cc.Post(
					config[working]["url"].asString(),
					req,
					res
				);
				if (config[working]["response"]["type"] == "commandline") {
					std::cout << res << std::endl;
				}
				else {
					std::ofstream out(config[working]["response"]["type"].asString(), std::ios::app);
					out << "\n" << getReadableTime() << "\n";
					out << res << "\n";
					out.close();
				}
			}
			else {
				std::cout << "Havno't Working" << std::endl;
			}
		}
		else if (command_1 == "debug") {
			basicconfig = jsonfile::readJsonFile(".\\ApiSender\\config.json");
			std::cout << "workspace:" << workname << std::endl;
			std::cout << "introduction:" << basicconfig[workname].get("introduction", "") << std::endl;
			std::cout << "working:" << working << std::endl;
			std::cout << "introduction:" << basicconfig[workname].get(working, "") << std::endl;
			std::cout << "workspace:=====" << std::endl;
			std::cout << jsonfile::jsontoString(basicconfig, "  ") << std::endl;
			std::cout << "working:======="<<std::endl;
			std::cout << jsonfile::jsontoString(config,"  ") << std::endl;
		}
		else if (command_1 == "space") {
			basicconfig = jsonfile::readJsonFile(".\\ApiSender\\config.json");
			std::vector<std::string> b = basicconfig.getMemberNames();
			for (const auto& it : b)std::cout << it << " ";
			std::cout << std::endl;
		}
		else if (command_1 == "work") {
			std::vector<std::string> c = config.getMemberNames();
			for (const auto& it : c)std::cout << it << " ";
			std::cout << std::endl;
		}
		else if (command_1 == "this") {
			std::cout << "workspace:" << workname << std::endl;
			std::cout << "introduction:" << basicconfig[workname].get("introduction", "") << std::endl;
			std::cout << "working:" << working << std::endl;
			std::cout << "introduction:" << basicconfig[workname].get(working, "") << std::endl;
			std::cout << "working:=======" << std::endl;
			std::cout << jsonfile::jsontoString(config[working], "  ") << std::endl;
		}
		command_1 = "";
		command_2 = "";
		command_3 = "";
		command_4 = "";
		std::cout << ">";
		std::cin >> command_1;
	}
	return 0;
}
