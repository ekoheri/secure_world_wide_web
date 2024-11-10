#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ROWS 10
#define NUM_COLS 21

// Daftar Sengkalan
const char *List_Sengkalan[NUM_ROWS][NUM_COLS] = {
    {"416b617361", "4177616e672d4177616e67", "426172616b616e", "42726173746861", "42796f6d61", "446f68", "476567616e61", "496c616e67", "4b6f6d62756c", "4b6f73", "4c616e676974", "4c75687572", "4d65736174", "4d6c6574696b", "4d756b7361", "4d756c756b", "4d75736e61", "4e656e6761", "4e676c6573", "4e6972", "4e6973"},
    {"426164616e", "4275646861", "42756469", "427577656e67", "43616e647261", "44617261", "4468617261", "456b61", "4775737469", "4879616e67", "496b75", "4a61676174", "4b617274696b61", "4b656e7961", "4c656b", "4c75776968", "4d616861", "4e616269", "4e617461", "4e656b756e67", "4e6979617461"},
    {"41706173616e67", "41737461", "417468692d61746869", "42756a61", "42756a616e61", "44726573746869", "447769", "47616e6468656e67", "6b616c6968", "4b616e746869", "6b656d626172", "4c6172", "4d616e64656e67", "4d796174", "4e6179616e61", "4e656d626568", "4e65747261", "4e676162656b7469", "50616b7361", "53696b617261", "53756e6775"},
    {"41676e69", "417069", "41707975", "4261686e69", "42656e746572", "4272616d61", "446168616e61", "47756e61", "4a61746861", "4b61656b7369", "4b6174696e67616c616e", "4b61746f6e", "4b6177727568", "4b617961", "4b6f626172", "4b756b7573", "4c6972", "4d75727562", "4e616c61", "4e617574", "4e61757469"},
    {"42756e", "4361747572", "4461647961", "47617765", "486572", "4a616c61647269", "4a616c616e69646869", "6b61727461", "4b61727469", "4b61727961", "4b65626c6174", "4d61726e61", "4d61727461", "4d61737568", "4e616469", "5061706174", "506174", "53616d6f647261", "536167617261", "53696e6475", "53756369"},
    {"416e67696e", "4173747261", "42616a7261", "42616e61", "42617975", "42757461", "43616b7261", "44697975", "47616c616b", "47617469", "47756c696e67", "487275", "496e647269", "496e6472697961", "4a656d706172696e67", "4c696d61", "4c756e676964", "4d61726761", "4d617267616e61", "4d6172757461", "4e617461"},
    {"416d6c61", "416e6767616e61", "416e6767616e672d416e6767616e67", "416d6e67676173", "417274617469", "436172656d", "476c696e6767616e67", "486f796167", "496c6174", "4b61726173656e67", "4b6172656e677961", "4b6179617361", "4b617975", "4b696c6174616e", "4c6964686168", "4c696e646875", "4c6f6e61", "4d616e6973", "4e617961", "4e656d", "4e656e656d"},
    {"4163616c61", "416a6172", "416e677361", "41726469", "41726761", "41737761", "42696b7375", "42696b73756b61", "4477696a61", "47697269", "476f7261", "48696d6177616e", "4b6173776172656e67", "4b756461", "4d756e69", "4e61626461", "50616e6468697461", "50697475", "50726162617461", "52657369", "5361626461"},
    {"416e676775737469", "4173746861", "42616a756c", "42617375", "426173756b69", "42617961", "426562617961", "427261686d61", "427261686d616e61", "42756a616e676761", "446972616461", "4477697061", "44776970616e676761", "44776972616461", "4573746861", "4573746869", "47616a6168", "4b756e6a617261", "4d61647961", "4c696d616e", "4d61647961"},
    {"416d62756b61", "416e6767616e67736972", "416e676c656e67", "416e67726f6e67", "4172756d", "4261626168616e", "4265646168", "426f6c6f6e67", "427574756c", "44657761", "4477617261", "47616e6461", "476170757261", "4761747261", "47757761", "4a6177617461", "4b6f7269", "4b7573756d61", "4c6177616e67", "4d616e6a696e67", "4d6173756b"}
};

// Fungsi Encode
char* encode(const char *input) {
    static char sentence[1024];
    strcpy(sentence, "");
    srand(time(NULL));

    for (int i = strlen(input) - 1; i >= 0; i--) {
        int index = input[i] - '0';  // Mengambil angka dari karakter
        if (strlen(sentence) == 0) {
            snprintf(sentence, sizeof(sentence), "%s", List_Sengkalan[index][rand() % NUM_COLS]);
        } else {
            strcat(sentence, "20");
            strcat(sentence, List_Sengkalan[index][rand() % NUM_COLS]);
        }
    }
    return sentence;
}

// Fungsi Decode
char* decode(const char *input) {
    static char number[1024];
    strcpy(number, "");
    
    char koreksi[1024];
    int idx = 0;

    // Menghapus karakter selain alfanumerik dan spasi
    for (int i = 0; input[i] != '\0'; i++) {
        if ((input[i] >= 'a' && input[i] <= 'z') || (input[i] >= '0' && input[i] <= '9') || input[i] == ' ') {
            koreksi[idx++] = input[i];
        }
    }
    koreksi[idx] = '\0';  // Menambahkan null-terminator

    // Menggunakan strstr untuk mencari pemisah "20" dan membagi string
    char *start = koreksi;
    char *delimiter = "20";
    char *word;

    while ((word = strstr(start, delimiter)) != NULL) {
        // Menampilkan substring sebelum "20"
        int length = word - start;
        char temp[length + 1];
        strncpy(temp, start, length);
        temp[length] = '\0';

        // Cari kata yang sesuai dengan daftar Sengkalan
        int found = 0;
        for (int row = 0; row < NUM_ROWS && !found; row++) {
            for (int col = 0; col < NUM_COLS && !found; col++) {
                if (strcmp(List_Sengkalan[row][col], temp) == 0) {
                    sprintf(number + strlen(number), "%d", row);
                    found = 1;
                }
            }
        }

        start = word + strlen(delimiter); // Lanjutkan setelah delimiter
    }

    // Menangani bagian terakhir setelah "20" terakhir
    if (*start != '\0') {
        // Cari kata yang sesuai dengan daftar Sengkalan
        int found = 0;
        for (int row = 0; row < NUM_ROWS && !found; row++) {
            for (int col = 0; col < NUM_COLS && !found; col++) {
                if (strcmp(List_Sengkalan[row][col], start) == 0) {
                    sprintf(number + strlen(number), "%d", row);
                    found = 1;
                }
            }
        }
    }

    // Balikkan string hasil
    int len = strlen(number);
    for (int i = 0; i < len / 2; i++) {
        char temp = number[i];
        number[i] = number[len - i - 1];
        number[len - i - 1] = temp;
    }

    return number;
}

int main() {
    const char *encoded = encode("1400");
    printf("Encoded: %s\n", encoded);

    const char *decoded = decode(encoded);
    printf("Decoded: %s\n", decoded);

    return 0;
}
