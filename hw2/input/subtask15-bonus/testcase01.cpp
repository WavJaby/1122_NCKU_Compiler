// Calculate Pi using the Nilakantha series
double calculate_pi(int terms) {
    double pi = 3.0;  // Start with the first term of the series
    bool add = true;  // Control addition or subtraction

    for (int k = 1; k < terms; k++) {
        double twok = (double)2 * (double)k;  // Cached value
        double term = (double)4 / (twok * (twok + (double)1) * (twok + (double)2));
        if (add) {
            pi += term;
            add = false;
        } else {
            pi -= term;
            add = true;
        }
    }

    return pi;
}

int main(string argv[]) {
    int terms = 100000;  // Number of terms to sum
    double pi = calculate_pi(terms);
    cout << "Approximation of Pi after " << terms << " terms: " << pi << endl;
    return 0;
}