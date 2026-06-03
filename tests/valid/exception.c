int x = 10;
float y = 5.5;

while (x > 0) {
    // Variable Shadowing: This local 'x' hides the global 'x'
    float x = 2.5; 
    y = y + x * 2.0; 

    if (y > 10.0) {
        // Deep nesting + accessing the shadowed float 'x' from parent block
        int z = 1;
        x = x + 1.0; 
    }
    // The float 'x' is still active here
    x = x - 1.0;
}

// Out of scope check: A new independent block
{
    int x = 100;
    int y = x * (2 + 3); // Tricky operator precedence with parentheses
}