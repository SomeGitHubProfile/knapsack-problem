#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

struct Item {
    int price;
    int weight;
    double specific_price;

    Item(int _price, int _weight) noexcept :
        price(_price),
        weight(_weight),
        specific_price(static_cast<double>(price) / weight) {}
};


int solve(int max_weight, const vector<Item>& items) noexcept {
    vector<Item> sorted_items = items;
    sort(sorted_items.begin(), sorted_items.end(), [](const Item& lhs, const Item& rhs) {
        return rhs.specific_price < lhs.specific_price;
    });

    int best_price = 0;
    int weight = 0;
    for (int i = 0; i < items.size() && weight < max_weight; ++i) {
        if (weight + sorted_items[i].weight <= max_weight) {
            best_price += sorted_items[i].price;
            weight += sorted_items[i].weight;
        }
    }
    return best_price;
}

vector<Item> manual_input(int& n, int& max_weight) noexcept {
    cout << "Input number of items: ";
    cin >> n;
    cout << "Input max backpack weight: ";
    cin >> max_weight;
    cout << "Input n pairs: price - weight\n";
    vector<Item> items;
    items.reserve(n);
    for (int i = 0; i < n; ++i) {
        int price, weight;
        cin >> price >> weight;
        items.emplace_back(price, weight);
    }
    return items;
}

vector<Item> file_input(int& n, int& max_weight, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл '" << filename << "'.\n";
        return {};
    }

    file >> n >> max_weight;
    vector<Item> items;
    items.reserve(n);
    for (int i = 0; i < n; ++i) {
        int price, weight;
        file >> price >> weight;
        items.emplace_back(price, weight);
    }
    return items;
}

vector<Item> input_data(int& n, int& max_weight) {
    string filename;
    cout << "Enter input filename or press Enter to manually input data: ";
    getline(cin, filename);
    if (filename.empty()) {
        return manual_input(n, max_weight);
    }
    return file_input(n, max_weight, filename);
}

int main() {
    int n, max_weight;
    vector<Item> items = input_data(n, max_weight);
    auto start = chrono::high_resolution_clock::now();
    int best_price = solve(max_weight, items);
    auto end = chrono::high_resolution_clock::now();
    auto time_spent = end - start;
    cout << "Maximum price that can be taken: " << best_price << '\n';
    cout << "Time spent:\n";
    cout << "t = " << time_spent.count() << " nanoseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::milliseconds>(time_spent).count() << " milliseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::seconds>(time_spent).count() << " seconds\n";
#if defined(NDEBUG)
    std::cout << "Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();
#endif
    return 0;
}