<p align="center">
  <img src="assets/mrt-logo.svg" alt="MRT Programming Language" width="380">
</p>

<p align="center">
  <strong>MRT Programming Language & MRT Studio IDE</strong><br>
  A modern, lightweight programming language and native Linux IDE crafted from scratch in pure C.
</p>

<p align="center">
  <a href="#english">English</a> • 
  <a href="#türkçe">Türkçe</a> • 
  <a href="MRT_El_Kitabi.pdf"><strong>📄 PDF Handbook / El Kitabı</strong></a> • 
  <a href="CHANGELOG.md">Changelog</a> • 
  <a href="AI_ASSISTED.md">AI Disclosure</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MRT-v0.2.0-blue.svg" alt="MRT Version">
  <img src="https://img.shields.io/badge/MRT%20Studio-v0.2.0-orange.svg" alt="MRT Studio Version">
  <img src="https://img.shields.io/badge/Handbook-v2.0-purple.svg" alt="Handbook 2.0">
  <img src="https://img.shields.io/badge/Language-C11%20%2F%20C17-00599C.svg" alt="Language C11/C17">
  <img src="https://img.shields.io/badge/GUI-GTK4%20%2B%20GtkSourceView%205-4B8BBE.svg" alt="GTK4">
  <img src="https://img.shields.io/badge/Tests-21%2F21%20Passing-brightgreen.svg" alt="Tests Passing">
  <img src="https://img.shields.io/badge/Memory-0%20Leaks%20(ASan%2FLSan%2FUBSan)-success.svg" alt="Memory Safe">
  <img src="https://img.shields.io/badge/CI-GitHub%20Actions-brightgreen.svg" alt="CI">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License MIT">
</p>

---

<a name="english"></a>
# English

## Overview

**MRT** is a lightweight, dynamically typed programming language designed and implemented entirely from scratch in standard C (C11/C17). It uses **zero third-party parser generators** (no Flex or Bison) and relies exclusively on standard C library primitives.

Alongside the language runtime, **MRT Studio** is a native, modern, bilingual (English & Turkish) Linux IDE built with **GTK4** and **GtkSourceView 5**. MRT Studio avoids webview/Electron bloat and delivers an ultra-fast, responsive developer experience.

Source files use the **`.mrt`** extension.

```bash
mrt run main.mrt
```

---

## What's New in 0.2.0

* **First-Class Collections:**
  * Dynamic, reference-counted Arrays: `[1, "two", 3.14]` with bracket indexing `arr[0]` and in-place mutation `arr[0] = 42`.
  * Hash Maps with string keys: `{"name": "Murat", "role": "dev"}` with key indexing `map["role"]` and mutation `map["role"] = "lead"`.
* **Collection Iteration (`each ... in ...`):**
  * Seamlessly loops over arrays, maps (yielding keys), or strings with full `break` and `continue` support.
* **Modular Codebase (`use "..."`):**
  * Relative file importing with circular dependency detection and single-load caching.
* **Extended Standard Builtins:**
  * Retained user input: `read([prompt])`.
  * Conversions: `number(v)` (aliased as `toNumber(v)`), `toText(v)`, `typeOf(v)`.
  * Collections: `append(arr, elem)`, `remove(arr, idx)`, `contains(container, elem)`, `keys(map)`, `values(map)`, `range(start, end[, step])`.
  * Multi-type `length(v)`: Works on strings, arrays, and maps.
  * Quality assurance: `assert(condition[, message])` and benchmarking `clock()`.
* **Expanded CLI Suite:**
  * `mrt run <file>` — Execute MRT scripts with optional arguments.
  * `mrt check <file> [--diagnostics=json]` — Syntax and parse verification with structured diagnostic reporting.
  * `mrt fmt <file> [--check]` — AST-based code formatter and CI formatting verification.
  * `mrt inspect <file> --symbols [--json]` — Symbol table extraction for IDEs and static analyzers.
  * `mrt repl` — Interactive Read-Eval-Print Loop.
  * `mrt version` & `mrt help` — Version info and help reference.
* **MRT Studio 0.2 Integration:**
  * In-IDE code formatting via `Shift+Alt+F` (invoking `mrt fmt`).
  * Instant syntax check via `Ctrl+Shift+I` (invoking `mrt check`).
  * Enhanced `mrt.lang` syntax highlighting and autocompletion for all 0.2.0 keywords and builtins.
