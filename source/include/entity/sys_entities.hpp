#ifndef SYS_ENTITIES_HPP
#define SYS_ENTITIES_HPP

#include <string>
#include <chrono>

namespace entity {

/**
 * 部门表
 */
struct SysDept {
    int64_t dept_id;             // 部门id
    int64_t parent_id;           // 父部门id
    std::string ancestors;       // 祖级列表
    std::string dept_name;       // 部门名称
    int order_num;               // 显示顺序
    std::string leader;          // 负责人
    std::string phone;           // 联系电话
    std::string email;           // 邮箱
    char status;                 // 部门状态（0正常 1停用）
    char del_flag;               // 删除标志（0代表存在 2代表删除）
    std::string create_by;       // 创建者
    std::chrono::system_clock::time_point create_time;  // 创建时间
    std::string update_by;       // 更新者
    std::chrono::system_clock::time_point update_time;  // 更新时间
};

/**
 * 用户信息表
 */
struct SysUser {
    int64_t user_id;             // 用户ID
    int64_t dept_id;             // 部门ID
    std::string user_name;       // 用户账号
    std::string nick_name;       // 用户昵称
    std::string user_type;       // 用户类型（00系统用户）
    std::string email;           // 用户邮箱
    std::string phonenumber;     // 手机号码
    char sex;                    // 用户性别（0男 1女 2未知）
    std::string avatar;          // 头像地址
    std::string password;        // 密码
    char status;                 // 账号状态（0正常 1停用）
    char del_flag;               // 删除标志（0代表存在 2代表删除）
    std::string login_ip;        // 最后登录IP
    std::chrono::system_clock::time_point login_date;   // 最后登录时间
    std::chrono::system_clock::time_point pwd_update_date; // 密码最后更新时间
    std::string create_by;       // 创建者
    std::chrono::system_clock::time_point create_time;  // 创建时间
    std::string update_by;       // 更新者
    std::chrono::system_clock::time_point update_time;  // 更新时间
    std::string remark;          // 备注
};

/**
 * 岗位信息表
 */
struct SysPost {
    int64_t post_id;             // 岗位ID
    std::string post_code;       // 岗位编码
    std::string post_name;       // 岗位名称
    int post_sort;               // 显示顺序
    char status;                 // 状态（0正常 1停用）
    std::string create_by;       // 创建者
    std::chrono::system_clock::time_point create_time;  // 创建时间
    std::string update_by;       // 更新者
    std::chrono::system_clock::time_point update_time;  // 更新时间
    std::string remark;          // 备注
};

/**
 * 角色信息表
 */
struct SysRole {
    int64_t role_id;             // 角色ID
    std::string role_name;       // 角色名称
    std::string role_key;        // 角色权限字符串
    int role_sort;               // 显示顺序
    char data_scope;             // 数据范围（1：全部数据权限 2：自定数据权限 3：本部门数据权限 4：本部门及以下数据权限）
    bool menu_check_strictly;    // 菜单树选择项是否关联显示
    bool dept_check_strictly;    // 部门树选择项是否关联显示
    char status;                 // 角色状态（0正常 1停用）
    char del_flag;               // 删除标志（0代表存在 2代表删除）
    std::string create_by;       // 创建者
    std::chrono::system_clock::time_point create_time;  // 创建时间
    std::string update_by;       // 更新者
    std::chrono::system_clock::time_point update_time;  // 更新时间
    std::string remark;          // 备注
};

/**
 * 菜单权限表
 */
struct SysMenu {
    int64_t menu_id;             // 菜单ID
    std::string menu_name;       // 菜单名称
    int64_t parent_id;           // 父菜单ID
    int order_num;               // 显示顺序
    std::string path;            // 路由地址
    std::string component;       // 组件路径
    std::string query;           // 路由参数
    std::string route_name;      // 路由名称
    int is_frame;                // 是否为外链（0是 1否）
    int is_cache;                // 是否缓存（0缓存 1不缓存）
    char menu_type;              // 菜单类型（M目录 C菜单 F按钮）
    char visible;                // 菜单状态（0显示 1隐藏）
    char status;                 // 菜单状态（0正常 1停用）
    std::string perms;           // 权限标识
    std::string icon;            // 菜单图标
    std::string create_by;       // 创建者
    std::chrono::system_clock::time_point create_time;  // 创建时间
    std::string update_by;       // 更新者
    std::chrono::system_clock::time_point update_time;  // 更新时间
    std::string remark;          // 备注
};

/**
 * 用户和角色关联表
 */
struct SysUserRole {
    int64_t user_id;             // 用户ID
    int64_t role_id;             // 角色ID
};

/**
 * 角色和菜单关联表
 */
struct SysRoleMenu {
    int64_t role_id;             // 角色ID
    int64_t menu_id;             // 菜单ID
};

/**
 * 角色和部门关联表
 */
struct SysRoleDept {
    int64_t role_id;             // 角色ID
    int64_t dept_id;             // 部门ID
};

/**
 * 用户与岗位关联表
 */
struct SysUserPost {
    int64_t user_id;             // 用户ID
    int64_t post_id;             // 岗位ID
};

} // namespace entity

#endif // SYS_ENTITIES_HPP 