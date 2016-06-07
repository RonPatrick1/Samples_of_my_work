/*
 * MyURL.h
 *
 *  Created on: May 15, 2016
 *      Author: Ron Patrick
 */

#ifndef SRC_MYURL_H_
#define SRC_MYURL_H_

#include <string>
#include <vector>
using namespace std;

class MyURL {

private:
	string _scheme = "http";
	string _domainName = "";
	string _path = "/";
	int _port = 80;
	int lastErr = 0;

public:
	static const int BAD_PORT = 1;
	static const int BAD_URL = 2;

	~MyURL() {
	}
	MyURL() {
	}
	MyURL(string url) {
		if (url == "") {
			lastErr = 2;
			cout << "Bad URL: None\n";
			return;
		}
		vector<string> parts = split(url, "//");
		vector<string> parts2;
		string toParse = "";
		if (parts.size() > 1) {
			parts2 = split(parts.at(0), ":");
			if (parts2.size() > 0) {
				_scheme = parts2.at(0);
				if (_scheme == "") {
					_scheme = "http";
				}
				toParse = parts.at(1);
			}
		} else {
			toParse = url;
		}
		while ((toParse.at(0) == '/' || toParse.at(0) == ' ')
				&& toParse.length() > 1) {
			toParse = toParse.substr(1, toParse.length() - 1);
		}
		if (toParse == "" || toParse.length() == 1) {
			lastErr = 2;
			cout << "Bad URL: " + toParse + "\n";
			return;
		}
		commonParsing(toParse);
	}

	MyURL(string newURL, MyURL* currentURL) {
		if(currentURL==NULL) {
			MyURL(newURL);
			return;
		}
		if(newURL.find("/")==string::npos) {
			if(currentURL->path().find("/")==string::npos) {
				_path="/"+newURL;
				_scheme=currentURL->scheme();
				_domainName=currentURL->domainName();
				_port=currentURL->port();
				return;
			}
			string tempP=currentURL->path();
			int found=tempP.find_last_of("/");
			//cout << "found: "+to_string(found)+"\n";
			_path=tempP.substr(0,found);
			_path+="/"+newURL;
			_scheme=currentURL->scheme();
			_domainName=currentURL->domainName();
			_port=currentURL->port();
			//cout << "newURL: "+toString()+"\n";
			return;
		}
		vector<string> parts = split(newURL, "//");
		vector<string> parts2;
		string toParse = "";
		bool gotScheme = false;
		if (parts.size() > 1) {
			parts2 = split(parts.at(0), ":");
			if (parts2.size() > 0) {
				_scheme = parts2.at(0);
				if (_scheme == "") {
					_scheme = "http";
				} else {
					gotScheme = true;
				}
				toParse = parts.at(1);
			}
		} else {
			toParse = newURL;
		}
		while ((toParse.at(0) == '/' || toParse.at(0) == ' ')
				&& toParse.length() > 1) {
			toParse = toParse.substr(1, toParse.length() - 1);
		}
		if (!gotScheme) {
			_domainName = currentURL->domainName();
			_port = currentURL->port();
			_scheme = currentURL->scheme();
			if(currentURL->path().find("/")==string::npos) {
				_path="/"+newURL;
				_scheme=currentURL->scheme();
				_domainName=currentURL->domainName();
				_port=currentURL->port();
				return;
			}
			string tempP=currentURL->path();
			int found=tempP.find_last_of("/");
			//cout << "found: "+to_string(found)+"\n";
			_path=tempP.substr(0,found);
			_path+="/"+newURL;
			_scheme=currentURL->scheme();
			_domainName=currentURL->domainName();
			_port=currentURL->port();
			/*parts2 = split(_path, "/");
			parts2.at(parts.size() - 1) = newURL;
			_path = "";
			for (auto p : parts2) {
				_path += "/" + p;
			}*/
			return;
		}
		if (toParse == "" || toParse.length() == 1) {
			lastErr = 2;
			cout << "Bad URL: " + toParse + "\n";
			return;
		}
		commonParsing(toParse);
		//cout << "newURL: "+toString()+"\n";
	}

	void commonParsing(string toParse) {
		vector<string> parts = split(toParse, "/");
		vector<string> parts2;
		if (parts.size() > 0) {
			parts2 = split(parts.at(0), ":");
			if (parts2.size() > 1) {
				_domainName = parts2.at(0);
				if (_domainName == "") {
					cout << "Bad URL: no domain name\n";
					lastErr = 2;
					return;
				}
				try {
					_port = stoi(parts2.at(1));
				} catch (invalid_argument &e) {
					cout
							<< "Invalid port number in URL: "
									+ parts2.at(1) + "\n";
					_port = 80;
					lastErr = 1;
					return;
				} catch (out_of_range &f) {
					cout
							<< "Port out of range in URL: "
									+ parts2.at(1) + "\n";
					_port = 80;
					lastErr = 1;
					return;
				}
			} else {
				if (parts2.size() > 0) {
					_domainName = parts2.at(0);
					if (_domainName == "") {
						cout << "Bad URL: no domain name\n";
						lastErr = 2;
						return;
					}
				}
			}
			if (parts.size() > 1) {
				int index = 0;
				_path = "";
				for (auto p : parts) {
					if (index > 0) {
						_path += "/" + p;
					}
					index++;
				}
			}
		}
	}

	int getLastErr() {
		return lastErr;
	}

	string scheme() {
		return _scheme;
	}

	string domainName() {
		return _domainName;
	}

	int port() {
		return _port;
	}

	string path() {
		return _path;
	}
	string toString() {
		string output = _scheme + "://" + _domainName;
		if (_port != 80) {
			output += ":" + to_string(_port);
		}
		output += _path;
		if (output.back() == '/') {
			output.pop_back();
		}
		return output;
	}
};

#endif /* SRC_MYURL_H_ */
