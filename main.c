#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include "filters.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void anim_run();
 

// CSS стиль 

static const char* APP_CSS =
"window {"
"  background-color: #FFF0E6;"
"}"
"button {"
"  background: linear-gradient(to bottom, #FFD9B3, #D4A843);"
"  color: #5C3A00;"
"  border: 2px solid #D4A843;"
"  border-radius: 10px;"
"  padding: 8px 16px;"
"  font-weight: bold;"
"  font-size: 13px;"
"}"
"button:hover {"
"  background: linear-gradient(to bottom, #FFE8CC, #E8B84B);"
"  border-color: #C49020;"
"}"
"button:active {"
"  background: linear-gradient(to bottom, #D4A843, #FFD9B3);"
"}"
"frame {"
"  border: 2px solid #D4A843;"
"  border-radius: 12px;"
"  background-color: #FFF8F0;"
"}"
"frame > label {"
"  color: #8B5E00;"
"  font-weight: bold;"
"  font-size: 13px;"
"}"
"radiobutton {"
"  color: #5C3A00;"
"  font-size: 13px;"
"}"
"radiobutton:checked label {"
"  color: #D4A843;"
"  font-weight: bold;"
"}"
"label {"
"  color: #5C3A00;"
"  font-size: 13px;"
"}"
"combobox {"
"  border: 1px solid #D4A843;"
"  border-radius: 6px;"
"  background-color: #FFF8F0;"
"  color: #5C3A00;"
"}"
"spinbutton {"
"  border: 1px solid #D4A843;"
"  border-radius: 6px;"
"  background-color: #FFF8F0;"
"  color: #5C3A00;"
"}"
"progressbar trough {"
"  background-color: #FFE4C4;"
"  border: 1px solid #D4A843;"
"  border-radius: 6px;"
"  min-height: 12px;"
"}"
"progressbar progress {"
"  background: linear-gradient(to right, #FFD9B3, #D4A843);"
"  border-radius: 6px;"
"}";
 

// Глобальное состояние

static Image_struct src_img = {0};
static Image_struct res_img = {0};
 
static GtkWidget* img_src_widget;
static GtkWidget* img_res_widget;
static GtkWidget* progress_bar;
static GtkWidget* radio_median;
static GtkWidget* radio_gauss;
static GtkWidget* radio_sobel;
static GtkWidget* radio_conv;
static GtkWidget* radio_pixel;
static GtkWidget* spin_radius;
static GtkWidget* spin_sigma;
static GtkWidget* combo_kernel;
static GtkWidget* box_sigma;
static GtkWidget* box_kernel;
 

// Вспомогательные функции

GdkPixbuf* image_to_pixbuf(Image_struct* im) {
    int c = im->channels;
    if (c == 1) {
        unsigned char* rgb = malloc(im->width * im->height * 3);
        for (int i = 0; i < im->width * im->height; i++) {
            rgb[i*3+0] = im->img[i];
            rgb[i*3+1] = im->img[i];
            rgb[i*3+2] = im->img[i];
        }
        GdkPixbuf* pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, im->width, im->height);
        int row = gdk_pixbuf_get_rowstride(pb);
        guchar* px = gdk_pixbuf_get_pixels(pb);
        for (int y = 0; y < im->height; y++)
            memcpy(px + y * row, rgb + y * im->width * 3, im->width * 3);
        free(rgb);
        return pb;
    }
    GdkPixbuf* pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, c == 4, 8, im->width, im->height);
    int row = gdk_pixbuf_get_rowstride(pb);
    guchar* px = gdk_pixbuf_get_pixels(pb);
    for (int y = 0; y < im->height; y++)
        memcpy(px + y * row, im->img + y * im->width * c, im->width * c);
    return pb;
}
 
void show_image(GtkWidget* widget, Image_struct* im) {
    GdkPixbuf* pb = image_to_pixbuf(im);
    int sw = 480, sh = 340;
    double scale = fmin((double)sw / im->width, (double)sh / im->height);
    int nw = (int)(im->width  * scale);
    int nh = (int)(im->height * scale);
    GdkPixbuf* scaled = gdk_pixbuf_scale_simple(pb, nw, nh, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(widget), scaled);
    g_object_unref(pb);
    g_object_unref(scaled);
}
 

// Fade-in

static float fade_alpha = 0.0f;
 
static gboolean fade_tick(gpointer data) {
    fade_alpha += 0.06f;
    if (fade_alpha >= 1.0f) {
        gtk_widget_set_opacity(img_res_widget, 1.0);
        return G_SOURCE_REMOVE;
    }
    gtk_widget_set_opacity(img_res_widget, fade_alpha);
    return G_SOURCE_CONTINUE;
}
 

// Показать/скрыть параметры

