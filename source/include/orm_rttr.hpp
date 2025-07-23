#ifndef ORM_RTTR_HPP
#define ORM_RTTR_HPP

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <chrono>
#include <functional>
#include <memory>
#include <algorithm>
#include <cmath> // For std::ceil

// 引入 RTTR 库的核心头文件
#include <rttr/registration>
#include <rttr/type>


namespace orm_rttr {

// ========== 核心数据类型和定义 ==========

// 扩展ValueVariant以支持更多类型
using ValueVariant = std::variant<
    std::monostate,  // 表示NULL
    int,
    long,
    long long,
    double,
    std::string,
    bool,
    std::chrono::system_clock::time_point
>;

// 数据库行表示
class DbRow {
private:
    std::unordered_map<std::string, ValueVariant> values;

public:
    DbRow() = default;

    void setValue(const std::string& column, const ValueVariant& value) {
        values[column] = value;
    }

    bool hasColumn(const std::string& column) const {
        return values.find(column) != values.end();
    }

    const ValueVariant& getValue(const std::string& column) const {
        auto it = values.find(column);
        if (it == values.end()) {
            throw std::runtime_error("Column not found: " + column);
        }
        return it->second;
    }

    template<typename T>
    T getValueAs(const std::string& column) const {
        const ValueVariant& variant = getValue(column);
        if (std::holds_alternative<std::monostate>(variant)) {
            // 返回默认值，如果需要NULL值处理则可以特化
            return T();
        }
        return std::get<T>(variant);
    }
};

// 结果集表示
using ResultSet = std::vector<DbRow>;

// SQL查询结果
struct SqlQueryResult {
    std::string sql;
    std::vector<ValueVariant> params;
};

// 锁定模式枚举
enum class LockMode { None, ForUpdate, ForShare };

// 查询条件枚举（已根据要求重命名）
enum class QueryOperator {
    __op_EQ, __op_NE, __op_GT, __op_GE, __op_LT, __op_LE,
    __op_LIKE, __op_NOT_LIKE,
    __op_IN, __op_NOT_IN,
    __op_IS_NULL, __op_IS_NOT_NULL,
    __op_BETWEEN, __op_NOT_BETWEEN
};

// 逻辑运算符枚举
enum class LogicOperator { AND, OR };

// 排序方向
enum class OrderDirection { ASC, DESC };

// ========== 分页相关结构 ==========

// 分页参数
struct PageParam {
    int page_index = 1; // 当前页码，从1开始
    int page_size = 10;  // 每页记录数
};

// 分页结果
template<typename T>
struct PageResult {
    std::vector<T> records;   // 当前页记录
    long long total;          // 总记录数
    int pages;                // 总页数
    int current;              // 当前页码
    int size;                 // 每页记录数
    bool has_next;            // 是否有下一页
    bool has_previous;        // 是否有上一页

    // 辅助函数，用于从数据库结果构建分页对象
    static PageResult<T> build(std::vector<T> recs, long long total_records, const PageParam& page) {
        PageResult<T> res;
        res.records = std::move(recs);
        res.total = total_records;
        res.size = page.page_size;
        res.current = page.page_index;
        res.pages = (total_records > 0 && page.page_size > 0) ? static_cast<int>(std::ceil(static_cast<double>(total_records) / page.page_size)) : 0;
        res.has_previous = res.current > 1;
        res.has_next = res.current < res.pages;
        return res;
    }
};

// ========== ORM元数据键 ==========
namespace meta {
    const char* const TABLE_NAME = "table_name";                  // 表名
    const char* const PRIMARY_KEY = "primary_key";                // 主键标记
    const char* const COLUMN_NAME = "column_name";                // 列名（与属性名不同时使用）
    const char* const AUTO_INCREMENT = "auto_increment";          // 自增标记
    const char* const USE_LOGICAL_DELETE = "use_logical_delete";  // 使用逻辑删除
    const char* const LOGICAL_DELETE_FIELD = "logical_delete_field"; // 逻辑删除字段名
    const char* const USE_VERSION = "use_version";                // 使用版本控制
    const char* const VERSION_FIELD = "version_field";            // 版本字段名
    const char* const CREATE_TIME_FIELD = "create_time_field";    // 创建时间字段名
    const char* const UPDATE_TIME_FIELD = "update_time_field";    // 更新时间字段名
    // 添加用于标记负数空值的元数据键
    const char* const NEGATIVE_AS_NULL = "negative_as_null";      // 将负数视为NULL值标记（布尔值）
    const char* const NULL_VALUE = "null_value";                  // 指定特定的负值作为NULL（如-1、-999等）
}

// ========== 辅助函数 ==========
namespace internal {
    // 通用模板函数处理数值类型的负数空值检测
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
    ValueVariant handle_numeric_value(const T& val, const rttr::property* prop) {
        // 如果属性有效，检查是否有负数空值标记
        if (prop && prop->is_valid() && prop->get_metadata(meta::NEGATIVE_AS_NULL).to_bool()) {
            // 获取空值标记
            auto null_val = prop->get_metadata(meta::NULL_VALUE);
            // 如果有指定具体空值，则比较是否相等
            if (null_val.is_valid()) {
                T null_value;
                if (null_val.convert(null_value)) {
                    if (val == null_value) {
                        return std::monostate{};
                    }
                }
            } 
            // 如果没有指定具体空值，则所有负值视为空
            else if constexpr (std::is_signed_v<T>) {
                if (val < 0) {
                    return std::monostate{};
                }
            }
        }
        return val;
    }
    
