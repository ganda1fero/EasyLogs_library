#include "EasyLogs.h"

// public 

EasyLogs::~EasyLogs() {
	Clear();
	txt_file_.close();
}

bool EasyLogs::open(std::string name) {
	txt_file_.close();

	logs_name_ = name;

	if (ReadFromFile() == false) {
		logs_name_.clear();
		return false;
	}

	txt_file_.open(logs_name_ + ".elt", std::ios::out | std::ios::app);	// открыли для записи (текст)

	if (txt_file_.is_open() == false) 
		return false;

	return true;
}

bool EasyLogs::save() {
	if (logs_name_.empty())
		return false;

	SaveToFile();

	return true;
}

void EasyLogs::close() {
	txt_file_.close();

	logs_name_.clear();

	Clear();
}

bool EasyLogs::insert(const unsigned char& type, const std::string& text) {
	return AddLogBack({ type }, text);
}

bool EasyLogs::insert(const unsigned char& type1, const unsigned char& type2, const std::string& text) {
	return AddLogBack({ type1, type2 }, text);
}

bool EasyLogs::insert(const unsigned char& type1, const unsigned char& type2, const unsigned char& type3, const std::string& text) {
	return AddLogBack({ type1, type2, type3 }, text);
}

// private

bool EasyLogs::ReadFromFile() {
	if (logs_name_.empty())
		return false;

	std::ifstream file(logs_name_ + ".elb", std::ios::binary | std::ios::in);
	if (file.is_open() == false)
		return false;

	// временные переменные
	uint32_t uint32_t_buffer;
	time_t time_t_buffer;
	unsigned char uchar_buffer;

	std::vector<LogNote*> AllLogs_data_;

	std::vector<LogNote*> ErrorLogs_;
	std::vector<LogNote*> SystemLogs_;
	std::vector<LogNote*> SecurityLogs_;
	std::vector<LogNote*> AuthLogs_;
	std::vector<LogNote*> ActionLogs_;
	std::vector<LogNote*> JudgeLogs_;
	std::vector<LogNote*> NetworkLogs_;

	// само чтение с проверкой
	try {
		file.read(reinterpret_cast<char*>(&uint32_t_buffer), sizeof(uint32_t_buffer));
		AllLogs_data_.reserve(uint32_t_buffer * 2);	// зарезервировали X2 памяти
		AllLogs_data_.resize(uint32_t_buffer);

		for (uint32_t i{ 0 }; i < AllLogs_data_.size(); i++) {	// чтение главных данных
			AllLogs_data_[i] = new LogNote;	// создали под указателем

			AllLogs_data_[i]->parent_index = i;	// восстановили логически

			file.read(reinterpret_cast<char*>(&time_t_buffer), sizeof(time_t_buffer));
			AllLogs_data_[i]->time = time_t_buffer;

			file.read(reinterpret_cast<char*>(&uint32_t_buffer), sizeof(uint32_t_buffer));
			AllLogs_data_[i]->log_types.resize(uint32_t_buffer);

			for (uint32_t g{ 0 }; g < AllLogs_data_[i]->log_types.size(); g++) {
				file.read(reinterpret_cast<char*>(&uchar_buffer), sizeof(uchar_buffer));
				AllLogs_data_[i]->log_types[g] = uchar_buffer;
			}

			file.read(reinterpret_cast<char*>(&uint32_t_buffer), sizeof(uint32_t_buffer));
			AllLogs_data_[i]->log_text.resize(uint32_t_buffer);
			
			if (uint32_t_buffer > 0)
				file.read(&AllLogs_data_[i]->log_text[0], uint32_t_buffer);
		}

		// Для ErrorLogs_
		__read_other_logs__(file, AllLogs_data_, ErrorLogs_);

		// Для SystemLogs_
		__read_other_logs__(file, AllLogs_data_, SystemLogs_);

		// Для SecurityLogs_
		__read_other_logs__(file, AllLogs_data_, SecurityLogs_);

		// Для AuthLogs_
		__read_other_logs__(file, AllLogs_data_, AuthLogs_);

		// Для ActionLogs_
		__read_other_logs__(file, AllLogs_data_, ActionLogs_);

		// Для JudgeLogs_
		__read_other_logs__(file, AllLogs_data_, JudgeLogs_);

		// Для NetworkLogs_
		__read_other_logs__(file, AllLogs_data_, NetworkLogs_);

		// конец чтения файла
		if (file.good() == false && file.eof() == false) {
			file.close();

			for (uint32_t i{ 0 }; i < AllLogs_data_.size(); i++)
				if (AllLogs_data_[i] != nullptr)
					delete AllLogs_data_[i];	// очищаем то, что успело записаться

			return false;
		}
	}
	catch (...) {
		file.close();

		for (uint32_t i{ 0 }; i < AllLogs_data_.size(); i++)
			if (AllLogs_data_[i] != nullptr)
				delete AllLogs_data_[i];	// очищаем то, что успело записаться

		return false;	// закрываем не изменив изначальные данные
	}

	file.close();

	Clear();	// очистим изначальные данные

	// переносим данные
	data_mutex.lock();
	this->AllLogs_data_ = AllLogs_data_;

	this->ErrorLogs_ = ErrorLogs_;
	this->SystemLogs_ = SystemLogs_;
	this->SecurityLogs_ = SecurityLogs_;
	this->AuthLogs_ = AuthLogs_;
	this->ActionLogs_ = ActionLogs_;
	this->JudgeLogs_ = JudgeLogs_;
	this->NetworkLogs_ = NetworkLogs_;
	data_mutex.unlock();

	return true;
}