static void on_filter_changed(GtkToggleButton* tb, gpointer data) {
    gboolean is_gauss = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_gauss));
    gboolean is_conv  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_conv));
    gtk_widget_set_visible(box_sigma,  is_gauss);
    gtk_widget_set_visible(box_kernel, is_conv);
}
 

// Открыть файл

static void on_open(GtkButton* btn, gpointer data) {
    GtkWidget* dlg = gtk_file_chooser_dialog_new(
        "Открыть изображение", NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Отмена",  GTK_RESPONSE_CANCEL,
        "_Открыть", GTK_RESPONSE_ACCEPT,
        NULL
    );
    GtkFileFilter* ff = gtk_file_filter_new();
    gtk_file_filter_set_name(ff, "Изображения (*.png, *.jpg)");
    gtk_file_filter_add_pattern(ff, "*.png");
    gtk_file_filter_add_pattern(ff, "*.jpg");
    gtk_file_filter_add_pattern(ff, "*.jpeg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), ff);
 
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        if (src_img.img) stbi_image_free(src_img.img);
        src_img.img = stbi_load(path, &src_img.width, &src_img.height, &src_img.channels, 0);
        if (src_img.img) {
            show_image(img_src_widget, &src_img);
            gtk_image_clear(GTK_IMAGE(img_res_widget));
            if (res_img.img) { free(res_img.img); res_img.img = NULL; }
        } else {
            GtkWidget* err = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK, "Не удалось загрузить файл:\n%s", path);
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        }
        g_free(path);
    }
    gtk_widget_destroy(dlg);
}
 

// Применить фильтр

static void on_apply(GtkButton* btn, gpointer data) {
    if (!src_img.img) {
        GtkWidget* dlg = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK, "Сначала откройте изображение!");
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return;
    }
 
    gtk_widget_set_visible(progress_bar, TRUE);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.1);
    while (gtk_events_pending()) gtk_main_iteration();
 
    if (res_img.img) { free(res_img.img); res_img.img = NULL; }
 
    int radius = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_radius));
 
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.3);
    while (gtk_events_pending()) gtk_main_iteration();
 
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_median))) {
        median(&src_img, &res_img, radius);
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_gauss))) {
        float sigma = (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_sigma));
        gauss(&src_img, &res_img, radius, sigma);
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_sobel))) {
        detector(&src_img, &res_img);
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_conv))) {
        int ki = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_kernel));
        if (ki < 0) ki = 0;
        convolution(&src_img, &res_img, 1, kernels[ki]);
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_pixel))) {
        pixel(&src_img, &res_img, radius * 4);
    }
 
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.85);
    while (gtk_events_pending()) gtk_main_iteration();
 
    fade_alpha = 0.0f;
    gtk_widget_set_opacity(img_res_widget, 0.0);
    show_image(img_res_widget, &res_img);
    g_timeout_add(16, fade_tick, NULL);
 
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 1.0);
    while (gtk_events_pending()) gtk_main_iteration();
    gtk_widget_set_visible(progress_bar, FALSE);
 
    anim_run();
}
 

// Сохранить

static void on_save(GtkButton* btn, gpointer data) {
    if (!res_img.img) {
        GtkWidget* dlg = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK, "Сначала примените фильтр!");
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return;
    }
    GtkWidget* dlg = gtk_file_chooser_dialog_new(
        "Сохранить результат", NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Отмена",    GTK_RESPONSE_CANCEL,
        "_Сохранить", GTK_RESPONSE_ACCEPT,
        NULL
    );
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dlg), "result.png");
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        stbi_write_png(path, res_img.width, res_img.height, res_img.channels, res_img.img, 0);
        g_free(path);
    }
    gtk_widget_destroy(dlg);
}
 