    // 特化版本：布尔类型不需要检查负数
    template<typename T, typename = void>
    typename std::enable_if_t<std::is_same_v<T, bool>, ValueVariant> 
    handle_numeric_value(const T& val, const rttr::property*) {
        return val;
    }
    
    // 特化版本：非算术类型直接返回
    template<typename T>
    typename std::enable_if_t<!std::is_arithmetic_v<T>, ValueVariant> 
    handle_numeric_value(const T& val, const rttr::property*) {
        return val;
    }

    // 将rttr::variant转换为我们的ValueVariant
    inline ValueVariant rttr_to_value_variant(rttr::variant v, const rttr::property* prop = nullptr) {
        if (!v.is_valid()) return std::monostate{};

        rttr::type t = v.get_type();
        
        // 高效处理基本类型
        if (t == rttr::type::get<int>())
            return handle_numeric_value(v.get_value<int>(), prop);
        
        if (t == rttr::type::get<long>())
            return handle_numeric_value(v.get_value<long>(), prop);
        
        if (t == rttr::type::get<long long>())
            return handle_numeric_value(v.get_value<long long>(), prop);
        
        if (t == rttr::type::get<double>())
            return handle_numeric_value(v.get_value<double>(), prop);
        
        if (t == rttr::type::get<float>())
            return handle_numeric_value(v.get_value<float>(), prop);
        
        if (t == rttr::type::get<char>()) // 添加对char类型的支持
            return std::string(1, v.get_value<char>());
        
        if (t == rttr::type::get<std::string>()) 
            return v.get_value<std::string>();
        
        if (t == rttr::type::get<bool>()) 
            return v.get_value<bool>();
        
        if (t == rttr::type::get<std::chrono::system_clock::time_point>()) 
            return v.get_value<std::chrono::system_clock::time_point>();
        
        // 快速检查是否为optional类型
        std::string type_name = t.get_name().to_string();
        bool is_optional = t.is_wrapper() && type_name.find("optional") != std::string::npos;
        
        // 专门处理 std::optional<std::string>，这是比较常见的情况
        if (is_optional && type_name.find("string") != std::string::npos) {
            try {
                rttr::variant extracted = v.extract_wrapped_value();
                if (extracted.is_valid() && extracted.can_convert<std::string>()) {
                    return extracted.to_string();
                }
            } catch (...) {
                // 处理失败时返回空值
            }
            return std::monostate{};
        }
        
        // 处理其它所有optional类型
        if (is_optional) {
            try {
                rttr::variant extracted = v.extract_wrapped_value();
                if (extracted.is_valid()) {
                    return rttr_to_value_variant(extracted, prop);
                }
            } catch (...) {
                // 处理失败时返回空值
            }
            return std::monostate{};
        }
        
        throw std::runtime_error("Unsupported type for ValueVariant conversion: " + type_name);
    }

    // 获取实体元数据，带默认值和错误检查
    inline std::string get_table_name(rttr::type t) {
        auto meta = t.get_metadata(meta::TABLE_NAME);
        if (!meta) throw std::runtime_error("Metadata '" + std::string(meta::TABLE_NAME) + "' not found for type " + std::string(t.get_name()));
        return meta.to_string();
    }

    inline rttr::property get_prop(rttr::type t, const std::string& name) {
        rttr::property prop = t.get_property(name);
        if (!prop) throw std::runtime_error("Property '" + name + "' not found in type " + std::string(t.get_name()));
        return prop;
    }

    inline std::string get_column_name(rttr::property prop) {
        auto meta = prop.get_metadata(meta::COLUMN_NAME);
        return meta ? meta.to_string() : prop.get_name().to_string();
    }

    inline rttr::property get_primary_key_prop(rttr::type t) {
        for (auto& prop : t.get_properties()) {
            if (prop.get_metadata(meta::PRIMARY_KEY).to_bool()) {
                return prop;
            }
        }
        throw std::runtime_error("No primary key defined for type " + std::string(t.get_name()));
    }
} // namespace internal

// ========== 查询条件包装器 ==========
template<typename Entity>
class QueryWrapper {
private:
    using Condition = std::tuple<std::string, QueryOperator, std::vector<ValueVariant>>;
    
protected:
    std::vector<Condition> m_conditions;
    std::vector<std::pair<std::string, OrderDirection>> m_order_by;
    LogicOperator m_logic_op = LogicOperator::AND;
    int m_limit = -1;
    int m_offset = -1;
    std::vector<std::string> m_group_by; // 用于存储GROUP BY字段

