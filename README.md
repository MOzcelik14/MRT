<p align="center">
  <img src="assets/mrt-logo.svg" alt="MRT Programming Language" width="380">
</p>

<p align="center">
  <strong>MRT Programming Language & MRT Studio IDE</strong><br>
  A modern, lightweight programming language and native Linux IDE crafted from scratch in pure C.
</p>

<p align="center">
  <a href="#english">English</a> • <a href="#türkçe">Türkçe</a> • <a href="MRT_El_Kitabi.pdf"><strong>📄 PDF El Kitabı / Handbook</strong></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MRT-v0.2.0-blue.svg" alt="MRT Version">
  <img src="https://img.shields.io/badge/MRT%20Studio-v0.2.0-orange.svg" alt="MRT Studio Version">
  <img src="https://img.shields.io/badge/Docs-PDF%20Handbook-purple.svg" alt="PDF Handbook">
  <img src="https://img.shields.io/badge/Language-C11%20%2F%20C17-00599C.svg" alt="Language C11/C17">
  <img src="https://img.shields.io/badge/GUI-GTK4%20%2B%20GtkSourceView%205-4B8BBE.svg" alt="GTK4">
  <img src="https://img.shields.io/badge/Tests-14%2F14%20Passing-brightgreen.svg" alt="Tests Passing">
  <img src="https://img.shields.io/badge/Memory-0%20Leaks%20(ASan%2FLSan)-success.svg" alt="Memory Safe">
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
mrt main.mrt
```

---

## MRT 0.2.0 Language Identity

MRT features its own distinctive, clean syntax:

* **Variable Bindings:** `var name = "Murat"`
* **Tasks (Functions):** `task add(a, b) { give a + b }`
* **Return Values:** `give result`
* **Output Statement:** `say "Hello, MRT!"` (`say <expr>` keyword statement, no parentheses required)
* **Conditional Logic:** `when (condition) { ... } otherwise when (...) { ... } otherwise { ... }`
* **Loops & Control Flow:** `repeat (counter < 10) { ... }`, with `break` and `continue`
* **Literals:** `yes` (true), `no` (false), `none` (null / void)
* **Comments:** `// single-line` and `/* multi-line block */`
* **Standard Builtins:** `typeOf(v)`, `length(s)`, `toText(v)`, `clock()`

### Example Code

```mrt
// fibonacci.mrt - MRT 0.2.0
task fib(n) {
    when n <= 1 {
        give n
    }
    give fib(n - 1) + fib(n - 2)
}

var i = 0
repeat i <= 10 {
    say "fib(" + toText(i) + ") = " + toText(fib(i))
    i = i + 1
}
```

---

## MRT Studio IDE

<p align="center">
  <img src="assets/mrt-studio-logo.svg" alt="MRT Studio Logo" width="340">
</p>

**MRT Studio** is a tailored development environment for MRT:

* **Bilingual Localization (TR & EN):** Native GNU gettext binding with an embedded dictionary fallback—ensures seamless Turkish and English translation even if system locales are missing.
* **Branded Welcome Screen:** Clean project dashboard with quick actions (New Project, Open Folder, Open File) and recent projects history.
* **Productivity Tools:**
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
* **Memory Safety**: Reference-counted runtime with AddressSanitizer (ASan) and LeakSanitizer (LSan) validation (0 leaks).

---

## Building and Running

### Prerequisites
* GCC (11+) or Clang (13+)
* GNU Make
* GTK4 & GtkSourceView 5 (bundled under `mrt-studio/deps` for out-of-the-box building)

### Build Targets

```bash
# Compile MRT language CLI
make

# Run the automated test suite (AddressSanitizer enabled)
make test

# Compile MRT Studio IDE
make studio

# Launch MRT Studio
./mrt-studio/mrt-studio

# Run an MRT script directly
./mrt examples/factorial.mrt
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

## Handbook & Documentation

A comprehensive, 15-page publication-ready PDF reference handbook is available:
* 📄 **[MRT_El_Kitabi.pdf](MRT_El_Kitabi.pdf)** (Complete language and IDE manual)

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
mrt ana.mrt
```

