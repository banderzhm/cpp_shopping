#include <iostream>
#include <fstream>
#include "entity/sys_entities.hpp"
#include "entity/sys_entities_mapping.hpp"
#include "orm_rttr.hpp"
#include <variant>
#include <doctest/doctest.h>

/**
 * 格式化参数列表为字符串
 */
void printParams(std::ostream& file, const std::vector<orm_rttr::ValueVariant>& params) {
    file << "[";
    
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) {
            file << ", ";
        }
        
        const auto& param = params[i];
        if (std::holds_alternative<std::monostate>(param)) {
            file << "NULL";
        } else if (std::holds_alternative<std::string>(param)) {
            file << "'" << std::get<std::string>(param) << "'";
        } else if (std::holds_alternative<int>(param)) {
            file << std::get<int>(param);
        } else if (std::holds_alternative<long>(param)) {
            file << std::get<long>(param);
        } else if (std::holds_alternative<long long>(param)) {
            file << std::get<long long>(param);
        } else if (std::holds_alternative<double>(param)) {
            file << std::get<double>(param);
        } else if (std::holds_alternative<bool>(param)) {
            file << (std::get<bool>(param) ? "true" : "false");
        } else if (std::holds_alternative<std::chrono::system_clock::time_point>(param)) {
            file << "<timestamp>";
        } else {
            file << "<other_type>";
        }
    }
    
    file << "]";
}

/**
 * 格式化参数列表为字符串（返回值版本）
 */
std::string formatParams(const std::vector<orm_rttr::ValueVariant>& params) {
    std::stringstream ss;
    printParams(ss, params);
    return ss.str();
}

/**
 * 连接查询测试
 */