int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
 
    // Применяем CSS
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css, APP_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
 
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "✨ Image Filter App");
    gtk_window_set_default_size(GTK_WINDOW(window), 1060, 720);
    gtk_container_set_border_width(GTK_CONTAINER(window), 16);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
 
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_add(GTK_CONTAINER(window), vbox);
 
    // Заголовок
    GtkWidget* title = gtk_label_new("✨  Image Filter App  ✨");
    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_scale_new(1.6));
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(attrs, pango_attr_foreground_new(0xD4D4 * 256, 0xA8A8 * 256, 0x4343 * 256));
    gtk_label_set_attributes(GTK_LABEL(title), attrs);
    pango_attr_list_unref(attrs);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 4);
 
    // Кнопка открытия
    GtkWidget* btn_open = gtk_button_new_with_label("📂   Открыть изображение");
    g_signal_connect(btn_open, "clicked", G_CALLBACK(on_open), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), btn_open, FALSE, FALSE, 0);
 
    // Два фрейма с изображениями
    GtkWidget* hbox_img = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_img, TRUE, TRUE, 0);
 
    GtkWidget* frame_src = gtk_frame_new("  Оригинал  ");
    img_src_widget = gtk_image_new();
    gtk_widget_set_size_request(img_src_widget, 480, 340);
    gtk_container_add(GTK_CONTAINER(frame_src), img_src_widget);
    gtk_box_pack_start(GTK_BOX(hbox_img), frame_src, TRUE, TRUE, 0);
 
    GtkWidget* frame_res = gtk_frame_new("  Результат  ");
    img_res_widget = gtk_image_new();
    gtk_widget_set_size_request(img_res_widget, 480, 340);
    gtk_container_add(GTK_CONTAINER(frame_res), img_res_widget);
    gtk_box_pack_start(GTK_BOX(hbox_img), frame_res, TRUE, TRUE, 0);
 
    // Прогресс-бар
    progress_bar = gtk_progress_bar_new();
    gtk_widget_set_visible(progress_bar, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);
 
    // Фрейм фильтров
    GtkWidget* frame_f = gtk_frame_new("  Настройки фильтра  ");
    GtkWidget* vbox_f  = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox_f), 10);
    gtk_container_add(GTK_CONTAINER(frame_f), vbox_f);
    gtk_box_pack_start(GTK_BOX(vbox), frame_f, FALSE, FALSE, 0);
 
    // Радиокнопки
    GtkWidget* hbox_radio = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start(GTK_BOX(vbox_f), hbox_radio, FALSE, FALSE, 0);
 
    radio_median = gtk_radio_button_new_with_label(NULL, "Медианный");
    radio_gauss  = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_median), "Гаусс");
    radio_sobel  = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_median), "Собель");
    radio_conv   = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_median), "Свёртка");
    radio_pixel  = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_median), "Пикселизация");
 
    gtk_box_pack_start(GTK_BOX(hbox_radio), radio_median, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_radio), radio_gauss,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_radio), radio_sobel,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_radio), radio_conv,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_radio), radio_pixel,  FALSE, FALSE, 0);
 
    g_signal_connect(radio_median, "toggled", G_CALLBACK(on_filter_changed), NULL);
    g_signal_connect(radio_gauss,  "toggled", G_CALLBACK(on_filter_changed), NULL);
    g_signal_connect(radio_sobel,  "toggled", G_CALLBACK(on_filter_changed), NULL);
    g_signal_connect(radio_conv,   "toggled", G_CALLBACK(on_filter_changed), NULL);
    g_signal_connect(radio_pixel,  "toggled", G_CALLBACK(on_filter_changed), NULL);
 
    // Радиус
    GtkWidget* hbox_r = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox_f), hbox_r, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_r), gtk_label_new("Радиус:"), FALSE, FALSE, 0);
    spin_radius = gtk_spin_button_new_with_range(1, 10, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_radius), 1);
    gtk_box_pack_start(GTK_BOX(hbox_r), spin_radius, FALSE, FALSE, 0);
 
    // Sigma
    box_sigma = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox_f), box_sigma, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_sigma), gtk_label_new("Sigma:"), FALSE, FALSE, 0);
    spin_sigma = gtk_spin_button_new_with_range(0.1, 5.0, 0.1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_sigma), 1.0);
    gtk_box_pack_start(GTK_BOX(box_sigma), spin_sigma, FALSE, FALSE, 0);
    gtk_widget_set_visible(box_sigma, FALSE);
 
    // Ядро свёртки
    box_kernel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox_f), box_kernel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_kernel), gtk_label_new("Ядро:"), FALSE, FALSE, 0);
    combo_kernel = gtk_combo_box_text_new();
    for (int i = 0; i < 4; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_kernel), kernel_names[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_kernel), 0);
    gtk_box_pack_start(GTK_BOX(box_kernel), combo_kernel, FALSE, FALSE, 0);
    gtk_widget_set_visible(box_kernel, FALSE);
 
    // Кнопки
    GtkWidget* hbox_btn = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_btn, FALSE, FALSE, 0);
 
    GtkWidget* btn_apply = gtk_button_new_with_label("▶   Применить");
    GtkWidget* btn_save  = gtk_button_new_with_label("💾   Сохранить");
    g_signal_connect(btn_apply, "clicked", G_CALLBACK(on_apply), NULL);
    g_signal_connect(btn_save,  "clicked", G_CALLBACK(on_save),  NULL);
    gtk_box_pack_start(GTK_BOX(hbox_btn), btn_apply, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_btn), btn_save,  TRUE, TRUE, 0);
 
    gtk_widget_show_all(window);
    gtk_widget_set_visible(box_sigma,    FALSE);
    gtk_widget_set_visible(box_kernel,   FALSE);
    gtk_widget_set_visible(progress_bar, FALSE);
 
    gtk_main();
 
    if (src_img.img) stbi_image_free(src_img.img);
    if (res_img.img) free(res_img.img);
    return 0;
}