---

## MRT 0.2.0 Dil Kimliği

MRT 0.2.0 sürümü ile birlikte dile özgün ve akıcı bir kimlik kazandırılmıştır:

* **Değişken Tanımlama:** `var isim = "Murat"`
* **Görevler (Fonksiyonlar):** `task topla(a, b) { give a + b }`
* **Değer Döndürme:** `give sonuc`
* **Konsol Çıktısı:** `say "Merhaba Dünya!"` (`say <ifade>` deyimi, parantez zorunluluğu yoktur)
* **Koşul İfadeleri:** `when (durum) { ... } otherwise when (...) { ... } otherwise { ... }`
* **Döngüler:** `repeat (sayac < 10) { ... }`, `break` ve `continue` destekli
* **Değer Sabitleri:** `yes` (doğru), `no` (yanlış), `none` (boş/yokluk)
* **Yorum Satırları:** `// tek satır` ve `/* çok satırlı blok */`
* **Yerleşik İşlevler:** `typeOf(d)`, `length(metin)`, `toText(d)`, `clock()`

### Örnek Kod

```mrt
// faktoriyel.mrt - MRT 0.2.0
task faktoriyel(n) {
    when n <= 1 {
        give 1
    }
    give n * faktoriyel(n - 1)
}

var sayi = 5
say toText(sayi) + "! = " + toText(faktoriyel(sayi))
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

## Mimari

MRT, modüler ve genişletilebilir bir mimariye sahiptir:

```text
MRT Kaynak Kodu (.mrt)
          ↓
     [ Lexer ]          --> Karakter tarama ve token üretimi (src/token.c, src/lexer.c)
          ↓ Tokenlar
     [ Parser ]         --> Recursive-descent & öncelik tırmanışı (src/parser.c)
          ↓ AST          <-- Frontend Sınırı
  [ AST Yorumlayıcı ]   --> Kapsam zinciri ve dinamik değer sistemi (src/interpreter.c)
          ↓
        Çıktı
```

* **Ayrık Mimari**: AST yapısı çalışma zamanından bağımsızdır; gelecekteki Bytecode VM ve C derleme hedeflerine doğrudan bağlanabilir.
* **Bellek Güvenliği**: Referans sayımı ve AddressSanitizer/LeakSanitizer testleri ile **0 bellek sızıntısı**.

---

## Derleme ve Çalıştırma

### Gereksinimler
* GCC (11+) veya Clang (13+)
* GNU Make
* GTK4 & GtkSourceView 5 (`mrt-studio/deps` altında paketlenmiş olarak hazırdır)

### Derleme Komutları

```bash
# MRT dil yorumlayıcısını derle
make

# Otomatik test paketini çalıştır (AddressSanitizer devrede)
make test

# MRT Studio IDE'sini derle
make studio

# MRT Studio'yu çalıştır
./mrt-studio/mrt-studio

# Bir MRT dosyasını komut satırından çalıştır
./mrt examples/conditions.mrt
```

### Sisteme Kurulum

```bash
# Kullanıcı düzeyinde (~/.local) kurulum (root gerekmez)
make install PREFIX=~/.local

# Veya sistem geneline kurulum
sudo make install PREFIX=/usr/local
```

`mrt` ve `mrt-studio` ikili dosyalarını, masaüstü kısayolunu (`mrt-studio.desktop`), dosya türü tanımını (`mrt.xml`), GtkSourceView sözdizimi renklendirmesini (`mrt.lang`), AppStream üstverisini ve simgeleri kurar.

---

## El Kitabı (PDF Handbook)

MRT dilinin tüm ayrıntılarını, sözdizimini, standart kütüphanesini ve MRT Studio kullanım kılavuzunu içeren 15 sayfalık yayın kalitesinde PDF el kitabı hazırlanmıştır:
* 📄 **[MRT_El_Kitabi.pdf](MRT_El_Kitabi.pdf)** — Tam Kılavuz ve Referans Belgesi

---

## Lisans

Bu proje [MIT Lisansı](LICENSE) ile lisanslanmıştır.
