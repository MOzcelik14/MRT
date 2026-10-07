# Bölüm 2 — Kurulum ve Komut Satırı Araçları

## Derleme ve Kurulum

MRT, hem `Makefile` hem de `Meson + Ninja` ile derlenebilir.

### Makefile ile Derleme

```bash
# Sadece yorumlayıcıyı derle
make

# Test paketini ASan ile çalıştır
make test

# MRT Studio IDE'sini derle
make studio

# Sisteme kur (~/.local/bin veya PREFIX belirtilerek)
make install PREFIX=/usr/local
```

### Meson ve Ninja ile Derleme

```bash
meson setup build
meson compile -C build
meson test -C build
```

## CLI Alt Komutları

MRT 0.2.0, zengin bir komut satırı arayüzü sunar:

### 1. `mrt run <dosya.mrt>`
Belirtilen betiği çalıştırır. `mrt <dosya.mrt>` yazıldığında da varsayılan olarak çalıştırılır.
```bash
mrt run main.mrt
```

### 2. `mrt check [--diagnostics=json] <dosya.mrt>`
Betiği çalıştırmadan sözdizimi denetimi yapar.
- İnsan dostu çıktı: Hataları kaynak satırı ve ok göstergesiyle ekrana basar.
- JSON modu (`--diagnostics=json`): IDE'ler ve CI botları için yapılandırılmış JSON çıktısı üretir.
```bash
mrt check --diagnostics=json main.mrt
```

### 3. `mrt fmt [--check] <dosya.mrt>`
Kaynak kodu MRT kodlama kurallarına göre biçimlendirir.
- `--check`: Dosyayı değiştirmeden biçiminin uygunluğunu denetler (CI için idealdir).
```bash
mrt fmt main.mrt
mrt fmt --check main.mrt
```

### 4. `mrt inspect --symbols [--json] <dosya.mrt>`
Dosyadaki fonksiyon ve değişken bildirimlerini satır/sütun bilgileriyle listeler.
```bash
mrt inspect --symbols main.mrt
mrt inspect --symbols --json main.mrt
```

### 5. `mrt repl`
Etkileşimli yorumlayıcı kabuğunu başlatır.
```bash
mrt repl
```