* **Dual Build System & CI:**
  * GNU Make and Meson/Ninja build configurations.
  * Automated GitHub Actions CI across GCC and Clang with AddressSanitizer and LeakSanitizer.
  * 21/21 automated tests passing with zero memory leaks.

---

## Language Syntax & Features

MRT provides an expressive, readable syntax:

* **Variable Declarations:** `var name = "Murat"`
* **Tasks (Functions):** `task add(a, b) { give a + b }`
* **Return Statements:** `give result`
* **Output:** `say "Hello, MRT!"` (`say <expr>` keyword statement, no parentheses required)
* **Conditionals:** `when (condition) { ... } otherwise when (...) { ... } otherwise { ... }`
* **Loops:** `repeat (counter < 10) { ... }` and `each item in list { ... }`
* **Literals:** `yes` (true), `no` (false), `none` (null)
* **Comments:** `// single-line` and `/* multi-line block */`
* **Imports:** `use "modules/math.mrt"`

### Example Code

```mrt
// math_demo.mrt - MRT 0.2.0
use "helpers.mrt"

var numbers = [10, 20, 30, 40, 50]
append(numbers, 60)

var user = {
    "name": "Murat",
    "role": "Engineer",
    "active": yes
}

say "User: " + user["name"] + " (" + user["role"] + ")"

// Collection iteration
each num in numbers {
    when num == 30 {
        continue
    }
    say "Number: " + toText(num)
}

// User-defined task
task factorial(n) {
    assert(n >= 0, "n must be non-negative")
    when n <= 1 {
        give 1
    }
    give n * factorial(n - 1)
}

say "5! = " + toText(factorial(5))
```

---

## CLI Reference

```bash
# Run a script
mrt run script.mrt

# Verify syntax with JSON diagnostics
mrt check script.mrt --diagnostics=json

# Format a file in place
mrt fmt script.mrt

# Verify formatting without modifying (CI mode)
mrt fmt --check script.mrt

# Inspect declared tasks, variables, and imports
mrt inspect script.mrt --symbols --json

# Launch interactive REPL
mrt repl
```

---

## MRT Studio IDE

<p align="center">
  <img src="assets/mrt-studio-logo.svg" alt="MRT Studio Logo" width="340">
</p>

**MRT Studio** is a dedicated Linux IDE for MRT development:

* **Bilingual Localization (TR & EN):** Native GNU gettext binding with an embedded dictionary fallback—ensures seamless Turkish and English translation even if system locales are missing.
* **Branded Welcome Screen:** Clean project dashboard with quick actions (New Project, Open Folder, Open File) and recent projects history.
* **Productivity Tools:**
  * **Document Formatting (`Shift+Alt+F`):** In-place AST-based formatting using `mrt fmt`.
  * **Syntax Check (`Ctrl+Shift+I`):** Direct diagnostic verification using `mrt check`.
  * **Command Palette (`Ctrl+Shift+P`):** Searchable access to all IDE actions.
  * **Quick Open (`Ctrl+P`):** Fast file fuzzy navigation across the active project.
  * **Find in Files (`Ctrl+Shift+F`):** Search project sources and jump directly to matches.
  * **Go to Line (`Ctrl+G`):** Direct line navigation dialog.
  * **Symbol Outline:** Sidebar tree inspecting all `task` and `var` symbols in the active buffer.
  * **Problems & Diagnostics Panel:** Automatically extracts runtime and compiler errors with file, line, and column clickable jumps.
  * **Status Bar:** Real-time display of cursor position (`Ln X, Col Y`), indentation settings, file encoding, and MRT version.
  * **Session Persistence & Crash Recovery:** Restores previous workspace tabs; periodically autosaves dirty buffers to `~/.local/state/mrt-studio/recovery/` with crash restoration prompts.
  * **Project Wizard:** Project creation templates (*Console Application*, *Empty Project*, *Library*) with `mrt.project` manifest support.

---

## Architecture

MRT is built with a strictly decoupled pipeline:

```text
MRT Source (.mrt)
        ↓
    [ Lexer ]          --> Hand-written scanner & token stream (src/token.c, src/lexer.c)
        ↓ Tokens
    [ Parser ]         --> Recursive-descent & precedence climbing (src/parser.c)
        ↓ AST          <-- Clean Frontend Boundary
  [ AST Interpreter ]  --> Environment scope hierarchy & tagged union values (src/interpreter.c)
        ↓
     Output
```

