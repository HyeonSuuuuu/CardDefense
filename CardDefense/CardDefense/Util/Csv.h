#pragma once
#include <string>
#include <vector>

// 데이터 CSV 읽기.
// 규칙: 첫 줄(주석 제외)이 열 이름, 쉼표로 구분, '#'로 시작하는 줄과 빈 줄은 무시, 칸 앞뒤 공백 무시.
struct CsvTable
{
	std::vector<std::string> columns;
	std::vector<std::vector<std::string>> rows;
};

// 실패하면 콘솔에 이유를 출력하고 false를 돌려준다.
bool LoadCsv(const char* path, CsvTable& table);

// 열 이름의 위치. 없으면 -1.
int CsvColumn(const CsvTable& table, const char* name);

// 사용 예:
//   CsvTable t;
//   if (LoadCsv("Data/mobs.csv", t)) {
//       int hp = CsvColumn(t, "hp_scale");
//       for (auto& row : t.rows) { float v = std::stof(row[hp]); ... }
//   }
