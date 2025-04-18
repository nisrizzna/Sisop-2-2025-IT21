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



# Soal_4
