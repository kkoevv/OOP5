
#ifndef QUEUE_H
#define QUEUE_H

#include <memory>
#include <memory_resource>
#include <iterator>
#include <stdexcept>


template<typename T>
class Queue {
private:
    // Узел односвязного списка
    struct Node {
        T data;           // Данные пользователя
        Node* next;       // Указатель на следующий узел
        
        
        template<typename... Args>
        Node(Args&&... args) : data(std::forward<Args>(args)...), next(nullptr) {}
    };
    
    // Указатель на первый элемент очереди (откуда удаляем)
    Node* head_;
    
    // Указатель на последний элемент очереди (куда добавляем)
    Node* tail_;
    
    // Количество элементов в очереди
    size_t size_;
    
    // Аллокатор для выделения памяти под узлы
    // Использует переданный memory_resource через polymorphic_allocator
    std::pmr::polymorphic_allocator<Node> allocator_;

public:
    // Forward-итератор для обхода элементов очереди
    class Iterator {
    private:
        Node* current_;  // Текущий узел
        
    public:
        using iterator_category = std::forward_iterator_tag;  
        using value_type = T;                                  
        using difference_type = std::ptrdiff_t;               
        using pointer = T*;                                   
        using reference = T&;                                  
        
        // Конструктор итератора
        explicit Iterator(Node* node = nullptr) : current_(node) {}
        
        // Оператор разыменования: получить ссылку на данные
        reference operator*() const {
            if (!current_) {
                throw std::runtime_error("Dereferencing end iterator");
            }
            return current_->data;
        }
        
        // Оператор стрелка: доступ к членам объекта
        pointer operator->() const {
            if (!current_) {
                throw std::runtime_error("Dereferencing end iterator");
            }
            return &(current_->data);
        }
        
        // Префиксный инкремент: ++it
        // Перемещает итератор к следующему элементу
        Iterator& operator++() {
            if (current_) {
                current_ = current_->next;
            }
            return *this;
        }
        
        // Постфиксный инкремент: it++
        // Возвращает копию старого состояния, затем инкрементирует
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        // Операторы сравнения
        bool operator==(const Iterator& other) const {
            return current_ == other.current_;
        }
        
        bool operator!=(const Iterator& other) const {
            return current_ != other.current_;
        }
    };
    
    // Конструктор: создаёт пустую очередь
    // mr - указатель на memory_resource для выделения памяти
    explicit Queue(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : head_(nullptr), tail_(nullptr), size_(0), allocator_(mr) {}
    
    // Деструктор: освобождает всю память
    ~Queue() {
        clear();
    }
    
    // Конструктор копирования: создаёт  копию очереди
    // Все узлы копируются, создаются новые объекты
    Queue(const Queue& other)
        : head_(nullptr), tail_(nullptr), size_(0), allocator_(other.allocator_) {
        
        // Копируем все элементы из other
        for (Node* current = other.head_; current != nullptr; current = current->next) {
            push(current->data);
        }
    }
    
    // Оператор присваивания копированием
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            // Очищаем текущую очередь
            clear();
            
            // Копируем элементы из other
            for (Node* current = other.head_; current != nullptr; current = current->next) {
                push(current->data);
            }
        }
        return *this;
    }
    
    // Конструктор перемещения: забирает содержимое из other
    // other остаётся в пустом состоянии
    Queue(Queue&& other) noexcept
        : head_(other.head_),
          tail_(other.tail_),
          size_(other.size_),
          allocator_(other.allocator_.resource()) {
        
        // Обнуляем other, чтобы он не удалил узлы при уничтожении
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }
    
    // Оператор присваивания перемещением
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            // Очищаем свои данные
            clear();
            
            // Забираем данные из other
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            
            
            // Обнуляем other
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    // Добавить элемент в конец очереди (копирование)
    void push(const T& value) {
        // Выделяем память для нового узла через аллокатор
        Node* new_node = allocator_.allocate(1);
        
        // Конструируем узел в выделенной памяти
        allocator_.construct(new_node, value);
        
        // Добавляем узел в конец списка
        if (empty()) {
            head_ = tail_ = new_node;
        } else {
            tail_->next = new_node;
            tail_ = new_node;
        }
        
        ++size_;
    }
    
    // Добавить элемент в конец очереди (перемещение)
    void push(T&& value) {
        Node* new_node = allocator_.allocate(1);
        allocator_.construct(new_node, std::move(value));
        
        if (empty()) {
            head_ = tail_ = new_node;
        } else {
            tail_->next = new_node;
            tail_ = new_node;
        }
        
        ++size_;
    }
    
    // Удалить первый элемент из очереди
    void pop() {
        if (empty()) {
            throw std::runtime_error("pop from empty queue");
        }
        
        // Сохраняем указатель на удаляемый узел
        Node* old_head = head_;
        
        // Перемещаем head на следующий элемент
        head_ = head_->next;
        
        // Если очередь стала пустой, обнуляем tail
        if (head_ == nullptr) {
            tail_ = nullptr;
        }
        
        // Уничтожаем объект (вызывается деструктор T)
        allocator_.destroy(old_head);
        
        // Освобождаем память через аллокатор
        // Память вернётся в free_blocks_ нашего FixedMemoryResource
        allocator_.deallocate(old_head, 1);
        
        --size_;
    }
    
    // Получить ссылку на первый элемент
    T& front() {
        if (empty()) {
            throw std::runtime_error("front on empty queue");
        }
        return head_->data;
    }
    
    const T& front() const {
        if (empty()) {
            throw std::runtime_error("front on empty queue");
        }
        return head_->data;
    }
    
    // Получить ссылку на последний элемент
    T& back() {
        if (empty()) {
            throw std::runtime_error("back on empty queue");
        }
        return tail_->data;
    }
    
    const T& back() const {
        if (empty()) {
            throw std::runtime_error("back on empty queue");
        }
        return tail_->data;
    }
    
    // Проверка на пустоту
    bool empty() const noexcept {
        return size_ == 0;
    }
    
    // Получить размер очереди
    size_t size() const noexcept {
        return size_;
    }
    
    // Удалить все элементы
    void clear() {
        while (!empty()) {
            pop();
        }
    }
    
    // Получить итератор на начало
    Iterator begin() {
        return Iterator(head_);
    }
    
    // Получить итератор на конец (за последним элементом)
    Iterator end() {
        return Iterator(nullptr);
    }
    
    // Константные версии
    Iterator begin() const {
        return Iterator(head_);
    }
    
    Iterator end() const {
        return Iterator(nullptr);
    }
};

#endif
