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
	int type;
	int layer;
	int line;
	wstring wlayer;
	wstring cond;
};
class filter
{
public:
	filter();
	~filter();

	bool pre_load();
	bool post_load();
	bool userdic_load();
	bool skiplayer_load();
	bool pre();
	bool post();

	vector<FILTERSTRUCT> PreFilter;
	vector<FILTERSTRUCT> PostFilter;
	vector<USERDICSTRUCT> UserDic;
	vector<SKIPLAYERSTRUCT> SkipLayer;

private:
	bool filter_load(vector<FILTERSTRUCT> &Filter, LPCWSTR lpPath, LPCWSTR lpFileName, int FilterType, int &g_line);
	bool userdic_load2(vector<USERDICSTRUCT> &Filter, LPCWSTR lpPath, LPCWSTR lpFileName, int &g_line);
};