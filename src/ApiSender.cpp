/*
	src/ApiSender.cpp
	Copyright (C) 2026 realjhen123

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

#include "OptionalFeatures.h"
#include "Apisenderh.h"

#ifndef APISENDER_PATH
	#define APISENDER_PATH ".ApiSender"
#endif
#define SUCCEED 1

bool ui = true;
bool ez = false;
bool history = false;
bool nevertimeout = true;
bool showheader = true;
namespace jsonfile {
	std::string sep = "\t";
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
	static std::string jsontoString(const Json::Value& json_val, std::string tap_ = jsonfile::sep) {
		Json::StreamWriterBuilder builder;
		builder["emitUTF8"] = true;
		builder["indentation"] = tap_;
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

	static std::string parse(Json::Value json_,std::string tap_ = jsonfile::sep) {
		return jsonfile::jsontoString(json_, tap_);
	}

	//	static void writeJsonFile(const string filename)
};
class CurlClient {
public:  
	bool stream = false;
	CURL* curl_;
	struct curl_slist* headers = nullptr;

    CurlClient() {
        
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
		if (!ez && showheader) curl_easy_setopt(curl_, CURLOPT_HEADER, 1L);
		else curl_easy_setopt(curl_, CURLOPT_HEADER, 0L);
		curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
		if (stream)curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION,CurlClient::PrintStreamCallback );
		else curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlClient::WriteCallback);
		curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
		if (nevertimeout) {
			curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 0L);
			curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 0L);
			curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_LIMIT, 0L);
			curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_TIME, 0L);
			curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
		}
		CURLcode res = curl_easy_perform(curl_);
		return (res == CURLE_OK);
	}
	bool Get(const std::string& url, std::string& response) {
		if (!curl_) {
			return false;
		}
		curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_, CURLOPT_POST, 0L);
		curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &WriteCallback);
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
	static size_t PrintStreamCallback(void* contents, size_t size, size_t nmemb, void* userp) {
		size_t realsize = size * nmemb;
		std::string* str = static_cast<std::string*>(userp);
		if (ez) { 
			if (jsonfile::parse((std::string)(char*)contents) != Json::nullValue) {
				Json::Value json = jsonfile::parse((std::string)(char*)contents);
				if (json.get("response", "") != "") { 
					std::cout << json["response"].asString(); 
					if (history) {
						std::ofstream out(APISENDER_PATH "/history.txt", std::ios::app);
						out << json["response"].asString();
					}
				}
				else {
					std::string p = jsonfile::parse(jsonfile::parse((std::string)(char*)contents)["response"], "");
					if (p != "")std::cout << p << std::endl;
				}
			}
		}
		else std::cout << (char*)contents;
		str->append(static_cast<char*>(contents), realsize);
		return realsize;
	}
	void OutputReqHeaders() const {
		std::cout << "\n";
		struct curl_slist* p = this->headers;
		while (p != NULL) {
			std::cout << p->data << "\n";
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
namespace apisender {
#ifdef APISENDER_STRESS_TESTING
	enum LogType {
		NONE,
		TRACE,
		DEBUG,
		INFO,
		WARN,
		FATAL
	};
	class Logger {
	private:
		std::mutex mtx;
		std::queue<std::string> list;
		std::unordered_map<apisender::LogType, std::string> LT_;
	public:
		Logger() {
			LT_[apisender::LogType::DEBUG] = "[DEBUG]";
			LT_[apisender::LogType::TRACE] = "[TRACE]";
			LT_[apisender::LogType::INFO] = "[INFO]";
			LT_[apisender::LogType::WARN] = "[WARN]";
			LT_[apisender::LogType::FATAL] = "[FATAL]";
		}
		bool status = true;
		LogType logtype = apisender::INFO;
		void log(apisender::LogType logtype_, std::string str) {
			std::lock_guard<std::mutex> guard(this->mtx);
			if (logtype_ <= logtype)this->list.push(LT_[logtype_] + " " + (std::string)str);
		}
		void printfunction() {
			while (status) {
				{
					std::lock_guard<std::mutex> guard(this->mtx);
					while (!this->list.empty()) {
						std::cout  << list.front() ;
						std::cout.flush();
						this->list.pop();
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
	};
	class stress_testing {
	public:
		bool status = true;
		Json::Value config;
		int workers_number = 10;
		apisender::Logger log;
		std::mutex mtx;
		int count = 0;
		stress_testing() {
			this->config = jsonfile::readJsonFile(APISENDER_PATH "/stress.json");
		}
		void save() {
			jsonfile::writeJsonFile(APISENDER_PATH "/stress.json", this->config);
		}
		void RunF(std::string working) {
			std::string request;
			if (this->config[working]["request"].isObject())request = jsonfile::parse(this->config[working]["request"]);
			else this->config[working]["request"].asString();
			std::string url = this->config[working]["url"].asString();
			CURL* curl_ = curl_easy_init();
			std::string response;
			curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl_, CURLOPT_POST, 1L);
			curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, request.c_str());
			curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlClient::WriteCallback);
			curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
			while (status) {
				curl_easy_perform(curl_);
				{ 
					std::lock_guard<std::mutex> guard(this->mtx);
					this->count++;
				}
				response = "";
			}
		}
		void runPerSecond() {
			
			while (status) {
				{
					std::lock_guard<std::mutex> guard(this->mtx);
					this->log.log(apisender::INFO, getReadableTime() + "\t" + "\tcount:" + std::to_string(this->count) + "\n");
					this->count = 0;
				}
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
	};
#endif
	bool stringcompare(std::string target_str,std::string compare_with) {
		int i = 0;
		const char a = (char)54;
		for (char c : target_str) {
			if ((compare_with[i] == c + 32) || (compare_with[i] == c - 32) || (compare_with[i] == c))i++;
			else return false;
		}
		return true;
	}
	namespace error {
		enum ErrorCode {
			ASE0001,

		};
		std::unordered_map<apisender::error::ErrorCode, std::string> ErrorInfo;
		void initErrorInfo() {
			ErrorInfo[ASE0001] = "No Allow Char";
		}
		void DropError(apisender::error::ErrorCode ecode, std::string rawcontent) {
			std::cout << "\n\n\n===============\n";
			std::cout << "We have some problem.\n";
			std::cout << " ErrorLocation Debug:" << rawcontent << std::endl;
			std::cout << "Error Code:" << ecode;
			std::cout << " " << apisender::error::ErrorInfo[ecode] << "\n";
			std::abort();
		}
	}
	
	std::string runawork(Json::Value config_,std::string working,std::string workspace,bool silence) {
		/*
		*config_
		* working
		*	request
		*		body
		*		header
		*	response
		*		stream
		*	url
		*	method
		*/
		CurlClient cc;
		std::string target_url = config_[working]["url"].asString();
		bool ez_b = ez;
		if (silence)showheader = false;
		if (config_[working]["response"]["stream"] == true) {
			cc.stream = true;
		}
		std::string res;
		for (const auto& it : config_[working]["request"]["header"].getMemberNames()) cc.addHeader(it + ": " + config_[working]["request"]["header"][it].asString());
		if (target_url.find("$base") != std::string::npos) {
			std::string t_url = target_url.substr(target_url.find("base") + target_url.size() == 5 ? 5 : 6);
			target_url = config_["base_url"].asString() + t_url;
		}
		if (config_[working]["method"].asString() == "get" || 
			config_[working]["method"].asString() == "Get" || 
			config_[working]["method"].asString() == "GET") {
			std::string req = "?";
			if (config_[working]["request"]["body"].isObject()) {
				for (const auto& it : config_[working]["request"]["body"].getMemberNames()) {
					req += it + "=" + config_[working]["request"]["body"][it].asString() + "&";
				}
				req = req.substr(0, req.size() - 1);
			}
			if (!silence)std::cout << "Url:" << config_[working]["url"].asString() + req << std::endl
				<< "Request Header: ";
			cc.OutputReqHeaders();
			if (!silence)std::cout << std::endl;
			cc.Get(
				target_url + req,
				res
			);
		}
		//else if (config_[working]["method"].asString() == "post" ||
		//	config_[working]["method"].asString() == "Post" ||
		//	config_[working]["method"].asString() == "POST") {
		else if (stringcompare(config_[working]["method"].asString(),"post")) {
			std::string req;
			if (config_[working]["request"]["body"].isObject()) {
				Json::Value req_json = config_[working]["request"]["body"];
				std::vector<std::string> h = req_json.getMemberNames();
				
				for (const auto& h_ : h) {
					std::string h_s = req_json[h_].asString();
					if (h_s.find('$') != std::string::npos) {
						if (h_s == "$(INPUT)") {
							std::cout << "Input \"" << h_ << "\" >";
							std::string in;
							std::getline(std::cin >> std::ws, in);
							req_json[h_] = in;
						}
						// $(apisender`/login)
						else if (h_s.find('`') != std::string::npos) {
							std::string target_working, target_space , target_spacename;
							target_spacename = h_s.substr(2, h_s.find('`') - 2);
							if (target_spacename == "~" || target_spacename == "") {
								target_spacename = workspace;
							}
							else if (target_spacename == ".") {
								target_spacename = "ApiSender";
							}
							target_space = APISENDER_PATH;
							target_space +=  "/" + target_spacename + ".txt";
							for (char c : h_s.substr(2, h_s.find('`') - 2)) {
								if ((c < 65 || c>122) && c != 46 && c != 126)apisender::error::DropError(apisender::error::ASE0001, "h_s");
								else if (c > 90 && c < 97 && c != 95 && c != 126)apisender::error::DropError(apisender::error::ASE0001, "h_s");
							}
							
							target_working = h_s.substr(h_s.find('`') +1 , h_s.size() - h_s.find('`'));
							target_working = target_working.substr(0, target_working.size() - 1);
							if (!ez)std::cout << target_spacename << " " << target_working << "\n";
							req_json[h_] = apisender::runawork(jsonfile::readJsonFile(target_space), target_working, target_spacename, true);
							if (!ez)std::cout << req_json[h_] << std::endl;
						}
					}
				}
				req = jsonfile::parse(req_json);
			}
			else {
				req = config_[working]["request"]["body"].asString();
			}
			if (!ez && !silence) {
				std::cout << "Url:" << config_[working]["url"].asString() << std::endl
					<< "Request Body:" << req << std::endl;
				std::cout << "Request Header: ";
				cc.OutputReqHeaders();
			}
			if (cc.stream && !silence) std::cout << "Response:" << std::endl;
			cc.Post(
				target_url,
				req,
				res
			);
		}
		if (silence)return res;
		if (config_[working]["response"]["type"] == "commandline") {
			if (!cc.stream)std::cout << "Response:" << std::endl;
			if (!cc.stream)std::cout << res << std::endl;
		}
		else if (config_[working]["response"]["type"] == "json") {
			if (!cc.stream)std::cout << jsonfile::parse(res);
		}
		else {
			std::ofstream out(config_[working]["response"]["type"].asString(), std::ios::app);
			out << "\n" << getReadableTime() << "\n";
			if (config_[working]["response"]["onJson"].asBool()) {
				out << jsonfile::parse(res) << "\n";
			}
			else {
				out << res << "\n";
			}
			out.close();
		}
		if (!silence)std::cout << "====================" << std::endl;
		cc.stream = false;
		showheader = true;
		return res;
	}
}
static std::string outputbool(bool A_) {
	if (A_)return std::string("True");
	else return std::string("False");
}
static void showBanner(std::string workspace_, std::string working_) {
	if (!ui)return;
	std::cout << "workspace:\t" << workspace_ << std::endl
		<< "working:\t" << working_ << std::endl
		<< "=====Apisender=====" << std::endl;
}
static void showBanner(std::string workspace_,std::string working_ , Json::Value c_){
	if (!ui)return;
	std::cout << "workspace:\t" << workspace_ << std::endl
		<< "working:\t" << working_ << std::endl
		<< "===================" << std::endl;
	std::cout << "work: " << std::endl;
	std::vector<std::string> c = c_.getMemberNames();
	for (const auto& it : c)if (it != "base_url")std::cout << it << " ";
	if (c_[working_].isObject()) {
		jsonfile::sep = "";
		std::cout << "\n===================" << std::endl;
		std::cout << "url:" << c_[working_]["url"].asString() << std::endl
			<< "method: " << c_[working_]["method"].asString() << std::endl
			<< "---request----" << std::endl
			<< "header:" << jsonfile::parse(c_[working_]["header"]) << std::endl
			<< "body:" << jsonfile::parse(c_[working_]["request"]["body"]) << std::endl
			<< "---response---" << std::endl
			<< "type:" << jsonfile::parse(c_[working_]["response"]["type"]);
		jsonfile::sep = "\t";
	}
	std::cout << "\n---PERSONAL---\nez:" << outputbool(ez) << " history:" << outputbool(history) << " ui:" << outputbool(ui);
	std::cout << "\n=====Apisender=====" << std::endl;
}
static void clearMonitor() {
	if (!ui)return;
#ifdef _WIN32
	system("cls");
#else 
	system("clear");
#endif
}

