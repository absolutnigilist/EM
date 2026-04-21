#include "List.hpp"
#include <stdexcept>
#include <glog/logging.h>

//------------------------------------------------
//	Деструктор класса
//------------------------------------------------
List::~List() {
	clear();
}
//------------------------------------------------
//	Метод очистки списка
//------------------------------------------------
void List::clear() noexcept {

	ListNode* current = head;
	while (current != nullptr)
	{
		ListNode* next = current->next;
		delete current;
		current = next;
	}
	head = nullptr;
	tail = nullptr;
	count = 0;
}
//------------------------------------------------
//	Метод получения указателя на начало
//------------------------------------------------
ListNode* List::getHead() const noexcept{
	return head;
}
//------------------------------------------------
//	Метод получения указателя на конец
//------------------------------------------------
ListNode* List::getTail() const noexcept{
	return tail;
}
//------------------------------------------------
//	Метод получения количества элементов в списке
//------------------------------------------------
std::size_t List::getCount() const noexcept{
	return count;
}
//------------------------------------------------
//	Метод добавления элемента в конец списка
//------------------------------------------------
ListNode* List::pushBack(const std::string& data) {
	
	if (count >= kNodeLimit) {

		const std::string msg = "List size exceeds 1,000,000 nodes";
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}

	//---Создаём новый узел
	ListNode* newNode = new ListNode();
	newNode->data = data;
	newNode->next = nullptr;
	newNode->prev = tail;
	newNode->rand = nullptr;

	if (tail != nullptr)
	{
		//---Добавляем новый узел в конец списка
		tail->next = newNode;
	}
	else
	{
		//---Список был пуст, новый узел становится началом списка
		head = newNode;
	}
	//---Новый узел становится концом списка
	tail = newNode;
	
	//---Увеличиваем количество элементов в списке
	++count;

	return newNode;
}
//------------------------------------------------
//	Метод добавления элемента в начало списка
//------------------------------------------------
ListNode* List::pushBegin(const std::string& data) {

	if (count >= kNodeLimit) {
		const std::string msg = "List size exceeds 1,000,000 nodes";
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}

	//---Создаём новый узел
	ListNode* newNode = new ListNode();

	newNode->data = data;
	newNode->next = head;
	newNode->prev = nullptr;
	newNode->rand = nullptr;

	if (head ==nullptr)
	{
		//---Список был пуст, новый узел становится концом списка
		tail = newNode;
	}
	else
	{
		//---Добавляем новый узел в начало списка
		head->prev = newNode;
	}
	
	//---Новый узел становится началом списка
	head = newNode;

	//---Увеличиваем количество элементов в списке
	++count;

	return newNode;
}
//------------------------------------------------
//	Метод вставки элемента после указанного узла
//------------------------------------------------
ListNode* List::insertAfter(ListNode* node, const std::string& data) {

	if (count >= kNodeLimit) {
		const std::string msg = "List size exceeds 1,000,000 nodes";
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}

	//---Создаём новый узел
	ListNode* newNode = new ListNode();
	newNode->data = data;
	newNode->rand = nullptr;
	
	//---Если node не равен nullptr, вставляем новый узел после node
	if (node != nullptr)
	{
		//---Вставка после node
		newNode->prev = node;
		newNode->next = node->next;

		if (node->next != nullptr)
		{
			node->next->prev = newNode;
		}
		else
		{
			tail = newNode;
		}
		node->next = newNode;
	}
	//---Если node равен nullptr, вставляем новый узел в начало списка
	else
	{
		//---Вставка в начало списка
		newNode->prev = nullptr;
		newNode->next = head;

		if (head != nullptr)
		{
			head->prev = newNode;
		}
		else
		{
			tail = newNode;
		}
		head = newNode;
	}
	++count;
	return newNode;
}
//------------------------------------------------
//	Метод вставки элемента перед указанным узлом
//------------------------------------------------
ListNode* List::insertBefore(ListNode* node, const std::string& data) {
	
	if (count >= kNodeLimit) {
		const std::string msg = "List size exceeds 1,000,000 nodes";
		LOG(ERROR) << msg;
		throw std::runtime_error(msg);
	}

	//---Создаём новый узел
	ListNode* newNode = new ListNode();
	newNode->data = data;
	newNode->rand = nullptr;

	//---Если node не равен nullptr, вставляем новый узел перед node
	if (node != nullptr)
	{
		//---Вставка перед node
		newNode->next = node;
		newNode->prev = node->prev;

		if (node->prev != nullptr)
		{
			node->prev->next = newNode;
		}
		else
		{
			head = newNode;
		}
		node->prev = newNode;
	}
	else
	{
		//---Если node равен nullptr, вставляем новый узел в конец списка
		newNode->next = nullptr;
		newNode->prev = tail;

		if (tail != nullptr)
		{
			tail->next = newNode;
		}
		else
		{
			head = newNode;
		}
		tail = newNode;
	}
	
	++count;
	return newNode;
}
//------------------------------------------------
//    Метод для удаления указанного узла
//------------------------------------------------
void List::erase(ListNode* node) {
	
	//---Если node равен nullptr, ничего не удаляем
	if (node == nullptr) return;


	if (node->prev != nullptr)
	{
		//---Узел не является началом списка, обновляем указатель next предыдущего узла
		node->prev->next = node->next;
	}
	else
	{
		//---Узел является началом списка, обновляем указатель head
		head = node->next;
	}

	if (node->next != nullptr)
	{
		//---Узел не является концом списка, обновляем указатель prev следующего узла
		node->next->prev = node->prev;
	}
	else
	{
		//---Узел является концом списка, обновляем указатель tail
		tail = node->prev;
	}

	//---Удаляем узел и уменьшаем количество элементов в списке
	delete node;
	--count;
}
//------------------------------------------------
//	Метод получения указателя на элемент по индексу
//------------------------------------------------
ListNode* List::getNodeAt(std::size_t index) const noexcept {
	
	//---Проверяем, что индекс не превышает количество элементов в списке
	if (index >= count) {
		return nullptr;
	}

	//---Ищем узел по индексу, начиная с головы списка
	ListNode* current = head;
	for (std::size_t i = 0; i < index && current != nullptr; ++i)
	{
		current = current->next;
	}
	return current;
}