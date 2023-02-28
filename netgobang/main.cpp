#include <WinSock2.h>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib")
using namespace std;
const int PORT = 8000;
#define MaxClient 10
#define MaxBufSize 1024
#define _CRT_SECURE_NO_WARINGS
int uid = 0;
int rid = 0;
typedef struct {
	int use;
	string RoomName;
	SOCKET player[2];
	int ready1;
	int ready2;
	char map[19][19];
	int rid;
	int now;
} ROOMS;
typedef struct {
	int uid;
	SOCKET ns;
	std::string usr;
	std::string pass;
} Register;
vector<SOCKET> client;
vector<Register> usr;
vector<ROOMS> rooms;
void logout(SOCKET sk) {
	// 从client删除
	for (int i = 0; i < client.size(); i++) {
		if (client[i] == sk) {
			client[i] = -1;
			break;
		}
	}
	for (int i = 0; i < rooms.size(); i++) {
		for (int j = 0; j < 2; j++) {
			if (rooms[i].player[j] == sk) {
				rooms[i].player[j] = -1;
			}
		}
		if (rooms[i].player[0] == -1 && rooms[i].player[1] == -1) {
			rooms[i].use = 0;
		}
	}
	for (int i = 0; i < usr.size(); i++) {
		if (usr[i].ns == sk) {
			usr[i].ns = -1;
		}
	}
}
vector<string> spilt(string str, string delimiter) {
	vector<string> s;
	int index(0);
	while (str.find(delimiter) != string::npos) {
		index = str.find(delimiter);
		s.push_back(str.substr(0, index));
		str = str.substr(index + 1);
		if (str.find(delimiter) == string::npos) {
			s.push_back(str);
			return s;
		}
	}
	s.push_back(str);
	return s;
}
int findSame(Register r) {
	for (int i = 0; i < usr.size(); i++) {
		if (r.usr == usr[i].usr) {
			//printf("a");
			return i;
		} else {
			//cout << "NO:\n";
			//cout <<i <<" " <<r.usr << " " << usr[i].usr << endl;
		}
	}
	return -1;
}
bool reg(Register r) {
	if (findSame(r) != -1) {
		return false;
	}
	r.uid = uid++;
	usr.push_back(r);
	r.ns = -1;
	return true;
}

bool log(SOCKET sk, Register r) {
	if (findSame(r) != -1) {
		Register right = usr[findSame(r)];
		cout << "log :" << r.usr << " " << r.pass << endl;
		cout << "log r:" << right.usr << " " << right.pass << endl;
		if (r.usr == right.usr && r.pass == right.pass) {
			client.push_back(sk);
			usr[findSame(r)].ns = sk;
			printf("OK\n");
			return true;
		} else {
			printf("NOK\n");
			return false;
		}
	}
	return false;
}
string lpchar2String(char* str) {
	string s = str;
	return s;
}
int GetUID(SOCKET sk) {
	for (int i = 0; i < usr.size(); i++) {
		if (usr[i].ns == sk) {
			return usr[i].uid;
		}
	}
	return 0;
}

Register GetPlayerFromUID(int uid) {
	for (int i = 0; i < usr.size(); i++) {
		if (usr[i].uid == uid) {
			return usr[i];
		}
	}
}
Register GetPlayerFromSocket(SOCKET sk) {
	for (int i = 0; i < usr.size(); i++) {
		if (usr[i].ns == sk) {
			return usr[i];
		}
	}
}
void CreateROOM(SOCKET sk, string RoomName) {
	ROOMS rs;
	rs.RoomName = RoomName;
	rs.rid = rid++;
	rs.ready1 = 0;
	rs.ready2 = 0;
	rs.player[0] = sk;
	rs.player[1] = -1;
	for (int i = 0; i < 19; i++) {
		for (int j = 0; j < 19; j++) {
			rs.map[i][j] = 0;
		}
	}
	rs.now = 0;
	rs.use = 1;
	rooms.push_back(rs);
}

