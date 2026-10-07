#include "i18n.h"
#include <locale.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>

static char current_lang[16] = "en";
static GHashTable *tr_table = NULL;

typedef struct {
    const char *msgid;
    const char *msgstr;
} MrtTrEntry;

static const MrtTrEntry tr_dictionary[] = {
    /* Menus */
    { "File", "Dosya" },
    { "Edit", "Düzen" },
    { "View", "Görünüm" },
    { "Run", "Çalıştır" },
    { "Help", "Yardım" },

    /* File Actions */
    { "New File", "Yeni Dosya" },
    { "New Project", "Yeni Proje" },
    { "Open File", "Dosya Aç" },
    { "Open Project", "Proje Aç" },
    { "Save", "Kaydet" },
    { "Save As", "Farklı Kaydet" },
    { "Save As…", "Farklı Kaydet…" },
    { "Save All", "Tümünü Kaydet" },
    { "Close File", "Dosyayı Kapat" },
    { "Close Project", "Projeyi Kapat" },
    { "Restore Previous Session", "Önceki Oturumu Geri Yükle" },
    { "Settings", "Ayarlar" },
    { "Exit", "Çıkış" },

    /* Edit Actions */
    { "Undo", "Geri Al" },
    { "Redo", "Yinele" },
    { "Cut", "Kes" },
    { "Copy", "Kopyala" },
    { "Paste", "Yapıştır" },
    { "Select All", "Tümünü Seç" },
    { "Find", "Bul" },
    { "Replace", "Değiştir" },
    { "Go to Line", "Satıra Git" },
    { "Go to Line…", "Satıra Git…" },
    { "Format Document", "Belgeyi Biçimlendir" },
    { "Check Syntax", "Sözdizimini Denetle" },

    /* View Actions */
    { "Toggle Sidebar", "Kenar Çubuğunu Göster/Gizle" },
    { "Toggle Output", "Çıktıyı Göster/Gizle" },
    { "Command Palette", "Komut Paleti" },
    { "Command Palette…", "Komut Paleti…" },
    { "Quick Open", "Hızlı Dosya Aç" },
    { "Quick Open…", "Hızlı Dosya Aç…" },
    { "Find in Files", "Dosyalarda Bul" },
    { "Find in Files…", "Dosyalarda Bul…" },

    /* Run Actions */
    { "Run Active File", "Geçerli Dosyayı Çalıştır" },
    { "Run Current File", "Geçerli Dosyayı Çalıştır" },
    { "Stop", "Durdur" },
    { "Stop Execution", "Çalışmayı Durdur" },

    /* Panels & Labels */
    { "Project", "Proje" },
    { "Files", "Dosyalar" },
    { "Symbols", "Semboller" },
    { "Functions", "Fonksiyonlar" },
    { "Variables", "Değişkenler" },
    { "Output", "Çıktı" },
    { "Problems", "Sorunlar" },
    { "Search", "Arama" },
    { "Search Results", "Arama Sonuçları" },
    { "Clear Output", "Çıktıyı Temizle" },

    /* Buttons */
    { "Cancel", "İptal" },
    { "Create", "Oluştur" },
    { "Open", "Aç" },
    { "Recover", "Kurtar" },
    { "Discard", "Sil" },
    { "Previous", "Önceki" },
    { "Next", "Sonraki" },
    { "Replace All", "Tümünü Değiştir" },
    { "Search…", "Ara…" },

    /* Welcome Page */
    { "Build with MRT.", "MRT ile geliştir." },
    { "Recent Projects", "Son Projeler" },
    { "No recent projects", "Henüz son proje yok" },

    /* Status Bar */
    { "Spaces: %d", "Boşluk: %d" },
    { "Tab: %d", "Sekme: %d" },
    { "Ln %d, Col %d", "Sat %d, Süt %d" },

    /* Dialogs & Messages */
    { "About MRT Studio", "MRT Studio Hakkında" },
    { "Native development for MRT.", "MRT için yerel geliştirme ortamı." },
    { "MRT Studio found unsaved changes from a previous session.", "MRT Studio önceki oturumdan kaydedilmemiş değişiklikler buldu." },
    { "Language changed. Please restart MRT Studio to apply changes completely.", "Arayüz dili değiştirildi. Değişikliklerin tam olarak uygulanması için lütfen MRT Studio'yu yeniden başlatın." },
    { "Unsaved Changes", "Kaydedilmemiş Değişiklikler" },
    { "Save changes to \"%s\" before closing?", "\"%s\" dosyasındaki değişiklikler kapatılmadan önce kaydedilsin mi?" },
    { "New MRT Project", "Yeni MRT Projesi" },
    { "Project Name:", "Proje Adı:" },
    { "Project Location:", "Proje Konumu:" },
    { "Template:", "Şablon:" },
    { "Empty Project", "Boş Proje" },
    { "Console Application", "Konsol Uygulaması" },
    { "Library", "Kütüphane" },
    { "Browse…", "Gözat…" },
    { "Preferences", "Tercihler" },
    { "Interface Language", "Arayüz Dili" },
    { "System Default", "Sistem Varsayılanı" },
    { "Font Family:", "Yazı Tipi:" },
    { "Font Size:", "Yazı Boyutu:" },
    { "Tab Width:", "Sekme Genişliği:" },
    { "Insert spaces instead of tabs", "Sekmeler yerine boşluk ekle" },
    { "Show line numbers", "Satır numaralarını göster" },
    { "Highlight current line", "Geçerli satırı vurgula" },
    { "Word wrap", "Sözcük kaydır" },
    { "Auto indentation", "Otomatik girintileme" },
    { "Highlight matching brackets", "Eşleşen parantezleri vurgula" },
    { "MRT Executable Path:", "MRT Çalıştırılabilir Yolu:" },
    { "Line Number:", "Satır Numarası:" },
    { "Go", "Git" },
    { "File not found", "Dosya bulunamadı" },
    { "No matches found", "Eşleşme bulunamadı" },

    { NULL, NULL }
};

