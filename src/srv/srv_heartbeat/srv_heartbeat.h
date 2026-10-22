/**
 * @file srv_heartbeat.h
 * @brief Сервис heartbeat: встроенный светодиод (D13) мигает, пока система жива.
 *
 * Отдельная задача FreeRTOS. Если LED перестал мигать — планировщик
 * или одна из задач «зависли». Простая и полезная диагностика.
 */
#ifndef SRV_HEARTBEAT_H
#define SRV_HEARTBEAT_H

#include <Arduino_FreeRTOS.h>
#include <task.h>

#define SRV_HEARTBEAT_PIN         LED_BUILTIN   // D13, светодиод «L» на плате
#define SRV_HEARTBEAT_REC_MS      500           // период переключения
#define SRV_HEARTBEAT_OFFSET_MS   0
#define SRV_HEARTBEAT_STACK       80
#define SRV_HEARTBEAT_PRIORITY    1

/** Создаёт задачу heartbeat. Вызывать в setup() приложения. */
void srv_heartbeat_setup(void);

TaskHandle_t srv_heartbeat_get_task_handle(void);

#endif // SRV_HEARTBEAT_H