* **Frontend**: Independent from execution details; ready for future Bytecode VM and C transpile targets.
* **Containers**: High-performance ref-counted Dynamic Arrays (`src/array.c`) and Hash Maps (`src/map.c`).
* **Memory Safety**: Reference-counted runtime with AddressSanitizer (ASan), LeakSanitizer (LSan), and UBSan validation (0 leaks).

---

## Building and Running

### Prerequisites
* GCC (11+) or Clang (13+)
* GNU Make or Meson & Ninja
* GTK4 & GtkSourceView 5 (bundled under `mrt-studio/deps` for out-of-the-box building)

### Build with GNU Make

```bash
# Compile MRT language CLI
make

# Run the automated test suite (AddressSanitizer enabled)
make test

# Compile MRT Studio IDE
make studio

# Launch MRT Studio
./mrt-studio/mrt-studio
```

### Build with Meson & Ninja

```bash
meson setup build
ninja -C build
meson test -C build
```

### Installation

```bash
# Install to ~/.local (no root required)
make install PREFIX=~/.local

# Or install system-wide
sudo make install PREFIX=/usr/local
```

Installs `mrt` and `mrt-studio` binaries, desktop entry (`mrt-studio.desktop`), MIME type association (`mrt.xml` for `*.mrt`), AppStream metadata, syntax highlighting (`mrt.lang`), and icons.

---

## Documentation & Handbook

* 📄 **[PDF Handbook (MRT_El_Kitabi.pdf)](MRT_El_Kitabi.pdf)** — Complete publication-ready 15-page language and IDE reference.
* 📚 **[Documentation Chapters](docs/handbook/)** — Available in both [Turkish](docs/handbook/tr/) and [English](docs/handbook/en/).
* 🧪 **[Runnable Examples](docs/examples/)** — Verified runnable scripts from hello world to modules and collections.
* 🤖 **[AI Disclosure](AI_ASSISTED.md)** — Transparent declaration of AI-assisted engineering practices.
* 📋 **[Changelog](CHANGELOG.md)** — Full release history.

---

<br>
<hr>
<br>

<a name="türkçe"></a>
# Türkçe

## Genel Bakış

**MRT**, standart C (C11/C17) diliyle hiçbir harici parser oluşturucu (Flex, Bison vb.) kullanılmadan, tamamen sıfırdan geliştirilmiş modern ve dinamik tipli bir programlama dilidir. Yalnızca standart C kütüphanesini kullanır.

Dilin yanı sıra **MRT Studio**, Linux için **GTK4** ve **GtkSourceView 5** teknolojileriyle geliştirilmiş, hafif ve çift dilli (Türkçe & İngilizce) yerel bir tümleşik geliştirme ortamıdır (IDE). Electron veya webview gibi ağır framework'ler içermez; doğrudan yerel ikili kod olarak çalışır.

MRT kaynak dosyaları **`.mrt`** uzantısını kullanır:

```bash
mrt run ana.mrt
```

---

## 0.2.0 Sürümünde Neler Yeni?

* **Birinci Sınıf Koleksiyon Yapıları:**
  * Dinamik, referans sayımlı Diziler: `[1, "iki", 3.14]`, köşeli parantez indeksleme `dizi[0]` ve değer atama `dizi[0] = 42`.
  * Metin anahtarlı Eşlemeler (Map): `{"isim": "Murat", "unvan": "Mühendis"}`, anahtar indeksleme `harita["unvan"]` ve atama `harita["unvan"] = "Yazar"`.
* **Koleksiyon Döngüsü (`each ... in ...`):**
  * Diziler, eşlemeler ve karakter dizileri üzerinde `break` ve `continue` destekli doğrudan yineleme.
* **Modüler Kod Mimarisi (`use "..."`):**
  * Göreli yol çözümlemesi, tek seferlik yükleme önbelleği ve döngüsel bağımlılık kontrolü ile harici dosya içe aktarma.
