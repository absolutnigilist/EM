#include "ListSerializer.hpp"
#include "List.hpp"

#include <limits>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <glog/logging.h>

namespace {
	
	//---Вспомогательные функции для бинарной записи и чтения примитивных типов

	void writeUint32(std::ofstream& out, std::uint32_t value) {
	
		out.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!out)
		{
			throw std::runtime_error("Failed to write uint32");
		}
	};
	void writeUint64(std::ofstream& out, std::uint64_t value) {
	
		out.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!out)
		{
			throw std::runtime_error("Failed to write uint64");
		}
	};
	void writeInt64(std::ofstream& out, std::int64_t value) {
	
		out.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!out)
		{
			throw std::runtime_error("Failed to write int64");
		}
	}
	std::uint32_t readUint32(std::ifstream& in)
	{
		std::uint32_t value = 0;
		in.read(reinterpret_cast<char*>(&value), sizeof(value));
		if (!in)
		{
			throw std::runtime_error("Failed to read uint32");
		}
		return value;
	}

	std::uint64_t readUint64(std::ifstream& in)
	{
		std::uint64_t value = 0;
		in.read(reinterpret_cast<char*>(&value), sizeof(value));
		if (!in)
		{
			throw std::runtime_error("Failed to read uint64");
		}
		return value;
	}

	std::int64_t readInt64(std::ifstream& in)
	{
		std::int64_t value = 0;
		in.read(reinterpret_cast<char*>(&value), sizeof(value));
		if (!in)
		{
			throw std::runtime_error("Failed to read int64");
		}
		return value;
	}

} //	namespace

