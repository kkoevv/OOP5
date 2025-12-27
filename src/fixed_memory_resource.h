#ifndef FIXED_MEMORY_RESOURCE_H
#define FIXED_MEMORY_RESOURCE_H

#include <memory_resource>
#include <map>
#include <cstddef>

// Аллокатор с фиксированным блоком памяти
// Выделяет память один раз при создании, затем управляет этим блоком
class FixedMemoryResource : public std::pmr::memory_resource {
private:
    // Указатель на начало фиксированного блока памяти
    void* memory_pool_;
    
    // Общий размер блока памяти в байтах
    size_t pool_size_;
    
    // Текущее смещение в блоке (до какого места выделена память)
    size_t current_offset_;
    
    // Список активных (занятых) блоков: адрес блока -> размер блока
    // информация о выделенных блоках хранится в std::map
    std::map<void*, size_t> allocated_blocks_;
    
    // Список свободных блоков для переиспользования: размер -> адрес
    // Используется multimap, так как может быть несколько блоков одного размера
    std::multimap<size_t, void*> free_blocks_;

public:
    // Конструктор: выделяет фиксированный блок памяти заданного размера
    // size - размер блока в байтах (по умолчанию 1 МБ)
    explicit FixedMemoryResource(size_t size = 1024 * 1024);
    
    // Деструктор: освобождает весь блок памяти
    ~FixedMemoryResource() override;
    
    // Запрещаем копирование (уникальныйй ресурс
    FixedMemoryResource(const FixedMemoryResource&) = delete; //коп
    FixedMemoryResource& operator=(const FixedMemoryResource&) = delete; //при коп
    
    // Разрешаем перемещение (передача владения)
    FixedMemoryResource(FixedMemoryResource&& other) noexcept; //пер
    FixedMemoryResource& operator=(FixedMemoryResource&& other) noexcept; //присв пер
    
    // Вывод статистики использования памяти
    void print_stats() const;
    
    // Методы для тестирования
    size_t get_allocated_count() const { return allocated_blocks_.size(); }
    size_t get_free_count() const { return free_blocks_.size(); }
    size_t get_current_offset() const { return current_offset_; }

protected:
    // Выделение памяти (переопределение виртуального метода базового класса)
    // bytes - количество байт для выделения
    // alignment - требуемое выравнивание адреса
    void* do_allocate(size_t bytes, size_t alignment) override;
    
    // Освобождение памяти (переопределение виртуального метода базового класса)
    // ptr - указатель на освобождаемый блок
    // bytes - размер блока
    // alignment - выравнивание блока
    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override;
    
    // Сравнение memory_resource (переопределение виртуального метода)
    // Два ресурса равны, если это один и тот же объект
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

private:
    // Поиск подходящего свободного блока для переиспользования
    // Возвращает указатель на блок или nullptr, если подходящий не найден
    void* find_free_block(size_t bytes, size_t alignment);
    
    // Очистка всех ресурсов (вызывается в деструкторе)
    void cleanup();
};

#endif