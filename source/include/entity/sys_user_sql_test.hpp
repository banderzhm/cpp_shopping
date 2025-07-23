#ifndef SYS_USER_SQL_TEST_HPP
#define SYS_USER_SQL_TEST_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <variant>
#include "sys_entities.hpp"
#include "sys_entities_mapping.hpp"
#include "../../include/orm_rttr.hpp"

namespace entity {

/**
 * 用户SQL查询测试类
 * 提供各种用户相关的SQL查询，包括连接查询
 */
class SysUserSqlTest {
public:
    /**
     * 获取用户基本信息查询SQL
     * @param userId 用户ID
     * @return SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserBasicInfoSql(int64_t userId) {
        orm_rttr::QueryWrapper<SysUser> wrapper;
        wrapper.eq("user_id", userId);
        return orm_rttr::OrmService<SysUser>::selectByCondition(wrapper);
    }

    /**
     * 获取用户列表查询SQL（分页）
     * @param pageParam 分页参数
     * @param userName 用户名（可选过滤）
     * @param status 状态（可选过滤）
     * @return SQL查询结果对（计数SQL和数据SQL）
     */
    static std::pair<orm_rttr::SqlQueryResult, orm_rttr::SqlQueryResult> getUserListSql(
            const orm_rttr::PageParam& pageParam, 
            const std::string& userName = "", 
            char status = '\0') {
        
        orm_rttr::QueryWrapper<SysUser> wrapper;
        
        // 添加条件过滤
        if (!userName.empty()) {
            wrapper.like("user_name", "%" + userName + "%");
        }
        
        if (status != '\0') {
            wrapper.eq("status", status);
        }
        
        // 只查询未删除的用户
        wrapper.eq("del_flag", '0');
        
        // 按照创建时间排序
        wrapper.orderBy("create_time", orm_rttr::OrderDirection::DESC);
        
        return orm_rttr::OrmService<SysUser>::selectPage(wrapper, pageParam);
    }

