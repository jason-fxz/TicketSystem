// #include <windows.h>
#include <bits/stdc++.h>
using namespace std;
mt19937 rd(random_device{}());
// mt19937 rd(GetTickCount());
int rnd(int l, int r) {
    return rd() % (r - l + 1) + l;
}
// shuffle();
char word[10000][40];
/**
 * @brief 
 
 * 
 * @return int 
 */
int main() {
    // uniform_int_distribution<int> ;
    // uniform_real_distribution<double> ;
    int n = 300000;
    int W = 1000;
    for (int i = 0; i < W; ++i) {
        for (int j = 0; j < 20; ++j) {
            word[i][j] = rnd('a', 'z');
        }
        word[i][20] = '\0';
        // word[i][0] = 'a' + i;
        // word[i][1] = '\0';
    }
    printf("%d\n", n);
    set<pair<int, string>> S;
    for (int i = 1; i <= n; ++i) {
        int op = rnd(1, 10);
        if (op <= 7) {
            string s = word[rnd(0, W - 1)];
            int d = rnd(1, 1000000000);
            S.insert({d, s});
            printf("insert %s %d\n", s.c_str(), d);
        } else if (op <= 9) {
            if (S.empty()) {
                printf("delete %s %d\n", word[rnd(0, W - 1)], rnd(1, 1000000000));
            } else {
                int p = rnd(1, 1000000000);
                auto it = S.lower_bound({p, ""});
                if (it == S.end()) {
                    --it;
                }

                printf("delete %s %d\n", it->second.c_str(), it->first);
                S.erase(it);
            }
        } else {
            printf("find %s\n", word[rnd(0, W - 1)]);
        }
    }
    return 0;
}