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
class filter
{
public:
	filter();
	~filter();
	bool pre_load();
	bool post_load();
	bool pre();
	bool post();

	vector<FILTERSTRUCT> PreFilter;
	vector<FILTERSTRUCT> PostFilter;
private:
	bool filter_load(vector<FILTERSTRUCT> &Filter, LPCWSTR lpPath, LPCWSTR lpFileName, int FilterType, int &g_line);
};