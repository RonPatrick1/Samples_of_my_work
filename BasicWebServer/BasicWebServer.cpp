/*
 * CIS 371 Web Server
 * Summer 2016
 *
 * Ron Patrick
 *
 * A lot of this code is from my Data Comm(CIS 457)
 * projects.  I did every project I could in C++.
 * I made it multi-threaded for future use.  My
 * Data Comm projects were multi-threaded anyway.
 *
 * For Assignment 6:
 * 		I tweaked the code I was using to get
 * 		output from the .php software to get
 * 		output from other language interpreters.
 * 		Currently this server works with Ruby,
 * 		Python, and Perl.  I'll include the
 * 		"Hello, World" code for each of those
 * 		languages with my assignment submission.
 * 		The new code can be found at line 321
 * 		in this file.(5/21/2016)
 */

#include <string>
#include <unordered_map>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <chrono>
#include <list>
#include <mutex>
#include <string.h>
#include <condition_variable>
#include <fstream>
#include <algorithm>
#include <sys/wait.h>
#include <errno.h>
#define BUFFERSIZE 4096
#define GetCurrentDir getcwd
using namespace std;

string port = "";
int portNumber;
//vector<pthread_t> ClientThreads;
unordered_map<int,pthread_t> ClientThreads;
//vector<int> ClientThreadSockets;
unordered_map<int,int> ClientThreadSockets;
pthread_t emptyThread = pthread_t();
bool mainThreadEnding = false;
mutex crmutex;
condition_variable cond_var;
unordered_map<string,string> typeExt;
unordered_map<string,string> otherApps;
int sockfd;

void PopulateExtensions();

typedef struct ClientAddressSocket {
	int socket;
	string clientAddress = "ip not set";
	int id;
} CLIENTSOCKETADDRESS, *PCLIENTSOCKETADDRESS;
//vector<ClientAddressSocket> threadData;
unordered_map<int,ClientAddressSocket> threadData;

/*I was using boost to split my strings and tokenize, but boost has
 * been removed from EOS/ArchLab profiles since last summer classes ended,
 *  so I made my own 'split' string function.  Seems to work perfectly
 *  so far.
 */
vector<string> split(string s, string any_of) {
	vector<string> seps;
	string temp = s;
	int foundat = temp.find(any_of);
	while (foundat != -1 && temp.length() > 0) {
		seps.push_back(temp.substr(0, foundat));
		temp = temp.substr(foundat + 1, temp.length());
		foundat = temp.find(any_of);
	}
	if (temp.length() > 0) {
		seps.push_back(temp);
	}
	return seps;
}

string sToLower(string in) {
	string output=in;
	transform(output.begin(),output.end(),output.begin(),::tolower);
	return output;
}

bool SendData(int &socket, void *buffer, int bufferlength) {

	char *pbuffer = (char*) buffer;
	while (bufferlength > 0) {
		int bytesSent = send(socket, pbuffer, bufferlength, 0);
		if (bytesSent == -1) {
			return false;
		}
		pbuffer += bytesSent;
		bufferlength -= bytesSent;
	}
	return true;
}

bool ReceiveData(int &socket, void *buffer, int bufferlength) {

	char *pbuffer = (char*) buffer;
	while (bufferlength > 0) {
		int bytesRead = recv(socket, pbuffer, bufferlength, 0);
		if (bytesRead == -1) {
			return false;
		} else if (bytesRead == 0) {
			return false;
		}
		pbuffer += bytesRead;
		bufferlength -= bytesRead;

	}
	return true;
}

int ReceiveDataByLine(int &socket, void *buffer, int bufferlength) {
	char *pbuffer = (char*) buffer;
	char *tpbuffer = pbuffer;
	int length = 0;
	while (bufferlength > 0) {
		int bytesRead = recv(socket, pbuffer, bufferlength, 0);
		if (bytesRead == -1) {
			return -1;
		} else if (bytesRead == 0)
			return -1;

		pbuffer += bytesRead;
		length += bytesRead;
		for (int i = 0; i < length; i++) {
			if (tpbuffer[i] == '\n') {
				return length;
			}
		}
		bufferlength -= bytesRead;
	}
	return length;
}

