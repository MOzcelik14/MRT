# Bölüm 8 — MRT Studio IDE

## Genel Bakış

**MRT Studio**, MRT programlama dili için özel olarak tasarlanmış, hafif, yerel (native) ve modern bir tümleşik geliştirme ortamıdır (IDE).

- **Teknolojiler**: Saf C, GTK4, GtkSourceView 5, GLib/GIO.
- **Hafif ve Hızlı**: Electron, WebView veya JavaScript çalışma zamanı barındırmaz. Anında açılır ve minimum bellek tüketir.
- **İki Dilli (Bilingual)**: Türkçe ve İngilizce dillerini tam destekler.

---

## Temel Özellikler

### 1. Kod Düzenleyici
- MRT 0.2 sözdizimi vurgulama (`data/mrt.lang`).
- Satır numaraları, geçerli satırı vurgulama, parantez eşleştirme.
- Otomatik tamamlama (MRT anahtar kelimeleri ve dahili fonksiyonlar).
- Kod biçimlendirme (`mrt fmt` entegrasyonu, `Shift+Alt+F`).

### 2. Proje Gezgini ve Sembol Ağacı (Outline)
- Sol panelde proje dosyaları ağacı (`Files`).
- MRT dosyalarındaki fonksiyonları ve değişkenleri hiyerarşik gösteren `Outline` paneli.

### 3. Çalıştırma ve Çıktı Paneli
- `F5` tuşu veya Araç Çubuğundaki **Run** butonu ile aktif dosyayı tek tıkla çalıştırma.
- Çıktı panelinde renkli standart çıktı (stdout) ve hata çıktısı (stderr).
- Tıklanabilir hata bağlantıları: Hata satırına ve sütununa doğrudan atlar.
- **Sorunlar (Problems)** paneli: `mrt check` çıktılarını tablo halinde listeler.

### 4. Kısayol Tuşları

| Kısayol | İşlev |
|---|---|
| `Ctrl+N` | Yeni Dosya |
| `Ctrl+O` | Dosya Aç |
| `Ctrl+S` | Kaydet |
| `Ctrl+Shift+S` | Farklı Kaydet |
| `Ctrl+F` | Dosyada Bul |
| `Ctrl+H` | Değiştir |
| `Ctrl+Shift+F` | Dosyalarda Bul |
| `Ctrl+G` | Satıra Git |
| `Ctrl+P` | Hızlı Dosya Aç (Quick Open) |
| `Ctrl+Shift+P` | Komut Paleti |
| `Shift+Alt+F` | Belgeyi Biçimlendir (Format Document) |
| `F5` | Çalıştır |
| `Ctrl+B` | Kenar Çubuğunu Göster/Gizle |
| `Ctrl+J` | Çıktı Panelini Göster/Gizle |