* **Gelişmiş Yerleşik İşlevler:**
  * Korunan kullanıcı girdisi: `read([mesaj])`.
  * Tip dönüşümleri: `number(d)` (ve `toNumber(d)`), `toText(d)`, `typeOf(d)`.
  * Koleksiyon işlevleri: `append(dizi, oge)`, `remove(dizi, indis)`, `contains(koleksiyon, oge)`, `keys(harita)`, `values(harita)`, `range(bas, son[, adim])`.
  * Çoklu tür `length(d)`: Metin, dizi ve haritaların eleman sayısını döndürür.
  * Doğrulama: `assert(kosul[, hata_mesaji])` ve `clock()`.
* **Genişletilmiş Komut Satırı (CLI):**
  * `mrt run <dosya>` — MRT betiklerini argüman desteğiyle çalıştırma.
  * `mrt check <dosya> [--diagnostics=json]` — Sözdizimi kontrolü ve JSON biçimli hata raporu.
  * `mrt fmt <dosya> [--check]` — AST tabanlı otomatik kod biçimlendirme ve doğrulama.
  * `mrt inspect <dosya> --symbols [--json]` — Sembol tablosu ve içe aktarma analiz aracı.
  * `mrt repl` — Etkileşimli komut satırı ortamı.
  * `mrt version` & `mrt help` — Sürüm ve yardım menüsü.
* **MRT Studio 0.2 IDE Geliştirmeleri:**
  * IDE içerisinden `Shift+Alt+F` ile tek tuşla kod biçimlendirme (`mrt fmt` entegrasyonu).
  * `Ctrl+Shift+I` ile anlık sözdizimi doğrulaması (`mrt check`).
  * 0.2.0 anahtar kelimeleri ve yerleşik işlevleri için güncellenen `mrt.lang` renklendirmesi ve otomatik tamamlama.
* **İkili Derleme Altyapısı ve Sürekli Entegrasyon (CI):**
  * GNU Make ve Meson/Ninja derleme desteği.
  * GCC ve Clang üzerinde AddressSanitizer/LeakSanitizer ile koşan GitHub Actions CI iş akışı.
  * 21/21 başarılı test ve 0 bellek sızıntısı.

---

## Dil Kimliği ve Sözdizimi

MRT, kendine özgü akıcı ve temiz bir sözdizimine sahiptir:

* **Değişken Tanımlama:** `var isim = "Murat"`
* **Görevler (Fonksiyonlar):** `task topla(a, b) { give a + b }`
* **Değer Döndürme:** `give sonuc`
* **Konsol Çıktısı:** `say "Merhaba Dünya!"` (`say <ifade>` deyimi, parantez zorunluluğu yoktur)
* **Koşul İfadeleri:** `when (durum) { ... } otherwise when (...) { ... } otherwise { ... }`
* **Döngüler:** `repeat (sayac < 10) { ... }` ve `each oge in liste { ... }`
* **Değer Sabitleri:** `yes` (doğru), `no` (yanlış), `none` (boş/yokluk)
* **Yorum Satırları:** `// tek satır` ve `/* çok satırlı blok */`
* **Modül Dahil Etme:** `use "moduller/matematik.mrt"`

### Örnek Kod

```mrt
// koleksiyonlar.mrt - MRT 0.2.0
use "yardimci.mrt"

var sayilar = [10, 20, 30, 40]
append(sayilar, 50)

var kullanici = {
    "isim": "Murat",
    "unvan": "Mühendis",
    "aktif": yes
}

say "Kullanıcı: " + kullanici["isim"] + " (" + kullanici["unvan"] + ")"

each sayi in sayilar {
    when sayi == 20 {
        continue
    }
    say "Sayı: " + toText(sayi)
}

task faktoriyel(n) {
    assert(n >= 0, "n negatif olamaz")
    when n <= 1 {
        give 1
    }
    give n * faktoriyel(n - 1)
}

say "5! = " + toText(faktoriyel(5))
```

---

## Komut Satırı Kullanımı

```bash
# Betiği çalıştır
mrt run betik.mrt

# Hata ve sözdizimi kontrolü (JSON çıktılı)
mrt check betik.mrt --diagnostics=json

# Kodu otomatik biçimlendir
mrt fmt betik.mrt

# Biçimlendirmeyi doğrula (CI modu)
mrt fmt --check betik.mrt

# Sembolleri incele (JSON çıktılı)
mrt inspect betik.mrt --symbols --json

# Etkileşimli REPL ortamını başlat
mrt repl
```

---

## MRT Studio IDE

<p align="center">
  <img src="assets/mrt-studio-logo.svg" alt="MRT Studio Logo" width="340">
</p>

