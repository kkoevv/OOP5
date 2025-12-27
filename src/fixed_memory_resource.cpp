#include "fixed_memory_resource.h"
#include <iostream>
#include <stdexcept>

// Конструктор: выделяет фиксированный блок памяти
FixedMemoryResource::FixedMemoryResource(size_t size) 
    : pool_size_(size), current_offset_(0) {
    
    // Выделяем один большой блок памяти через operator new
    // Этот блок будет использоваться для всех последующих выделений
    memory_pool_ = ::operator new(pool_size_);
}

// Деструктор: освобождает блок памяти
FixedMemoryResource::~FixedMemoryResource() {
    cleanup();
}

// Конструктор перемещения: передаёт владение ресурсом
FixedMemoryResource::FixedMemoryResource(FixedMemoryResource&& other) noexcept
    : memory_pool_(other.memory_pool_),
      pool_size_(other.pool_size_),
      current_offset_(other.current_offset_),
      allocated_blocks_(std::move(other.allocated_blocks_)),
      free_blocks_(std::move(other.free_blocks_)) {
    
    // Обнуляем источник, чтобы он не освободил память при уничтожении
    other.memory_pool_ = nullptr;
    other.pool_size_ = 0;
    other.current_offset_ = 0;
}

// Оператор присваивания перемещением
FixedMemoryResource& FixedMemoryResource::operator=(FixedMemoryResource&& other) noexcept {
    if (this != &other) {
        // Освобождаем свои текущие ресурсы
        cleanup();
        
        // Забираем ресурсы из other
        memory_pool_ = other.memory_pool_;
        pool_size_ = other.pool_size_;
        current_offset_ = other.current_offset_;
        allocated_blocks_ = std::move(other.allocated_blocks_);
        free_blocks_ = std::move(other.free_blocks_);
        
        // Обнуляем источник
        other.memory_pool_ = nullptr;
        other.pool_size_ = 0;
        other.current_offset_ = 0;
    }
    return *this;
}

// Выделение памяти
void* FixedMemoryResource::do_allocate(size_t bytes, size_t alignment) {
    // Сначала пытаемся найти подходящий свободный блок для переиспользования
    void* ptr = find_free_block(bytes, alignment);
    if (ptr) {
        // Нашли свободный блок - переиспользуем его
        allocated_blocks_[ptr] = bytes;
        return ptr;
    }
    
    // Свободного блока нет - выделяем новый из основного пула
    
    // Вычисляем выровненное смещение
    // Формула: (current + alignment - 1) / alignment * alignment
    // Это округляет current_offset_ вверх до ближайшего кратного alignment
    size_t aligned_offset = (current_offset_ + alignment - 1) / alignment * alignment;
    
    // Проверяем, достаточно ли места в пуле
    if (aligned_offset + bytes > pool_size_) {
        throw std::bad_alloc();
    }
    
    // Вычисляем адрес: начало пула + смещение
    ptr = static_cast<char*>(memory_pool_) + aligned_offset;
    
    // Сохраняем информацию о выделенном блоке в map
    allocated_blocks_[ptr] = bytes;
    
    // Обновляем текущее смещение
    current_offset_ = aligned_offset + bytes;
    
    return ptr;
}

// Освобождение памяти
void FixedMemoryResource::do_deallocate(void* ptr, size_t bytes, size_t /*alignment*/) {
    // Проверяем, что этот блок действительно был выделен нами
    auto it = allocated_blocks_.find(ptr);
    if (it == allocated_blocks_.end()) {
        throw std::invalid_argument("Block not allocated by this resource");
    }
    
    // Удаляем блок из списка активных
    allocated_blocks_.erase(it);
    
    // Добавляем блок в список свободных для последующего переиспользования
    free_blocks_.insert({bytes, ptr});
}

// Сравнение memory_resource
bool FixedMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    // Два ресурса равны только если это один и тот же объект
    return this == &other;
}

// Вывод статистики
void FixedMemoryResource::print_stats() const {
    std::cout << "\nСтатистика использования памяти:\n"
              << "Общий размер: " << pool_size_ << " байт\n"
              << "Использовано: " << current_offset_ << " байт\n"
              << "Активных блоков: " << allocated_blocks_.size() << "\n"
              << "Свободных блоков: " << free_blocks_.size() << "\n\n";
}

// Поиск свободного блока для переиспользования
void* FixedMemoryResource::find_free_block(size_t bytes, size_t alignment) {
    for (auto it = free_blocks_.lower_bound(bytes); it != free_blocks_.end(); ++it) {
        void* ptr = it->second;
        
        // Проверяем, что адрес блока удовлетворяет требованиям выравнивания
        // reinterpret_cast<uintptr_t> преобразует указатель в целое число
        if (reinterpret_cast<uintptr_t>(ptr) % alignment == 0) {
            // Блок подходит - удаляем из списка свободных и возвращаем
            free_blocks_.erase(it);
            return ptr;
        }
    }
    
    // Подходящий блок не найден
    return nullptr;
}

// Очистка ресурсов
void FixedMemoryResource::cleanup() {
    if (memory_pool_) {
        // Если остались неосвобождённые блоки - выводим предупреждение
        if (!allocated_blocks_.empty()) {
            std::cout << "Внимание: освобождается память с " 
                      << allocated_blocks_.size() << " неосвобождёнными блоками\n";
        }
        
        // Освобождаем весь блок памяти через operator delete
        ::operator delete(memory_pool_);
        memory_pool_ = nullptr;
    }
}