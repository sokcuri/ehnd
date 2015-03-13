#pragma once
struct FILTERSTRUCT
{
	int g_line;
	int line;
	wstring src;
	wstring dest;
	int layer;
	int regex;
	wstring db;
	bool unicode;
	int operator<(FILTERSTRUCT fs) { return (layer) < (fs.layer) || ((layer) == (fs.layer) && (g_line < fs.g_line)); }
};
struct USERDICSTRUCT
{
	int line;
	char hidden;
	char jpn[31];
	char kor[31];
	char part[5];
	char attr[42];
	int operator<(USERDICSTRUCT uds) { return ((strcmp(jpn, uds.jpn) > 0) || (strcmp(jpn, uds.jpn) == 0) && (line < uds.line)); }
};
struct SKIPLAYERSTRUCT
{
	wstring wtype;
	int type;
	int layer;
	int g_line;
	int line;
	wstring wlayer;
	wstring cond;
	int operator<(SKIPLAYERSTRUCT ss) { return (layer) < (ss.layer) || ((layer) == (ss.layer) && (g_line < ss.g_line)); }
};
class filter
{
public:
	filter();
	~filter();

	bool load();
	bool load_dic();

	bool pre_load();
	bool post_load();
	bool userdic_load();
	bool userdic_anedic_load();
	bool jkdic_load();
	bool skiplayer_load();
	bool ehnddic_cleanup();
	bool ehnddic_create();
	bool anedic_load();
	bool pre(wstring &wsText);
	bool post(wstring &wsText);
	bool cmd(wstring &wsText);

private:
	bool skiplayer_load2(vector<SKIPLAYERSTRUCT> &SkipLayer, LPCWSTR lpPath, LPCWSTR lpFileName, int &g_line);
	bool filter_load(vector<FILTERSTRUCT> &Filter, LPCWSTR lpPath, LPCWSTR lpFileName, int FilterType, bool IsUnicode, int &g_line);
	bool userdic_load2(LPCWSTR lpPath, LPCWSTR lpFileName, int &g_line);
	bool filter_proc(vector<FILTERSTRUCT> &Filter, const int FilterType, wstring &wsText);

	vector<FILTERSTRUCT> PreFilter;
	vector<FILTERSTRUCT> PostFilter;
	vector<USERDICSTRUCT> UserDic;
	vector<SKIPLAYERSTRUCT> SkipLayer;
	
	HANDLE hLoadEvent;
};