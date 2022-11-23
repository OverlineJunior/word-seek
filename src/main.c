// NOTE: For now, it is being assumed that the amount of unique words is equal to MAX_WORDS.
// TODO: Ask for the words to find and replace MAX_WORDS with the amount given *where appropriate*.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MATRIX_SCALE 7
#define MAX_WORDS 3

typedef enum {
    RIGHT,
    LEFT,
    UP,
    DOWN,
} Direction;

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    int found;
    Position first_pos;
    Position last_pos;
} FindFirstResult;

typedef struct {
	Position first_pos;
	Direction dir;
} Encounter;

typedef struct {
    char word[MATRIX_SCALE + 1];
    int count;
    Encounter encounters[MATRIX_SCALE * MATRIX_SCALE];
} WordData;

typedef struct {
    WordData word_datas[MAX_WORDS];
} MatrixData;

void assert_msg(int condition, char msg[]) {
	if (!condition) {
		puts(msg);
		exit(1);
	}
}

void display_find_first_result(FindFirstResult res) {
    char found[6];
    strcpy(found, res.found == true ? "true" : "false");

    printf(
        "Found: %s\n"
        "FirstPos: {row: %i, col: %i}\n"
        "LastPos: {row: %i, col: %i}\n",
        found,
        res.first_pos.row,
        res.first_pos.col,
        res.last_pos.row,
        res.last_pos.col
    );
}

void display_matrix_data(MatrixData matrix_data) {
    for (int i = 0; i < MAX_WORDS; i++) {
        const WordData word_data = matrix_data.word_datas[i];
        printf("'%s' foi encontrado %i vezes:\n", word_data.word, word_data.count);

        for (int j = 0; j < word_data.count; j++) {
            const Encounter enc = word_data.encounters[j];

            char dir_str[11];
            strcpy(dir_str, enc.dir == DOWN || enc.dir == UP ? "Vertical" : "Horizontal");

            printf("%u. 1a posicao: (%u, %u)   Direcao: %s\n", j + 1, enc.first_pos.row, enc.first_pos.col, dir_str);
        }

        printf("\n");
    }
}

void fill_matrix_from_file(char matrix[][MATRIX_SCALE], FILE *file_ptr) {
	int row = 0, col = 0;
	char ch;

	do {
		ch = fgetc(file_ptr);

		if ((ch == ' ') || (int) ch == -1) continue;
		
		if (ch == '\n') {
			assert_msg(col == MATRIX_SCALE, "Nao pode haver menos do que 7 letras em uma linha");

			row++;
			col = 0;
		} else {
			assert_msg(col < MATRIX_SCALE, "Nao pode haver mais do que 7 letras em uma linha");

			matrix[row][col] = toupper(ch);
			col++;
		}
	} while (ch != EOF);

	assert_msg(row < MATRIX_SCALE, "Nao pode haver mais do que 7 linhas");
}

FindFirstResult find_first(
    char word[],
    Position start_pos,
    Direction dir,
    char matrix[MATRIX_SCALE][MATRIX_SCALE]
) {
    int match_combo = 0;
    int word_len = strlen(word);
    Position first_pos;
    FindFirstResult res = {.found = false};

    // Init: if direction is vertical, start from start_pos.row since we will be moving by rows; else, move by col.
    // Condition: if direction is down or right, use a condition that accepts increasing row or col; else, decreasing row or col.
    // Increment: going down or right means increasing row or col while up or left means decreasing it.
    for (
        int i = dir == DOWN || dir == UP ? start_pos.row : start_pos.col;
        dir == DOWN || dir == RIGHT ? i < MATRIX_SCALE : i >= 0;
        dir == DOWN || dir == RIGHT ? i++ : i--
    ) {
        // If direction is vertical, move by row, while the col is constant; else, move by col, while the row is constant.
        const Position pos = {
            .row = dir == DOWN || dir == UP ? i : start_pos.row,
            .col = dir == DOWN || dir == UP ? start_pos.col : i
        };
        const char ch = matrix[pos.row][pos.col];

        if (ch == word[match_combo]) {
            match_combo++;

            if (match_combo == 1) {
                first_pos = pos;
            }

            if (match_combo == word_len) {
                res.found = true;
                res.first_pos = first_pos;
                res.last_pos = pos;

                break;
            }
        } else {
            match_combo = 0;
        }
    }

    return res;
}

