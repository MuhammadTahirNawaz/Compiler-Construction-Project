int counter = 0;
float total = 0.0;

while (counter < 5) {
    float local = total + 1.5;
    total = local * 2.0;
    counter = counter + 1;
}

if (total > 3.0) {
    int bonus = 1;
    total = total + bonus;
} else {
    int penalty = 2;
    total = total - penalty;
}
