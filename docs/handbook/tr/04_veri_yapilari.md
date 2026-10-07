# Bölüm 4 — Koleksiyonlar ve Veri Yapıları

MRT 0.2.0, iki temel birinci sınıf koleksiyon yapısı sunar: **Diziler (Arrays)** ve **Haritalar (Maps)**.

## 1. Diziler (Arrays)

Diziler, köşeli parantez `[...]` içinde tanımlanan, sıfır tabanlı indekslenen dinamik listelerdir.

```mrt
var liste = [10, 20, 30]
var bos_liste = []
```

### Elemana Erişim ve Değiştirme
```mrt
var ilk = liste[0]      // 10
liste[1] = 99           // İkinci elemanı güncelle
say liste               // [10, 99, 30]
```

### Dizi İşlemleri
- `append(dizi, eleman)`: Dizi sonuna eleman ekler.
- `remove(dizi, indeks)`: Belirtilen indeksteki elemanı diziden çıkarır ve döndürür.
- `contains(dizi, eleman)`: Elemanın dizide bulunup bulunmadığını (`yes`/`no`) döndürür.
- `length(dizi)`: Dizideki eleman sayısını verir.
- `+`: İki diziyi birleştirir (`[1, 2] + [3, 4]`).

---

## 2. Haritalar (Maps)

Haritalar, süslü parantez `{ ... }` içerisinde `anahtar: deger` çiftleri şeklinde tanımlanan anahtar-değer yapılarıdır. Anahtarlar metin (`string`) tipindedir.

```mrt
var kullanici = {
    "ad": "Murat",
    "unvan": "Mimar",
    "puan": 100
}
```

### Harita Erişimi ve Güncelleme
```mrt
say kullanici["ad"]        // Murat
kullanici["unvan"] = "Kıdemli Mimar"
kullanici["sehir"] = "Ankara"
```

### Harita İşlemleri
- `keys(harita)`: Haritadaki tüm anahtarları dizi olarak döndürür.
- `values(harita)`: Haritadaki tüm değerleri dizi olarak döndürür.
- `contains(harita, anahtar)`: Anahtarın haritada mevcut olup olmadığını sorgular.
- `remove(harita, anahtar)`: Anahtarı ve değerini siler, değeri döndürür.
- `length(harita)`: Haritadaki kayıt sayısını verir.
