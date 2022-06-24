#include <cstdio>
#include <iostream>
#include <time.h>
#include "lib/winini/winini.h"
#include <fstream>
#include "usersystem.h"
#include "lib/sha256/sha256.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#elif defined(__linux__)
//#error
#endif
#if !(defined(ASKII_GS) || defined(ASKII_RS) || defined(ASKII_US))
#define ASKII_GS 0x1D
#define ASKII_RS 0x1E
#define ASKII_US 0x1F
#else
#error
#endif

using namespace std;

//============================================ In Memory =============================================

typedef std::uint64_t hash_t;

constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;

/*char* U2G(const char* utf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}*/




#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
string _stdcall usersys_hash_cookie(string name, string password) {
#elif defined(__linux__)
string __attribute__((__stdcall__)) usersys_hash_cookie(string name, string password) {
#endif
	int logTime = clock() * (clock() % 10);
	name = name + to_string(logTime) + password;

	SHA256 sha;
	sha.update(name);
	uint8_t* digest = sha.digest();

	//std::cout << SHA256::toString(digest) << std::endl;
	
	//delete[] digest; // Don't forget to free the digest!
	return SHA256::toString(digest);
}
	
struct User {
	/*
	*level: 0==guest,1==user,2==admin,3==root
	* User can not use this website if tis able == false;
	*/
	string name = "";//Contains a maximum of 20 characters
	string username = "";
	string password = "";
	int ID = 0;
	string cookie = "";
	long long time_str_creation_cookie = 0;
	bool able = false;
	int level = -1;
	bool logined = false;
	string time_login = "";
	int offset = 0;
	void Init_readIn(
		int ID,
		string name,
		int level,
		string password,
		string cookie,
		int creation_time_cookie,
		bool able
	) {
		this->cookie = cookie;
		this->time_str_creation_cookie = creation_time_cookie;
		this->ID = ID;
		this->name = name;
		this->password = password;
		this->level = level;
	}
};

struct treeNode {
	treeNode* leftChild = NULL, * rightChild = NULL;
	int left = 0, right = 0, heavy = 0, offset = 0;
	User* member = NULL;
};

/*int usersys_login_nameAndPassword(string* password) {
	
}*/

User* usersys_find_node(int ID, treeNode* thisone) {
	if (thisone->left == thisone->right) {
		return thisone->member;
	}
	else if (ID >= thisone->leftChild->left && ID <= thisone->leftChild->right) {
		return usersys_find_node(ID, thisone->leftChild);
	}
	else if (ID >= thisone->rightChild->left && ID <= thisone->rightChild->right) {
		return usersys_find_node(ID, thisone->rightChild);
	}
	return NULL;
}

/*bool usersys_login_cookie(int ID, string* cookie, treeNode* userTree) {
	time_t tv;
	tv = time(NULL);
	User* user = usersys_find_node(ID, userTree);
	if (*cookie == user->cookie && user->able && user->time_num_creation_cookie <= tv) {
		user->logined = true;
		return true;
	}
	else if (user->time_num_creation_cookie > tv) {
		user->cookie = "";
	}
	return false;
}*/

void usersys_login_SysAPI(int ID, treeNode* userTree) {
	User* user = usersys_find_node(ID, userTree);
	user->logined = true;
}

string usersys_login_IDAndPassword(int ID, string pass, treeNode* userTree) {
	User* user = usersys_find_node(ID, userTree);
	/*If the funcution returns 0,name or password can't be matched.
	* If -1 is returned ,the user has been disabled.
	* If 1 is returned ,login is allowed.
	* If 2 is returned ,the user has logined.
	*/
	if (user->able == true) {
		if (user->logined == true) {
			return "Has logined";
		}
		else if (user->password == pass){
			return "Succeed";
		}
		else {
			return "Failed";
		}
	}
	else {
		return "Fefused";
	}
}

void usersys_new_node(int left, int right, treeNode* thisone) {
	//cout << left << ' ' << right << endl;
	thisone->left = left;
	thisone->right = right;
	if (left < right) {
		thisone->leftChild = new treeNode;
		thisone->rightChild = new treeNode;
		thisone->member = NULL;
		
		usersys_new_node(left, (left + right) / 2, thisone->leftChild);
		usersys_new_node((left + right) / 2 + 1, right, thisone->rightChild);
	}else{
		thisone->member = new User;
		thisone->leftChild = thisone->rightChild = NULL;
	}
}



treeNode* usersys_newTree(int size) {
	treeNode *user = new treeNode;
	usersys_new_node(1, size, user);
	return user;
}
	

treeNode* usersys_Init() {
	
	//FILE* stream1;
	//freopen_s(&stream1, "userinfo.in", "r", stdin);
	int number;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	//scanf_s("Number:%d,,,,,,", &number);
#endif
	//scanf("Number:%d,,,,,,", &number);
	
	treeNode* userTree;
	userTree = usersys_newTree(number % 2 == 0 ? number : number + 1);
	for (int i = 1; i <= number; i++) {
		string ablee = {};
		int ID = 0;
		string ID_str = {};
		string name = {};
		int level = 0;
		string password = {};
		string cookie = {};
		int creation_time_cookie = 0;
		string creation_time_cookie_str = {};
		bool able = false;
		string s = {};
		cin >> s;
		int j = 0;
		while (s[j] != ',') {
			ID_str += s[j];
			j++;
		}
		j++;
		while (s[j] != ',') {
			name += s[j];
			j++;
		}
		j++;
		level = s[j] - '0';
		j+=2;
		while (s[j] != ',') {
			password += s[j];
			j++;
		}
		j++;
		while (s[j] != ',') {
			cookie += s[j];
			j++;
		}
		j++;
		while (s[j] != ',') {
			creation_time_cookie_str += s[j];
			j++;
		}
		int* x = new int;
		*x = 1;
		for (int k = creation_time_cookie_str.length() - 1; k >= 0; k--) {
			creation_time_cookie += creation_time_cookie_str[k] * (*x);
			*x *= 10;
		}
		delete x;
		j++;
		while (j < s.length()) {
			ablee += s[j];
			j++;
		}
			if (ablee == "TRUE") {
			able = true;
		}
		else if (ablee == "FALSE") {
			able = false;
		}
		else {
			cout << "database/userinfo.dba:ERROR Line" << i + 1 << ":D001" <<ablee<< endl;
			continue;
		}
		User* adding = usersys_find_node(i, userTree);
		adding->Init_readIn(i, name, level, password, cookie, creation_time_cookie, able);
		adding->cookie.resize(MAXLEN_COOKIE);
		adding->name.resize(MAXLEN_NAME);
		adding->password.resize(MAXLEN_PASSWORD);
		adding->username.resize(MAXLEN_USERNAME);
		cout << adding->ID << ' ' << adding->name << ' ' << adding->level << endl;
		//time_t timer = time(NULL);
		//cout << timer << endl;
		//delete adding;
	}
	//fclose(stdin);
	return userTree;
}

//============================================ In Disc =============================================

struct lineNode {
	lineNode* next = NULL;
	treeNode* member = NULL;
	//int offset = 0;
};

void DFS1(treeNode* node) {// This function will tongji zishu de zongzhong!!!!
	if (node->leftChild != NULL) {
		DFS1(node->leftChild);
		node->heavy += node->leftChild->heavy + 1;
	}
	if (node->rightChild != NULL) {
		DFS1(node->rightChild);
		node->heavy += node->rightChild->heavy + 1;
	}
}

void DFS2(treeNode* treenode, lineNode* &linenode, int &offset) {
	//linenode->offset = offset;
	treenode->offset = offset;
	offset += 8 + 8 + 4 + 4 + 8 + 1;//three offsets and two numbers and "1D(16)";
	lineNode* nextlinenode = new lineNode;
	linenode->member = treenode;
	linenode->next = nextlinenode;
	if (treenode->leftChild == NULL && treenode->rightChild == NULL) {
		delete linenode->next;//offset
		linenode->member->member->offset = offset;
		offset += linenode->member->member->cookie.length() \
			+ linenode->member->member->name.length() \
			+ linenode->member->member->password.length() \
			+ linenode->member->member->username.length() \
			+ linenode->member->member->time_login.length();
		//two bool and two int and six string and 1D(16);
		return;
	}
	else {
		linenode = linenode->next;
		if (treenode->leftChild == NULL) {
			DFS2(treenode->rightChild, linenode, offset);
			return;
		}
		else if (treenode->rightChild == NULL) {
			DFS2(treenode->leftChild, linenode, offset);
			return;
		}
		else if (treenode->leftChild->heavy >= treenode->rightChild->heavy) {
			DFS2(treenode->leftChild, linenode, offset);
			lineNode* nextlinenode2 = new lineNode;
			linenode->next = nextlinenode2;
			linenode = linenode->next;
			DFS2(treenode->rightChild, linenode, offset);
		}
		else {
			DFS2(treenode->rightChild, linenode, offset);
			lineNode* nextlinenode2 = new lineNode;
			linenode->next = nextlinenode2;
			linenode = linenode->next;
			DFS2(treenode->leftChild, linenode, offset);
		}
	}
}


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
lineNode* _stdcall Heavy_path_decomposition(treeNode* Node) {
#elif defined(__linux__)
lineNode* __attribute__((__stdcall__)) Heavy_path_decomposition(treeNode * Node) {
#endif

	lineNode* line = new lineNode;
	lineNode* line_ret = line;
	int offset = 0;
	DFS1(Node);
	DFS2(Node, line, offset);
	return line_ret;
}

string intToString(int a) {
	string b = "";
	for (int i = 0; a != 0; i++) {
		b.resize(b.length() + 1);
		b[i] = '0' + a % 10;
		a /= 10;
	}
	string c = "";
	for (int i = b.length()-1; i >= 0; i--) {
		c += b[i];
	}
	return c;
}



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
bool usersys_writeWholeTreeToTheHardDrive(treeNode *usertree) {
	/*
	
	*/
	int size = usertree->right - usertree->left + 1;
	lineNode* line = Heavy_path_decomposition(usertree);
	CMyINI* ini = new CMyINI();
	ini->ReadINI("./regedit.ini");
	string db_src = ini->GetValue("database", "userinfo01");
	ofstream outFile("./Database/userinfo01-out.dba", ios::out | ios::binary);
	lineNode* line2 = line;
	int offset = 0;
	//outFile.open(db_src.c_str(), ios::out | ios::binary);
	outFile.seekp(ios::beg);
	//outFile.write((char*)line, sizeof(line));
	string writearray = "Number ";
	writearray[writearray.length() - 1] = ASKII_RS;
	writearray += to_string(abs(line->member->left - line->member->right) + 1) + " ";
	writearray[writearray.length() - 1] = ASKII_US;
	/*
		4E 75 6D 62 65 72 3A 34 35 1F
		N  u  m  b  e  r  RS 4  5  US
	*/
	outFile.write(writearray.c_str(), writearray.length());
	return true;
}

#elif(__linux__)
//#error
#endif

int main() {
	treeNode* s = usersys_Init();
	string n = "tommyzhang";
	string p = "Zyt7074061026?";
	while(1)
	cout << usersys_hash_cookie(n, p);
	//usersys_writeWholeTreeToTheHardDrive(s);
	//CMyINI* ini = new CMyINI();
	//ini->ReadINI("D:/Code/C++/Projects/Network/User System/regedit.ini");
	//string db_src = ini->GetValue("database", "userinfo01");
	//ofstream outFile("D:/Code/C++/Projects/Network/User System/Database/userinfo01.dba", ios::out | ios::binary);
	//outFile.seekp(ios::beg);
	string kkk = "jigoerugrty";
	//outFile.write(kkk.c_str(), kkk.length());
	return 0;
}