void EasyLogs::__read_other_logs__(std::ifstream& file, const std::vector<LogNote*> main_vector, std::vector<LogNote*>& other_vector) {
	uint32_t uint32_t_buffer;

	file.read(reinterpret_cast<char*>(&uint32_t_buffer), sizeof(uint32_t_buffer));
	other_vector.reserve(uint32_t_buffer * 2);
	other_vector.resize(uint32_t_buffer);

	for (uint32_t i{ 0 }; i < other_vector.size(); i++) {
		file.read(reinterpret_cast<char*>(&uint32_t_buffer), sizeof(uint32_t_buffer));
		other_vector[i] = main_vector[uint32_t_buffer];
	}
}

bool EasyLogs::SaveToFile() {
	if (logs_name_.empty())
		return false;

	std::vector<char> save_data;

	if (GetCharAllData(save_data) == false) {
		// не удалось сохранить
		return false;
	}
	// сохраненеие удачно, записываем

	std::ofstream file(logs_name_ + ".elb", std::ios::binary | std::ios::out | std::ios::trunc);
	if (file.is_open() == false)
		return false;

	file.write(&save_data[0], save_data.size());

	file.close();

	return true;
}

bool EasyLogs::GetCharAllData(std::vector<char>& vector) {	// сохраняем все
	// копируем данные (указатели) (чтобы не занимать время)
	data_mutex.lock();
	std::vector<LogNote*> AllLogs_data_ = this->AllLogs_data_;

	std::vector<LogNote*> ErrorLogs_ = this->ErrorLogs_;
	std::vector<LogNote*> SystemLogs_ = this->SystemLogs_;
	std::vector<LogNote*> SecurityLogs_ = this->SecurityLogs_;
	std::vector<LogNote*> AuthLogs_ = this->AuthLogs_;
	std::vector<LogNote*> ActionLogs_ = this->ActionLogs_;
	std::vector<LogNote*> JudgeLogs_ = this->JudgeLogs_;
	std::vector<LogNote*> NetworkLogs_ = this->NetworkLogs_;
	data_mutex.unlock();

	try {
		// дальше работаем со скопированными данными (на момент запроса)
		vector.clear();

		// временные переменные
		uint32_t uint32_t_buffer;
		time_t time_t_buffer;
		unsigned char uchar_buffer;
		char* tmp_ptr;

		// начинаем перенос
		uint32_t_buffer = AllLogs_data_.size();
		tmp_ptr = reinterpret_cast<char*>(&uint32_t_buffer);
		vector.insert(vector.end(), tmp_ptr, tmp_ptr + sizeof(uint32_t_buffer));

		for (uint32_t i{ 0 }; i < AllLogs_data_.size(); i++) {
			time_t_buffer = AllLogs_data_[i]->time;
			tmp_ptr = reinterpret_cast<char*>(&time_t_buffer);
			vector.insert(vector.end(), tmp_ptr, tmp_ptr + sizeof(time_t_buffer));

			uint32_t_buffer = AllLogs_data_[i]->log_types.size();
			tmp_ptr = reinterpret_cast<char*>(&uint32_t_buffer);
			vector.insert(vector.end(), tmp_ptr, tmp_ptr + sizeof(uint32_t_buffer));

			for (uint32_t g{ 0 }; g < AllLogs_data_[i]->log_types.size(); g++) {
				uchar_buffer = AllLogs_data_[i]->log_types[g];
				vector.push_back(*reinterpret_cast<char*>(&uchar_buffer));
			}

			uint32_t_buffer = AllLogs_data_[i]->log_text.length();
			tmp_ptr = reinterpret_cast<char*>(&uint32_t_buffer);
			vector.insert(vector.end(), tmp_ptr, tmp_ptr + sizeof(uint32_t_buffer));

			tmp_ptr = &AllLogs_data_[i]->log_text[0];
			vector.insert(vector.end(), tmp_ptr, tmp_ptr + AllLogs_data_[i]->log_text.length());
		}

		// Для ErrorLogs_
		__get_char_other__(vector, ErrorLogs_);

		// Для SystemLogs_
		__get_char_other__(vector, SystemLogs_);

		// Для SecurityLogs_
		__get_char_other__(vector, SecurityLogs_);

		// Для AuthLogs_
		__get_char_other__(vector, AuthLogs_);

		// Для ActionLogs_
		__get_char_other__(vector, ActionLogs_);

		// Для JudgeLogs_
		__get_char_other__(vector, JudgeLogs_);

		// Для NetworkLogs_
		__get_char_other__(vector, NetworkLogs_);
		
		// конец записи
	}
	catch (...) {
		// значит произошла какая-то ошибка
		vector.clear();

		return false;
	}

	return true;	// значит все данные сохранились в char массив (вектор)
}

void EasyLogs::__get_char_other__(std::vector<char>& vector, const std::vector<LogNote*>& other_vector) {
	uint32_t uint32_t_buffer = other_vector.size();
	char* tmp_ptr = reinterpret_cast<char*>(&uint32_t_buffer);

	vector.insert(vector.end(), tmp_ptr, tmp_ptr + sizeof(uint32_t_buffer));

	for (uint32_t i{ 0 }; i < other_vector.size(); i++) {
		uint32_t_buffer = other_vector[i]->parent_index;
		tmp_ptr = reinterpret_cast<char*>(&uint32_t_buffer);

		vector.insert(vector.end(), tmp_ptr, tmp_ptr + sizeof(uint32_t_buffer));
	}
}