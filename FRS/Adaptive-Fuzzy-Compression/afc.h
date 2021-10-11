
#pragma once

#include <cstdio>
#include <cstlib>
#include <ctime>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <set>
#include <algorithm>
#include <random>
#include <array>


class Data{
	protected:
	size_t m_size;
	std::vector<char> m_data;

	public:
	Data(const std::string& file_path);
	virtual ~Data();
};

class RawData: public Data{
	private:
	;
};

class CompressedData: public Data{};

class RuleBase: public Data{
	private:
	;

	public:
	;
};

class AFC{
	private:
	RawData m_raw_data;
	CompressedData m_compressed_data;
	RuleBase m_rule_base;

	public:
	;
};

