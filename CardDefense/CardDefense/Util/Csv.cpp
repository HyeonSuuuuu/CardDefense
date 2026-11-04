#include "Csv.h"
#include <cstdio>
#include <fstream>
#include <sstream>

static std::string Trim(const std::string& text)
{
	size_t first = text.find_first_not_of(" \t\r");
	if (first == std::string::npos) {
		return "";
	}
	size_t last = text.find_last_not_of(" \t\r");
	return text.substr(first, last - first + 1);
}

static std::vector<std::string> Split(const std::string& line)
{
	std::vector<std::string> cells;
	std::stringstream ss(line);
	std::string cell;
	while (std::getline(ss, cell, ',')) {
		cells.push_back(Trim(cell));
	}
	if (!line.empty() && line.back() == ',') {
		cells.push_back("");  // 마지막 칸이 빈 경우
	}
	return cells;
}

bool LoadCsv(const char* path, CsvTable& table)
{
	std::ifstream file(path);
	if (!file) {
		printf("CSV 파일을 열 수 없음: %s\n", path);
		return false;
	}

	table = CsvTable{};
	std::string line;
	int lineNumber = 0;
	while (std::getline(file, line)) {
		++lineNumber;
		if (lineNumber == 1 && line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
			line.erase(0, 3);  // UTF-8 BOM (엑셀이 붙임)
		}
		line = Trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}

		std::vector<std::string> cells = Split(line);
		if (table.columns.empty()) {
			table.columns = cells;
			continue;
		}
		if (cells.size() != table.columns.size()) {
			printf("%s %d행: 칸 %d개, 열 이름은 %d개\n", path, lineNumber, (int)cells.size(), (int)table.columns.size());
			return false;
		}
		table.rows.push_back(cells);
	}

	if (table.columns.empty()) {
		printf("열 이름 줄이 없음: %s\n", path);
		return false;
	}
	return true;
}

int CsvColumn(const CsvTable& table, const char* name)
{
	for (int i = 0; i < (int)table.columns.size(); ++i) {
		if (table.columns[i] == name) {
			return i;
		}
	}
	return -1;
}
