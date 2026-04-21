#include "ListSerializer.hpp"
#include "List.hpp"

#include <climits>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <unordered_map>

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

	//---Лямбда: удаляет символ '\r' в конце строки (для Windows-совместимости)
	auto removeCRLF = [](std::string str)->std::string {
		if (!str.empty() && str.back() == '\r')
		{
			str.pop_back();
		}
		return str;
	};

	list.clear();	//	Очищаем список перед загрузкой

	std::ifstream in(fileName);
	
	if (!in.is_open())
	{
		throw std::runtime_error("Failed to open file: " + fileName);
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
			
			line = removeCRLF(line);	// Нормализуем конец строки

			//---Ищем разделитель ';' с конца (data может содержать ';')
			const std::size_t pos = line.rfind(';');
			
			//---Валидация данных
			if (pos==std::string::npos)
			{
				throw std::runtime_error("Invalid format at line " + std::to_string(lineNumber) + ": separator ';' not found");
			}
			
			//---Извлекаем data и rand_index
			const std::string data = line.substr(0, pos);
			const std::string randIndexStr = line.substr(pos + 1);

			//---Валидация данных
			if (data.size() > List::kMaxDataSize)
			{
				throw std::runtime_error("Data is to long at line " + std::to_string(lineNumber));
			}
			if (randIndexStr.empty())
			{
				throw std::runtime_error("Invalid rand index at line" + std::to_string(lineNumber));
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
				throw std::runtime_error("Invalid rand index at line " + std::to_string(lineNumber));
			}

			//---Проверяем, что вся строка распаршена
			if (parsedCount != randIndexStr.size())
			{
				throw std::runtime_error("Invalid index at line " + std::to_string(lineNumber));
			}
			//---Проверяем, что rand-индекс в допустимых пределах
			if (randIndexLong < -1 || randIndexLong > static_cast<long long>(std::numeric_limits<int>::max()))
			{
				throw std::runtime_error("rand index is out of range at line " + std::to_string(lineNumber));
			}
			if (list.getCount() >= List::kNodeLimit)
			{
				throw std::runtime_error("Too many nodes is input file: maximum is 1'000'000");
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
				throw std::runtime_error("rand index out of bounds for node " + std::to_string(i));
			}
			nodes[i]->rand = nodes[randIndex];
		}
	}
	catch (const std::exception& e)
	{
		list.clear();	//	Очищаем список при возникновении ошибки
		throw std::runtime_error("Error while loading list from file: " + std::string(e.what()));
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
		throw std::runtime_error("Failed to open output file: " + fileName);
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
			throw std::runtime_error("Node data is too large to serialize");
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
				throw std::runtime_error("Failed to write node data");
			}
		}

		//---Записываем индекс rand-указателя узла (-1, если rand == nullptr)
		std::int64_t randIndex = -1;
		if (node->rand != nullptr)
		{
			const auto it = indexByPtr.find(node->rand);
			if (it == indexByPtr.end())
			{
				throw std::runtime_error("rand points to a node outside the list");
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
		throw std::runtime_error("Failed to open binary file: " + fileName);
	}

	//---Читаем количество узлов в списке(первые 8 байт количество узлов (uint64_t))
	const std::uint64_t nodeCount = readUint64(in);

	//---Валидируем количество узлов
	if (nodeCount > List::kNodeLimit)
	{
		throw std::runtime_error("Too many nodes in binary file");
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
				throw std::runtime_error("Node data is too large at binary node " + std::to_string(i));
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
					throw std::runtime_error("Failed to read node data at binary node " + std::to_string(i));
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
				throw std::runtime_error("rand index out of bounds for binary node " + std::to_string(i));
			}

			//---Устанавливаем rand-указатель узла
			nodes[i]->rand = nodes[static_cast<std::size_t>(randIndex)];
		}
	}
	catch (const std::exception& e)
	{
		list.clear();	//	Очищаем список при возникновении ошибки
		throw std::runtime_error("Error while loading list from binary file: " + std::string(e.what()));
	}
}
