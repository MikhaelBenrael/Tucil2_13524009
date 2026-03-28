# Tucil2_13524009
Tugas Kecil 2  IF2211 Strategi Algoritma: Voxelization 3D menggunakan Octree dengan Algoritma Divide and Conquer
## Deskripsi Program

Program ini mengimplementasikan voxelization objek 3D menggunakan algoritma Divide and Conquer berbasis struktur data Octree. Program menerima file `.obj` sebagai input dan mengonversinya menjadi model `.obj` baru yang tersusun dari voxel-voxel (kubus-kubus kecil seragam) pada permukaan objek.

## Requirement

- Compiler C (GCC / MinGW)
- Library standar C: `math.h`, `stdio.h`, `stdlib.h`, `string.h`, `time.h`, `stdbool.h`

## Cara Kompilasi

### Linux / macOS
```bash
gcc -O2 -o bin/voxelize src/main.c -lm
```
### Windows (MinGW)
```bash
gcc -O2 -o bin/voxelize.exe src/main.c -lm
```
## Cara Menjalankan

### Linux / macOS
```bash
bin/voxelize <path/ke/file.obj> <kedalaman>
```

### Windows
```bash
bin/voxelize.exe <path/ke/file.obj> <kedalaman>
```

### Contoh
```bash
bin/voxelize test/line.obj 5
```

### Contoh Output CLI
```
Banyaknya voxel yang terbentuk: 287
Banyaknya vertex yang terbentuk: 2296
Banyaknya faces yang terbentuk: 1722

Statistik node octree yang terbentuk:
1 : 8
2 : 24
3 : 56
4 : 216
5 : 672

Statistik node yang tidak perlu ditelusuri:
1 : 5
2 : 17
3 : 29
4 : 132
5 : 385

Kedalaman octree: 5
Lama waktu program berjalan: 0.000 s
Path file .obj disimpan: test/line.obj_hasil.obj
```

File hasil voxelisasi akan disimpan di lokasi yang sama dengan file input, dengan nama `<nama_file>_hasil.obj`.

## Struktur Repository

```
Tucil2_13524009/
├── src/
│   └── main.c
├── bin/
│   └── voxelize.exe
├── test/
│   └── (file .obj uji beserta hasil)
├── doc/
│   └── Laporan_Tucil2_13524009.pdf
└── README.md
```

## Author

| Nama | NIM |
|------|-----|
| Mikhael Benrael Tampubolon | 13524009 |

IF2211 Strategi Algoritma — Semester II 2025/2026  
Sekolah Teknik Elektro dan Informatika  
Institut Teknologi Bandung