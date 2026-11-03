int i = 0;
int sum = 0;

while (i < 5) {
    sum = sum + i;
    i = i + 1;
}

if (sum >= 10) {
    printf("%d\n", sum);
} else {
    printf("%d\n", 0);
}