TEST_CASE("连接查询测试") {
    std::cout << "\n===== 连接查询测试 =====\n";

    // 简单的用户-部门连接查询
    {
        std::cout << "\n--- 1. 简单的用户-部门连接查询 ---\n";
        
        orm_rttr::JoinQueryWrapper<entity::SysUser> wrapper;
        wrapper.mainTableAlias("u")
            .leftJoin<entity::SysDept>("d").on("u.dept_id = d.dept_id")
            .select({"u.*", "d.dept_name", "d.ancestors"})
            .eq("u.user_id", 1)
            .eq("u.del_flag", std::string("0"));
        
        // 只生成SQL，不执行
        orm_rttr::SqlQueryResult result;
        try {
            result = orm_rttr::OrmService<entity::SysUser>::selectWithJoin(wrapper);
            
            std::cout << "SQL: " << result.sql << std::endl;
            std::cout << "参数: ";
            printParams(std::cout, result.params);
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cout << "生成SQL时出错: " << e.what() << std::endl;
        }
    }
    
    // 用户-角色-菜单权限连接查询
    {
        std::cout << "\n--- 2. 用户-角色-菜单权限连接查询 ---\n";
        
        orm_rttr::JoinQueryWrapper<entity::SysUser> wrapper;
        wrapper.mainTableAlias("u")
            .leftJoin<entity::SysUserRole>("ur").on("u.user_id = ur.user_id")
            .leftJoin<entity::SysRole>("r").on("ur.role_id = r.role_id")
            .leftJoin<entity::SysRoleMenu>("rm").on("r.role_id = rm.role_id")
            .leftJoin<entity::SysMenu>("m").on("rm.menu_id = m.menu_id")
            .select({"DISTINCT m.perms"})
            .eq("u.user_id", 1)
            .eq("m.status", std::string("0"))
            .isNotNull("m.perms")
            .eq("r.del_flag", std::string("0"));
        
        // 只生成SQL，不执行
        orm_rttr::SqlQueryResult result;
        try {
            result = orm_rttr::OrmService<entity::SysUser>::selectWithJoin(wrapper);
            
            std::cout << "SQL: " << result.sql << std::endl;
            std::cout << "参数: ";
            printParams(std::cout, result.params);
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cout << "生成SQL时出错: " << e.what() << std::endl;
        }
    }
    
    // 分组和聚合查询
    {
        std::cout << "\n--- 3. 分组和聚合查询 ---\n";
        
        orm_rttr::JoinQueryWrapper<entity::SysUser> wrapper;
        wrapper.mainTableAlias("u")
            .leftJoin<entity::SysDept>("d").on("u.dept_id = d.dept_id")
            .leftJoin<entity::SysUserRole>("ur").on("u.user_id = ur.user_id")
            .leftJoin<entity::SysRole>("r").on("ur.role_id = r.role_id AND r.del_flag = \"0\"")
            .select({
                "u.*", 
                "d.dept_name", 
                "d.ancestors",
                "GROUP_CONCAT(DISTINCT r.role_name) AS role_names"
            })
            .eq("u.user_id", 1)
            .eq("u.del_flag", std::string("0"))
            .groupBy("u.user_id");
        
        // 只生成SQL，不执行
        orm_rttr::SqlQueryResult result;
        try {
            result = orm_rttr::OrmService<entity::SysUser>::selectWithJoin(wrapper);
            
            std::cout << "SQL: " << result.sql << std::endl;
            std::cout << "参数: ";
            printParams(std::cout, result.params);
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cout << "生成SQL时出错: " << e.what() << std::endl;
        }
    }
    
    // 分页查询
    {
        std::cout << "\n--- 4. 分页查询 ---\n";
        
        orm_rttr::JoinQueryWrapper<entity::SysUser> wrapper;
        wrapper.mainTableAlias("u")
            .leftJoin<entity::SysDept>("d").on("u.dept_id = d.dept_id")
            .select({"u.*", "d.dept_name"})
            .like("u.user_name", "%admin%")
            .eq("u.del_flag", std::string("0"))
            .orderBy("u.create_time", orm_rttr::OrderDirection::DESC);
            
        orm_rttr::PageParam page;
        page.page_index = 1;
        page.page_size = 10;
        
        // 只生成SQL，不执行
        orm_rttr::SqlQueryResult countSql, dataSql;
        try {
            std::tie(countSql, dataSql) = orm_rttr::OrmService<entity::SysUser>::selectJoinPage(wrapper, page);
            
            std::cout << "计数SQL: " << countSql.sql << std::endl;
            std::cout << "计数参数: ";
            printParams(std::cout, countSql.params);
            std::cout << std::endl;
            
            std::cout << "数据SQL: " << dataSql.sql << std::endl;
            std::cout << "数据参数: ";
            printParams(std::cout, dataSql.params);
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cout << "生成SQL时出错: " << e.what() << std::endl;
        }
    }
    
    // 复杂多表连接查询
    {
        std::cout << "\n--- 5. 复杂多表连接查询 ---\n";
        
        orm_rttr::JoinQueryWrapper<entity::SysUser> wrapper;
        wrapper.mainTableAlias("u")
            .leftJoin<entity::SysDept>("d").on("u.dept_id = d.dept_id")
            .leftJoin<entity::SysUserRole>("ur").on("u.user_id = ur.user_id")
            .leftJoin<entity::SysRole>("r").on("ur.role_id = r.role_id AND r.del_flag = \"0\"")
            .leftJoin<entity::SysUserPost>("up").on("u.user_id = up.user_id")
            .leftJoin<entity::SysPost>("p").on("up.post_id = p.post_id AND p.status = \"0\"")
            .select({
                "u.*", 
                "d.dept_name", 
                "d.ancestors",
                "GROUP_CONCAT(DISTINCT r.role_name) AS role_names", 
                "GROUP_CONCAT(DISTINCT p.post_name) AS post_names"
            })
            .eq("u.user_id", 1)
            .eq("u.del_flag", std::string("0"))
            .groupBy("u.user_id");
        
        // 只生成SQL，不执行
        orm_rttr::SqlQueryResult result;
        try {
            result = orm_rttr::OrmService<entity::SysUser>::selectWithJoin(wrapper);
            
            std::cout << "SQL: " << result.sql << std::endl;
            std::cout << "参数: ";
            printParams(std::cout, result.params);
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cout << "生成SQL时出错: " << e.what() << std::endl;
        }
    }
}