MRT için özel olarak geliştirilen **MRT Studio**:

* **Çift Dilli Destek (Türkçe & İngilizce):** GNU gettext altyapısı ve dahili sözlük yedeği ile sistemde Türkçe yerel ayar olmasa bile eksiksiz Türkçe arayüz.
* **Markalı Karşılama Ekranı:** Açılışta ve sekme yokken gösterilen; Yeni Proje, Proje Aç, Dosya Aç butonları ve Son Projeler listesi.
* **Üretkenlik Özellikleri:**
  * **Belge Biçimlendirme (`Shift+Alt+F`):** `mrt fmt` motoruyla kodları anında düzenleme.
  * **Sözdizimi Doğrulama (`Ctrl+Shift+I`):** `mrt check` ile hızlı kod denetimi.
  * **Komut Paleti (`Ctrl+Shift+P`):** Tüm IDE işlevlerine hızlı erişim.
  * **Hızlı Dosya Aç (`Ctrl+P`):** Proje dosyaları arasında anında arama ve açma.
  * **Dosyalarda Bul (`Ctrl+Shift+F`):** Proje genelinde kod arama ve satıra doğrudan atlama.
  * **Satıra Git (`Ctrl+G`):** İstenen satıra navigasyon kutusu.
  * **Sembol Ağacı (Outline):** Dosyadaki `task` ve `var` tanımlarını listeleyen kenar çubuğu.
  * **Sorunlar ve Hata Paneli:** Derleme ve çalışma zamanı hatalarını ayrıştırıp tıklanabilir bağlantılarla hata satırına odaklanma.
  * **Durum Çubuğu:** İmleç konumu (`Ln X, Col Y`), girinti türü, kodlama ve MRT sürümü bilgisi.
  * **Oturum ve Çökme Kurtarma:** Açık sekmeleri hatırlama, 15 saniyede bir otomatik yedekleme ve kurtarma bildirimi.
  * **Yeni Proje Sihirbazı:** *Konsol Uygulaması*, *Boş Proje* ve *Kütüphane* şablonları (`mrt.project` manifesti ile).

---

## Derleme ve Kurulum

### Gereksinimler
* GCC (11+) veya Clang (13+)
* GNU Make veya Meson & Ninja
* GTK4 & GtkSourceView 5 (`mrt-studio/deps` altında paketlenmiş olarak hazırdır)

### Makefile ile Derleme

```bash
# MRT yorumlayıcısını derle
make

# Otomatik test paketini çalıştır (AddressSanitizer aktif)
make test

# MRT Studio IDE'sini derle
make studio

# MRT Studio'yu çalıştır
./mrt-studio/mrt-studio
```

### Meson & Ninja ile Derleme

```bash
meson setup build
ninja -C build
meson test -C build
```

### Sisteme Kurulum

```bash
# Kullanıcı düzeyinde (~/.local) kurulum (root yetkisi gerektirmez)
make install PREFIX=~/.local

# Veya sistem geneline kurulum
sudo make install PREFIX=/usr/local
```

`mrt` ve `mrt-studio` ikili dosyalarını, masaüstü kısayolunu (`mrt-studio.desktop`), dosya türü tanımını (`mrt.xml`), GtkSourceView sözdizimi renklendirmesini (`mrt.lang`), AppStream üstverisini ve simgeleri kurar.

---

## Belgeler ve El Kitabı

* 📄 **[PDF El Kitabı (MRT_El_Kitabi.pdf)](MRT_El_Kitabi.pdf)** — 15 sayfalık tam kapsamlı yayın kalitesinde kılavuz.
* 📚 **[Bölüm Dokümanları](docs/handbook/)** — Hem [Türkçe](docs/handbook/tr/) hem [İngilizce](docs/handbook/en/) ayrıntılı kılavuzlar.
* 🧪 **[Çalıştırılabilir Örnekler](docs/examples/)** — Doğrulanmış ve açıklamalı örnek kodlar.
* 🤖 **[Yapay Zeka Şeffaflık Bildirgesi](AI_ASSISTED.md)** — Geliştirme sürecine dair açık kaynak AI kullanım beyanı.
* 📋 **[Sürüm Notları (Changelog)](CHANGELOG.md)** — Tüm sürüm ve yenilik geçmişi.

---

## Lisans

Bu proje [MIT Lisansı](LICENSE) ile lisanslanmıştır.
