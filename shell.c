#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>

// Komutlar ve argümanlar için maksimum uzunluk tanımları
#define MAX_CMD_LEN 1024 // Kullanıcının girdiği komutun maksimum uzunluğu
#define MAX_ARG_LEN 64   // Komut içindeki argüman sayısının maksimum uzunluğu

// Fonksiyon prototipleri
void print_prompt(); // Komut istemini ekrana yazdırır
void check_background_processes(); // Arka plan süreçlerini kontrol eder
void parse_command(char *input, char **args, char **input_file, char **output_file, int *is_background); // Kullanıcıdan alınan komutu ayrıştırır
void execute_command(char **args, char *input_file, char *output_file, int is_background); // Tek bir komutu çalıştırır
void execute_pipe(char *input); // Boru (pipe) içeren komutları çalıştırır

// Arka plan süreçleri için bir yapı tanımı
typedef struct {
    pid_t pid;   // Sürecin PID'si
    int active;  // Sürecin aktif olup olmadığını gösterir (1: aktif, 0: tamamlanmış)
} BackgroundProcess;

// Arka plan süreçlerini takip eden bir dizi
BackgroundProcess bg_processes[64];
int bg_count = 0; // Arka plan süreci sayısı

// Arka plan süreçlerini kontrol eder ve tamamlananları temizler
void check_background_processes() {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].active) { // Sadece aktif süreçler kontrol edilir
            int status;
            pid_t result = waitpid(bg_processes[i].pid, &status, WNOHANG); // Süreç tamamlandı mı kontrol edilir
            if (result > 0) { // Süreç tamamlandıysa
                printf("\n[%d] retval: %d\n", bg_processes[i].pid, WEXITSTATUS(status)); // Süreç sonucu yazdırılır
                bg_processes[i].active = 0; // Süreç tamamlandığı için aktifliği sıfırlanır
                print_prompt(); // Kullanıcıya yeni bir komut istemi gösterilir
            }
        }
    }
}

// Komut istemini (prompt) ekrana yazdırır
void print_prompt() {
    printf("> "); // Komut satırı başlangıcında ">" işareti
    fflush(stdout); // Çıktıyı hemen ekrana yazdırır
}

// Kullanıcıdan alınan komutu ayrıştırır
void parse_command(char *input, char **args, char **input_file, char **output_file, int *is_background) {
    char *token;
    int i = 0;
    *input_file = NULL; // Giriş dosyası için başlangıç değeri
    *output_file = NULL; // Çıkış dosyası için başlangıç değeri
    *is_background = 0; // Arka plan işlemi varsayılan olarak devre dışı

    token = strtok(input, " \n"); // Komutu boşluklara ve yeni satırlara göre ayrıştırır
    while (token != NULL) {
        if (strcmp(token, "<") == 0) { // Giriş dosyası operatörü "<"
            token = strtok(NULL, " \n");
            *input_file = token;
        } else if (strcmp(token, ">") == 0) { // Çıkış dosyası operatörü ">"
            token = strtok(NULL, " \n");
            *output_file = token;
        } else if (strcmp(token, "&") == 0) { // Arka plan operatörü "&"
            *is_background = 1;
        } else { // Normal komut ve argümanlar
            args[i++] = token;
        }
        token = strtok(NULL, " \n");
    }
    args[i] = NULL; // Argüman dizisinin sonuna NULL eklenir
}

