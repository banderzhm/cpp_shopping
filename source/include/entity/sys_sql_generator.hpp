#ifndef SYS_SQL_GENERATOR_HPP
#define SYS_SQL_GENERATOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "sys_entities.hpp"
#include "../../include/orm_rttr.hpp"

namespace entity {

/**
 * RBAC系统SQL生成器
 * 根据实体类生成相应的SQL查询语句
 */
class SysSqlGenerator {
public:
    /**
     * 使用ORM框架生成插入语句
     * @tparam T 实体类类型
     * @param entity 实体对象
     * @return SQL查询结果
     */
    template<typename T>
    static orm_rttr::SqlQueryResult generateInsertSql(T& entity) {
        return orm_rttr::OrmService<T>::insert(entity);
    }

    /**
     * 使用ORM框架生成批量插入语句
     * @tparam T 实体类类型
     * @param entities 实体对象列表
     * @return SQL查询结果
     */
    template<typename T>
    static orm_rttr::SqlQueryResult generateBatchInsertSql(std::vector<T>& entities) {
        return orm_rttr::OrmService<T>::batchInsert(entities);
    }

    /**
     * 使用ORM框架生成更新语句
     * @tparam T 实体类类型
     * @param entity 实体对象
     * @return SQL查询结果
     */
    template<typename T>
    static orm_rttr::SqlQueryResult generateUpdateSql(T& entity) {
        return orm_rttr::OrmService<T>::updateById(entity);
    }

    /**
     * 使用ORM框架生成查询语句
     * @tparam T 实体类类型
     * @param id 主键ID
     * @param lock 锁定模式
     * @return SQL查询结果
     */
    template<typename T, typename IdType>
    static orm_rttr::SqlQueryResult generateSelectByIdSql(IdType id, orm_rttr::LockMode lock = orm_rttr::LockMode::None) {
        return orm_rttr::OrmService<T>::getById(id, lock);
    }

    /**
     * 使用ORM框架生成条件查询语句
     * @tparam T 实体类类型
     * @param wrapper 查询条件包装器
     * @param lock 锁定模式
     * @return SQL查询结果
     */
    template<typename T>
    static orm_rttr::SqlQueryResult generateSelectByConditionSql(const orm_rttr::QueryWrapper<T>& wrapper, orm_rttr::LockMode lock = orm_rttr::LockMode::None) {
        return orm_rttr::OrmService<T>::selectByCondition(wrapper, lock);
    }

    /**
     * 使用ORM框架生成分页查询语句
     * @tparam T 实体类类型
     * @param wrapper 查询条件包装器
     * @param page 分页参数
     * @param lock 锁定模式
     * @return SQL查询结果对（计数查询和数据查询）
     */
    template<typename T>
    static std::pair<orm_rttr::SqlQueryResult, orm_rttr::SqlQueryResult> generateSelectPageSql(
        const orm_rttr::QueryWrapper<T>& wrapper,
        const orm_rttr::PageParam& page,
        orm_rttr::LockMode lock = orm_rttr::LockMode::None) {
        return orm_rttr::OrmService<T>::selectPage(wrapper, page, lock);
    }

    /**
     * 使用ORM框架生成逻辑删除语句
     * @tparam T 实体类类型
     * @param id 主键ID
     * @param version 版本号(可选)
     * @return SQL查询结果
     */
    template<typename T, typename IdType>
    static orm_rttr::SqlQueryResult generateLogicalDeleteSql(IdType id, long long version = -1) {
        return orm_rttr::OrmService<T>::deleteById(id, version);
    }
};

} // namespace entity

#endif // SYS_SQL_GENERATOR_HPP 