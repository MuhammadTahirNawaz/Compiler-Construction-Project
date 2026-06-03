int globalVar = 10;
while (globalVar > 0) {
    int level1 = 5;
    if (level1 == 5) {
        int level2 = 2;
        // Test if the compiler allows accessing variables from parent scopes
        globalVar = globalVar - level2; 
    }
    // Test if level2 is correctly destroyed/inaccessible here
}