void *ClientSession(void *threadarg) {

	PCLIENTSOCKETADDRESS cas = (PCLIENTSOCKETADDRESS) threadarg;
	int clientsocket = cas->socket;
	string clientAddress = cas->clientAddress;
	int id=cas->id;
	cout << "started thread id: " << id << endl;

	char tbuffer[10000];

	//set a 10 second activity timeout for these threads.
	time_t start=time(NULL);

	timeval to;
	to.tv_sec = 0;
	to.tv_usec = 50000;//half a second div by 10
	setsockopt(clientsocket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &to,
			sizeof(to));
	bool connectionClose=false;
	while (!connectionClose) {

		int bytesRead = 0;
		bool out = false;
		string tempHeader = "";

		if(difftime(time(NULL),start)>10.0) {
			shutdown(clientsocket, SHUT_RDWR);
			ClientThreadSockets.erase(id);
			threadData.erase(id);
			ClientThreads.erase(id);
			cout << "Client: " + clientAddress + " disconnected.\n\n";
			return 0;
		}

		cout << "Before rcvData\n";
		do {
			bytesRead = ReceiveDataByLine(clientsocket, tbuffer, 10000);
		} while (bytesRead<2) ;
		while (bytesRead > 1) {
			cout << "bytesRead: " << bytesRead << endl;
			for (int i = 0; i < bytesRead; i++) {
				cout << tbuffer[i];
				tempHeader += tbuffer[i];
			}
			if (bytesRead < 3) {
				out = true;
			}
			if (!out) {
				bytesRead = ReceiveData(clientsocket, tbuffer, 10000);
			}
		}
		cout << "After rcvData\n";

		vector<string> headerLineSplit = split(tempHeader, "\n");

		//Inital 'get' parsing.  Just checking if there's a query string
		//in the file name.  Also checking if the server should close
		//this thread or not.
		bool doQueryString=false;
		bool doPost=false;
		string queryString="";
		for(auto h:headerLineSplit) {
			string h2=h;
			h2=sToLower(h2);
			/*if(h2.find("post ")!=string::npos) {
				doQueryString=true;
				queryString=headerLineSplit.at(headerLineSplit.size()-1);
				queryString="?"+queryString;
				doPost=true;
			}*/
			if(h2.find("get ")!=string::npos) {
				if(h2.find("?")!=string::npos) {
					vector<string> querySplit=split(h," ");
					if(querySplit.size()>1) {
						queryString=querySplit.at(1);
						doQueryString=true;
					}
				}
			}
			if(h2.find("connection:")!=string::npos) {
				vector<string> connSplit=split(h2," ");
				if(connSplit.size()>1) {
					if(connSplit.at(1).find("close")!=string::npos) {
						cout << "Found Connection close msg\n\n";
						connectionClose=true;
					}
				}
			}
		}
		bool notFound = false;
		string remotePath = "";
		unsigned long long fsize=0;
		string contentType="";
		bool otherApp=false;
		string otherAppExt="";

		//get file extension
		//Also checking to see if I told the server to quit
		//(sometimes the OS won't release the socket for a
		//couple of minutes after I terminate the server
		//making it harder to debug it)
		if (headerLineSplit.size() > 0) {
			vector<string> firstLineSplit = split(headerLineSplit.at(0),
					" ");
			if (firstLineSplit.size() > 0) {
				remotePath = firstLineSplit.at(1);
				if(doQueryString) {
					vector<string> qfSplit=split(remotePath,"?");
					remotePath=qfSplit.at(0);
				}
				string remoteFileName = remotePath;
				while (remoteFileName.at(0) == '/'
						&& remoteFileName.length() > 1) {
					remoteFileName = remoteFileName.substr(1,
							remoteFileName.length() - 1);
				}
				if (remoteFileName=="RonSaysQuit") {
					shutdown(clientsocket, SHUT_RDWR);
					ClientThreadSockets.erase(id);
					threadData.erase(id);
					ClientThreads.erase(id);
					cout << "Client: " + clientAddress + " disconnected.\n\n";
					mainThreadEnding=true;
					cond_var.notify_one();
					return 0;
				}

				vector<string> rSplit = split(remoteFileName, ".");
				if (rSplit.size() > 1) {
					string tempExtension = rSplit.at(rSplit.size() - 1);
					tempExtension=sToLower(tempExtension);
					auto search=otherApps.find(tempExtension);
					if(search!=otherApps.end()) {
						otherApp=true;
						otherAppExt=search->second;
					}
				}
			}
		}

		//Process any file except .py, .rb, .pl, .php
		if(!otherApp) {
			if (headerLineSplit.size() > 0) {
				vector<string> firstLineSplit = split(headerLineSplit.at(0),
						" ");
				if (firstLineSplit.size() > 0) {
					remotePath = firstLineSplit.at(1);
					string remoteFileName = remotePath;
					while (remoteFileName.at(0) == '/'
							&& remoteFileName.length() > 1) {
						remoteFileName = remoteFileName.substr(1,
								remoteFileName.length() - 1);
					}

					fstream inFile;
					inFile.open(remoteFileName,
							ios::in | ios::binary | ios::ate);
					if (inFile.is_open()) {
						vector<string> rSplit = split(remoteFileName, ".");
						string contentType = "text/plain";
						if (rSplit.size() > 1) {
							string tempExtension = rSplit.at(rSplit.size() - 1);
							tempExtension=sToLower(tempExtension);
							auto search=typeExt.find(tempExtension);
							if (search!=typeExt.end()) {
								contentType=search->second;
							}
						}

						fsize = inFile.tellg();
						inFile.seekg(0, ios::beg);

						string outputS = "HTTP/1.1 200 OK\nContent-Type: "
								+ contentType + "\nContent-Length: "
								+ to_string(fsize) + "\nConnection: keep-alive\n\n";
						char output[outputS.length() + fsize];
						strncpy(output,outputS.c_str(),outputS.length());
						inFile.read(output + outputS.length(), fsize);
						inFile.close();

						fsize+=outputS.length();
						SendData(clientsocket, (void*) output, fsize);
					} else { notFound = true; }
				} else { notFound = true; }
			} else { notFound = true; }
		}

		//Process otherApp file and parse a query_string
		if(otherApp && !notFound) {
			bool queryError=false;
			unordered_map<string,string> keyVal;

			//construct a hash map of all the variables
			//and values in the query string
			if(doQueryString) {
				vector<string> qSplit=split(queryString,"?");
				vector<string> qSplit2;
				qSplit2=split(qSplit.at(1),"&");
				if(qSplit2.size()<1) {
					cout << "qError2\n";
					notFound=true;
					queryError=true;
				}
				else {
					for(auto v:qSplit2) {
						vector<string> vSplit=split(v,"=");
						if(vSplit.size()>0) {
							string vFirst=vSplit.at(0);
							string vSecond="NULL";
							if(vSplit.size()>1) {
								vSecond=vSplit.at(1);
							}
							keyVal.insert(pair<string,string>(vFirst,vSecond));
						}
					}
				}
			}
			string outData="";
			string outputS="";
			if(!queryError) {
				if (headerLineSplit.size() > 0) {
					vector<string> firstLineSplit = split(headerLineSplit.at(0),
							" ");
					if (firstLineSplit.size() > 0) {
						remotePath = firstLineSplit.at(1);
						if(doQueryString) {
							vector<string> qfSplit=split(remotePath,"?");
							remotePath=qfSplit.at(0);
						}
						string remoteFileName = remotePath;
						while (remoteFileName.at(0) == '/'
								&& remoteFileName.length() > 1) {
							remoteFileName = remoteFileName.substr(1,
									remoteFileName.length() - 1);
						}

						//check if the .php (or .rb, .py, .pl) file is
						//actually on the server.
						fstream inFile;
						inFile.open(remoteFileName,
								ios::in | ios::binary | ios::ate);
						if (!inFile.is_open()) {
							notFound=true;
						}
						else {
							contentType = "text/html";

							//open a pipe and run the PHP software on the
							//server's system (or other language interpreter)
							//For my system, the command-line
							//goes something like this:
							//php-cgi -f calendar.php month=2 year=2014
							//Recieve the piped output into a string
							//called 'fromOtherApp' then send the html back to
							//the browser.
							pid_t pid;
							int p[2];
							string fromOtherApp="";
							pipe(p);
							pid = fork();	//makes a copy of the program running
											//now two processes are running

							if (pid == 0) {
								int argSize=keyVal.size()+3;
								char *argv[argSize];
								vector<string> argvV;
								argvV.push_back(otherAppExt);
								if(otherAppExt=="php-cgi") {
									argvV.push_back("-f");
									argSize++;
								}
								argvV.push_back(remoteFileName);
								if(keyVal.size()>0) {
									for(auto k:keyVal) {
										argvV.push_back(k.first+"="+k.second);
									}
								}
								for(int i=0;i<argSize-1;i++) {
									argv[i]=(char*)malloc(argvV.at(i).length()+1);
									strncpy(argv[i],argvV.at(i).c_str(),
											(unsigned long)argvV.at(i).length());
									argv[i][argvV.at(i).length()]='\0';
								}
								argv[argSize-1]=0;
								dup2(p[1], 1);
								close(p[0]);
								execvp(argv[0], argv);
								exit(EXIT_FAILURE);
							}
							else {
								close(p[1]);
								fd_set rfds;
								char buffer[10] = {0};
								pid_t waitres;
								int status;
								bool endPipe=false;
								while (!endPipe) {
									FD_ZERO(&rfds);
									FD_SET(p[0], &rfds);
									select(p[0] + 1, &rfds, NULL, NULL, NULL);
									if(FD_ISSET(p[0], &rfds)) {
										int ret = 0;
										while ((ret = read(p[0], buffer, 10)) > 0) {
											fromOtherApp+=string(buffer,ret);
											//write(1, buffer, ret);
											memset(buffer, 0, 10);
										}
									}
									waitres=waitpid(pid, &status, WNOHANG);
									if (waitres==pid) {
										endPipe=true;
									}
								}
							}
							fsize=fromOtherApp.length();
							string outputS = "HTTP/1.1 200 OK\nContent-Type: "
								+ contentType + "\nContent-Length: "
								+ to_string(fsize) + "\nConnection: keep-alive\n\n";
							fsize+=outputS.length();
							char output[fsize];
							strncpy(output,outputS.c_str(),outputS.length());
							strncpy(output+outputS.length(),fromOtherApp.c_str(),
									fromOtherApp.length());
							SendData(clientsocket, (void*) output, fsize);
						}
						inFile.close();
					}
				} else { notFound=true; }
			} else { notFound=true; }
		}

		// This is just a 404 Not found html message to
		// send back when nothing is found.
		if (notFound) {
			string outData =
				"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
				+ string("<html><head>\n<title>404 Not Found</title>\n</head>")
				+ string("<body>\n<h1>Not Found</h1>\n<p>The requested URL ")
				+ remotePath
				+ " was not found on this server.</p>\n</body></html>";
			string outputS =
					"HTTP/1.1 404 Not Found\nServer: RonsWebServer_1.0\n"
					+ string("Content-Length: ")
					+ to_string(outData.length())
					+"\nConnection: close\nContent-Type: text/html; "
					+"charset=iso-8859-1\n\n";
			outputS += outData;
			SendData(clientsocket, (void*) outputS.c_str(),
					outputS.length());
		}
	}

	shutdown(clientsocket, SHUT_RDWR);
	ClientThreadSockets.erase(id);
	threadData.erase(id);
	ClientThreads.erase(id);
	cout << "Client: " + clientAddress + " disconnected.\n\n";
	return 0;
}

