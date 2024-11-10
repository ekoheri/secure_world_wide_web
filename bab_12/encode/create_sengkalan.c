#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 10
#define MAX_COLS 21
#define MAX_WORD_LENGTH 50

const char List_Sengkalan[MAX_ROWS][MAX_COLS][MAX_WORD_LENGTH] = {
    {"akasa", "awang-awang", "barakan", "brastha", "byoma", "doh", "gegana", "ilang", "kombul", "kos", "langit", "luhur", "mesat", "mletik", "muksa", "muluk", "musna", "nenga", "ngles", "nir", "nis"},
    {"badan", "budha", "budi", "buweng", "candra", "dara", "dhara", "eka", "gusti", "hyang", "iku", "jagat", "kartika", "kenya", "lek", "luwih", "maha", "nabi", "nata", "nekung", "niyata"},
    {"apasang", "asta", "athi-athi", "buja", "bujana", "dresthi", "dwi", "gandheng", "kalih", "kanthi", "kembar", "lar", "mandeng", "myat", "nayana", "nembeh", "netra", "ngabekti", "paksa", "sikara", "sungu"},
    {"agni", "api", "apyu", "bahni", "benter", "brama", "dahana", "guna", "jatha", "kaeksi", "katingalan", "katon", "kawruh", "kaya", "kobar", "kukus", "lir", "murub", "nala", "naut", "nauti"},
    {"bun", "catur", "dadya", "gawe", "her", "jaladri", "jalanidhi", "karta", "karti", "karya", "keblat", "marna", "marta", "masuh", "nadi", "papat", "pat", "samodra", "sagara", "sindu", "suci"},
    {"angin", "astra", "bajra", "bana", "bayu", "buta", "cakra", "diyu", "galak", "gati", "guling", "hru", "indri", "indriya", "jemparing", "lima", "lungid", "marga", "margana", "maruta", "nata"},
    {"amla", "anggana", "anggang-anggang", "amnggas", "artati", "carem", "glinggang", "hoyag", "ilat", "karaseng", "karengya", "kayasa", "kayu", "kilatan", "lidhah", "lindhu", "lona", "manis", "naya", "nem", "nenem"},
    {"acala", "ajar", "angsa", "ardi", "arga", "aswa", "biksu", "biksuka", "dwija", "giri", "gora", "himawan", "kaswareng", "kuda", "muni", "nabda", "pandhita", "pitu", "prabata", "resi", "sabda"},
    {"anggusti", "astha", "bajul", "basu", "basuki", "baya", "bebaya", "brahma", "brahmana", "bujangga", "dirada", "dwipa", "dwipangga", "dwirada", "estha", "esthi", "gajah", "kunjara", "madya", "liman", "madya"},
    {"ambuka", "anggangsir", "angleng", "angrong", "arum", "babahan", "bedah", "bolong", "butul", "dewa", "dwara", "ganda", "gapura", "gatra", "guwa", "jawata", "kori", "kusuma", "lawang", "manjing", "masuk"}
};

char *utf8_to_hex(const char *utf8_str) {
    int len = strlen(utf8_str);
    char *hex_str = malloc((len * 2) + 1);  // +1 untuk null terminator
    if (hex_str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", (unsigned char)utf8_str[i]);
    }
    hex_str[len * 2] = '\0';  // Null-terminate string

    return hex_str;
}

void print_sengkalan_in_hex() {
    printf("{\n");
    for (int i = 0; i < MAX_ROWS; i++) {
        printf("    {");
        for (int j = 0; j < MAX_COLS; j++) {
            char *hex = utf8_to_hex(List_Sengkalan[i][j]);
            printf("\"%s\"", hex);
            free(hex);  // Bebaskan memori yang dialokasikan oleh utf8_to_hex

            // Tambahkan koma setelah elemen kecuali elemen terakhir
            if (j < MAX_COLS - 1) {
                printf(", ");
            }
        }
        printf("},\n");
    }
    printf("}\n");
}

int main() {
    //print_sengkalan_in_hex();
    printf("%s\n", utf8_to_hex("HN792016"));
    return 0;
}
