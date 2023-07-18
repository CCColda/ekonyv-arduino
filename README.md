# EKönyv

Egy Arduino MKRZERO-n és ETH shield-en futó szerver, ami könyvekről tárol
adatokat és ISBN kódokat kérdez le; egy Android alkalmazás backend-je.

## Kábelezés

![](https://docs.arduino.cc/static/e111efcef9f69d012577319bcce2f67c/4ef49/MKRETH_T1_IMG02.png)

![](https://docs.arduino.cc/static/afbe069c7e55e2a6b795727f75a03252/4ef49/MKRETH_T1_IMG03.png)

## Roadmap

- [x] Csatlakozás DHCP-re
- [x] HTTP szerver és útelválasztás
- [x] SD-kártya I/O
- [x] Event loop (~~fizikai → LCD frissítés~~ (később) → DHCP → bejövő →
      eventek)
- [x] Adatok veszteségmentes _?_ tárolása az SD-kártyán
  - [x] Fájlkezelés
- [x] Regisztráció négyjegyű kóddal
- [x] Idő lekérdezése NTP szerverről
- [x] Felhasználók jelszavainak SHA-256 titkosítása
- [x] Bejelentkezés session token-ért
- [x] Session token ellenőrzése védett API végpontokon
- [ ] ~~ISBN-lekérdezés API végpont~~
  - [ ] ~~Csak felhasználókra szűkítve~~
  - [ ] ~~Lekérdezés (3-4 HTTP GET)~~
  - [ ] ~~Ideiglenes tárolás~~
  - [ ] ~~Adat visszaküldése HTTP-n~~
- [ ] Könyvtárolás API végpont (+ permission-ök)
- [ ] Lekérdezés API végpont
- [ ] Törlés, szerkesztés végpont
- [ ] LCD UI; alapvető adatok kiírása, 2-3 bemenet
  - [ ] IP-cím kiírása az LCD-re
  - [ ] API-kulcs a felhasználóknak LCD-s bejelentkeztetéssel
