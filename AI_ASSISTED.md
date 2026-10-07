# MRT AI-Assisted Development Disclosure / Yapay Zekâ Destekli Geliştirme Bildirimi

[English](#english) | [Türkçe](#türkçe)

---

## English

### Transparency Disclosure

The **MRT Programming Language** and **MRT Studio** ecosystem was developed using an AI-assisted engineering methodology, pairing human software architecture and direction with modern generative AI coding assistants (Google Antigravity & DeepMind Advanced Agentic Coding).

### Roles and Responsibilities

- **Human Architect & Lead Developer (Murat Özçelik)**:
  - Language identity, syntax design, and feature roadmapping.
  - Architecture decisions: pure C11 standard library, handcrafted lexer/parser, AST-based interpreter, call frame design.
  - Native GTK4 and GtkSourceView 5 decisions for MRT Studio without Electron or JavaScript runtimes.
  - Rigorous code review, memory safety validation via AddressSanitizer and UndefinedBehaviorSanitizer, and final release gating.
  - System integration, package dependency curation, and Linux desktop environment verification.

- **AI Coding Assistant (Antigravity)**:
  - Rapid implementation of handcrafted recursive-descent parser routines and AST nodes.
  - Data structure implementation (ref-counted arrays, hash maps, environments).
  - Built-in function library implementation and test suite authoring.
  - Dual build system configuration (Makefile, Meson + Ninja) and CI workflow automation.
  - Generating comprehensive documentation (Handbook 2.0, PDF reference, Markdown specs).

### Quality Assurance & Verification

Every component generated or refactored with AI assistance has been subjected to:
1. Strict compilation under `-std=c11 -Wall -Wextra -Wpedantic` with zero warnings.
2. Comprehensive unit test suites covering lexing, parsing, runtime semantics, and edge cases.
3. Memory safety inspection using `-fsanitize=address,undefined` to guarantee zero memory leaks or invalid pointer accesses.
4. Native Linux desktop testing for the MRT Studio GTK4 GUI.

---

## Türkçe

### Şeffaflık ve Geliştirme Bildirimi

**MRT Programlama Dili** ve **MRT Studio** ekosistemi, insan yazılım mimarisi ve vizyonu ile modern üretken yapay zekâ kodlama asistanlarının (Google Antigravity & DeepMind Advanced Agentic Coding) iş birliğiyle geliştirilmiştir.

### Roller ve Sorumluluklar

- **İnsan Mimar ve Baş Geliştirici (Murat Özçelik)**:
  - Dil kimliği, sözdizimi kararları ve sürüm yol haritası.
  - Mimari kararlar: saf C11 standart kütüphanesi, harici üreteç içermeyen el yapımı sözcük/ayrıştırıcı, AST yorumlayıcı, çağrı yığını tasarımı.
  - MRT Studio için Electron veya web katmanı yerine saf GTK4 ve GtkSourceView 5 tercihi.
  - Titiz kod denetimi, AddressSanitizer ve UndefinedBehaviorSanitizer ile bellek güvenliği doğrulaması.
  - Sistem entegrasyonu, Linux masaüstü uyumluluğu ve sürüm onaylama.

- **Yapay Zekâ Asistanı (Antigravity)**:
  - El yapımı recursive-descent ayrıştırıcı yordamlarının ve AST düğümlerinin kodlanması.
  - Referans sayımlı veri yapıları (dinamik dizi, hash harita, ortam kapsamları) implementasyonu.
  - Dahili fonksiyon kütüphanesinin ve birim test senaryolarının hazırlanması.
  - İkili derleme sistemi (Makefile, Meson + Ninja) ve GitHub Actions CI konfigürasyonu.
  - Kapsamlı el kitabı (Handbook 2.0, PDF kılavuz, dokümantasyon) üretimi.

### Kalite Güvencesi ve Doğrulama

Yapay zekâ desteğiyle yazılan tüm kodlar şu süreçlerden geçirilmiştir:
1. `-std=c11 -Wall -Wextra -Wpedantic` bayraklarıyla sıfır uyarı veren temiz derleme.
2. Sözdizim, çalışma zamanı ve sınır durumları kapsayan kapsamlı test paketi.
3. Bellek sızıntılarını ve geçersiz işaretçi erişimlerini engellemek için `-fsanitize=address,undefined` doğrulaması.
4. MRT Studio GTK4 arayüzünün Linux masaüstünde gerçek ortam doğrulaması.
