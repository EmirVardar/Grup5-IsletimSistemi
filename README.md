# Grup 5 - İşletim Sistemi Projesi

## Proje Tanımı

Bu proje, Linux işletim sisteminde temel bir Kabuk (Shell) uygulaması geliştirmeye odaklanmaktadır. Kabuk uygulaması, komut istemi (prompt), komut yürütme, giriş ve çıkış yönlendirme, borulama (pipe) ve arka plan işlemleri gibi işlevleri destekler. 

Projenin amacı, işletim sistemi temel kavramlarını anlamak ve uygulayarak proses yönetimi, I/O yönlendirme ve sinyal kullanımı konularında pratik yapmaktır.

---

## Proje Özellikleri

- Komut istemi (prompt): Kullanıcıdan gelen komutları alır.
- Dahili komutlar: Örneğin, `quit` komutu kabuğu sonlandırır.
- Tekli komut yürütme: Standart Linux komutları veya yürütülebilir dosyalar çalıştırılabilir.
- Giriş ve çıkış yönlendirme: `>` ve `<` operatörleri kullanılarak giriş ve çıkışlar yönlendirilebilir.
- Borulama (pipe): `|` sembolü ile bir komutun çıktısı diğer komutun girdisine bağlanabilir.
- Arka plan işlemleri: `&` sembolü ile işlemler arka planda yürütülebilir.

---

## Proje Dosya Yapısı

```plaintext
Grup5-IsletimSistemi-main/
├── myshell.c         # Kabuk uygulamasının ana kod dosyası
├── myshell.h         # Kabuk uygulamasına ait başlık dosyası
├── makefile          # Projenin derlenmesi için kullanılan dosya
├── shell             # Derlenmiş kabuk uygulaması
├── cat, ls, sleep    # Yardımcı yürütülebilir dosyalar
├── input.txt         # Test için giriş dosyası
├── output.txt        # Test için çıkış dosyası
├── test.txt          # Test verisi dosyası
└── testfile.txt      # Ek test verisi dosyası
```

---

## Nasıl Kurulur ve Çalıştırılır?

### Adımlar:

1. **Depoyu Klonlayın:**
   ```bash
   git clone https://github.com/EmirVardar/Grup5-IsletimSistemi
   cd Grup5-IsletimSistemi-main
   ```

2. **Projeyi Derleyin:**
   ```bash
   make
   ```
   Bu komut, `myshell.c` dosyasını derler ve çalıştırılabilir bir `shell` dosyası oluşturur.

3. **Kabuk Uygulamasını Çalıştırın:**
   ```bash
   ./shell
   ```

---

## Kullanım Kılavuzu

### Komut İstemi (Prompt)
Kabuk, her komut sonrasında veya bir arka plan işlemi başlatıldıktan hemen sonra `>` sembolünü gösterir.

### Dahili Komutlar
- **`quit`:** Kabuktan çıkış yapar. Eğer arka planda çalışan işlemler varsa, kabuk çıkmadan önce işlemlerin tamamlanmasını bekler.

### Tekli Komutlar
Standart Linux komutlarını veya yürütülebilir dosyaları çalıştırabilirsiniz:
```bash
> ls -l
```

### Giriş ve Çıkış Yönlendirme
- **Giriş Yönlendirme:**
  ```bash
  > cat < input.txt
  ```
- **Çıkış Yönlendirme:**
  ```bash
  > ls -l > output.txt
  ```

### Borulama (Pipe)
Birden fazla komutun çıktısı ve girdisi zincirlenebilir:
```bash
> echo 12 | ./increment
```

### Arka Plan İşlemleri
Bir komutu arka planda çalıştırabilirsiniz:
```bash
> sleep 5 &
```
Arka plan işlemi tamamlandığında, işlem bilgileri şu şekilde görüntülenir:
```plaintext
[pid] retval: <exitcode>
```

---

## Katkıda Bulunma

1. Depoyu fork’layın.
2. Yeni bir branch oluşturun:
   ```bash
   git checkout -b feature/yeni-ozellik
   ```
3. Değişikliklerinizi commit edin:
   ```bash
   git commit -m "Yeni bir özellik eklendi."
   ```
4. Branch’inizi gönderin:
   ```bash
   git push origin feature/yeni-ozellik
   ```
5. Bir Pull Request açın.

---

