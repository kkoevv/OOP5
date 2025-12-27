#include <gtest/gtest.h>
#include "fixed_memory_resource.h"
#include "queue.h"
#include <string>
#include <type_traits>

// Структура для тестирования со сложным типом
// Содержит несколько полей разных типов
struct Person {
    std::string name;
    int age;
    double salary;
    
    Person() : name(""), age(0), salary(0.0) {}
    Person(std::string n, int a, double s) : name(std::move(n)), age(a), salary(s) {}
    
    bool operator==(const Person& other) const {
        return name == other.name && age == other.age && salary == other.salary;
    }
};

// Набор тестов для FixedMemoryResource
class FixedMemoryResourceTest : public ::testing::Test {
protected:
    FixedMemoryResource* memory_resource;
    
    void SetUp() override {
        memory_resource = new FixedMemoryResource(4096);
    }
    
    void TearDown() override {
        delete memory_resource;
    }
};

TEST_F(FixedMemoryResourceTest, Allocation) {
    void* ptr = memory_resource->allocate(100);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(memory_resource->get_allocated_count(), 1);
    memory_resource->deallocate(ptr, 100);
}

TEST_F(FixedMemoryResourceTest, Deallocation) {
    void* ptr = memory_resource->allocate(100);
    ASSERT_NE(ptr, nullptr);
    
    memory_resource->deallocate(ptr, 100);
    EXPECT_EQ(memory_resource->get_allocated_count(), 0);
    EXPECT_EQ(memory_resource->get_free_count(), 1);
}

TEST_F(FixedMemoryResourceTest, MemoryReuse) {
    void* ptr1 = memory_resource->allocate(100);
    size_t offset_after_first = memory_resource->get_current_offset();
    
    memory_resource->deallocate(ptr1, 100);
    
    void* ptr2 = memory_resource->allocate(100);
    size_t offset_after_second = memory_resource->get_current_offset();
    
    EXPECT_EQ(ptr1, ptr2);
    EXPECT_EQ(offset_after_first, offset_after_second);
    
    memory_resource->deallocate(ptr2, 100);
}

TEST_F(FixedMemoryResourceTest, OutOfMemory) {
    EXPECT_THROW({
        [[maybe_unused]] void* ptr = memory_resource->allocate(5000);
    }, std::bad_alloc);
}

TEST_F(FixedMemoryResourceTest, InvalidDeallocation) {
    int dummy;
    void* invalid_ptr = &dummy;
    EXPECT_THROW(memory_resource->deallocate(invalid_ptr, 100), std::invalid_argument);
}

TEST_F(FixedMemoryResourceTest, MoveConstructor) {
    void* ptr = memory_resource->allocate(100);
    size_t original_count = memory_resource->get_allocated_count();
    
    FixedMemoryResource moved(std::move(*memory_resource));
    
    EXPECT_EQ(moved.get_allocated_count(), original_count);
    EXPECT_EQ(memory_resource->get_allocated_count(), 0);
    
    moved.deallocate(ptr, 100);
}
// Набор тестов для Queue с простым типом (int)
class QueueTest : public ::testing::Test {
protected:
    FixedMemoryResource* memory_resource;
    Queue<int>* queue;
    
    void SetUp() override {
        memory_resource = new FixedMemoryResource(4096);
        queue = new Queue<int>(memory_resource);
    }
    
    void TearDown() override {
        delete queue;
        delete memory_resource;
    }
};

// Тест: пустая очередь
TEST_F(QueueTest, EmptyQueue) {
    EXPECT_TRUE(queue->empty());
    EXPECT_EQ(queue->size(), 0);
}

// Тест: добавление элементов и размер
TEST_F(QueueTest, PushAndSize) {
    queue->push(10);
    EXPECT_FALSE(queue->empty());
    EXPECT_EQ(queue->size(), 1);
    
    queue->push(20);
    EXPECT_EQ(queue->size(), 2);
}

