/*
 * WebTransactionClient.h
 *
 *  Created on: May 15, 2016
 *      Author: rpatrick
 */

#ifndef SRC_WEBTRANSACTIONCLIENT_H_
#define SRC_WEBTRANSACTIONCLIENT_H_

#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <netdb.h>
#include <gtk/gtk.h>
#include <fstream>
#include <string.h>
using namespace std;

class WebTransactionClient {

private:
	int Socket = -1;
	string _response = ""; // The entire response string (e.g., "HTTP/1.1 200 Ok")
	unordered_map<string, string> headers;
	unsigned char buffer[8192];
	int nDataLength = 0;
	int bufferIndex = 0;

public:
	WebTransactionClient(MyURL url) {

		struct addrinfo *result = NULL;
		struct addrinfo hints;
		struct sockaddr_in  *sockaddr_ipv4;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
		hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
		hints.ai_protocol = 0;          /* Any protocol */
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;

		Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//struct hostent *host;
		//host = gethostbyname(url.domainName().c_str());
		int dwRetval = getaddrinfo(url.domainName().c_str(), "http", &hints, &result);
		if ( dwRetval != 0 ) {
			printf("getaddrinfo failed with error: %d\n", dwRetval);
			_response="HTTP/1.1 404 something";
			finalize();
			return;
		}
		struct sockaddr_in SockAddr;
		sockaddr_ipv4 = (struct sockaddr_in *) result->ai_addr;
		SockAddr.sin_port = htons(url.port());
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = sockaddr_ipv4->sin_addr.s_addr;//*((unsigned long*) host->h_addr);

		//cout << "\nConnecting to "+url.domainName()+"\n";
		if (connect(Socket, (struct sockaddr*) &SockAddr,
				sizeof(struct sockaddr_in)) != 0) {
			cout << "Could not connect.\n";
			_response="HTTP/1.1 404 something";
			finalize();
			return;
		}
		//cout << "Connected.\n";

		string msg = "GET " + url.path() + " HTTP/1.1\r\nHost: "
				+ url.domainName() + "\r\nConnection: close\r\n\r\n";
		send(Socket, msg.c_str(), msg.length(), 0);

		int newLineChecker = 0;
		bool headerDone = false;
		string tempHeader = "";

		while (!headerDone
				&& (nDataLength = recv(Socket, buffer, 8192, 0)) > 0) {
			for (bufferIndex = 0;
					bufferIndex < nDataLength && !headerDone;
					bufferIndex++) {
				tempHeader += buffer[bufferIndex];
				if (buffer[bufferIndex] == '\n') {
					newLineChecker++;
					if (newLineChecker == 2) {
						headerDone = true;
					}
				} else {
					if (buffer[bufferIndex] != '\r') {
						newLineChecker = 0;
					}
				}
			}
		}

		if (tempHeader == "") {
			headers.insert(
					pair<string, string>("Error with transaction",
							"true"));
			return;
		}
		transform(tempHeader.begin(), tempHeader.end(),
				tempHeader.begin(), ::tolower);
		vector<string> tempHeadSplit = split(tempHeader, "\n");
		_response = tempHeadSplit.at(0);
		string first = "";
		string second = "";
		size_t found;
		for (auto h : tempHeadSplit) {
			//cout << "headCheck: "+h+"\n";
			found = h.find(":");
			second = "";
			if (found != string::npos) {
				if (h.substr(found + 1, 1) == " ") {
					first = h.substr(0, found);
					second = h.substr(found + 2,
							h.length() - found - 2);
				}
			} else {
				first = h;
			}
			if (first.length() > 1) {
				while (second.back() == '\n' || second.back() == '\r') {
					second.pop_back();
				}
				while (first.back() == '\n' || first.back() == '\r') {
					first.pop_back();
				}
				headers.insert(pair<string, string>(first, second));
			}
		}
	}

	string getText() {

		string tempData = "";
		if (bufferIndex < nDataLength) {
			for (; bufferIndex < nDataLength; bufferIndex++) {
				tempData += buffer[bufferIndex];
			}
		}
		while ((nDataLength = recv(Socket, buffer, 8192, 0)) > 0) {
			for (bufferIndex = 0; bufferIndex < nDataLength;
					bufferIndex++) {
				tempData += buffer[bufferIndex];
			}
		}

		return tempData;
	} // end getText

	GdkPixbuf* getImage() {

		GdkPixbufLoader *loader3;
		GdkPixbuf *output;
		unsigned long iLength = 0;

		auto search = headers.find("content-length");
		if (search == headers.end()) {
			return output;
		}
		string contentLengthString = search->second;
		try {
			iLength = stol(contentLengthString);
		} catch (invalid_argument &e) {
			cout << "Error reading image.\n";
			return output;
		} catch (out_of_range &f) {
			cout << "Error reading image.\n";
			return output;
		}
		unsigned long iIndex = 0;
		unsigned char inImage[iLength];
		if (bufferIndex < nDataLength) {
			while (bufferIndex < nDataLength) {
				inImage[iIndex++] = buffer[bufferIndex++];
			}
		}
		while ((nDataLength = recv(Socket, buffer, 8192, 0)) > 0) {
			for (bufferIndex = 0; bufferIndex < nDataLength;
					bufferIndex++) {
				inImage[iIndex++] = buffer[bufferIndex];
			}
		}

		loader3 = gdk_pixbuf_loader_new();
		GError *error = NULL;
		bool test=gdk_pixbuf_loader_write(loader3, inImage, iLength, &error);
		if (error != NULL || !test) {
			printf("%s\n", error[0].message);
		}
		gdk_pixbuf_loader_close(loader3, 0);
		output = gdk_pixbuf_loader_get_pixbuf(loader3);
		int width = gdk_pixbuf_get_width(output);
		int height = gdk_pixbuf_get_height(output);
		//cout << "client: width: "+to_string(width)+" height: "+to_string(height)+"\n";
		//GdkPixbufFormat *tempf=gdk_pixbuf_loader_get_format(loader3);
		//string formats=gdk_pixbuf_format_get_name(tempf);
		//cout << "client image format: "+formats+"\n";
		//gimp_cairo_surface_create_from_pixbuf
		//gdk_cairo_set_source_pixbuf


		//I was testing large images
		//output=gdk_pixbuf_scale_simple(output,500,500,GDK_INTERP_NEAREST);
		return output;
	}

	string response() {
		return _response;
	}

	int responseCode() {
		vector<string> repSplit = split(_response, " ");
		if (repSplit.size() < 2) {
			return -1;
		}
		int output = -1;
		try {
			output = stoi(repSplit.at(1));
		} catch (invalid_argument &e) {
			cout << "Error reading responseCode.\n";
		} catch (out_of_range &f) {
			cout << "Error reading responseCode.\n";
		}
		return output;
	}

	unordered_map<string, string> responseHeaders() {
		return headers;
	}

	string getHeader(string key) {
		transform(key.begin(), key.end(), key.begin(), ::tolower);
		string output = "";
		auto search = headers.find(key);
		if (search != headers.end()) {
			output = search->second;
		}
		return output;
	}

	void finalize() {
		//super.finalize();
		//in.close();
		//out.close();
		shutdown(Socket, SHUT_RDWR);
	}
};
// end WebTransactionClient

#endif /* SRC_WEBTRANSACTIONCLIENT_H_ */