    // 将任意类型转换为ValueVariant的辅助函数
    template<typename T>
    static ValueVariant to_val(const T& value) {
        rttr::variant v = value;
        return internal::rttr_to_value_variant(v);
    }

public:
    QueryWrapper() = default;

    // 所有条件方法
    template<typename T> QueryWrapper& eq(const std::string& field, const T& value) { m_conditions.emplace_back(field, QueryOperator::__op_EQ, std::vector{to_val(value)}); return *this; }
    template<typename T> QueryWrapper& ne(const std::string& field, const T& value) { m_conditions.emplace_back(field, QueryOperator::__op_NE, std::vector{to_val(value)}); return *this; }
    template<typename T> QueryWrapper& gt(const std::string& field, const T& value) { m_conditions.emplace_back(field, QueryOperator::__op_GT, std::vector{to_val(value)}); return *this; }
    template<typename T> QueryWrapper& ge(const std::string& field, const T& value) { m_conditions.emplace_back(field, QueryOperator::__op_GE, std::vector{to_val(value)}); return *this; }
    template<typename T> QueryWrapper& lt(const std::string& field, const T& value) { m_conditions.emplace_back(field, QueryOperator::__op_LT, std::vector{to_val(value)}); return *this; }
    template<typename T> QueryWrapper& le(const std::string& field, const T& value) { m_conditions.emplace_back(field, QueryOperator::__op_LE, std::vector{to_val(value)}); return *this; }
    QueryWrapper& like(const std::string& field, const std::string& pattern) { m_conditions.emplace_back(field, QueryOperator::__op_LIKE, std::vector<ValueVariant>{to_val(pattern)}); return *this; }
    QueryWrapper& notLike(const std::string& field, const std::string& pattern) { m_conditions.emplace_back(field, QueryOperator::__op_NOT_LIKE, std::vector<ValueVariant>{to_val(pattern)}); return *this; }
    QueryWrapper& isNull(const std::string& field) { m_conditions.emplace_back(field, QueryOperator::__op_IS_NULL, std::vector<ValueVariant>{}); return *this; }
    QueryWrapper& isNotNull(const std::string& field) { m_conditions.emplace_back(field, QueryOperator::__op_IS_NOT_NULL, std::vector<ValueVariant>{}); return *this; }

    template<typename T>
    QueryWrapper& in(const std::string& field, const std::vector<T>& values) {
        std::vector<ValueVariant> variants;
        std::transform(values.begin(), values.end(), std::back_inserter(variants), [](const T& v){ return to_val(v); });
        m_conditions.emplace_back(field, QueryOperator::__op_IN, variants);
        return *this;
    }

    template<typename T>
    QueryWrapper& notIn(const std::string& field, const std::vector<T>& values) {
        std::vector<ValueVariant> variants;
        std::transform(values.begin(), values.end(), std::back_inserter(variants), [](const T& v){ return to_val(v); });
        m_conditions.emplace_back(field, QueryOperator::__op_NOT_IN, variants);
        return *this;
    }

    template<typename T1, typename T2>
    QueryWrapper& between(const std::string& field, const T1& start, const T2& end) {
        m_conditions.emplace_back(field, QueryOperator::__op_BETWEEN, std::vector{to_val(start), to_val(end)});
        return *this;
    }

    template<typename T1, typename T2>
    QueryWrapper& notBetween(const std::string& field, const T1& start, const T2& end) {
        m_conditions.emplace_back(field, QueryOperator::__op_NOT_BETWEEN, std::vector{to_val(start), to_val(end)});
        return *this;
    }

    // 逻辑与排序
    QueryWrapper& and_() { m_logic_op = LogicOperator::AND; return *this; }
    QueryWrapper& or_() { m_logic_op = LogicOperator::OR; return *this; }
    QueryWrapper& orderBy(const std::string& field, OrderDirection dir = OrderDirection::ASC) { m_order_by.emplace_back(field, dir); return *this; }
    
    // 添加groupBy方法
    QueryWrapper& groupBy(const std::string& field) {
        if (!m_group_by.empty()) {
            m_group_by.push_back(field);
        } else {
            m_group_by = {field};
        }
        return *this; 
    }

    QueryWrapper& limit(int count) { m_limit = count; return *this; }
    QueryWrapper& offset(int start) { m_offset = start; return *this; }

