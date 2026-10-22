/**
 * @file app_config.h
 * @brief Выбор активной лабораторной работы (compiler switch).
 *
 * Все лабораторные живут в одном проекте. main.cpp передаёт управление
 * (setup/loop) приложению, выбранному символом APP_NAME.
 * Чтобы собрать другую лабу — поменяйте одну строку ниже и пересоберите.
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_LAB_3_1   31   // 3.1 Датчики — снятие сигнала
#define APP_LAB_3_2   32   // 3.2 Датчики — кондиционирование сигнала

#define APP_NAME      APP_LAB_3_1

#endif // APP_CONFIG_H
