#include "fixed_memory_resource.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

// Конструктор
FixedMemoryResource::FixedMemoryResource(size_t size) 
    : pool_size_(size), current_offset_(0) {
    
    memory_pool_ = ::operator new(pool_size_);
}

// Деструктор
FixedMemoryResource::~FixedMemoryResource() {
    cleanup();
}

// Конструктор перемещения
FixedMemoryResource::FixedMemoryResource(FixedMemoryResource&& other) noexcept
    : memory_pool_(other.memory_pool_),
      pool_size_(other.pool_size_),
      current_offset_(other.current_offset_),
      blocks_info_(std::move(other.blocks_info_)) {
    
    other.memory_pool_ = nullptr;
    other.pool_size_ = 0;
    other.current_offset_ = 0;
}

// Оператор присваивания перемещением
FixedMemoryResource& FixedMemoryResource::operator=(FixedMemoryResource&& other) noexcept {
    if (this != &other) {
        cleanup();
        
        memory_pool_ = other.memory_pool_;
        pool_size_ = other.pool_size_;
        current_offset_ = other.current_offset_;
        blocks_info_ = std::move(other.blocks_info_);
        
        other.memory_pool_ = nullptr;
        other.pool_size_ = 0;
        other.current_offset_ = 0;
    }
    return *this;
}

// Выделение памяти
void* FixedMemoryResource::do_allocate(size_t bytes, size_t alignment) {
    // Пытаемся найти свободный блок
    void* ptr = find_free_block(bytes, alignment);
    if (ptr) {
        // Нашли  помечаем как занятый
        blocks_info_[ptr].is_free = false;
        blocks_info_[ptr].alignment = alignment;
        return ptr;
    }
    
    // Свободного блока нет выделяем новый
    size_t aligned_offset = (current_offset_ + alignment - 1) / alignment * alignment;
    
    if (aligned_offset + bytes > pool_size_) {
        throw std::bad_alloc();
    }
    
    ptr = static_cast<char*>(memory_pool_) + aligned_offset;
    
    // Сохраняем информацию о блоке
    blocks_info_[ptr] = BlockInfo(bytes, false, alignment);
    
    current_offset_ = aligned_offset + bytes;
    
    return ptr;
}

// Освобождение памяти
void FixedMemoryResource::do_deallocate(void* ptr, size_t bytes, size_t alignment) {
    // Проверяем, что блок существует
    auto it = blocks_info_.find(ptr);
    if (it == blocks_info_.end()) {
        throw std::invalid_argument("Block not allocated by this resource");
    }
    
    // Проверяем размер
    if (it->second.size != bytes) {
        throw std::invalid_argument("Block size mismatch");
    }
    
    // Помечаем как свободный (но не удаляем из map!)
    it->second.is_free = true;
    it->second.alignment = alignment;
}

// Сравнение memory_resource
bool FixedMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

// Статистика
void FixedMemoryResource::print_stats() const {
    size_t allocated_count = 0;
    size_t free_count = 0;
    
    for (const auto& pair : blocks_info_) {
        if (pair.second.is_free) {
            free_count++;
        } else {
            allocated_count++;
        }
    }
    
    std::cout << "\nСтатистика использования памяти:\n"
              << "Общий размер: " << pool_size_ << " байт\n"
              << "Использовано: " << current_offset_ << " байт\n"
              << "Активных блоков: " << allocated_count << "\n"
              << "Свободных блоков: " << free_count << "\n\n";
}

// Поиск свободного блока
void* FixedMemoryResource::find_free_block(size_t bytes, size_t alignment) {
    // Проходим по всем блокам и ищем подходящий свободный
    for (auto& pair : blocks_info_) {
        BlockInfo& info = pair.second;
        
        if (info.is_free && info.size >= bytes) {
            // Проверяем выравнивание
            if (reinterpret_cast<uintptr_t>(pair.first) % alignment == 0) {
                return pair.first;
            }
        }
    }
    
    return nullptr;
}

// Очистка ресурсов
void FixedMemoryResource::cleanup() {
    if (memory_pool_) {
        if (!blocks_info_.empty()) {
            std::cout << "освобождается память с " 
                      << get_allocated_count() << " неосвобождёнными блоками\n";
        }
        
        ::operator delete(memory_pool_);
        memory_pool_ = nullptr;
    }
}

// Подсчёт занятых блоков
size_t FixedMemoryResource::get_allocated_count() const {
    return std::count_if(blocks_info_.begin(), blocks_info_.end(),
        [](const auto& pair) { return !pair.second.is_free; });
}

// Подсчёт свободных блоков
size_t FixedMemoryResource::get_free_count() const {
    return std::count_if(blocks_info_.begin(), blocks_info_.end(),
        [](const auto& pair) { return pair.second.is_free; });
}