    // SQL生成
    std::pair<std::string, std::vector<ValueVariant>> generateConditionSql() const {
        std::stringstream sql;
        std::vector<ValueVariant> params;
        if (m_conditions.empty()) return {"", {}};

        sql << " WHERE ";
        rttr::type t = rttr::type::get<Entity>();

        for (size_t i = 0; i < m_conditions.size(); ++i) {
            if (i > 0) sql << (m_logic_op == LogicOperator::AND ? " AND " : " OR ");

            const auto& [field_name, op, values] = m_conditions[i];
            std::string col_name = internal::get_column_name(internal::get_prop(t, field_name));
            sql << "`" << col_name << "`";

            switch (op) {
                case QueryOperator::__op_EQ: sql << " = ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_NE: sql << " != ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_GT: sql << " > ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_GE: sql << " >= ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_LT: sql << " < ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_LE: sql << " <= ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_LIKE: sql << " LIKE ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_NOT_LIKE: sql << " NOT LIKE ?"; params.push_back(values[0]); break;
                case QueryOperator::__op_IS_NULL: sql << " IS NULL"; break;
                case QueryOperator::__op_IS_NOT_NULL: sql << " IS NOT NULL"; break;
                case QueryOperator::__op_BETWEEN: sql << " BETWEEN ? AND ?"; params.push_back(values[0]); params.push_back(values[1]); break;
                case QueryOperator::__op_NOT_BETWEEN: sql << " NOT BETWEEN ? AND ?"; params.push_back(values[0]); params.push_back(values[1]); break;
                case QueryOperator::__op_IN:
                case QueryOperator::__op_NOT_IN:
                    sql << (op == QueryOperator::__op_IN ? " IN (" : " NOT IN (");
                    for (size_t j = 0; j < values.size(); ++j) {
                        sql << (j > 0 ? ",?" : "?");
                        params.push_back(values[j]);
                    }
                    sql << ")";
                    break;
            }
        }
        return {sql.str(), params};
    }

    SqlQueryResult getSelectSql(const std::vector<std::string>& select_cols = {"*"}) const {
        std::stringstream sql;
        sql << "SELECT ";
        for(size_t i = 0; i < select_cols.size(); ++i) {
            sql << (i > 0 ? "," : "") << select_cols[i];
        }
        sql << " FROM `" << internal::get_table_name(rttr::type::get<Entity>()) << "`";

        auto [cond_sql, params] = generateConditionSql();
        sql << cond_sql;

        if (!m_order_by.empty()) {
            sql << " ORDER BY ";
            for (size_t i = 0; i < m_order_by.size(); ++i) {
                if (i > 0) sql << ", ";
                sql << "`" << internal::get_column_name(internal::get_prop(rttr::type::get<Entity>(), m_order_by[i].first)) << "`"
                    << (m_order_by[i].second == OrderDirection::ASC ? " ASC" : " DESC");
            }
        }
        if (m_limit > -1) sql << " LIMIT " << m_limit;
        if (m_offset > -1) sql << " OFFSET " << m_offset;
        return {sql.str(), params};
    }

    SqlQueryResult getCountSql() const {
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM `" << internal::get_table_name(rttr::type::get<Entity>()) << "`";
        auto [cond_sql, params] = generateConditionSql();
        sql << cond_sql;
        return {sql.str(), params};
    }
};

// ========== JOIN查询条件包装器 ==========
template<typename MainEntity>
class JoinQueryWrapper : public QueryWrapper<MainEntity> {
private:
    struct JoinInfo {
        std::string joinType;      // "INNER", "LEFT", "RIGHT"
        std::string tableName;     // 表名
        std::string tableAlias;    // 表别名
        std::string onCondition;   // 连接条件
    };
    
    std::vector<JoinInfo> m_joins;
    std::vector<std::string> m_selectColumns;
    std::string m_mainTableAlias;
    std::vector<std::string> m_groupByFields;  // 改名，避免与基类m_group_by冲突
    std::string m_having;
    
public:
    JoinQueryWrapper() : QueryWrapper<MainEntity>() {
        // 默认使用主表别名
        m_mainTableAlias = "t";
    }
    
    // 设置主表别名
    JoinQueryWrapper& mainTableAlias(const std::string& alias) {
        m_mainTableAlias = alias;
        return *this;
    }
    
    // 添加INNER JOIN
    template<typename JoinEntity>
    JoinQueryWrapper& innerJoin(const std::string& alias) {
        rttr::type t = rttr::type::get<JoinEntity>();
        std::string tableName = internal::get_table_name(t);
        m_joins.push_back({
            "INNER JOIN", 
            tableName, 
            alias,
            "" // 暂不设置ON条件
        });
        return *this;
    }
    
    // 添加LEFT JOIN
    template<typename JoinEntity>
    JoinQueryWrapper& leftJoin(const std::string& alias) {
        rttr::type t = rttr::type::get<JoinEntity>();
        std::string tableName = internal::get_table_name(t);
        m_joins.push_back({
            "LEFT JOIN", 
            tableName, 
            alias,
            ""
        });
        return *this;
    }
    
    // 添加RIGHT JOIN
    template<typename JoinEntity>
    JoinQueryWrapper& rightJoin(const std::string& alias) {
        rttr::type t = rttr::type::get<JoinEntity>();
        std::string tableName = internal::get_table_name(t);
        m_joins.push_back({
            "RIGHT JOIN", 
            tableName, 
            alias,
            ""
        });
        return *this;
    }
    
    // 设置JOIN条件
    JoinQueryWrapper& on(const std::string& condition) {
        if (!m_joins.empty()) {
            m_joins.back().onCondition = condition;
        }
        return *this;
    }
    
