#ifndef EASYLOGS_H
#define EASYLOGS_H

#define EL_ERROR 1
#define EL_SYSTEM 2
#define EL_SECURITY 3
#define EL_AUTH 4
#define EL_ACTION 5
#define EL_JUDGE 6
#define EL_NETWORK 7

#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <mutex>

class EasyLogs {
public:
	EasyLogs();
	EasyLogs(std::string name);
	EasyLogs(std::vector<char> data);
	~EasyLogs();

	bool open(std::string name);
	bool open_via_char(std::vector<char> data);
	
	bool is_open();

	bool create(std::string name);
	
	bool save();
	bool save_as(std::string name);

	void close();

	bool select_all(std::vector<char>& vector);
	bool select_all(unsigned char type, std::vector<char>& vector);
	bool select_from(time_t time_from, std::vector<char>& vector);
	bool select_from(unsigned char type, time_t time_from, std::vector<char>& vector);
	bool select_to(time_t time_to, std::vector<char>& vector);
	bool select_to(unsigned char type, time_t time_to, std::vector<char>& vector);
	bool select_from_to(time_t time_from, time_t time_to, std::vector<char>& vector);
	bool select_from_to(unsigned char type, time_t time_from, time_t time_to, std::vector<char>& vector);

	bool insert(const unsigned char& type, const std::string& text);
	bool insert(const unsigned char& type1, const unsigned char& type2, const std::string& text);
	bool insert(const unsigned char& type1, const unsigned char& type2, const unsigned char& type3, const std::string& text);

	void print_all();

private:
	std::mutex data_mutex;

	struct LogNote {
		time_t time = -1;						// время лога
		std::vector<unsigned char> log_types;	// типы лога
		std::string log_text = "";				// текст лога
		uint32_t parent_index = 0;				// расположение в общей памяти
	};
	
	std::string logs_name_;	// имя ллгов

	std::ofstream txt_file_;// текстовый файл логов

	bool is_open_;
	
	//---
	std::vector<LogNote*> AllLogs_data_;

	std::vector<LogNote*> ErrorLogs_;
	std::vector<LogNote*> SystemLogs_;
	std::vector<LogNote*> SecurityLogs_;
	std::vector<LogNote*> AuthLogs_;
	std::vector<LogNote*> ActionLogs_;
	std::vector<LogNote*> JudgeLogs_;
	std::vector<LogNote*> NetworkLogs_;
	//---

	bool ReadFromFile();
	void __read_other_logs__(std::ifstream& file, const std::vector<LogNote*> main_vector, std::vector<LogNote*>& other_vector);

	bool SaveToFile();

	bool GetCharAllData(std::vector<char>& vector);
	void __get_char_other__(std::vector<char>& vector, const std::vector<LogNote*>& other_vector);

	bool OpenViaCharData(const std::vector<char>& vector);
	void __open_via_char__(const std::vector<char>& vector, uint32_t& data_index, const std::vector<LogNote*> main_vector, std::vector<LogNote*>& other_vector);

	bool AddLogBack(std::vector<unsigned char> types, std::string text);
	
	void Clear();
};


#endif