MatrixData get_vertical_matrix_data(
    char matrix[MATRIX_SCALE][MATRIX_SCALE],
    char words[MAX_WORDS][MATRIX_SCALE + 1]
) {
    const int last_index = MATRIX_SCALE - 1;
    MatrixData matrix_data;
    
    for (int i = 0; i < MAX_WORDS; i++) {
        Position pos = {0, 0};
        Direction dir = DOWN;

        char word[MATRIX_SCALE + 1];
        strcpy(word, words[i]);

        WordData word_data = {.count = 0};
        strcpy(word_data.word, word);

        while (true) {
            FindFirstResult res = find_first(word, pos, dir, matrix);
            const int finishing_row = dir == DOWN ? last_index : 0;
            const int is_dir_finished = res.found ? res.last_pos.row == finishing_row : true;
            const int is_scan_finished = is_dir_finished && dir == UP && pos.col == last_index;

            if (res.found) {
                Encounter enc = {res.first_pos, dir};
                word_data.encounters[word_data.count] = enc;

                word_data.count++;
            }

            if (is_scan_finished) break;

            if (is_dir_finished) {
                if (dir == UP) {
                    pos.row = 0;
                    pos.col++;
                    dir = DOWN;
                } else {
                    pos.row = last_index;
                    dir = UP;
                }
            } else {
                pos.row = res.last_pos.row + 1;
            }
        }

        matrix_data.word_datas[i] = word_data;
    }

    return matrix_data;
}

MatrixData get_horizontal_matrix_data(
    char matrix[MATRIX_SCALE][MATRIX_SCALE],
    char words[MAX_WORDS][MATRIX_SCALE + 1]
) {
    const int last_index = MATRIX_SCALE - 1;
    MatrixData matrix_data;

    for (int i = 0; i < MAX_WORDS; i++) {
        Position pos = {0, 0};
        Direction dir = RIGHT;

        char word[MATRIX_SCALE + 1];
        strcpy(word, words[i]);

        WordData word_data = {.count = 0};
        strcpy(word_data.word, word);

        while (true) {
            FindFirstResult res = find_first(word, pos, dir, matrix);
            const int finishing_col = dir == RIGHT ? last_index : 0;
            const int is_dir_finished = res.found ? res.last_pos.col == finishing_col : true;
            const int is_scan_finished = is_dir_finished && dir == LEFT && pos.row == last_index;

            if (res.found) {
                Encounter enc = {res.first_pos, dir};
                word_data.encounters[word_data.count] = enc;

                word_data.count++;
            }

            if (is_scan_finished) break;

            if (is_dir_finished) {
                if (dir == LEFT) {
                    pos.row++;
                    pos.col = 0;
                    dir = RIGHT;
                } else {
                    pos.col = last_index;
                    dir = LEFT;
                }
            } else {
                pos.col = res.last_pos.col + 1;
            }
        }

        matrix_data.word_datas[i] = word_data;
    }

    return matrix_data;
}

MatrixData get_matrix_data(
    char matrix[MATRIX_SCALE][MATRIX_SCALE],
    char words[MAX_WORDS][MATRIX_SCALE + 1]
) {
    const MatrixData vertical_data = get_vertical_matrix_data(matrix, words);
    const MatrixData horizontal_data = get_horizontal_matrix_data(matrix, words);
    MatrixData matrix_data;

    for (int i = 0; i < MAX_WORDS; i++) {
        WordData vert_wdata = vertical_data.word_datas[i];
        WordData hori_wdata = horizontal_data.word_datas[i];

        WordData merged_wdata = {
            .count = vert_wdata.count + hori_wdata.count,
        };
        strcpy(merged_wdata.word, vert_wdata.word);

        for (int j = 0; j < vert_wdata.count; j++) {
            merged_wdata.encounters[j] = vert_wdata.encounters[j];
        }

        for (int j = 0; j < hori_wdata.count; j++) {
            merged_wdata.encounters[j + vert_wdata.count] = hori_wdata.encounters[j];
        }

        matrix_data.word_datas[i] = merged_wdata;
    }

    return matrix_data;
}

int main() {
    FILE *file_ptr = fopen("../playground.txt", "r");
	assert_msg(file_ptr != NULL, "Nao foi possivel encontrar ou abrir o caminho '../playground.txt'");

    char matrix[MATRIX_SCALE][MATRIX_SCALE];
    fill_matrix_from_file(matrix, file_ptr);

    char words[MAX_WORDS][MATRIX_SCALE + 1] = {"FOO", "BAR", "BAZ"};

    const MatrixData data = get_matrix_data(matrix, words);
    display_matrix_data(data);

    return 0;
}
