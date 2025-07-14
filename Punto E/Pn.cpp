#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
using namespace std;

// nCr
double comb(int n, int r)
{
    if (r < 0 || r > n)
        return 0;
    double res = 1.0;
    for (int i = 1; i <= r; ++i)
        res = res * (n - i + 1) / i;
    return res;
}

int main()
{
    const int m = 5, N = 5;
    const double p = 0.3, s = 0.4;

    // Matriz de transición
    vector<vector<double>> A(N + 1, vector<double>(N + 1, 0.0));

    for (int n = 0; n <= N; ++n)
    {
        int busy = std::min(n, m);
        double arrival = (n < N) ? p : 0.0;
        double no_arrival = 1.0 - p;
        for (int d = 0; d <= busy; ++d)
        {
            double depart_prob = comb(busy, d) * pow(s, d) * pow(1 - s, busy - d);
            int next = n - d;
            if (next < 0)
                next = 0;
            if (n < N && (n - d + 1) <= N)
            {
                A[n][n - d + 1] += arrival * depart_prob;
            }

            A[n][next] += no_arrival * depart_prob;
        }
    }

    vector<vector<double>> M(N + 2, vector<double>(N + 2, 0.0));
    vector<double> b(N + 2, 0.0);

    for (int i = 0; i <= N; ++i)
        for (int j = 0; j <= N; ++j)
            M[i][j] = A[j][i] - (i == j ? 1.0 : 0.0);

    for (int j = 0; j <= N; ++j)
        M[N + 1][j] = 1.0;
    b[N + 1] = 1.0;

    vector<double> P(N + 1, 0.0);
    for (int i = 0; i <= N; ++i)
        M[i][N + 1] = 0.0; // Col extra

    for (int k = 0; k <= N; ++k)
    {
        double maxval = fabs(M[k][k]);
        int maxrow = k;
        for (int i = k + 1; i <= N + 1; ++i)
        {
            if (fabs(M[i][k]) > maxval)
            {
                maxval = fabs(M[i][k]);
                maxrow = i;
            }
        }
        if (maxrow != k)
        {
            swap(M[k], M[maxrow]);
            swap(b[k], b[maxrow]);
        }
        for (int i = k + 1; i <= N + 1; ++i)
        {
            double c = (M[i][k] != 0) ? M[i][k] / M[k][k] : 0;
            for (int j = k; j <= N + 1; ++j)
                M[i][j] -= c * M[k][j];
            b[i] -= c * b[k];
        }
    }

    vector<double> x(N + 2, 0.0);
    for (int i = N + 1; i >= 0; --i)
    {
        double sum = 0.0;
        for (int j = i + 1; j <= N + 1; ++j)
            sum += M[i][j] * x[j];
        x[i] = (M[i][i] != 0) ? (b[i] - sum) / M[i][i] : 0.0;
    }
    for (int i = 0; i <= N; ++i)
        P[i] = x[i];

    cout << fixed << setprecision(8);
    cout << "n\tPn(teorico)" << endl;
    for (int i = 0; i <= N; ++i)
        cout << i << "\t" << P[i] << endl;

    double s0 = pow(1 - s, m);
    double Pb = P[N] * s0;
    cout << "Probabilidad de bloqueo (teorico): " << Pb << endl;

    return 0;
}
