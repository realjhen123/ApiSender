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
#define APISENDER_VERSION "v2026.6.21.1"
#define APISENDER_VERSION_KIND "beta"

bool ui = true;
bool ez = false;
bool history = false;
bool nevertimeout = true;
bool showheader = true;
bool autosync = false;
std::string U_default;
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
	bool Post(const std::string& url, const std::string& data, std::string& response, bool header = true) {
		if (!curl_) {
			return false;
		}
		curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, this->headers);
		curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_, CURLOPT_POST, 1L);
		if (!ez && showheader && header) curl_easy_setopt(curl_, CURLOPT_HEADER, 1L);
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
static std::string outputbool(bool A_) {
	if (A_)return std::string("True");
	else return std::string("False");
}
namespace apisender {
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
    std::string sha256(std::string str_){
        SHA256 sha;
        sha.update(str_.c_str());
        return SHA256::toString(sha.digest());
    }
#ifdef APISENDER_STRESS_TESTING
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
	static long long msc() {
		auto now = std::chrono::system_clock::now();
		auto duration = now.time_since_epoch();
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		return milliseconds.count();
	}
	bool stringcompare(std::string target_str,std::string compare_with) {
		int i = 0;
		for (char c : target_str) {
			if ((compare_with[i] == c + 32) || (compare_with[i] == c - 32) || (compare_with[i] == c))i++;
			else return false;
		}
        if (target_str != compare_with)return false;
        else return true;
	}
	namespace error {
        enum ErrorCode {
            ASE0000,
            ASE0001,
        };
        std::unordered_map<apisender::error::ErrorCode, std::string> ErrorInfo;
        void initErrorInfo() {
            ErrorInfo[ASE0000] = "";
            ErrorInfo[ASE0001] = "No Allow Char";
        }
        void setCustomMessage(std::string E_){
            ErrorInfo[ASE0000] = E_;
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
#ifdef APISENDER_REMOTE_CLOUD
	class ASRCloud {
	private:
		std::string cloudname;
		bool usingcloud = false;
	public:
		Json::Value cloud;
		Json::Value cloudconfig;
		bool get_status();
		std::string get_cloudpath();
		std::string get_cloudname();
		ASRCloud();
		ASRCloud(std::string cloudname_);
		void save();
		void first_init();
		void assistant();
        Json::Value calcpushstr(Json::Value bc_);
		void push(Json::Value bc_);
		void pull(Json::Value& bc_, std::string localconfig_sha256);
		long long get_cloud_msc();
	};
	std::string runawork(Json::Value config_, std::string working, std::string workspace, bool silence, apisender::ASRCloud cloud_);
	bool ASRCloud::get_status() { return usingcloud; };
	std::string ASRCloud::get_cloudpath() {
			std::string path = ".ApiSender";
			path += "/";
			path += cloudname;
			return path;
		}
	std::string ASRCloud::get_cloudname() { return this->cloudname; }
	ASRCloud::ASRCloud() { this->usingcloud = false; };
	ASRCloud::ASRCloud(std::string cloudname_) {
			this->cloudname = cloudname_;
			this->cloud = jsonfile::readJsonFile(this->get_cloudpath());
			this->usingcloud = true;
		}
	void ASRCloud::save() {
			std::ofstream of(this->get_cloudpath());
			of << jsonfile::parse(cloud);
			of.close();
		}
	void ASRCloud::first_init() {
			cloud["base_url"] = "";
			cloud["cloud"] = "this";

			cloud["push"]["request"]["body"]["data"] = "$(D)";
			cloud["push"]["request"]["body"]["object"] = "APISENDER_CLOUD";
			cloud["push"]["request"]["body"]["token"] = "$(`login)";
			cloud["push"]["request"]["header"] = Json::nullValue;
			cloud["push"]["url"] = "$base /push";
			cloud["push"]["response"]["stream"] = false;
			cloud["push"]["response"]["type"] = "commandline";
			cloud["push"]["method"] = "POST";

			cloud["pull"]["request"]["body"]["object"] = "APISENDER_CLOUD";
			cloud["pull"]["request"]["body"]["token"] = "$(`login)";
			cloud["pull"]["request"]["header"] = Json::nullValue;
			cloud["pull"]["url"] = "$base /pull";
			cloud["pull"]["response"]["stream"] = false;
			cloud["pull"]["response"]["type"] = "commandline";
			cloud["pull"]["method"] = "POST";

			cloud["login"]["request"]["body"]["username"] = "";
			cloud["login"]["request"]["body"]["password"] = "";
			cloud["login"]["request"]["header"] = Json::nullValue;
			cloud["login"]["url"] = "$base /login";
			cloud["login"]["response"]["stream"] = false;
			cloud["login"]["response"]["type"] = "commandline";
			cloud["login"]["method"] = "POST";

			this->cloudname = "cloud.json";
			this->save();
		}
	void ASRCloud::assistant() {
			std::string i;
			std::cout << "=======Cloud Assistant=============\n"
				<< "Input Your Base Url:\n";
			std::cin >> i;
			cloud["base_url"] = i;
			std::cout << "Input Your Login Route:\n";
			std::cin >> i;
			cloud["login"]["url"] = "$base " + i;
			std::cout << "Input Your Pull Route:\n";
			std::cin >> i;
			cloud["pull"]["url"] = "$base " + i;
			std::cout << "Input Your Push Route:\n";
			std::cin >> i;
			cloud["push"]["url"] = "$base " + i;
			std::cout << "Using Username and Password? (Confirm y or n):";
			std::cin >> i;
			if (i == "y" || i == "Y") {
				std::cout << "Pay a Attention ,Your Password and Username will be a unencrypted way to save on file\n";
				std::cout << "Please confirm that the compromise of this username and password is not a concern for you.Because it Storing in PLAINTEXT.\n";
				std::cout << "Or You can using $(INPUT) on username or password ,To set up Input username or password When needed\n";
				std::cout << "Using $(INPUT)? (y or n)\n";
				std::string j;
				std::cin >> j;
				if (j == "y") {
					cloud["login"]["request"]["body"]["username"] = "$(INPUT)";
					cloud["login"]["request"]["body"]["password"] = "$(INPUT)";
				}
				else {
					std::cout << "Input Your Username:";
					std::cin >> i;
					cloud["login"]["request"]["body"]["username"] = i;
					std::cout << "Input Your Password:";
					std::cin >> i;
					cloud["login"]["request"]["body"]["password"] = i;
				}
			}
			std::cout << "So as to set up in detailed ,using cloud set and just like other apisender's apis\n";
			std::cout << "Here some commands\n";
			std::cout << "cloud push\n"
				<< "cloud pull\n"
				<< "cloud assistant\n"
				<< "cloud set\n"
				<< "cloud clear\n";
			this->save();
		}
    Json::Value ASRCloud::calcpushstr(Json::Value bc_){
		Json::Value d;
		for (const auto& spacename : bc_["Apis"].getMemberNames()) {
			std::string path = APISENDER_PATH "/";
			d[spacename] = base64_encode(
				jsonfile::parse(
					jsonfile::readJsonFile(path + "/" + spacename + ".txt")));
		}
		d["APISENDER_U_WORKING"] = bc_["personal"]["u"]["working"].asString();
		d["APISENDER_U_WORKSPACE"] = bc_["personal"]["u"]["workspace"].asString();
        return d;
    }
	void ASRCloud::push(Json::Value bc_) {
		Json::Value d;
		for (const auto& spacename : bc_["Apis"].getMemberNames()) {
			std::string path = APISENDER_PATH "/";
			d[spacename] = base64_encode(
				jsonfile::parse(
					jsonfile::readJsonFile(path + "/" + spacename + ".txt")));
		}
		d["APISENDER_TIME_COUNT"] = std::to_string(apisender::msc());
		d["APISENDER_U_WORKING"] = bc_["personal"]["u"]["working"].asString();
		d["APISENDER_U_WORKSPACE"] = bc_["personal"]["u"]["workspace"].asString();
		std::string rawstr = base64_encode(
			jsonfile::parse(d));
		this->cloudconfig["raw"] = gzip::compress(rawstr.data(), rawstr.size(),7);
		apisender::runawork(this->cloud, "push", "cloud", true, *this);
	}
	void ASRCloud::pull(Json::Value& bc_,std::string localconfig_sha256 = "") {
        //apisender::sha256( jsonfile::parse( this->calcpushstr(bc_) , ""));
		std::string rawstr = apisender::runawork(cloud, "pull", "cloud", true, *this);
        Json::Value d;
        try {
    		d = jsonfile::parse(
	    		base64_decode(
		    		gzip::decompress(rawstr.data(), rawstr.size())
			));
        }
        catch (const std::runtime_error& e){
            std::string custom_error_str = "ASRCloud.pull rawstr=" + rawstr;
            apisender::error::setCustomMessage(e.what());
            apisender::error::DropError(apisender::error::ASE0000, custom_error_str);
        }
        std::string tc = d["APISENDER_TIME_COUNT"].asString();
        if (localconfig_sha256 != ""){
            d.removeMember("APISENDER_TIME_COUNT");
            std::string remote_s = apisender::sha256(
                jsonfile::parse(
                    d,""));
            if (remote_s != localconfig_sha256){
                std::cout << "Pull Error\nSave config on remote or local?(r or l)";
                std::string c;
                std::cin >> c;
                if (c == "r" || c == "remote" || c == "cloud"){
                   ; 
                }else {
                    return;
                }
            }
        }
        d["APISENDER_TIME_COUNT"] = tc;
		for (const auto& spacename : d.getMemberNames()) {
			Json::Value space;
			if (spacename.find("APISENDER_") == std::string::npos) {
				space = jsonfile::parse(
					base64_decode(
						d[spacename].asString()));
				std::string path = APISENDER_PATH "/";
				jsonfile::writeJsonFile(path + spacename + ".txt", space);
				bc_["Apis"][spacename] = Json::nullValue;
			}
		}
		bc_["personal"]["u"]["working"] = d["APISENDER_U_WORKING"];
		bc_["personal"]["u"]["workspace"] = d["APISENDER_U_WORKSPACE"];
	}
	long long ASRCloud::get_cloud_msc()
	{
		Json::Value res = jsonfile::parse(apisender::runawork(this->cloud, "pull", "cloud", true,apisender::ASRCloud()));
		return std::stoll(res.get("APISENDER_TIME_COUNT", -1).asString());
	}
#endif
    enum par_tp{
      _int,
      _str,
      _bool
    };
    struct par_re{
        int int_;
        std::string str_;
        bool bool_;
        bool haveparse;
        std::string raw;
        par_tp t_ = _str;
    };
    par_re paraparse(std::string pa_){
        par_re P;
        if (pa_.find("$(") == std::string::npos){
            P.haveparse = false;
            P.raw = pa_;
            return P;
        }
        P.haveparse = true;
        std::queue<char> c;
        std::queue<char> d;
        int a = 0 ,b = 0;
        for (const char p : pa_){
            switch (p){
                case '<':
                    if (a != 1)a = 1;
                    break;
                case '>':
                    if (a == 1)a = 2;
                case '$':
                    b = 1;
                    break;
                case '(':
                    if (b == 1)b = 2;
                    break;
                case ')':
                    if (b == 2)b = 3;
                default:
                    if (a == 1)c.push(p);
                    else if (b == 2)d.push(p);
                    if (a == 2){
                        std::string p_ = "";
                        while (!c.empty()){
                            p_ += c.front();
                            c.pop();
                        }
                        if (apisender::stringcompare(p_ , "string") || apisender::stringcompare(p_ , "str")){
                            P.t_ = _str;
                        }else if (p_ == "int"){
                            P.t_ = _int;
                        }else if (p_ == "bool"){
                            P.t_ = _bool;
                        }
                        a = 0;
                    }else if (b == 3){
                        std::string p_ = "";
                        while (!d.empty()){
                            p_ += d.front();
                            d.pop();
                        }
                        switch (P.t_){
                            case _str:
                                P.str_ = p_;
                                break;
                            case _bool:
                                if (apisender::stringcompare(p_ , "true"))P.bool_ = true;
                                else if (apisender::stringcompare(p_ , "false"))P.bool_ = false;
                                break;
                            case _int:
                                P.int_ = std::atoi(p_.c_str());
                                break;
                            default:
                                break;
                        }
                        b = 0;
                    }
                    break;
            }
        }
        return P;
    }
    struct sub_run_re{
        std::string str;
        bool isCmd = false;
        par_re R;
    };
    sub_run_re sub_runawork(std::string str){
        sub_run_re R;
        R.R = paraparse(str);
        if (!R.R.havepare){
            R.str = str;
            R.isCmd = false;
            return R;
        }
        if (R.R.t_ == _str){
            if (P.str_ == "INPUT"){
                R.str = "NeedInput";
                R.isCmd = true;
                return R;
            }else if(P.str_.find("`") != std::string::npos){
                R.str = "NeedWeb";
                R.isCmd = true;
                return R;
            }
            else {
                R.str = R.R.str_;
                R.isCmd = false;
                return R;
            }
        }
        R.str = "";
        R.isCmd = false;
        return R;
    }
	std::string runawork(Json::Value config_,std::string working,std::string workspace,bool silence 
#ifdef APISENDER_REMOTE_CLOUD
		, apisender::ASRCloud cloud_ = apisender::ASRCloud() 
#endif
	) 
	{
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
		if (apisender::stringcompare(config_[working]["method"].asString(),"get")) {
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
		else if (stringcompare(config_[working]["method"].asString(),"post")) {
			std::string req;
			if (config_[working]["request"]["body"].isObject()) {
				Json::Value req_json = config_[working]["request"]["body"];
				std::vector<std::string> h = req_json.getMemberNames();
				for (const auto& h_ : h) {
					std::string h_s = req_json[h_].asString();
                    auto R = apisender::sub_runawork(h_s);
                    if (R.isCmd){
						if (R.str == "NeedInput") {
							std::cout << "Input \"" << h_ << "\" >";
							std::string in;
							std::getline(std::cin >> std::ws, in);
                            switch (R.R.t_){
                                case apisender::_int:
                                    req_json[h_] = std::atoi(in);
                                    break;
                                case apisender::_str:
                                    req_json[h_] = in;
                                    break;
                                case apisender::_bool:
                                    if (apisender::stringcompare(in,"true")){
                                        req_json[h_] = true;
                                    }else{
                                        req_json[h_] = false;
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
						// $(apisender`/login)
						//else if (h_s.find('`') != std::string::npos) {
                        else if (R.str == "NeedWeb"){
                            h_s = "$(" + R.R.str_ + ")";
							std::string target_working, target_space , target_spacename;
							target_spacename = h_s.substr(2, h_s.find('`') - 2);
							if (target_spacename == "~" || target_spacename == "") {
								target_spacename = workspace;
							}
							else if (target_spacename == ".") {
								target_spacename = "ApiSender";
							}
							target_space = APISENDER_PATH;

							//if not define APISENDER_REMOTE_CLOUD
							//also exist target_sapce += "/" + target_spacename + ".txt";
#ifdef APISENDER_REMOTE_CLOUD
							if (cloud_.get_status()) {
								target_space = cloud_.get_cloudpath();
							}
							else {
#endif
								target_space += "/" + target_spacename + ".txt";
#ifdef APISENDER_REMOTE_CLOUD
							}
#endif
							for (char c : h_s.substr(2, h_s.find('`') - 2)) {
								if ((c < 65 || c>122) && c != 46 && c != 126)apisender::error::DropError(apisender::error::ASE0001, "h_s");
								else if (c > 90 && c < 97 && c != 95 && c != 126)apisender::error::DropError(apisender::error::ASE0001, "h_s");
							}
							
							target_working = h_s.substr(h_s.find('`') +1 , h_s.size() - h_s.find('`'));
							target_working = target_working.substr(0, target_working.size() - 1);
							if (!ez && !silence)std::cout << target_spacename << " " << target_working << "\n";
                            std::string re_ = apisender::runawork(jsonfile::readJsonFile(target_space), target_working, target_spacename, true);
                            switch (R.R.t_){
                                case apisender::_str:
                                    req_json[h_] = "<string>" + re_;
                                    break;
                                case apisender::_int:
                                    req_json[h_] = "<int>" + re_;
                                    break;
                                case apisender::_bool:
                                    req_json[h_] = "<bool>" + re_;
                            }
							if (!ez && !silence)std::cout << req_json[h_] << std::endl;
						}
#ifdef APISENDER_REMOTE_CLOUD
						else if (h_s == "$(D)") {
							if (cloud_.get_status()) {
								req_json[h_] = cloud_.cloudconfig["raw"];
							}
							
						}
#endif 
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
				res,
				!silence
			);
		}
        if (config_[working]["response"]["type"].asString() == "json"){
            std::string parse = config_[working]["response"]["parse"].asString();
            if (parse == "null"){    
                ;
            } else { 
                std::queue<std::string> plist;    
                std::stack<char> tmp;
                int itc = 0;
                bool ti = false;
                for (const char p : parse){
                    tmp.push(p);
                    if (p == '-' && parse.size() >= (itc+1)){
                        if (parse[itc+1] == '>'){
                            while(!tmp.empty()){
                                char t = tmp.top();
                                if (t == ' ' || t == '-'){
                                    tmp.pop();
                                }else{
                                    break;
                                }
                            }
                        }
                        std::string cs = "";
                        while (!tmp.empty()){
                            std::string c_s = cs;
                            cs = tmp.top() + c_s;
                            tmp.pop();
                        }
                        plist.push(cs);
                        ti = true;
                    } else {
                        if (ti){
                            if (p == ' ' || p == '>')tmp.pop();
                            else ti=false;
                        }
                    }
                    itc++;
                }
                Json::Value r_json = jsonfile::parse(res);
                while(!plist.empty()){
                    std::string pname = plist.front();
                    r_json = r_json[pname];
                    plist.pop();
                }
                res = r_json.asString();
            }
        }
		if (silence)return res;
		
		if (config_[working]["response"]["type"].asString() == "commandline") {
			if (!cc.stream)std::cout << "Response:" << std::endl;
			if (!cc.stream)std::cout << res << std::endl;
		}
		else if (config_[working]["response"]["type"].asString() == "json") {
            std::string parse = config_[working]["response"]["parse"].asString();
            if (parse == "null"){    
                if (!cc.stream)std::cout << jsonfile::parse(res);
            } else {
			    if (!cc.stream)std::cout << "Response:" << std::endl;
                if (!cc.stream)std::cout << res << std::endl;
            }
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
        if (c_[working_]["response"]["type"].asString() == "json")std::cout << "\nparse:" << c_[working_]["response"]["parse"].asString();
		jsonfile::sep = "\t";
	}
	std::cout << "\n---PERSONAL---\nez:" << outputbool(ez) << " history:" << outputbool(history) << " ui:" << outputbool(ui) << std::endl;
#ifdef APISENDER_REMOTE_CLOUD
	std::cout << "AutoSync:" << outputbool(autosync) << " udefault:" << U_default << std::endl;
#endif
	std::cout << "=====Apisender=====" << std::endl;
}
static void clearMonitor() {
	if (!ui)return;
#ifdef _WIN32
	system("cls");
#else 
	system("clear");
#endif
}

int main(int argc, char* argv[])
{
    {
        std::string a = apisender::sub_runawork("aaaINPUT)");
        return 0;
    }
	std::string command_1 = "", command_2, command_3, command_4;
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
	std::string workfile = APISENDER_PATH "/ApiSender.txt" , workname;
	std::string working;
	Json::Value basicconfig = jsonfile::readJsonFile(APISENDER_PATH "/config.json");
	jsonfile::sep = "  ";
	ui = basicconfig["personal"].get("ui", true).asBool();
	ez = basicconfig["personal"].get("ez", false).asBool();
	history = basicconfig["personal"].get("history", false).asBool();
	autosync = basicconfig["personal"].get("autosync", false).asBool();
	U_default = basicconfig["personal"]["u"].get("U_default", "local").asString();
	for (int i = 0; i < argc; i++) {
		std::string c = argv[i];
		if (c == "--unautosync" || c == "-nsync")autosync = false;
	}
    showBanner(workname, working);
	Json::Value config;
#ifdef APISENDER_REMOTE_CLOUD
	apisender::ASRCloud cloud(
		basicconfig["personal"].get("cloudname", "cloud.json").asString()
	);
	if (autosync){
        cloud.pull(basicconfig, apisender::sha256(
            jsonfile::parse(
                cloud.calcpushstr(
                    basicconfig),"")
                )
            );
    }
#endif
	std::cout << ">";
	std::cin >> command_1;
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
			config["."]["response"]["parse"] = "null";
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
				config[command_2]["response"]["parse"] = "null";
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
#ifdef APISENDER_REMOTE_CLOUD
			if (U_default == "remote")cloud.pull(basicconfig);
#endif
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
			std::cout << "Version: " << APISENDER_VERSION << "\n";
			std::cout << "Version Kind: " << APISENDER_VERSION_KIND << "\n";
		}
		else if (command_1 == "personal") {
			std::cin >> command_2 >> command_3;
			if (command_2 == "autosync") {
				if (command_3 == "on") {
					autosync = true;
#ifdef APISENDER_REMOTE_CLOUD
					cloud.pull(basicconfig);
#endif
				}
				else if (command_3 == "off") {
					autosync = false;
				}
				basicconfig["personal"]["autosync"] = autosync;
			}
			else if (command_2 == "udefault") {
				if (command_3 == "remote")basicconfig["personal"]["u"]["U_default"] = command_3;
				else basicconfig["personal"]["u"]["U_default"] = "local";
				U_default = basicconfig["personal"]["u"]["U_default"].asString();
			}
			jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
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
#ifdef APISENDER_REMOTE_CLOUD
		else if (command_1 == "cloud" || command_1 == "push" || command_1 == "pushy") {
			if (cloud.cloud == Json::nullValue) {
				cloud.first_init();
			}
			if (command_1 == "push" || command_1 == "pushy")command_2 = "push";
			else {
				std::cin >> command_2;
			}
			if (command_2 == "cl") {
				std::cout << "\ncloud help login push pull logout set uwork\n";
			}
			else if (command_2 == "set") {
				std::string p = cloud.get_cloudpath();
#ifdef _WIN32
				system((std::string("notepad ") + p).c_str());
#elif __linux__
				system((std::string("vim ") + p).c_str());
#endif
				cloud.cloud = jsonfile::readJsonFile(cloud.get_cloudpath());
			}
			else if (command_2 == "assistant" || command_2 == "help") {
				cloud.assistant();
			}
			else if (command_2 == "push") {
				std::string doublecheck;
				if (command_1 != "pushy") {	
					std::cout << "This Command Will COVER remote, Are you sure?(Only 'y')";
					std::cin >> doublecheck;
				}
				if (doublecheck == "y" || command_1 == "pushy") {
					basicconfig["personal"]["u"]["working"] = working;
					basicconfig["personal"]["u"]["workspace"] = workname;
					cloud.push(basicconfig);
				}
			}
			else if (command_2 == "pull") {
				cloud.pull(basicconfig);
				basicconfig["personal"]["Lpull"] = std::to_string(cloud.get_cloud_msc());
				jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
			}
			else if (command_2 == "u") {
				cloud.pull(basicconfig);
				command_1 = "u";
				command_2 = "";
				command_3 = "";
				command_4 = "";
				goto NO_INPUT_COMMAND;
			}
			clearMonitor();
			showBanner(workname, working, config);
		}
#endif
		else if (command_1 == "remove") {
			std::cout << "Are you sure?";
			std::cin >> command_2;
			if (command_2 == "y") {
				autosync = false;
				basicconfig["personal"]["autosync"] = false;
				for (std::string it : basicconfig["Apis"].getMemberNames()) {
					std::string p = APISENDER_PATH;
					p += "/";
					p += it;
					p += ".txt";
					std::remove(p.c_str());
					basicconfig["Apis"].removeMember(it);
				}
				jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
			}
		}
		else if (command_1 == "print") {
			std::cin >> command_2;
			if (command_2 == "config")std::cout << config.toStyledString();
			else if (command_2 == "basicconfig")std::cout << basicconfig.toStyledString();
			else if (command_2 == "cloud")std::cout << cloud.cloud.toStyledString();
			else if (command_2 == "cloudconfig")std::cout << cloud.cloudconfig.toStyledString();
		}
		command_1 = "";
		command_2 = "";
		command_3 = "";
		command_4 = "";
		std::cout << ">";
		std::cin >> command_1;
	NO_INPUT_COMMAND:;
	}
	if (working != "" && workname != "") {
		basicconfig["personal"]["u"]["working"] = working;
		basicconfig["personal"]["u"]["workspace"] = workname;
	}
#ifdef APISENDER_REMOTE_CLOUD
	if (autosync)cloud.push(basicconfig);
#endif
	jsonfile::writeJsonFile(APISENDER_PATH "/config.json", basicconfig);
	return 0;
}