/*This is the accept thread that only listens for new connections and
 * then spawns a new socket and thread for that one particular client.
 * I use a stack of threads in a vector and a stack of socket numbers
 * also to keep track.
 */
void *AcceptThread(void *) {

	int idIndex=0;
	fflush(stdout);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(portNumber);
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
	listen(sockfd, 10);

	/*This thread never ends until the main thread calls:
	 * pthread_cancel on it.
	 */
	while (true) {
		int clientsocketA = 0;
		unsigned int len = sizeof(clientaddr);
		clientsocketA = accept(sockfd, (struct sockaddr*) &clientaddr,
				&len);


		/*print out the ip of the client that just connected to
		 * the server's console.
		 */
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientaddr.sin_addr), str,
		INET_ADDRSTRLEN);
		string clientAddress = string(str);
		cout << "Connected to client: " + clientAddress + "\n";

		/*push the new thread and socket on to their respective stacks*/
		ClientThreadSockets[idIndex]=clientsocketA;
		ClientThreads[idIndex]=emptyThread;
		ClientAddressSocket cas;
		cas.clientAddress = clientAddress;
		cas.socket=ClientThreadSockets[idIndex];
		cas.id=idIndex++;
		threadData[idIndex]=cas;
		pthread_create(&ClientThreads[idIndex], NULL, ClientSession,
				(void *) &threadData[idIndex]);
		idIndex=((idIndex+1)%8192);
	}
	return 0;
}

