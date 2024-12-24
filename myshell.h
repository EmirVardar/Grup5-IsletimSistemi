#ifndef MYSHELL_H // Dosya çoklu dahil etmeye karşı korunur
#define MYSHELL_H

// Gerekli kütüphaneler
#include <stdio.h>      // Standart giriş/çıkış işlemleri
#include <stdlib.h>     // Genel amaçlı yardımcı fonksiyonlar (malloc, free, exit vb.)
#include <string.h>     // String işlemleri için fonksiyonlar
#include <unistd.h>     // POSIX standart fonksiyonlar (fork, exec, pipe vb.)
#include <sys/types.h>  // Veri türleri (pid_t vb.)
#include <sys/wait.h>   // Süreç yönetimi (waitpid vb.)
#include <fcntl.h>      // Dosya kontrol işlemleri (open, close vb.)
#include <signal.h>     // Sinyal işlemleri
#include <sys/select.h> // Çoklu giriş/çıkış bekleme (select vb.)

// Makro tanımları
#define MAX_CMD_LEN 1024 // Kullanıcının girdiği komutun maksimum uzunluğu
#define MAX_ARG_LEN 64   // Komut içindeki argüman sayısının maksimum uzunluğu

// Arka plan süreçleri için yapı tanımı
typedef struct {
    pid_t pid;   // Sürecin PID'si (Process ID)
    int active;  // Sürecin aktif olup olmadığını gösterir (1: aktif, 0: tamamlanmış)
} BackgroundProcess;

// Fonksiyon prototipleri

/**
 * @brief Komut istemini ekrana yazdırır.
 * 
 * Kullanıcının komut girebileceği bir ">" simgesini ekrana yazdırır.
 * fflush ile tamponu temizleyerek çıktının hemen görünmesini sağlar.
 */
void print_prompt();

/**
 * @brief Arka plan süreçlerini kontrol eder.
 * 
 * Çalışmakta olan tüm arka plan süreçlerini kontrol eder ve tamamlanmış süreçleri temizler.
 * Tamamlanan süreçlerin çıkış durumunu ekrana yazdırır.
 */
void check_background_processes();

/**
 * @brief Kullanıcıdan alınan komutu ayrıştırır.
 * 
 * @param input Kullanıcının girdiği komut satırı.
 * @param args Komut ve argümanları içeren dizi.
 * @param input_file Yönlendirme için giriş dosyasının adı.
 * @param output_file Yönlendirme için çıkış dosyasının adı.
 * @param is_background Arka plan işlemi bayrağı (1: arka plan, 0: ön plan).
 * 
 * Komut satırını boşluklara göre parçalayarak argümanları ayırır. Giriş ve çıkış
 * yönlendirme dosyalarını tespit eder ve arka plan işlemi işaretini kontrol eder.
 */
void parse_command(char *input, char **args, char **input_file, char **output_file, int *is_background);

/**
 * @brief Tek bir komutu çalıştırır.
 * 
 * @param args Çalıştırılacak komut ve argümanlar.
 * @param input_file Giriş dosyasının adı (yönlendirme için).
 * @param output_file Çıkış dosyasının adı (yönlendirme için).
 * @param is_background Arka plan işlemi bayrağı (1: arka plan, 0: ön plan).
 * 
 * Yeni bir süreç oluşturur ve verilen komutu çalıştırır. Gerekirse giriş ve çıkış
 * dosyalarını yönlendirir. Komut arka planda çalışacaksa, bu süreç arka plan süreçleri
 * dizisine eklenir.
 */
void execute_command(char **args, char *input_file, char *output_file, int is_background);

/**
 * @brief Pipe (boru) kullanan komutları çalıştırır.
 * 
 * @param input Kullanıcının girdiği pipe içeren komut satırı.
 * 
 * Pipe sembolü (`|`) kullanılarak ayrılmış iki komut arasında bir pipe oluşturur. 
 * İlk komutun çıktısını ikinci komutun girdisi olarak kullanır ve her iki komutu
 * ayrı süreçlerde çalıştırır.
 */
void execute_pipe(char *input);

#endif // MYSHELL_H

