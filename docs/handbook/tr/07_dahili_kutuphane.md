# Bölüm 7 — Dahili Kütüphane (Built-in Functions)

MRT standart çalışma zamanı ortamında hazır bulunan dahili fonksiyonlar aşağıda listelenmiştir:

| Fonksiyon | Parametreler | Açıklama |
|---|---|---|
| `read([mesaj])` | `mesaj` (isteğe bağlı string) | Kullanıcıdan konsoldan girdi alır. Girdi metin (`string`) olarak döner. |
| `number(deger)` | `deger` | Metin veya mantıksal değeri tam sayıya (`int`) veya ondalığa (`float`) dönüştürür. `toNumber` takma adı da geçerlidir. |
| `toText(deger)` | `deger` | Herhangi bir MRT değerini metin (`string`) gösterimine çevirir. |
| `typeOf(deger)` | `deger` | Değerin tip adını metin olarak döndürür (`"int"`, `"float"`, `"string"`, `"bool"`, `"array"`, `"map"`, `"function"`, `"none"`). |
| `length(koleksiyon)`| `string`, `array` veya `map` | Metnin karakter sayısını, dizinin veya haritanın eleman sayısını döndürür. |
| `append(dizi, eleman)`| `dizi`, `eleman` | Dizinin sonuna yeni bir eleman ekler. |
| `remove(koleksiyon, k)` | `array` + `indeks` veya `map` + `anahtar` | Belirtilen indeksteki veya anahtardaki elemanı siler ve döndürür. |
| `contains(kol, o)` | `kol` (dizi, harita veya metin), `aranan` | Elemanın var olup olmadığını (`yes` / `no`) denetler. |
| `keys(harita)` | `map` | Haritanın tüm anahtarlarını metin dizisi olarak döndürür. |
| `values(harita)` | `map` | Haritanın tüm değerlerini dizi olarak döndürür. |
| `range(bas, son[, adim])` | `int`, `int`[, `int`] | Belirtilen aralıkta tam sayılardan oluşan bir dizi üretir. |
| `assert(kosul[, mesaj])` | `kosul`, `mesaj` | Koşul sağlanmıyorsa çalışma zamanı hatası fırlatır. Testler için idealdir. |
| `clock()` | - | Program başlangıcından itibaren geçen süreyi saniye cinsinden kayan noktalı sayı olarak döndürür. |

## Örnek Kullanımlar

```mrt
// Kullanıcı girdisi ve tip dönüşümü
var girdi = read("Lütfen yaşınızı girin: ")
var yas = number(girdi)
assert(yas > 0, "Yaş sıfırdan büyük olmalıdır!")

// Koleksiyon işlemleri
var liste = range(1, 5) // [1, 2, 3, 4]
append(liste, 10)
say "Mevcut liste: " + toText(liste)
say "10 var mı: " + toText(contains(liste, 10))
```