    // 添加查询列
    JoinQueryWrapper& select(const std::vector<std::string>& columns) {
        m_selectColumns = columns;
        return *this;
    }
    
    // 添加分组条件
    JoinQueryWrapper& groupBy(const std::string& field) {
        m_groupByFields.push_back(field);
        return *this;
    }
    
    // 添加分组条件（多字段）
    JoinQueryWrapper& groupBy(const std::vector<std::string>& fields) {
        m_groupByFields.insert(m_groupByFields.end(), fields.begin(), fields.end());
        return *this;
    }
    
    // 添加HAVING条件
    JoinQueryWrapper& having(const std::string& condition) {
        m_having = condition;
        return *this;
    }
    
    // 生成JOIN SQL
    SqlQueryResult getJoinSql() const {
        std::string mainTable = internal::get_table_name(rttr::type::get<MainEntity>());
        
        std::stringstream sql;
        
        // 构建SELECT子句
        sql << "SELECT ";
        if (m_selectColumns.empty()) {
            sql << m_mainTableAlias << ".*";
        } else {
            for (size_t i = 0; i < m_selectColumns.size(); ++i) {
                if (i > 0) sql << ", ";
                sql << m_selectColumns[i];
            }
        }
        
        // FROM子句
        sql << " FROM `" << mainTable << "` " << m_mainTableAlias;
        
        // JOIN子句
        for (const auto& join : m_joins) {
            sql << " " << join.joinType << " `" << join.tableName 
                << "` " << join.tableAlias << " ON " << join.onCondition;
        }
        
        // WHERE条件
        auto [whereSql, params] = this->generateConditionSql();
        if (!whereSql.empty()) {
            sql << whereSql;
        }
        
        // GROUP BY子句
        if (!m_groupByFields.empty()) {
            sql << " GROUP BY ";
            for (size_t i = 0; i < m_groupByFields.size(); ++i) {
                if (i > 0) sql << ", ";
                sql << m_groupByFields[i];
            }
        }
        
        // HAVING子句
        if (!m_having.empty()) {
            sql << " HAVING " << m_having;
        }
        
        // ORDER BY子句
        if (!this->m_order_by.empty()) {
            sql << " ORDER BY ";
            for (size_t i = 0; i < this->m_order_by.size(); ++i) {
                if (i > 0) sql << ", ";
                sql << this->m_order_by[i].first;
                sql << (this->m_order_by[i].second == OrderDirection::ASC ? " ASC" : " DESC");
            }
        }
        
        // LIMIT和OFFSET子句
        if (this->m_limit > 0) {
            sql << " LIMIT " << this->m_limit;
            if (this->m_offset >= 0) {
                sql << " OFFSET " << this->m_offset;
            }
        }
        
        return {sql.str(), params};
    }
    
    // 生成JOIN计数SQL
    SqlQueryResult getJoinCountSql() const {
        std::string mainTable = internal::get_table_name(rttr::type::get<MainEntity>());
        
        std::stringstream sql;
        
        // 构建COUNT查询
        sql << "SELECT COUNT(*) FROM `" << mainTable << "` " << m_mainTableAlias;
        
        // JOIN子句
        for (const auto& join : m_joins) {
            sql << " " << join.joinType << " `" << join.tableName 
                << "` " << join.tableAlias << " ON " << join.onCondition;
        }
        
        // WHERE条件
        auto [whereSql, params] = this->generateConditionSql();
        if (!whereSql.empty()) {
            sql << whereSql;
        }
        
        return {sql.str(), params};
    }
};

// 结果映射类
template<typename MainEntity>
class JoinResultMapper {
private:
    // 映射规则
    struct ColumnMapping {
        std::string columnPrefix;  // 列前缀，如 "t."
        std::string propertyName;  // 属性名
    };
    
    using EntityMappings = std::vector<ColumnMapping>;
    std::unordered_map<rttr::type, EntityMappings> mappings;

public:
    JoinResultMapper() {
        // 默认映射主实体的所有属性
        addEntityMapping<MainEntity>("t");
    }
    
    // 添加实体映射
    template<typename EntityType>
    JoinResultMapper& addEntityMapping(const std::string& prefix) {
        rttr::type entityType = rttr::type::get<EntityType>();
        EntityMappings entityMappings;
        
        for (auto& prop : entityType.get_properties()) {
            std::string columnName = internal::get_column_name(prop);
            entityMappings.push_back({
                prefix + "." + columnName,
                prop.get_name().to_string()
            });
        }
        
        mappings[entityType] = std::move(entityMappings);
        return *this;
    }
    
    // 添加自定义列映射
    template<typename EntityType>
    JoinResultMapper& addColumnMapping(const std::string& columnName, const std::string& propertyName) {
        rttr::type entityType = rttr::type::get<EntityType>();
        if (mappings.find(entityType) == mappings.end()) {
            mappings[entityType] = EntityMappings();
        }
        
        mappings[entityType].push_back({columnName, propertyName});
        return *this;
    }
    
