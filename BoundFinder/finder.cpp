#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <vector>

using namespace std;

double A(double t, double Z) {
    return exp(-t * Z) / (2 - exp(t));
}
double B(double t, double Z) {
    return exp(exp(t) - 1 - t * Z);
}

struct Result {
    // bound is C * (e^(-t))^ R
    double C;
    double et;
    double t;
};

std::optional<Result> solver(vector<int> Z, double t) {
    if (Z.size() == 1) {
        Z.push_back(Z[0]);
    }
    assert(Z.size() >= 2);
    if (t <= 0 || t >= log(2.0))
        return {};

    int RepZ = Z.back();
    Z.pop_back();

    double g = B(t, Z[0]);
    for (size_t i = 1; i < Z.size(); ++i) {
        g = A(t, Z[i]) * (1 + g) * (1 + g);
    }
    // solve gn = w (g+1)**2
    // check

    double w = A(t, RepZ);
    if (w <= 0 || w >= 1. / 4)
        return {};
    double gp = ((1 - 2 * w) + sqrt(1 - 4 * w)) / (2 * w);
    if (g >= gp)
        return {};

    Result R;
    R.C = gp;
    R.et = exp(-t);
    R.t = t;
    return R;
}

struct ResultRow {
    Result R;
    double MaxT;
    double MinT;
};

std::optional<ResultRow> solver(vector<int> Z) {
    double MaxT = NAN, MinT = NAN;
    double gap = 1e-6, t = 0;
    double ln2 = log(2.0);
    while (t < ln2) {
        if (auto R = solver(Z, t); R.has_value()) {
            MaxT = fmax(MaxT, t);
            MinT = fmin(MinT, t);
        }
        t += gap;
    }

    if (isnan(MaxT))
        return {};
    double tg = std::trunc(MaxT * 1000.0) / 1000.0;
    tg = fmax(MinT, tg);

    ResultRow R;
    R.R = solver(Z, tg).value();
    R.MaxT = MaxT;
    R.MinT = MinT;
    return R;
}

void PrintConf(const vector<int> &Z) {
    cout << "$(";
    for (size_t i = 0; i < Z.size(); ++i) {
        if (i)
            cout << ",";
        cout << Z[i];
    }
    cout << ")$";
}

double avgZ(vector<int> Z) {
    double sum = 0;

    auto getZ = [&](size_t idx) {
        if (idx < Z.size())
            return Z[idx];
        return Z.back();
    };

    // A lazy approach
    for (int i = 0; i < 400; ++i) {
        sum += getZ(i) * pow(2, -(i + 1));
    }

    return sum;
}

double Up(double V) {
    return (std::ceil(V * 1000)) / 1000;
}

double Down(double V) {
    return (std::floor(V * 1000)) / 1000;
}

int main() {
    cout << fixed << setprecision(4);

    // {
    //     Result R = solver({4, 4, 4, 4, 5}, 0.529).value();
    //     cout << R.C << ' ' << R.et << ' ' << R.t << endl;
    // }

    vector<vector<int>> Cases = {
        {5},
        {4, 5},
        {4, 4, 5},
        {4, 4, 4, 5},
        {4, 4, 4, 4, 5},
        {4, 4, 4, 4, 4, 5},
        {1, 5},
        {0, 5},
        {1, 5, 4, 5},
        {1, 5, 5, 4, 5},
        {0, 5, 4, 5},
        {1, 4, 4, 5},
        {1, 5, 3, 5},
        {4},
    };

    cout << R"(\textbf{Conf.} & \textbf{avg.$Z$} & \textbf{feasible $t$} & \textbf{bound}\\\hline)" << '\n';
    for (const auto &conf : Cases) {

        auto Res = solver(conf);

        PrintConf(conf);
        cout << " & ";

        if (Res.has_value()) {
            auto R = Res.value();
            cout << fixed << setprecision(4);
            cout << avgZ(conf) << " & ";
            cout << fixed << setprecision(3);
            cout << "$[" << Up(R.MinT) << "," << Down(R.MaxT) << "]$" << " & ";
            cout << "$" << Up(R.R.C) << "\\times " << Up(R.R.et) << "^R$";
        } else {
            cout << R"(\multicolumn{3}{c}{divergent})";
        }

        cout << R"( \\)";
        if (Res.has_value()) { // Debug
            auto R = Res.value();
            cout << "% " << R.R.t;
        }

        cout << "\n";
    }
}
