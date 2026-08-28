#include "dialog.h"
#include <lvgl.h>
#include "fonts.h"

static lv_obj_t *current_dialog = NULL;

static void dialog_close_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lv_obj_t* dialog = (lv_obj_t*)lv_event_get_user_data(e);
        lv_obj_del(dialog);
        current_dialog = NULL;
    }
}

static void dialog_bg_close_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lv_obj_t* dialog = (lv_obj_t*)lv_event_get_user_data(e);
        lv_obj_del(dialog);
        current_dialog = NULL;
    }
}

// One-shot timer (see lv_timer_set_repeat_count below); closes the dialog
// only if it's still the one showing, in case the user already tapped it away.
static void dialog_auto_close_cb(lv_timer_t* timer)
{
    lv_obj_t* dialog = (lv_obj_t*)lv_timer_get_user_data(timer);
    if(current_dialog == dialog) {
        lv_obj_del(dialog);
        current_dialog = NULL;
    }
}

void showDialog(const char* message, uint32_t auto_close_ms)
{
    // If a dialog is already open, close it
    if(current_dialog != NULL) {
        lv_obj_del(current_dialog);
        current_dialog = NULL;
    }

    // Creiamo il contenitore principale del dialogo (overlay modale)
    lv_obj_t* dialog_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dialog_bg, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(dialog_bg, 0, 0);
    lv_obj_set_style_bg_color(dialog_bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(dialog_bg, LV_OPA_50, 0);
    lv_obj_set_style_border_width(dialog_bg, 0, 0);
    lv_obj_set_style_pad_all(dialog_bg, 0, 0);
    lv_obj_add_flag(dialog_bg, LV_OBJ_FLAG_CLICKABLE);

    current_dialog = dialog_bg;

    // Creiamo la finestra del dialogo
    lv_obj_t* dialog_win = lv_obj_create(dialog_bg);
    lv_obj_set_size(dialog_win, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(dialog_win, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(dialog_win, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dialog_win, 2, 0);
    lv_obj_set_style_border_color(dialog_win, lv_color_hex(0x666666), 0);
    lv_obj_set_style_shadow_width(dialog_win, 20, 0);
    lv_obj_set_style_shadow_opa(dialog_win, LV_OPA_30, 0);
    lv_obj_set_style_radius(dialog_win, 10, 0);
    lv_obj_set_style_pad_all(dialog_win, 20, 0);
    lv_obj_center(dialog_win);

    // Vertical layout for the content
    lv_obj_set_layout(dialog_win, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(dialog_win, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(dialog_win, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(dialog_win, 15, 0);

    // label for the message
    lv_obj_t* msg_label = lv_label_create(dialog_win);
    lv_label_set_text(msg_label, message);
    lv_obj_set_style_text_color(msg_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(msg_label, get_font_12(), 0);
    lv_obj_set_style_text_align(msg_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(msg_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg_label, 250); // Max text size

    // "OK" button
    lv_obj_t* ok_btn = lv_btn_create(dialog_win);
    lv_obj_set_size(ok_btn, 80, 35);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x007ACC), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ok_btn, 5, 0);

    // Button label
    lv_obj_t* ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "OK");
    lv_obj_set_style_text_color(ok_label, lv_color_white(), 0);
    lv_obj_center(ok_label);

    // Aggiungiamo gli event handler
    lv_obj_add_event_cb(ok_btn, dialog_close_cb, LV_EVENT_CLICKED, dialog_bg);
    lv_obj_add_event_cb(dialog_bg, dialog_bg_close_cb, LV_EVENT_CLICKED, dialog_bg);

    // Preveniamo la propagazione del click dalla finestra al background
    lv_obj_add_flag(dialog_win, LV_OBJ_FLAG_EVENT_BUBBLE);

    if(auto_close_ms > 0) {
        lv_timer_t* t = lv_timer_create(dialog_auto_close_cb, auto_close_ms, dialog_bg);
        lv_timer_set_repeat_count(t, 1);
    }
}
