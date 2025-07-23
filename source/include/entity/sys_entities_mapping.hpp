#ifndef SYS_ENTITIES_MAPPING_HPP
#define SYS_ENTITIES_MAPPING_HPP

#include <rttr/registration>

#include "orm_rttr.hpp"
#include "sys_entities.hpp"

// 使用RTTR库注册实体类映射关系
RTTR_REGISTRATION
{
    using namespace rttr;
    using namespace entity;
    using namespace orm_rttr::meta;

    // ================ 部门表映射 ================
    registration::class_<SysDept>("sys_dept")
        (
            metadata(TABLE_NAME, "sys_dept")
        )
        .property("dept_id", &SysDept::dept_id)(
            metadata(PRIMARY_KEY, true),
            metadata(AUTO_INCREMENT, true)
        )
        .property("parent_id", &SysDept::parent_id)(
            metadata(COLUMN_NAME, "parent_id")
        )
        .property("ancestors", &SysDept::ancestors)(
            metadata(COLUMN_NAME, "ancestors")
        )
        .property("dept_name", &SysDept::dept_name)(
            metadata(COLUMN_NAME, "dept_name")
        )
        .property("order_num", &SysDept::order_num)(
            metadata(COLUMN_NAME, "order_num")
        )
        .property("leader", &SysDept::leader)(
            metadata(COLUMN_NAME, "leader")
        )
        .property("phone", &SysDept::phone)(
            metadata(COLUMN_NAME, "phone")
        )
        .property("email", &SysDept::email)(
            metadata(COLUMN_NAME, "email")
        )
        .property("status", &SysDept::status)(
            metadata(COLUMN_NAME, "status")
        )
        .property("del_flag", &SysDept::del_flag)(
            metadata(COLUMN_NAME, "del_flag"),
            metadata(LOGICAL_DELETE_FIELD, true)
        )
        .property("create_by", &SysDept::create_by)(
            metadata(COLUMN_NAME, "create_by")
        )
        .property("create_time", &SysDept::create_time)(
            metadata(COLUMN_NAME, "create_time"),
            metadata(CREATE_TIME_FIELD, true)
        )
        .property("update_by", &SysDept::update_by)(
            metadata(COLUMN_NAME, "update_by")
        )
        .property("update_time", &SysDept::update_time)(
            metadata(COLUMN_NAME, "update_time"),
            metadata(UPDATE_TIME_FIELD, true)
        );

    // ================ 用户表映射 ================
    registration::class_<SysUser>("sys_user")
        (
            metadata(TABLE_NAME, "sys_user")
        )
        .property("user_id", &SysUser::user_id)(
            metadata(PRIMARY_KEY, true),
            metadata(AUTO_INCREMENT, true)
        )
        .property("dept_id", &SysUser::dept_id)(
            metadata(COLUMN_NAME, "dept_id")
        )
        .property("user_name", &SysUser::user_name)(
            metadata(COLUMN_NAME, "user_name")
        )
        .property("nick_name", &SysUser::nick_name)(
            metadata(COLUMN_NAME, "nick_name")
        )
        .property("user_type", &SysUser::user_type)(
            metadata(COLUMN_NAME, "user_type")
        )
        .property("email", &SysUser::email)(
            metadata(COLUMN_NAME, "email")
        )
        .property("phonenumber", &SysUser::phonenumber)(
            metadata(COLUMN_NAME, "phonenumber")
        )
        .property("sex", &SysUser::sex)(
            metadata(COLUMN_NAME, "sex")
        )
        .property("avatar", &SysUser::avatar)(
            metadata(COLUMN_NAME, "avatar")
        )
        .property("password", &SysUser::password)(
            metadata(COLUMN_NAME, "password")
        )
        .property("status", &SysUser::status)(
            metadata(COLUMN_NAME, "status")
        )
        .property("del_flag", &SysUser::del_flag)(
            metadata(COLUMN_NAME, "del_flag"),
            metadata(LOGICAL_DELETE_FIELD, true)
        )
        .property("login_ip", &SysUser::login_ip)(
            metadata(COLUMN_NAME, "login_ip")
        )
        .property("login_date", &SysUser::login_date)(
            metadata(COLUMN_NAME, "login_date")
        )
        .property("pwd_update_date", &SysUser::pwd_update_date)(
            metadata(COLUMN_NAME, "pwd_update_date")
        )
        .property("create_by", &SysUser::create_by)(
            metadata(COLUMN_NAME, "create_by")
        )
        .property("create_time", &SysUser::create_time)(
            metadata(COLUMN_NAME, "create_time"),
            metadata(CREATE_TIME_FIELD, true)
        )
        .property("update_by", &SysUser::update_by)(
            metadata(COLUMN_NAME, "update_by")
        )
        .property("update_time", &SysUser::update_time)(
            metadata(COLUMN_NAME, "update_time"),
            metadata(UPDATE_TIME_FIELD, true)
        )
        .property("remark", &SysUser::remark)(
            metadata(COLUMN_NAME, "remark")
        );

    // ================ 岗位表映射 ================
    registration::class_<SysPost>("sys_post")
        (
            metadata(TABLE_NAME, "sys_post")
        )
        .property("post_id", &SysPost::post_id)(
            metadata(PRIMARY_KEY, true),
            metadata(AUTO_INCREMENT, true)
        )
        .property("post_code", &SysPost::post_code)(
            metadata(COLUMN_NAME, "post_code")
        )
        .property("post_name", &SysPost::post_name)(
            metadata(COLUMN_NAME, "post_name")
        )
        .property("post_sort", &SysPost::post_sort)(
            metadata(COLUMN_NAME, "post_sort")
        )
        .property("status", &SysPost::status)(
            metadata(COLUMN_NAME, "status")
        )
        .property("create_by", &SysPost::create_by)(
            metadata(COLUMN_NAME, "create_by")
        )
        .property("create_time", &SysPost::create_time)(
            metadata(COLUMN_NAME, "create_time"),
            metadata(CREATE_TIME_FIELD, true)
        )
        .property("update_by", &SysPost::update_by)(
            metadata(COLUMN_NAME, "update_by")
        )
        .property("update_time", &SysPost::update_time)(
            metadata(COLUMN_NAME, "update_time"),
            metadata(UPDATE_TIME_FIELD, true)
        )
        .property("remark", &SysPost::remark)(
            metadata(COLUMN_NAME, "remark")
        );

    // ================ 角色表映射 ================
    registration::class_<SysRole>("sys_role")
        (
            metadata(TABLE_NAME, "sys_role")
        )
        .property("role_id", &SysRole::role_id)(
            metadata(PRIMARY_KEY, true),
            metadata(AUTO_INCREMENT, true)
        )
        .property("role_name", &SysRole::role_name)(
            metadata(COLUMN_NAME, "role_name")
        )
        .property("role_key", &SysRole::role_key)(
            metadata(COLUMN_NAME, "role_key")
        )
        .property("role_sort", &SysRole::role_sort)(
            metadata(COLUMN_NAME, "role_sort")
        )
        .property("data_scope", &SysRole::data_scope)(
            metadata(COLUMN_NAME, "data_scope")
        )
        .property("menu_check_strictly", &SysRole::menu_check_strictly)(
            metadata(COLUMN_NAME, "menu_check_strictly")
        )
        .property("dept_check_strictly", &SysRole::dept_check_strictly)(
            metadata(COLUMN_NAME, "dept_check_strictly")
        )
        .property("status", &SysRole::status)(
            metadata(COLUMN_NAME, "status")
        )
        .property("del_flag", &SysRole::del_flag)(
            metadata(COLUMN_NAME, "del_flag"),
            metadata(LOGICAL_DELETE_FIELD, true)
        )
        .property("create_by", &SysRole::create_by)(
            metadata(COLUMN_NAME, "create_by")
        )
        .property("create_time", &SysRole::create_time)(
            metadata(COLUMN_NAME, "create_time"),
            metadata(CREATE_TIME_FIELD, true)
        )
        .property("update_by", &SysRole::update_by)(
            metadata(COLUMN_NAME, "update_by")
        )
        .property("update_time", &SysRole::update_time)(
            metadata(COLUMN_NAME, "update_time"),
            metadata(UPDATE_TIME_FIELD, true)
        )
        .property("remark", &SysRole::remark)(
            metadata(COLUMN_NAME, "remark")
        );

    // ================ 菜单表映射 ================
    registration::class_<SysMenu>("sys_menu")
        (
            metadata(TABLE_NAME, "sys_menu")
        )
        .property("menu_id", &SysMenu::menu_id)(
            metadata(PRIMARY_KEY, true),
            metadata(AUTO_INCREMENT, true)
        )
        .property("menu_name", &SysMenu::menu_name)(
            metadata(COLUMN_NAME, "menu_name")
        )
        .property("parent_id", &SysMenu::parent_id)(
            metadata(COLUMN_NAME, "parent_id")
        )
        .property("order_num", &SysMenu::order_num)(
            metadata(COLUMN_NAME, "order_num")
        )
        .property("path", &SysMenu::path)(
            metadata(COLUMN_NAME, "path")
        )
        .property("component", &SysMenu::component)(
            metadata(COLUMN_NAME, "component")
        )
        .property("query", &SysMenu::query)(
            metadata(COLUMN_NAME, "query")
        )
        .property("route_name", &SysMenu::route_name)(
            metadata(COLUMN_NAME, "route_name")
        )
        .property("is_frame", &SysMenu::is_frame)(
            metadata(COLUMN_NAME, "is_frame")
        )
        .property("is_cache", &SysMenu::is_cache)(
            metadata(COLUMN_NAME, "is_cache")
        )
        .property("menu_type", &SysMenu::menu_type)(
            metadata(COLUMN_NAME, "menu_type")
        )
        .property("visible", &SysMenu::visible)(
            metadata(COLUMN_NAME, "visible")
        )
        .property("status", &SysMenu::status)(
            metadata(COLUMN_NAME, "status")
        )
        .property("perms", &SysMenu::perms)(
            metadata(COLUMN_NAME, "perms")
        )
        .property("icon", &SysMenu::icon)(
            metadata(COLUMN_NAME, "icon")
        )
        .property("create_by", &SysMenu::create_by)(
            metadata(COLUMN_NAME, "create_by")
        )
        .property("create_time", &SysMenu::create_time)(
            metadata(COLUMN_NAME, "create_time"),
            metadata(CREATE_TIME_FIELD, true)
        )
        .property("update_by", &SysMenu::update_by)(
            metadata(COLUMN_NAME, "update_by")
        )
        .property("update_time", &SysMenu::update_time)(
            metadata(COLUMN_NAME, "update_time"),
            metadata(UPDATE_TIME_FIELD, true)
        )
        .property("remark", &SysMenu::remark)(
            metadata(COLUMN_NAME, "remark")
        );

    // ================ 用户角色关联表映射 ================
    registration::class_<SysUserRole>("sys_user_role")
        (
            metadata(TABLE_NAME, "sys_user_role")
        )
        .property("user_id", &SysUserRole::user_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "user_id")
        )
        .property("role_id", &SysUserRole::role_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "role_id")
        );

    // ================ 角色菜单关联表映射 ================
    registration::class_<SysRoleMenu>("sys_role_menu")
        (
            metadata(TABLE_NAME, "sys_role_menu")
        )
        .property("role_id", &SysRoleMenu::role_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "role_id")
        )
        .property("menu_id", &SysRoleMenu::menu_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "menu_id")
        );

    // ================ 角色部门关联表映射 ================
    registration::class_<SysRoleDept>("sys_role_dept")
        (
            metadata(TABLE_NAME, "sys_role_dept")
        )
        .property("role_id", &SysRoleDept::role_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "role_id")
        )
        .property("dept_id", &SysRoleDept::dept_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "dept_id")
        );

    // ================ 用户岗位关联表映射 ================
    registration::class_<SysUserPost>("sys_user_post")
        (
            metadata(TABLE_NAME, "sys_user_post")
        )
        .property("user_id", &SysUserPost::user_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "user_id")
        )
        .property("post_id", &SysUserPost::post_id)(
            metadata(PRIMARY_KEY, true),
            metadata(COLUMN_NAME, "post_id")
        );
}

#endif // SYS_ENTITIES_MAPPING_HPP 