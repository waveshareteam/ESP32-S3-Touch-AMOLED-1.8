/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Settings.hpp"
#include "ui/SettingsUI.hpp"
#include "lvgl.h"
#include "esp_brookesia.hpp"
#ifdef ESP_UTILS_LOG_TAG
#undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:Settings"
#include "esp_lib_utils.h"

// Settings page icons.
LV_IMG_DECLARE(WIFI);
LV_IMG_DECLARE(sound);
LV_IMG_DECLARE(info);
LV_IMG_DECLARE(BLE);
LV_IMG_DECLARE(backlights);
LV_IMG_DECLARE(battery);
LV_IMG_DECLARE(arrow);
LV_IMG_DECLARE(img_app_settings);

namespace esp_brookesia::apps
{
    Settings *Settings::_instance = nullptr;

    Settings *Settings::requestInstance(bool use_status_bar, bool use_navigation_bar)
    {
        if (_instance == nullptr)
        {
            _instance = new Settings(use_status_bar, use_navigation_bar);
        }
        return _instance;
    }

    Settings::Settings(bool use_status_bar, bool use_navigation_bar)
        : App("Settings", &img_app_settings, true, use_status_bar, use_navigation_bar),
          page_root(nullptr), list1(nullptr), active_page(ActivePage::None)
    {
    }

    Settings::~Settings()
    {
    }

    void Settings::create_settings_ui()
    {
        destroy_settings_ui();
        lv_obj_clean(lv_scr_act());

        page_root = settings_ui::create_page(lv_scr_act());
        settings_ui::create_header(page_root, "Settings");
        settings_ui::init_list_styles(style_list, style_list_btn, style_list_text, style_list_btn_pressed);

        list1 = settings_ui::create_content_list(page_root);
        lv_obj_add_style(list1, &style_list, LV_PART_MAIN);

        settings_ui::add_section(list1, "Wireless", style_list_text);
        add_button_with_arrow(&WIFI, "WLAN");

        settings_ui::add_section(list1, "Media", style_list_text);
        add_button_with_arrow(&sound, "Sound");
        add_button_with_arrow(&backlights, "Display");

        settings_ui::add_section(list1, "System", style_list_text);
        add_button_with_arrow(&battery, "Power");
        add_button_with_arrow(LV_SYMBOL_SD_CARD, "Storage");

        settings_ui::add_section(list1, "More", style_list_text);
        add_button_with_arrow(&info, "About");
    }

    void Settings::destroy_settings_ui()
    {
        if (page_root != nullptr)
        {
            lv_obj_del(page_root);
            page_root = nullptr;
            list1 = nullptr;
            settings_ui::reset_list_styles(style_list, style_list_btn, style_list_text, style_list_btn_pressed);
        }
    }

    void Settings::add_button_with_arrow(const void *icon, const char *text)
    {
        lv_obj_t *btn = lv_list_add_button(list1, icon, text);
        lv_obj_add_style(btn, &style_list_btn, LV_PART_MAIN);
        lv_obj_add_style(btn, &style_list_btn_pressed, LV_STATE_PRESSED);
        settings_ui::use_ellipsis_for_button_label(btn);
        lv_obj_add_event_cb(btn, event_handler_cb, LV_EVENT_CLICKED, this);

        lv_obj_t *arrow_img = lv_image_create(btn);
        lv_image_set_src(arrow_img, &arrow);
    }

    void Settings::event_handler_cb(lv_event_t *e)
    {
        Settings *instance = static_cast<Settings *>(lv_event_get_user_data(e));
        if (instance != nullptr)
        {
            instance->event_handler(e);
        }
    }

    void Settings::open_page_async_cb(void *user_data)
    {
        const char *page_name = static_cast<const char *>(user_data);
        Settings *instance = Settings::requestInstance();
        if (instance != nullptr && page_name != nullptr)
        {
            instance->open_page(page_name);
        }
    }

    void Settings::event_handler(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
            return;
        }

        lv_obj_t *obj = static_cast<lv_obj_t *>(lv_event_get_target(e));
        const char *txt = lv_list_get_button_text(list1, obj);
        ESP_UTILS_LOGI("Clicked: %s", txt);

