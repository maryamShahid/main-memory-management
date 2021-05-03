struct Buddy {
    int free;
    int space;
    struct Buddy *pair;
} Buddy;

int buddyAlloc(struct Buddy *bud, int size, int address) {
    if (bud == NULL) {
        return -1;
    }
    if (bud->free != 1) {
        return buddyAlloc(bud->pair, size, address + bud->space);
    }
    if (bud->space == size) {
        bud->free = 0;
        return address;
    } else if (bud->space < size) {
        return buddyAlloc(bud->pair, size, address + bud->space);
    } else {
        int newBlock = bud->space;
        newBlock = newBlock / 2;
        struct Buddy *temp1 = bud->pair;

        bud->pair = malloc(sizeof(Buddy));
        bud->free = 1;
        bud->space = newBlock;

        struct Buddy *temp2 = bud->pair;
        temp2->pair = temp1;
        temp2->free = 1;
        temp2->space = newBlock;

        return buddyAlloc(bud, size, address);
    }
}

int buddyFree(struct Buddy *bud, int address) {
    if (address == 0) {
        if (bud->free != 0) {
            return -1;
        }
        bud->free = 1;

        return (1);
    }
    if (bud == NULL) {
        return (-1);
    } else {
        return buddyFree(bud->pair, address - bud->space);
    }
}

void mergeBuddy(struct Buddy* bud) {
    if (bud == NULL) {
        return;
    }

    mergeBuddy(bud->pair);

    if (bud->pair == NULL) {
        return;
    }

    struct Buddy *nextBud = bud->pair;

    if (bud->space == nextBud->space && bud->free == nextBud->free && bud->free == 1) {
        struct Buddy *temp = nextBud->pair;

        free(nextBud);

        bud->space = bud->space * 2;
        bud->pair = temp;

        mergeBuddy(bud);
        return;
    }
}