    // 从结果集映射单个对象
    template<typename T>
    T mapToObject(const DbRow& row) const {
        T obj;
        rttr::type objType = rttr::type::get<T>();
        
        auto it = mappings.find(objType);
        if (it == mappings.end()) {
            throw std::runtime_error("No mapping defined for type: " + std::string(objType.get_name()));
        }
        
        for (const auto& mapping : it->second) {
            if (!row.hasColumn(mapping.columnPrefix)) {
                continue;  // 跳过不存在的列
            }
            
            auto prop = objType.get_property(mapping.propertyName);
            if (!prop.is_valid()) {
                continue;  // 跳过不存在的属性
            }
            
            const ValueVariant& value = row.getValue(mapping.columnPrefix);
            // 将ValueVariant转换为rttr::variant并设置属性值
            rttr::variant var;
            if (std::holds_alternative<std::monostate>(value)) {
                // 空值处理
                // var = rttr::variant(); // 默认无效值
            } 
            else if (std::holds_alternative<int>(value)) {
                var = std::get<int>(value);
            }
            else if (std::holds_alternative<long>(value)) {
                var = std::get<long>(value);
            }
            else if (std::holds_alternative<long long>(value)) {
                var = std::get<long long>(value);
            }
            else if (std::holds_alternative<double>(value)) {
                var = std::get<double>(value);
            }
            else if (std::holds_alternative<std::string>(value)) {
                var = std::get<std::string>(value);
            }
            else if (std::holds_alternative<bool>(value)) {
                var = std::get<bool>(value);
            }
            else if (std::holds_alternative<std::chrono::system_clock::time_point>(value)) {
                var = std::get<std::chrono::system_clock::time_point>(value);
            }
            
            if (var.is_valid()) {
                prop.set_value(obj, var);
            }
        }
        
        return obj;
    }
    
    // 从结果集映射到对象列表
    template<typename T>
    std::vector<T> mapToObjects(const ResultSet& resultSet) const {
        std::vector<T> objects;
        objects.reserve(resultSet.size());
        
        for (const auto& row : resultSet) {
            objects.push_back(mapToObject<T>(row));
        }
        
        return objects;
    }
    
    // 映射连表查询结果到多个不同类型的对象
    template<typename... Entities>
    std::vector<std::tuple<Entities...>> mapToTuples(const ResultSet& resultSet) const {
        std::vector<std::tuple<Entities...>> tuples;
        tuples.reserve(resultSet.size());
        
        for (const auto& row : resultSet) {
            tuples.push_back(mapRowToTuple<Entities...>(row));
        }
        
        return tuples;
    }

private:
    // 递归辅助函数：映射行到元组
    template<typename FirstEntity, typename... RestEntities>
    std::tuple<FirstEntity, RestEntities...> mapRowToTuple(const DbRow& row) const {
        if constexpr (sizeof...(RestEntities) == 0) {
            return std::make_tuple(mapToObject<FirstEntity>(row));
        } else {
            return std::tuple_cat(
                std::make_tuple(mapToObject<FirstEntity>(row)),
                mapRowToTuple<RestEntities...>(row)
            );
        }
    }
};

// ========== ORM服务类 ==========
template<typename Entity>
class OrmService {
private:
    // 自动填充时间戳和版本号
    static void auto_fill_insert(Entity& entity) {
        rttr::type t = rttr::type::get(entity);
        auto now = std::chrono::system_clock::now();
        if (auto meta = t.get_metadata(meta::CREATE_TIME_FIELD); meta) {
            if (auto prop = t.get_property(meta.to_string()); prop) prop.set_value(entity, now);
        }
        if (auto meta = t.get_metadata(meta::UPDATE_TIME_FIELD); meta) {
            if (auto prop = t.get_property(meta.to_string()); prop) prop.set_value(entity, now);
        }
        if (auto meta = t.get_metadata(meta::USE_VERSION); meta.to_bool()) {
            if (auto v_meta = t.get_metadata(meta::VERSION_FIELD); v_meta) {
                if (auto prop = t.get_property(v_meta.to_string()); prop) prop.set_value(entity, 1);
            }
        }
    }

public:
    // 插入操作
    static SqlQueryResult insert(Entity& entity) {
        auto_fill_insert(entity);
        rttr::type t = rttr::type::get(entity);
        std::stringstream cols_sql, vals_sql;
        std::vector<ValueVariant> params;

        bool first = true;
        for (auto& prop : t.get_properties()) {
            // 跳过自增主键
            if (prop.get_metadata(meta::AUTO_INCREMENT).to_bool()) continue;

            if (!first) { cols_sql << ", "; vals_sql << ", "; }
            cols_sql << "`" << internal::get_column_name(prop) << "`";
            vals_sql << "?";
            params.push_back(internal::rttr_to_value_variant(prop.get_value(entity)));
            first = false;
        }

        std::string sql = "INSERT INTO `" + internal::get_table_name(t) + "` (" + cols_sql.str() + ") VALUES (" + vals_sql.str() + ")";
        return {sql, params};
    }