//------------------------------------------------
//  Читает список из текстового файла
//  Формат строки: <data>;<rand_index>
//------------------------------------------------
void ListSerializer::loadFromFile(List& list, const std::string& fileName) {

	//---Лямбда: удаляет символы " \t\r\n" в строке (для Windows-совместимости)
	auto trimSpaces = [](std::string str) -> std::string {
		const char* whitespace = " \t\r\n";

		const std::size_t begin = str.find_first_not_of(whitespace);
		if (begin == std::string::npos) {
			return "";
		}

		const std::size_t end = str.find_last_not_of(whitespace);
		return str.substr(begin, end - begin + 1);
		};

	list.clear();	//	Очищаем список перед загрузкой

	std::ifstream in(fileName);
	
	if (!in.is_open())
	{
		const std::string msg = "Failed to open file: " + fileName;
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}

	std::vector<ListNode*> nodes;
	std::vector<int> randIndices;

	nodes.reserve(1024);		//	Резервируем память для узлов
	randIndices.reserve(1024);	//	Резервируем память для индексов rand

	std::string line;
	std::size_t lineNumber = 0;

	try
	{
		//---Читаем файл построчно
		while (std::getline(in, line))
		{
			++lineNumber;
			
			line = trimSpaces(line);	// Нормализуем конец строки

			//---Ищем разделитель ';' с конца (data может содержать ';')
			const std::size_t pos = line.rfind(';');
			
			//---Валидация данных
			if (pos==std::string::npos)
			{
				const std::string msg = "Invalid format at line " + std::to_string(lineNumber) + ": separator ';' not found";
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}
			
			//---Извлекаем data и rand_index
			const std::string data = line.substr(0, pos);
			const std::string randIndexStr = trimSpaces(line.substr(pos + 1));

			//---Валидация данных
			if (data.size() > List::kMaxDataSize)
			{
				const std::string msg = "Data is to long at line " + std::to_string(lineNumber);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}
			if (randIndexStr.empty())
			{
				const std::string msg = "Invalid rand index at line" + std::to_string(lineNumber);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}

			//---Парсим rand-индекс
			std::size_t parsedCount = 0;
			long long randIndexLong = -1;

			try
			{
				randIndexLong = std::stoll(randIndexStr, &parsedCount);
			}
			catch (const std::exception&)
			{
				const std::string msg = "Invalid rand index at line " + std::to_string(lineNumber);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}

			//---Проверяем, что вся строка распаршена
			if (parsedCount != randIndexStr.size())
			{
				const std::string msg = "Invalid index at line " + std::to_string(lineNumber);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}
			//---Проверяем, что rand-индекс в допустимых пределах
			if (randIndexLong < -1 || randIndexLong > static_cast<long long>(std::numeric_limits<int>::max()))
			{
				const std::string msg = "rand index is out of range at line " + std::to_string(lineNumber);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}
			if (list.getCount() >= List::kNodeLimit)
			{
				const std::string msg = "Too many nodes is input file: maximum is 1'000'000";
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}

			//---Создаём новый узел с данными и добавляем его в список
			ListNode* newNode = list.pushBack(data);

			nodes.push_back(newNode);
			randIndices.push_back(static_cast<int>(randIndexLong));
		}

		//---Второй проход: привязываем rand-указатели по сохранённым индексам
		for (size_t i = 0; i < nodes.size(); i++)
		{
			const int randIndex = randIndices[i];

			if (randIndex == -1)
			{
				nodes[i]->rand = nullptr;
				continue;
			}
			if (randIndex < 0 || randIndex >= static_cast<int>(nodes.size())) 
			{
				const std::string msg = "rand index out of bounds for node " + std::to_string(i);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}
			nodes[i]->rand = nodes[randIndex];
		}
	}
	catch (const std::exception& e)
	{
		list.clear();	//	Очищаем список при возникновении ошибки
		const std::string msg = "Error while loading list from file: " + std::string(e.what());
		throw std::runtime_error(msg);
	}
}
//------------------------------------------------
//	Метод сериализует список в бинарный файл
//	Формат: | count (uint64) | для каждого узла: | dataSize (uint32) | data | randIndex (int64) |
//------------------------------------------------
void ListSerializer::serializeBinary(const List& list, const std::string& fileName) {

	//---Открываем файл для записи в бинарном режиме
	std::ofstream out(fileName, std::ios::binary);
	if (!out.is_open())
	{
		const std::string msg = "Failed to open output file: " + fileName;
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}
	//---Собираем узлы списка в вектор для удобства доступа по индексу
	std::vector<ListNode*> nodes;
	nodes.reserve(list.getCount());	//	Резервируем память для узлов

	for (ListNode* current = list.getHead() ; current != nullptr; current = current->next)
	{
		nodes.push_back(current);
	}

	//---Записываем количество узлов в списке
	writeUint64(out, static_cast<std::uint64_t>(nodes.size()));	//	Записываем количество узлов в списке
	
	//---Карта: узел -> его индекс в векторе
	std::unordered_map<ListNode*, std::int64_t> indexByPtr;
	indexByPtr.reserve(nodes.size());

	for (size_t i = 0; i < nodes.size(); i++)
	{
		indexByPtr[nodes[i]] = static_cast<std::int64_t>(i);
	}

	//---Записываем данные каждого узла и индекс его rand-указателя
	for (ListNode* node : nodes)
	{
		//---Проверяем, что размер данных не превышает максимальный размер для uint32
		if (node->data.size() > static_cast<std::size_t>(UINT32_MAX))
		{
			const std::string msg = "Node data is too large to serialize";
			LOG(ERROR) << msg;
			throw std::runtime_error(msg);
		}

		//---Записываем размер данных узла
		const std::uint32_t dataSize = static_cast<std::uint32_t>(node->data.size());
		writeUint32(out, dataSize);

		//---Записываем данные узла
		if (!node->data.empty())
		{
			out.write(node->data.data(), static_cast<std::streamsize>(node->data.size()));
			if (!out)
			{
				const std::string msg = "Failed to write node data";
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}
		}

		//---Записываем индекс rand-указателя узла (-1, если rand == nullptr)
		std::int64_t randIndex = -1;
		if (node->rand != nullptr)
		{
			const auto it = indexByPtr.find(node->rand);
			if (it == indexByPtr.end())
			{
				const std::string msg = "rand points to a node outside the list";
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}

			randIndex = it->second;
		}

		writeInt64(out, randIndex);
	}
}
//------------------------------------------------
//    Десериализует список из бинарного файла
//------------------------------------------------
void ListSerializer::deserializeBinary(List& list, const std::string& fileName) {

	list.clear();	//	Очищаем список перед загрузкой

	//---Открываем файл для чтения в бинарном режиме
	std::ifstream in(fileName, std::ios::binary);
	if (!in.is_open())
	{
		const std::string msg = "Failed to open binary file: " + fileName;
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}

	//---Читаем количество узлов в списке(первые 8 байт количество узлов (uint64_t))
	const std::uint64_t nodeCount = readUint64(in);

	//---Валидируем количество узлов
	if (nodeCount > List::kNodeLimit)
	{
		const std::string msg = "Too many nodes in binary file";
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}

	//---Временные векторы для хранения указателей на узлы и их rand-индексов
	std::vector<ListNode*> nodes;
	std::vector<std::int64_t> randIndices;

	try
	{
		//--- ПЕРВЫЙ ПРОХОД: создаём все узлы, читаем данные и rand-индексы
		for (std::uint64_t i = 0; i < nodeCount; i++)
		{
			//---Читаем размер данных узла (4 байта, uint32_t)
			const std::uint32_t dataSize = readUint32(in);

			//---Проверяем, не превышает ли размер данных лимит
			if (dataSize > List::kMaxDataSize)
			{
				const std::string msg = "Node data is too large at binary node " + std::to_string(i);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}

			//---Читаем данные узла
			std::string data;
			data.resize(dataSize);	//	Резервируем место для данных узла

			if (dataSize > 0)
			{
				//---Читаем ровно dataSize байт в строку data
				in.read(data.data(), static_cast<std::streamsize>(dataSize));
				if (!in)
				{
					const std::string msg = "Failed to read node data at binary node " + std::to_string(i);
					LOG(ERROR) << msg;
					throw std::runtime_error(msg);
				}
			}

			//---Читаем индекс rand-указателя узла (8 байт, int64_t)
			const std::int64_t randIndex = readInt64(in);

			//----Создаём новый узел с данными и добавляем его в список
			ListNode* newNode = list.pushBack(data);
			nodes.push_back(newNode);
			randIndices.push_back(randIndex);
		}

		//--- ВТОРОЙ ПРОХОД: восстанавливаем rand-указатели
		for (std::size_t i = 0; i < nodes.size(); ++i)
		{
			const  std::int64_t randIndex = randIndices[i];

			//---Если randIndex == -1, указатель должен быть nullptr
			if (randIndex == -1)
			{
				nodes[i]->rand = nullptr;
				continue;
			}

			//---Проверяем, что индекс в допустимых пределах
			if (randIndex < 0 || randIndex >= static_cast<std::int64_t>(nodes.size()))
			{
				const std::string msg = "rand index out of bounds for binary node " + std::to_string(i);
				LOG(ERROR) << msg;
				throw std::runtime_error(msg);
			}

			//---Устанавливаем rand-указатель узла
			nodes[i]->rand = nodes[static_cast<std::size_t>(randIndex)];
		}
	}
	catch (const std::exception& e)
	{
		list.clear();	//	Очищаем список при возникновении ошибки
		const std::string msg = "Error while loading list from binary file: " + std::string(e.what());
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}
}
