# Bölüm 1 — MRT Programlama Diline Giriş

## MRT Nedir?

**MRT**, sıfırdan C11 standart kütüphanesi kullanılarak geliştirilmiş, hafif, modern, okunabilir ve genişletilebilir bir genel amaçlı programlama dilidir. Harici bir parser üreteci (Lex/Yacc/Bison/Flex) veya üçüncü parti bağımlılık barındırmaz.

Sürüm **0.2.0** ile MRT, hem dil kimliğini güçlendirmiş hem de zengin koleksiyon yapıları (Diziler, Haritalar), modül sistemi (`use`), gelişmiş denetleme araçları (`mrt check`, `mrt fmt`, `mrt inspect`) ve saf GTK4 tabanlı **MRT Studio 0.2** geliştirme ortamı ile bütüncül bir ekosisteme kavuşmuştur.

## Temel Tasarım İlkeleri

1. **Özgün ve Anlaşılır Sözdizimi**: Diğer dillerin sıradan bir taklidi olmak yerine (`var`, `task`, `give`, `when`, `otherwise`, `repeat`, `each ... in ...`, `say`) kelimeleriyle kendi dil kimliğini sunar.
2. **Hafif ve Bağımsız**: Yalnızca C derleyicisi (GCC veya Clang) ile standart C kütüphanesi gerektirir.
3. **Bellek Güvenliği**: Referans sayımlı dinamik bellek yönetimiyle AddressSanitizer ve UndefinedBehaviorSanitizer kontrollerinden tam puan alır.
4. **Çift Yönlü CLI Araç Zinciri**: Kod çalıştırma, sözdizimi doğrulama (`check`), biçimlendirme (`fmt`) ve sembol analizi (`inspect`) araçları komut satırında ve IDE içinde aynı tutarlılıkla çalışır.
5. **Linux Öncelikli Ekosistem**: Modern Linux masaüstü ortamlarına sorunsuz entegre olur.
