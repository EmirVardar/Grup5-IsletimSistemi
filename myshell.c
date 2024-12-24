#include "myshell.h"

// Arka plan süreçlerini takip eden bir dizi
BackgroundProcess bg_processes[64]; // Maksimum 64 arka plan süreci desteklenir
int bg_count = 0; // Aktif arka plan süreci sayısını tutar

// Arka plan süreçlerini kontrol eder ve tamamlananları temizler
void check_background_processes() {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].active) { // Sadece aktif olan süreçler kontrol edilir
            int status;
            pid_t result = waitpid(bg_processes[i].pid, &status, WNOHANG); // Sürecin tamamlanıp tamamlanmadığını kontrol et
            if (result > 0) { // Süreç tamamlanmışsa
                printf("\n[%d] retval: %d\n", bg_processes[i].pid, WEXITSTATUS(status)); // Sürecin çıkış durumu yazdırılır
                bg_processes[i].active = 0; // Sürecin artık aktif olmadığı işaretlenir
                print_prompt(); // Kullanıcıya yeni bir komut istemi gösterilir
            }
        }
    }
}

// Komut istemini ekrana yazdırır
void print_prompt() {
    printf("> "); // Kullanıcının komut gireceği yer
    fflush(stdout); // Çıktının hemen ekrana yazdırılmasını sağlar
}

// Kullanıcıdan alınan komutu ayrıştırır
void parse_command(char *input, char **args, char **input_file, char **output_file, int *is_background) {
    char *token;
    int i = 0;
    *input_file = NULL; // Varsayılan olarak giriş dosyası yok
    *output_file = NULL; // Varsayılan olarak çıkış dosyası yok
    *is_background = 0; // Varsayılan olarak işlem arka planda çalıştırılmaz

    token = strtok(input, " \n"); // Komutu boşluklara ve satır sonlarına göre ayır
    while (token != NULL) {
        if (strcmp(token, "<") == 0) { // Giriş yönlendirmesi
            token = strtok(NULL, " \n");
            *input_file = token; // Giriş dosyasını kaydet
        } else if (strcmp(token, ">") == 0) { // Çıkış yönlendirmesi
            token = strtok(NULL, " \n");
            *output_file = token; // Çıkış dosyasını kaydet
        } else if (strcmp(token, "&") == 0) { // Arka plan işlemi işareti
            *is_background = 1; // Arka planda çalıştırılacağını işaretle
        } else { // Komut ve argümanları
            args[i++] = token; // Argümanı kaydet
        }
        token = strtok(NULL, " \n");
    }
    args[i] = NULL; // Argüman dizisinin sonuna NULL ekle
}

// Alt süreç oluşturur ve bir komut çalıştırır
void execute_command(char **args, char *input_file, char *output_file, int is_background) {
    pid_t pid = fork(); // Yeni bir süreç oluştur

    if (pid == 0) { // Çocuk süreç
        if (input_file) { // Giriş dosyası yönlendirmesi
            int fd = open(input_file, O_RDONLY); // Giriş dosyasını aç
            if (fd < 0) {
                perror("Giriş dosyası açılamadı"); // Hata durumunda mesaj yazdır
                exit(1); // Süreç başarısızlıkla çık
            }
            dup2(fd, STDIN_FILENO); // Giriş dosyasını standart girişe yönlendir
            close(fd); // Dosya tanımlayıcısını kapat
        }
        if (output_file) { // Çıkış dosyası yönlendirmesi
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Çıkış dosyasını aç veya oluştur
            if (fd < 0) {
                perror("Çıkış dosyası açılamadı"); // Hata durumunda mesaj yazdır
                exit(1); // Süreç başarısızlıkla çık
            }
            dup2(fd, STDOUT_FILENO); // Çıkış dosyasını standart çıkışa yönlendir
            close(fd); // Dosya tanımlayıcısını kapat
        }
        execvp(args[0], args); // Komutu çalıştır
        perror("Komut çalıştırılamadı"); // Eğer execvp başarısız olursa hata yazdır
        exit(1); // Başarısızlıkla çık
    } else if (pid > 0) { // Ana süreç
        if (is_background) { // Eğer arka planda çalışacaksa
            bg_processes[bg_count].pid = pid; // Sürecin PID'sini kaydet
            bg_processes[bg_count].active = 1; // Süreci aktif olarak işaretle
            bg_count++;
            printf("[%d] arka planda çalışıyor\n", pid); // Kullanıcıya bilgi ver
        } else { // Ön planda çalışıyorsa
            waitpid(pid, NULL, 0); // Süreç tamamlanana kadar bekle
        }
    } else {
        perror("Fork başarısız oldu"); // Fork işleminde hata
    }
}

