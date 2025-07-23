#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <cmath>
#include <doctest/doctest.h>

#include "orm_rttr.hpp"

// --- 实体定义 (Entities) ---
struct User {
    long long id;
    std::string name;
    int age;
    std::string email;  // 改为std::string，不使用std::optional
    bool is_deleted;
    long long version;
    int score;  // 新增字段，用于测试负数空值
    std::chrono::system_clock::time_point create_time;
    std::chrono::system_clock::time_point update_time;
};

// --- RTTR 注册与元数据配置 ---
RTTR_REGISTRATION {
    using namespace orm_rttr;
    rttr::registration::class_<User>("User")
        // 类级别元数据
        (
            rttr::metadata(meta::TABLE_NAME, "users"),
            rttr::metadata(meta::USE_LOGICAL_DELETE, true),
            rttr::metadata(meta::LOGICAL_DELETE_FIELD, "is_deleted"),
            rttr::metadata(meta::USE_VERSION, true),
            rttr::metadata(meta::VERSION_FIELD, "version"),
            rttr::metadata(meta::CREATE_TIME_FIELD, "create_time"),
            rttr::metadata(meta::UPDATE_TIME_FIELD, "update_time")
        )
        // 属性级别元数据
        .property("id", &User::id)
            (
                rttr::metadata(meta::PRIMARY_KEY, true),
                rttr::metadata(meta::AUTO_INCREMENT, true)
            )
        .property("name", &User::name)
        .property("age", &User::age)
        .property("email", &User::email)
        .property("is_deleted", &User::is_deleted)
        .property("version", &User::version)
        .property("score", &User::score)
            (
                rttr::metadata(meta::NEGATIVE_AS_NULL, true),
                rttr::metadata(meta::NULL_VALUE, -1)  // 将-1视为NULL值
            )
        .property("create_time", &User::create_time)
        .property("update_time", &User::update_time);
}

// 将SQL语句中的占位符?替换为实际的参数值
std::string formatSql(const std::string& sql, const std::vector<orm_rttr::ValueVariant>& params) {
    std::string result = sql;
    size_t pos = 0;
    size_t param_index = 0;
    
    while ((pos = result.find('?', pos)) != std::string::npos && param_index < params.size()) {
        std::string param_str;
        std::visit([&param_str](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            std::stringstream ss;
            if constexpr (std::is_same_v<T, std::monostate>)
                ss << "NULL";
            else if constexpr (std::is_same_v<T, std::string>)
                ss << "'" << arg << "'";
            else if constexpr (std::is_same_v<T, bool>)
                ss << (arg ? "TRUE" : "FALSE");
            else if constexpr (std::is_same_v<T, std::chrono::system_clock::time_point>) {
                auto time_t = std::chrono::system_clock::to_time_t(arg);
                char buffer[80];
                std::strftime(buffer, sizeof(buffer), "'%Y-%m-%d %H:%M:%S'", std::localtime(&time_t));
                ss << buffer;
            }
            else
                ss << arg;
            param_str = ss.str();
        }, params[param_index]);
        
        result.replace(pos, 1, param_str);
        pos += param_str.length();
        param_index++;
    }
    
    return result;
}

// 输出SQL查询结果
void print_sql(const std::string& title, const orm_rttr::SqlQueryResult& res) {
    std::cout << "\n===== " << title << " =====\n";
    std::cout << "[原始SQL]: " << res.sql << "\n";
    
    // 输出参数列表
    std::cout << "[参数列表]: ";
    if (res.params.empty()) {
        std::cout << "无参数";
    } else {
        std::cout << "\n";
        for (size_t i = 0; i < res.params.size(); i++) {
            std::cout << "  参数[" << i << "]: ";
            std::visit([](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) 
                    std::cout << "NULL";
                else if constexpr (std::is_same_v<T, std::string>) 
                    std::cout << "'" << arg << "'";
                else if constexpr (std::is_same_v<T, bool>) 
                    std::cout << (arg ? "TRUE" : "FALSE");
                else if constexpr (std::is_same_v<T, std::chrono::system_clock::time_point>) {
                    auto time_t = std::chrono::system_clock::to_time_t(arg);
                    char buffer[80];
                    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
                    std::cout << "TIMESTAMP '" << buffer << "'";
                }
                else 
                    std::cout << arg;
            }, res.params[i]);
            std::cout << "\n";
        }
    }
    
    // 输出格式化后的SQL（将?替换为实际参数）
    std::cout << "[最终SQL]: " << formatSql(res.sql, res.params) << "\n";
    std::cout << "=========================\n\n";
}