int main()
{
#ifdef _DEBUG
	std::cout << "Debug";
#endif
#ifdef _WIN32
	system("title Apisender");
	system("chcp 65001");
#endif
	curl_global_init(CURL_GLOBAL_ALL);
	apisender::error::initErrorInfo();
	system("mkdir " APISENDER_PATH);
	clearMonitor();
	std::string command_1, command_2, command_3, command_4;
	std::string workfile = APISENDER_PATH "/ApiSender.txt" , workname;
	std::string working;
	Json::Value basicconfig = jsonfile::readJsonFile(APISENDER_PATH "/config.json");
	jsonfile::sep = "  ";
	if ((!basicconfig["personal"]["ui"].asBool()) && basicconfig["personal"]["ui"].isBool())ui = false;
	else if (!basicconfig["personal"]["ui"].isBool()) { basicconfig["personal"]["ui"] = true; ui = true; };
	ez = basicconfig["personal"].get("ez", false).asBool();
	history = basicconfig["personal"].get("history", false).asBool();
	showBanner(workname, working);
	std::cout << ">";
	std::cin >> command_1;
	Json::Value config;
	CurlClient cc;
	while (command_1 != "q") {
		if (command_1 == "init" || command_1 == "i") {
			config = Json::nullValue;
			std::cin >> command_2 >> command_3;
			for (char c : command_2) {
				if ((c < 65 || c>122) && c != 46)std::abort();
				else if (c > 90 && c < 97 && c != 95)std::abort();
				else if (c == '`')std::abort();
			}
			config["."]["url"] = "";
			config["."]["request"]["header"] = Json::nullValue;
			config["."]["request"]["body"] = "";
			config["."]["request"]["type"] = "get";
			config["."]["cookies"] = Json::nullValue;
			config["."]["response"]["type"] = "commandline";
			config["."]["response"]["onJson"] = false;
			config["."]["response"]["stream"] = false;
			config["base_url"] = "";
			basicconfig["Apis"]["ApiSender"]["introduction"] = "";
			working = ".";
			if (command_2 == ".") {
				workfile = APISENDER_PATH "/ApiSender.txt";
				workname = "ApiSender";
				basicconfig["Apis"]["ApiSender"]["introduction"] = command_3;
			}
			else {
				workfile = std::string(APISENDER_PATH "/") + command_2 + ".txt";
				workname = command_2;
				basicconfig["Apis"][command_2]["introduction"] = command_3;
			}
			jsonfile::writeJsonFile(workfile, config);
			jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);

			clearMonitor();
			showBanner(workname, working,config);
		}
		else if (command_1 == "set") {
#ifdef _WIN32
			system((std::string("notepad ")+ workfile).c_str());
#elif __linux__
			system((std::string("vim ") + workfile).c_str());
#endif
			config = jsonfile::readJsonFile(workfile);

			clearMonitor();
			showBanner(workname, working, config);
		}
		else if (command_1 == "switch" || command_1 == "sw") {
			config = jsonfile::readJsonFile(workfile);
			std::cin >> command_2;
			if (command_2.find('`') != std::string::npos)std::abort();
			working = command_2;
			if (!config[working].isObject()) {
				config[command_2]["url"] = "$base " + command_2;
				config[command_2]["request"]["header"] = Json::nullValue;
				config[command_2]["request"]["body"] = "";
				config[command_2]["cookies"] = Json::nullValue;
				config[command_2]["method"] = "get";
				config[command_2]["response"]["type"] = "commandline";
				config[command_2]["response"]["onJson"] = false;
				config[command_2]["response"]["stream"] = false;
				basicconfig["Apis"][workname][command_2] = "";
			}
			jsonfile::writeJsonFile(workfile,config);
			jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);

			clearMonitor();
			showBanner(workname, working, config);
		}
		else if (command_1 == "load" || command_1 == "l" || command_1 == "spaceload" || command_1 == "loadspace") {
			std::cin >> command_2;
			bool h = false;
			if (command_2 == "history") {
				std::ifstream in(APISENDER_PATH "/history.txt", std::ios::in);
				std::string line; 
				while (std::getline(in, line))std::cout << line << std::endl;
				h = true;
				goto ENDLOAD;
			}
			for (char c : command_2) {
				if ((c < 65 || c>122) && c != 46 )std::abort();
				else if (c > 90 && c < 97 && c != 95)std::abort();
			}
			if (command_2 == ".") {
				workfile = APISENDER_PATH "/ApiSender.txt";
				workname = "ApiSender";
			}
			else {
				workfile = std::string(APISENDER_PATH "/") + command_2 + ".txt";
				workname = command_2;
			}
			config = jsonfile::readJsonFile(workfile);
			ENDLOAD:
			if (!h)clearMonitor();
			if (!h)showBanner(workname, working, config);
		}
		else if (command_1 == "reload" || command_1 == "rl") {
			config = jsonfile::readJsonFile(workfile);

			clearMonitor();
			showBanner(workname, working, config);
		}
		else if (command_1 == "run") {
			std::cin.ignore();
			apisender::runawork(config, working, workname, false);
		}
		else if (command_1 == "debug") {
			basicconfig = jsonfile::readJsonFile(APISENDER_PATH "/config.json");
			std::cout << "workspace:" << workname << std::endl;
			std::cout << "introduction:" << basicconfig["Apis"][workname].get("introduction", "") << std::endl;
			std::cout << "working:" << working << std::endl;
			std::cout << "introduction:" << basicconfig["Apis"][workname].get(working, "") << std::endl;
			std::cout << "workspace:=====" << std::endl;
			std::cout << jsonfile::jsontoString(basicconfig, "  ") << std::endl;
			std::cout << "working:======="<<std::endl;
			std::cout << jsonfile::jsontoString(config,"  ") << std::endl;
		}
		else if (command_1 == "space") {
			basicconfig = jsonfile::readJsonFile(APISENDER_PATH "/config.json");
			Json::Value apis = basicconfig["Apis"];
			std::vector<std::string> b = apis.getMemberNames();
			for (const auto& it : b)std::cout << it << " ";
			std::cout << std::endl;
		}
		else if (command_1 == "work") {
			std::vector<std::string> c = config.getMemberNames();
			for (const auto& it : c)std::cout << it << " ";
			std::cout << std::endl;
		}
		else if (command_1 == "ui") {
			std::cin >> command_2;
			if (command_2 == "on")ui = true;
			else ui = false;
			basicconfig["personal"]["ui"] = ui;
			clearMonitor();
			showBanner(workname, working, config);
			jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
		}
		else if (command_1 == "ez") {
			std::cin >> command_2;
			if (command_2 == "on")ez = true;
			else ez = false;
			basicconfig["personal"]["ez"] = ez;
			jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
		}
		else if (command_1 == "history") {
			std::cin >> command_2;
			
			if (command_2 == "on")history = true;
			else if (command_2 == "set") {
#ifdef _WIN32
				system((std::string("notepad ") + APISENDER_PATH "/history.txt").c_str());
#elif __linux__
				system((std::string("vim ") + APISENDER_PATH "/history.txt").c_str());
#endif
			}
			else history = false;
			basicconfig["personal"]["history"] = history;
			jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
		}
		else if (command_1 == "this") {
			std::cout << "workspace:" << workname << std::endl;
			std::cout << "introduction:" << basicconfig[workname].get("introduction", "") << std::endl;
			std::cout << "working:" << working << std::endl;
			std::cout << "introduction:" << basicconfig[workname].get(working, "") << std::endl;
			std::cout << "working:=======" << std::endl;
			std::cout << jsonfile::jsontoString(config[working], "  ") << std::endl;
		}
		else if (command_1 == "u") {
			working = basicconfig["personal"]["u"]["working"].asString();
			workname = basicconfig["personal"]["u"]["workspace"].asString();
			command_2 = workname;
			for (char c : command_2) {
				if ((c < 65 || c>122) && c != 46)std::abort();
				else if (c > 90 && c < 97 && c != 95)std::abort();
			}
			if (command_2 == ".") {
				workfile = APISENDER_PATH "/ApiSender.txt";
				workname = "ApiSender";
			}
			else {
				workfile = std::string(APISENDER_PATH "/") + command_2 + ".txt";
				workname = command_2;
			}
			config = jsonfile::readJsonFile(workfile);
			clearMonitor();
			showBanner(workname, working, config);
		}
		else if (command_1 == "version" || command_1 == "v") {
			std::cout << "Compile: " << __DATE__ << " " << __TIME__ << "\n";
		}
