#include <lvgl.h>
#include <string.h>
#include <stdlib.h>

#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/widgets/wpm_status.h>

/* 
 * Ý tưởng:
 * - Khởi tạo widget layer_status và wpm_status (ZMK đã làm sẵn logic đo WPM & layer).
 * - Ẩn hai widget này đi, chỉ dùng để lấy text/giá trị.
 * - Vẽ label Layer (khung [ABC]/[NUM]/[↑↓←→]/[L2]) và thanh WPM bar + số.
 * - Cập nhật định kỳ bằng lv_timer.
 */

/* Widget gốc của ZMK (ẩn) */
static struct zmk_widget_layer_status layer_widget;
static struct zmk_widget_wpm_status   wpm_widget;

/* Đối tượng hiển thị của chúng ta (hiện) */
static lv_obj_t *layer_box_label;
static lv_obj_t *wpm_bar;
static lv_obj_t *wpm_num_label;

/* --- Trợ giúp: lấy label text từ widget ZMK --- */
/* Một số phiên bản ZMK có hàm getter; nếu bản của bạn không có,
   chúng ta sẽ lấy child đầu tiên kiểu label từ container được trả về khi init. */

static lv_obj_t *get_label_from_container(lv_obj_t *container) {
    /* Lấy child đầu tiên; giả định là label của widget. */
    /* LVGL v8: lv_obj_get_child(container, index) */
    lv_obj_t *child = lv_obj_get_child(container, 0);
    return child; /* Nếu cần, bạn có thể duyệt thêm để chắc là label */
}

/* Hàm cập nhật màn hình định kỳ */
static void status_update_cb(lv_timer_t *timer) {
    /* ----- LAYER -> khung text ----- */
    /* Lấy container đã init của layer_widget (được trả về từ init, mình lưu trong user data timer) */
    lv_obj_t **objs = (lv_obj_t **)timer->user_data;
    lv_obj_t *layer_container = objs[0];
    lv_obj_t *wpm_container   = objs[1];

    const char *layer_txt = NULL;
    {
        lv_obj_t *layer_label_src = get_label_from_container(layer_container);
        if (layer_label_src) {
            layer_txt = lv_label_get_text(layer_label_src);
        }
    }

    const char *box = "[?]";
    if (layer_txt) {
        /* Map theo tên layer. 
           Gợi ý: đặt name layer trong keymap là "Base", "Num", "Nav", "L2" để bắt dễ. */
        if (strstr(layer_txt, "Base") || strstr(layer_txt, "ABC")) {
            box = "[ABC]";
        } else if (strstr(layer_txt, "Num") || strstr(layer_txt, "Number") || strstr(layer_txt, "NUM")) {
            box = "[NUM]";
        } else if (strstr(layer_txt, "Nav") || strstr(layer_txt, "NAV")) {
            box = "[↑↓←→]";
        } else {
            /* Layer 2 hoặc bất kỳ layer còn lại */
            box = "[L2]";
        }
    }
    lv_label_set_text(layer_box_label, box);

    /* ----- WPM -> bar + số ----- */
    int wpm_val = 0;
    {
        lv_obj_t *wpm_label_src = get_label_from_container(wpm_container);
        if (wpm_label_src) {
            const char *wpm_txt = lv_label_get_text(wpm_label_src);
            /* wpm_txt thường kiểu "65 WPM" hoặc "65" -> atoi vẫn ổn */
            if (wpm_txt) {
                wpm_val = atoi(wpm_txt);
                if (wpm_val < 0) wpm_val = 0;
                if (wpm_val > 160) wpm_val = 160; /* trần hợp lý cho bar */
            }
        }
    }

    lv_bar_set_value(wpm_bar, wpm_val, LV_ANIM_OFF);

    char buf[16];
    snprintf(buf, sizeof(buf), "%dwpm", wpm_val);
    lv_label_set_text(wpm_num_label, buf);
}

/* Hàm ZMK sẽ gọi khi khởi tạo màn hình */
void zmk_display_status_init(void) {
    lv_obj_t *scr = lv_scr_act();

    /* 1) Tạo widget ZMK (ẩn), dùng để lấy dữ liệu */
    lv_obj_t *layer_container = zmk_widget_layer_status_init(&layer_widget, scr);
    lv_obj_add_flag(layer_container, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *wpm_container = zmk_widget_wpm_status_init(&wpm_widget, scr);
    lv_obj_add_flag(wpm_container, LV_OBJ_FLAG_HIDDEN);

    /* 2) Layer box label (dòng 1) */
    layer_box_label = lv_label_create(scr);
    lv_label_set_text(layer_box_label, "[ABC]"); /* mặc định */
    lv_obj_align(layer_box_label, LV_ALIGN_TOP_LEFT, 0, 0);
    /* Tùy chọn font to hơn (nếu project bạn đã enable font): 
       lv_obj_set_style_text_font(layer_box_label, &lv_font_montserrat_14, 0); */

    /* 3) WPM bar (dòng 2) */
    wpm_bar = lv_bar_create(scr);
    lv_obj_set_size(wpm_bar, 100, 10);         /* rộng 100px, cao 10px cho dễ nhìn */
    lv_bar_set_range(wpm_bar, 0, 160);         /* thang WPM 0..160 */
    lv_obj_align(wpm_bar, LV_ALIGN_TOP_LEFT, 0, 16);

    /* Số WPM bên phải thanh bar */
    wpm_num_label = lv_label_create(scr);
    lv_label_set_text(wpm_num_label, "0wpm");
    lv_obj_align_to(wpm_num_label, wpm_bar, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    /* 4) Timer update (mỗi 200ms) */
    static lv_obj_t *objs[2];
    objs[0] = layer_container;
    objs[1] = wpm_container;
    lv_timer_create(status_update_cb, 200, objs);
}
