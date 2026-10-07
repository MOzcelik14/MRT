# Bölüm 5 — Kontrol Akışı

MRT, sade ve okunaklı kontrol akış yapıları sunar.

## Koşul İfadeleri (`when` / `otherwise`)

MRT'de koşullu dallanma `when` ve `otherwise` anahtar sözcükleriyle gerçekleştirilir:

```mrt
var puan = 85

when puan >= 90 {
    say "Not: AA"
} otherwise when puan >= 80 {
    say "Not: BA"
} otherwise {
    say "Not: CC"
}
```

## Döngüler

### 1. Koşullu Döngü (`repeat`)
Belirli bir koşul doğru (`yes`) olduğu sürece bloğu tekrarlar:

```mrt
var sayac = 5
repeat sayac > 0 {
    say "Geri sayım: " + toText(sayac)
    sayac = sayac - 1
}
```

### 2. Koleksiyon Döngüsü (`each ... in ...`)
Diziler, haritalar, aralıklar ve metinler üzerinde yineleme yapmak için kullanılır:

```mrt
// Dizi üzerinde dönme
each eleman in ["elma", "armut", "muz"] {
    say "Meyve: " + eleman
}

// Aralık üzerinde dönme
each i in range(1, 10) {
    say "Adım: " + toText(i)
}

// Harita anahtarları üzerinde dönme
var profil = {"ad": "Ali", "rol": "Geliştirici"}
each anahtar in keys(profil) {
    say anahtar + ": " + toText(profil[anahtar])
}
```

## Döngü Denetimleri (`break` / `continue`)

- `break`: Döngüyü anında sonlandırır.
- `continue`: Döngünün bir sonraki yinelemesine geçer.

```mrt
each sayi in range(1, 20) {
    when sayi == 5 {
        continue // 5'i atla
    }
    when sayi == 10 {
        break    // 10'da döngüyü bitir
    }
    say sayi
}
```
