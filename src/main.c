#include <stdio.h>
#include <gio/gio.h>
#include "loader.h"

StringFunctions algorithm=NULL;
SetupFunctions setup=NULL;
GtkBuilder* builder=NULL;
static GtkCssProvider *cssProvider;
int pluginsState=0;

void changeTheme(void);

void clear()
{
    GtkLabel* output_label = GTK_LABEL(gtk_builder_get_object(builder,
                                                              widgetName(outputLabel)));
    GtkEntry* input_label = GTK_ENTRY(gtk_builder_get_object(builder, widgetName(inputLabel)));
    gtk_label_set_text(output_label, "");
    gtk_entry_set_text(input_label, "");
}

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void toggledButton()
{
    static gboolean toggle = FALSE;
    GtkWidget* credit = GTK_WIDGET(gtk_builder_get_object(builder,widgetName(creditLabel))),
            *name= GTK_WIDGET(gtk_builder_get_object(builder,widgetName(nameLabel)));
    if(toggle)
    {
        gtk_widget_hide(credit);
        gtk_widget_show(name);
    }
    else
    {
        gtk_widget_show(credit);
        gtk_widget_hide(name);
    }
    toggle = !toggle;
}

void populate_widget()
{
    DIR *d;
    pluginsState=1;
    struct dirent *dir;
    d = opendir("./plugs");
    GtkComboBoxText* combo = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder,
                                                                       widgetName(algosbox)));
    gtk_combo_box_text_remove_all(combo);
    if(d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(dir->d_type != DT_REG)
                continue;
            gtk_combo_box_text_append_text(combo, dir->d_name);
        }
    }
    closedir(d);
    combo = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder,
                                                      widgetName(themeBox)));
    gtk_combo_box_text_remove_all(combo);
    d = opendir("./stylesheets");
    if(d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(dir->d_type != DT_REG)
                continue;
            if(EndsWith(dir->d_name,".css"))
            {
                dir->d_name[strlen(dir->d_name)-4]=0;
                gtk_combo_box_text_append_text(combo, dir->d_name);
            }
        }
    }
    closedir(d);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),0);
    pluginsState=0;
    changeTheme();
}

void changeTheme()
{
    if(pluginsState)
        return;
    char fileName[50] = "./stylesheets/";
    strcat(fileName,
           gtk_combo_box_text_get_active_text(
               GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder,widgetName(themeBox)))));
    strcat(fileName, ".css");
    gtk_css_provider_load_from_path(cssProvider, fileName, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
}


void check()
{
    const char* inputString = gtk_entry_get_text(
                GTK_ENTRY(gtk_builder_get_object(builder, widgetName(inputLabel))));
    if(algorithm == NULL)
        return;
    const char* outputString = process(inputString);
    gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, widgetName(outputLabel))),
                       outputString);
}

int main(int argc, char** argv)
{
    GFileMonitor *stylemon = g_file_monitor_directory(g_file_new_for_path("./stylesheets"),
                                                      G_FILE_MONITOR_WATCH_MOVES,NULL,NULL),
            *plugmon = g_file_monitor_directory(g_file_new_for_path("./plugs"),
                                                G_FILE_MONITOR_WATCH_MOVES,NULL,NULL);
    gtk_init(&argc, &argv);
    builder = gtk_builder_new_from_file(widgetName(./mainWindow.xml));
    GtkWindow* window= GTK_WINDOW(gtk_builder_get_object(builder, widgetName(window_main)));
    cssProvider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_file_chooser_set_current_folder(
                GTK_FILE_CHOOSER(gtk_builder_get_object(builder,widgetName(wordlist))),
                                        "../wordlists/");
    gtk_widget_show(GTK_WIDGET(window));
    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect (stylemon, "changed", populate_widget, NULL);
    g_signal_connect (plugmon, "changed", populate_widget, NULL);
    populate_widget();
    gtk_main();
    g_object_unref(builder);
    g_object_unref(stylemon);
    g_object_unref(plugmon);
    return 0;
}