// Alt süreç oluşturur ve bir komut çalıştırır
void execute_command(char **args, char *input_file, char *output_file, int is_background) {
    pid_t pid = fork(); // Yeni bir süreç oluşturulur

    if (pid == 0) { // Çocuk süreç
        if (input_file) { // Eğer giriş dosyası belirtilmişse
            int fd = open(input_file, O_RDONLY); // Giriş dosyası açılır
            if (fd < 0) {
                perror("Giriş dosyası açılamadı");
                exit(1); // Hata durumunda çıkış yapılır
            }
            dup2(fd, STDIN_FILENO); // Giriş dosyasını standart girişe yönlendir
            close(fd); // Dosya tanımlayıcısını kapat
        }
        if (output_file) { // Eğer çıkış dosyası belirtilmişse
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Çıkış dosyası açılır
            if (fd < 0) {
                perror("Çıkış dosyası açılamadı");
                exit(1); // Hata durumunda çıkış yapılır
            }
            dup2(fd, STDOUT_FILENO); // Çıkış dosyasını standart çıkışa yönlendir
            close(fd); // Dosya tanımlayıcısını kapat
        }
        execvp(args[0], args); // Komut çalıştırılır
        perror("Komut çalıştırılamadı"); // Eğer execvp başarısız olursa hata mesajı yazdırılır
        exit(1);
    } else if (pid > 0) { // Ana süreç
        if (is_background) { // Eğer komut arka planda çalıştırılacaksa
            bg_processes[bg_count].pid = pid; // Sürecin PID'si kaydedilir
            bg_processes[bg_count].active = 1; // Süreç aktif olarak işaretlenir
            bg_count++;
            printf("[%d] arka planda çalışıyor\n", pid); // Arka plan bilgisi yazdırılır
        } else { // Eğer arka planda değilse, ana süreç çocuk süreci bekler
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("Fork başarısız oldu"); // Fork işlemi başarısız olursa hata yazdırılır
    }
}

// Pipe (boru) kullanan komutları çalıştırır
void execute_pipe(char *input) {
    char *commands[2];
    char *args1[MAX_ARG_LEN], *args2[MAX_ARG_LEN];
    char *input_file = NULL, *output_file = NULL;
    int is_background = 0;

    // Komutu boruya göre ikiye ayırır
    commands[0] = strtok(input, "|");
    commands[1] = strtok(NULL, "|");

    if (!commands[1]) { // Eğer ikinci komut yoksa hata mesajı yazdırılır
        fprintf(stderr, "Hatalı boru komutu\n");
        return;
    }

    // İlk ve ikinci komutları ayrıştırır
    parse_command(commands[0], args1, &input_file, &output_file, &is_background);
    parse_command(commands[1], args2, &input_file, &output_file, &is_background);

    int pipe_fd[2]; // Boru için dosya tanımlayıcıları
    if (pipe(pipe_fd) == -1) { // Pipe oluşturulamazsa hata
        perror("Pipe oluşturulamadı");
        return;
    }

    pid_t pid1 = fork(); // İlk süreç
    if (pid1 == 0) {
        dup2(pipe_fd[1], STDOUT_FILENO); // İlk komutun çıktısını boruya yönlendirir
        close(pipe_fd[0]); // Borunun okuma ucunu kapatır
        close(pipe_fd[1]); // Borunun yazma ucunu kapatır
        execvp(args1[0], args1); // İlk komut çalıştırılır
        perror("Pipe: İlk komut çalıştırılamadı");
        exit(1);
    }

    pid_t pid2 = fork(); // İkinci süreç
    if (pid2 == 0) {
        dup2(pipe_fd[0], STDIN_FILENO); // İkinci komutun girişini borudan alır
        close(pipe_fd[1]); // Borunun yazma ucunu kapatır
        close(pipe_fd[0]); // Borunun okuma ucunu kapatır
        execvp(args2[0], args2); // İkinci komut çalıştırılır
        perror("Pipe: İkinci komut çalıştırılamadı");
        exit(1);
    }

    close(pipe_fd[0]); // Ana süreç borunun okuma ucunu kapatır
    close(pipe_fd[1]); // Ana süreç borunun yazma ucunu kapatır
    waitpid(pid1, NULL, 0); // İlk süreci bekler
    waitpid(pid2, NULL, 0); // İkinci süreci bekler
}

// Ana döngü
int main() {
    char input[MAX_CMD_LEN]; // Kullanıcı girişi için tampon
    char *args[MAX_ARG_LEN]; // Argüman listesi
    char *input_file, *output_file; // Giriş ve çıkış dosyaları
    int is_background; // Arka plan işlemi işareti

    print_prompt(); // İlk komut istemi yazdırılır

    while (1) {
        fd_set fds; // Dosya tanımlayıcı seti
        FD_ZERO(&fds); // Set sıfırlanır
        FD_SET(STDIN_FILENO, &fds); // Standart giriş sete eklenir

        // Zaman aşımı ayarı (100 ms)
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        // Kullanıcı girişini kontrol eder
        int ready = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);

        if (ready > 0 && FD_ISSET(STDIN_FILENO, &fds)) { // Giriş varsa
            if (fgets(input, MAX_CMD_LEN, stdin)) { // Giriş okuma
                if (strcmp(input, "quit\n") == 0) { // Kullanıcı çıkmak istiyorsa
                    printf("Tüm arka plan işlemleri tamamlanana kadar bekleniyor...\n");
                    while (1) {
                        check_background_processes(); // Arka plan süreçleri kontrol edilir
                        int active = 0;
                        for (int i = 0; i < bg_count; i++) {
                            if (bg_processes[i].active) { // Aktif süreç var mı kontrol edilir
                                active = 1;
                                break;
                            }
                        }
                        if (!active) break; // Tüm süreçler tamamlandıysa döngüden çık
                        usleep(100000); // 100 ms bekle
                    }
                    printf("Çıkılıyor...\n");
                    break;
                }

                if (strchr(input, '|')) { // Eğer girişte pipe varsa
                    execute_pipe(input); // Pipe komutlarını çalıştırır
                } else { // Tekli komutlar için
                    parse_command(input, args, &input_file, &output_file, &is_background);
                    execute_command(args, input_file, output_file, is_background);
                }

                print_prompt(); // Yeni komut istemi gösterilir
            }
        }

        // Arka plan süreçleri kontrol edilir
        check_background_processes();
    }

    return 0;
}