// --- 测试ORM功能 ---
TEST_CASE("ORM功能测试") {
    std::cout << "\n===== ORM功能测试 =====\n";
    
    using namespace orm_rttr;

    try {
        // 1. 插入操作
        User user_to_insert;
        user_to_insert.name = "Alice";
        user_to_insert.age = 30;
        user_to_insert.email = ""; // 空字符串代替std::nullopt
        user_to_insert.score = 100; // 正常分数
        auto insert_res = OrmService<User>::insert(user_to_insert);
        print_sql("1. 插入用户", insert_res);

        // 2. 批量插入
        std::vector<User> users_to_batch_insert = { 
            {0, "Bob", 42, "", false, 0, 80}, 
            {0, "Charlie", 25, "", false, 0, -1} // Charlie的score是-1，将被视为NULL
        };
        auto batch_insert_res = OrmService<User>::batchInsert(users_to_batch_insert);
        print_sql("2. 批量插入用户", batch_insert_res);

        // 3. 根据ID更新所有字段
        User user_to_update_full;
        user_to_update_full.id = 1;
        user_to_update_full.name = "Alice V2";
        user_to_update_full.age = 31;
        user_to_update_full.email = "";
        user_to_update_full.score = -1; // 将分数设为-1，应该被转换为NULL
        user_to_update_full.version = 1; // 假设从数据库读到的版本是1
        auto update_full_res = OrmService<User>::updateById(user_to_update_full);
        print_sql("3. 根据ID更新全部字段", update_full_res);

        // 4. 根据ID更新部分字段，包含空值字段
        User user_to_update_partial;
        user_to_update_partial.id = 2;
        user_to_update_partial.age = 43; // 只想更新年龄
        user_to_update_partial.score = 95; // 正常分数
        user_to_update_partial.version = 1;
        auto update_partial_res = OrmService<User>::updateFieldsById(user_to_update_partial, {"age", "score"});
        print_sql("4. 根据ID更新部分字段", update_partial_res);

        // 4.1 更新时包含空值字段（应该被排除）
        User user_with_null;
        user_with_null.id = 3;
        user_with_null.name = "Updated Name";
        user_with_null.score = -1; // 这是空值，应该不被包含在更新字段中
        user_with_null.version = 1;
        auto update_with_null = OrmService<User>::updateFieldsById(user_with_null, {"name", "score"});
        print_sql("4.1 更新包含空值字段", update_with_null);

        // 5. 根据ID删除 (逻辑删除)
        auto delete_res = OrmService<User>::deleteById(3, 2LL); // 假设版本号是2
        print_sql("5. 根据ID删除（逻辑删除）", delete_res);

        // 6. 根据ID查询 (带行锁)
        auto get_by_id_res = OrmService<User>::getById(4, LockMode::ForUpdate);
        print_sql("6. 根据ID查询（带行锁）", get_by_id_res);

        // 7. 复杂条件查询
        QueryWrapper<User> wrapper;
        wrapper.ge("age", 30)
               .like("name", "A%")
               .orderBy("age", OrderDirection::DESC);
        auto select_cond_res = OrmService<User>::selectByCondition(wrapper);
        print_sql("7. 复杂条件查询", select_cond_res);

        // 8. 分页查询
        QueryWrapper<User> page_wrapper;
        page_wrapper.gt("age", 20);
        PageParam page = {2, 5}; // 第2页，每页5条
        auto [count_res, data_res] = OrmService<User>::selectPage(page_wrapper, page);
        print_sql("8.1. 分页查询（计数）", count_res);
        print_sql("8.2. 分页查询（数据）", data_res);

        // 9. IN查询
        QueryWrapper<User> in_wrapper;
        in_wrapper.in("id", std::vector<long long>{10, 20, 30});
        auto in_res = OrmService<User>::selectByCondition(in_wrapper);
        print_sql("9. IN条件查询", in_res);

        // 9.1 带读锁的条件查询
        QueryWrapper<User> read_lock_wrapper;
        read_lock_wrapper.ge("age", 25);
        auto read_lock_res = OrmService<User>::selectByCondition(read_lock_wrapper, LockMode::ForShare);
        print_sql("9.1 带读锁的条件查询", read_lock_res);
        
        // 9.2 带写锁的条件查询
        QueryWrapper<User> write_lock_wrapper;
        write_lock_wrapper.eq("name", "Bob");
        auto write_lock_res = OrmService<User>::selectByCondition(write_lock_wrapper, LockMode::ForUpdate);
        print_sql("9.2 带写锁的条件查询", write_lock_res);

        // 9.3 带写锁的分页查询
        QueryWrapper<User> page_lock_wrapper;
        page_lock_wrapper.gt("age", 30);
        PageParam lock_page = {1, 10};
        auto [lock_count_res, lock_data_res] = OrmService<User>::selectPage(page_lock_wrapper, lock_page, LockMode::ForUpdate);
        print_sql("9.3 带锁的分页查询（计数）", lock_count_res);
        print_sql("9.3 带锁的分页查询（数据）", lock_data_res);

        // 10. 测试电子邮件
        User user_with_email;
        user_with_email.name = "David";
        user_with_email.age = 35;
        user_with_email.email = "david@example.com";
        user_with_email.score = -99; // 负分数，也会被视为NULL
        auto email_res = OrmService<User>::insert(user_with_email);
        print_sql("10. 插入带电子邮件的用户", email_res);

        // 11. 测试不同负值作为空值
        QueryWrapper<User> negative_wrapper;
        negative_wrapper.eq("score", -1); // 这应该会查询score为NULL的记录
        auto negative_res = OrmService<User>::selectByCondition(negative_wrapper);
        print_sql("11. 查询空值字段（使用负值）", negative_res);
    } catch (const std::exception& e) {
        std::cout << "生成SQL时出错: " << e.what() << std::endl;
    }
}