int Win(ROOMS rm) {

	int i, j;
	for (i = 0; i < 19; i++) {
		for (j = 0; j < 19; j++) {
			int w_count = 0, b_count = 0;
			int k, l;
			for (k = 0; k < 5; k++) {
				if (rm.map[i][j] == 1) {
					if (rm.map[i + k][j] == 1)
						b_count++;
					else
						break;
				} else if (rm.map[i][j] == 2) {
					if (rm.map[i + k][j] == 2)
						w_count++;
					else
						break;
				}
			}
			if (b_count == 5)
				return 1;
			else if (w_count == 5)
				return 2;
			else {
				b_count = 0;
				w_count = 0;
			}
			for (l = 0; l < 5; l++) {
				if (rm.map[i][j] == 1) {
					if (rm.map[i][j + l] == 1)
						b_count++;
					else
						break;
				} else if (rm.map[i][j] == 2) {
					if (rm.map[i][j + l] == 2)
						w_count++;
					else
						break;
				}
			}
			if (b_count == 5)
				return 1;
			else if (w_count == 5)
				return 2;
			else {
				b_count = 0;
				w_count = 0;
			}
			for (k = 0; k < 5; k++) {
				if (rm.map[i][j] == 1) {
					if (rm.map[i + k][j + k] == 1)
						b_count++;
					else
						break;
				} else if (rm.map[i][j] == 2) {
					if (rm.map[i + k][j + k] == 2)
						w_count++;
					else
						break;
				}
			}
			if (b_count == 5)
				return 1;
			else if (w_count == 5)
				return 2;
			else {
				b_count = 0;
				w_count = 0;
			}
			for (k = 0; k < 5; k++) {
				if (rm.map[i][j] == 1) {
					if (rm.map[i - k][j + k] == 1)
						b_count++;
					else
						break;
				} else if (rm.map[i][j] == 2) {
					if (rm.map[i - k][j + k] == 2)
						w_count++;
					else
						break;
				}
			}
			if (b_count == 5)
				return 1;
			else if (w_count == 5)
				return 2;
			else {
				b_count = 0;
				w_count = 0;
			}

		}
	}
	return 0;
}
DWORD WINAPI ServerThread(LPVOID lpParameter) {
	SOCKET* ClientSocket = (SOCKET*)lpParameter;
	int receByt = 0;
	char RecvBuf[MaxBufSize];
	char SendBuf[MaxBufSize];
	while (1) {
		memset(RecvBuf, 0, sizeof(RecvBuf));
		memset(SendBuf, 0, sizeof(SendBuf));
		receByt = recv(*ClientSocket, RecvBuf, sizeof(RecvBuf), 0);
		// buf[receByt]='';
		RecvBuf[receByt] = 0;
		if (receByt > 0) {
			if (strcmp(RecvBuf, "TEST") == 0) {
				printf("A Client Send TEST Command.\n");
				strcpy(SendBuf, "OK");
				send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
			} else if (strcmp(RecvBuf, "GETUID") == 0) {
				sprintf(SendBuf, "%d", GetUID(*ClientSocket));
				send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
			} else if (strcmp(RecvBuf, "RMLS") == 0) {
				string res = "";
				for (int i = 0; i < rooms.size(); i++) {
					if (rooms[i].use) {
						res.append(to_string(rooms[i].rid));
						res.append(" ");
						res.append(rooms[i].RoomName);
						res.append(",");
					}

				}
				sprintf(SendBuf, "%s", res.c_str());
				send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
			} else if (strcmp(RecvBuf, "EXIT") == 0) {
				printf("一个玩家想要退出房间\n");
				for (int i = 0; i < rooms.size(); i++) {
					if (rooms[i].player[0] == *ClientSocket) {
						rooms[i].player[0] = rooms[i].player[1];
						rooms[i].player[1] = -1;
						rooms[i].now = 0;
						rooms[i].ready1 = 0;
						rooms[i].ready2 = 0;
						sprintf(SendBuf, "War:001");
						send(rooms[i].player[0], SendBuf, sizeof(SendBuf), 0);
						memset(SendBuf, 0, sizeof(SendBuf));
						strcpy(SendBuf, "OK");
						send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);

					} else if (rooms[i].player[1] == *ClientSocket) {
						rooms[i].player[1] = -1;
						rooms[i].now = 0;
						rooms[i].ready1 = 0;
						rooms[i].ready2 = 0;
						sprintf(SendBuf, "War:001");
						send(rooms[i].player[0], SendBuf, sizeof(SendBuf), 0);
						memset(SendBuf, 0, sizeof(SendBuf));
						strcpy(SendBuf, "OK");
						send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
					}
					if (rooms[i].player[0] == -1 && rooms[i].player[1] == -1) {
						rooms[i].use = 0;
					}
				}
			} else if (strcmp(RecvBuf, "R") == 0) {
				for (int i = 0; i < rooms.size(); i++) {
					if (rooms[i].player[0] == *ClientSocket) {
						rooms[i].ready1 = 1;
						if (rooms[i].ready1 + rooms[i].ready2 == 2) {
							strcpy(SendBuf, "OK");
							send(rooms[i].player[0], SendBuf, sizeof(SendBuf), 0);
							send(rooms[i].player[1], SendBuf, sizeof(SendBuf), 0);
						}

					} else if (rooms[i].player[1] == *ClientSocket) {
						rooms[i].ready2 = 1;
						if (rooms[i].ready1 + rooms[i].ready2 == 2) {
							strcpy(SendBuf, "OK");
							send(rooms[i].player[0], SendBuf, sizeof(SendBuf), 0);
							send(rooms[i].player[1], SendBuf, sizeof(SendBuf), 0);
						}
					}
				}
			} else {
				vector<string> sv;
				sv = spilt(lpchar2String(RecvBuf), " ");
				if (sv[0] == "REG") {
					Register r;
					r.usr = sv[1];
					r.pass = sv[2];
					cout << "注册：" << r.usr << " " << r.pass << endl;
					if (reg(r)) {

						strcpy(SendBuf, "OK");
						send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
					} else {
						strcpy(SendBuf, "Err:001");
						send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
					}
				} else if (sv[0] == "LOG") {
					Register r;
					r.usr = sv[1];
					r.pass = sv[2];
					if (log(*ClientSocket, r)) {
						strcpy(SendBuf, "OK");
						send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
					} else {
						strcpy(SendBuf, "Err:002");
						send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
					}
				} else if (sv[0] == "CRT") {
					printf("创建房间\n");
					CreateROOM(*ClientSocket, sv[1]);
					strcpy(SendBuf, "0");
					send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				} else if (sv[0] == "GETPL") {
					printf("GETPL-->\n");
					Register r = GetPlayerFromUID(atoi(sv[1].c_str()));
					sprintf(SendBuf, "%s %d", r.usr.c_str(), r.uid);
					send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				} else if (sv[0] == "IN") {
					for (int i = 0; i < rooms.size(); i++) {
						if (rooms[i].rid == atoi(sv[i].c_str())) {
							if (rooms[i].player[1] == -1) {
								rooms[i].player[1] = *ClientSocket;
								for (int a = 0; a < 19; a++) {
									for (int b = 0; b < 19; b++) {
										rooms[i].map[a][b] = 0;
									}
								}
								sprintf(SendBuf, "1 %d", GetPlayerFromSocket(rooms[i].player[0]).uid);
								send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
								memset(SendBuf, 0, sizeof(SendBuf));
								sprintf(SendBuf, "%d", GetPlayerFromSocket(rooms[i].player[1]).uid);
								send(rooms[i].player[0], SendBuf, sizeof(SendBuf), 0);
								continue;
							} else {
								strcpy(SendBuf, "Err:003");
								send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
								continue;
							}
						}
					}
					strcpy(SendBuf, "Err:404");
					send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
				} else if (sv[0] == "U") {
					printf("落子\n");
					int i;
					int x, y;
					x = atoi(sv[1].c_str());
					y = atoi(sv[2].c_str());
					printf("X:%d,Y:%d\n", x, y);
					for (i = 0; i < rooms.size(); i++) {
						if (rooms[i].ready1 + rooms[i].ready2 != 2) {
							if (rooms[i].player[0] == *ClientSocket || rooms[i].player[1] == *ClientSocket) {
								strcpy(SendBuf, "Err:007");
								send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
							}
							continue;
						}
						if (rooms[i].player[rooms[i].now] == *ClientSocket) {
							if (rooms[i].map[x][y] == 0) {
								rooms[i].map[x][y] = rooms[i].now + 1;

								int r = Win(rooms[i]);
								if (r != 0) {
									printf("A Win\n");
									int uid = GetPlayerFromSocket(rooms[i].player[r - 1]).uid;
									rooms[i].now = 0;
									rooms[i].ready1 = 0;
									rooms[i].ready2 = 0;
									for (int k = 0; k < 19; k++) {
										for (int j = 0; j < 19; j++) {
											rooms[i].map[k][j] = 0;
										}
									}
									sprintf(SendBuf, "WIN %d", uid);
									send(rooms[i].player[0], SendBuf, sizeof(SendBuf), 0);
									send(rooms[i].player[1], SendBuf, sizeof(SendBuf), 0);
								} else {
									sprintf(SendBuf, "Y %d %d", x, y);
									printf("(!(rooms[i].now))=%d\n", (!(rooms[i].now)));
									send(rooms[i].player[(!(rooms[i].now))], SendBuf, sizeof(SendBuf), 0);
									rooms[i].now = !rooms[i].now;
								}
							} else {
								strcpy(SendBuf, "Err:005");
								send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);

							}
						} else if (rooms[i].player[(!rooms[i].now)] == *ClientSocket) {
							strcpy(SendBuf, "Err:004");
							send(*ClientSocket, SendBuf, sizeof(SendBuf), 0);
						}
					}

				}
			}
		} else {
			cout << "END" << endl;
			logout(*ClientSocket);
			break;
		}
	}  // while
	closesocket(*ClientSocket);
	free(ClientSocket);
	return 0;
}

int main() {
	WSAData wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN ListenAddr;
	ListenAddr.sin_family = AF_INET;
	ListenAddr.sin_addr.S_un.S_addr = INADDR_ANY;  //表示填入本机ip
	ListenAddr.sin_port = htons(PORT);
	int n;
	n = bind(ListenSocket, (LPSOCKADDR)&ListenAddr, sizeof(ListenAddr));
	if (n == SOCKET_ERROR) {
		cout << "Bind failed" << endl;
		return -1;
	} else {
		cout << "OK" << PORT << endl;
	}
	int l = listen(ListenSocket, 20);
	cout << "Server OK" << endl;

	while (1) {
		//循环接收客户端连接请求并创建服务线程
		SOCKET* ClientSocket = new SOCKET;
		ClientSocket = (SOCKET*)malloc(sizeof(SOCKET));
		//接收客户端连接请求
		int SockAddrlen = sizeof(sockaddr);
		*ClientSocket = accept(ListenSocket, 0, 0);
		cout << "A Client:" << *ClientSocket << endl;
		CreateThread(NULL, 0, &ServerThread, ClientSocket, 0, NULL);
	}  // while
	closesocket(ListenSocket);
	WSACleanup();
	return (0);
}  // main