        if (strcmp(txt, "WLAN") == 0)
        {
            lv_async_call(open_page_async_cb, (void *)"WLAN");
        }
        else if (strcmp(txt, "Sound") == 0)
        {
            lv_async_call(open_page_async_cb, (void *)"Sound");
        }
        else if (strcmp(txt, "Display") == 0)
        {
            lv_async_call(open_page_async_cb, (void *)"Display");
        }
        else if (strcmp(txt, "Power") == 0)
        {
            lv_async_call(open_page_async_cb, (void *)"Power");
        }
        else if (strcmp(txt, "Storage") == 0)
        {
            lv_async_call(open_page_async_cb, (void *)"Storage");
        }
        else if (strcmp(txt, "About") == 0)
        {
            lv_async_call(open_page_async_cb, (void *)"About");
        }
    }

    void Settings::open_page(const char *page_name)
    {
        destroy_settings_ui();

        if (strcmp(page_name, "WLAN") == 0)
        {
            active_page = ActivePage::Wlan;
            WlanPage::requestInstance(false, false)->run();
        }
        else if (strcmp(page_name, "Sound") == 0)
        {
            active_page = ActivePage::Sound;
            SoundPage::requestInstance(false, false)->run();
        }
        else if (strcmp(page_name, "Display") == 0)
        {
            active_page = ActivePage::Display;
            DisplayPage::requestInstance(false, false)->run();
        }
        else if (strcmp(page_name, "Power") == 0)
        {
            active_page = ActivePage::Power;
            PowerPage::requestInstance(false, false)->run();
        }
        else if (strcmp(page_name, "Storage") == 0)
        {
            active_page = ActivePage::Storage;
            StoragePage::requestInstance(false, false)->run();
        }
        else if (strcmp(page_name, "About") == 0)
        {
            active_page = ActivePage::About;
            AboutPage::requestInstance(false, false)->run();
        }
    }

    void Settings::close_active_page()
    {
        switch (active_page)
        {
        case ActivePage::Wlan:
            WlanPage::requestInstance(false, false)->close();
            break;
        case ActivePage::Sound:
            SoundPage::requestInstance(false, false)->close();
            break;
        case ActivePage::Display:
            DisplayPage::requestInstance(false, false)->close();
            break;
        case ActivePage::Power:
            PowerPage::requestInstance(false, false)->close();
            break;
        case ActivePage::Storage:
            StoragePage::requestInstance(false, false)->close();
            break;
        case ActivePage::About:
            AboutPage::requestInstance(false, false)->close();
            break;
        case ActivePage::None:
        default:
            break;
        }
        active_page = ActivePage::None;
    }

    void Settings::showRootPage()
    {
        close_active_page();
        create_settings_ui();
    }

    bool Settings::run(void)
    {
        ESP_UTILS_LOGD("Run");
        active_page = ActivePage::None;
        if (page_root == nullptr)
        {
            create_settings_ui();
        }
        return true;
    }

    bool Settings::back(void)
    {
        ESP_UTILS_LOGD("Back");
        if (active_page != ActivePage::None)
        {
            showRootPage();
            return true;
        }
        return notifyCoreClosed();
    }

    bool Settings::close(void)
    {
        ESP_UTILS_LOGD("Close");
        close_active_page();
        destroy_settings_ui();
        return true;
    }

    bool Settings::init()
    {
        ESP_UTILS_LOGD("Init");
        return initWifi() == ESP_OK;
    }

    bool Settings::deinit()
    {
        ESP_UTILS_LOGD("Deinit");
        return true;
    }

    bool Settings::pause()
    {
        ESP_UTILS_LOGD("Pause");
        return true;
    }

    bool Settings::resume()
    {
        ESP_UTILS_LOGD("Resume");
        return true;
    }

    esp_err_t Settings::initWifi()
    {
        // system_status owns native Wi-Fi/netif/event-loop initialization and
        // lifecycle.  Settings only presents its application-facing controls.
        return ESP_OK;
    }
} // namespace esp_brookesia::apps
