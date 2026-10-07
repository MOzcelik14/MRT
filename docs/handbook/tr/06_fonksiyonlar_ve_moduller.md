# Bölüm 6 — Fonksiyonlar ve Modül Sistemi

## Fonksiyon Bildirimi (`task`)

MRT'de fonksiyonlar `task` anahtar sözcüğü ile tanımlanır. Fonksiyonlar birinci sınıf nesnelerdir (değişkenlere atanabilir veya parametre olarak geçirilebilir).

```mrt
task topla(a, b) {
    give a + b
}

var sonuc = topla(15, 25)
say "Sonuç: " + toText(sonuc)
```

## Değer Döndürme (`give`)

Bir fonksiyondan değer döndürmek için `give` kullanılır. `give` ifadesi çalıştırıldığı anda fonksiyon icrasını bitirir ve değeri çağırana iletir:

```mrt
task mutlak_deger(x) {
    when x < 0 {
        give -x
    }
    give x
}
```

Eğer bir fonksiyonda `give` çalıştırılmadan blok sonuna ulaşılırsa, örtük olarak `none` döner.

---

## Modül Sistemi (`use`)

MRT 0.2.0, kodlarınızı dosyalara bölmenize ve tekrar kullanmanıza olanak tanıyan temiz bir modül sistemi sunar.

### Modül Tanımlama (`matematik.mrt`)
```mrt
// matematik.mrt
task carp(a, b) {
    give a * b
}

task kare(x) {
    give x * x
}

var pi = 3.14159
```

### Modülü İçe Aktarma (`main.mrt`)
```mrt
// main.mrt
use "matematik.mrt"

say "Pi sayısı: " + toText(pi)
say "Kare: " + toText(kare(6))
say "Çarpım: " + toText(carp(5, 8))
```

### Modül Özellikleri
- **Bağıl Yol Çözümleme**: `use` yolları o an çalışan dosyanın bulunduğu dizine göre göreceli olarak çözülür.
- **Tekil Yükleme (Caching)**: Aynı dosya birden fazla kez `use` edilse bile yalnızca bir defa yorumlanır.
- **Döngüsel İçe Aktarım Koruması (Circular Dependency Prevention)**: A dosyası B'yi, B dosyası A'yı içe aktarmaya çalışırsa MRT bunu runtime hatası olarak algılar ve sonsuz döngüyü engeller.
