// (Test) Return: 200
// switch, including fallthrough between labels, a default, and the way break
// leaves the switch while continue carries on with the enclosing loop. A label
// is any constant expression, folded before the cases are collected.

int classify(int n)
{
    int r;

    r = 0;
    switch (n) {
        case 1:
            r = 10;
            break;
        case 2:
        case 3:
            r = 20;
            break;
        case 4:
            r = 30;
        case 5:
            r += 5;
            break;
        default:
            r = 99;
    }
    return r;
}

int main()
{
    int total;

    if (classify(1) != 10) return 1;
    if (classify(2) != 20) return 2;
    if (classify(3) != 20) return 3;
    if (classify(4) != 35) return 4;
    if (classify(5) != 5) return 5;
    if (classify(9) != 99) return 6;

    total = 0;
    for (int i = 0; i < 6; i++) {
        switch (i) {
            case 0:
                continue;
            case 3:
                break;
            default:
                total += i;
        }
        total += 100;
    }
    if (total != 512) return 7;

    switch ('b') {
        case 'a': return 8;
        case 'b': total += 1; break;
        default: return 9;
    }

    switch (-1) {
        case -1: total += 1; break;
        case 2 * 3: return 10;
        default: return 11;
    }
    switch (6) {
        case -1: return 12;
        case 2 * 3: total -= 1; break;
        default: return 13;
    }

    if (total - 270 != 243) return 14;
    return 200;
}