    /**
     * 获取带部门信息的用户查询SQL
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserWithDeptSql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT u.*, d.dept_name, d.ancestors "
            << "FROM sys_user u "
            << "LEFT JOIN sys_dept d ON u.dept_id = d.dept_id "
            << "WHERE u.user_id = ? AND u.del_flag = '0'";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 获取用户角色信息查询SQL
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserRolesSql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT r.* "
            << "FROM sys_role r "
            << "INNER JOIN sys_user_role ur ON r.role_id = ur.role_id "
            << "WHERE ur.user_id = ? AND r.del_flag = '0'";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 获取用户岗位信息查询SQL
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserPostsSql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT p.* "
            << "FROM sys_post p "
            << "INNER JOIN sys_user_post up ON p.post_id = up.post_id "
            << "WHERE up.user_id = ? AND p.status = '0'";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 获取用户所有权限菜单查询SQL
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserMenusSql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT DISTINCT m.* "
            << "FROM sys_menu m "
            << "LEFT JOIN sys_role_menu rm ON m.menu_id = rm.menu_id "
            << "LEFT JOIN sys_user_role ur ON rm.role_id = ur.role_id "
            << "WHERE ur.user_id = ? AND m.status = '0' "
            << "ORDER BY m.parent_id, m.order_num";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 获取用户权限代码查询SQL
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserPermsSql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT DISTINCT m.perms "
            << "FROM sys_menu m "
            << "LEFT JOIN sys_role_menu rm ON m.menu_id = rm.menu_id "
            << "LEFT JOIN sys_user_role ur ON rm.role_id = ur.role_id "
            << "WHERE ur.user_id = ? AND m.status = '0' AND m.perms IS NOT NULL AND m.perms != ''";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 获取用户角色数据权限查询SQL
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserDataScopeSql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT r.data_scope "
            << "FROM sys_role r "
            << "INNER JOIN sys_user_role ur ON r.role_id = ur.role_id "
            << "WHERE ur.user_id = ? AND r.status = '0' "
            << "ORDER BY r.data_scope ASC "
            << "LIMIT 1";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 复杂的用户查询：获取用户完整信息（包括部门、角色、岗位）
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUserFullInfoSql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT u.*, d.dept_name, d.ancestors, "
            << "GROUP_CONCAT(DISTINCT r.role_name) AS role_names, "
            << "GROUP_CONCAT(DISTINCT p.post_name) AS post_names "
            << "FROM sys_user u "
            << "LEFT JOIN sys_dept d ON u.dept_id = d.dept_id "
            << "LEFT JOIN sys_user_role ur ON u.user_id = ur.user_id "
            << "LEFT JOIN sys_role r ON ur.role_id = r.role_id AND r.del_flag = '0' "
            << "LEFT JOIN sys_user_post up ON u.user_id = up.user_id "
            << "LEFT JOIN sys_post p ON up.post_id = p.post_id AND p.status = '0' "
            << "WHERE u.user_id = ? AND u.del_flag = '0' "
            << "GROUP BY u.user_id";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 获取拥有特定权限的所有用户
     * @param permCode 权限代码
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUsersByPermissionSql(const std::string& permCode) {
        std::stringstream sql;
        sql << "SELECT DISTINCT u.* "
            << "FROM sys_user u "
            << "INNER JOIN sys_user_role ur ON u.user_id = ur.user_id "
            << "INNER JOIN sys_role r ON ur.role_id = r.role_id AND r.status = '0' AND r.del_flag = '0' "
            << "INNER JOIN sys_role_menu rm ON r.role_id = rm.role_id "
            << "INNER JOIN sys_menu m ON rm.menu_id = m.menu_id AND m.status = '0' "
            << "WHERE u.del_flag = '0' AND u.status = '0' AND m.perms LIKE ?";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back("%" + permCode + "%");
        
        return result;
    }

    /**
     * 获取用户所在部门及子部门的所有用户
     * @param userId 用户ID
     * @return 自定义SQL查询结果
     */
    static orm_rttr::SqlQueryResult getUsersByDeptHierarchySql(int64_t userId) {
        std::stringstream sql;
        sql << "SELECT u.* FROM sys_user u "
            << "INNER JOIN sys_dept d ON u.dept_id = d.dept_id "
            << "WHERE d.dept_id IN ("
            << "  SELECT dept_id FROM sys_dept WHERE ancestors LIKE ("
            << "    SELECT CONCAT('%,', d2.dept_id, ',%') FROM sys_dept d2"
            << "    INNER JOIN sys_user u2 ON d2.dept_id = u2.dept_id"
            << "    WHERE u2.user_id = ?"
            << "  ) OR dept_id = (SELECT dept_id FROM sys_user WHERE user_id = ?)"
            << ")"
            << "AND u.del_flag = '0'";
        
        orm_rttr::SqlQueryResult result;
        result.sql = sql.str();
        result.params.push_back(userId);
        result.params.push_back(userId);
        
        return result;
    }