#ifdef APISENDER_STRESS_TESTING
		else if (command_1 == "stress") {
			apisender::stress_testing stress;
			clearMonitor();
			if (stress.config != Json::nullValue) {
				std::vector<std::string> worklist = stress.config.getMemberNames();
				for (const auto& w : worklist) {
					std::cout << w << " \n";
				}
			}
			else {
				stress.save();
			}
			std::thread(&apisender::Logger::printfunction, &stress.log).detach();
			stress.log.log(apisender::DEBUG, "stress.log printfunction print\n");
			stress.log.log(apisender::LogType::WARN, "Tip:$() on stress mode is unsupport\n");
			std::string working_stress;
			stress.workers_number = 100;
			std::string command_2_1;

			std::vector<std::thread> threads;
			int thread_top = 0;
			bool status =false;
			std::cin >> command_2_1;
			while (command_2_1 != "q") {
				if (command_2_1 == "start") {
					std::cin >> working_stress;
					stress.status = true;
					stress.log.log(apisender::DEBUG, "status: true\n");
					for (int i = 0; i < stress.workers_number; i++) {
						threads.push_back(std::thread(&apisender::stress_testing::RunF, &stress, working_stress));
						threads[i + thread_top].detach();
					}
					thread_top += stress.workers_number;
					stress.log.log(apisender::DEBUG, "thread_top:" + std::to_string(thread_top) + "\n");
					stress.log.log(apisender::DEBUG, "thread: create RunF\n");
					if (!status)
						std::thread(&apisender::stress_testing::runPerSecond, &stress).detach();
					stress.log.log(apisender::DEBUG, "thread: runPerSecond\n");
					status = true;

				}
				else if (command_2_1 == "stop") {
					threads.clear();
					thread_top = 0;
					stress.status = false;
					status = false;
					stress.log.log(apisender::DEBUG, "status: false\n");
				}
				else if (command_2_1 == "new") {
					stress.log.logtype = apisender::NONE;
					std::string url, request, nm;
					std::cout << "====STRESS TESTING HELPER====\n";
					std::cout << "Working Stress Name:";
					std::cin >> nm;
					std::cout << "FULL URL:";
					std::cin >> url;
					std::cout << "Request Body:";
					std::cin >> request;
					if (jsonfile::parse(request) != Json::nullValue) {
						stress.config[nm]["request"] = jsonfile::parse(request);
					}
					else {
						stress.config[nm]["request"] = request;
					}
					stress.config[nm]["url"] = url;

					stress.save();
					stress.log.logtype = apisender::INFO;
				}
				else if (command_2_1 == "set") {
#ifdef _WIN32
					system((std::string("notepad ") + APISENDER_PATH "/stress.json").c_str());
#elif __linux__
					system((std::string("vim ") + APISENDER_PATH "/stress.json").c_str());
#endif
					stress.config = jsonfile::readJsonFile(APISENDER_PATH "/stress.json");
				}
				std::cin >> command_2_1;
			}
		}
#endif
		command_1 = "";
		command_2 = "";
		command_3 = "";
		command_4 = "";
		std::cout << ">";
		std::cin >> command_1;
	}
	if (working != "" && workname != "") {
		basicconfig["personal"]["u"]["working"] = working;
		basicconfig["personal"]["u"]["workspace"] = workname;
	}	
	jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
	return 0;
}
