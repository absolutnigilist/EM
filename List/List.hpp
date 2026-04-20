#pragma once
#include "ListNode.hpp"

//------------------------------------------------
//	Класс, реализующий двусвязный список
//------------------------------------------------
class List
{

public:
	
	//---Конструктор по умолчанию: создаёт пустой список
	List() = default;
	//----Деструктор: очищает список
	~List();

	//---Запрещаем копирование и перемещение списка
	List(const List&) = delete;
	List& opeartor = (const List&) = delete;
	List(List&&) = delete;
	List& operator = (List&&) = delete;

	//---Метод для очистки списка
	void clear() noexcept;

	//---Методы для получения указателей на начало и конец списка, а также количества элементов
	ListNode* getHead() const noexcept;
	ListNode* getTail() const noexcept;
	std::size_t getCount() const noexcept;

	//---Метод добавления элемента в конец списка
	ListNode* pushBack(const std::string& data);

	//---Метод добавления элемента в начало списка
	ListNode* pushBegin(const std::string& data);	

	//---Метод вставки элемента после указанного узла
	ListNode* insertAfter(ListNode* node, const std::string& data);

	//---Метод вставки элемента перед указанным узлом
	ListNode* insertBefore(ListNode* node, const std::string& data);

	//---Метод удаления указанного узла
	void erase(ListNode* node);

	//---Метод получения указателя на элемент по индексу
	ListNode* getNodeAt(std::size_t index) const;

private:
	
	ListNode* head = nullptr;	//	Указатель на начало списка
	ListNode* tail = nullptr;	//	Указатель на конец списка
	std::size_t count = 0;		//	Количество элементов в списке
	std::size_t capacity = 6;   //	Емкость списка
}