    /**
     * 将所有SQL查询保存到txt文件
     * @param filePath 文件路径
     * @param userId 用于测试的用户ID
     * @return 是否保存成功
     */
    static bool saveAllSqlToFile(const std::string& filePath, int64_t userId = 1) {
        try {
            std::ofstream file(filePath);
            if (!file.is_open()) {
                return false;
            }
            
            // 设置分页参数
            orm_rttr::PageParam pageParam;
            pageParam.page_index = 1;
            pageParam.page_size = 10;
            
            // 格式化并写入SQL
            file << "===== 用户系统所有SQL查询 =====\n\n";
            
            // 1. 获取用户基本信息
            auto basicInfo = getUserBasicInfoSql(userId);
            file << "1. 获取用户基本信息\n";
            file << "SQL: " << basicInfo.sql << "\n";
            file << "参数: " << formatParams(basicInfo.params) << "\n\n";
            
            // 2. 获取用户列表（分页）
            auto [countSql, listSql] = getUserListSql(pageParam, "admin");
            file << "2. 获取用户列表（分页）\n";
            file << "计数SQL: " << countSql.sql << "\n";
            file << "计数参数: " << formatParams(countSql.params) << "\n";
            file << "数据SQL: " << listSql.sql << "\n";
            file << "数据参数: " << formatParams(listSql.params) << "\n\n";
            
            // 3. 获取带部门信息的用户
            auto userWithDept = getUserWithDeptSql(userId);
            file << "3. 获取带部门信息的用户\n";
            file << "SQL: " << userWithDept.sql << "\n";
            file << "参数: " << formatParams(userWithDept.params) << "\n\n";
            
            // 4. 获取用户角色信息
            auto userRoles = getUserRolesSql(userId);
            file << "4. 获取用户角色信息\n";
            file << "SQL: " << userRoles.sql << "\n";
            file << "参数: " << formatParams(userRoles.params) << "\n\n";
            
            // 5. 获取用户岗位信息
            auto userPosts = getUserPostsSql(userId);
            file << "5. 获取用户岗位信息\n";
            file << "SQL: " << userPosts.sql << "\n";
            file << "参数: " << formatParams(userPosts.params) << "\n\n";
            
            // 6. 获取用户所有权限菜单
            auto userMenus = getUserMenusSql(userId);
            file << "6. 获取用户所有权限菜单\n";
            file << "SQL: " << userMenus.sql << "\n";
            file << "参数: " << formatParams(userMenus.params) << "\n\n";
            
            // 7. 获取用户权限代码
            auto userPerms = getUserPermsSql(userId);
            file << "7. 获取用户权限代码\n";
            file << "SQL: " << userPerms.sql << "\n";
            file << "参数: " << formatParams(userPerms.params) << "\n\n";
            
            // 8. 获取用户角色数据权限
            auto userDataScope = getUserDataScopeSql(userId);
            file << "8. 获取用户角色数据权限\n";
            file << "SQL: " << userDataScope.sql << "\n";
            file << "参数: " << formatParams(userDataScope.params) << "\n\n";
            
            // 9. 获取用户完整信息
            auto userFullInfo = getUserFullInfoSql(userId);
            file << "9. 获取用户完整信息（包括部门、角色、岗位）\n";
            file << "SQL: " << userFullInfo.sql << "\n";
            file << "参数: " << formatParams(userFullInfo.params) << "\n\n";
            
            // 10. 获取拥有特定权限的所有用户
            auto usersByPerm = getUsersByPermissionSql("system:user:list");
            file << "10. 获取拥有特定权限的所有用户\n";
            file << "SQL: " << usersByPerm.sql << "\n";
            file << "参数: " << formatParams(usersByPerm.params) << "\n\n";
            
            // 11. 获取用户所在部门及子部门的所有用户
            auto usersByDeptHierarchy = getUsersByDeptHierarchySql(userId);
            file << "11. 获取用户所在部门及子部门的所有用户\n";
            file << "SQL: " << usersByDeptHierarchy.sql << "\n";
            file << "参数: " << formatParams(usersByDeptHierarchy.params) << "\n\n";
            
            file.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "保存SQL到文件失败: " << e.what() << std::endl;
            return false;
        }
    }

private:
    /**
     * 格式化参数列表为字符串
     * @param params 参数列表
     * @return 格式化后的字符串
     */
    static std::string formatParams(const std::vector<orm_rttr::ValueVariant>& params) {
        if (params.empty()) {
            return "[]";
        }
        
        std::stringstream ss;
        ss << "[";
        
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) {
                ss << ", ";
            }
            
            const auto& param = params[i];
            if (std::holds_alternative<std::monostate>(param)) {
                ss << "NULL";
            } else if (std::holds_alternative<std::string>(param)) {
                ss << "'" << std::get<std::string>(param) << "'";
            } else if (std::holds_alternative<int>(param)) {
                ss << std::get<int>(param);
            } else if (std::holds_alternative<long>(param)) {
                ss << std::get<long>(param);
            } else if (std::holds_alternative<long long>(param)) {
                ss << std::get<long long>(param);
            } else if (std::holds_alternative<double>(param)) {
                ss << std::get<double>(param);
            } else if (std::holds_alternative<bool>(param)) {
                ss << (std::get<bool>(param) ? "true" : "false");
            } else if (std::holds_alternative<std::chrono::system_clock::time_point>(param)) {
                // 简化处理时间点
                ss << "<timestamp>";
            }
        }
        
        ss << "]";
        return ss.str();
    }
};

} // namespace entity

#endif // SYS_USER_SQL_TEST_HPP 