    // 批量插入
    static SqlQueryResult batchInsert(std::vector<Entity>& entities) {
        if (entities.empty()) return {"", {}};

        rttr::type t = rttr::type::get<Entity>();
        std::stringstream cols_sql, vals_sql;
        std::vector<ValueVariant> params;
        std::vector<rttr::property> props;

        bool first = true;
        for (auto& prop : t.get_properties()) {
            if (prop.get_metadata(meta::AUTO_INCREMENT).to_bool()) continue;
            if (!first) cols_sql << ", ";
            cols_sql << "`" << internal::get_column_name(prop) << "`";
            props.push_back(prop);
            first = false;
        }

        for (Entity& entity : entities) {
            auto_fill_insert(entity);
            if (!vals_sql.str().empty()) vals_sql << ",";
            vals_sql << "(";
            first = true;
            for (const auto& prop : props) {
                if (!first) vals_sql << ",";
                vals_sql << "?";
                params.push_back(internal::rttr_to_value_variant(prop.get_value(entity)));
                first = false;
            }
            vals_sql << ")";
        }

        std::string sql = "INSERT INTO `" + internal::get_table_name(t) + "` (" + cols_sql.str() + ") VALUES " + vals_sql.str();
        return {sql, params};
    }

    // 根据ID更新所有字段
    static SqlQueryResult updateById(Entity& entity) {
        std::unordered_set<std::string> fields_to_update;
        rttr::type t = rttr::type::get(entity);
        for(auto& prop : t.get_properties()) {
            fields_to_update.insert(prop.get_name().to_string());
        }
        return updateFieldsById(entity, fields_to_update);
    }

    // 根据ID更新指定字段
    static SqlQueryResult updateFieldsById(Entity& entity, const std::unordered_set<std::string>& fields_to_update) {
        rttr::type t = rttr::type::get(entity);
        std::stringstream set_sql;
        std::vector<ValueVariant> params;

        // 自动填充更新时间和版本
        rttr::variant original_version;
        if (auto meta = t.get_metadata(meta::UPDATE_TIME_FIELD); meta) {
            if (auto prop = t.get_property(meta.to_string()); prop) {
                prop.set_value(entity, std::chrono::system_clock::now());
            }
        }
        if (t.get_metadata(meta::USE_VERSION).to_bool()) {
            if (auto v_meta = t.get_metadata(meta::VERSION_FIELD); v_meta) {
                auto prop = t.get_property(v_meta.to_string());
                if(prop) {
                    original_version = prop.get_value(entity);
                    long long new_version = original_version.get_value<long long>() + 1;
                    prop.set_value(entity, new_version);
                }
            }
        }

        // 只保留非空字段
        std::unordered_set<std::string> effective_fields;
        for (const auto& field_name : fields_to_update) {
            auto prop = internal::get_prop(t, field_name);
            // 跳过主键
            if (prop.get_metadata(meta::PRIMARY_KEY).to_bool()) {
                continue;
            }

            // 获取属性值并直接使用属性对象进行检测
            rttr::variant value = prop.get_value(entity);
            ValueVariant val_variant = internal::rttr_to_value_variant(value, &prop);
            
            // 如果不是空值，则加入有效字段集合
            if (!std::holds_alternative<std::monostate>(val_variant)) {
                effective_fields.insert(field_name);
            }
        }

        // 生成SQL
        bool first = true;
        for (const auto& field_name : effective_fields) {
            auto prop = internal::get_prop(t, field_name);
            
            if (!first) set_sql << ", ";
            set_sql << "`" << internal::get_column_name(prop) << "` = ?";
            params.push_back(internal::rttr_to_value_variant(prop.get_value(entity), &prop));
            first = false;
        }

        if (params.empty()) throw std::runtime_error("No fields to update.");

        auto pk_prop = internal::get_primary_key_prop(t);
        set_sql << " WHERE `" << internal::get_column_name(pk_prop) << "` = ?";
        params.push_back(internal::rttr_to_value_variant(pk_prop.get_value(entity)));

        if (t.get_metadata(meta::USE_VERSION).to_bool() && original_version.is_valid()) {
            set_sql << " AND `" << t.get_metadata(meta::VERSION_FIELD).to_string() << "` = ?";
            params.push_back(internal::rttr_to_value_variant(original_version));
        }

        std::string sql = "UPDATE `" + internal::get_table_name(t) + "` SET " + set_sql.str();
        return {sql, params};
    }

