#include "fixed_memory_resource.h"
#include "queue.h"
#include <iostream>
#include <string>
#include <iomanip>

// Структура для демонстрации работы со сложным типом
struct Person {
    std::string name;
    int age;
    double salary;
    
    Person(std::string n, int a, double s) : name(std::move(n)), age(a), salary(s) {}
    
    void print() const {
        std::cout << "  Имя: " << std::left << std::setw(20) << name 
                  << " Возраст: " << std::setw(3) << age 
                  << " Зарплата: " << std::fixed << std::setprecision(2) << salary << std::endl;
    }
};

void print_separator(const std::string& title) {
    std::cout << "\n";
    std::cout << title << "\n";
    std::cout << std::string(70, '-') << "\n\n";
}

void demo_simple_type() {
    print_separator("ДЕМОНСТРАЦИЯ 1: Работа с простым типом (int)");
    
    std::cout << "Создание аллокатора с блоком памяти 4096 байт\n\n";
    FixedMemoryResource memory(4096);
    
    std::cout << "Создание очереди целых чисел\n";
    Queue<int> queue(&memory);
    
    // Проверка пустой очереди
    std::cout << "\nПроверка пустой очереди:\n";
    std::cout << "  Пуста: " << (queue.empty() ? "да" : "нет") << "\n";
    std::cout << "  Размер: " << queue.size() << "\n";
    
    // Добавление элементов
    std::cout << "\nДобавление элементов: 10, 20, 30, 40, 50\n";
    for (int i = 1; i <= 5; ++i) {
        queue.push(i * 10);
        std::cout << "  Добавлен элемент: " << (i * 10) << ", размер очереди: " << queue.size() << "\n";
    }
    
    // Доступ к элементам
    std::cout << "\nДоступ к элементам:\n";
    std::cout << "  Первый элемент (front): " << queue.front() << "\n";
    std::cout << "  Последний элемент (back): " << queue.back() << "\n";
    std::cout << "  Размер очереди: " << queue.size() << "\n";
    
    // Обход через итератор
    std::cout << "\nОбход через итератор:\n";
    std::cout << "  Элементы: ";
    for (auto it = queue.begin(); it != queue.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    
    // Обход через range-based for
    std::cout << "\nОбход через range-based for:\n";
    std::cout << "  Элементы: ";
    for (const auto& value : queue) {
        std::cout << value << " ";
    }
    std::cout << "\n";
    
    // Удаление элементов
    std::cout << "\nУдаление элементов:\n";
    while (!queue.empty()) {
        std::cout << "  Удаляем: " << queue.front() << ", остался размер: " << (queue.size() - 1) << "\n";
        queue.pop();
    }
    
    std::cout << "\nОчередь пуста: " << (queue.empty() ? "да" : "нет") << "\n";
    
    // Статистика памяти
    
    memory.print_stats();
}

void demo_complex_type() {
    print_separator("ДЕМОНСТРАЦИЯ 2: Работа со сложным типом (struct Person)");
    
    std::cout << "Создание аллокатора с блоком памяти 8192 байт\n\n";
    FixedMemoryResource memory(8192);
    
    std::cout << "Создание очереди объектов Person\n";
    Queue<Person> queue(&memory);
    
    // Добавление людей
    std::cout << "\nДобавление людей в очередь:\n";
    queue.push(Person("Иванов Иван Иванович", 25, 75000.50));
    std::cout << "  Добавлен: Иванов Иван Иванович\n";
    
    queue.push(Person("Петрова Анна Сергеевна", 30, 92000.75));
    std::cout << "  Добавлена: Петрова Анна Сергеевна\n";
    
    queue.push(Person("Сидоров Пётр Алексеевич", 35, 120000.00));
    std::cout << "  Добавлен: Сидоров Пётр Алексеевич\n";
    
    queue.push(Person("Козлова Мария Дмитриевна", 28, 85000.25));
    std::cout << "  Добавлена: Козлова Мария Дмитриевна\n";
    
    // Информация об очереди
    std::cout << "\nИнформация об очереди:\n";
    std::cout << "  Размер: " << queue.size() << " человек\n";
    std::cout << "\n  Первый в очереди:\n";
    queue.front().print();
    std::cout << "\n  Последний в очереди:\n";
    queue.back().print();
    
    // Вывод всех людей через итератор
    std::cout << "\nВсе люди в очереди:\n";
    int position = 1;
    for (const auto& person : queue) {
        std::cout << position++ << ". ";
        person.print();
    }
    
    // Использование оператора стрелка
    std::cout << "\nДоступ к полям через оператор стрелка итератора:\n";
    auto it = queue.begin();
    std::cout << "  Имя первого человека: " << it->name << "\n";
    std::cout << "  Возраст: " << it->age << "\n";
    std::cout << "  Зарплата: " << it->salary << "\n";
    
    // Удаление одного человека
    std::cout << "\nОбслуживание очереди (удаление первого):\n";
    std::cout << "  Обслужен: " << queue.front().name << "\n";
    queue.pop();
    std::cout << "  Осталось в очереди: " << queue.size() << " человек\n";
    
    // Оставшиеся люди
    std::cout << "\nОставшиеся в очереди:\n";
    position = 1;
    for (const auto& person : queue) {
        std::cout << position++ << ". ";
        person.print();
    }
    
    // Статистика памяти
  
    memory.print_stats();
}

void demo_memory_reuse() {
    print_separator("ДЕМОНСТРАЦИЯ 3: Переиспользование освобождённой памяти");
    
    std::cout << "Создание аллокатора с блоком памяти 4096 байт\n\n";
    FixedMemoryResource memory(4096);
    
    std::cout << "Создание очереди целых чисел\n";
    Queue<int> queue(&memory);
    
    // Фаза 1: Выделение
    std::cout << "\nФАЗА 1: Добавление 5 элементов\n";
    for (int i = 1; i <= 5; ++i) {
        queue.push(i);
    }
    std::cout << "Содержимое: ";
    for (const auto& val : queue) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    memory.print_stats();
    
    // Сохраняем offset для проверки
    size_t offset_after_allocation = memory.get_current_offset();
    std::cout << "Текущее смещение в памяти: " << offset_after_allocation << " байт\n";
    
    // Фаза 2: Освобождение
    std::cout << "\nФАЗА 2: Удаление 3 элементов\n";
    std::cout << "(Память должна добавиться в список свободных блоков)\n";
    for (int i = 0; i < 3; ++i) {
        std::cout << "  Удаляем: " << queue.front() << "\n";
        queue.pop();
    }
    std::cout << "Содержимое: ";
    for (const auto& val : queue) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    memory.print_stats();
    
    // Фаза 3: Переиспользование
    std::cout << "\nФАЗА 3: Добавление 3 новых элементов\n";
    std::cout << "(Память должна переиспользоваться из списка свободных блоков)\n";
    for (int i = 10; i <= 12; ++i) {
        queue.push(i);
    }
    std::cout << "Содержимое: ";
    for (const auto& val : queue) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    memory.print_stats();
    
    size_t offset_after_reuse = memory.get_current_offset();
    std::cout << "Текущее смещение в памяти: " << offset_after_reuse << " байт\n";
    
    // Проверка переиспользования
    std::cout << "\nПРОВЕРКА ПЕРЕИСПОЛЬЗОВАНИЯ:\n";
    if (offset_after_allocation == offset_after_reuse) {
        std::cout << "  УСПЕШНО: Смещение не изменилось!\n";
        std::cout << "  Память была успешно переиспользована.\n";
    } else {
        std::cout << "  ВНИМАНИЕ: Смещение изменилось с " 
                  << offset_after_allocation << " до " << offset_after_reuse << "\n";
    }
}

void demo_copy_and_move() {
    print_separator("ДЕМОНСТРАЦИЯ 4: Копирование и перемещение ");
    
    FixedMemoryResource memory(4096);
    
    // Создание оригинальной очереди
    std::cout << "Создание оригинальной очереди:\n";
    Queue<int> original(&memory);
    original.push(100);
    original.push(200);
    original.push(300);
    
    std::cout << "Оригинальная очередь: ";
    for (const auto& val : original) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    std::cout << "  Размер: " << original.size() << "\n";
    
    // Конструктор копирования
    std::cout << "\nКонструктор копирования:\n";
    Queue<int> copied = original;
    std::cout << "Скопированная очередь: ";
    for (const auto& val : copied) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    std::cout << "  Размер: " << copied.size() << "\n";
    
    // Проверка независимости
    std::cout << "\nПроверка независимости (удаляем элемент из копии):\n";
    copied.pop();
    std::cout << "Оригинальная очередь: ";
    for (const auto& val : original) {
        std::cout << val << " ";
    }
    std::cout << " (размер: " << original.size() << ")\n";
    std::cout << "Скопированная очередь: ";
    for (const auto& val : copied) {
        std::cout << val << " ";
    }
    std::cout << " (размер: " << copied.size() << ")\n";
    
    // Конструктор перемещения
    std::cout << "\nКонструктор перемещения:\n";
    std::cout << "Размер copied перед перемещением: " << copied.size() << "\n";
    Queue<int> moved = std::move(copied);
    std::cout << "Перемещённая очередь: ";
    for (const auto& val : moved) {
        std::cout << val << " ";
    }
    std::cout << " (размер: " << moved.size() << ")\n";
    std::cout << "Источник после перемещения (copied): размер = " << copied.size() 
              << " (должен быть 0)\n";
    
    // Оператор присваивания копированием
    std::cout << "\nОператор присваивания копированием:\n";
    Queue<int> assigned(&memory);
    assigned.push(999);
    std::cout << "До присваивания: ";
    for (const auto& val : assigned) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    assigned = original;
    std::cout << "После присваивания: ";
    for (const auto& val : assigned) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

void demo_iterator_operations() {
    print_separator("ДЕМОНСТРАЦИЯ 5: Операции с итераторами");
    
    FixedMemoryResource memory(4096);
    Queue<int> queue(&memory);
    
    // Заполнение
    std::cout << "Заполнение очереди числами: 5, 10, 15, 20, 25\n\n";
    for (int i = 1; i <= 5; ++i) {
        queue.push(i * 5);
    }
    
    // Префиксный инкремент
    std::cout << "Префиксный инкремент (++it):\n";
    auto it = queue.begin();
    std::cout << "  *it = " << *it << "\n";
    ++it;
    std::cout << "  После ++it: *it = " << *it << "\n";
    
    // Постфиксный инкремент
    std::cout << "\nПостфиксный инкремент (it++):\n";
    auto old_it = it++;
    std::cout << "  old_it (до инкремента): *old_it = " << *old_it << "\n";
    std::cout << "  it (после инкремента): *it = " << *it << "\n";
    
    // Сравнение итераторов
    std::cout << "\nСравнение итераторов:\n";
    auto it1 = queue.begin();
    auto it2 = queue.begin();
    auto it3 = queue.end();
    std::cout << "  it1 == it2: " << (it1 == it2 ? "true" : "false") << "\n";
    std::cout << "  it1 != it3: " << (it1 != it3 ? "true" : "false") << "\n";
    
    // Модификация через итератор
    std::cout << "\nМодификация элементов через итератор (умножение на 2):\n";
    std::cout << "  До: ";
    for (const auto& val : queue) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    for (auto iter = queue.begin(); iter != queue.end(); ++iter) {
        *iter *= 2;
    }
    
    std::cout << "  После: ";
    for (const auto& val : queue) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    // Проход до конца
    std::cout << "\nПроход от begin() до end():\n";
    int step = 1;
    for (auto iter = queue.begin(); iter != queue.end(); ++iter) {
        std::cout << "  Шаг " << step++ << ": *iter = " << *iter << "\n";
    }
}

void demo_edge_cases() {
    print_separator("ДЕМОНСТРАЦИЯ 6: Граничные случаи и обработка ошибок");
    
    FixedMemoryResource memory(4096);
    Queue<int> queue(&memory);
    
    // Попытка доступа к пустой очереди
    std::cout << "Попытка front() на пустой очереди:\n";
    try {
        queue.front();
        std::cout << "  ОШИБКА: исключение не выброшено!\n";
    } catch (const std::runtime_error& e) {
        std::cout << "  Корректно выброшено исключение: " << e.what() << "\n";
    }
    
    std::cout << "\nПопытка pop() на пустой очереди:\n";
    try {
        queue.pop();
        std::cout << "  ОШИБКА: исключение не выброшено!\n";
    } catch (const std::runtime_error& e) {
        std::cout << "  Корректно выброшено исключение: " << e.what() << "\n";
    }
    
    // Один элемент
    std::cout << "\nРабота с одним элементом:\n";
    queue.push(42);
    std::cout << "  Добавлен элемент: 42\n";
    std::cout << "  front() == back(): " << (queue.front() == queue.back() ? "true" : "false") << "\n";
    std::cout << "  Размер: " << queue.size() << "\n";
    queue.pop();
    std::cout << "  После pop() размер: " << queue.size() << "\n";
    std::cout << "  Пуста: " << (queue.empty() ? "true" : "false") << "\n";
    
    // Очистка непустой очереди
    std::cout << "\nОчистка непустой очереди:\n";
    queue.push(1);
    queue.push(2);
    queue.push(3);
    std::cout << "  Размер до clear(): " << queue.size() << "\n";
    queue.clear();
    std::cout << "  Размер после clear(): " << queue.size() << "\n";
    std::cout << "  Пуста: " << (queue.empty() ? "true" : "false") << "\n";
}

int main() {

    
    try {
        demo_simple_type();
        demo_complex_type();
        demo_memory_reuse();
        demo_copy_and_move();
        demo_iterator_operations();
        demo_edge_cases();
        
        print_separator("ВСЕ ДЕМОНСТРАЦИИ ЗАВЕРШЕНЫ УСПЕШНО");
        
    
    } catch (const std::exception& e) {
        std::cerr << "\nОШИБКА: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}