// Тест: доступ к первому и последнему элементу
TEST_F(QueueTest, FrontAndBack) {
    queue->push(10);
    queue->push(20);
    queue->push(30);
    
    EXPECT_EQ(queue->front(), 10);
    EXPECT_EQ(queue->back(), 30);
}

// Тест: удаление элементов
TEST_F(QueueTest, Pop) {
    queue->push(10);
    queue->push(20);
    queue->push(30);
    
    queue->pop();
    EXPECT_EQ(queue->front(), 20);
    EXPECT_EQ(queue->size(), 2);
    
    queue->pop();
    EXPECT_EQ(queue->front(), 30);
    EXPECT_EQ(queue->size(), 1);
}

// Тест: pop из пустой очереди
TEST_F(QueueTest, PopEmpty) {
    EXPECT_THROW(queue->pop(), std::runtime_error);
}

// Тест: front из пустой очереди
TEST_F(QueueTest, FrontEmpty) {
    EXPECT_THROW(queue->front(), std::runtime_error);
}

// Тест: очистка очереди
TEST_F(QueueTest, Clear) {
    queue->push(10);
    queue->push(20);
    queue->push(30);
    
    queue->clear();
    EXPECT_TRUE(queue->empty());
    EXPECT_EQ(queue->size(), 0);
}

// Тест: конструктор копирования
TEST_F(QueueTest, CopyConstructor) {
    queue->push(10);
    queue->push(20);
    queue->push(30);
    
    Queue<int> copied(*queue);
    
    EXPECT_EQ(copied.size(), queue->size());
    EXPECT_EQ(copied.front(), queue->front());
    EXPECT_EQ(copied.back(), queue->back());
    
    // Проверяем независимость копий
    copied.pop();
    EXPECT_NE(copied.size(), queue->size());
}

// Тест: конструктор перемещения
TEST_F(QueueTest, MoveConstructor) {
    queue->push(10);
    queue->push(20);
    size_t original_size = queue->size();
    
    Queue<int> moved(std::move(*queue));
    
    EXPECT_EQ(moved.size(), original_size);
    EXPECT_EQ(queue->size(), 0);
    EXPECT_TRUE(queue->empty());
}

// Тест: оператор присваивания копированием
TEST_F(QueueTest, CopyAssignment) {
    queue->push(10);
    queue->push(20);
    
    Queue<int> assigned(memory_resource);
    assigned = *queue;
    
    EXPECT_EQ(assigned.size(), queue->size());
    EXPECT_EQ(assigned.front(), queue->front());
}

// Тест: оператор присваивания перемещением
TEST_F(QueueTest, MoveAssignment) {
    queue->push(10);
    queue->push(20);
    size_t original_size = queue->size();
    
    Queue<int> assigned(memory_resource);
    assigned = std::move(*queue);
    
    EXPECT_EQ(assigned.size(), original_size);
    EXPECT_EQ(queue->size(), 0);
}

// Тест: итератор begin/end
TEST_F(QueueTest, IteratorBeginEnd) {
    queue->push(10);
    queue->push(20);
    queue->push(30);
    
    auto it = queue->begin();
    EXPECT_EQ(*it, 10);
    
    ++it;
    EXPECT_EQ(*it, 20);
    
    ++it;
    EXPECT_EQ(*it, 30);
    
    ++it;
    EXPECT_EQ(it, queue->end());
}

// Тест: range-based for
TEST_F(QueueTest, IteratorRangeFor) {
    queue->push(10);
    queue->push(20);
    queue->push(30);
    
    int sum = 0;
    for (const auto& value : *queue) {
        sum += value;
    }
    
    EXPECT_EQ(sum, 60);
}

// Тест: модификация через итератор
TEST_F(QueueTest, IteratorModification) {
    queue->push(10);
    queue->push(20);
    queue->push(30);
    
    for (auto it = queue->begin(); it != queue->end(); ++it) {
        *it *= 2;
    }
    
    EXPECT_EQ(queue->front(), 20);
    queue->pop();
    EXPECT_EQ(queue->front(), 40);
    queue->pop();
    EXPECT_EQ(queue->front(), 60);
}