    // 根据ID删除
    template<typename IdType>
    static SqlQueryResult deleteById(IdType id, long long version = -1) {
        rttr::type t = rttr::type::get<Entity>();
        auto pk_prop = internal::get_primary_key_prop(t);
        std::string pk_col = internal::get_column_name(pk_prop);
        std::vector<ValueVariant> params;

        if (t.get_metadata(meta::USE_LOGICAL_DELETE).to_bool()) {
            std::string ld_field = t.get_metadata(meta::LOGICAL_DELETE_FIELD).to_string();
            std::string sql = "UPDATE `" + internal::get_table_name(t) + "` SET `" + ld_field + "` = 1";

            if (auto meta = t.get_metadata(meta::UPDATE_TIME_FIELD); meta) {
                sql += ", `" + meta.to_string() + "` = ?";
                params.push_back(std::chrono::system_clock::now());
            }
            sql += " WHERE `" + pk_col + "` = ?";
            params.push_back(id);
            if (t.get_metadata(meta::USE_VERSION).to_bool() && version > -1) {
                sql += " AND `" + t.get_metadata(meta::VERSION_FIELD).to_string() + "` = ?";
                params.push_back(version);
            }
            return {sql, params};
        } else {
            std::string sql = "DELETE FROM `" + internal::get_table_name(t) + "` WHERE `" + pk_col + "` = ?";
            params.push_back(id);
            if (t.get_metadata(meta::USE_VERSION).to_bool() && version > -1) {
                sql += " AND `" + t.get_metadata(meta::VERSION_FIELD).to_string() + "` = ?";
                params.push_back(version);
            }
            return {sql, params};
        }
    }

    // 根据ID查询
    template<typename IdType>
    static SqlQueryResult getById(IdType id, LockMode lock = LockMode::None) {
        rttr::type t = rttr::type::get<Entity>();
        auto pk_prop = internal::get_primary_key_prop(t);
        std::string pk_col = internal::get_column_name(pk_prop);

        std::string sql = "SELECT * FROM `" + internal::get_table_name(t) + "` WHERE `" + pk_col + "` = ?";
        std::vector<ValueVariant> params = {id};

        if (t.get_metadata(meta::USE_LOGICAL_DELETE).to_bool()) {
            sql += " AND `" + t.get_metadata(meta::LOGICAL_DELETE_FIELD).to_string() + "` != 1";
        }
        if (lock == LockMode::ForUpdate) sql += " FOR UPDATE";
        if (lock == LockMode::ForShare) sql += " FOR SHARE";
        return {sql, params};
    }

    // 条件查询
    static SqlQueryResult selectByCondition(const QueryWrapper<Entity>& wrapper, LockMode lock = LockMode::None) {
        rttr::type t = rttr::type::get<Entity>();
        auto result = wrapper.getSelectSql();
        
        // 添加逻辑删除条件
        if (t.get_metadata(meta::USE_LOGICAL_DELETE).to_bool()) {
            std::string ld_cond = " `" + t.get_metadata(meta::LOGICAL_DELETE_FIELD).to_string() + "` != 1";
            size_t where_pos = result.sql.find(" WHERE ");
            if (where_pos == std::string::npos) {
                result.sql += " WHERE" + ld_cond;
            } else {
                size_t order_pos = result.sql.find(" ORDER BY");
                result.sql.insert(order_pos == std::string::npos ? result.sql.length() : order_pos, " AND" + ld_cond);
            }
        }
        
        // 添加读写锁
        if (lock == LockMode::ForUpdate) {
            result.sql += " FOR UPDATE";
        } else if (lock == LockMode::ForShare) {
            result.sql += " FOR SHARE";
        }
        
        return result;
    }

    // 分页查询
    static std::pair<SqlQueryResult, SqlQueryResult> selectPage(const QueryWrapper<Entity>& wrapper, const PageParam& page, LockMode lock = LockMode::None) {
        auto count_wrapper = wrapper;
        auto data_wrapper = wrapper;
        data_wrapper.limit(page.page_size).offset((page.page_index - 1) * page.page_size);

        return {count_wrapper.getCountSql(), selectByCondition(data_wrapper, lock)};
    }

    // 计数
    static SqlQueryResult count(const QueryWrapper<Entity>& wrapper) {
        return wrapper.getCountSql();
    }
    
    // 连表查询
    static SqlQueryResult selectWithJoin(const JoinQueryWrapper<Entity>& wrapper, LockMode lock = LockMode::None) {
        auto result = wrapper.getJoinSql();
        
        // 添加逻辑删除条件 - 注：在使用JOIN时，通常由用户自行处理逻辑删除条件
        
        // 添加读写锁
        if (lock == LockMode::ForUpdate) {
            result.sql += " FOR UPDATE";
        } else if (lock == LockMode::ForShare) {
            result.sql += " FOR SHARE";
        }
        
        return result;
    }
    
    // 连表分页查询
    static std::pair<SqlQueryResult, SqlQueryResult> selectJoinPage(
        const JoinQueryWrapper<Entity>& wrapper, 
        const PageParam& page, 
        LockMode lock = LockMode::None) 
    {
        auto count_sql = wrapper.getJoinCountSql();
        
        auto data_wrapper = wrapper;
        data_wrapper.limit(page.page_size).offset((page.page_index - 1) * page.page_size);
        auto data_sql = selectWithJoin(data_wrapper, lock);
        
        return {count_sql, data_sql};
    }
};

} // namespace orm_rttr

#endif // ORM_RTTR_HPP