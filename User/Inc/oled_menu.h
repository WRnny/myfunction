#ifndef OLED_MENU_H
#define OLED_MENU_H

#include "my_Ticks.h"
#include "bsp_key.h"
#include "bsp_oledi2c.h"

/* 前向声明 */
typedef struct MenuItem MenuItem;
typedef struct MenuPage MenuPage;

typedef void (*FuncPtr)(void);

/* 定义 MenuItem - 注意没有 typedef！ */
struct MenuItem {
    char *menu_name;                            // 选项名称
    void (*func)(void);                         // 选项指向函数
    const MenuPage *chil_menupage;              // 选项指向子菜单地址
};

/* 定义 MenuPage - 注意没有 typedef！ */
struct MenuPage {
    const MenuItem *items;                      // 选项信息存放数组
    uint8_t num;                                // 存放选项的数量
    const MenuPage *parent_menupage;            // 菜单页的父类菜单地址
};

/**
 * @brief 菜单主功能函数
 * 
 */
void Menu_Task(void);

#endif