int main() {

	PopulateExtensions();

	portNumber = 8080;
	pthread_t DoAcceptThread;
	pthread_create(&DoAcceptThread, NULL, AcceptThread, 0);

	cout << "Waiting on port: " << portNumber << "\n";

	unique_lock<mutex> lk(crmutex);
	cond_var.wait(lk, [] {return mainThreadEnding;});

	cout << "\nmain() got to here\n";

	pthread_cancel(DoAcceptThread);
	for (auto ct : ClientThreads) {
		/*pthread_join(ct, &status);*/
		pthread_cancel(ct.second);
	}

	/*Shut down every socket smoothly.  Every socket the program has
	 * created since it started running.
	 */
	for (auto cs : ClientThreadSockets) {
		shutdown(cs.second, SHUT_RDWR);
	}
	shutdown(sockfd,SHUT_RDWR);

	return 0;
}

void PopulateExtensions() {
	typeExt["html"]="text/html";
	typeExt["htm"]="text/html";
	typeExt["jpg"]="image/jpeg";
	typeExt["jpeg"]="image/jpeg";
	typeExt["png"]="image/png";
	typeExt["ico"]="image/x-icon";
	typeExt["gif"]="image/gif";
	typeExt["txt"]="text/plain";
	typeExt["cpp"]="text/plain";
	typeExt["c"]="text/plain";
	typeExt["java"]="text/plain";
	typeExt["pl"]="text/plain";
	typeExt["css"]="text/css";
	typeExt["php"]="text/php";
	otherApps["rb"]="ruby";
	otherApps["pl"]="perl";
	otherApps["py"]="python";
	otherApps["php"]="php-cgi";
}