static void init_hash_table_if_needed(void) {
    if (tr_table) return;
    tr_table = g_hash_table_new(g_str_hash, g_str_equal);
    for (int i = 0; tr_dictionary[i].msgid != NULL; i++) {
        g_hash_table_insert(tr_table, (gpointer)tr_dictionary[i].msgid, (gpointer)tr_dictionary[i].msgstr);
    }
}

void mrt_i18n_init(const char *lang_code) {
    init_hash_table_if_needed();

    const char *target = lang_code;
    if (!target || strcmp(target, "system") == 0 || strlen(target) == 0) {
        const char *sys_lang = getenv("LC_ALL");
        if (!sys_lang || strlen(sys_lang) == 0) sys_lang = getenv("LC_MESSAGES");
        if (!sys_lang || strlen(sys_lang) == 0) sys_lang = getenv("LANG");

        if (sys_lang && (g_str_has_prefix(sys_lang, "tr") || g_str_has_prefix(sys_lang, "TR"))) {
            target = "tr";
        } else {
            target = "en";
        }
    }

    if (g_str_has_prefix(target, "tr")) {
        g_strlcpy(current_lang, "tr", sizeof(current_lang));
        setlocale(LC_ALL, "tr_TR.UTF-8");
        g_setenv("LANGUAGE", "tr", TRUE);
    } else {
        g_strlcpy(current_lang, "en", sizeof(current_lang));
        setlocale(LC_ALL, "en_US.UTF-8");
        g_setenv("LANGUAGE", "en", TRUE);
    }

    bindtextdomain("mrt-studio", "locale");
    bindtextdomain("mrt-studio", "mrt-studio/locale");
    bindtextdomain("mrt-studio", "/usr/local/share/locale");
    bind_textdomain_codeset("mrt-studio", "UTF-8");
    textdomain("mrt-studio");
}

const char *mrt_i18n_get_current_language(void) {
    return current_lang;
}

const char *mrt_gettext(const char *msgid) {
    if (!msgid) return "";

    /* If English is requested, return original string */
    if (strcmp(current_lang, "en") == 0) {
        return msgid;
    }

    /* First check gettext */
    const char *translated = gettext(msgid);
    if (translated && translated != msgid && strcmp(translated, msgid) != 0) {
        return translated;
    }

    /* Fallback to embedded translation dictionary */
    if (tr_table) {
        const char *val = (const char *)g_hash_table_lookup(tr_table, msgid);
        if (val) return val;
    }

    return msgid;
}

void mrt_i18n_cleanup(void) {
    if (tr_table) {
        g_hash_table_destroy(tr_table);
        tr_table = NULL;
    }
}
