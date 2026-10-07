# Bölüm 3 — Temel Sözdizimi ve Değişkenler

## Yorum Satırları

MRT tek satırlık ve çok satırlı blok yorumları destekler:

```mrt
// Tek satırlık yorum
/*
   Çok satırlı
   blok yorum
*/
```

## Değişken Bildirimi (`var`)

Değişkenler `var` anahtar sözcüğü ile tanımlanır:

```mrt
var isim = "Murat"
var yas = 30
var pi = 3.14159
var aktif = yes
var pasif = no
var bos = none
```

## Temel Veri Tipleri

- **Tam Sayılar (`int`)**: 64-bit işaretli tam sayılar (`42`, `-10`, `0`).
- **Kayan Noktalı Sayılar (`float`)**: 64-bit IEEE-754 sayılar (`3.14`, `-0.05`).
- **Mantıksal Değerler (`bool`)**: `yes` (doğru) ve `no` (yanlış).
- **Metinler (`string`)**: Çift tırnak içerisindeki UTF-8 uyumlu karakter dizileri (`"Merhaba Dünya!"`).
- **Yokluk (`none`)**: Değerin bulunmadığını belirtir (`none`).

## Operatörler

- **Aritmetik**: `+`, `-`, `*`, `/`, `%`
- **Karşılaştırma**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Mantıksal**: `and`, `or`, `not`
- **Metin ve Dizi Birleştirme**: `+` operatörü metinleri ve dizileri uç uca ekler.

```mrt
var tam_ad = "Murat " + "Özçelik"
var sayilar = [1, 2] + [3, 4]
```

## Ekrana Yazdırma (`say`)

MRT'de ekrana çıktı vermek için `say` ifadesi kullanılır. `say` ekrana değeri yazar ve satır sonu ekler:

```mrt
say "Merhaba Dünya!"
say 100 + 250
```
