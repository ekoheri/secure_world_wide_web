#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_ROWS 10
#define MAX_COLS 21
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 256
#define HASH_SIZE 1024

typedef struct HashNode {
    char word[MAX_WORD_LENGTH];
    int row;
    struct HashNode *next;
} HashNode;

typedef struct HashTable {
    HashNode *table[HASH_SIZE];
} HashTable;

// Definisi List_Sengkalan
const char List_Sengkalan[MAX_ROWS][MAX_COLS][MAX_WORD_LENGTH] = {
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

// Fungsi hash untuk mengonversi string menjadi indeks
unsigned int hash(const char *word) {
    unsigned int hash = 5381;
    int c;
    while ((c = *word++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % HASH_SIZE;
}

// Fungsi untuk menambahkan kata ke hash table
void insert(HashTable *ht, const char *word, int row) {
    unsigned int index = hash(word);
    HashNode *newNode = malloc(sizeof(HashNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->word, word);
    newNode->row = row;
    newNode->next = ht->table[index];
    ht->table[index] = newNode;
}

// Fungsi untuk mencari kata di hash table
int search(HashTable *ht, const char *word) {
    unsigned int index = hash(word);
    HashNode *node = ht->table[index];
    while (node != NULL) {
        if (strcmp(node->word, word) == 0) {
            return node->row; // Mengembalikan baris yang ditemukan
        }
        node = node->next;
    }
    return -1; // Tidak ditemukan
}

// Fungsi untuk encode input
char *encode(const char *input) {
    srand(time(NULL)); // Inisialisasi seed untuk fungsi rand()
    char *sentence = malloc(MAX_INPUT_LENGTH);
    if (sentence == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    sentence[0] = '\0'; // Inisialisasi string kosong

    for (int i = strlen(input) - 1; i >= 0; i--) {
        int index = input[i] - '0'; // Konversi karakter ke digit
        if (index >= 0 && index < MAX_ROWS) {
            int random_index = rand() % MAX_COLS; // Pilih indeks acak 0-21
            if (strlen(sentence) > 0) {
                strcat(sentence, "20");
            }
            strcat(sentence, List_Sengkalan[index][random_index]);
        }
    }
    return sentence;
}

// Fungsi untuk decode input
/*char *decode(const char *input) {
    static char number[MAX_INPUT_LENGTH] = "";
    number[0] = '\0'; // Inisialisasi string kosong

    char *koreksi = malloc(strlen(input) + 1);
    if (koreksi == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Hapus karakter non-alphabet
    int k = 0;
    for (int i = 0; input[i]; i++) {
        if (isalnum(input[i]) || isspace(input[i]) || input[i] == '-') {
            koreksi[k++] = input[i];
        }
    }
    koreksi[k] = '\0'; // Akhiri string
    // Inisialisasi hash table
    HashTable ht = {0};
    
    // Tambahkan kata ke hash table
    for (int rows = 0; rows < MAX_ROWS; rows++) {
        for (int cols = 0; cols < MAX_COLS; cols++) {
            insert(&ht, List_Sengkalan[rows][cols], rows);
        }
    }

    char *word = strtok(koreksi, " ");
    while (word != NULL) {
        int row = search(&ht, word);
        if (row != -1) {
            sprintf(number + strlen(number), "%d", row); // Tambahkan angka
        }
        word = strtok(NULL, " ");
    }
    free(koreksi); // Bebaskan memori

    // Balikkan string hasil
    int len = strlen(number);
    for (int i = 0; i < len / 2; i++) {
        char temp = number[i];
        number[i] = number[len - i - 1];
        number[len - i - 1] = temp;
    }

    return number;
}*/

char *decode(const char *input) {
    static char number[MAX_INPUT_LENGTH] = "";
    number[0] = '\0'; // Inisialisasi string kosong

    char *koreksi = (char*) malloc(strlen(input) + 1);
    if (koreksi == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Hapus karakter non-heksadesimal
    int k = 0;
    for (int i = 0; input[i]; i++) {
        if (isxdigit(input[i]) || (input[i] == '2' && input[i + 1] == '0')) {
            koreksi[k++] = input[i];
            if (input[i] == '2' && input[i + 1] == '0') {
                koreksi[k++] = input[++i];
            }
        }
    }
    koreksi[k] = '\0'; // Akhiri string dengan null terminator

    // Inisialisasi hash table
    HashTable ht = {0};
    
    // Tambahkan kata ke hash table
    for (int rows = 0; rows < MAX_ROWS; rows++) {
        for (int cols = 0; cols < MAX_COLS; cols++) {
            if (List_Sengkalan[rows][cols][0] != '\0') {
                insert(&ht, List_Sengkalan[rows][cols], rows);
            }
        }
    }

    // Menggunakan strstr untuk memproses segmen yang dipisahkan "20"
    char *segment = koreksi;
    char *next_segment;
    while ((next_segment = strstr(segment, "20")) != NULL) {
        *next_segment = '\0'; // Gantikan "20" dengan null terminator untuk memisahkan segmen
        int row = search(&ht, segment);
        if (row != -1) {
            sprintf(number + strlen(number), "%d", row); // Tambahkan angka hasil
        }
        segment = next_segment + 2; // Lanjutkan setelah "20"
    }

    // Proses segmen terakhir
    int row = search(&ht, segment);
    if (row != -1) {
        sprintf(number + strlen(number), "%d", row);
    }

    free(koreksi); // Bebaskan memori

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
    // Contoh penggunaan encode
    // Membaca input dari pengguna
    char input[1024];
    printf("Masukkan teks untuk encode: ");
    fgets(input, sizeof(input), stdin);

    // Menghilangkan newline karakter jika ada
    input[strcspn(input, "\n")] = '\0';

    char *encoded = encode(input);
    printf("Encoded: %s\n", encoded);
    
    // Contoh penggunaan decode
    char *decoded = decode(encoded);
    printf("Decoded: %s\n", decoded);
    
    free(encoded); // Bebaskan memori
    return 0;
}
