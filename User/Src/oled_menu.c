#include "oled_menu.h"

#define PAGE_ITEMS_NUM(x)   sizeof(x) / sizeof(MenuItem)        // 菜单选项计算数量函数
#define VISIBLE_MENU_NUM 4                                      // OLED屏幕最多显示多少选项

void led_toggle(void)
{
    WR_TASK_PERIODIC(led_test_id, 500);
    DL_GPIO_togglePins(GPIOB, DL_GPIO_PIN_18);
}
void led_on(void)
{
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_18);
}
void led_off(void)
{
    DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_18);
}

void buzzer_toggle(void)
{
    WR_TASK_PERIODIC(buzzer_test_id, 1000);
    DL_GPIO_togglePins(GPIOB, DL_GPIO_PIN_27); // 蜂鸣器
}

void buzzer_on(void)
{
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_27);
}
void buzzer_off(void)
{
    DL_GPIO_clearPins(GPIOB, DL_GPIO_PIN_27);
}

void Idle_func(void)
{

}
/**************************************************************************************************************/
/****************************************** 头部声明菜单 *******************************************************/
/**************************************************************************************************************/

static const MenuPage main_root_menu;
static const MenuPage led_ctrl_menu;
static const MenuPage buzzer_ctrl_menu;

/**************************************************************************************************************/
/****************************************** 填充菜单内容 *******************************************************/
/**************************************************************************************************************/

static const MenuItem main_root_menu_items[] = {
    {"led_contorl", NULL, &led_ctrl_menu},
    {"buzzer_contorl", NULL, &buzzer_ctrl_menu}
};
static const MenuPage main_root_menu = {main_root_menu_items, PAGE_ITEMS_NUM(main_root_menu_items), NULL};

static MenuItem led_ctrl_menu_items[] = {
    {"led_toggle", led_toggle, NULL}, 
    {"led_on", led_on, NULL}, 
    {"led_off", led_off, NULL}
};
static const MenuPage led_ctrl_menu = {led_ctrl_menu_items, PAGE_ITEMS_NUM(led_ctrl_menu_items), &main_root_menu};

static const MenuItem buzzer_ctrl_menu_items[] = {
    {"buzzer_toggle", buzzer_toggle, NULL}, 
    {"buzzer_on", buzzer_on, NULL}, 
    {"buzzer_off", buzzer_off, NULL}
};
static const MenuPage buzzer_ctrl_menu = {buzzer_ctrl_menu_items, PAGE_ITEMS_NUM(led_ctrl_menu_items), &main_root_menu};

/**************************************************************************************************************/
/************************************** OLED菜单所用变量声明 ***************************************************/
/**************************************************************************************************************/

static const MenuPage *show_menupage = &main_root_menu;
static uint8_t menu_start_index = 0;
static uint8_t showpage_index = 0;
static const MenuPage *last_menupage = NULL;
static FuncPtr now_func = Idle_func;
static uint8_t menu_refresh_flag = 1;

/**************************************************************************************************************/
/**************************************** 功能函数定义部分 *****************************************************/
/**************************************************************************************************************/

/**
 * @brief 按键控制选项菜单向上偏移
 * 
 */
static void Menu_keyup(void)
{
    if (showpage_index)
    {
        showpage_index--;
        if (showpage_index < menu_start_index)
        {
            menu_start_index--;
        }
    }else{
        showpage_index =  show_menupage->num - 1;
        if (show_menupage->num > VISIBLE_MENU_NUM)
        {
            menu_start_index = show_menupage->num - VISIBLE_MENU_NUM;
        }else {
            menu_start_index = 0;
        }
    }
    menu_refresh_flag++;
}

/**
 * @brief 按键控制菜单选项向下偏移
 * 
 */
static void Menu_keydown(void)
{
    if (showpage_index < (show_menupage->num - 1) )
    {
        showpage_index++;
        if (showpage_index >= menu_start_index + VISIBLE_MENU_NUM)
        {
            menu_start_index++;
        }
    }else{
        showpage_index = 0;
        menu_start_index = 0;
    }
    menu_refresh_flag++;
}

/**
 * @brief 按键控制菜单执行选项指向函数
 * 
 */
static void Menu_keyexecute(void)
{
    if(show_menupage->items[showpage_index].func != NULL)
    {
        now_func = show_menupage->items[showpage_index].func;
        menu_refresh_flag++;
    }

}

/**
 * @brief 按键控制菜单进入选项子级菜单
 * 
 */
static void Menu_keyenter(void)
{
    MenuItem now_item = show_menupage->items[showpage_index];

    if (now_item.chil_menupage != NULL)
    {
        last_menupage = show_menupage;
        show_menupage = now_item.chil_menupage;
        showpage_index = 0;
        menu_start_index = 0;
        menu_refresh_flag++;
    }
}

/**
 * @brief 按键控制菜单返回父级菜单
 * 
 */
static void Menu_keyback(void)
{
    if (last_menupage != NULL)
    {
        show_menupage = last_menupage;
        showpage_index = 0;
        last_menupage = last_menupage->parent_menupage;
        menu_refresh_flag++;
    }
}

/**************************************************************************************************************/
/**************************************** 菜单逻辑主要部分 *****************************************************/
/**************************************************************************************************************/

/**
 * @brief 菜单主功能函数
 * 
 */
void Menu_Task(void)
{
    now_func();
    WR_KeyControlTask(Menu_keyexecute, &bsp_key_param[Key_center].key_shortpressflag);
    WR_KeyControlTask(Menu_keyup, &bsp_key_param[Key_up].key_shortpressflag);
    WR_KeyControlTask(Menu_keydown, &bsp_key_param[Key_down].key_shortpressflag);
    WR_KeyControlTask(Menu_keyenter, &bsp_key_param[Key_right].key_shortpressflag);
    WR_KeyControlTask(Menu_keyback, &bsp_key_param[Key_left].key_shortpressflag);

    if(!menu_refresh_flag) return;
    
    OLED_ClearScreen();   // 清屏（只在需要刷新时清）

    for (uint8_t i = 0; i < VISIBLE_MENU_NUM; i++)
    {
        uint8_t item_idx = menu_start_index + i;

        if (item_idx >= show_menupage->num) break;

        if (item_idx == showpage_index)
        {
            OLED_Printf_Regret(0, i * 2, 8, show_menupage->items[item_idx].menu_name);
        }
        else
        {
            OLED_Printf_Hope(0, i * 2, 8, show_menupage->items[item_idx].menu_name);
        }
    }

    menu_refresh_flag = 0; // 刷新完成，清零标志（不再重复绘制）
}