// Тест: постфиксный инкремент итератора
TEST_F(QueueTest, IteratorPostIncrement) {
    queue->push(10);
    queue->push(20);
    
    auto it = queue->begin();
    auto old_it = it++;
    
    EXPECT_EQ(*old_it, 10);
    EXPECT_EQ(*it, 20);
}

// Тест: сравнение итераторов
TEST_F(QueueTest, IteratorComparison) {
    queue->push(10);
    
    auto it1 = queue->begin();
    auto it2 = queue->begin();
    auto it3 = queue->end();
    
    EXPECT_TRUE(it1 == it2);
    EXPECT_TRUE(it1 != it3);
    EXPECT_FALSE(it1 == it3);
}

// Набор тестов для Queue со сложным типом (struct Person)
class QueueComplexTypeTest : public ::testing::Test {
protected:
    FixedMemoryResource* memory_resource;
    Queue<Person>* queue;
    
    void SetUp() override {
        memory_resource = new FixedMemoryResource(8192);
        queue = new Queue<Person>(memory_resource);
    }
    
    void TearDown() override {
        delete queue;
        delete memory_resource;
    }
};

// Тест: добавление и доступ к структуре
TEST_F(QueueComplexTypeTest, PushAndFront) {
    Person alice("Alice", 25, 75000.0);
    queue->push(alice);
    
    EXPECT_EQ(queue->front().name, "Alice");
    EXPECT_EQ(queue->front().age, 25);
    EXPECT_EQ(queue->front().salary, 75000.0);
}

// Тест: семантика перемещения
TEST_F(QueueComplexTypeTest, MoveSemantics) {
    queue->push(Person("Bob", 30, 90000.0));
    
    EXPECT_EQ(queue->front().name, "Bob");
    EXPECT_EQ(queue->size(), 1);
}

// Тест: оператор стрелка итератора
TEST_F(QueueComplexTypeTest, IteratorArrowOperator) {
    queue->push(Person("Charlie", 35, 120000.0));
    
    auto it = queue->begin();
    EXPECT_EQ(it->name, "Charlie");
    EXPECT_EQ(it->age, 35);
}

// Тест: несколько объектов
TEST_F(QueueComplexTypeTest, MultiplePeople) {
    queue->push(Person("Alice", 25, 75000.0));
    queue->push(Person("Bob", 30, 90000.0));
    queue->push(Person("Charlie", 35, 120000.0));
    
    EXPECT_EQ(queue->size(), 3);
    EXPECT_EQ(queue->front().name, "Alice");
    EXPECT_EQ(queue->back().name, "Charlie");
}

// Интеграционный тест: циклическое добавление и удаление
TEST(MemoryReuseIntegrationTest, CyclicPushPop) {
    FixedMemoryResource memory(1024);
    Queue<int> queue(&memory);
    
    // Добавляем 5 элементов
    for (int i = 0; i < 5; ++i) {
        queue.push(i);
    }
    
    size_t offset_after_first_batch = memory.get_current_offset();
    
    // Удаляем 3 элемента
    for (int i = 0; i < 3; ++i) {
        queue.pop();
    }
    
    // Добавляем ещё 3 элемента
    for (int i = 10; i < 13; ++i) {
        queue.push(i);
    }
    
    size_t offset_after_reuse = memory.get_current_offset();
    
    // Проверяем, что offset не изменился (память переиспользована)
    EXPECT_EQ(offset_after_first_batch, offset_after_reuse);
}

// Тест: проверка категории итератора
TEST(IteratorConceptTest, ForwardIteratorRequirements) {
    FixedMemoryResource memory(1024);
    Queue<int> queue(&memory);
    
    queue.push(10);
    queue.push(20);
    
    auto it = queue.begin();
    
    // Проверяем, что категория итератора - forward_iterator_tag
    EXPECT_TRUE((std::is_same_v<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::forward_iterator_tag
    >));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}