// Pipe (boru) kullanan komutları çalıştırır
void execute_pipe(char *input) {
    char *commands[2]; // Pipe ile ayrılmış iki komut
    char *args1[MAX_ARG_LEN], *args2[MAX_ARG_LEN]; // Her iki komutun argümanları
    char *input_file = NULL, *output_file = NULL;
    int is_background = 0;

    // Komutları pipe karakterine göre ayır
    commands[0] = strtok(input, "|");
    commands[1] = strtok(NULL, "|");

    if (!commands[1]) { // Eğer ikinci komut yoksa hata
        fprintf(stderr, "Hatalı boru komutu\n");
        return;
    }

    // Her iki komutu ayrıştır
    parse_command(commands[0], args1, &input_file, &output_file, &is_background);
    parse_command(commands[1], args2, &input_file, &output_file, &is_background);

    int pipe_fd[2]; // Pipe için dosya tanımlayıcıları
    if (pipe(pipe_fd) == -1) { // Pipe oluşturulamazsa hata
        perror("Pipe oluşturulamadı");
        return;
    }

    pid_t pid1 = fork(); // İlk süreç
    if (pid1 == 0) {
        dup2(pipe_fd[1], STDOUT_FILENO); // İlk komutun çıktısını pipe'a yönlendir
        close(pipe_fd[0]); // Pipe'ın okuma ucunu kapat
        close(pipe_fd[1]); // Pipe'ın yazma ucunu kapat
        execvp(args1[0], args1); // İlk komutu çalıştır
        perror("Pipe: İlk komut çalıştırılamadı");
        exit(1);
    }

    pid_t pid2 = fork(); // İkinci süreç
    if (pid2 == 0) {
        dup2(pipe_fd[0], STDIN_FILENO); // İkinci komutun girişini pipe'dan al
        close(pipe_fd[1]); // Pipe'ın yazma ucunu kapat
        close(pipe_fd[0]); // Pipe'ın okuma ucunu kapat
        execvp(args2[0], args2); // İkinci komutu çalıştır
        perror("Pipe: İkinci komut çalıştırılamadı");
        exit(1);
    }

    close(pipe_fd[0]); // Ana süreç pipe'ın okuma ucunu kapatır
    close(pipe_fd[1]); // Ana süreç pipe'ın yazma ucunu kapatır
    waitpid(pid1, NULL, 0); // İlk süreci bekle
    waitpid(pid2, NULL, 0); // İkinci süreci bekle
}

// Ana döngü
int main() {
    char input[MAX_CMD_LEN]; // Kullanıcı girişi için tampon
    char *args[MAX_ARG_LEN]; // Argüman listesi
    char *input_file, *output_file; // Giriş ve çıkış dosyaları
    int is_background; // Arka plan işlemi bayrağı

    print_prompt(); // İlk komut istemi yazdırılır

    while (1) {
        fd_set fds; // Giriş için dosya tanımlayıcı seti
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds); // Standart giriş sete eklenir

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100 ms zaman aşımı ayarı

        int ready = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout); // Giriş bekle

        if (ready > 0 && FD_ISSET(STDIN_FILENO, &fds)) { // Eğer giriş varsa
            if (fgets(input, MAX_CMD_LEN, stdin)) { // Kullanıcı girdisi alınır
                if (strcmp(input, "quit\n") == 0) { // Kullanıcı çıkmak istiyorsa
                    printf("Tüm arka plan işlemleri tamamlanana kadar bekleniyor...\n");
                    while (1) {
                        check_background_processes(); // Arka plan süreçlerini kontrol et
                        int active = 0;
                        for (int i = 0; i < bg_count; i++) {
                            if (bg_processes[i].active) { // Aktif süreç var mı?
                                active = 1;
                                break;
                            }
                        }
                        if (!active) break; // Aktif süreç yoksa döngüden çık
                        usleep(100000); // 100 ms bekle
                    }
                    printf("Çıkılıyor...\n");
                    break;
                }

                if (strchr(input, '|')) { // Eğer girişte pipe varsa
                    execute_pipe(input); // Pipe komutlarını çalıştır
                } else { // Pipe yoksa normal komut çalıştır
                    parse_command(input, args, &input_file, &output_file, &is_background);
                    execute_command(args, input_file, output_file, is_background);
                }

                print_prompt(); // Yeni komut istemi yazdırılır
            }
        }

        check_background_processes(); // Arka plan süreçlerini kontrol et
    }

    return 0;
}

